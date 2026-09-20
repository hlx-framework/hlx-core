#include "modinfo.h"
#include "boot.h"
#include "hlx_common.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *ReadEntireFileText(const char *path)
{
    FILE *f;
    if (fopen_s(&f, path, "rb") != 0 || !f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0 || size > 0x100000) {
        fclose(f);
        return NULL;
    }
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t read = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[read] = 0;
    return buf;
}

static bool ParseVersionComponents(const char *text, int32_t *outComponents, int *outCount)
{
    int count = 0;
    const char *p = text;
    while (*p == ' ' || *p == '\t') p++;
    while (*p >= '0' && *p <= '9' && count < 4) {
        char *next;
        outComponents[count++] = (int32_t)strtol(p, &next, 10);
        p = next;
        if (*p == '.') p++;
        else break;
    }
    *outCount = count;
    return count > 0;
}

static bool VersionAtLeast(int32_t major, int32_t minor, int32_t patch, int32_t build, const int32_t *min, int minCount)
{
    int32_t bound[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < minCount; i++) bound[i] = min[i];
    int32_t cur[4] = { major, minor, patch, build };
    for (int i = 0; i < 4; i++) {
        if (cur[i] != bound[i]) return cur[i] > bound[i];
    }
    return true;
}

/* An omitted trailing component means "any" here, not "zero": maxGameVersion=1.2 must accept any
 * 1.2.x. Bumping the last WRITTEN component (not always the 4th) into an exclusive bound gives
 * that for free, and a fully-specified max (1.2.3.4 -> bound 1.2.3.5) reduces to the old plain
 * inclusive "<= 1.2.3.4" automatically - same rule either way, no branching on precision here.
 * Mirrors vortex-farever-extension/src/hlboot/version.ts's Version.parseMaxBound exactly. */
static bool VersionBelowMaxBound(int32_t major, int32_t minor, int32_t patch, int32_t build, const int32_t *maxComponents, int maxCount)
{
    int32_t bound[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < maxCount; i++) bound[i] = maxComponents[i];
    bound[maxCount - 1]++;
    int32_t cur[4] = { major, minor, patch, build };
    for (int i = 0; i < 4; i++) {
        if (cur[i] != bound[i]) return cur[i] < bound[i];
    }
    return false;
}

static void TrimInPlace(char *s)
{
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\r') start++;
    size_t len = strlen(start);
    while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t' || start[len - 1] == '\r' || start[len - 1] == '\n')) len--;
    memmove(s, start, len);
    s[len] = 0;
}

bool modinfo_allows_version(const char *modInfoPath, int32_t major, int32_t minor, int32_t patch, int32_t build)
{
    char *text = ReadEntireFileText(modInfoPath);
    if (!text) return true;

    bool allowed = true;
    char *ctx = NULL;
    char *line = strtok_s(text, "\n", &ctx);
    while (line) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            char *key = line;
            char *value = eq + 1;
            TrimInPlace(key);
            TrimInPlace(value);

            int32_t components[4];
            int count;
            if (strcmp(key, "minGameVersion") == 0 && ParseVersionComponents(value, components, &count)) {
                if (!VersionAtLeast(major, minor, patch, build, components, count)) allowed = false;
            } else if (strcmp(key, "maxGameVersion") == 0 && ParseVersionComponents(value, components, &count)) {
                if (!VersionBelowMaxBound(major, minor, patch, build, components, count)) allowed = false;
            }
        }
        line = strtok_s(NULL, "\n", &ctx);
    }

    free(text);
    return allowed;
}

static bool hlx_mod_info_allows_version_impl(const unsigned short *modInfoPathW, int major, int minor, int patch, int build)
{
    char modInfoPath[MAX_PATH];
    hlx_narrow_utf16(modInfoPathW, modInfoPath, MAX_PATH);
    return modinfo_allows_version(modInfoPath, major, minor, patch, build);
}

HLX_NATIVE_EXPORT(hlp_hlx_mod_info_allows_version, "PBiiii_b", hlx_mod_info_allows_version_impl)
