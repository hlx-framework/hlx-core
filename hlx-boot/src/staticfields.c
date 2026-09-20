#include "staticfields.h"
#include "module.h"
#include "hlx_common.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *typeName;
    char *fieldName;
    int value;
} hlx_static_field_entry_t;

static hlx_static_field_entry_t *g_entries;
static int g_count;
static int g_capacity;

static void AddEntry(const char *typeName, const char *fieldName, int value)
{
    if (g_count >= g_capacity) {
        int newCap = g_capacity == 0 ? 16 : g_capacity * 2;
        hlx_static_field_entry_t *grown = (hlx_static_field_entry_t *)realloc(g_entries, sizeof(hlx_static_field_entry_t) * newCap);
        if (!grown) return;
        g_entries = grown;
        g_capacity = newCap;
    }
    size_t typeLen = strlen(typeName) + 1;
    size_t fieldLen = strlen(fieldName) + 1;
    char *typeCopy = (char *)malloc(typeLen);
    char *fieldCopy = (char *)malloc(fieldLen);
    if (!typeCopy || !fieldCopy) {
        free(typeCopy);
        free(fieldCopy);
        return;
    }
    memcpy(typeCopy, typeName, typeLen);
    memcpy(fieldCopy, fieldName, fieldLen);
    g_entries[g_count].typeName = typeCopy;
    g_entries[g_count].fieldName = fieldCopy;
    g_entries[g_count].value = value;
    g_count++;
}

/* fi is a flattened cross-hierarchy field index, ancestors first (documentation/type-system.md,
 * an unambiguous wire-format rule the Haxe compiler itself must follow when emitting SetField -
 * so summing declared hl_type_obj::nfields up the .super chain to find where leaf's OWN fields
 * start is safe, ordinary arithmetic over static, file-declared data, not runtime layout logic).
 *
 * Deliberately does NOT resolve a fi that belongs to an ancestor, even though it could keep
 * walking to find one: the real hl_obj_field_fetch (hl.h) doesn't do this same simple walk at
 * all - it resolves against hl_runtime_obj, a cached class-layout structure hl_get_obj_rt builds
 * (src/std/obj.c), which isn't vendored here and pulls in enough of the runtime (GC roots,
 * mutexes) that vendoring it for real isn't the small addition code.c's reader was. Every caller
 * of this file only ever asks about a field declared directly on the type it already named
 * (Farever's $Const has no superclass at all), so there's no reason to carry a hand-derived
 * mirror of hl_get_obj_rt's own tree-walk for a case nothing here actually exercises. */
static hl_obj_field *ResolveOwnField(hl_type_obj *leaf, int fi)
{
    int superFieldOffset = 0;
    for (hl_type *t = leaf->super; t && (t->kind == HOBJ_KIND || t->kind == HSTRUCT_KIND); t = t->obj->super) {
        superFieldOffset += t->obj->nfields;
    }

    if (fi < superFieldOffset || fi >= superFieldOffset + leaf->nfields) return NULL;
    return &leaf->fields[fi - superFieldOffset];
}

void staticfields_index_entrypoint(hl_code *code)
{
    for (int i = 0; i < code->nfunctions; i++) {
        hl_function *fn = &code->functions[i];
        if (fn->findex != code->entrypoint) continue;
        if (!fn->ops || !fn->regs || fn->nops <= 0) return;

        __try {
            for (int k = 2; k < fn->nops; k++) {
                hl_opcode *setField = &fn->ops[k];
                if (setField->op != OSetField) continue;
                int objReg = setField->p1, fi = setField->p2, srcReg = setField->p3;

                hl_opcode *getGlobal = &fn->ops[k - 1];
                if (getGlobal->op != OGetGlobal || getGlobal->p1 != objReg) continue;
                int globalIndex = getGlobal->p2;
                if (globalIndex < 0 || globalIndex >= code->nglobals) continue;

                hl_opcode *intLoad = &fn->ops[k - 2];
                if (intLoad->op != OInt || intLoad->p1 != srcReg) continue;
                int intIndex = intLoad->p2;
                if (intIndex < 0 || intIndex >= code->nints) continue;

                hl_type *globalType = code->globals[globalIndex];
                if (!globalType || (globalType->kind != HOBJ_KIND && globalType->kind != HSTRUCT_KIND) || !globalType->obj) continue;

                hl_obj_field *field = ResolveOwnField(globalType->obj, fi);
                if (!field || !field->name || !globalType->obj->name) continue;

                char typeName[256], fieldName[256];
                hlx_narrow_utf16((const unsigned short *)globalType->obj->name, typeName, sizeof(typeName));
                hlx_narrow_utf16((const unsigned short *)field->name, fieldName, sizeof(fieldName));
                AddEntry(typeName, fieldName, code->ints[intIndex]);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] staticfields_index_entrypoint: faulted mid-scan");
        }
        return;
    }
}

HLX_API bool resolve_static_int_field_literal(const char *typeName, const char *fieldName, int *outValue)
{
    for (int i = 0; i < g_count; i++) {
        if (strcmp(g_entries[i].typeName, typeName) == 0 && strcmp(g_entries[i].fieldName, fieldName) == 0) {
            *outValue = g_entries[i].value;
            return true;
        }
    }
    return false;
}
