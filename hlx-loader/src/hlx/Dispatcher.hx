package hlx;

import hlx.Registry;

class Dispatcher {
    public static function dispatch(key:PatchTargetKey, args:Array<Dynamic>):Dynamic {
        var target = Registry.registry.get(key);
        if (target == null) return null;

        var skip = false;
        var result:Dynamic = null;

        // HlxPrefixControl and HlxPrefixResult<T> share constructor indices 0/1, so enumIndex handles both uniformly.
        for (prefix in target.prefixes) {
            // A mod's prefix/postfix is foreign code - never let it abort dispatch.
            var prefixRes:Dynamic = try Reflect.callMethod(null, prefix, args) catch (e:Dynamic) {
                trace('prefix threw for $key: $e');
                null;
            }
            if (prefixRes == null) continue; // defensive: a buggy mod forgot to return a control value
            switch (Type.enumIndex(prefixRes)) {
                case 0: // Continue
                case 1: skip = true; // Skip
                case 2: skip = true; result = Type.enumParameters(prefixRes)[0]; // SkipWith
            }
            if (skip) break;
        }

        if (!skip) {
            result = Native.callOriginal(target.trampolineHandle, args);
        }

        for (postfix in target.postfixes) {
            result = try Reflect.callMethod(null, postfix, args.concat([result])) catch (e:Dynamic) {
                trace('postfix threw for $key: $e');
                result;
            }
        }

        return result;
    }
}
