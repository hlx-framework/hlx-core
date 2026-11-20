#include "hl_compat.h"
#include "module.h"
#include "boot.h"
#include "hlmodule.h"
#include "hashlink_v6_old.h"
#include "hashlink_v6_new.h"
#include "hashlink_v7.h"
#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define HLX_COMPAT_MAX_CTX 4
#define HLX_COMPAT_MAX_SAMPLES 16
#define HLX_COMPAT_MIN_SAMPLES 8
#define HLX_COMPAT_WHY 320
#define HLX_HL_LAST_KIND 24

_Static_assert(sizeof(hlx_v6old_hl_code_t) == sizeof(hl_code), "v6-old hl_code must match the vendored reader's hl_code");
_Static_assert(offsetof(hlx_v6old_hl_code_t, ntypes) == offsetof(hl_code, ntypes), "ntypes offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, nglobals) == offsetof(hl_code, nglobals), "nglobals offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, nstrings) == offsetof(hl_code, nstrings), "nstrings offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, nints) == offsetof(hl_code, nints), "nints offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, nfunctions) == offsetof(hl_code, nfunctions), "nfunctions offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, nnatives) == offsetof(hl_code, nnatives), "nnatives offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, entrypoint) == offsetof(hl_code, entrypoint), "entrypoint offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, ints) == offsetof(hl_code, ints), "ints offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, types) == offsetof(hl_code, types), "types offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, globals) == offsetof(hl_code, globals), "globals offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, functions) == offsetof(hl_code, functions), "functions offset");
_Static_assert(sizeof(hlx_v6old_hl_code_t) == sizeof(hlx_v6new_hl_code_t), "v6-old and v6-new hl_code differ only in the type of `types`");
_Static_assert(offsetof(hlx_v6old_hl_code_t, types) == offsetof(hlx_v6new_hl_code_t, types), "types offset");
_Static_assert(offsetof(hlx_v6old_hl_code_t, entrypoint) == offsetof(hlx_v6new_hl_code_t, entrypoint), "entrypoint offset");
_Static_assert(sizeof(hlx_v6old_hl_type_t) == sizeof(hlx_v6new_hl_type_t), "hl_type layout is variant-invariant");
_Static_assert(sizeof(hlx_v6old_hl_type_t) == sizeof(hlx_v7_hl_type_t), "hl_type layout is variant-invariant");
_Static_assert(sizeof(hlx_v6old_hl_type_t) == sizeof(hlx_type_mirror_t), "hlx_type_mirror_t must match every variant's hl_type");
_Static_assert(offsetof(hlx_v6old_hl_type_t, kind) == offsetof(hlx_type_mirror_t, kind), "kind offset");
_Static_assert(offsetof(hlx_v6old_hl_type_t, vobj_proto) == offsetof(hlx_type_mirror_t, vobj_proto), "vobj_proto offset");
_Static_assert(offsetof(hlx_v6old_hl_type_t, mark_bits) == offsetof(hlx_type_mirror_t, mark_bits), "mark_bits offset");
_Static_assert(sizeof(hlx_v6old_hl_function_t) == sizeof(hlx_function_mirror_v6_t), "hlx_function_mirror_v6_t must match every variant's hl_function");
_Static_assert(offsetof(hlx_v6old_hl_function_t, type) == offsetof(hlx_function_mirror_v6_t, type), "hl_function.type offset");
_Static_assert(offsetof(hlx_v6old_hl_function_t, ops) == offsetof(hlx_function_mirror_v6_t, ops), "hl_function.ops offset");
_Static_assert(sizeof(hlx_v6old_hl_enum_construct_t) == sizeof(hlx_enum_construct_mirror_t), "hlx_enum_construct_mirror_t must match every variant's hl_enum_construct");
_Static_assert(offsetof(hlx_v6old_hl_enum_construct_t, offsets) == offsetof(hlx_enum_construct_mirror_t, offsets), "hl_enum_construct.offsets offset");
_Static_assert(offsetof(hlx_v7_hl_enum_construct_t, offsets) == offsetof(hlx_enum_construct_mirror_t, offsets), "v7 only appends pnames after offsets");
_Static_assert(sizeof(hlx_v7_hl_enum_construct_t) == sizeof(hlx_enum_construct_mirror_t) + sizeof(void *), "v7 hl_enum_construct must be exactly pnames larger");

