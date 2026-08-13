#ifndef HLX_COMMON_H
#define HLX_COMMON_H

#include <stdbool.h>

// Mirrors an HL closure value; .fun is the callable address, .t its type.
typedef struct {
    void *t;
    void *fun;
    int hasValue;
    void *value;
} hlx_vclosure_mirror_t;

/* hl_add_root/hl_remove_root (real libhl.dll GC exports, gc.c) - resolved once here rather
 * than separately by every consumer, since hlx-boot IS the "std" library carrier itself and
 * never links a real libhl import lib (see hlx-boot/CMakeLists.txt - only user32 is linked).
 * Both registry.c (Registry values) and bus.c (Bus subscriber closures) need to root a
 * mod-owned Dynamic/closure so the GC doesn't collect it out from under this DLL's own
 * process-wide storage - this is that one shared resolution, called once from boot.c's
 * ResolveSetup. hlx_add_root/hlx_remove_root silently no-op if resolution never succeeded
 * (hlx_gc_ready lets a caller refuse up front instead, e.g. registry_register/bus_subscribe). */
void hlx_gc_resolve_setup(void *realLibhlModule);
bool hlx_gc_ready(void);
void hlx_add_root(void **root);
void hlx_remove_root(void **root);

// Truncates rather than overflows; ASCII-range only, not Unicode-correct.
void hlx_narrow_utf16(const unsigned short *wide, char *out, int outSize);

// Reverse of hlx_narrow_utf16 - widens ASCII to UTF16 code units, NUL-terminated. narrow may be NULL (produces an empty string) - callers must never crash on it.
void hlx_widen_ascii(const char *narrow, unsigned short *out, int outSizeInChars);

/* "Current module" identity stack, relocated here from log.c: it already had a
 * non-logging consumer (boot.c's own hlx-loader load bracket) before this file
 * gained a second one (BuildModuleNameTrampoline below), so it was never really
 * log-specific - just historically housed next to its first consumer. */
void PushModuleName(const char *name);
void PopModuleName(void);
const char *CurrentModuleName(void);

// Strips directory prefix and trailing ".hl", e.g. "hlx/mods/MyMod.hl" -> "MyMod".
void ExtractModuleShortName(const char *path, char *out, size_t outSize);

/* Builds a fresh per-module trampoline (same JIT technique as BuildSysPrintTrampoline
 * in log.c) that returns the module's own name, baked in at native-resolution time -
 * see hlp_hlx_module_name below for why this must be built once per module, not shared. */
void *BuildModuleNameTrampoline(void);

#endif /* HLX_COMMON_H */
