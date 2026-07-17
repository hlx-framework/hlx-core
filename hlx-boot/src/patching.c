#include "patching.h"
#include "reflection.h"
#include "module.h"
#include "boot.h"
#include "hlx_common.h"
#include "trampoline.h"
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int ModRMExtraBytes(unsigned char modrm)
{
    unsigned char mod = (modrm >> 6) & 3;
    unsigned char rm = modrm & 7;
    if (mod == 3) return 0;
    if (rm == 4) return -1;
    if (mod == 0) return (rm == 5) ? -1 : 0;
    if (mod == 1) return 1;
    return 4;
}

// Whitelists 4 instruction patterns, refuses (0) on anything else - never CALL/JMP or RIP-relative forms, which would break once copied into the trampoline's own address.
static int DecodeSafeInstruction(const unsigned char *p)
{
    int i = 0;
    bool hasSseprefix = false;
    bool hasRex = false;
    unsigned char rex = 0;

    if (p[i] == 0xF2 || p[i] == 0xF3 || p[i] == 0x66) {
        hasSseprefix = true;
        i++;
    }
    if ((p[i] & 0xF0) == 0x40) {
        hasRex = true;
        rex = p[i];
        i++;
    }

    if (!hasSseprefix && p[i] >= 0x50 && p[i] <= 0x57) return i + 1;

    if (!hasSseprefix && hasRex && (rex & 0x08) && (p[i] == 0x89 || p[i] == 0x8B)) {
        int extra = ModRMExtraBytes(p[i + 1]);
        if (extra < 0) return 0;
        return i + 2 + extra;
    }

    if (!hasSseprefix && hasRex && (rex & 0x08) && p[i] == 0x81) {
        unsigned char modrm = p[i + 1];
        unsigned char reg = (modrm >> 3) & 7;
        if ((modrm & 0xC0) == 0xC0 && (reg == 0 || reg == 5)) return i + 2 + 4;
        return 0;
    }

    if (!hasSseprefix && hasRex && (rex & 0x08) && p[i] == 0x83) {
        unsigned char modrm = p[i + 1];
        unsigned char reg = (modrm >> 3) & 7;
        if ((modrm & 0xC0) == 0xC0 && (reg == 0 || reg == 5)) return i + 2 + 1;
        return 0;
    }

    if (p[i] == 0x0F) {
        unsigned char op2 = p[i + 1];
        if (op2 == 0x10 || op2 == 0x11 || op2 == 0x28 || op2 == 0x29) {
            int extra = ModRMExtraBytes(p[i + 2]);
            if (extra < 0) return 0;
            return i + 3 + extra;
        }
        return 0;
    }

    return 0;
}

static void **g_sortedFunctionStarts = NULL;
static int g_sortedFunctionCount = 0;
static bool g_boundaryBuildAttempted = false;

static int CompareFunctionAddrs(const void *a, const void *b)
{
    uintptr_t pa = (uintptr_t)*(void *const *)a;
    uintptr_t pb = (uintptr_t)*(void *const *)b;
    return pa < pb ? -1 : (pa > pb ? 1 : 0);
}

static void EnsureFunctionBoundaries(void)
{
    if (g_boundaryBuildAttempted) return;
    g_boundaryBuildAttempted = true; /* one attempt per process */

    void *codePtr = module_get_code();
    void **functionsPtrs = module_get_functions_ptrs();
    if (!codePtr || !functionsPtrs) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] EnsureFunctionBoundaries: module not recovered yet - cut-point scans will fall back to the fixed maxScan heuristic alone this run");
        return;
    }

    int n = ((hlx_code_mirror_t *)codePtr)->nfunctions;
    if (n <= 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] EnsureFunctionBoundaries: code->nfunctions=%d - nothing to bound with", n);
        return;
    }

    void **addrs = (void **)malloc(sizeof(void *) * n);
    if (!addrs) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] EnsureFunctionBoundaries: allocation failed - cut-point scans will fall back to the fixed maxScan heuristic alone this run");
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
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] EnsureFunctionBoundaries: functions_ptrs scan faulted or found nothing - cut-point scans will fall back to the fixed maxScan heuristic alone this run");
        free(addrs);
        return;
    }

    qsort(addrs, count, sizeof(void *), CompareFunctionAddrs);
    g_sortedFunctionStarts = addrs;
    g_sortedFunctionCount = count;
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] EnsureFunctionBoundaries: cached %d known function start addresses for cut-point boundary checks", count);
}

// Nearest known function start strictly above fun - a hard ceiling the scan must never cross. NULL if boundaries aren't built or fun is at/after every known address.
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

static int FindSafeCutPoint(const unsigned char *fun, int minLen, int maxScan, const unsigned char *hardLimit)
{
    int pos = 0;
    while (pos < minLen) {
        if (pos >= maxScan) return 0;
        int len = DecodeSafeInstruction(fun + pos);
        if (len <= 0) return 0;
        pos += len;
        if (hardLimit && fun + pos > hardLimit) {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] FindSafeCutPoint: cutting at %d bytes would cross the next known function's start (%p) - refusing rather than spilling into it", pos, (void *)hardLimit);
            return 0;
        }
    }
    return pos;
}

