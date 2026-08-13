#include "registry.h"
#include "boot.h"
#include "hlx_common.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

#define REGISTRY_NAME_CAP 1024

/* Heap-allocated, singly-linked entries - the plain-C equivalent of imgui_native.cpp's
 * std::vector<PanelEntry*> (see that file's register()/eraseEntryByName, lines ~495-536):
 * indirection is required, not stylistic, because hl_add_root roots the FIXED address
 * &entry->value - a realloc-based growable array would relocate elements (and silently
 * invalidate any root already taken) the moment a later register() call grew it past
 * capacity. A linked list sidesteps that entirely, at the cost of O(n) lookups, which is
 * fine here: bucket sizes (mods/plugins/commands) are expected to stay tiny. */
typedef struct RegistryEntry {
    struct RegistryEntry *next;
    char bucket[REGISTRY_NAME_CAP];
    char key[REGISTRY_NAME_CAP];
    void *value; /* rooted Dynamic - see registry_register */
} RegistryEntry;

static RegistryEntry *g_entries = NULL;

static RegistryEntry *FindEntry(const char *bucket, const char *key)
{
    for (RegistryEntry *e = g_entries; e; e = e->next) {
        if (strcmp(e->bucket, bucket) == 0 && strcmp(e->key, key) == 0) return e;
    }
    return NULL;
}

/* Shared by registry_register (must REPLACE, not duplicate, an existing same-(bucket,key)
 * entry) and registry_unregister - same erase-by-name shape as imgui_native.cpp's
 * eraseEntryByName, adapted to a linked list instead of a vector. */
static void EraseEntry(const char *bucket, const char *key)
{
    RegistryEntry *prev = NULL;
    for (RegistryEntry *e = g_entries; e; prev = e, e = e->next) {
        if (strcmp(e->bucket, bucket) == 0 && strcmp(e->key, key) == 0) {
            hlx_remove_root(&e->value);
            if (prev) prev->next = e->next;
            else g_entries = e->next;
            free(e);
            return;
        }
    }
}

void registry_register(const unsigned short *bucket, const unsigned short *key, void *value)
{
    if (!bucket || !key) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] registry_register: called with a null bucket/key - ignoring");
        return;
    }
    if (!hlx_gc_ready()) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] registry_register: hl_add_root not resolved - ignoring");
        return;
    }

    char narrowBucket[REGISTRY_NAME_CAP], narrowKey[REGISTRY_NAME_CAP];
    hlx_narrow_utf16(bucket, narrowBucket, sizeof(narrowBucket));
    hlx_narrow_utf16(key, narrowKey, sizeof(narrowKey));

    EraseEntry(narrowBucket, narrowKey);

    RegistryEntry *entry = (RegistryEntry *)malloc(sizeof(RegistryEntry));
    if (!entry) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] registry_register: allocation failed for ('%s','%s') - ignoring", narrowBucket, narrowKey);
        return;
    }
    strcpy_s(entry->bucket, sizeof(entry->bucket), narrowBucket);
    strcpy_s(entry->key, sizeof(entry->key), narrowKey);
    entry->value = value;
    entry->next = g_entries;
    g_entries = entry;
    hlx_add_root(&entry->value); /* keeps this mod-owned Dynamic alive - nothing else in the process references it */

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] registry_register: ('%s','%s') -> %p", narrowBucket, narrowKey, value);
}

HLX_NATIVE_EXPORT(hlp_hlx_kv_register, "PBBD_v", registry_register)

void registry_unregister(const unsigned short *bucket, const unsigned short *key)
{
    if (!bucket || !key) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] registry_unregister: called with a null bucket/key - ignoring");
        return;
    }
    char narrowBucket[REGISTRY_NAME_CAP], narrowKey[REGISTRY_NAME_CAP];
    hlx_narrow_utf16(bucket, narrowBucket, sizeof(narrowBucket));
    hlx_narrow_utf16(key, narrowKey, sizeof(narrowKey));
    EraseEntry(narrowBucket, narrowKey);
}

HLX_NATIVE_EXPORT(hlp_hlx_kv_unregister, "PBB_v", registry_unregister)

void *registry_get(const unsigned short *bucket, const unsigned short *key)
{
    if (!bucket || !key) return NULL;
    char narrowBucket[REGISTRY_NAME_CAP], narrowKey[REGISTRY_NAME_CAP];
    hlx_narrow_utf16(bucket, narrowBucket, sizeof(narrowBucket));
    hlx_narrow_utf16(key, narrowKey, sizeof(narrowKey));
    RegistryEntry *e = FindEntry(narrowBucket, narrowKey);
    return e ? e->value : NULL;
}

HLX_NATIVE_EXPORT(hlp_hlx_kv_get, "PBB_D", registry_get)

int registry_count(const unsigned short *bucket)
{
    if (!bucket) return 0;
    char narrowBucket[REGISTRY_NAME_CAP];
    hlx_narrow_utf16(bucket, narrowBucket, sizeof(narrowBucket));
    int count = 0;
    for (RegistryEntry *e = g_entries; e; e = e->next) {
        if (strcmp(e->bucket, narrowBucket) == 0) count++;
    }
    return count;
}

HLX_NATIVE_EXPORT(hlp_hlx_kv_count, "PB_i", registry_count)

/* index is just this bucket's current entries in list order (most-recently-registered
 * first, since register() prepends) - good enough for a single Haxe-side list() call to
 * enumerate a whole bucket in one register()/unregister()-free pass (see Registry.hx). Not
 * a stable identity across mutations; callers must not cache an index. */
void *registry_value_at(const unsigned short *bucket, int index)
{
    if (!bucket || index < 0) return NULL;
    char narrowBucket[REGISTRY_NAME_CAP];
    hlx_narrow_utf16(bucket, narrowBucket, sizeof(narrowBucket));
    int i = 0;
    for (RegistryEntry *e = g_entries; e; e = e->next) {
        if (strcmp(e->bucket, narrowBucket) != 0) continue;
        if (i == index) return e->value;
        i++;
    }
    return NULL;
}

HLX_NATIVE_EXPORT(hlp_hlx_kv_value_at, "PBi_D", registry_value_at)
