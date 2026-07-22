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

/* Eager, whole-file New+Call bytecode scan building the name-keyed type->candidate-findex(es)
 * table that construct_instance_by_name queries. Parses hlboot.dat directly off disk (via a
 * vendored hl_code_read, see hlx-boot/vendor/hashlink/) rather than reading the live process's
 * module - so, unlike the table this superseded, it has NO dependency on module_recover()
 * having completed. Call once, as early as convenient (right after reflection_resolve_setup
 * succeeds - see boot.c); safe to call more than once, later calls are a no-op. */
void reflection_init_constructor_table(void);

#endif /* HLX_REFLECTION_H */