typedef struct {
    int index;
    int kind;
    /* Points straight into the file-parsed hl_code's `alloc` arena (hl_get_ustring), which
     * hl_code_free never releases - so no fixed buffer, and a name longer than any buffer
     * could hold can never be truncated into a false mismatch against the live type. */
    const unsigned short *name;
} hl_compat_type_sample_t;

typedef struct {
    bool fromFileParse;
    int ntypes, nglobals, nstrings, nfunctions, entrypoint;
    int nSamples;
    hl_compat_type_sample_t samples[HLX_COMPAT_MAX_SAMPLES];
} hl_compat_ref_t;

static hl_compat_ctx_t g_ctx[HLX_COMPAT_MAX_CTX];
static const void *g_ctxCode[HLX_COMPAT_MAX_CTX];
static int g_ctxCount;
static hl_compat_ref_t g_fileRef;
static bool g_fileRefValid;
static bool g_missingCtxLogged;
static bool g_faultLogged;
static int g_rejectsLogged;
static unsigned char g_hdr[64];
static int g_hdrLen;
static bool g_hdrLoaded;

static bool LoadBootHeader(void);

static void MissingCtx(const char *fn)
{
    if (g_missingCtxLogged) return;
    g_missingCtxLogged = true;
    hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: %s - no validated context for this hl_code, failing closed", fn);
}

static void Faulted(const char *fn, const hl_compat_ctx_t *c)
{
    if (g_faultLogged) return;
    g_faultLogged = true;
    hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: %s - reading the %s hl_code faulted (%s), failing closed", fn,
            c ? hl_compat_variant_name(c->variant) : "unknown", c ? c->evidence : "no context");
}

const char *hl_compat_variant_name(hl_compat_variant_t variant)
{
    switch (variant) {
        case HLX_VARIANT_V6_OLD: return "v6-old";
        case HLX_VARIANT_V6_NEW: return "v6-new";
        case HLX_VARIANT_V7: return "v7";
        default: return "unknown";
    }
}

static const hl_compat_ctx_t *CtxFor(const void *code)
{
    if (!code) return NULL;
    for (int i = 0; i < g_ctxCount; i++) {
        if (g_ctxCode[i] == code && g_ctx[i].variant != HLX_VARIANT_UNKNOWN) return &g_ctx[i];
    }
    return NULL;
}

const hl_compat_ctx_t *hl_compat_ctx_for(const void *code)
{
    return CtxFor(code);
}

static bool Attach(const void *code, hl_compat_variant_t variant, hl_compat_src_t src, int bytecodeVersion,
                   const hl_compat_counts_t *counts, const char *evidence)
{
    if (!code || variant == HLX_VARIANT_UNKNOWN || !counts) return false;
    const hl_compat_ctx_t *existing = CtxFor(code);
    if (existing) return existing->variant == variant;
    if (g_ctxCount >= HLX_COMPAT_MAX_CTX) return false;
    g_ctxCode[g_ctxCount] = code;
    g_ctx[g_ctxCount].variant = variant;
    g_ctx[g_ctxCount].src = src;
    g_ctx[g_ctxCount].bytecode_version = bytecodeVersion;
    g_ctx[g_ctxCount].counts = *counts;
    strcpy_s(g_ctx[g_ctxCount].evidence, sizeof(g_ctx[g_ctxCount].evidence), evidence);
    g_ctxCount++;
    return true;
}

