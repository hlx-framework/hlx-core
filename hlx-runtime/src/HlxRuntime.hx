package;

import hlx.runtime.PatchTargetKey;
import hlx.runtime.ResolvedMember;

// Thin @:hlNative forwards into hlx-boot's mod-facing ABI; each mod compiles its own copy - only the patch registry (in hlx-loader) is shared.
@:access(String)
class HlxRuntime {
    @:hlNative("std", "hlx_resolve_type")
    static function hlxResolveType(typeName:hl.Bytes):hl.Bytes {
        return null;
    }

    @:hlNative("std", "hlx_resolve_member")
    static function hlxResolveMember(resolvedType:hl.Bytes, memberName:hl.Bytes, outType:hl.Bytes):hl.Bytes {
        return null;
    }

    @:hlNative("std", "hlx_resolve_static_member")
    static function hlxResolveStaticMember(resolvedType:hl.Bytes, memberName:hl.Bytes, outType:hl.Bytes):hl.Bytes {
        return null;
    }

    @:hlNative("std", "hlx_get_static_companion_instance")
    static function hlxGetStaticCompanionInstance(resolvedType:hl.Bytes):Dynamic {
        return null;
    }

    @:hlNative("std", "hlx_alloc_instance")
    static function hlxAllocInstance(resolvedType:hl.Bytes):Dynamic {
        return null;
    }

    @:hlNative("std", "hlx_call_resolved")
    static function hlxCallResolved(targetFun:hl.Bytes, realType:hl.Bytes, argsArray:Dynamic):Dynamic {
        return null;
    }

    // No by-name resolution for constructors; ctorFindex is a bytecode-recovered constant baked in at generation time (see HxEmitter.EmitConstructorFactory).
    @:hlNative("std", "hlx_construct_instance")
    static function hlxConstructInstance(resolvedType:hl.Bytes, ctorFindex:Int, argsArray:Dynamic):Dynamic {
        return null;
    }

    // Live by-name constructor resolution: unlike hlxConstructInstance above, no findex is
    // baked in here. The native side scans the whole module once, eagerly, at load time
    // for the same New+Call bytecode pattern ConstructorCollector.cs recovers offline
    // (HL's `New` opcode is bare allocation with no constructor reference of its own), then
    // disambiguates a type's candidate findex(es) by matching expectedArgCount (receiver +
    // declared params) against each candidate's real declared arity. See HxEmitter.EmitConstructorFactory.
    @:hlNative("std", "hlx_construct_instance_by_name")
    static function hlxConstructInstanceByName(resolvedType:hl.Bytes, expectedArgCount:Int, argsArray:Dynamic):Dynamic {
        return null;
    }

    @:hlNative("std", "hlx_install_patch")
    static function hlxInstallPatch(realAddress:hl.Bytes, realType:hl.Bytes, receiverFn:Dynamic):Int {
        return -1;
    }

    @:hlNative("std", "hlx_call_original")
    static function hlxCallOriginal(handle:Int, argsArray:Dynamic):Dynamic {
        return null;
    }

    @:hlNative("std", "hlx_registry_register_prefix")
    static function hlxRegisterPrefix(key:Dynamic, fn:Dynamic, receiver:Dynamic):Void {}

    @:hlNative("std", "hlx_registry_register_postfix")
    static function hlxRegisterPostfix(key:Dynamic, fn:Dynamic, receiver:Dynamic):Void {}

    @:hlNative("std", "hlx_dispatch")
    static function hlxDispatch(key:Dynamic, argsArray:Dynamic):Dynamic {
        return null;
    }

    @:hlNative("std", "hlx_module_name")
    static function hlxModuleName():hl.Bytes {
        return null;
    }

    static var typeCache = new Map<String, hl.Bytes>();
    static var memberCache = new Map<String, ResolvedMember>();

    public static function resolveType(typeName:String):hl.Bytes {
        var cached = typeCache.get(typeName);
        if (cached != null) return cached;
        var resolved = hlxResolveType(typeName.bytes);
        if (resolved != null) typeCache.set(typeName, resolved);
        return resolved;
    }

    public static function resolveMember(resolvedType:hl.Bytes, memberName:String):ResolvedMember {
        var outTypeBuf = new hl.Bytes(8);
        var address = hlxResolveMember(resolvedType, memberName.bytes, outTypeBuf);
        if (address == null) return null;
        var realType = hl.Bytes.fromAddress(haxe.Int64.make(outTypeBuf.getI32(4), outTypeBuf.getI32(0)));
        return { address: address, type: realType };
    }

