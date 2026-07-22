#include "reflection.h"
#include "module.h"
#include "boot.h"
#include "hlx_common.h"
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef void *(WINAPI *HlDynCallFn)(void *closure, void **args, int nargs);
typedef void *(WINAPI *HlGetObjRtFn)(void *ot);
typedef void *(WINAPI *HlGetObjProtoFn)(void *ot);
typedef void *(WINAPI *HlAllocObjFn)(void *t);
typedef int(WINAPI *UcmpFn)(const void *a, const void *b);
typedef void *(WINAPI *HlDynGetpFn)(void *d, int hfield, void *t);
typedef int(WINAPI *HlHashUtf8Fn)(const char *str);
typedef void *(WINAPI *HlLookupFindFn)(void *l, int size, int hash);

static HlDynCallFn g_hlDynCall;
static HlGetObjRtFn g_hlGetObjRt;
static HlGetObjProtoFn g_hlGetObjProto;
static HlAllocObjFn g_hlAllocObj;
static UcmpFn g_ucmp;
static HlDynGetpFn g_hlDynGetp;
static HlHashUtf8Fn g_hlHashUtf8;
static void *g_hltDynAddr;
static HlLookupFindFn g_hlLookupFind;

void reflection_resolve_setup(void *realLibhlModule)
{
    HMODULE m = (HMODULE)realLibhlModule;
    g_hlDynCall = (HlDynCallFn)GetProcAddress(m, "hl_dyn_call");
    g_hlGetObjRt = (HlGetObjRtFn)GetProcAddress(m, "hl_get_obj_rt");
    g_hlGetObjProto = (HlGetObjProtoFn)GetProcAddress(m, "hl_get_obj_proto");
    g_hlAllocObj = (HlAllocObjFn)GetProcAddress(m, "hl_alloc_obj");
    g_ucmp = (UcmpFn)GetProcAddress(m, "ucmp");
    g_hlDynGetp = (HlDynGetpFn)GetProcAddress(m, "hl_dyn_getp");
    g_hlHashUtf8 = (HlHashUtf8Fn)GetProcAddress(m, "hl_hash_utf8");
    g_hltDynAddr = (void *)GetProcAddress(m, "hlt_dyn");
    g_hlLookupFind = (HlLookupFindFn)GetProcAddress(m, "hl_lookup_find");

    bool allResolved = g_hlDynCall && g_hlGetObjRt && g_hlGetObjProto && g_hlAllocObj && g_ucmp && g_hlDynGetp &&
                        g_hlHashUtf8 && g_hltDynAddr && g_hlLookupFind;
    if (allResolved) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] reflection natives resolved OK");
    } else {
        hlx_log(HLX_LOG_ERROR,
                "[hlx-boot] reflection_resolve_setup: hl_dyn_call=%p hl_get_obj_rt=%p "
                "hl_get_obj_proto=%p hl_alloc_obj=%p ucmp=%p hl_dyn_getp=%p hl_hash_utf8=%p "
                "hlt_dyn=%p hl_lookup_find=%p - ONE OR MORE MISSING, reflection natives will fail closed",
                (void *)g_hlDynCall, (void *)g_hlGetObjRt, (void *)g_hlGetObjProto, (void *)g_hlAllocObj, (void *)g_ucmp,
                (void *)g_hlDynGetp, (void *)g_hlHashUtf8, g_hltDynAddr, (void *)g_hlLookupFind);
    }
}

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

/* Mirrors both hl_type_fun and its nested `closure` sub-struct (same {args,ret,nargs,parent}
 * shape) - a bound closure's .t->fun union slot points here, and .parent recovers the original
 * full type stashed by hl_get_closure_type. */
typedef struct {
    void *args;
    void *ret;
    int nargs;
    void *parent;
} hlx_type_fun_mirror_t;

/* Mirrors hl_type_enum's leading fields; name is first (unlike hl_type_obj). Trailing fields
 * (nconstructs/constructs) are read as opaque - only name/global_value are used here. */
typedef struct {
    const unsigned short *name;
    int nconstructs;
    void *constructs;
    void **global_value;
} hlx_type_enum_mirror_t;

void *resolve_type_by_name(const unsigned short *typeName)
{
    if (!typeName) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_type_by_name: called with null name - returning null");
        return NULL;
    }
    void *codePtr = module_get_code();
    if (!codePtr) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_type_by_name: no live hl_code recovered this run - returning null");
        return NULL;
    }
    if (!g_ucmp) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_type_by_name: ucmp not resolved - returning null");
        return NULL;
    }

    void *result = NULL;
    bool ok = false;
    __try {
        hlx_code_mirror_t *code = (hlx_code_mirror_t *)codePtr;
        hlx_type_mirror_t *types = (hlx_type_mirror_t *)code->types;
        for (int i = 0; i < code->ntypes; i++) {
            hlx_type_mirror_t *t = &types[i];
            const unsigned short *candidateName = NULL;
            if (t->kind == HOBJ_KIND) {
                hlx_type_obj_mirror_t *obj = (hlx_type_obj_mirror_t *)t->objPtr;
                if (obj) candidateName = obj->name;
            } else if (t->kind == HENUM_KIND) {
                /* objPtr is the same union slot reinterpreted per-kind, just read as the enum shape. */
                hlx_type_enum_mirror_t *en = (hlx_type_enum_mirror_t *)t->objPtr;
                if (en) candidateName = en->name;
            } else {
                continue;
            }
            if (!candidateName) continue;
            if (g_ucmp(candidateName, typeName) == 0) {
                result = t;
                break;
            }
        }
        ok = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_type_by_name: code->types scan faulted - returning null");
        return NULL;
    }

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] resolve_type_by_name: -> %p", result);
    return result;
}

/* Signature strings follow HL's native-signature encoding: kind char, then one char
 * per argument, then '_', then the return type's char. */
HLX_NATIVE_EXPORT(hlp_hlx_resolve_type, "PB_B", resolve_type_by_name)

typedef enum { MEMBER_NOT_FOUND, MEMBER_METHOD, MEMBER_FIELD } MemberLookupResult;

/* Mirrors HL's own obj_resolve_field: methods[] is always read off the originally-resolved
 * type, never the ancestor the name was found on, to reproduce correct virtual dispatch. */