static bool ReadCounts(const void *code, hl_compat_variant_t variant, hl_compat_counts_t *out)
{
    bool ok = false;
    __try {
        switch (variant) {
            case HLX_VARIANT_V6_OLD: {
                const hlx_v6old_hl_code_t *k = (const hlx_v6old_hl_code_t *)code;
                out->ntypes = k->ntypes;
                out->nglobals = k->nglobals;
                out->nstrings = k->nstrings;
                out->nints = k->nints;
                out->nfunctions = k->nfunctions;
                out->nnatives = k->nnatives;
                out->entrypoint = k->entrypoint;
                ok = true;
                break;
            }
            case HLX_VARIANT_V6_NEW: {
                const hlx_v6new_hl_code_t *k = (const hlx_v6new_hl_code_t *)code;
                out->ntypes = k->ntypes;
                out->nglobals = k->nglobals;
                out->nstrings = k->nstrings;
                out->nints = k->nints;
                out->nfunctions = k->nfunctions;
                out->nnatives = k->nnatives;
                out->entrypoint = k->entrypoint;
                ok = true;
                break;
            }
            case HLX_VARIANT_V7: {
                const hlx_v7_hl_code_t *k = (const hlx_v7_hl_code_t *)code;
                out->ntypes = k->ntypes;
                out->nglobals = k->nglobals;
                out->nstrings = k->nstrings;
                out->nints = k->nints;
                out->nfunctions = k->nfunctions;
                out->nnatives = k->nnatives;
                out->entrypoint = k->entrypoint;
                ok = true;
                break;
            }
            default:
                ok = false;
                break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    return ok;
}

static void *ReadType(const void *code, hl_compat_variant_t variant, int index)
{
    void *type = NULL;
    __try {
        switch (variant) {
            case HLX_VARIANT_V6_OLD: {
                const hlx_v6old_hl_code_t *k = (const hlx_v6old_hl_code_t *)code;
                type = k->types ? (void *)&k->types[index] : NULL;
                break;
            }
            case HLX_VARIANT_V6_NEW: {
                const hlx_v6new_hl_code_t *k = (const hlx_v6new_hl_code_t *)code;
                type = k->types ? (void *)k->types[index] : NULL;
                break;
            }
            case HLX_VARIANT_V7: {
                const hlx_v7_hl_code_t *k = (const hlx_v7_hl_code_t *)code;
                type = k->types ? (void *)k->types[index] : NULL;
                break;
            }
            default:
                break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        type = NULL;
    }
    return type;
}

static void *ReadGlobal(const void *code, hl_compat_variant_t variant, int index)
{
    void *type = NULL;
    __try {
        switch (variant) {
            case HLX_VARIANT_V6_OLD: {
                const hlx_v6old_hl_code_t *k = (const hlx_v6old_hl_code_t *)code;
                type = k->globals ? (void *)k->globals[index] : NULL;
                break;
            }
            case HLX_VARIANT_V6_NEW: {
                const hlx_v6new_hl_code_t *k = (const hlx_v6new_hl_code_t *)code;
                type = k->globals ? (void *)k->globals[index] : NULL;
                break;
            }
            case HLX_VARIANT_V7: {
                const hlx_v7_hl_code_t *k = (const hlx_v7_hl_code_t *)code;
                type = k->globals ? (void *)k->globals[index] : NULL;
                break;
            }
            default:
                break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        type = NULL;
    }
    return type;
}

/* shortShape selects the pre-bytecode-6 hl_function layout (no nassigns/assigns), which no
 * hl_compat variant produces - only a live module from an older game build can. */
static void *FunctionAt(const void *code, hl_compat_variant_t variant, bool shortShape, int index)
{
    void *fn = NULL;
    __try {
        switch (variant) {
            case HLX_VARIANT_V6_OLD: {
                const hlx_v6old_hl_code_t *k = (const hlx_v6old_hl_code_t *)code;
                if (k->functions)
                    fn = shortShape ? (void *)&((const hlx_function_mirror_t *)k->functions)[index]
                                    : (void *)&k->functions[index];
                break;
            }
            case HLX_VARIANT_V6_NEW: {
                const hlx_v6new_hl_code_t *k = (const hlx_v6new_hl_code_t *)code;
                if (k->functions)
                    fn = shortShape ? (void *)&((const hlx_function_mirror_t *)k->functions)[index]
                                    : (void *)&k->functions[index];
                break;
            }
            case HLX_VARIANT_V7: {
                const hlx_v7_hl_code_t *k = (const hlx_v7_hl_code_t *)code;
                if (k->functions)
                    fn = shortShape ? (void *)&((const hlx_function_mirror_t *)k->functions)[index]
                                    : (void *)&k->functions[index];
                break;
            }
            default:
                break;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        fn = NULL;
    }
    return fn;
}

int hl_compat_ntypes(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_ntypes");
        return 0;
    }
    return c->counts.ntypes;
}

int hl_compat_nglobals(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_nglobals");
        return 0;
    }
    return c->counts.nglobals;
}

int hl_compat_nstrings(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_nstrings");
        return 0;
    }
    return c->counts.nstrings;
}

int hl_compat_nints(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_nints");
        return 0;
    }
    return c->counts.nints;
}

int hl_compat_nfunctions(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_nfunctions");
        return 0;
    }
    return c->counts.nfunctions;
}

