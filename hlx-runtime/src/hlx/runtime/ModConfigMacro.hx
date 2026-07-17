package hlx.runtime;

#if macro
import haxe.macro.Context;
import haxe.macro.Expr;
import haxe.macro.TypeTools;

// @:build macro for `@:hlx.config static var config:Config = {...};`.
//
// Unlike @:hlx.prefix/@:hlx.postfix (HookDiscoveryMacro), which run via Context.onAfterTyping
// and can only ADD new types via defineType, this needs to REWRITE the annotated field's own
// type and initializer - onAfterTyping runs after a field is already typed and immutable, so
// only a @:build macro (which operates on the raw pre-typed field AST) can do that. That is
// why a mod using @:hlx.config needs a @:build macro on its class, rather than being purely
// field-metadata-driven like @:hlx.prefix/@:hlx.postfix.
//
// Called directly only by hlx.runtime.Mod.build(), the standard `@:build(...)` a mod's main
// class should use - kept as its own class since the field-rewrite logic here is unrelated to
// Mod.build()'s other job (HookDiscoveryMacro.use()).
class ModConfigMacro {
    static var definedWrappers = new Map<String, Bool>();

    public static function build():Array<Field> {
        var localClass = Context.getLocalClass().get();
        var fields = Context.getBuildFields();
        var out:Array<Field> = [];

        for (field in fields) {
            var configMeta = field.meta == null ? [] : field.meta.filter(m -> m.name == ":hlx.config");
            if (configMeta.length == 0) {
                out.push(field);
                continue;
            }
            if (configMeta.length > 1) {
                Context.error('@:hlx.config may only be applied once to "${field.name}"', field.pos);
                out.push(field);
                continue;
            }

            var entry = configMeta[0];
            if (entry.params != null && entry.params.length > 0) {
                Context.error('@:hlx.config takes no arguments - the mod name is resolved at runtime via HlxRuntime.moduleName()', entry.pos);
                out.push(field);
                continue;
            }

            if (field.access == null || field.access.indexOf(AStatic) < 0) {
                Context.error('@:hlx.config "${field.name}" must be a static field', field.pos);
                out.push(field);
                continue;
            }

            switch (field.kind) {
                case FVar(configType, defaultExpr):
                    if (configType == null) {
                        Context.error('@:hlx.config "${field.name}" must declare an explicit type, e.g. static var ${field.name}:Config = {...}', field.pos);
                        out.push(field);
                        continue;
                    }
                    if (defaultExpr == null) {
                        Context.error('@:hlx.config "${field.name}" must have a default value expression - it is used as the fallback when no saved config exists', field.pos);
                        out.push(field);
                        continue;
                    }

                    out.push(rewriteField(localClass.name, field, configType, defaultExpr, null));
                case FProp(get, set, configType, defaultExpr):
                    // Property form, e.g. `static var config(default, null):Config = {...};`
                    // - the standard Haxe idiom for "publicly readable, only this class
                    // can reassign the whole reference" - a caller elsewhere still
                    // mutates it in place (`config.someField = x; config.save();`),
                    // it just can't do `SomeClass.config = anotherConfig`. `get`/`set`
                    // are passed through as-is; Haxe's own property typing validates
                    // the accessor pair, same as any hand-written property.
                    if (configType == null) {
                        Context.error('@:hlx.config "${field.name}" must declare an explicit type, e.g. static var ${field.name}($get, $set):Config = {...}', field.pos);
                        out.push(field);
                        continue;
                    }
                    if (defaultExpr == null) {
                        Context.error('@:hlx.config "${field.name}" must have a default value expression - it is used as the fallback when no saved config exists', field.pos);
                        out.push(field);
                        continue;
                    }

                    out.push(rewriteField(localClass.name, field, configType, defaultExpr, {get: get, set: set}));
                default:
                    Context.error('@:hlx.config only supports "static var ${field.name}:Type = defaultValue" or "static var ${field.name}(get, set):Type = defaultValue" fields', field.pos);
                    out.push(field);
            }
        }

        return out;
    }

    static function rewriteField(className:String, field:Field, configType:ComplexType, defaultExpr:Expr, ?accessors:{get:String, set:String}):Field {
        // Resolved+re-portabilized (TypeTools.toComplexType, same trick HookDiscoveryMacro's own
        // safeComplexType uses) before it's spliced into a brand-new, unrelated module below -
        // the raw source-level ComplexType (e.g. a bare "Config") is only valid against THIS
        // field's own file's imports, and would fail to resolve inside a freshly-defined module
        // that has no relation to that file's import list.
        var portableConfigType = TypeTools.toComplexType(Context.resolveType(configType, field.pos));

        var wrapperName = "HlxModConfig_" + className + "_" + field.name;
        defineWrapper(wrapperName, portableConfigType, field.pos);
        var wrapperType:ComplexType = TPath({ pack: ["hlx", "runtime"], name: wrapperName, params: [] });

        // (defaultExpr : configType) forces T=configType unambiguously in the generic
        // ModConfig.load<T> call below, rather than leaving T to be inferred bottom-up from an
        // untyped structure literal - the raw (unresolved) configType is fine here since this
        // expression stays in the field's own original file/module, not a new one.
        var typedDefault:Expr = { expr: ECheckType(defaultExpr, configType), pos: defaultExpr.pos };
        var loadExpr:Expr = macro hlx.runtime.ModConfig.load(HlxRuntime.moduleName(), $e{typedDefault});

        var kind = accessors == null
            ? FVar(wrapperType, loadExpr)
            : FProp(accessors.get, accessors.set, wrapperType, loadExpr);

        return {
            name: field.name,
            doc: field.doc,
            access: field.access,
            kind: kind,
            pos: field.pos,
            meta: field.meta.filter(m -> m.name != ":hlx.config"),
        };
    }

    // One synthesized @:forward abstract per annotated field: @:forward gives transparent
    // field read/write access to the underlying Config value (the same technique the gamelib
    // generator already uses for wrapping native objects with @:forward-chained abstracts),
    // plus an inline save() that resolves the mod name at call time via HlxRuntime.moduleName() -
    // reliable from anywhere, including a prefix/postfix hook firing well after boot.
    static function defineWrapper(name:String, configType:ComplexType, pos:Position):Void {
        if (definedWrappers.exists(name)) return;
        definedWrappers.set(name, true);

        Context.defineType({
            pos: pos,
            pack: ["hlx", "runtime"],
            name: name,
            kind: TDAbstract(configType, [configType], [configType]),
            meta: [{ name: ":forward", params: [], pos: pos }],
            fields: [{
                name: "save",
                pos: pos,
                access: [APublic, AInline],
                kind: FFun({
                    args: [],
                    ret: macro :Void,
                    expr: macro hlx.runtime.ModConfig.save(HlxRuntime.moduleName(), this),
                }),
            }],
        });
    }
}
#end
