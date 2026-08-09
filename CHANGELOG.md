# Changelog

All notable changes to hlx-core (hlx-boot, hlx-loader, hlx-runtime) are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

hlx-boot, hlx-loader, and hlx-runtime version together as a single unit.

Add entries under `[Unreleased]` as changes land.
Before tagging a release, move the `[Unreleased]` entries under a new `[X.Y.Z] - YYYY-MM-DD` heading,
and bump the version in `hlx-runtime/haxelib.json` to match the tag — the release workflow checks this and fails otherwise.

## [Unreleased]

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
