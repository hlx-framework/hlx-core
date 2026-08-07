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

/* Extracts the raw pointer payload out of a boxed Dynamic value (vdynamic.v.ptr) with NO
 * abs_name/hl_same_type check at all - the counterpart, on the return-value side, to
 * shadercache.cpp's own `state->v.ptr` trick for argument Dynamic values (see that file's
 * comment on library_store_graphics). Needed because hl_dyn_call (used by call_resolved
 * above to invoke a hooked function's real body) always boxes a non-void, non-Dynamic return
 * value into a fresh vdynamic for its own generic void* result - so a HookDiscoveryMacro-
 * generated receiver whose own declared return type is Dynamic (because its prefix/postfix
 * contributors must use Dynamic to route around the SAME abs_name check on THEIR side, see
 * hlx-runtime's HookDiscoveryMacro.hx) would otherwise hand the boxed wrapper's address back
 * to the real caller instead of the real pointer, silently corrupting anything expecting a
 * concrete hl.Abstract<...>-typed return (e.g. h3d.impl.DX12Driver.makePipeline's dx_resource). */
void *unbox_dynamic_ptr(void *dynValue);

/* Wraps a raw pointer as a Dynamic tagged with an arbitrary, caller-supplied hl_type - see
 * reflection.c's own comment on box_dynamic_ptr for the full rationale (counterpart to
 * unbox_dynamic_ptr above; the piece hl.Type itself can't reach, since hl_alloc_dynamic is
 * HL_API-only). Used by hlx-runtime's HlxRuntime.resolveAbstract to make a cross-module
 * Dynamic->hl.Abstract<"name"> cast succeed by matching abstract NAME instead of abs_name
 * pointer identity. */
void *box_dynamic_ptr(void *ptr, void *targetType);

/* Eager, whole-file New+Call bytecode scan building the name-keyed type->candidate-findex(es)
 * table that construct_instance_by_name queries. Parses hlboot.dat directly off disk (via a
 * vendored hl_code_read, see hlx-boot/vendor/hashlink/) rather than reading the live process's
 * module - so, unlike the table this superseded, it has NO dependency on module_recover()
 * having completed. Call once, as early as convenient (right after reflection_resolve_setup
 * succeeds - see boot.c); safe to call more than once, later calls are a no-op. */
void reflection_init_constructor_table(void);

#endif /* HLX_REFLECTION_H */