static MemberLookupResult LookupMemberOnType(const void *resolvedType, const unsigned short *memberName, void **outAddr,
                                             void **outType)
{
    if (!g_hlGetObjProto || !g_hlLookupFind || !g_hlHashUtf8) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: required exports not resolved - returning not-found");
        return MEMBER_NOT_FOUND;
    }

    MemberLookupResult verdict = MEMBER_NOT_FOUND;
    bool ok = false;
    __try {
        const hlx_type_mirror_t *t = (const hlx_type_mirror_t *)resolvedType;
        if (t->kind != HOBJ_KIND) {
            ok = true;
        } else {
            char narrow[256];
            hlx_narrow_utf16(memberName, narrow, sizeof(narrow));
            int hash = g_hlHashUtf8(narrow);

            /* hl_get_obj_proto's own signature isn't const-aware. */
            hlx_runtime_obj_mirror_t *origRt = (hlx_runtime_obj_mirror_t *)g_hlGetObjProto((void *)resolvedType);
            if (!origRt) {
                ok = false;
            } else {
                hlx_runtime_obj_mirror_t *rt = origRt;
                hlx_field_lookup_mirror_t *f = NULL;
                while (rt) {
                    f = (hlx_field_lookup_mirror_t *)g_hlLookupFind(rt->lookup, rt->nlookup, hash);
                    if (f) break;
                    rt = (hlx_runtime_obj_mirror_t *)rt->parent;
                }

                if (!f) {
                    verdict = MEMBER_NOT_FOUND;
                    ok = true;
                } else if (f->field_index >= 0) {
                    verdict = MEMBER_FIELD;
                    ok = true;
                } else {
                    int methodIndex = -(f->field_index) - 1;
                    if (methodIndex < 0 || methodIndex >= origRt->nmethods) {
                        ok = false;
                    } else {
                        *outAddr = origRt->methods[methodIndex];
                        if (outType) *outType = f->t;
                        verdict = MEMBER_METHOD;
                        ok = true;
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: LookupMemberOnType faulted - treating as not-found");
        return MEMBER_NOT_FOUND;
    }
    return verdict;
}

/* Covers ctors and `dyn`-declared members via the class's global_value, not the instance
 * method table. A `dyn` member comes back as a bound closure whose .t drops the receiver
 * arg - recover the real type via cl->t->fun->parent, or it silently mismarshals. */
static void *LookupClassMemberViaReflection(const void *resolvedType, const unsigned short *memberName, void **outType)
{
    if (!g_hlDynGetp || !g_hlHashUtf8 || !g_hltDynAddr) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: required exports not resolved - returning null");
        return NULL;
    }

    void *result = NULL;
    bool ok = false;
    __try {
        const hlx_type_mirror_t *t = (const hlx_type_mirror_t *)resolvedType;
        if (t->kind != HOBJ_KIND) {
            ok = true;
        } else {
            hlx_type_obj_mirror_t *obj = (hlx_type_obj_mirror_t *)t->objPtr;
            if (!obj || !obj->global_value || !*obj->global_value) {
                ok = true;
            } else {
                void *classGlobal = *obj->global_value;
                char narrow[256];
                hlx_narrow_utf16(memberName, narrow, sizeof(narrow));
                int hash = g_hlHashUtf8(narrow);
                void *closure = g_hlDynGetp(classGlobal, hash, g_hltDynAddr);
                if (!closure) {
                    ok = true;
                } else {
                    hlx_vclosure_mirror_t *cl = (hlx_vclosure_mirror_t *)closure;
                    void *realType = cl->t;
                    if (cl->hasValue == 1) {
                        hlx_type_mirror_t *reducedType = (hlx_type_mirror_t *)cl->t;
                        if (!reducedType || reducedType->kind != HFUN_KIND) {
                            hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: '%s' is a bound closure but its type kind %d is not HFUN(%d) - refusing to guess", narrow, reducedType ? reducedType->kind : -1, HFUN_KIND);
                            result = NULL;
                        } else {
                            hlx_type_fun_mirror_t *reducedFun = (hlx_type_fun_mirror_t *)reducedType->objPtr;
                            void *fullType = reducedFun ? reducedFun->parent : NULL;
                            if (!fullType) {
                                hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: '%s' is a bound closure but ->fun->parent is null - refusing to guess", narrow);
                                result = NULL;
                            } else {
                                result = cl->fun;
                                realType = fullType;
                            }
                        }
                    } else if (cl->hasValue != 0) {
                        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: '%s' resolved to a closure with unsupported hasValue=%d - refusing to guess", narrow, cl->hasValue);
                        result = NULL;
                    } else {
                        result = cl->fun;
                    }
                    if (result && outType) *outType = realType;
                    ok = true;
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: class-object reflection fallback faulted - returning null");
        return NULL;
    }
    return result;
}

void *resolve_member_by_name(const void *resolvedType, const unsigned short *memberName, void **outType)
{
    if (!resolvedType || !memberName) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: called with null type/name - returning null");
        return NULL;
    }

    void *addr = NULL;
    MemberLookupResult verdict = LookupMemberOnType(resolvedType, memberName, &addr, outType);
    if (verdict == MEMBER_METHOD) {
        return addr;
    }
    if (verdict == MEMBER_FIELD) {
        char narrow[256];
        hlx_narrow_utf16(memberName, narrow, sizeof(narrow));
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_member_by_name: '%s' resolved to a plain data field, not a callable member - refusing", narrow);
        return NULL;
    }
    return LookupClassMemberViaReflection(resolvedType, memberName, outType);
}

HLX_NATIVE_EXPORT(hlp_hlx_resolve_member, "PBBB_B", resolve_member_by_name)

typedef struct {
    const unsigned short *name;
    void *t;
    int hashed_name;
} hlx_obj_field_mirror_t;

typedef enum {
    STATIC_LOOKUP_FAULT,
    STATIC_LOOKUP_NOT_FOUND,
    STATIC_LOOKUP_NOT_A_METHOD,
    STATIC_LOOKUP_UNBOUND,
    STATIC_LOOKUP_FOUND
} StaticLookupResult;

/* Every static method is a function-typed FIELD on a compiler-generated "$Helper" companion
 * type; the field->function mapping is that type's own `bindings` array - pure module-structural
 * data, available before any live object or static-initializer timing matters. */
/* Mirrors hl_runtime_binding. `fid` is a GLOBAL field index across the super chain, not local.
 * `closure==NULL` means `ptr` is a pre-built unbound vclosure; `closure!=NULL` means `ptr` is a
 * raw function pointer instead - do not misread one as the other. */
typedef struct {
    void *ptr;
    void *closure;
    int fid;
} hlx_runtime_binding_mirror_t;

static StaticLookupResult LookupStaticOnCompanionType(void *companionType, const unsigned short *memberName,
                                                       void **outAddr, void **outType)
{
    StaticLookupResult verdict = STATIC_LOOKUP_FAULT;
    __try {
        hlx_type_mirror_t *t = (hlx_type_mirror_t *)companionType;
        hlx_type_obj_mirror_t *obj = (hlx_type_obj_mirror_t *)t->objPtr;
        hlx_obj_field_mirror_t *fields = (hlx_obj_field_mirror_t *)obj->fields;

        int localIndex = -1;
        for (int i = 0; i < obj->nfields; i++) {
            if (fields[i].name && g_ucmp(fields[i].name, memberName) == 0) {
                localIndex = i;
                break;
            }
        }

        if (localIndex < 0) {
            verdict = STATIC_LOOKUP_NOT_FOUND;
        } else if (!g_hlGetObjProto) {
            verdict = STATIC_LOOKUP_FAULT;
        } else {
            hlx_runtime_obj_mirror_t *rt = (hlx_runtime_obj_mirror_t *)g_hlGetObjProto(companionType);
            hlx_runtime_obj_mirror_t *parentRt = (hlx_runtime_obj_mirror_t *)rt->parent;
            int ancestorFieldCount = parentRt ? parentRt->nfields : 0;
            int globalFid = localIndex + ancestorFieldCount;

            hlx_runtime_binding_mirror_t *bindings = (hlx_runtime_binding_mirror_t *)rt->bindings;
            hlx_runtime_binding_mirror_t *match = NULL;
            for (int j = 0; j < rt->nbindings; j++) {
                if (bindings[j].fid == globalFid) {
                    match = &bindings[j];
                    break;
                }
            }

            if (!match) {
                verdict = STATIC_LOOKUP_UNBOUND;
            } else if (match->closure != NULL) {
                verdict = STATIC_LOOKUP_NOT_A_METHOD;
            } else if (!match->ptr) {
                verdict = STATIC_LOOKUP_UNBOUND;
            } else {
                hlx_vclosure_mirror_t *cl = (hlx_vclosure_mirror_t *)match->ptr;
                *outAddr = cl->fun;
                if (outType) *outType = cl->t;
                verdict = STATIC_LOOKUP_FOUND;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        verdict = STATIC_LOOKUP_FAULT;
    }
    return verdict;
}

// Haxe/HL convention: companion type name = instance name with '$' prefixed onto the last dotted segment ("App" -> "$App", "some.pkg.Foo" -> "some.pkg.$Foo").
static bool BuildCompanionTypeName(const unsigned short *instanceName, unsigned short *out, int outCapacity)
{
    int len = 0;
    while (instanceName[len]) len++;
    if (len + 2 > outCapacity) return false; /* +1 for '$', +1 for the NUL */

    int lastDot = -1;
    for (int i = 0; i < len; i++) {
        if (instanceName[i] == (unsigned short)'.') lastDot = i;
    }

    int pos = 0;
    for (int i = 0; i <= lastDot; i++) out[pos++] = instanceName[i]; /* no-op when lastDot==-1 */
    out[pos++] = (unsigned short)'$';
    for (int i = lastDot + 1; i < len; i++) out[pos++] = instanceName[i];
    out[pos] = 0;
    return true;
}

void *resolve_static_member_by_name(const void *resolvedType, const unsigned short *memberName, void **outType)
{
    if (!resolvedType || !memberName) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: called with null type/name - returning null");
        return NULL;
    }
    if (!g_ucmp) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: ucmp not resolved - returning null");
        return NULL;
    }

    char narrow[256];
    hlx_narrow_utf16(memberName, narrow, sizeof(narrow));

    const unsigned short *instanceName = NULL;
    int instanceKind = -1;
    bool ok = false;
    __try {
        const hlx_type_mirror_t *t = (const hlx_type_mirror_t *)resolvedType;
        instanceKind = t->kind;
        if (t->kind == HOBJ_KIND) {
            hlx_type_obj_mirror_t *obj = (hlx_type_obj_mirror_t *)t->objPtr;
            if (obj) instanceName = obj->name;
        }
        ok = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' - reading resolvedType faulted - returning null", narrow);
        return NULL;
    }
    if (instanceKind != HOBJ_KIND) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' - resolvedType is not an object type (kind %d), has no companion class type - returning null", narrow, instanceKind);
        return NULL;
    }
    if (!instanceName) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' - resolvedType has no name - returning null", narrow);
        return NULL;
    }

    unsigned short companionName[256];
    if (!BuildCompanionTypeName(instanceName, companionName, 256)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' - instance type name too long to derive a companion type name - returning null", narrow);
        return NULL;
    }

    void *companionType = resolve_type_by_name(companionName);
    if (!companionType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' - no companion class type found - returning null", narrow);
        return NULL;
    }

    void *addr = NULL;
    StaticLookupResult verdict = LookupStaticOnCompanionType(companionType, memberName, &addr, outType);
    switch (verdict) {
        case STATIC_LOOKUP_FOUND:
            hlx_log(HLX_LOG_DEBUG, "[hlx-boot] resolve_static_member_by_name: '%s' -> %p", narrow, addr);
            return addr;
        case STATIC_LOOKUP_NOT_FOUND:
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' - no such field on the companion type, not a static member", narrow);
            return NULL;
        case STATIC_LOOKUP_NOT_A_METHOD:
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' resolved to a receiver-bound member, not a plain static method - use resolve_member_by_name for this target instead", narrow);
            return NULL;
        case STATIC_LOOKUP_UNBOUND:
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' is a field on the companion type but has no compiled function bound to it (a plain static var, not a static function) - refusing", narrow);
            return NULL;
        case STATIC_LOOKUP_FAULT:
        default:
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] resolve_static_member_by_name: '%s' companion-type scan faulted - returning null", narrow);
            return NULL;
    }
}

HLX_NATIVE_EXPORT(hlp_hlx_resolve_static_member, "PBBB_B", resolve_static_member_by_name)

/* Fallback for companion types whose global_value header is NULL (some classes with only
 * static data fields never get it wired up) - scans the module's globals table for the one
 * slot typed to this companion, refusing on 0 or >1 matches rather than guessing. */
static void *FindCompanionBoxViaGlobalsScan(const void *companionType)
{
    hlx_code_mirror_t *code = (hlx_code_mirror_t *)module_get_code();
    unsigned char *globalsData = (unsigned char *)module_get_globals_data();
    int *globalsIndexes = module_get_globals_indexes();
    if (!code || !code->globals || !globalsData || !globalsIndexes) return NULL;

    void *result = NULL;
    __try {
        void **globals = code->globals;
        int matchIndex = -1;
        int matchCount = 0;
        for (int i = 0; i < code->nglobals; i++) {
            if (globals[i] == companionType) {
                matchIndex = i;
                matchCount++;
            }
        }
        if (matchCount == 1) {
            result = *(void **)(globalsData + globalsIndexes[matchIndex]);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        result = NULL;
    }
    return result;
}

/* A static DATA field has no function to hand back - only a live value inside the companion
 * type's own global singleton object, which ordinary Reflect.field/setField can then read/write. */
/* Enum types carry their own global_value directly (hl_type_enum, hl.h) - no separate
 * "$EnumName" companion type needs resolving, unlike the HOBJ_KIND path below. */
static void *GetEnumCompanionInstance(const hlx_type_mirror_t *t, const unsigned short **outName)
{
    hlx_type_enum_mirror_t *en = (hlx_type_enum_mirror_t *)t->objPtr;
    if (!en) return NULL;
    if (outName) *outName = en->name;
    if (!en->global_value) return NULL;
    return *en->global_value;
}

void *get_static_companion_instance(const void *resolvedType)
{
    if (!resolvedType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: called with a null type - returning null");
        return NULL;
    }

    const unsigned short *instanceName = NULL;
    int instanceKind = -1;
    bool ok = false;
    bool isEnum = false;
    void *enumResult = NULL;
    __try {
        const hlx_type_mirror_t *t = (const hlx_type_mirror_t *)resolvedType;
        instanceKind = t->kind;
        if (t->kind == HOBJ_KIND) {
            hlx_type_obj_mirror_t *obj = (hlx_type_obj_mirror_t *)t->objPtr;
            if (obj) instanceName = obj->name;
        } else if (t->kind == HENUM_KIND) {
            isEnum = true;
            enumResult = GetEnumCompanionInstance(t, &instanceName);
        }
        ok = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: reading resolvedType faulted - returning null");
        return NULL;
    }
    if (isEnum) {
        char enumNarrow[256] = "?";
        if (instanceName) hlx_narrow_utf16(instanceName, enumNarrow, sizeof(enumNarrow));
        if (!enumResult) {
            hlx_log(HLX_LOG_DEBUG, "[hlx-boot] get_static_companion_instance: enum '%s' - global_value not yet populated (this enum's own constant-construction bytecode, part of the module's real entry point, hasn't run yet) - returning null", enumNarrow);
            return NULL;
        }
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] get_static_companion_instance: enum '%s' -> %p (reflection companion, for Reflect.field(..., \"<ConstructorName>\") to read a specific 0-arg value off)", enumNarrow, enumResult);
        return enumResult;
    }
    if (instanceKind != HOBJ_KIND) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: resolvedType is not an object or enum type (kind %d), has no companion to read - returning null", instanceKind);
        return NULL;
    }
    if (!instanceName) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: resolvedType has no name - returning null");
        return NULL;
    }

    char narrow[256];
    hlx_narrow_utf16(instanceName, narrow, sizeof(narrow));

    unsigned short companionName[256];
    if (!BuildCompanionTypeName(instanceName, companionName, 256)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: '%s' - instance type name too long to derive a companion type name - returning null", narrow);
        return NULL;
    }

    void *companionType = resolve_type_by_name(companionName);
    if (!companionType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: '%s' - no companion class type found - this class has no static members at all, returning null", narrow);
        return NULL;
    }

    void *result = NULL;
    bool ok2 = false;
    bool globalValueSlotMissing = false;
    __try {
        hlx_type_mirror_t *ct = (hlx_type_mirror_t *)companionType;
        if (ct->kind != HOBJ_KIND) {
            ok2 = true;
        } else {
            hlx_type_obj_mirror_t *cobj = (hlx_type_obj_mirror_t *)ct->objPtr;
            if (!cobj || !cobj->global_value) {
                globalValueSlotMissing = true;
                ok2 = true;
            } else {
                result = *cobj->global_value;
                ok2 = true;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok2 = false;
    }
    if (!ok2) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: '%s' - reading companion global_value faulted - returning null", narrow);
        return NULL;
    }
    bool usedGlobalsScanFallback = false;
    if (globalValueSlotMissing) {
        result = FindCompanionBoxViaGlobalsScan(companionType);
        if (!result) {
            hlx_log(HLX_LOG_ERROR, "[hlx-boot] get_static_companion_instance: '%s' - companion type has no "
                    "global_value slot, and no unambiguous singleton of this exact companion type "
                    "was found in the module's own globals table either (0 or >1 candidates) - "
                    "returning null",
                    narrow);
            return NULL;
        }
        usedGlobalsScanFallback = true;
    }
    if (!result) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] get_static_companion_instance: '%s' - companion type found but its "
                "global_value is not yet populated (this class's own static initializer hasn't run "
                "yet - too early to read/write static field VALUES; expected if called before the "
                "game's real entry point runs, e.g. from boot-time install logic rather than "
                "dispatched mod/gamelib-accessor code) - returning null",
                narrow);
        return NULL;
    }

    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] get_static_companion_instance: '%s' -> %p%s", narrow, result, usedGlobalsScanFallback ? " (via globals-table fallback - this companion type has no global_value slot of its own)" : "");
    return result;
}

