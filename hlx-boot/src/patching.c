#include "patching.h"
#include "reflection.h"
#include "module.h"
#include "boot.h"
#include "hlx_common.h"
#include <MinHook.h>
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static void **g_sortedFunctionStarts = NULL;
static int g_sortedFunctionCount = 0;

static int CompareFunctionAddrs(const void *a, const void *b)
{
    uintptr_t pa = (uintptr_t)*(void *const *)a;
    uintptr_t pb = (uintptr_t)*(void *const *)b;
    return pa < pb ? -1 : (pa > pb ? 1 : 0);
}

// Retries on every call until it succeeds - the module may not be recovered yet on an
// early call, and a transient allocation/scan failure must not permanently disable this
// safety check for the rest of the process. g_sortedFunctionStarts itself is the latch:
// it's only ever set on the success path below.
static void EnsureFunctionBoundaries(void)
{
    if (g_sortedFunctionStarts) return;

    void *codePtr = module_get_code();
    void **functionsPtrs = module_get_functions_ptrs();
    if (!codePtr || !functionsPtrs) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] EnsureFunctionBoundaries: module not recovered yet");
        return;
    }

    int n = ((hlx_code_mirror_t *)codePtr)->nfunctions;
    if (n <= 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] EnsureFunctionBoundaries: code->nfunctions=%d", n);
        return;
    }

    void **addrs = (void **)malloc(sizeof(void *) * n);
    if (!addrs) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] EnsureFunctionBoundaries: allocation failed");
        return;
    }

    int count = 0;
    bool ok = true;
    __try {
        for (int i = 0; i < n; i++) {
            if (functionsPtrs[i]) addrs[count++] = functionsPtrs[i];
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok || count == 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] EnsureFunctionBoundaries: functions_ptrs scan faulted or found nothing");
        free(addrs);
        return;
    }

    qsort(addrs, count, sizeof(void *), CompareFunctionAddrs);
    g_sortedFunctionStarts = addrs;
    g_sortedFunctionCount = count;
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] EnsureFunctionBoundaries: cached %d function start addresses", count);
}

// Nearest known JIT'd function start above fun, or NULL if unknown. MinHook has no
// equivalent of this - it only knows the target address it was given, not where the next
// real function begins, and HL packs functions back-to-back with no padding between them.
static const unsigned char *FindNextFunctionBoundary(const unsigned char *fun)
{
    EnsureFunctionBoundaries();
    if (!g_sortedFunctionStarts || g_sortedFunctionCount == 0) return NULL;

    int lo = 0, hi = g_sortedFunctionCount;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if ((uintptr_t)g_sortedFunctionStarts[mid] <= (uintptr_t)fun)
            lo = mid + 1;
        else
            hi = mid;
    }
    return (lo < g_sortedFunctionCount) ? (const unsigned char *)g_sortedFunctionStarts[lo] : NULL;
}

typedef void (*TrampolineFn)(void);

static void DumpPrologueBytes(const void *fun, int len)
{
    static const char hexDigits[] = "0123456789ABCDEF";
    const unsigned char *p = (const unsigned char *)fun;
    char line[256];
    int pos = 0;

    __try {
        for (int i = 0; i < len && pos < (int)sizeof(line) - 4; i++) {
            unsigned char b = p[i];
            line[pos++] = hexDigits[(b >> 4) & 0xF];
            line[pos++] = hexDigits[b & 0xF];
            line[pos++] = ' ';
        }
        line[pos] = 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] DumpPrologueBytes: dereference FAULTED reading %p", fun);
        return;
    }

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] DumpPrologueBytes: %p, first %d bytes: %s", fun, len, line);
}

static bool g_minHookInitialized = false;

