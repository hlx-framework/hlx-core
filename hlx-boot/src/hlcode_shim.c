/* Supplies the handful of externs hashlink's vendored code.c (hlx-boot/vendor/hashlink/code.c)
 * needs but doesn't itself define, so it can be compiled directly into hlx-boot rather than
 * linked against the real libhl.dll (which doesn't export hl_code_read/hl_module_* at all -
 * see vendor/hashlink/README.md). LIBHL_STATIC (set in CMakeLists.txt for this whole target)
 * makes hl.h's HL_API macro expand to plain `extern`, not `__declspec(dllimport)`, so these
 * local definitions don't conflict with the header's own declarations.
 *
 * This file is used ONLY for the one-shot, load-once hlboot.dat parse
 * (reflection_init_constructor_table, reflection.c) - never touched by the live, running
 * module or its GC.
 */
#include "hl.h"
#include <stdlib.h>
#include <string.h>

/* ---- Allocator primitives (hl_malloc/hl_zalloc/hl_alloc_init/hl_free) ----
 *
 * DESIGN CHOICE: private block-chaining shim, NOT GetProcAddress-resolved from the real
 * libhl.dll (even though all 4 are confirmed genuinely exported by it, the same way
 * reflection.c's reflection_resolve_setup already resolves hl_dyn_call/hl_get_obj_rt/etc.).
 * Reasoning: the hl_code* this shim backs is a throwaway, private parse tree - it is never
 * registered with the live GC, never referenced by any live object, and never touches the
 * running module in any way. Routing its allocations through the REAL allocator would mean
 * calling into GC-adjacent machinery from a point in the host process's lifecycle (as early
 * as DLL_PROCESS_ATTACH) that isn't guaranteed safe or even meaningful for that machinery yet
 * (e.g. before hl_module_init's own GC setup has necessarily run) - for zero actual benefit,
 * since nothing about this parse tree needs to coexist with or be visible to the GC.
 *
 * This is modeled directly on HashLink's own real hl_alloc_block/hl_malloc/hl_zalloc/hl_free
 * (hashlink/src/gc.c) - a growable chain of heap blocks, carved into by hl_malloc/hl_zalloc,
 * released block-by-block by hl_free. hl_alloc_block's exact layout is private to whichever
 * .c file implements these 4 functions (hl.h only forward-declares the type: `typedef struct
 * hl_alloc_block hl_alloc_block;`), so this doesn't need to match gc.c's own field layout
 * byte-for-byte, only behave the same way. One deliberate divergence: the real hl_malloc pads
 * every allocation via hl_pad_size/hl_type_size for GC pointer-scanning alignment - skipped
 * here, since this arena is never registered with or scanned by the live GC, so that padding
 * buys nothing for a parse tree that's read once and then torn down.
 *
 * hl_code_free (the only caller, reflection.c) calls hl_free on `code->falloc` (the ops/regs
 * arena) right after the one-time scan has finished extracting everything it needs into
 * hlx-boot's own name-keyed table - that now genuinely returns the memory to the heap.
 * `code->alloc` (types/strings, including every type's .name) is never freed by hl_code_free
 * at all (true of the real HashLink implementation too) and is simply leaked for the
 * process's lifetime, matching this being a one-time, load-once structure - see
 * reflection.c's reflection_init_constructor_table.
 */
struct hl_alloc_block {
    hl_alloc_block *next;
    unsigned char *p;
    int remaining;
};

void hl_alloc_init(hl_alloc *a) {
    a->cur = NULL;
}

void *hl_malloc(hl_alloc *a, int size) {
    if (size <= 0) return NULL;
    hl_alloc_block *b = a->cur;
    if (!b || b->remaining < size) {
        int chunk = size < 65536 ? 65536 : size;
        b = (hl_alloc_block *)malloc(sizeof(hl_alloc_block) + (size_t)chunk);
        if (!b) return NULL;
        b->p = ((unsigned char *)b) + sizeof(hl_alloc_block);
        b->remaining = chunk;
        b->next = a->cur;
        a->cur = b;
    }
    void *ptr = b->p;
    b->p += size;
    b->remaining -= size;
    return ptr;
}