int hl_compat_nnatives(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_nnatives");
        return 0;
    }
    return c->counts.nnatives;
}

int hl_compat_entrypoint(const void *code)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_entrypoint");
        return -1;
    }
    return c->counts.entrypoint;
}

int hl_compat_get_int(const void *code, int index)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_get_int");
        return 0;
    }
    int value = 0;
    bool ok = false;
    __try {
        int *ints = NULL;
        switch (c->variant) {
            case HLX_VARIANT_V6_OLD: ints = ((const hlx_v6old_hl_code_t *)code)->ints; break;
            case HLX_VARIANT_V6_NEW: ints = ((const hlx_v6new_hl_code_t *)code)->ints; break;
            case HLX_VARIANT_V7: ints = ((const hlx_v7_hl_code_t *)code)->ints; break;
            default: break;
        }
        if (ints && index >= 0) {
            value = ints[index];
            ok = true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) Faulted("hl_compat_get_int", c);
    return value;
}

void *hl_compat_get_type(const void *code, int index)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_get_type");
        return NULL;
    }
    if (index < 0 || index >= c->counts.ntypes) return NULL;
    void *type = ReadType(code, c->variant, index);
    if (!type) Faulted("hl_compat_get_type", c);
    return type;
}

void *hl_compat_get_global(const void *code, int index)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_get_global");
        return NULL;
    }
    if (index < 0 || index >= c->counts.nglobals) return NULL;
    void *type = ReadGlobal(code, c->variant, index);
    if (!type) Faulted("hl_compat_get_global", c);
    return type;
}

void *hl_compat_function_at(const void *code, int index)
{
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_function_at");
        return NULL;
    }
    if (c->src != HLX_SRC_FILE_PARSE) {
        if (!g_faultLogged) {
            g_faultLogged = true;
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: hl_compat_function_at asked for a live-module hl_code - "
                    "a running module's ops/regs arena is not ours to read, failing closed");
        }
        return NULL;
    }
    if (index < 0 || index >= c->counts.nfunctions) return NULL;
    void *fn = FunctionAt(code, c->variant, false, index);
    if (!fn) Faulted("hl_compat_function_at", c);
    return fn;
}

