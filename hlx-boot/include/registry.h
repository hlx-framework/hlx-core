#ifndef HLX_REGISTRY_H
#define HLX_REGISTRY_H

/* Generic bucket/key/value store - Phase 1 "Core Registry" of
 * plans/CORE-SHARED-BUS-REGISTRY.md: lets isolated mods (each its own HL module, sharing no
 * Haxe statics) discover/read state retained here, in the one native component loaded once
 * per process. The Registry has no knowledge of what a bucket means or what shape its values
 * are - it only stores whatever a producer registers, keyed by (bucket, key).
 *
 * Natives are exported as "hlx_kv_*" (see registry.c), NOT "hlx_registry_*" - that name is
 * already taken by boot.c's hlx_registry_register_prefix/postfix, the UNRELATED prefix/postfix
 * patch-hook table forwarded to hlx-loader.hl. Two different features both wanted the word
 * "registry"; the hook table claimed it first, so this one uses "kv" (key/value) instead to
 * avoid confusing the two at the native-symbol level. */

void registry_register(const unsigned short *bucket, const unsigned short *key, void *value);
void registry_unregister(const unsigned short *bucket, const unsigned short *key);
void *registry_get(const unsigned short *bucket, const unsigned short *key);
int registry_count(const unsigned short *bucket);
void *registry_value_at(const unsigned short *bucket, int index);

#endif /* HLX_REGISTRY_H */
