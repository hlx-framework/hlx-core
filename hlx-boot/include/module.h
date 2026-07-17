#ifndef HLX_MODULE_H
#define HLX_MODULE_H

#include <stdbool.h>

#define HOBJ_KIND 11
#define HFUN_KIND 10
/* hl_type_kind value 18 (hashlink/src/hl.h) - compiled Haxe `enum` type. */
#define HENUM_KIND 18

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
    void *types;
    void **globals;  /* hl_type**, per-slot declared type (not value). */
    void *natives;   /* hl_native*, unused - kept only for layout alignment with hl_code. */
    void *functions; /* hl_function*, storage order (not findex order); index via module_get_functions_indexes(). */
} hlx_code_mirror_t;

bool module_recover(const void *targetFun);
void *module_get_code(void);
void **module_get_functions_ptrs(void);
/* Globals table + per-slot byte offsets (hl_module.globals_data/globals_indexes); index bound is nglobals. */
void *module_get_globals_data(void);
int *module_get_globals_indexes(void);
/* findex -> position in hlx_code_mirror_t.functions; NOT the same as functions_ptrs (already findex-indexed). */
int *module_get_functions_indexes(void);

#endif /* HLX_MODULE_H */
