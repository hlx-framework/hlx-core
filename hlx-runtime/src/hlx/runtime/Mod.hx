package hlx.runtime;

#if macro
import haxe.macro.Expr;

// Standard @:build entry point for a mod's main class - put `@:build(hlx.runtime.Mod.build())`
// on the same class that has `static function main()` and nothing else is needed at the
// compile.hxml level. Replaces what used to be two independent setup steps: an hxml
// `--macro hlx.runtime.HookDiscoveryMacro.use()` line (required by every mod, to discover
// @:hlx.prefix/@:hlx.postfix hooks anywhere in its sources) plus, only for mods also using
// @:hlx.config, a second, separate `@:build(ModConfigMacro.build())` on the main class.
//
// Those two mechanisms can't actually merge into one - HookDiscoveryMacro.use() runs via
// Context.onAfterTyping and can only ADD new types, so it must run once for the whole
// compilation; ModConfigMacro.build() REWRITES the annotated class's own fields, which only a
// per-class @:build macro can do (onAfterTyping sees fields after they're already typed and
// frozen). This class just collapses both call sites into the one place a mod already has to
// annotate: HookDiscoveryMacro.use() is idempotent (guarded by its own `used` flag), so calling
// it here is safe whether or not this mod also uses @:hlx.config.
class Mod {
    public static function build():Array<Field> {
        HookDiscoveryMacro.use();
        return ModConfigMacro.build();
    }
}
#end
