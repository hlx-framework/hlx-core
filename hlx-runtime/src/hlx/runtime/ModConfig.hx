package hlx.runtime;

// Non-instantiable static-only namespace: `new ModConfig()` has no valid construction
// path, unlike a plain class (which gets an implicit public constructor unless one is
// explicitly hidden). `inline` still gives every call site the same zero-overhead
// splicing a plain static function would, so nothing is traded away for that guarantee.
abstract ModConfig(Void) {
    // Falls back to defaultValue on a missing OR malformed config.json - a hand-edited/
    // corrupt file must not crash the mod it's diagnosing, same convention hlx-boot's own
    // config.c already follows for loader.conf. Uses trace()/Sys.println rather than a new
    // native log bridge: those already land in hlx.log tagged with this mod's own name via
    // the existing sys_print trampoline, so no new native surface is needed for visibility.
    public static inline function load<T>(modName:String, defaultValue:T):T {
        if (modName == null) return defaultValue;
        var path = configPath(modName);
        if (!sys.FileSystem.exists(path)) return defaultValue;
        return try {
            haxe.Json.parse(sys.io.File.getContent(path));
        } catch (e:Dynamic) {
            Sys.println('[ModConfig] "$modName": config.json is malformed (${Std.string(e)}) - falling back to the default value');
            defaultValue;
        }
    }

    public static inline function save<T>(modName:String, value:T):Void {
        if (modName == null) return;
        var dir = configDir(modName);
        if (!sys.FileSystem.exists(dir)) sys.FileSystem.createDirectory(dir);
        sys.io.File.saveContent(configPath(modName), haxe.Json.stringify(value, null, "  "));
    }

    static inline function configDir(modName:String):String
        return "hlx/config/" + modName;

    static inline function configPath(modName:String):String
        return configDir(modName) + "/config.json";
}
