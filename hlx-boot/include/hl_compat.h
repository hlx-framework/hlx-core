#ifndef HLX_COMPAT_H
#define HLX_COMPAT_H

#include <stdbool.h>

/* The one place in hlx-boot that knows HashLink's ABI-sensitive structure layouts.
 *
 * Three hl_code layouts coexist in this single compiled DLL (no per-variant #ifdefs) and are
 * picked at runtime; their definitions live side by side in vendor/hashlink_compat/, under
 * distinct prefixes. Everything outside this file goes through the normalized accessors
 * below - no raw code->types, no hlx_code_mirror_t field poking.
 *
 * Every hlx_*_mirror_t here is a HashLink layout shared by ALL three variants (git-verified
 * byte-identical across 12403307 -> HEAD); hl_compat.c static-asserts them against the
 * per-variant vendor structs so that stays true mechanically rather than by inspection. */

typedef enum {
    /* Bytecode v6, hl_code.types is a contiguous hl_type array: index with `types + i`. */
    HLX_VARIANT_V6_OLD = 0,
    /* Bytecode v6, hl_code.types is an hl_type* pointer array: index with `types[i]`. */
    HLX_VARIANT_V6_NEW = 1,
    /* Bytecode v7 (fd8c227a + 9f47d259 layout): file_size/build_id, types_vslots, lnames. */
    HLX_VARIANT_V7 = 2,
    HLX_VARIANT_UNKNOWN = 3
} hl_compat_variant_t;

typedef enum {
    /* Our own vendored hl_code_read (vendor/hashlink/) - always the old ABI for v6 files. */
    HLX_SRC_FILE_PARSE = 0,
    /* The game build's own layout, recovered by module_recover()'s memory scan. */
    HLX_SRC_LIVE_MODULE = 1
} hl_compat_src_t;

typedef struct {
    int ntypes;
    int nglobals;
    int nstrings;
    int nints;
    int nfunctions;
    int nnatives;
    int entrypoint;
} hl_compat_counts_t;

typedef struct {
    hl_compat_variant_t variant;
    hl_compat_src_t src;
    int bytecode_version;
    /* The five cross-validated counts; every count accessor reads these instead of the
     * struct, so a caller outside any __try gets the already-validated value. */
    hl_compat_counts_t counts;
    char evidence[192];
} hl_compat_ctx_t;

const char *hl_compat_variant_name(hl_compat_variant_t variant);

/* Attaches the FILE_PARSE context (and captures the hlboot.dat cross-validation reference)
 * for a freshly parsed hl_code. Call once, immediately after hl_code_read succeeds. */
void hl_compat_register_file_code(const void *code, int bytecode_version);

/* Returns the attached context, or NULL when the code pointer is unknown or was rejected. */
const hl_compat_ctx_t *hl_compat_ctx_for(const void *code);

/* Detects + structurally cross-validates a recovered live hl_code and attaches its context.
 * bytecode_version comes from reflection_get_bytecode_version(); when it is unavailable
 * (< 0) the HLB header byte is read from hlboot.dat directly. Returns false - and attaches
 * nothing - when no supported layout validates, so callers fail closed. */
bool hl_compat_detect_live(const void *code, int bytecode_version);

int hl_compat_ntypes(const void *code);
int hl_compat_nglobals(const void *code);
int hl_compat_nstrings(const void *code);
int hl_compat_nints(const void *code);
int hl_compat_nfunctions(const void *code);
int hl_compat_nnatives(const void *code);
int hl_compat_entrypoint(const void *code);
int hl_compat_get_int(const void *code, int index);

/* Always a usable hl_type*, whichever way the variant stores the types collection. */
void *hl_compat_get_type(const void *code, int index);
void *hl_compat_get_global(const void *code, int index);

/* HLX_SRC_FILE_PARSE producers only (the vendored reader's hl_function shape); returns an
 * element of hl_code.functions for code that owns its own ops/regs. Live-module callers use
 * hl_compat_function_type instead - a live module's ops/regs arena is not ours to read. */
void *hl_compat_function_at(const void *code, int index);
bool hl_compat_function_type(const void *code, int index, void **outType);

/* Reads entrypoint under every supported hl_code layout - the variant of a candidate found
 * by module_recover()'s scan is not known until after that scan has already had to locate
 * the entrypoint function. Returns how many candidate values were written. */
int hl_compat_entrypoint_candidates(const void *code, int *outValues, int maxValues);

/* hl_setup_t.load_plugin for the game's own libhl build; bytecode_version selects between
 * the pre-v6 (no capture_break_context) and v6+ setup prefixes. */
