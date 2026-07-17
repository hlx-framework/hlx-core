package hlx.runtime;

#if macro
import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.Expr.MetadataEntry;
import haxe.macro.Type;
import haxe.macro.TypeTools;
import haxe.macro.ComplexTypeTools;

// Compile-time discovery for @:hlx.prefix/@:hlx.postfix; `use()` runs once from a mod's .hxml and scans the whole compilation after typing.
// Uses onAfterTyping, not onGenerate: defineType calls from onGenerate are silently dropped from the output.
// Metadata params are raw untyped Expr, so a target naming a nonexistent-here type still compiles; the name is never validated, just used at runtime.
class HookDiscoveryMacro {
    static var used = false;
    static var generated = false;

    // Forces inclusion of the mod's own srcPaths (default ["src"]) - annotated classes are otherwise never referenced, so onAfterTyping would never see them.
    public static function use(?srcPaths:Array<String>):Void {
        if (used) return;
        used = true;

        haxe.macro.Compiler.include("", true, null, srcPaths == null ? ["src"] : srcPaths);

        Context.onAfterTyping(onAfterTyping);
    }

    static function onAfterTyping(moduleTypes:Array<haxe.macro.Type.ModuleType>):Void {
        if (generated) return; // guards against the recursive re-invocation defineType below triggers

        var classTypes:Array<ClassType> = [];
        for (t in moduleTypes) {
            switch (t) {
                case TClassDecl(ref):
                    var c = ref.get();
                    if (c.isExtern) continue;
                    classTypes.push(c);
                default:
            }
        }

        var targets = new Map<String, TargetInfo>();
        var haveAny = false;

        for (c in classTypes) {
            for (field in c.statics.get()) {
                var prefixMeta = field.meta.extract(":hlx.prefix");
                var postfixMeta = field.meta.extract(":hlx.postfix");
                if (prefixMeta.length == 0 && postfixMeta.length == 0) continue;

                switch (field.kind) {
                    case FMethod(_):
                    default:
                        Context.error('@:hlx.prefix/@:hlx.postfix only supports static methods - "${fqName(c)}.${field.name}" is not one', field.pos);
                        continue;
                }

                for (entry in prefixMeta) addContributor(targets, c, field, entry, true);
                for (entry in postfixMeta) addContributor(targets, c, field, entry, false);
                haveAny = true;
            }
        }

        if (!haveAny) return;

        var pos = Context.currentPos();
        var genFields:Array<Field> = [];
        var initExprs:Array<Expr> = [];

        for (key in targets.keys()) {
            buildTargetCode(targets.get(key), pos, genFields, initExprs);
        }

        if (genFields.length == 0) return;

        genFields.push({
            name: "__init__",
            access: [AStatic],
            // @:privateAccess: annotated methods are private by default, but the generated field access here lives in a different module (HlxGenerated).
            kind: FFun({
                args: [],
                ret: tpath("Void"),
                expr: macro @:privateAccess $b{initExprs},
            }),
            pos: pos,
        });

        generated = true;
        Context.defineType({
            pos: pos,
            pack: ["hlx", "runtime"],
            name: "HlxGenerated",
            meta: [{ name: ":keep", params: [], pos: pos }],
            kind: TDClass(),
            fields: genFields,
        });
    }

    static function fqName(c:ClassType):String {
        return c.pack.length == 0 ? c.name : c.pack.join(".") + "." + c.name;
    }

    static function kindLabel(isPrefix:Bool):String {
        return isPrefix ? "prefix" : "postfix";
    }

