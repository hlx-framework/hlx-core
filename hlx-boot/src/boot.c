#include "boot.h"
#include "module.h"
#include "reflection.h"
#include "log.h"
#include "config.h"
#include "hlx_common.h"
#include <windows.h>
#include <stdbool.h>
#include <string.h>

/* Mirrors hl_setup_t field-for-field so load_plugin's offset matches exactly. */
typedef struct {
    void *file_path;
    void *sys_args;
    int sys_nargs;
    void *throw_jump;
    void *resolve_symbol;
    void *capture_stack;
    void *reload_check;
    void *static_call;
    void *get_wrapper;
    void *profile_event;
    void *before_exit;
    void *vtune_init;
    void *load_plugin;
    void *resolve_type;
    bool static_call_ref;
    int closure_stack_capture;
    bool is_debugger_enabled;
    bool is_debugger_attached;
} hlx_setup_mirror_t;

static const char *LogLevelName(HlxLogLevel level)
{
    switch (level) {
        case HLX_LOG_DEBUG: return "DEBUG";
        case HLX_LOG_INFO: return "INFO";
        case HLX_LOG_ERROR: return "ERROR";
        default: return "?";
    }
}

static HMODULE g_realLibhl;
static hlx_setup_mirror_t *g_setup;

typedef void *(WINAPI *HlDynCallSafeFn)(void *closure, void **args, int nargs, void *isExcept);
static HlDynCallSafeFn g_realHlDynCallSafe;

static void ResolveSetup(void)
{
    g_realLibhl = GetModuleHandleA("libhl.dll");
    if (!g_realLibhl) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] FATAL: libhl.dll not found in process");
        return;
    }
    g_setup = (hlx_setup_mirror_t *)GetProcAddress(g_realLibhl, "hl_setup");
    if (!g_setup) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_setup export not found on libhl.dll");
        return;
    }
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] libhl.dll base=%p hl_setup=%p (load_plugin field at %p)", (void *)g_realLibhl, (void *)g_setup, (void *)&g_setup->load_plugin);

    reflection_resolve_setup(g_realLibhl);
}

static hlx_vclosure_mirror_t g_registerPrefixClosure;
static hlx_vclosure_mirror_t g_registerPostfixClosure;
static hlx_vclosure_mirror_t g_dispatchClosure;
static bool g_haveLoaderClosures = false;

static void hlx_loader_ready_impl(const void *registerPrefixFn, const void *registerPostfixFn, const void *dispatchFn)
{
    if (!registerPrefixFn || !registerPostfixFn || !dispatchFn) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hlx_loader_ready: called with a null closure - ignoring");
        return;
    }
    g_registerPrefixClosure = *(const hlx_vclosure_mirror_t *)registerPrefixFn;
    g_registerPostfixClosure = *(const hlx_vclosure_mirror_t *)registerPostfixFn;
    g_dispatchClosure = *(const hlx_vclosure_mirror_t *)dispatchFn;
    g_haveLoaderClosures = true;
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] hlx_loader_ready: OK - cached registerPrefix.fun=%p registerPostfix.fun=%p dispatch.fun=%p", g_registerPrefixClosure.fun, g_registerPostfixClosure.fun, g_dispatchClosure.fun);
}

HLX_NATIVE_EXPORT(hlp_hlx_loader_ready, "PDDD_v", hlx_loader_ready_impl)

static void ForwardToRegistry(hlx_vclosure_mirror_t *closure, void *key, void *fn, void *receiver, const char *what)
{
    if (!g_haveLoaderClosures) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] %s: hlx_loader_ready hasn't run yet - ignoring", what);
        return;
    }
    void *args[3] = { key, fn, receiver };
    call_closure(closure, args, 3);
}

static void hlx_registry_register_prefix_impl(void *key, void *fn, void *receiver)
{
    ForwardToRegistry(&g_registerPrefixClosure, key, fn, receiver, "hlx_registry_register_prefix");
}

HLX_NATIVE_EXPORT(hlp_hlx_registry_register_prefix, "PDDD_v", hlx_registry_register_prefix_impl)

static void hlx_registry_register_postfix_impl(void *key, void *fn, void *receiver)
{
    ForwardToRegistry(&g_registerPostfixClosure, key, fn, receiver, "hlx_registry_register_postfix");
}

HLX_NATIVE_EXPORT(hlp_hlx_registry_register_postfix, "PDDD_v", hlx_registry_register_postfix_impl)

static void *hlx_dispatch_impl(void *key, void *argsArray)
{
    if (!g_haveLoaderClosures) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hlx_dispatch: hlx_loader_ready hasn't run yet - returning null");
        return NULL;
    }
    void *args[2] = { key, argsArray };
    return call_closure(&g_dispatchClosure, args, 2);
}

HLX_NATIVE_EXPORT(hlp_hlx_dispatch, "PDD_D", hlx_dispatch_impl)

static void *g_bootTargetFun;

static bool hlx_mods_loaded_impl(void)
{
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] hlx_mods_loaded: running module recovery now");
    bool recovered = module_recover(g_bootTargetFun);
    if (recovered) {
        /* Eager, once-per-process whole-module New+Call scan for construct_instance_by_name's
         * type->constructor table - done here, right after recovery succeeds, rather than
         * lazily on a mod's first `new` call: the scan is inherently whole-module regardless
         * of which type triggers it, so laziness would buy nothing and would risk a random
         * hitch mid-gameplay on whichever mod happens to construct something first. */
        reflection_scan_constructors();
    }
    return recovered;
}

