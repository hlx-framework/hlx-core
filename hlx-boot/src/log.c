#include "log.h"
#include "boot.h"
#include "config.h"
#include "hlx_common.h"
#include "trampoline.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void GetExeDir(char *outPath, size_t outPathSize)
{
    GetModuleFileNameA(NULL, outPath, (DWORD)outPathSize);
    char *slash = strrchr(outPath, '\\');
    if (slash)
        slash[1] = 0;
    else
        outPath[0] = 0;
}

/* libhl64.dll must stay exe-adjacent (resolve_library("std") probes there); hlx/ doesn't. */
void EnsureHlxDir(char *outPath, size_t outPathSize)
{
    char exeDir[MAX_PATH];
    GetExeDir(exeDir, MAX_PATH);

    strcpy_s(outPath, outPathSize, exeDir);
    strcat_s(outPath, outPathSize, "hlx");
    CreateDirectoryA(outPath, NULL);
}

static void EnsureHlxLogDir(char *outPath, size_t outPathSize)
{
    char hlxDir[MAX_PATH];
    EnsureHlxDir(hlxDir, MAX_PATH);

    strcpy_s(outPath, outPathSize, hlxDir);
    strcat_s(outPath, outPathSize, "\\logs");
    CreateDirectoryA(outPath, NULL);
}

static void WriteLine(HANDLE h, const char *msg)
{
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD written;
    WriteFile(h, msg, (DWORD)strlen(msg), &written, NULL);
    WriteFile(h, "\r\n", 2, &written, NULL);
    FlushFileBuffers(h); /* flush every line - a crash right after must not lose it */
}

/* Opened once at DLL_PROCESS_ATTACH (CREATE_ALWAYS truncates any leftover log) and kept
 * open for the process lifetime - avoids a CreateFileA/CloseHandle round trip per line. */
static HANDLE OpenLogFile(const char *filename)
{
    char path[MAX_PATH];
    EnsureHlxLogDir(path, MAX_PATH);
    strcat_s(path, MAX_PATH, filename);

    return CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

static void CloseLogFile(HANDLE *handle)
{
    if (*handle != INVALID_HANDLE_VALUE) {
        CloseHandle(*handle);
        *handle = INVALID_HANDLE_VALUE;
    }
}

static HANDLE g_hlxLogFile = INVALID_HANDLE_VALUE;

void OpenHlxLog(void)
{
    g_hlxLogFile = OpenLogFile("\\hlx.log");
}

void CloseHlxLog(void)
{
    CloseLogFile(&g_hlxLogFile);
}

void hlx_log(HlxLogLevel level, const char *fmt, ...)
{
    if (level < ConfigGetLogLevel()) return;

    char msg[1024];
    va_list args;
    va_start(args, fmt);
    wvsprintfA(msg, fmt, args);
    va_end(args);
    WriteLine(g_hlxLogFile, msg);
}

/* The game's own trace()/Sys.print calls land here (not hlx.log), gated by game_trace. */
static HANDLE g_gameLogFile = INVALID_HANDLE_VALUE;

void OpenGameLog(void)
{
    if (!ConfigGameTraceEnabled()) return;
    g_gameLogFile = OpenLogFile("\\game_trace.log");
}

void CloseGameLog(void)
{
    CloseLogFile(&g_gameLogFile);
}

HlSysPrintFn g_realSysPrint;

// moduleName is a parameter, not a call-time lookup, so each per-module trampoline attributes trace() correctly at any later call time.
static void HookedSysPrintCommon(void *msg, const char *moduleName)
{
    if (msg) {
        char narrow[1024];
        hlx_narrow_utf16((const unsigned short *)msg, narrow, sizeof(narrow));
        // Sys.println sends the message and a bare "\n" as two separate sys_print calls; skip the bare one since WriteLine already appends its own line ending.
        if (narrow[0] != '\n' || narrow[1] != 0) {
            if (moduleName) {
                // wsprintfA hard-caps output at 1024 regardless of buffer size - do not shrink below that.
                char tagged[1024 + 64];
                wsprintfA(tagged, "[%s] %s", moduleName, narrow);
                WriteLine(g_hlxLogFile, tagged);
            } else if (ConfigGameTraceEnabled()) {
                WriteLine(g_gameLogFile, narrow);
            }
        }
    }
    if (g_realSysPrint) g_realSysPrint(msg);
}

static void WINAPI HookedSysPrintFallback(void *msg)
{
    HookedSysPrintCommon(msg, NULL);
}

void *BuildSysPrintTrampoline(void)
{
    const char *moduleName = CurrentModuleName();
    char *nameCopy = NULL;
    if (moduleName) {
        size_t len = strlen(moduleName) + 1;
        nameCopy = (char *)malloc(len);
        if (nameCopy) memcpy(nameCopy, moduleName, len);
    }

    unsigned char *buf = JitAlloc(22);
    if (!buf) return (void *)&HookedSysPrintFallback;

    int pos = JitEmitMovImm64(buf, 0, JIT_REG_RDX, (unsigned long long)(uintptr_t)nameCopy);
    WriteAbsoluteJumpStub(buf + pos, (void *)&HookedSysPrintCommon);

    return buf;
}

HlSysLoadPluginFn g_realSysLoadPlugin;

bool WINAPI HookedSysLoadPlugin(const wchar_t *file)
{
    char narrowPath[MAX_PATH];
    hlx_narrow_utf16((const unsigned short *)file, narrowPath, sizeof(narrowPath));

    char shortName[64];
    ExtractModuleShortName(narrowPath, shortName, sizeof(shortName));

    PushModuleName(shortName);
    bool result = g_realSysLoadPlugin ? g_realSysLoadPlugin(file) : false;
    PopModuleName();
    return result;
}