    static function addContributor(targets:Map<String, TargetInfo>, modClass:ClassType, field:ClassField, entry:MetadataEntry,
            isPrefix:Bool):Void {
        var kindName = kindLabel(isPrefix);
        var params = entry.params == null ? [] : entry.params;
        if (params.length != 1) {
            Context.error('@:hlx.$kindName expects exactly one argument, e.g. @:hlx.$kindName(hxd.App.update)', entry.pos);
            return;
        }

        var identity = extractIdentity(params[0], entry.pos);
        if (identity == null) return; // extractIdentity already raised Context.error

        var key = identity.typeName + "." + identity.methodName;
        var info = targets.get(key);
        if (info == null) {
            info = { typeName: identity.typeName, methodName: identity.methodName, contributors: [] };
            targets.set(key, info);
        }
        info.contributors.push({ isPrefix: isPrefix, modClass: modClass, field: field, metaPos: entry.pos });
    }

    // The dotted chain before the trailing method name is used verbatim as the type name, never resolved against a real compiled type.
    static function extractIdentity(e:Expr, pos:Position):Null<{typeName:String, methodName:String}> {
        switch (e.expr) {
            case EField(sub, methodName):
                var idents = flattenIdent(sub);
                if (idents == null) {
                    Context.error("@:hlx.prefix/@:hlx.postfix expects a direct Type.method reference, e.g. @:hlx.prefix(hxd.App.update)", pos);
                    return null;
                }
                return { typeName: idents.join("."), methodName: methodName };
            default:
                Context.error("@:hlx.prefix/@:hlx.postfix expects a direct Type.method reference, e.g. @:hlx.prefix(hxd.App.update)", pos);
                return null;
        }
    }

    static function flattenIdent(e:Expr):Null<Array<String>> {
        return switch (e.expr) {
            case EConst(CIdent(s)): [s];
            case EField(sub, f):
                var rest = flattenIdent(sub);
                if (rest == null) null; else { rest.push(f); rest; }
            default: null;
        }
    }

    static function tpath(name:String):ComplexType {
        return TPath({ pack: [], name: name, params: [] });
    }

    // Not cached in a static var: __init__ runs before its own class's static var initializers, so a cached key would still be null there.
    static function newKeyExpr(info:TargetInfo):Expr {
        return macro new hlx.runtime.PatchTargetKey($v{info.typeName}, $v{info.methodName});
    }

    static function safeComplexType(t:Type):ComplexType {
        var ct = TypeTools.toComplexType(t);
        return ct == null ? tpath("Dynamic") : ct;
    }

    // A resolved Void ({StdTypes, sub:"Void"}) and hand-built tpath("Void") name the same type but aren't string-equal; hence the explicit check.
    static function complexTypeIsVoid(ct:ComplexType):Bool {
        return switch (ct) {
            case TPath(p): (p.name == "Void" && p.sub == null) || (p.name == "StdTypes" && p.sub == "Void");
            default: false;
        }
    }

    // Target signature is derived from what the annotated function declares (trusted, not verified against the real gamelib method).
    // Prefix return type is unwrapped from HlxPrefixControl/HlxPrefixResult<T>; postfix drops the trailing `result` param and keeps its own return type.
    static function contributorSignature(c:Contributor):Null<ContributorSignature> {
        var args:Array<{name:String, opt:Bool, t:Type}>;
        var retType:Type;
        switch (c.field.type) {
            case TFun(a, r):
                args = a;
                retType = r;
            default:
                Context.error('@:hlx.${kindLabel(c.isPrefix)} target "${c.field.name}" is not a function', c.metaPos);
                return null;
        }

        if (c.isPrefix) {
            var params = [for (a in args) { name: a.name, type: safeComplexType(a.t) }];
            return switch (retType) {
                case TEnum(ref, enumParams):
                    var e = ref.get();
                    if (e.name == "HlxPrefixControl") { params: params, ret: tpath("Void"), retIsVoid: true };
                    else if (e.name == "HlxPrefixResult" && enumParams.length == 1) {
                        var ret = safeComplexType(enumParams[0]);
                        { params: params, ret: ret, retIsVoid: complexTypeIsVoid(ret) };
                    } else {
                        Context.error('@:hlx.prefix "${c.field.name}" must return HlxPrefixControl or HlxPrefixResult<T>, found "${e.name}"', c.metaPos);
                        null;
                    }
                default:
                    Context.error('@:hlx.prefix "${c.field.name}" must return HlxPrefixControl or HlxPrefixResult<T>', c.metaPos);
                    null;
            };
        } else {
            if (args.length == 0) {
                Context.error('@:hlx.postfix "${c.field.name}" must declare a trailing result parameter', c.metaPos);
                return null;
            }
            var withoutResult = args.slice(0, args.length - 1);
            var params = [for (a in withoutResult) { name: a.name, type: safeComplexType(a.t) }];
            var ret = safeComplexType(retType);
            return { params: params, ret: ret, retIsVoid: complexTypeIsVoid(ret) };
        }
    }

