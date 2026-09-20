#include "driver.h"
#include "boot.h"
#include "log.h"
#include <windows.h>

typedef bool (*GetGameVersionFn)(int32_t *, int32_t *, int32_t *, int32_t *);

static bool g_driverChecked = false;
static bool g_driverValid = false;
static int32_t g_major, g_minor, g_patch, g_build;

static void ResolveGameVersionDriver(void)
{
    if (g_driverChecked) return;
    g_driverChecked = true;

    char hlxDir[MAX_PATH];
    EnsureHlxDir(hlxDir, MAX_PATH);
    char dllPath[MAX_PATH];
    strcpy_s(dllPath, MAX_PATH, hlxDir);
    strcat_s(dllPath, MAX_PATH, "\\drivers\\game_version\\game_version.dll");

    HMODULE driver = LoadLibraryA(dllPath);
    if (!driver) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] ResolveGameVersionDriver: no driver at %s - version gating disabled", dllPath);
        return;
    }

    GetGameVersionFn getVersion = (GetGameVersionFn)GetProcAddress(driver, "hlx_driver_game_version_v1");
    if (!getVersion) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] ResolveGameVersionDriver: %s has no hlx_driver_game_version_v1 export - version gating disabled", dllPath);
        return;
    }

    if (!getVersion(&g_major, &g_minor, &g_patch, &g_build)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] ResolveGameVersionDriver: %s could not resolve the game version - version gating disabled", dllPath);
        return;
    }

    g_driverValid = true;
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] ResolveGameVersionDriver: %s resolved %d.%d.%d.%d", dllPath, g_major, g_minor, g_patch, g_build);
}

bool driver_try_get_game_version(int32_t *outMajor, int32_t *outMinor, int32_t *outPatch, int32_t *outBuild)
{
    ResolveGameVersionDriver();
    if (!g_driverValid) return false;
    *outMajor = g_major;
    *outMinor = g_minor;
    *outPatch = g_patch;
    *outBuild = g_build;
    return true;
}

static bool hlx_try_get_game_version_impl(void *outBuf)
{
    int32_t major, minor, patch, build;
    if (!driver_try_get_game_version(&major, &minor, &patch, &build)) return false;
    int32_t *out = (int32_t *)outBuf;
    out[0] = major;
    out[1] = minor;
    out[2] = patch;
    out[3] = build;
    return true;
}

HLX_NATIVE_EXPORT(hlp_hlx_try_get_game_version, "PB_b", hlx_try_get_game_version_impl)