HLX_NATIVE_EXPORT(hlp_hlx_get_static_companion_instance, "PB_D", get_static_companion_instance)

/* Must mirror hl_function in FULL, not just `type` - this struct is indexed as an array
 * element (functions[pos]), so a truncated mirror scales every nonzero index to the wrong
 * byte offset and silently reads garbage from neighboring elements.
 *
 * regs/ops are dereferenced by the constructor scan below (regs as hl_type* per register,
 * per hl_read_function in hashlink/src/code.c; ops as hlx_opcode_mirror_t*, see that struct) -
 * every other consumer of this mirror only ever reads .type/.findex, hence the untyped void*. */
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

/* Mirrors hl_opcode (hashlink/src/hlmodule.h). `extra`'s meaning depends on `op` - see
 * ExtractCallInfo below, and the Call2 bit-cast trap it documents. */
typedef struct {
    int op;
    int p1;
    int p2;
    int p3;
    int *extra;
} hlx_opcode_mirror_t;

/* Shared tail of call_resolved/construct_instance: builds a synthetic vclosure (hasValue=0,
 * so hl_dyn_call never unwraps a receiver from it - the receiver, if any, is already
 * elements[0]) and invokes it. */
static void *InvokeVClosure(const void *targetFun, const void *realType, void **elements, int length)
{
    if (!g_hlDynCall) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InvokeVClosure: hl_dyn_call not resolved - skipping");
        return NULL;
    }
    hlx_vclosure_mirror_t synth; /* mirrors a real hl_vclosure, whose fields aren't const */
    synth.t = (void *)realType;
    synth.fun = (void *)targetFun;
    synth.hasValue = 0;
    synth.value = NULL;

    void *result = NULL;
    __try {
        result = g_hlDynCall(&synth, elements, length);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] InvokeVClosure: hl_dyn_call faulted");
        result = NULL;
    }
    return result;
}