// "P_b": niladic Bool return - leading 'P' is HFUN's own kind marker, not an argument char.
HLX_NATIVE_EXPORT(hlp_hlx_mods_loaded, "P_b", hlx_mods_loaded_impl)

static void *WINAPI HookedHlDynCallSafe(void *closure, void **args, int nargs, void *isExcept)
{
    static bool fired = false;
    if (!fired) {
        fired = true;
        hlx_vclosure_mirror_t *orig = (hlx_vclosure_mirror_t *)closure;
        g_bootTargetFun = orig->fun;
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] HookedHlDynCallSafe FIRED (boot call), target=%p", g_bootTargetFun);
        if (g_realLibhl && g_setup && g_setup->load_plugin) {
            HlSysLoadPluginFn loadPlugin = (HlSysLoadPluginFn)GetProcAddress(g_realLibhl, "hl_sys_load_plugin");
            if (loadPlugin) {
                PushModuleName("hlx-loader");
                bool ok = loadPlugin(L"hlx\\loader\\hlx-loader.hl");
                PopModuleName();
                hlx_log(ok ? HLX_LOG_DEBUG : HLX_LOG_ERROR, "[hlx-boot] hl_sys_load_plugin(L\"hlx\\\\loader\\\\hlx-loader.hl\") returned %s", ok ? "true" : "false");
            } else {
                hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_sys_load_plugin export not found on libhl.dll");
            }
        } else {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] skipping hl_sys_load_plugin call - prerequisites not met");
        }
    }
    return g_realHlDynCallSafe(closure, args, nargs, isExcept);
}

static void InstallIATHook(void)
{
    HMODULE exeBase = GetModuleHandleA(NULL);
    if (!exeBase) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: GetModuleHandleA(NULL) failed, err=%lu", GetLastError());
        return;
    }

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)exeBase;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: bad DOS signature on exe base %p", (void *)exeBase);
        return;
    }
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE *)exeBase + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: bad NT signature");
        return;
    }

    IMAGE_DATA_DIRECTORY importDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDir.VirtualAddress == 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: exe has no import directory");
        return;
    }

    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)exeBase + importDir.VirtualAddress);
    for (; imp->Name != 0; imp++) {
        const char *dllName = (const char *)((BYTE *)exeBase + imp->Name);
        if (_stricmp(dllName, "libhl.dll") != 0) continue;

        if (imp->OriginalFirstThunk == 0) {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: libhl.dll import has no OriginalFirstThunk - giving up");
            return;
        }

        PIMAGE_THUNK_DATA origThunk = (PIMAGE_THUNK_DATA)((BYTE *)exeBase + imp->OriginalFirstThunk);
        PIMAGE_THUNK_DATA iatThunk = (PIMAGE_THUNK_DATA)((BYTE *)exeBase + imp->FirstThunk);

        for (; origThunk->u1.AddressOfData != 0; origThunk++, iatThunk++) {
            if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal)) continue;

            PIMAGE_IMPORT_BY_NAME byName = (PIMAGE_IMPORT_BY_NAME)((BYTE *)exeBase + origThunk->u1.AddressOfData);
            if (strcmp((const char *)byName->Name, "hl_dyn_call_safe") != 0) continue;

            void **slot = (void **)&iatThunk->u1.Function;
            g_realHlDynCallSafe = (HlDynCallSafeFn)*slot;

            DWORD oldProtect;
            if (!VirtualProtect(slot, sizeof(void *), PAGE_READWRITE, &oldProtect)) {
                hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: VirtualProtect failed, err=%lu", GetLastError());
                return;
            }
            *slot = (void *)HookedHlDynCallSafe;
            DWORD dummy;
            VirtualProtect(slot, sizeof(void *), oldProtect, &dummy);

            hlx_log(HLX_LOG_DEBUG, "[hlx-boot] InstallIATHook: OK - real hl_dyn_call_safe=%p, IAT slot %p hooked", (void *)g_realHlDynCallSafe, (void *)slot);
            return;
        }
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: hl_dyn_call_safe not found among libhl.dll imports");
        return;
    }
    hlx_log(HLX_LOG_ERROR, "[hlx-boot] InstallIATHook: no libhl.dll import descriptor found");
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        OpenHlxLog();
        LoadLoaderConfig();
        OpenGameLog();
        hlx_log(HLX_LOG_INFO, "[hlx-boot] ==== hlx-boot libhl64.dll loaded via resolve_library(\"std\"), pid=%lu ====", GetCurrentProcessId());
        hlx_log(HLX_LOG_INFO, "[hlx-boot] loader.conf: game_trace=%s log_level=%s", ConfigGameTraceEnabled() ? "true" : "false", LogLevelName(ConfigGetLogLevel()));
        ResolveSetup();
        InstallIATHook();
    } else if (reason == DLL_PROCESS_DETACH) {
        CloseHlxLog();
        CloseGameLog();
    }
    return TRUE;
}

// This DLL fully replaces the "std" library carrier - every other real export must still be forwarded transparently.
#include "hlp_forwards.inc"