    static function signaturesMatch(a:ContributorSignature, b:ContributorSignature):Bool {
        if (a.params.length != b.params.length) return false;
        for (i in 0...a.params.length) {
            if (ComplexTypeTools.toString(a.params[i].type) != ComplexTypeTools.toString(b.params[i].type)) return false;
        }
        if (a.retIsVoid || b.retIsVoid) return a.retIsVoid == b.retIsVoid;
        return ComplexTypeTools.toString(a.ret) == ComplexTypeTools.toString(b.ret);
    }

    static function buildTargetCode(info:TargetInfo, pos:Position, genFields:Array<Field>, initExprs:Array<Expr>):Void {
        var canonical:Null<ContributorSignature> = null;
        var validContributors:Array<Contributor> = [];

        for (c in info.contributors) {
            var sig = contributorSignature(c);
            if (sig == null) continue; // contributorSignature already raised Context.error

            if (canonical == null) {
                canonical = sig;
            } else if (!signaturesMatch(canonical, sig)) {
                Context.error('@:hlx.${kindLabel(c.isPrefix)} "${c.field.name}" disagrees with the physical '
                    + 'signature already established for ${info.typeName}.${info.methodName} by an earlier prefix/postfix '
                    + 'targeting the same method - all prefixes/postfixes for one target must agree on parameter and '
                    + 'result types', c.metaPos);
                continue;
            }
            validContributors.push(c);
        }

        if (canonical == null || validContributors.length == 0) return;

        var safe = (info.typeName + "_" + info.methodName).split(".").join("_");
        var receiverName = "__hlxReceiver_" + safe;

        var argExprs = [for (p in canonical.params) (macro $i{p.name})];
        var dispatchCall:Expr = macro HlxRuntime.dispatch($e{newKeyExpr(info)}, [$a{argExprs}]);
        var bodyExpr:Expr = canonical.retIsVoid ? dispatchCall : macro return $e{dispatchCall};

        genFields.push({
            name: receiverName,
            access: [AStatic],
            kind: FFun({
                args: [for (p in canonical.params) { name: p.name, type: p.type }],
                ret: canonical.ret,
                expr: macro $b{[bodyExpr]},
            }),
            pos: pos,
        });

        for (c in validContributors) {
            var fnExpr:Expr = macro $p{c.modClass.pack.concat([c.modClass.name, c.field.name])};
            initExprs.push(c.isPrefix
                ? macro HlxRuntime.registerPrefix($e{newKeyExpr(info)}, $e{fnExpr}, $i{receiverName})
                : macro HlxRuntime.registerPostfix($e{newKeyExpr(info)}, $e{fnExpr}, $i{receiverName}));
        }
    }
}

private typedef Contributor = {
    isPrefix: Bool,
    modClass: ClassType,
    field: ClassField,
    metaPos: Position,
};

private typedef TargetInfo = {
    typeName: String,
    methodName: String,
    contributors: Array<Contributor>,
};

private typedef ContributorSignature = {
    params: Array<{name:String, type:ComplexType}>,
    ret: ComplexType,
    retIsVoid: Bool,
};
#end
