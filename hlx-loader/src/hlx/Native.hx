package hlx;

// Thin @:hlNative forwards into hlx-boot's native ABI. Native bodies are dummies, replaced at link time.
@:access(String)
@:access(Sys)
class Native {
    @:hlNative("std", "hlx_loader_ready")
    public static function loaderReady(registerPrefix:Dynamic, registerPostfix:Dynamic, dispatch:Dynamic):Void {}

    @:hlNative("std", "hlx_mods_loaded")
    public static function modsLoaded():Bool {
        return false;
    }

    @:hlNative("std", "hlx_resolve_type")
    static function hlxResolveType(typeName:hl.Bytes):hl.Bytes {
        return null;
    }

    public static inline function resolveType(typeName:String):hl.Bytes {
        return hlxResolveType(typeName.bytes);
    }

    // outType must stay plain hl.Bytes: hl.Ref<hl.Bytes> mismatches the native's hardcoded "PBBB_B" signature and hl_fatal4s at module load.
    @:hlNative("std", "hlx_resolve_member")
    static function hlxResolveMember(resolvedType:hl.Bytes, memberName:hl.Bytes, outType:hl.Bytes):hl.Bytes {
        return null;
    }

    // outTypeBuf is 8 bytes: pointer-sized, 64-bit only (this project only builds libhl64.dll).
    public static function resolveMember(resolvedType:hl.Bytes, memberName:String):{address:hl.Bytes, type:hl.Bytes} {
        var outTypeBuf = new hl.Bytes(8);
        var address = hlxResolveMember(resolvedType, memberName.bytes, outTypeBuf);
        if (address == null) return null;
        var realType = hl.Bytes.fromAddress(haxe.Int64.make(outTypeBuf.getI32(4), outTypeBuf.getI32(0)));
        return { address: address, type: realType };
    }

    // Try resolveMember first, this as fallback only - never reverse: hlx_resolve_static_member rejects members needing a receiver.
    @:hlNative("std", "hlx_resolve_static_member")
    static function hlxResolveStaticMember(resolvedType:hl.Bytes, memberName:hl.Bytes, outType:hl.Bytes):hl.Bytes {
        return null;
    }

    public static function resolveStaticMember(resolvedType:hl.Bytes, memberName:String):{address:hl.Bytes, type:hl.Bytes} {
        var outTypeBuf = new hl.Bytes(8);
        var address = hlxResolveStaticMember(resolvedType, memberName.bytes, outTypeBuf);
        if (address == null) return null;
        var realType = hl.Bytes.fromAddress(haxe.Int64.make(outTypeBuf.getI32(4), outTypeBuf.getI32(0)));
        return { address: address, type: realType };
    }

    @:hlNative("std", "hlx_install_patch")
    public static function installPatch(realAddress:hl.Bytes, realType:hl.Bytes, receiverFn:Dynamic):Int {
        return -1;
    }

    @:hlNative("std", "hlx_call_original")
    public static function callOriginal(handle:Int, argsArray:Dynamic):Dynamic {
        return null;
    }

    @:hlNative("std", "sys_load_plugin")
    static function sysLoadPlugin(file:hl.Bytes):Bool {
        return false;
    }

    // Goes through Sys.getPath, not raw .bytes - hl_sys_load_plugin needs that path-encoding conversion.
    public static inline function loadPlugin(file:String):Bool {
        return sysLoadPlugin(Sys.getPath(file));
    }
}