/* functions_ptrs is indexed directly by findex; functions_indexes is a separate table,
 * needed only to find a function's position in code->functions for its .type - do not
 * conflate the two. */
static bool ResolveFunctionByFindex(int findex, void **outFn, void **outRealType)
{
    void *codePtr = module_get_code();
    void **functionsPtrs = module_get_functions_ptrs();
    int *functionsIndexes = module_get_functions_indexes();
    if (!codePtr || !functionsPtrs || !functionsIndexes) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] ResolveFunctionByFindex: module not fully recovered (code=%p functionsPtrs=%p functionsIndexes=%p) - returning false", codePtr, (void *)functionsPtrs, (void *)functionsIndexes);
        return false;
    }

    bool ok = false;
    __try {
        hlx_code_mirror_t *code = (hlx_code_mirror_t *)codePtr;
        int total = code->nfunctions + code->nnatives;
        if ((unsigned)findex >= (unsigned)total) {
            ok = false;
        } else {
            void *fn = functionsPtrs[findex];
            int pos = functionsIndexes[findex];
            if (!fn || pos < 0 || pos >= code->nfunctions) {
                ok = false;
            } else {
                hlx_function_mirror_t *functions = (hlx_function_mirror_t *)code->functions;
                *outFn = fn;
                *outRealType = functions[pos].type;
                ok = true;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }

    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] ResolveFunctionByFindex(%d): failed - code=%p functionsPtrs=%p functionsIndexes=%p", findex, codePtr, (void *)functionsPtrs, (void *)functionsIndexes);
    }
    return ok;
}

