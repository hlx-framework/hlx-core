# Changelog

All notable changes to hlx-core (hlx-boot, hlx-loader, hlx-runtime) are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

hlx-boot, hlx-loader, and hlx-runtime version together as a single unit.

Add entries under `[Unreleased]` as changes land.
Before tagging a release, move the `[Unreleased]` entries under a new `[X.Y.Z] - YYYY-MM-DD` heading,
and bump the version in both `package.json` and `hlx-runtime/haxelib.json` to match the tag — the release workflow checks this and fails otherwise.

## [Unreleased]

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