typedef void (*TrampolineFn)(void);

static void DumpPrologueBytes(const void *fun, int len)
{
    static const char hexDigits[] = "0123456789ABCDEF";
    const unsigned char *p = (const unsigned char *)fun;
    char line[256];
    int pos = 0;
    bool readOk = true;

    __try {
        for (int i = 0; i < len && pos < (int)sizeof(line) - 4; i++) {
            unsigned char b = p[i];
            line[pos++] = hexDigits[(b >> 4) & 0xF];
            line[pos++] = hexDigits[b & 0xF];
            line[pos++] = ' ';
        }
        line[pos] = 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        readOk = false;
    }

    if (readOk) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] DumpPrologueBytes: %p, first %d bytes: %s", fun, len, line);
    } else {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] DumpPrologueBytes: dereference FAULTED reading %p - not a valid code address, module recovery likely found the wrong candidate", fun);
    }
}

static bool BuildTrampoline(const unsigned char *fun, int cutLen, TrampolineFn *outTrampoline)
{
    int totalLen = cutLen + 12;
    void *buf = JitAlloc(totalLen);
    if (!buf) return false;
    bool copyOk = true;
    __try {
        memcpy(buf, fun, cutLen);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        copyOk = false;
    }
    if (!copyOk) {
        VirtualFree(buf, 0, MEM_RELEASE);
        return false;
    }
    WriteAbsoluteJumpStub((unsigned char *)buf + cutLen, fun + cutLen);
    *outTrampoline = (TrampolineFn)buf;
    return true;
}

static bool PatchFunctionPrologue(void *targetFun, void *hookFn, TrampolineFn *outTrampoline, const char *label)
{
    unsigned char *fun = (unsigned char *)targetFun;

    DumpPrologueBytes(fun, 32);

    const unsigned char *hardLimit = FindNextFunctionBoundary(fun);
    if (!hardLimit) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] PatchFunctionPrologue(%s): no known next-function boundary available - cut-point scan is only bounded by the fixed maxScan heuristic this run", label);
    }

    int cutLen = 0;
    bool decodeOk = true;
    __try {
        cutLen = FindSafeCutPoint(fun, 12, 64, hardLimit);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        decodeOk = false;
    }
    if (!decodeOk) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): FindSafeCutPoint FAULTED reading %p - refusing to patch", label, targetFun);
        return false;
    }
    if (cutLen <= 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): no safe cut point found - refusing to patch", label);
        return false;
    }

    if (!BuildTrampoline(fun, cutLen, outTrampoline)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): trampoline build failed - refusing to patch", label);
        return false;
    }

    DWORD oldProtect;
    if (!VirtualProtect(fun, 12, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] PatchFunctionPrologue(%s): VirtualProtect failed, err=%lu", label, GetLastError());
        return false;
    }
    WriteAbsoluteJumpStub(fun, hookFn);
    DWORD dummy;
    VirtualProtect(fun, 12, oldProtect, &dummy);

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] PatchFunctionPrologue(%s): OK - cut point %d bytes, patched first 12 bytes at %p to redirect to hook=%p, trampoline at %p resumes original body at %p", label, cutLen, targetFun, hookFn, (void *)*outTrampoline, fun + cutLen);
    return true;
}

#define MAX_PATCHES 256

typedef struct {
    void *realAddress;
    TrampolineFn trampoline;
    const void *realType;
} PatchEntry;

static PatchEntry g_patches[MAX_PATCHES];
static int g_patchCount = 0;

int install_patch(void *realAddress, const void *realType, void *receiverFn)
{
    if (!realAddress || !receiverFn) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: called with a null realAddress/receiverFn - ignoring");
        return -1;
    }

    for (int i = 0; i < g_patchCount; i++) {
        if (g_patches[i].realAddress == realAddress) {
            hlx_log(HLX_LOG_INFO, "[hlx-boot] install_patch: %p already patched - returning existing handle %d", realAddress, i);
            return i;
        }
    }

    if (g_patchCount >= MAX_PATCHES) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: MAX_PATCHES (%d) reached - ignoring", MAX_PATCHES);
        return -1;
    }

    char label[32];
    wsprintfA(label, "patch#%d", g_patchCount);

    hlx_vclosure_mirror_t *receiverClosure = (hlx_vclosure_mirror_t *)receiverFn;
    void *receiverCode = receiverClosure->fun;
    if (!receiverCode) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] install_patch: receiverFn closure has a null .fun - ignoring");
        return -1;
    }

    TrampolineFn trampoline = NULL;
    if (!PatchFunctionPrologue(realAddress, receiverCode, &trampoline, label)) {
        return -1;
    }

    int handle = g_patchCount++;
    g_patches[handle].realAddress = realAddress;
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

HLX_NATIVE_EXPORT(hlp_hlx_install_patch, "PBBD_i", install_patch)
HLX_NATIVE_EXPORT(hlp_hlx_call_original, "PiD_D", call_original)