// MinHook decodes and relocates the target's real instructions (CALL/JMP/Jcc, RIP-relative
// operands included) - see documentation/minhook.md. FindNextFunctionBoundary is still
// needed alongside it: MinHook only knows the address it was given, not where the next real
// function starts. A target under 5 bytes (its shortest possible redirect) forces it to pull
// in the next function's own first instruction to make up the difference - anything 5 bytes
// or longer is safe, since a function's own instructions always sum to its own real length.
static bool PatchFunctionPrologue(void *targetFun, void *hookFn, TrampolineFn *outTrampoline, const char *label)
{
    unsigned char *fun = (unsigned char *)targetFun;
    DumpPrologueBytes(fun, 32);

    const unsigned char *hardLimit = FindNextFunctionBoundary(fun);
    if (hardLimit && hardLimit - fun < 5) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): only %d byte(s) before the next known function - refusing", label, (int)(hardLimit - fun));
        return false;
    }

    if (!g_minHookInitialized) {
        MH_STATUS status = MH_Initialize();
        if (status != MH_OK) {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): MH_Initialize failed: %s", label, MH_StatusToString(status));
            return false;
        }
        g_minHookInitialized = true;
    }

    void *original = NULL;
    MH_STATUS status = MH_OK;
    __try {
        status = MH_CreateHook(targetFun, hookFn, &original);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): MH_CreateHook FAULTED - refusing", label);
        return false;
    }
    if (status != MH_OK) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): MH_CreateHook failed: %s", label, MH_StatusToString(status));
        return false;
    }

    status = MH_EnableHook(targetFun);
    if (status != MH_OK) {
        MH_RemoveHook(targetFun);
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): MH_EnableHook failed: %s", label, MH_StatusToString(status));
        return false;
    }

    *outTrampoline = (TrampolineFn)original;
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] PatchFunctionPrologue(%s): OK - %p redirected to %p, trampoline %p", label, targetFun, hookFn, (void *)*outTrampoline);
    return true;
}

#define MAX_PATCHES 256

typedef struct {
    void *realAddress;
    void *receiverCode;
    TrampolineFn trampoline;
    const void *realType;
} PatchEntry;

static PatchEntry g_patches[MAX_PATCHES];
static int g_patchCount = 0;

int install_patch(void *realAddress, const void *realType, void *receiverFn, const unsigned short *label)
{
    if (!realAddress || !receiverFn) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: called with a null realAddress/receiverFn - ignoring");
        return -1;
    }

    hlx_vclosure_mirror_t *receiverClosure = (hlx_vclosure_mirror_t *)receiverFn;
    void *receiverCode = receiverClosure->fun;
    if (!receiverCode) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: receiverFn closure has a null .fun - ignoring");
        return -1;
    }

    // Two different PatchTargetKeys (e.g. an inherited, non-overridden method) can resolve
    // to the same underlying realAddress. Sharing the existing handle is only correct when
    // it's truly the same hook being re-registered - a different receiver here means a
    // second, unrelated hook would otherwise be silently dropped with no error anywhere.
    for (int i = 0; i < g_patchCount; i++) {
        if (g_patches[i].realAddress == realAddress) {
            if (g_patches[i].receiverCode == receiverCode) {
                hlx_log(HLX_LOG_INFO, "[hlx-boot] install_patch: %p already patched - returning existing handle %d", realAddress, i);
                return i;
            }
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: %p already patched by a different receiver (handle %d) - refusing", realAddress, i);
            return -1;
        }
    }

    if (g_patchCount >= MAX_PATCHES) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: MAX_PATCHES (%d) reached - ignoring", MAX_PATCHES);
        return -1;
    }

    char narrowLabel[256];
    narrowLabel[0] = '\0';
    if (label) hlx_narrow_utf16(label, narrowLabel, sizeof(narrowLabel));

    char fullLabel[288];
    if (narrowLabel[0]) wsprintfA(fullLabel, "patch#%d %s", g_patchCount, narrowLabel);
    else wsprintfA(fullLabel, "patch#%d", g_patchCount);

    TrampolineFn trampoline = NULL;
    if (!PatchFunctionPrologue(realAddress, receiverCode, &trampoline, fullLabel)) {
        return -1;
    }

    int handle = g_patchCount++;
    g_patches[handle].realAddress = realAddress;
    g_patches[handle].receiverCode = receiverCode;
    g_patches[handle].trampoline = trampoline;
    g_patches[handle].realType = realType;
    return handle;
}

void *call_original(int handle, void *argsArray)
{
    if (handle < 0 || handle >= g_patchCount) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_original: unknown handle %d - skipping", handle);
        return NULL;
    }
    PatchEntry *e = &g_patches[handle];
    return call_resolved((void *)e->trampoline, e->realType, argsArray);
}

HLX_NATIVE_EXPORT(hlp_hlx_install_patch, "PBBDB_i", install_patch)
HLX_NATIVE_EXPORT(hlp_hlx_call_original, "PiD_D", call_original)
