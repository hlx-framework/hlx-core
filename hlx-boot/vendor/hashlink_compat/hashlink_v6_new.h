#ifndef HLX_HASHLINK_V6_NEW_H
#define HLX_HASHLINK_V6_NEW_H

#include <stdbool.h>

/* HashLink structure layouts: bytecode v6, post-c04f8897 (2026-11-10) ABI.
 *
 * Identical to hashlink_v6_old.h in every field offset - c04f8897 only changed the TYPE of
 * hl_code.types from `hl_type *` to `hl_type **` (each type individually allocated, HOBJ
 * proto table appended after its hl_type), so indexing flips from `types + i` to `types[i]`
 * while every other offset in this DLL's sight stays put. Also carries 8c21e768's
 * vobj_proto -> vobj_table rename, which is a rename only: same offset, same layout. */

typedef struct hlx_v6new_hl_type hlx_v6new_hl_type_t;
typedef struct hlx_v6new_hl_type_obj hlx_v6new_hl_type_obj_t;
typedef struct hlx_v6new_hl_type_fun hlx_v6new_hl_type_fun_t;
typedef struct hlx_v6new_hl_type_enum hlx_v6new_hl_type_enum_t;
typedef struct hlx_v6new_hl_type_virtual hlx_v6new_hl_type_virtual_t;

struct hlx_v6new_hl_type {
    int kind;
    union {
        const unsigned char *abs_name;
        hlx_v6new_hl_type_fun_t *fun;
        hlx_v6new_hl_type_obj_t *obj;
        hlx_v6new_hl_type_enum_t *tenum;
        hlx_v6new_hl_type_virtual_t *virt;
        hlx_v6new_hl_type_t *tparam;
    };
    void **vobj_table;
    unsigned int *mark_bits;
};

typedef struct {
    const unsigned char *name;
    int nparams;
    hlx_v6new_hl_type_t **params;
    int size;
    bool hasptr;
    int *offsets;
} hlx_v6new_hl_enum_construct_t;

typedef struct {
    int op;
    int p1;
    int p2;
    int p3;
    int *extra;
} hlx_v6new_hl_opcode_t;

typedef struct hlx_v6new_hl_function hlx_v6new_hl_function_t;

struct hlx_v6new_hl_function {
    int findex;
    int nregs;
    int nops;
    int ref;
    int nassigns;
    hlx_v6new_hl_type_t *type;
    hlx_v6new_hl_type_t **regs;
    hlx_v6new_hl_opcode_t *ops;
    int *debug;
    int *assigns;
    hlx_v6new_hl_type_obj_t *obj;
    union {
        const unsigned char *name;
        hlx_v6new_hl_function_t *ref;
    } field;
};

typedef struct {
    const char *lib;
    const char *name;
    hlx_v6new_hl_type_t *t;
    int findex;
} hlx_v6new_hl_native_t;

typedef struct {
    int global;
    int nfields;
    int *fields;
} hlx_v6new_hl_constant_t;

typedef struct hlx_v6new_hl_alloc_block hlx_v6new_hl_alloc_block_t;
typedef struct {
    hlx_v6new_hl_alloc_block_t *cur;
} hlx_v6new_hl_alloc_t;

typedef struct {
    int version;
    int nints;
    int nfloats;
    int nstrings;
    int nbytes;
    int ntypes;
    int nglobals;
    int nnatives;
    int nfunctions;
    int nconstants;
    int entrypoint;
    int ndebugfiles;
    bool hasdebug;
    int *ints;
    double *floats;
    char **strings;
    int *strings_lens;
    char *bytes;
    int *bytes_pos;
    char **debugfiles;
    int *debugfiles_lens;
    void **ustrings;
    hlx_v6new_hl_type_t **types;
    hlx_v6new_hl_type_t **globals;
    hlx_v6new_hl_native_t *natives;
    hlx_v6new_hl_function_t *functions;
    hlx_v6new_hl_constant_t *constants;
    hlx_v6new_hl_alloc_t alloc;
    hlx_v6new_hl_alloc_t falloc;
} hlx_v6new_hl_code_t;

#endif /* HLX_HASHLINK_V6_NEW_H */
