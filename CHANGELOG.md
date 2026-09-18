# Changelog

All notable changes to hlx-core (hlx-boot, hlx-loader, hlx-runtime) are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

hlx-boot, hlx-loader, and hlx-runtime version together as a single unit.

Add entries under `[Unreleased]` as changes land.
Before tagging a release, move the `[Unreleased]` entries under a new `[X.Y.Z] - YYYY-MM-DD` heading,
and bump the version in `hlx-runtime/haxelib.json` to match the tag — the release workflow checks this and fails otherwise.

## [Unreleased]

## [0.0.9] - 2026-09-18

Fix native hook installation failing on functions the previous byte-level whitelist couldn't safely relocate

- `patching.c`'s hand-rolled instruction whitelist is replaced with MinHook 1.3.4 (vendored in
  `hlx-boot/vendor/minhook/`), which decodes and relocates real `CALL`/`JMP`/`Jcc` and
  RIP-relative operands instead of refusing to patch past them - fixes `install_patch failed`
  for short, straight-line functions whose JIT'd prologue no longer padded past those opcodes
  once the game's JIT got leaner (see `documentation/minhook.md` for the full root-cause trace)
- The existing known-function-boundary safety check stays - MinHook has no equivalent of it,
  only local heuristics - and MinHook's own hotpatch-above fallback is disabled outright, since
  JIT'd functions are packed back-to-back with no padding before them for it to use
- `MH_CreateHook` is now wrapped in SEH - MinHook has no internal exception handling of its own,
  and a target too close to the end of the JIT code region could otherwise crash the whole
  process instead of failing just that one patch
- Thanks to [xWink](https://github.com/xWink), who pointed to MinHook as the fix for HL 2.0's
  leaner JIT prologues breaking the old whitelist scanner.

Fix two `install_patch` correctness bugs found while rewriting the patcher

- A transient allocation or scan failure while building the function-boundary cache no longer
  permanently disables that safety check for the rest of the process - it now retries on the
  next `install_patch` call instead of latching the failure forever
- Two different patch targets that resolve to the same underlying function address (e.g. an
  inherited, non-overridden method) no longer silently share one hook - installing a second,
  different receiver against an address already patched by another now fails loudly instead of
  silently dropping the second mod's hook with no error anywhere

## [0.0.8] - 2026-09-13

Show a clear in-game error and exit cleanly when the game's bytecode version isn't supported

- `reflection_init_constructor_table` (`hlx-boot`) now shows a `MessageBoxA` naming the
  unsupported bytecode version and inviting the user to disable the hlx-core mod - via mod
  manager, or by deleting `libhl64.dll` - before terminating the process with `ExitProcess(1)`,
  instead of logging the failure and silently leaving `construct_instance_by_name` failing
  closed for every type

Support the new game version's HashLink 2.0 bytecode format alongside the current one, from the same build

- `hlx-boot`'s vendored HashLink reader (`vendor/hashlink/`) updated from HashLink 1.16 (bytecode
  version 4) to 2.0 (bytecode version 6) - both versions parse correctly from the same build since
  HashLink's own reader is already runtime-gated on `c->version`, not a hardcoded one-version reader
- `ResolveFunctionByFindex`'s live-module function mirror (`hlx_function_mirror_t`) now branches on
  the running game's actual bytecode version before reading `.type` - real `hl_function` gained two
  new fields (`nassigns`/`assigns`) in the 2.0 layout, which silently shifted every field after them;
  reading the old fixed offsets against a 2.0 game would have corrupted `construct_instance`/
  `construct_instance_by_name` resolution instead of failing loudly
- Same fix applied to the `hl_setup` live mirror that gates loading `hlx-loader.hl` - `hl_setup_t`
  also gained a field (`capture_break_context`) shifting `load_plugin`'s offset in the 2.0 layout

Include the patched function's name in patch-installation failure logs

- `install_patch` (`hlx-boot`) now takes the target's qualified type+method name and includes it
  in every `PatchFunctionPrologue` log line (e.g. "no safe cut point found"), replacing the
  previous bare `patch#12` index - lets a native-side patch failure be matched directly to its
  corresponding `hlx-loader` log line without counting registrations
- `hlx-loader`'s `Native.installPatch` extern gained the matching `label` parameter, and
  `Registry.installAllPendingPatches` now passes the target's `PatchTargetKey` string through -
  needed its own `@:access(String)` on `Registry` to reach `String.bytes` for that conversion,
  the same access `Native.hx` already grants itself for its other by-name native calls

## [0.0.7] - 2026-08-14

Add `Registry`, a generic bucket/key/value store for sharing retained state across isolated mods

- New `hlx-runtime` API `hlx.runtime.Registry` (`register`/`unregister`/`get`/`list`), Phase 1
  ("Core Registry") of `plans/CORE-SHARED-BUS-REGISTRY.md` - lets mods, each compiled into their
  own separate HL module sharing no Haxe statics, discover and read each other's state (e.g. a mod
  registering itself under the `mods` bucket as `{id, name, version}`)
