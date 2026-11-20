#ifndef HLX_MODULE_H
#define HLX_MODULE_H

#include <stdbool.h>

#define HOBJ_KIND 11
#define HFUN_KIND 10
/* hl_type_kind value 18 (hashlink/src/hl.h) - compiled Haxe `enum` type. */
#define HENUM_KIND 18
/* hl_type_kind value 21 (hashlink/src/hl.h) - compiled Haxe `struct` (not `class`) type; shares
 * hl_type_obj's layout/union slot with HOBJ_KIND (hashlink/src/code.c's hl_read_type handles
 * HOBJ/HSTRUCT identically). ConstructorCollector.cs's ObjectType model covers both (see
 * HLX.Core/Model/HlType.cs) - reflection.c's constructor scan must too, see
 * IsScannableObjectKind in reflection.c. */
#define HSTRUCT_KIND 21

bool module_recover(const void *targetFun);
bool module_is_recovered(void);
void *module_get_code(void);
void **module_get_functions_ptrs(void);
/* Globals table + per-slot byte offsets (hl_module.globals_data/globals_indexes); index bound is nglobals. */
void *module_get_globals_data(void);
int *module_get_globals_indexes(void);
/* findex -> position in hl_code.functions; NOT the same as functions_ptrs (already findex-indexed). */
int *module_get_functions_indexes(void);

#endif /* HLX_MODULE_H */