bool hl_compat_function_type(const void *code, int index, void **outType)
{
    if (!outType) return false;
    const hl_compat_ctx_t *c = CtxFor(code);
    if (!c) {
        MissingCtx("hl_compat_function_type");
        return false;
    }
    if (index < 0 || index >= c->counts.nfunctions) return false;
    bool shortShape = (c->src == HLX_SRC_LIVE_MODULE) && (c->bytecode_version < 6);
    bool ok = false;
    __try {
        void *fn = FunctionAt(code, c->variant, shortShape, index);
        if (fn) {
            *outType = shortShape ? ((const hlx_function_mirror_t *)fn)->type
                                  : ((const hlx_function_mirror_v6_t *)fn)->type;
            ok = true;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) Faulted("hl_compat_function_type", c);
    return ok;
}

int hl_compat_entrypoint_candidates(const void *code, int *outValues, int maxValues)
{
    if (!code || !outValues || maxValues <= 0) return 0;
    int n = 0;
    __try {
        const hlx_v6old_hl_code_t volatile *k = (const hlx_v6old_hl_code_t volatile *)code;
        if (n < maxValues) outValues[n++] = k->entrypoint;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    __try {
        const hlx_v7_hl_code_t volatile *k = (const hlx_v7_hl_code_t volatile *)code;
        if (n < maxValues) outValues[n++] = k->entrypoint;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return n;
}

void *hl_compat_setup_load_plugin(const void *setup, int bytecode_version)
{
    if (!setup) return NULL;
    /* load_plugin sits at a different offset before/after bytecode v6 added
     * capture_break_context, so a version we don't know must not pick one by default. */
    if (bytecode_version < 0) {
        if (!LoadBootHeader() || g_hdrLen < 4 || g_hdr[0] != 'H' || g_hdr[1] != 'L' || g_hdr[2] != 'B') return NULL;
        bytecode_version = g_hdr[3];
    }
    void *fn = NULL;
    __try {
        fn = bytecode_version >= 6 ? (void *)((const hlx_setup_mirror_v6_t *)setup)->load_plugin
                                   : (void *)((const hlx_setup_mirror_t *)setup)->load_plugin;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        fn = NULL;
    }
    return fn;
}

typedef int (*HlCompatUcmpFn)(const void *a, const void *b);

static HlCompatUcmpFn ResolveUcmp(void)
{
    static HlCompatUcmpFn fn;
    if (!fn) fn = (HlCompatUcmpFn)GetProcAddress(GetModuleHandleA("libhl.dll"), "ucmp");
    return fn;
}

static const char *MarkerExportName(void)
{
    HMODULE libhl = GetModuleHandleA("libhl.dll");
    if (!libhl) return NULL;
    static const char *const kMarkers[] = { "hl_date_new2", "hl_sys_stat2", "hlp_hl_date_new2", "hlp_hl_sys_stat2" };
    for (int i = 0; i < (int)(sizeof(kMarkers) / sizeof(kMarkers[0])); i++) {
        if (GetProcAddress(libhl, kMarkers[i])) return kMarkers[i];
    }
    return NULL;
}

static bool LoadBootHeader(void)
{
    if (g_hdrLoaded) return g_hdrLen > 0;
    g_hdrLoaded = true;
    char path[MAX_PATH];
    GetExeDir(path, MAX_PATH);
    strcat_s(path, MAX_PATH, "hlboot.dat");
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD got = 0;
    BOOL read = ReadFile(h, g_hdr, (DWORD)sizeof(g_hdr), &got, NULL);
    CloseHandle(h);
    if (!read) return false;
    g_hdrLen = (int)got;
    return g_hdrLen > 0;
}

typedef struct {
    const unsigned char *b;
    int size;
    int pos;
    bool error;
} hl_compat_hdr_reader;

/* hashlink/src/code.c's hl_read_index: 7-bit payload with 0x80 = "more", 0x40 = long form,
 * 0x20 = negative. Reproduced here so the counts can be read without the full parser. */
static int HdrIndex(hl_compat_hdr_reader *r)
{
    if (r->error || r->pos >= r->size) {
        r->error = true;
        return 0;
    }
    unsigned char b = r->b[r->pos++];
    if ((b & 0x80) == 0) return b & 0x7F;
    if ((b & 0x40) == 0) {
        if (r->pos + 1 > r->size) {
            r->error = true;
            return 0;
        }
        int v = r->b[r->pos] | ((b & 31) << 8);
        r->pos++;
        return (b & 0x20) == 0 ? v : -v;
    }
    if (r->pos + 3 > r->size) {
        r->error = true;
        return 0;
    }
    int c = r->b[r->pos];
    int d = r->b[r->pos + 1];
    int e = r->b[r->pos + 2];
    r->pos += 3;
    int v = ((b & 31) << 24) | (c << 16) | (d << 8) | e;
    return (b & 0x20) == 0 ? v : -v;
}

static bool DecodeHeaderRef(hl_compat_ref_t *out)
{
    if (!LoadBootHeader()) return false;
    if (g_hdrLen < 4 || g_hdr[0] != 'H' || g_hdr[1] != 'L' || g_hdr[2] != 'B') return false;
    int version = g_hdr[3];
    hl_compat_hdr_reader r;
    r.b = g_hdr;
    r.size = g_hdrLen;
    r.pos = 4;
    r.error = false;
    if (version >= 7) {
        if (g_hdrLen < 24) return false;
        r.pos = 24; /* 'HLB'+version, int file_size, build_id[16] */
    }
    HdrIndex(&r);            /* flags */
    HdrIndex(&r);            /* nints */
    HdrIndex(&r);            /* nfloats */
    out->nstrings = HdrIndex(&r);
    if (version >= 5) HdrIndex(&r); /* nbytes */
    out->ntypes = HdrIndex(&r);
    out->nglobals = HdrIndex(&r);
    HdrIndex(&r);            /* nnatives */
    out->nfunctions = HdrIndex(&r);
    if (version >= 4) HdrIndex(&r); /* nconstants */
    out->entrypoint = HdrIndex(&r);
    if (r.error) return false;
    out->fromFileParse = false;
    out->nSamples = 0;
    return true;
}

static void AddSample(const void *code, hl_compat_ref_t *ref, int index)
{
    if (index < 0 || index >= ref->ntypes || ref->nSamples >= HLX_COMPAT_MAX_SAMPLES) return;
    for (int i = 0; i < ref->nSamples; i++) {
        if (ref->samples[i].index == index) return;
    }

    int kind = -1;
    const unsigned short *name = NULL;
    void *type = ReadType(code, HLX_VARIANT_V6_OLD, index);
    if (!type) return;
    __try {
        const hlx_type_mirror_t *m = (const hlx_type_mirror_t *)type;
        kind = m->kind;
        if (kind == HOBJ_KIND || kind == HSTRUCT_KIND) {
            const hlx_type_obj_mirror_t *o = (const hlx_type_obj_mirror_t *)m->objPtr;
            name = o ? o->name : NULL;
        } else if (kind == HENUM_KIND) {
            const hlx_type_enum_mirror_t *e = (const hlx_type_enum_mirror_t *)m->objPtr;
            name = e ? e->name : NULL;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        kind = -1;
        name = NULL;
    }
    if (kind < 0 || kind >= HLX_HL_LAST_KIND) return;

    hl_compat_type_sample_t *s = &ref->samples[ref->nSamples++];
    s->index = index;
    s->kind = kind;
    s->name = name;
}

static void CaptureFileRef(const void *code)
{
    memset(&g_fileRef, 0, sizeof(g_fileRef));
    g_fileRefValid = false;

    hl_compat_counts_t counts;
    if (!ReadCounts(code, HLX_VARIANT_V6_OLD, &counts)) return;
    if (counts.ntypes <= 0 || counts.nfunctions <= 0) return;

    g_fileRef.fromFileParse = true;
    g_fileRef.ntypes = counts.ntypes;
    g_fileRef.nglobals = counts.nglobals;
    g_fileRef.nstrings = counts.nstrings;
    g_fileRef.nfunctions = counts.nfunctions;
    g_fileRef.entrypoint = counts.entrypoint;

    AddSample(code, &g_fileRef, 0);
    AddSample(code, &g_fileRef, counts.ntypes - 1);
    for (int i = 0; i < counts.ntypes && g_fileRef.nSamples < HLX_COMPAT_MAX_SAMPLES; i++) {
        const hlx_type_mirror_t *t = (const hlx_type_mirror_t *)ReadType(code, HLX_VARIANT_V6_OLD, i);
        if (!t) continue;
        int kind = -1;
        __try {
            kind = t->kind;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            kind = -1;
        }
        if (kind == HOBJ_KIND || kind == HSTRUCT_KIND || kind == HENUM_KIND) AddSample(code, &g_fileRef, i);
    }
    int spread = counts.ntypes / HLX_COMPAT_MIN_SAMPLES;
    if (spread < 1) spread = 1;
    for (int i = 0; i < counts.ntypes && g_fileRef.nSamples < HLX_COMPAT_MIN_SAMPLES; i += spread) {
        AddSample(code, &g_fileRef, i);
    }

    g_fileRefValid = g_fileRef.nSamples > 0;
}

void hl_compat_register_file_code(const void *code, int bytecode_version)
{
    if (!code || CtxFor(code)) return;

    hl_compat_counts_t counts;
    memset(&counts, 0, sizeof(counts));
    if (!ReadCounts(code, HLX_VARIANT_V6_OLD, &counts)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: file-parsed hl_code %p unreadable - live hl_code detection "
                "will have no cross-validation reference", code);
        return;
    }

    if (!Attach(code, HLX_VARIANT_V6_OLD, HLX_SRC_FILE_PARSE, bytecode_version, &counts,
                "vendored vendor/hashlink hl_code_read, always the old ABI")) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: could not attach FILE_PARSE context to hl_code %p", code);
        return;
    }

    CaptureFileRef(code);
    if (!g_fileRefValid) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: could not capture type samples from the file-parsed hl_code - "
                "live hl_code detection will have no structural reference");
    }
}