void *hl_zalloc(hl_alloc *a, int size) {
    void *p = hl_malloc(a, size);
    if (p) memset(p, 0, (size_t)size);
    return p;
}

void hl_free(hl_alloc *a) {
    hl_alloc_block *b = a->cur;
    while (b) {
        hl_alloc_block *next = b->next;
        free(b);
        b = next;
    }
    a->cur = NULL;
}

/* ---- hl_hash_gen: real hashing algorithm (HashLink's own std/obj.c), caching side-effect
 * (the global hl_cache table, mutex, ustrdup) deliberately dropped. code.c calls this
 * unconditionally for every object field/proto name while reading types; the resulting
 * hashed_name value is never consulted by reflection.c's constructor scan (which only ever
 * reads a type's .name), so the cache would be pure unused overhead - and pulling in
 * hl_mutex_alloc/hl_lookup_find/hl_lookup_insert/ustrdup for a table nothing reads is exactly
 * the kind of unrelated-subsystem dependency this shim exists to avoid. */
int hl_hash_gen(const uchar *name, bool cache_name) {
    (void)cache_name;
    int h = 0;
    const uchar *p = name;
    while (*p) {
        h = 223 * h + (unsigned)*p;
        p++;
    }
    h %= 0x1FFFFF7B;
    return h;
}

/* ---- hl_utf8_length / hl_from_utf8: ported verbatim from hashlink/src/std/string.c (pure,
 * portable, no OS or GC dependency) - needed because hl_read_ustring decodes every type/field/
 * proto name eagerly while hl_code_read runs (hl_get_ustring, code.c). */
int hl_utf8_length(const vbyte *s, int pos) {
    int len = 0;
    s += pos;
    while (true) {
        unsigned char c = (unsigned)*s;
        len++;
        if (c < 0x80) {
            if (c == 0) {
                len--;
                break;
            }
            s++;
        } else if (c < 0xC0)
            return len - 1;
        else if (c < 0xE0) {
            if ((s[1] & 0x80) == 0) return len - 1;
            s += 2;
        } else if (c < 0xF0) {
            if (((s[1] & s[2]) & 0x80) == 0) return len - 1;
            s += 3;
        } else if (c < 0xF8) {
            if (((s[1] & s[2] & s[3]) & 0x80) == 0) return len - 1;
            len++; /* surrogate pair */
            s += 4;
        } else
            return len;
    }
    return len;
}

int hl_from_utf8(uchar *out, int outLen, const char *str) {
    int p = 0;
    unsigned int c, c2, c3;
    while (p++ < outLen) {
        c = *(unsigned char *)str++;
        if (c < 0x80) {
            if (c == 0) break;
        } else if (c < 0xE0) {
            c2 = *(unsigned char *)str++;
            c = ((c & 0x1F) << 6) | (c2 & 0x3F);
        } else if (c < 0xF0) {
            c2 = *(unsigned char *)str++;
            c3 = *(unsigned char *)str++;
            c = ((c & 0xF) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        } else {
            unsigned int c4;
            c2 = *(unsigned char *)str++;
            c3 = *(unsigned char *)str++;
            c4 = *(unsigned char *)str++;
            c = ((c & 0x7) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
        }
        if (c >= 0x10000) {
            c -= 0x10000;
            *out++ = (uchar)(0xD800 | (c >> 10));
            *out++ = (uchar)(0xDC00 | (c & 0x3FF));
            p++;
        } else
            *out++ = (uchar)c;
    }
    *out = 0;
    return p - 1;
}

/* ---- hl_detect_debugger: only reachable via code.c's own hl_debug_break()/ERROR() path on
 * malformed bytecode input - never hit when parsing a real, well-formed hlboot.dat. Stubbed
 * false rather than left unresolved. */
bool hl_detect_debugger(void) {
    return false;
}

/* ---- hlt_void: only referenced by hl_code_hash_remap_globals, part of HashLink's dev-only
 * hot-reload path - reflection_init_constructor_table never calls it (this shim only calls
 * hl_code_read/hl_code_free). Provided purely to satisfy the linker. */
hl_type hlt_void = { HVOID };
