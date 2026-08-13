package hlx.runtime;

// Generic bucket/key/value store - Phase 1 "Core Registry" of
// plans/CORE-SHARED-BUS-REGISTRY.md. Thin @:hlNative forwards into hlx-boot's "hlx_kv_*"
// natives (registry.c) - each mod compiles its own copy of this class, but the underlying
// store lives once per process, in hlx-boot, the same way HlxRuntime's own natives do.
//
// The Registry has NO knowledge of what a bucket means or what shape its values are - it
// just stores whatever value a producer registers, keyed by (bucket, key). Re-registering
// the same (bucket, key) replaces the existing entry rather than duplicating it. This is
// retained state only - no publish/subscribe/observation semantics (that's the separate,
// later Message Bus).
//
// Consumer pitfall: register()'s value is Dynamic, so a nested array literal inside it (e.g.
// a "mods" entry's commands:[...]) gets NO Array<Dynamic> expected-type context at its point
// of construction - Haxe infers a narrower Array<T>, which is always hl.types.ArrayObj across
// a module boundary, never hl.types.ArrayDyn. A consumer in a DIFFERENT mod that reads it back
// as `var x:Array<Dynamic> = Reflect.field(...)` then hits "Can't cast hl.types.ArrayObj to
// hl.types.ArrayDyn" - real bug, confirmed via two genuinely separate .hl modules
// (single-module testing hides it, since Haxe's own type unification retroactively infers
// Array<Dynamic> once it sees a same-compile use site expecting it). Read a nested array out
// of a registered value as `hl.types.ArrayBase` (`.length`/`.getDyn(i)`) instead of
// `Array<Dynamic>` - the real common ancestor for an ArrayObj value, same reason the
// auto-generated farever-gamelib bindings read cross-module arrays that way too.
@:access(String)
class Registry {
    @:hlNative("std", "hlx_kv_register")
    static function hlxRegister(bucket:hl.Bytes, key:hl.Bytes, value:Dynamic):Void {}

    @:hlNative("std", "hlx_kv_unregister")
    static function hlxUnregister(bucket:hl.Bytes, key:hl.Bytes):Void {}

    @:hlNative("std", "hlx_kv_get")
    static function hlxGet(bucket:hl.Bytes, key:hl.Bytes):Dynamic {
        return null;
    }

    @:hlNative("std", "hlx_kv_count")
    static function hlxCount(bucket:hl.Bytes):Int {
        return 0;
    }

    @:hlNative("std", "hlx_kv_value_at")
    static function hlxValueAt(bucket:hl.Bytes, index:Int):Dynamic {
        return null;
    }

    public static inline function register(bucket:String, key:String, value:Dynamic):Void {
        hlxRegister(bucket.bytes, key.bytes, value);
    }

    public static inline function unregister(bucket:String, key:String):Void {
        hlxUnregister(bucket.bytes, key.bytes);
    }

    public static inline function get(bucket:String, key:String):Dynamic {
        return hlxGet(bucket.bytes, key.bytes);
    }

    // Returns this bucket's currently registered values (not keys) - e.g. Registry.list("mods")
    // for a mod-config UI to discover every registered {id, name, version} entry.
    public static function list(bucket:String):Array<Dynamic> {
        var b = bucket.bytes;
        var count = hlxCount(b);
        var out = [];
        for (i in 0...count) out.push(hlxValueAt(b, i));
        return out;
    }
}