static bool ValidateVariant(const void *code, hl_compat_variant_t variant, const hl_compat_ref_t *ref, char *why,
                            size_t whySize)
{
    hl_compat_counts_t k;
    memset(&k, 0, sizeof(k));
    if (!ReadCounts(code, variant, &k)) {
        sprintf_s(why, whySize, "hl_code read faulted");
        return false;
    }
    if (k.ntypes != ref->ntypes || k.nglobals != ref->nglobals || k.nstrings != ref->nstrings ||
        k.nfunctions != ref->nfunctions || k.entrypoint != ref->entrypoint) {
        sprintf_s(why, whySize, "counts ntypes=%d/%d nglobals=%d/%d nstrings=%d/%d nfunctions=%d/%d entrypoint=%d/%d",
                  k.ntypes, ref->ntypes, k.nglobals, ref->nglobals, k.nstrings, ref->nstrings, k.nfunctions,
                  ref->nfunctions, k.entrypoint, ref->entrypoint);
        return false;
    }

    HlCompatUcmpFn ucmp = ResolveUcmp();
    for (int i = 0; i < ref->nSamples; i++) {
        const hl_compat_type_sample_t *s = &ref->samples[i];
        void *type = ReadType(code, variant, s->index);
        if (!type) {
            sprintf_s(why, whySize, "types[%d] unreadable", s->index);
            return false;
        }
        int kind = -1;
        __try {
            kind = ((const hlx_type_mirror_t *)type)->kind;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            kind = -1;
        }
        if (kind != s->kind) {
            sprintf_s(why, whySize, "types[%d].kind=%d, expected %d", s->index, kind, s->kind);
            return false;
        }
        if (!s->name) continue;

        const unsigned short *name = NULL;
        __try {
            const hlx_type_mirror_t *m = (const hlx_type_mirror_t *)type;
            if (s->kind == HENUM_KIND) {
                const hlx_type_enum_mirror_t *e = (const hlx_type_enum_mirror_t *)m->objPtr;
                name = e ? e->name : NULL;
            } else {
                const hlx_type_obj_mirror_t *o = (const hlx_type_obj_mirror_t *)m->objPtr;
                name = o ? o->name : NULL;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            name = NULL;
        }
        if (!name) {
            sprintf_s(why, whySize, "types[%d] has no name", s->index);
            return false;
        }
        if (ucmp && ucmp(name, s->name) != 0) {
            sprintf_s(why, whySize, "types[%d] name mismatch", s->index);
            return false;
        }
    }
    return true;
}

static bool BuildRef(hl_compat_ref_t *out)
{
    if (g_fileRefValid) {
        *out = g_fileRef;
        return true;
    }
    return DecodeHeaderRef(out);
}

bool hl_compat_detect_live(const void *code, int bytecode_version)
{
    if (!code) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: detect_live called with a null hl_code");
        return false;
    }
    if (CtxFor(code)) return true;

    if (bytecode_version < 0) {
        if (!LoadBootHeader() || g_hdrLen < 4 || g_hdr[0] != 'H' || g_hdr[1] != 'L' || g_hdr[2] != 'B') {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: cannot determine the bytecode version - rejecting hl_code %p",
                    code);
            return false;
        }
        bytecode_version = g_hdr[3];
    }

    const char *marker = MarkerExportName();
    hl_compat_variant_t order[3];
    int n = 0;
    if (bytecode_version >= 7) {
        order[n++] = HLX_VARIANT_V7;
    } else if (bytecode_version >= 2) {
        if (marker) {
            order[n++] = HLX_VARIANT_V6_NEW;
            order[n++] = HLX_VARIANT_V6_OLD;
        } else {
            order[n++] = HLX_VARIANT_V6_OLD;
            order[n++] = HLX_VARIANT_V6_NEW;
        }
    } else {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: unsupported bytecode version %d - rejecting hl_code %p",
                bytecode_version, code);
        return false;
    }

    hl_compat_ref_t ref;
    memset(&ref, 0, sizeof(ref));
    if (!BuildRef(&ref)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: no hlboot.dat cross-validation reference available - "
                "rejecting hl_code %p rather than guessing a layout", code);
        return false;
    }
    if (n > 1 && ref.nSamples == 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: no type samples to confirm the old/new types ABI against - "
                "rejecting hl_code %p rather than trusting the export heuristic alone", code);
        return false;
    }

    char why[3][HLX_COMPAT_WHY];
    for (int i = 0; i < n; i++) why[i][0] = 0;

    for (int i = 0; i < n; i++) {
        if (!ValidateVariant(code, order[i], &ref, why[i], HLX_COMPAT_WHY)) continue;

        char markerEvidence[96];
        if (order[i] == HLX_VARIANT_V7) {
            strcpy_s(markerEvidence, sizeof(markerEvidence), "bytecode version >= 7");
        } else if (marker) {
            sprintf_s(markerEvidence, sizeof(markerEvidence), "marker %s present", marker);
        } else {
            strcpy_s(markerEvidence, sizeof(markerEvidence), "marker absent");
        }

        hl_compat_counts_t counts;
        if (!ReadCounts(code, order[i], &counts)) continue;

        char evidence[192];
        sprintf_s(evidence, sizeof(evidence), "%s, ntypes=%d cross-checked against hlboot.dat %s", markerEvidence,
                  counts.ntypes, ref.fromFileParse ? "file parse" : "header");
        if (!Attach(code, order[i], HLX_SRC_LIVE_MODULE, bytecode_version, &counts, evidence)) {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: could not attach LIVE_MODULE context to hl_code %p", code);
            return false;
        }
        hlx_log(HLX_LOG_INFO, "[hlx-boot] hl_compat: live hl_code ABI=%s (%s)", hl_compat_variant_name(order[i]),
                evidence);
        return true;
    }

    char detail[1200];
    int pos = 0;
    for (int i = 0; i < n && pos < (int)sizeof(detail) - 1; i++) {
        pos += sprintf_s(detail + pos, sizeof(detail) - (size_t)pos, "%s%s: %s", i ? "; " : "",
                         hl_compat_variant_name(order[i]), why[i]);
    }
    if (g_rejectsLogged < 4) {
        g_rejectsLogged++;
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] hl_compat: REJECTING hl_code %p (bytecode v%d, marker %s) - no supported "
                "layout cross-validates: %s", code, bytecode_version, marker ? marker : "absent", detail);
    }
    return false;
}