void *hl_compat_setup_load_plugin(const void *setup, int bytecode_version);

typedef struct {
    int kind;
    void *objPtr;
    void **vobj_proto;
    void *mark_bits;
} hlx_type_mirror_t;

typedef struct {
    int nfields;
    int nproto;
    int nbindings;
    const unsigned short *name;
    void *super;
    void *fields;
    void *proto;
    void *bindings;
    void **global_value;
} hlx_type_obj_mirror_t;

/* Mirrors both hl_type_fun and its nested `closure` sub-struct (same {args,ret,nargs,parent}
 * shape) - a bound closure's .t->fun union slot points here, and .parent recovers the original
 * full type stashed by hl_get_closure_type. */
typedef struct {
    void *args;
    void *ret;
    int nargs;
    void *parent;
} hlx_type_fun_mirror_t;

/* Mirrors hl_type_enum's leading fields; name is first (unlike hl_type_obj). */
typedef struct {
    const unsigned short *name;
    int nconstructs;
    void *constructs;
    void **global_value;
} hlx_type_enum_mirror_t;

/* Mirrors hl_enum_construct (hashlink/src/hl.h) in full - construct_enum_by_name below needs
 * every field (nparams/params/offsets), not just name, to allocate+fill a value generically.
 * Bytecode v7 appends pnames after offsets, so these offsets hold for all three variants. */
typedef struct {
    const unsigned short *name;
    int nparams;
    void **params; /* hl_type** */
    int size;
    bool hasptr;
    int *offsets;
} hlx_enum_construct_mirror_t;

typedef struct {
    void *t;
    int nfields;
    int nproto;
    int size;
    int nmethods;
    int nbindings;
    unsigned char pad_size;
    unsigned char largest_field;
    bool hasPtr;
    void **methods;
    int *fields_indexes;
    void *bindings;
    void *parent;
    void *toStringFun;
    void *compareFun;
    void *castFun;
    void *getFieldFun;
    int nlookup;
    int ninterfaces;
    void *lookup;
} hlx_runtime_obj_mirror_t;

typedef struct {
    void *t;
    int hashed_name;
    int field_index;
} hlx_field_lookup_mirror_t;

typedef struct {
    const unsigned short *name;
    void *t;
    int hashed_name;
} hlx_obj_field_mirror_t;

/* Mirrors hl_runtime_binding. `fid` is a GLOBAL field index across the super chain, not local.
 * `closure==NULL` means `ptr` is a pre-built unbound vclosure; `closure!=NULL` means `ptr` is a
 * raw function pointer instead - do not misread one as the other. */
typedef struct {
    void *ptr;
    void *closure;
    int fid;
} hlx_runtime_binding_mirror_t;

/* hl_function layout for bytecode >= 6 - identical in all three hl_compat variants. Must be
 * mirrored in FULL, not just `type`: it is indexed as an array element (functions[pos]), so a
 * truncated mirror scales every nonzero index to the wrong byte offset. */
typedef struct {
    int findex;
    int nregs;
    int nops;
    int ref;
    int nassigns;
    void *type;
    void *regs;
    void *ops;
    void *debug;
    void *assigns;
    void *obj;
    void *field;
} hlx_function_mirror_v6_t;

/* hl_function layout for bytecode <= 5 live modules only - never produced by any hl_compat
 * variant (all three are v6/v7, all with nassigns). */
typedef struct {
    int findex;
    int nregs;
    int nops;
    int ref;
    void *type;
    void *regs;
    void *ops;
    void *debug;
    void *obj;
    void *field;
} hlx_function_mirror_t;

/* Mirrors hl_setup_t field-for-field so load_plugin's offset matches exactly. Bytecode v6+
 * builds insert capture_break_context after capture_stack; bytecode-7-era builds only append
 * trailing fields, which cannot move load_plugin. */
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

typedef struct {
    void *file_path;
    void *sys_args;
    int sys_nargs;
    void *throw_jump;
    void *resolve_symbol;
    void *capture_stack;
    void *capture_break_context;
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
} hlx_setup_mirror_v6_t;

/* Leading hl_module fields module_recover()'s scan walks. The hot-reload fields 9f47d259
 * inserts land AFTER jit_code (before `bool debug`), so this prefix is identical in all
 * three variants - hashlink 12403307 .. HEAD. */
typedef struct {
    void *code;
    int codesize;
    int globals_size;
    void *globals_indexes;
    void *globals_data;
    void *functions_ptrs;
    void *functions_indexes;
    void *jit_code;
} hlx_module_mirror_t;

#endif /* HLX_COMPAT_H */
