# hlx-core

Core components of the **HLX Modding Framework**, a native mod-injection framework for [HashLink](https://hashlink.haxe.org/)-based games.
It lets mod authors hook into a game's own compiled functions by name, with no game source, no per-game native code, and no manual offset/index bookkeeping.

## Architecture

Six components, six jobs, no overlap:

1. **hlx-boot** — a minimal native kernel. Boot, hook, find the game's real module, patch machine code, and expose a handful of generic reflection primitives.
   Knows nothing about mods, games, prefixes, or dispatch order.
2. **`hlx-loader`** — one Haxe-compiled bootstrap module, loaded once. Owns the *only* piece of genuinely shared, cross-mod runtime state:
   the patch registry and the dispatch algorithm.
3. **`hlx-runtime`** (haxelib) — shared Haxe *source*: externs, the `@:hlx.prefix`/`@:hlx.postfix`/`@:hlx.config` macro, shared types, caching helpers.
   No shared *state* — every mod compiles its own copy, and that's fine, because nothing here needs to be shared.
4. **HLX.GamelibGenerator** — an offline, dev-time-only tool. Not part of the runtime stack.
5. **Gamelib** — per-game, generated (or hand-written) Haxe source. Compile-time only; leaves no runtime footprint of its own.
6. **Mod code** — what a mod author actually writes, against the gamelib and `hlx.runtime`.

```
┌─────────────┐     ┌──────────────────────────────────────────────┐
│   hlx-boot  │◄────┤ hlx-loader (registry, dispatch, mod loading) │
└──────▲──────┘     └─────────────────┬────────────────────────────┘
       │                              │ registers via hlx-runtime externs
       │ raw reflection primitives    │ 
       │                              │
┌──────┴──────────────────────────────▼─────────────────────────┐
│ Mod code  →  gamelib (compile-time)  →  hlx-runtime (haxelib) │
└───────────────────────────────────────────────────────────────┘

Offline, separate from all of the above:  
HLX.GamelibGenerator (reads hlboot.dat) → emits → Gamelib source
```

A mod targets a real game method by name only:

```haxe
@:build(hlx.runtime.Mod.build())
class MyMod {
    @:hlx.prefix(App.update)
    static function beforeUpdate(instance:App, dt:Float):HlxPrefixControl {
        trace('about to update, dt=$dt');
        return Continue;
    }
}
```

No findexes, no field indices, no manual signature bookkeeping.

## Repository layout

| Path             | Component                  |
| -----------------| -------------------------- |
| `.tools`         | Build and deploy scripts   |
| `hlx-boot`       | minimal native kernel      |
| `hlx-loader`     | the mod loader module      |
| `hlx-runtime`    | haxelib: mod-authoring API |

## Building

Requires Windows, MSVC (or the "Desktop development with C++" Visual Studio workload), CMake, and the Haxe compiler.

```bash
pnpm install
pnpm run setup   # one-time: records your game's install path
pnpm run build   # builds libhl64.dll and hlx-loader.hl
pnpm run deploy  # copies both into the configured game folder, plus hlx/mods and hlx/logs
```

After deploying, launch the game and `hlx-loader.hl` loads at boot and scans `hlx/mods/` for one subfolder per mod, loading `hlx/mods/<mod-name>/<mod-name>.hl` from each.
Check `hlx/logs/hlx.log` for boot/hook/native diagnostics.
To roll back, delete `libhl64.dll` and the `hlx/` folder from the game's install directory.

## License

MIT
