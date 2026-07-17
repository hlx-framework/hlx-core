package hlx;

class Boot {
    // trace() is auto-tagged per module: hlx-boot intercepts sys_print natively.
    static function main() {
        trace("Loader bootstrapping...");

        // Must be plain, unbound static function refs - the native side caches their arity/type.
        Native.loaderReady(Registry.registerPrefix, Registry.registerPostfix, Dispatcher.dispatch);

        var modsDir = "hlx/mods";
        var names = try sys.FileSystem.readDirectory(modsDir) catch (e:Dynamic) {
            trace('could not read $modsDir: $e');
            [];
        }
        names = names.filter(n -> sys.FileSystem.isDirectory(modsDir + "/" + n));
        // Sort explicitly - directory read order isn't guaranteed, and load order = dispatch order.
        names.sort((a, b) -> a < b ? -1 : (a > b ? 1 : 0));

        for (name in names) {
            var path = modsDir + "/" + name + "/" + name + ".hl";
            if (!sys.FileSystem.exists(path)) {
                trace('skipping $name: no $path');
                continue;
            }
            var ok = try Native.loadPlugin(path) catch (e:Dynamic) {
                trace('mod $path threw during load: $e');
                false;
            }
            trace('loadPlugin($path) -> $ok');
        }

        // Only valid once every mod's main() has run.
        var recovered = Native.modsLoaded();
        if (!recovered) {
            trace("module recovery failed - skipping patch installation");
            return;
        }

        Registry.installAllPendingPatches();
        trace("boot complete");
    }
}
