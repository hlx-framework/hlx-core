#ifndef HLX_REFLECTION_H
#define HLX_REFLECTION_H

void *resolve_type_by_name(const unsigned short *typeName);
void *resolve_member_by_name(const void *resolvedType, const unsigned short *memberName, void **outType);
void *resolve_static_member_by_name(const void *resolvedType, const unsigned short *memberName, void **outType);
void *get_static_companion_instance(const void *resolvedType);
void *alloc_instance(const void *resolvedType);
void *call_resolved(const void *targetFun, const void *realType, void *argsArray);
void *construct_instance(const void *resolvedType, int ctorFindex, void *argsArray);
void *call_closure(void *closure, void **args, int nargs);
void reflection_resolve_setup(void *realLibhlModule);

#endif /* HLX_REFLECTION_H */
