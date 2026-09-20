#ifndef HLX_STATICFIELDS_H
#define HLX_STATICFIELDS_H

#include <stdbool.h>
#include "hlmodule.h"
#include "boot.h"

/* Indexes every static field literal (the Int(src,ri) -> GetGlobal(obj,g) -> SetField(obj,fi,src)
 * pattern every `static var X = <int literal>` compiles to) found in code's entrypoint, for later
 * by-name lookup via resolve_static_int_field_literal. Must run before hl_code_free frees
 * code->falloc (the ops/regs arena this reads) - reflection_init_constructor_table
 * (reflection.c) owns the one disk parse this answers from and calls this once, right before
 * freeing it. Generic by design: has no idea what type/field name it'll ever be asked for; only
 * Farever-specific code (farever-mods/game-version-driver) knows to ask for
 * "$Const"/"VERSION_MAJOR" etc. */
void staticfields_index_entrypoint(hl_code *code);

/* Cross-DLL entry point: called via GetProcAddress by whatever optional driver hlx-boot loads
 * (hlx/drivers/<kind>/<kind>.dll, see driver.c) from ITS OWN process, not through HL's native
 * resolution at all - so this is a plain exported C function, not an HLX_NATIVE_EXPORT thunk. */
HLX_API bool resolve_static_int_field_literal(const char *typeName, const char *fieldName, int *outValue);

#endif /* HLX_STATICFIELDS_H */
