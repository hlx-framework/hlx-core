#include "config.h"
#include "log.h"
#include <windows.h>
#include <string.h>

// hlx/config/loader.conf: plain key=value lines, '#' at line start comments one out.
static bool g_gameTraceEnabled = false;
static HlxLogLevel g_logLevel = HLX_LOG_INFO;

static bool ConfigGetBool(const char *buf, const char *key, bool defaultValue)
{
    size_t keyLen = strlen(key);
    const char *p = buf;
    while ((p = strstr(p, key)) != NULL) {
        bool atLineStart = (p == buf) || (*(p - 1) == '\n') || (*(p - 1) == '\r');
        const char *afterKey = p + keyLen;
        if (atLineStart && *afterKey == '=') {
            const char *value = afterKey + 1;
            if (_strnicmp(value, "false", 5) == 0 || value[0] == '0') return false;
            if (_strnicmp(value, "true", 4) == 0 || value[0] == '1') return true;
        }
        p = afterKey;
    }
    return defaultValue;
}

static HlxLogLevel ConfigGetLogLevelFromBuf(const char *buf, const char *key, HlxLogLevel defaultValue)
{
    size_t keyLen = strlen(key);
    const char *p = buf;
    while ((p = strstr(p, key)) != NULL) {
        bool atLineStart = (p == buf) || (*(p - 1) == '\n') || (*(p - 1) == '\r');
        const char *afterKey = p + keyLen;
        if (atLineStart && *afterKey == '=') {
            const char *value = afterKey + 1;
            if (_strnicmp(value, "DEBUG", 5) == 0) return HLX_LOG_DEBUG;
            if (_strnicmp(value, "INFO", 4) == 0) return HLX_LOG_INFO;
            if (_strnicmp(value, "ERROR", 5) == 0) return HLX_LOG_ERROR;
        }
        p = afterKey;
    }
    return defaultValue;
}

static void WriteDefaultLoaderConfig(const char *path)
{
    static const char DEFAULT_CONFIG[] =
        "# hlx-loader config\r\n"
        "\r\n"
        "# game_trace: capture the GAME's own trace()/Sys.print calls into\r\n"
        "# hlx/logs/game_trace.log. Off by default - this is the game's own\r\n"
        "# output, not hlx-loader/mod diagnostics (those always go to\r\n"
        "# hlx.log regardless of this setting).\r\n"
        "game_trace=false\r\n"
        "\r\n"
        "# log_level: minimum severity written to hlx/logs/hlx.log - DEBUG,\r\n"
        "# INFO, or ERROR. DEBUG adds every internal resolve/scan detail;\r\n"
        "# ERROR shows only refuse/fault paths. INFO (the default) is the\r\n"
        "# recommended level for normal use.\r\n"
        "log_level=INFO\r\n";

    HANDLE h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    DWORD written;
    WriteFile(h, DEFAULT_CONFIG, sizeof(DEFAULT_CONFIG) - 1, &written, NULL);
    CloseHandle(h);
}

static void EnsureHlxConfigDir(char *outPath, size_t outPathSize)
{
    char hlxDir[MAX_PATH];
    EnsureHlxDir(hlxDir, MAX_PATH);

    strcpy_s(outPath, outPathSize, hlxDir);
    strcat_s(outPath, outPathSize, "\\config");
    CreateDirectoryA(outPath, NULL);
}

void LoadLoaderConfig(void)
{
    char path[MAX_PATH];
    EnsureHlxConfigDir(path, MAX_PATH);
    strcat_s(path, MAX_PATH, "\\loader.conf");

    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        WriteDefaultLoaderConfig(path);
        return;
    }

    char buf[4096];
    DWORD readBytes = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &readBytes, NULL);
    buf[readBytes] = 0;
    CloseHandle(h);

    g_gameTraceEnabled = ConfigGetBool(buf, "game_trace", g_gameTraceEnabled);
    g_logLevel = ConfigGetLogLevelFromBuf(buf, "log_level", g_logLevel);
}

bool ConfigGameTraceEnabled(void)
{
    return g_gameTraceEnabled;
}

HlxLogLevel ConfigGetLogLevel(void)
{
    return g_logLevel;
}