    public static function resolveStaticMember(resolvedType:hl.Bytes, memberName:String):ResolvedMember {
        var outTypeBuf = new hl.Bytes(8);
        var address = hlxResolveStaticMember(resolvedType, memberName.bytes, outTypeBuf);
        if (address == null) return null;
        var realType = hl.Bytes.fromAddress(haxe.Int64.make(outTypeBuf.getI32(4), outTypeBuf.getI32(0)));
        return { address: address, type: realType };
    }

    public static function resolveMemberOf(cls:Class<Dynamic>, memberName:String):ResolvedMember {
        var typeName = Type.getClassName(cls);
        var cacheKey = '$typeName#$memberName';
        var cached = memberCache.get(cacheKey);
        if (cached != null) return cached;
        var resolvedType = resolveType(typeName);
        if (resolvedType == null) return null;
        var member = resolveMember(resolvedType, memberName);
        if (member == null) member = resolveStaticMember(resolvedType, memberName);
        if (member != null) memberCache.set(cacheKey, member);
        return member;
    }

    public static inline function allocInstance(resolvedType:hl.Bytes):Dynamic {
        return hlxAllocInstance(resolvedType);
    }

    public static inline function callResolved(member:ResolvedMember, args:Array<Dynamic>):Dynamic {
        return member == null ? null : hlxCallResolved(member.address, member.type, args);
    }

    // Offline equivalent of `new ClassName(args)`, for classes not sharing this module's class identity. Not cached: ctorFindex is already a compile-time constant.
    public static inline function constructInstance(resolvedType:hl.Bytes, ctorFindex:Int, args:Array<Dynamic>):Dynamic {
        return hlxConstructInstance(resolvedType, ctorFindex, args);
    }

    // Offline equivalent of `new ClassName(args)`, resolved live by name - no baked findex.
    // Not cached here: the native side owns its own process-wide type->constructor
    // candidate table (shared across every loaded mod), built once at module load, not a
    // per-mod Haxe static like typeCache/memberCache above.
    public static inline function constructInstanceByName(resolvedType:hl.Bytes, expectedArgCount:Int, args:Array<Dynamic>):Dynamic {
        return hlxConstructInstanceByName(resolvedType, expectedArgCount, args);
    }

    public static inline function resolveField(obj:Dynamic, fieldName:String):Dynamic {
        return Reflect.field(obj, fieldName);
    }

    public static inline function setField(obj:Dynamic, fieldName:String, value:Dynamic):Void {
        Reflect.setField(obj, fieldName, value);
    }

    // Also resolves 0-arg enum constructor values by name via the enum's reflection companion.
    public static inline function resolveStaticField(resolvedType:hl.Bytes, fieldName:String):Dynamic {
        var companion = hlxGetStaticCompanionInstance(resolvedType);
        return companion == null ? null : Reflect.field(companion, fieldName);
    }

    public static inline function setStaticField(resolvedType:hl.Bytes, fieldName:String, value:Dynamic):Void {
        var companion = hlxGetStaticCompanionInstance(resolvedType);
        if (companion != null) Reflect.setField(companion, fieldName, value);
    }

    public static inline function installPatch(realAddress:hl.Bytes, realType:hl.Bytes, receiverFn:Dynamic):Int {
        return hlxInstallPatch(realAddress, realType, receiverFn);
    }

    public static inline function callOriginal(handle:Int, args:Array<Dynamic>):Dynamic {
        return hlxCallOriginal(handle, args);
    }

    public static inline function registerPrefix(key:PatchTargetKey, fn:Dynamic, receiver:Dynamic):Void {
        hlxRegisterPrefix(key, fn, receiver);
    }

    public static inline function registerPostfix(key:PatchTargetKey, fn:Dynamic, receiver:Dynamic):Void {
        hlxRegisterPostfix(key, fn, receiver);
    }

    public static inline function dispatch(key:PatchTargetKey, args:Array<Dynamic>):Dynamic {
        return hlxDispatch(key, args);
    }

    // UTF16, NUL-terminated - widen from hlx_common.c's hlx_widen_ascii. Null if the
    // trampoline's VirtualAlloc failed at resolution time (extremely unlikely, never crashes).
    public static function moduleName():String {
        var b = hlxModuleName();
        if (b == null) return null;
        var len = 0;
        while (b.getUI16(len << 1) != 0) len++;
        return String.__alloc__(b, len);
    }
}
