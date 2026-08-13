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

    // Enum constructors have no companion-class static method to resolve - hl_type_enum
    // carries no function pointers at all (unlike hl_type_obj's methods table), only layout
    // data (per-construct size/offsets/param types). resolveStaticMember/callResolved is the
    // wrong tool for these; the native side allocates the value directly via hl_alloc_enum +
    // hl_write_dyn, the same primitives std.Type.createEnum itself is built on.
    @:hlNative("std", "hlx_construct_enum")
    static function hlxConstructEnum(resolvedType:hl.Bytes, constructorName:hl.Bytes, argsArray:Dynamic):Dynamic {
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

    // Raw vdynamic.v.ptr extraction, no abs_name/hl_same_type check at all - see
    // reflection.h's unbox_dynamic_ptr for the full rationale. Needed by HookDiscoveryMacro's
    // generated receivers: hl_dyn_call (used by callOriginal to invoke a hooked function's
    // real body) always boxes a non-void, non-Dynamic return into a fresh vdynamic, but a
    // receiver whose own declared return type is Dynamic (forced whenever its prefix/postfix
    // contributors return Dynamic to route around the same abs_name check on their own end -
    // see bugs 6/7 in shader-cache's REPORT.md for the concrete case this was found from)
    // would otherwise hand that box's address back to the real caller instead of the real
    // pointer. The result of this call is only ever meaningfully `cast` to a concrete
    // hl.Abstract<...> type at the call site - never used as Dynamic itself.
    @:hlNative("std", "hlx_unbox_ptr")
    public static function unboxPointer(d:Dynamic):hl.Bytes {
        return null;
    }

    // Counterpart to unboxPointer above, going the other way - see reflection.c's own comment
    // on box_dynamic_ptr for the full native-side rationale.
    @:hlNative("std", "hlx_box_ptr")
    static function hlxBoxPtr(ptr:hl.Bytes, target:hl.Type):Dynamic {
        return null;
    }

    // Makes a Dynamic value carrying a native hl.Abstract<"..."> (e.g. from resolveField/
    // resolveMember off a gamelib instance) usable as that concrete abstract type, even though
    // it was produced by a DIFFERENT compiled module than the caller's. A plain `cast` fails
    // here: HashLink's hl_same_type compares abstracts by abs_name POINTER identity, and two
    // independently-compiled modules never share that pointer even for the exact same name
    // (see hlx-toolkit's GamelibGenerator design notes). This instead compares the name by
    // STRING CONTENT via hl.Type (the same approach HL's own native-signature loader already
    // uses for exactly this problem, just applied here to the reflection path instead of the
    // native-load path), then re-boxes the raw pointer under the CALLER's own hl_type - so the
    // caller's own (unmodified) cast machinery sees a pointer-identical type and succeeds.
    //
    // Must stay `inline`, not a plain generic method: a normal generic function's type parameter
    // erases to Dynamic in compiled HL bytecode, which would make hl.Type.get(sample) inspect a
    // boxed runtime value instead of the real static type T. `inline` splices this body into
    // each concrete call site before that erasure happens, so hl.Type.get(sample) still resolves
    // correctly there - confirmed empirically; the non-inline version silently breaks this.
    public static inline function resolveAbstract<T>(d:Dynamic, sample:T):T {
        if (d == null)
            return sample;
        var srcName = hl.Type.getDynamic(d).getTypeName();
        var dstType = hl.Type.get(sample);
        if (srcName != dstType.getTypeName())
            throw 'HlxRuntime.resolveAbstract: expected $srcName to match ${dstType.getTypeName()}';
        return cast hlxBoxPtr(unboxPointer(d), dstType);
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

    // Live by-name equivalent of `EnumName.Ctor(args)` for a cross-module enum type - see
    // hlxConstructEnum's own comment for why this isn't resolveStaticMember/callResolved.
    public static inline function constructEnum(resolvedType:hl.Bytes, constructorName:String, args:Array<Dynamic>):Dynamic {
        return hlxConstructEnum(resolvedType, constructorName.bytes, args);
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