void *alloc_instance(const void *resolvedType)
{
    if (!g_hlAllocObj) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] alloc_instance: hl_alloc_obj not resolved - skipping");
        return NULL;
    }
    if (!resolvedType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] alloc_instance: called with a null type - skipping");
        return NULL;
    }
    void *result = NULL;
    __try {
        result = g_hlAllocObj((void *)resolvedType); /* hl_alloc_obj's own signature isn't const-aware */
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] alloc_instance: hl_alloc_obj faulted (type %p) - skipping", resolvedType);
        return NULL;
    }
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] alloc_instance: allocated %p (type %p)", result, resolvedType);
    return result;
}

HLX_NATIVE_EXPORT(hlp_hlx_alloc_instance, "PB_D", alloc_instance)

#define MAX_DYN_ARGS 16
#define ARRAY_GETDYN_PINDEX 0
#define ARRAYDYN_ARRAY_FIELD_INDEX 0

typedef void *(WINAPI *GetDynFn)(void *thisPtr, int index);

static bool GetRealTypeName(void *obj, const unsigned short **outName)
{
    if (!obj) return false;
    bool ok = false;
    __try {
        hlx_type_mirror_t *t = (hlx_type_mirror_t *)(*(void **)obj);
        if (t->kind == HOBJ_KIND) {
            hlx_type_obj_mirror_t *to = (hlx_type_obj_mirror_t *)t->objPtr;
            if (to && to->name) {
                *outName = to->name;
                ok = true;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    return ok;
}

static bool NameEquals(const unsigned short *name, const char *ascii)
{
    if (!name) return false;
    size_t i = 0;
    for (; ascii[i]; i++) {
        if (name[i] != (unsigned short)(unsigned char)ascii[i]) return false;
    }
    return name[i] == 0;
}

/* args:Array<Dynamic> is often an ArrayObj-wrapping hl.types.ArrayDyn, not a plain ArrayObj. */
static void *ResolveToArrayObj(void *obj)
{
    const unsigned short *name = NULL;
    if (!GetRealTypeName(obj, &name)) return NULL;

    if (NameEquals(name, "hl.types.ArrayObj")) {
        return obj;
    }

    if (NameEquals(name, "hl.types.ArrayDyn")) {
        void *underlying = NULL;
        bool ok = true;
        __try {
            void *t = *(void **)obj;
            hlx_runtime_obj_mirror_t *rt = (hlx_runtime_obj_mirror_t *)g_hlGetObjRt(t);
            if (!rt || rt->nfields < 1) {
                ok = false;
            } else {
                underlying = *(void **)((char *)obj + rt->fields_indexes[ARRAYDYN_ARRAY_FIELD_INDEX]);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ok = false;
        }
        if (!ok || !underlying) return NULL;

        const unsigned short *underlyingName = NULL;
        if (!GetRealTypeName(underlying, &underlyingName)) return NULL;
        if (NameEquals(underlyingName, "hl.types.ArrayObj")) return underlying;
        return NULL;
    }

    return NULL;
}

static bool ResolveDynArgs(void *argsArray, void **outElements, int maxElements, int *outLength)
{
    if (!g_hlGetObjRt || !g_hlGetObjProto) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_resolved: required exports not resolved - skipping");
        return false;
    }
    if (!argsArray) {
        *outLength = 0;
        return true;
    }
    void *lengthSource = ResolveToArrayObj(argsArray);
    if (!lengthSource) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_resolved: args is neither a plain hl.types.ArrayObj nor an ArrayObj-wrapping hl.types.ArrayDyn - unsupported, skipping");
        return false;
    }

    int length = 0;
    bool ok = true;
    __try {
        void *lt = *(void **)lengthSource;
        hlx_runtime_obj_mirror_t *rt = (hlx_runtime_obj_mirror_t *)g_hlGetObjRt(lt);
        if (!rt || rt->nfields < 1) {
            ok = false;
        } else {
            length = *(int *)((char *)lengthSource + rt->fields_indexes[0]);
            if (length < 0 || length > maxElements) {
                hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_resolved: args.length=%d out of supported range (0..%d) - skipping", length, maxElements);
                ok = false;
            } else {
                void *ot = *(void **)argsArray;
                g_hlGetObjProto(ot);
                hlx_type_mirror_t *tm = (hlx_type_mirror_t *)ot;
                GetDynFn getDyn = (GetDynFn)tm->vobj_proto[ARRAY_GETDYN_PINDEX];
                for (int i = 0; i < length; i++) {
                    outElements[i] = getDyn(argsArray, i);
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ok = false;
    }
    if (!ok) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_resolved: reading the args array faulted or was invalid - skipping");
        return false;
    }
    *outLength = length;
    return true;
}

void *call_closure(void *closure, void **args, int nargs)
{
    if (!g_hlDynCall || !closure) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_closure: hl_dyn_call not resolved or null closure - skipping");
        return NULL;
    }
    void *result = NULL;
    __try {
        result = g_hlDynCall(closure, args, nargs);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_closure: hl_dyn_call faulted");
        result = NULL;
    }
    return result;
}

void *call_resolved(const void *targetFun, const void *realType, void *argsArray)
{
    if (!targetFun || !realType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] call_resolved: called with a null target address/type - skipping");
        return NULL;
    }

    void *elements[MAX_DYN_ARGS];
    int length = 0;
    if (!ResolveDynArgs(argsArray, elements, MAX_DYN_ARGS, &length)) {
        return NULL;
    }

    return InvokeVClosure(targetFun, realType, elements, length);
}

HLX_NATIVE_EXPORT(hlp_hlx_call_resolved, "PBBD_D", call_resolved)

/* ctorFindex is baked in at generation time, not resolved by name here: HL's `New` opcode
 * is bare allocation with no constructor reference, so a findex is the only stable identity
 * - and unlike a name, it can shift on any recompile of the game. */
void *construct_instance(const void *resolvedType, int ctorFindex, void *argsArray)
{
    if (!resolvedType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance: called with a null type - skipping");
        return NULL;
    }

    void *instance = alloc_instance(resolvedType);
    if (!instance) {
        return NULL; /* alloc_instance already logged why */
    }

    void *ctorFn = NULL;
    void *ctorRealType = NULL;
    if (!ResolveFunctionByFindex(ctorFindex, &ctorFn, &ctorRealType)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance: findex %d did not resolve to a real JIT'd function - the allocated instance is returned UNCONSTRUCTED (fields left at their zero-initialized default), not null, since hl_alloc_obj itself already succeeded", ctorFindex);
        return instance;
    }

    int ctorRealKind = -1;
    __try {
        ctorRealKind = ctorRealType ? *(int *)ctorRealType : -2;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        ctorRealKind = -3; /* reading realType's own kind faulted */
    }
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] construct_instance: resolved findex %d -> fn=%p realType=%p (kind=%d, expect HFUN_KIND=%d) instance=%p", ctorFindex, ctorFn, ctorRealType, ctorRealKind, HFUN_KIND, instance);

    /* elements[0] is the receiver; declared args unpack starting at elements[1]. */
    void *elements[MAX_DYN_ARGS];
    int ctorArgsLength = 0;
    if (!ResolveDynArgs(argsArray, elements + 1, MAX_DYN_ARGS - 1, &ctorArgsLength)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance: failed to unpack constructor args - the allocated instance is returned UNCONSTRUCTED, not null");
        return instance;
    }
    elements[0] = instance;

    InvokeVClosure(ctorFn, ctorRealType, elements, ctorArgsLength + 1); /* return value (always void) discarded */
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] construct_instance: constructed %p via findex %d", instance, ctorFindex);
    return instance;
}

