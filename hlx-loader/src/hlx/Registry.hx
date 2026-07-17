package hlx;

abstract PatchTargetKey(String) {
    public inline function new(typeName:String, methodName:String) {
        this = typeName + "." + methodName;
    }

    public inline function toString():String {
        return this;
    }

    // Split on the LAST '.' - type names are dotted (e.g. "h2d.Object"), method names never are.
    public var typeName(get, never):String;
    inline function get_typeName():String
        return this.substring(0, this.lastIndexOf("."));

    public var methodName(get, never):String;
    inline function get_methodName():String
        return this.substring(this.lastIndexOf(".") + 1);
}

typedef PatchTarget = {
    prefixes: Array<Dynamic>,
    postfixes: Array<Dynamic>,
    receiver: Dynamic,
    trampolineHandle: Int
};

class Registry {
    public static var registry = new Map<PatchTargetKey, PatchTarget>();

    public static function registerPrefix(key:PatchTargetKey, fn:Dynamic, receiver:Dynamic):Void {
        var target = registry.get(key);
        if (target == null) {
            target = { prefixes: [], postfixes: [], receiver: receiver, trampolineHandle: -1 };
            registry.set(key, target);
        }
        target.prefixes.push(fn);
    }

    public static function registerPostfix(key:PatchTargetKey, fn:Dynamic, receiver:Dynamic):Void {
        var target = registry.get(key);
        if (target == null) {
            target = { prefixes: [], postfixes: [], receiver: receiver, trampolineHandle: -1 };
            registry.set(key, target);
        }
        target.postfixes.push(fn);
    }

    // Must run after every mod has registered and hlx_mods_loaded has completed; each target resolves/installs independently, so one bad target is skipped, not fatal.
    public static function installAllPendingPatches():Void {
        for (key => target in registry) {
            var resolvedType = Native.resolveType(key.typeName);
            if (resolvedType == null) {
                trace('install: type not found: ${key.typeName}');
                continue;
            }

            // A PatchTargetKey never says static vs instance, so try instance resolution first and fall back to static.
            var resolved = Native.resolveMember(resolvedType, key.methodName);
            if (resolved == null) resolved = Native.resolveStaticMember(resolvedType, key.methodName);
            if (resolved == null) {
                trace('install: member not found: ${key}');
                continue;
            }

            var handle = Native.installPatch(resolved.address, resolved.type, target.receiver);
            if (handle < 0) {
                trace('install: install_patch failed for ${key}');
                continue;
            }

            target.trampolineHandle = handle;
        }
    }
}