- New `hlx-boot` natives (`registry.c`, `hlx_kv_register`/`_unregister`/`_get`/`_count`/`_value_at`)
  back the store once per process, rooting each registered `Dynamic` value with `hl_add_root` so
  it survives independently of the registering mod's own module lifetime; re-registering the same
  `(bucket, key)` replaces the existing entry rather than duplicating it
- No pub/sub or event semantics - purely retained key/value state; the separate Message Bus
  (Phase 2) is not part of this change

Add `Bus`, a transient publish/subscribe mechanism for cross-mod notifications

- New `hlx-runtime` API `hlx.runtime.Bus` (`publish`/`subscribe`/`unsubscribe`), Phase 2 ("Core
  Message Bus") of `plans/CORE-SHARED-BUS-REGISTRY.md` - lets one mod notify others of a transient
  event (e.g. a chat mod publishing `"command.execute"` for whichever mod owns that command to
  react to) without either side knowing about the other's module
- New `hlx-boot` natives (`bus.c`, `hlx_bus_publish`/`_subscribe`/`_unsubscribe`) dispatch a topic's
  current subscribers, in registration order, via `hl_dyn_call`, passing `payload` as the
  handler's one argument; re-subscribing the same (topic, handler) replaces the existing
  subscription instead of accumulating a duplicate, the same re-init safety net `Registry.register`
  already gives named entries, adapted here to a handler's code address + receiver since
  `subscribe` has no separate key of its own
- No RPC, responses, queues, priorities, or delivery guarantees - a topic can have any number of
  subscribers, but publish/subscribe carry no acknowledgement or ordering promise beyond
  registration order
- `hlx_common.c` now resolves `hl_add_root`/`hl_remove_root` once (`hlx_gc_resolve_setup`), shared
  by both `registry.c` and `bus.c`, rather than each resolving its own copy

## [0.0.6] - 2026-08-10

Fix `@:hlx.config` silently dropping new fields added to a mod's config default after a config.json was already saved

- `ModConfig.load` now recursively merges a loaded `config.json` against the mod's default value instead of using
  the saved file as-is - a field added (or moved a level deeper) in the mod's code default since the file was last
  saved was previously surfacing as `null` on first read instead of falling back to the code default

## [0.0.5] - 2026-08-07

Add `HlxRuntime.resolveAbstract` for cross-module native abstract casts

- A `Dynamic` value carrying a native `hl.Abstract<"...">` read off another compiled module (e.g.
  a gamelib field resolved via reflection) can now be cast to that concrete abstract type - HL's
  own `hl_same_type` compares abstracts by `abs_name` *pointer* identity, which two independently
  compiled modules never share even for the same name, so a plain `cast` always failed here before
- New `hlx-boot` native `box_dynamic_ptr`/`hlx_box_ptr`: re-tags a raw pointer as `Dynamic` under
  an arbitrary caller-supplied `hl_type`, the one piece of this that pure Haxe can't do
  (`hl_alloc_dynamic` is HL_API-only, never exposed via a `DEFINE_PRIM`)

## [0.0.4] - 2026-07-29

Add native plugin loading, harden module scan, fix hook return corruption

- Support plain .hdll native plugins in hlx/plugins/<mod>/, loaded eagerly
  before hlx-loader.hl so same-name DLL shadowing resolves deterministically
- Ship PDB symbols with Release builds for post-mortem crash debugging
- Rename loadPlugin -> loadMod in Boot/Native to disambiguate from the new
  native plugin concept
- Guard FindPrimaryModule's process-wide scan with SEH and bounds checks
  against reading memory freed mid-scan by another thread
- Add opt-in rawReturn path for hook receivers whose real return type is a
  native pointer (hl.Abstract<...>), fixing silent corruption when
  contributors must declare Dynamic to avoid the abs_name check

## [0.0.3] - 2026-07-22

Fixes a reliability bug in 0.0.2's constructor resolution: reading the game's already-running bytecode could intermittently miss or misidentify a constructor, since HashLink frees that data almost immediately after the game starts. Constructors now resolve by reading the game's own hlboot.dat file directly instead, which is both reliable and no longer timing-dependent.

- No longer depends on catching the game's bytecode before HashLink frees it - reads hlboot.dat directly instead
- Confirmed working end-to-end against a real game run, with every constructor call site resolving correctly

## [0.0.2] - 2026-07-22

Constructors for generated wrapper types now survive a game update without the gamelib needing to be regenerated first.

- New instances are found by scanning the loaded game's own bytecode at load time, not a number baked in when the gamelib was generated
- Falls back to a clear error instead of silently calling the wrong function if a future game build ever makes construction ambiguous

## [0.0.1] - 2026-07-18

### Added
- `hlx-boot`: minimal native kernel. Boot, hook, patch machine code and expose a generic reflection primitives.
- `hlx-loader`: the mod loader module.
- `hlx-runtime` haxelib: mod-authoring API (`@:hlx.prefix`/`@:hlx.postfix`/`@:hlx.config`).
- `.tools/` scripts for setup, build, and deploy.
- CI workflow: compile checks and native build verification.
- Release workflow: packaging and GitHub release publishing.