HLX_NATIVE_EXPORT(hlp_hlx_construct_instance, "PBiD_D", construct_instance)

/* ===================================================================================
 * Live constructor resolution (construct_instance_by_name)
 *
 * HL's `New` opcode is bare allocation with no constructor reference at all - unlike
 * hl_type_obj's proto table (named methods), there is no bytecode-format-level table
 * linking a type to "its constructor". The only signal is a `New dst` followed shortly
 * by a Call-family instruction whose first argument is that same `dst` register.
 *
 * Rather than trust a findex baked in at gamelib-generation time (HLX.GamelibGenerator's
 * ConstructorCollector.cs, doing this exact same scan offline against the SAME bytecode),
 * this scans the whole module ONCE, here, natively, eagerly at module-load time (see
 * reflection_scan_constructors, called from boot.c right after module_recover succeeds).
 * A findex baked in at generation time can silently point at the wrong function after the
 * game recompiles (findex numbering isn't stable across recompiles - the entire premise of
 * every OTHER by-name resolution path in this file); a live scan of the CURRENTLY running
 * module can't have that problem; and since this table is naturally whole-module, it lives
 * here in reflection.c (one shared native DLL across every loaded mod), not per-mod Haxe
 * statics like HlxRuntime.hx's typeCache/memberCache.
 *
 * Ambiguity is resolved by filtering a type's raw candidates by declared arg count
 * (receiver + params) - see FilterCandidatesByArgCount. If 0 or >1 candidates remain after
 * filtering, construct_instance_by_name fails closed (loud hlx_log(HLX_LOG_ERROR, ...), then
 * returns the allocated-but-uninitialized instance, exactly mirroring construct_instance's
 * own existing behavior above when ResolveFunctionByFindex fails) rather than falling back to
 * any generation-time-baked findex: identical bytecode run through this identical heuristic
 * at generation time would have hit the same ambiguity, so ConstructorCollector would never
 * have emitted a baked value for this class to fall back to in the first place; and in the
 * cross-build case where an old baked value WOULD exist (game recompiled since the gamelib was
 * generated), reusing it is actively dangerous, not merely stale - it could point at a
 * completely unrelated function in the new build. Empirically, against Farever's real
 * hlboot.dat, ConstructorCollector reports zero ambiguous classes out of 2391 resolved
 * (TotalCandidateSitesFound=12913, ClassesResolved=2391, ClassesAmbiguous=0) - the arg-count
 * filter here is a hedge against a coincidental collision in some FUTURE recompile, not a
 * fix for a currently observed problem.
 * =================================================================================== */

/* HL opcode integer values - re-derived independently here (this file has no dependency on
 * the C# model types) from hashlink/src/opcodes.h's OP_BEGIN/OP_END list; cross-checked
 * against this repo's own mirror at HLX.Core/Model/HlOpcode.cs, which agrees. */
enum {
    HLX_OP_NEW = 82,
    HLX_OP_CALL0 = 24,
    HLX_OP_CALL1 = 25,
    HLX_OP_CALL2 = 26,
    HLX_OP_CALL3 = 27,
    HLX_OP_CALL4 = 28,
    HLX_OP_CALLN = 29,
};

/* One class type's raw, deduplicated candidate findex(es) - "raw" because these are NOT
 * yet filtered by declared arg count; that filtering happens per-call in
 * construct_instance_by_name, once the caller's expected arg count is known. */
typedef struct {
    void *typePtr;   /* hl_type* identity (same pointer instance as a register's declared type) - NULL means empty slot */
    int *findexes;
    int count;
    int capacity;
} hlx_ctor_bucket_t;

static hlx_ctor_bucket_t *g_ctorBuckets;
static int g_ctorTableSize; /* power-of-two slot count; 0 until reflection_scan_constructors runs */
static bool g_ctorTableBuilt;

static int NextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* Open-addressing, linear probing; typePtr==NULL marks an empty slot (never itself a valid
 * hl_type* candidate). createIfMissing==false is used for read-only queries from
 * construct_instance_by_name and must never mutate the table. */
