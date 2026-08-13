#include "hlx_common.h"
#include "trampoline.h"
#include "boot.h"
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef void(WINAPI *HlAddRootFn)(void **r);
typedef void(WINAPI *HlRemoveRootFn)(void **r);
static HlAddRootFn g_hlAddRoot;
static HlRemoveRootFn g_hlRemoveRoot;

void hlx_gc_resolve_setup(void *realLibhlModule)
{
    HMODULE m = (HMODULE)realLibhlModule;
    g_hlAddRoot = (HlAddRootFn)GetProcAddress(m, "hl_add_root");
    g_hlRemoveRoot = (HlRemoveRootFn)GetProcAddress(m, "hl_remove_root");
    if (!g_hlAddRoot || !g_hlRemoveRoot) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hlx_gc_resolve_setup: hl_add_root=%p hl_remove_root=%p - ONE OR MORE MISSING, GC-root-dependent natives (registry/bus) will fail closed", (void *)g_hlAddRoot, (void *)g_hlRemoveRoot);
    } else {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] GC root natives resolved OK");
    }
}

bool hlx_gc_ready(void)
{
    return g_hlAddRoot != NULL && g_hlRemoveRoot != NULL;
}

void hlx_add_root(void **root)
{
    if (g_hlAddRoot) g_hlAddRoot(root);
}

void hlx_remove_root(void **root)
{
    if (g_hlRemoveRoot) g_hlRemoveRoot(root);
}

void hlx_narrow_utf16(const unsigned short *wide, char *out, int outSize)
{
    int i = 0;
    for (; wide && i < outSize - 1 && wide[i]; i++) out[i] = (char)wide[i];
    out[i] = 0;
}

void hlx_widen_ascii(const char *narrow, unsigned short *out, int outSizeInChars)
{
    int i = 0;
    for (; narrow && i < outSizeInChars - 1 && narrow[i]; i++) out[i] = (unsigned short)(unsigned char)narrow[i];
    out[i] = 0;
}

/* strcpy_s aborts on overflow; module/file names are unbounded, so truncate instead. */
static void CopyTruncate(char *out, size_t outSize, const char *in)
{
    if (outSize == 0) return;
    size_t i = 0;
    for (; i < outSize - 1 && in[i]; i++) out[i] = in[i];
    out[i] = 0;
}

#define MAX_MODULE_STACK 8
static char g_moduleStack[MAX_MODULE_STACK][64];
static int g_moduleStackDepth = 0;

void PushModuleName(const char *name)
{
    if (g_moduleStackDepth >= MAX_MODULE_STACK) return;
    CopyTruncate(g_moduleStack[g_moduleStackDepth], sizeof(g_moduleStack[0]), name);
    g_moduleStackDepth++;
}

void PopModuleName(void)
{
    if (g_moduleStackDepth > 0) g_moduleStackDepth--;
}

const char *CurrentModuleName(void)
{
    return g_moduleStackDepth > 0 ? g_moduleStack[g_moduleStackDepth - 1] : NULL;
}

void ExtractModuleShortName(const char *path, char *out, size_t outSize)
{
    const char *base = path;
    for (const char *p = path; *p; p++)
        if (*p == '\\' || *p == '/') base = p + 1;

    CopyTruncate(out, outSize, base);
    size_t len = strlen(out);
    if (len > 3 && _stricmp(out + len - 3, ".hl") == 0) out[len - 3] = 0;
}

#define MODULE_NAME_MAX 64

static void *WINAPI HookedModuleNameFallback(void)
{
    return NULL;
}

void *BuildModuleNameTrampoline(void)
{
    unsigned short *wideCopy = (unsigned short *)malloc(sizeof(unsigned short) * (MODULE_NAME_MAX + 1));
    if (wideCopy) hlx_widen_ascii(CurrentModuleName(), wideCopy, MODULE_NAME_MAX + 1);

    unsigned char *buf = JitAlloc(11);
    if (!buf) return (void *)&HookedModuleNameFallback;

    int pos = JitEmitMovImm64(buf, 0, JIT_REG_RAX, (unsigned long long)(uintptr_t)wideCopy);
    JitEmitRet(buf, pos);

    return buf;
}

HLX_API void *hlp_hlx_module_name(const char **sign)
{
    *sign = "P_B";
    return BuildModuleNameTrampoline();
}
