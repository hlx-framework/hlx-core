#ifndef HLX_LOG_H
#define HLX_LOG_H

#include <windows.h>
#include <stdbool.h>

typedef enum { HLX_LOG_DEBUG = 0, HLX_LOG_INFO = 1, HLX_LOG_ERROR = 2 } HlxLogLevel;

void GetExeDir(char *outPath, size_t outPathSize);

/* Creates (CreateDirectoryA is idempotent) and returns the hlx/ dir path. */
void EnsureHlxDir(char *outPath, size_t outPathSize);

void OpenHlxLog(void);
void CloseHlxLog(void);

void OpenGameLog(void);
void CloseGameLog(void);

typedef void(WINAPI *HlSysPrintFn)(void *msg);
extern HlSysPrintFn g_realSysPrint;

void *BuildSysPrintTrampoline(void);

typedef bool(WINAPI *HlSysLoadPluginFn)(const wchar_t *file);
extern HlSysLoadPluginFn g_realSysLoadPlugin;
bool WINAPI HookedSysLoadPlugin(const wchar_t *file);

#endif /* HLX_LOG_H */