static hlx_ctor_bucket_t *FindBucket(void *typePtr, bool createIfMissing)
{
    if (!g_ctorBuckets || g_ctorTableSize <= 0 || !typePtr) return NULL;
    unsigned int mask = (unsigned int)(g_ctorTableSize - 1);
    unsigned int h = (unsigned int)(((uintptr_t)typePtr) >> 4) & mask;
    for (int probe = 0; probe < g_ctorTableSize; probe++) {
        unsigned int idx = (h + probe) & mask;
        hlx_ctor_bucket_t *b = &g_ctorBuckets[idx];
        if (b->typePtr == typePtr) return b;
        if (b->typePtr == NULL) {
            if (!createIfMissing) return NULL;
            b->typePtr = typePtr;
            return b;
        }
    }
    /* Table full - sized with headroom in reflection_scan_constructors, should not happen. */
    return NULL;
}

static void AddCandidate(void *typePtr, int findex)
{
    hlx_ctor_bucket_t *b = FindBucket(typePtr, true);
    if (!b) return;
    for (int i = 0; i < b->count; i++)
        if (b->findexes[i] == findex) return; /* dedup - mirrors ConstructorCollector.cs's HashSet<int> */
    if (b->count >= b->capacity) {
        int newCap = b->capacity == 0 ? 4 : b->capacity * 2;
        int *grown = (int *)realloc(b->findexes, sizeof(int) * newCap);
        if (!grown) return; /* OOM - silently drop this one candidate rather than crash the scan */
        b->findexes = grown;
        b->capacity = newCap;
    }
    b->findexes[b->count++] = findex;
}

/* Mirrors ConstructorCollector.cs's ExtractCallInfo exactly. p3 is the first argument's
 * register for every Call1..Call4 (see hashlink/src/jit.c's OCall1/OCall2/OCall3/OCall4
 * handling: args[0] is always o->p3) - CallN alone carries no args inline, so its first
 * arg (if any) is extra[0]. Call2's own first-arg register is p3 too; its use of `extra`
 * for arg2 (not arg1) is the bit-cast trap handled separately, in the arg-COUNT-agnostic
 * candidate resolution below - this function never touches Call2's `extra`. */
static bool ExtractCallInfo(const hlx_opcode_mirror_t *o, int *outFindex, int *outFirstArgReg)
{
    switch (o->op) {
        case HLX_OP_CALL0:
            *outFindex = o->p2;
            *outFirstArgReg = -1;
            return true;
        case HLX_OP_CALL1:
        case HLX_OP_CALL2:
        case HLX_OP_CALL3:
        case HLX_OP_CALL4:
            *outFindex = o->p2;
            *outFirstArgReg = o->p3;
            return true;
        case HLX_OP_CALLN:
            *outFindex = o->p2;
            *outFirstArgReg = (o->p3 > 0 && o->extra) ? o->extra[0] : -1;
            return true;
        default:
            return false;
    }
}

/* Mirrors ConstructorCollector.cs's WritesRegister exactly: opcodes whose p1 is NOT a
 * destination register (a condition/jump target/...), or that write no register at all;
 * every other opcode's dst register IS p1 (hashlink/src/jit.c: `vreg *dst = R(o->p1);` at
 * the top of its main per-opcode switch). Numeric values per HLX.Core/Model/HlOpcode.cs. */
static bool WritesRegister(int op, int p1, int reg)
{
    switch (op) {
        case 39: /* SetField */
        case 41: /* SetThis */
        case 37: /* SetGlobal */
        case 43: /* DynSet */
        case 44: case 45: case 46: case 47: /* JTrue, JFalse, JNull, JNotNull */
        case 48: case 49: case 50: case 51: /* JSLt, JSGte, JSGt, JSLte */
        case 52: case 53: case 54: case 55: /* JULt, JUGte, JNotLt, JNotGte */
        case 56: case 57: case 58: /* JEq, JNotEq, JAlways */
        case 66: /* Label */
        case 67: /* Ret */
        case 68: /* Throw */
        case 69: /* Rethrow */
        case 70: /* Switch */
        case 71: /* NullCheck */
        case 72: /* Trap */
        case 73: /* EndTrap */
        case 78: case 79: case 80: case 81: /* SetI8, SetI16, SetMem, SetArray */
        case 94: /* SetEnumField */
        case 95: /* Assert */
        case 98: /* Nop */
        case 99: /* Prefetch */
        case 100: /* Asm */
        case 101: /* Catch */
            return false;
        default:
            return p1 == reg;
    }
}

static int g_totalCandidateSites; /* diagnostic parity counter with ConstructorCollector.cs's TotalCandidateSitesFound */

/* Mirrors ConstructorCollector.cs's FindPairedConstructorCall: callInfo is checked BEFORE
 * the clobber check each iteration, so a call's own same-numbered void-return dst isn't
 * mistaken for a clobber before its args are even inspected. */
static void ScanFunctionForConstructors(const hlx_function_mirror_t *fn)
{
    if (!fn->ops || !fn->regs || fn->nops <= 0 || fn->nregs <= 0) return;
    const hlx_opcode_mirror_t *ops = (const hlx_opcode_mirror_t *)fn->ops;
    void **regs = (void **)fn->regs; /* hl_type* per register - hl_read_function, hashlink/src/code.c */

    for (int i = 0; i < fn->nops; i++) {
        if (ops[i].op != HLX_OP_NEW) continue;
        int dstReg = ops[i].p1;
        if ((unsigned)dstReg >= (unsigned)fn->nregs) continue;
        void *regType = regs[dstReg];
        if (!regType) continue;
        hlx_type_mirror_t *t = (hlx_type_mirror_t *)regType;
        if (t->kind != HOBJ_KIND) continue;

        for (int j = i + 1; j < fn->nops; j++) {
            int findex = -1, firstArgReg = -1;
            if (ExtractCallInfo(&ops[j], &findex, &firstArgReg) && firstArgReg == dstReg) {
                g_totalCandidateSites++;
                AddCandidate(regType, findex);
                break;
            }
            if (WritesRegister(ops[j].op, ops[j].p1, dstReg)) break;
        }
    }
}

