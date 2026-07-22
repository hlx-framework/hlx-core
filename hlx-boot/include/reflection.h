#ifndef HLX_REFLECTION_H
#define HLX_REFLECTION_H

void *resolve_type_by_name(const unsigned short *typeName);
void *resolve_member_by_name(const void *resolvedType, const unsigned short *memberName, void **outType);
void *resolve_static_member_by_name(const void *resolvedType, const unsigned short *memberName, void **outType);
void *get_static_companion_instance(const void *resolvedType);
void *alloc_instance(const void *resolvedType);
void *call_resolved(const void *targetFun, const void *realType, void *argsArray);
void *construct_instance(const void *resolvedType, int ctorFindex, void *argsArray);
void *construct_instance_by_name(const void *resolvedType, int expectedArgCount, void *argsArray);
void *call_closure(void *closure, void **args, int nargs);
void reflection_resolve_setup(void *realLibhlModule);

/* Eager, whole-module New+Call bytecode scan building the type->candidate-findex(es)
 * table that construct_instance_by_name queries. Call once, right after module_recover()
 * succeeds (see boot.c's hlx_mods_loaded_impl) - safe to call more than once, later calls
 * are a no-op. */
void reflection_scan_constructors(void);

#endif /* HLX_REFLECTION_H */
