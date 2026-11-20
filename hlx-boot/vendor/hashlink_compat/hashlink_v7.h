#ifndef HLX_HASHLINK_V7_H
#define HLX_HASHLINK_V7_H

#include <stdbool.h>

/* HashLink structure layouts: bytecode v7, as defined by hashlink fd8c227a + 9f47d259
 * (both 2026-11-18, the tree hlx-framework's reference checkout sits on).
 *
 * Three deltas relative to the v6 layouts, all of which shift offsets this DLL reads:
 *   - fd8c227a inserts `int file_size; unsigned char build_id[16];` right after `version`
 *     (+20 bytes on everything below it);
 *   - 9f47d259 inserts `int *types_vslots;` between `types` and `globals` (+8 on
 *     globals/natives/functions/lnames/constants);
 *   - fd8c227a appends `int *lnames;` after `functions`, and appends `const uchar **pnames`
 *     to hl_enum_construct (existing construct fields keep their offsets).
 * The struct above is what a libhl built from that tree lays out for ANY bytecode file it
 * reads - fd8c227a only fills file_size/build_id from the stream when version >= 7. */

typedef struct hlx_v7_hl_type hlx_v7_hl_type_t;
typedef struct hlx_v7_hl_type_obj hlx_v7_hl_type_obj_t;
typedef struct hlx_v7_hl_type_fun hlx_v7_hl_type_fun_t;
typedef struct hlx_v7_hl_type_enum hlx_v7_hl_type_enum_t;
typedef struct hlx_v7_hl_type_virtual hlx_v7_hl_type_virtual_t;

struct hlx_v7_hl_type {
    int kind;
    union {
        const unsigned char *abs_name;
        hlx_v7_hl_type_fun_t *fun;
        hlx_v7_hl_type_obj_t *obj;
        hlx_v7_hl_type_enum_t *tenum;
        hlx_v7_hl_type_virtual_t *virt;
        hlx_v7_hl_type_t *tparam;
    };
    void **vobj_table;
    unsigned int *mark_bits;
};

typedef struct {
    const unsigned char *name;
    int nparams;
    hlx_v7_hl_type_t **params;
    int size;
    bool hasptr;
    int *offsets;
    const unsigned char **pnames;
} hlx_v7_hl_enum_construct_t;

typedef struct {
    int op;
    int p1;
    int p2;
    int p3;
    int *extra;
} hlx_v7_hl_opcode_t;

typedef struct hlx_v7_hl_function hlx_v7_hl_function_t;

struct hlx_v7_hl_function {
    int findex;
    int nregs;
    int nops;
    int ref;
    int nassigns;
    hlx_v7_hl_type_t *type;
    hlx_v7_hl_type_t **regs;
    hlx_v7_hl_opcode_t *ops;
    int *debug;
    int *assigns;
    hlx_v7_hl_type_obj_t *obj;
    union {
        const unsigned char *name;
        hlx_v7_hl_function_t *ref;
    } field;
};

typedef struct {
    const char *lib;
    const char *name;
    hlx_v7_hl_type_t *t;
    int findex;
} hlx_v7_hl_native_t;

typedef struct {
    int global;
    int nfields;
    int *fields;
} hlx_v7_hl_constant_t;

typedef struct hlx_v7_hl_alloc_block hlx_v7_hl_alloc_block_t;
typedef struct {
    hlx_v7_hl_alloc_block_t *cur;
} hlx_v7_hl_alloc_t;

typedef struct {
    int version;
    int file_size;
    unsigned char build_id[16];
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
    hlx_v7_hl_type_t **types;
    int *types_vslots;
    hlx_v7_hl_type_t **globals;
    hlx_v7_hl_native_t *natives;
    hlx_v7_hl_function_t *functions;
    int *lnames;
    hlx_v7_hl_constant_t *constants;
    hlx_v7_hl_alloc_t alloc;
    hlx_v7_hl_alloc_t falloc;
} hlx_v7_hl_code_t;

#endif /* HLX_HASHLINK_V7_H */