void reflection_scan_constructors(void)
{
    if (g_ctorTableBuilt) {
        hlx_log(HLX_LOG_DEBUG, "[hlx-boot] reflection_scan_constructors: already built - skipping");
        return;
    }
    void *codePtr = module_get_code();
    if (!codePtr) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] reflection_scan_constructors: no live hl_code recovered this run - "
                "construct_instance_by_name will fail closed for every type until the game is restarted");
        return;
    }

    DWORD startTick = GetTickCount();
    hlx_code_mirror_t *code = (hlx_code_mirror_t *)codePtr;

    g_ctorTableSize = NextPow2(code->ntypes > 0 ? code->ntypes * 2 + 16 : 1024);
    g_ctorBuckets = (hlx_ctor_bucket_t *)calloc((size_t)g_ctorTableSize, sizeof(hlx_ctor_bucket_t));
    if (!g_ctorBuckets) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] reflection_scan_constructors: allocation of a %d-slot table failed - "
                "construct_instance_by_name will fail closed for every type", g_ctorTableSize);
        g_ctorTableSize = 0;
        return;
    }

    g_totalCandidateSites = 0;
    int functionsScanned = 0, functionsFaulted = 0;
    hlx_function_mirror_t *functions = (hlx_function_mirror_t *)code->functions;
    for (int i = 0; i < code->nfunctions; i++) {
        bool ok = true;
        __try {
            ScanFunctionForConstructors(&functions[i]);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ok = false;
        }
        if (ok) functionsScanned++; else functionsFaulted++;
    }

    int distinctTypesWithCandidates = 0, unambiguousByRawFindex = 0, stillAmbiguousByRawFindex = 0;
    for (int i = 0; i < g_ctorTableSize; i++) {
        if (!g_ctorBuckets[i].typePtr) continue;
        distinctTypesWithCandidates++;
        if (g_ctorBuckets[i].count == 1) unambiguousByRawFindex++; else stillAmbiguousByRawFindex++;
    }

    g_ctorTableBuilt = true;
    hlx_log(HLX_LOG_INFO,
            "[hlx-boot] reflection_scan_constructors: done in %lums - %d/%d function(s) scanned (%d faulted), "
            "%d New+Call candidate site(s) found, %d distinct type(s) with a candidate "
            "(%d unambiguous by raw findex alone, %d still ambiguous pending per-call arg-count "
            "filtering in construct_instance_by_name)",
            (unsigned long)(GetTickCount() - startTick), functionsScanned, code->nfunctions, functionsFaulted,
            g_totalCandidateSites, distinctTypesWithCandidates, unambiguousByRawFindex, stillAmbiguousByRawFindex);
}

/* Haxe classes have at most one constructor: any raw candidate whose real declared arg
 * count doesn't match expectedNargs must be a false-positive New+Call pattern-match, not a
 * real overload. expectedNargs is the receiver plus the constructor's declared param count
 * (GameConstructor.Params, dropSelf:true - see ClassCollector.cs/HxEmitter.cs), i.e. exactly
 * what ResolveFunctionByFindex's realType (an HFUN, via hlx_type_fun_mirror_t.nargs) would
 * report for the real constructor. Returns the number of surviving matches; *outFn/*outRealType
 * are only meaningful when exactly 1 is returned (the first match is written eagerly so a
 * caller checking for ==1 doesn't need a second pass, but a caller must still check the count). */
static int FilterCandidatesByArgCount(const int *candidates, int candidateCount, int expectedNargs, void **outFn,
                                       void **outRealType)
{
    int matches = 0;
    for (int i = 0; i < candidateCount; i++) {
        void *fn = NULL;
        void *realType = NULL;
        if (!ResolveFunctionByFindex(candidates[i], &fn, &realType)) continue;

        bool ok = false;
        int nargs = -1;
        __try {
            hlx_type_mirror_t *rt = (hlx_type_mirror_t *)realType;
            if (rt && rt->kind == HFUN_KIND) {
                hlx_type_fun_mirror_t *funType = (hlx_type_fun_mirror_t *)rt->objPtr;
                if (funType) nargs = funType->nargs;
            }
            ok = true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ok = false;
        }
        if (!ok || nargs != expectedNargs) continue;

        if (matches == 0) {
            *outFn = fn;
            *outRealType = realType;
        }
        matches++;
    }
    return matches;
}

/* Live-by-name sibling of construct_instance above: no findex is baked in anywhere -
 * expectedArgCount (the constructor's declared param count, receiver excluded, same as
 * HxEmitter's emitted `new(a0:T0, ...)`) is used only to disambiguate this type's raw
 * New+Call candidates, never to pick a function directly. On 0 or >1 surviving candidates
 * this fails closed exactly like construct_instance does on a bad findex: loud
 * hlx_log(HLX_LOG_ERROR, ...), then return the allocated-but-uninitialized instance rather
 * than null, since hl_alloc_obj itself already succeeded. See the design note above this
 * section for why there is no fallback to any generation-time-baked findex here. */
void *construct_instance_by_name(const void *resolvedType, int expectedArgCount, void *argsArray)
{
    if (!resolvedType) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance_by_name: called with a null type - skipping");
        return NULL;
    }

    void *instance = alloc_instance(resolvedType);
    if (!instance) {
        return NULL; /* alloc_instance already logged why */
    }

    if (!g_ctorTableBuilt) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance_by_name: constructor table was never built (module "
                "recovery must not have completed this run) - the allocated instance is returned UNCONSTRUCTED "
                "(fields left at their zero-initialized default), not null, since hl_alloc_obj itself already succeeded");
        return instance;
    }

    hlx_ctor_bucket_t *bucket = NULL;
    bool lookupOk = false;
    __try {
        bucket = FindBucket((void *)resolvedType, false);
        lookupOk = true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        lookupOk = false;
    }
    if (!lookupOk || !bucket || bucket->count == 0) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance_by_name: no New+Call constructor candidate was found "
                "anywhere in the module for type %p - the allocated instance is returned UNCONSTRUCTED, not null",
                resolvedType);
        return instance;
    }

    int expectedNargs = expectedArgCount + 1; /* +1 for the implicit receiver */
    void *ctorFn = NULL;
    void *ctorRealType = NULL;
    int matches = FilterCandidatesByArgCount(bucket->findexes, bucket->count, expectedNargs, &ctorFn, &ctorRealType);
    if (matches != 1) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance_by_name: type %p has %d raw New+Call candidate(s) but "
                "%d matched the expected arg count %d (receiver + %d declared param(s)) - refusing to guess, the "
                "allocated instance is returned UNCONSTRUCTED, not null",
                resolvedType, bucket->count, matches, expectedNargs, expectedArgCount);
        return instance;
    }

    /* elements[0] is the receiver; declared args unpack starting at elements[1] - mirrors construct_instance. */
    void *elements[MAX_DYN_ARGS];
    int ctorArgsLength = 0;
    if (!ResolveDynArgs(argsArray, elements + 1, MAX_DYN_ARGS - 1, &ctorArgsLength)) {
        hlx_log(HLX_LOG_ERROR, "[hlx-boot] construct_instance_by_name: failed to unpack constructor args - the "
                "allocated instance is returned UNCONSTRUCTED, not null");
        return instance;
    }
    elements[0] = instance;

    InvokeVClosure(ctorFn, ctorRealType, elements, ctorArgsLength + 1); /* return value (always void) discarded */
    hlx_log(HLX_LOG_DEBUG, "[hlx-boot] construct_instance_by_name: constructed %p (type %p) via live-resolved findex",
            instance, resolvedType);
    return instance;
}

HLX_NATIVE_EXPORT(hlp_hlx_construct_instance_by_name, "PBiD_D", construct_instance_by_name)
