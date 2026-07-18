# Human-Scale Route Access Slice

This pass closes the P0 world row for human-scale doors, windows, stairs, and cover by making the already-running route-access systems auditable. The game now has visible purpose-coded navigation architecture, collision-cleared critical access zones, nonblocking route-adjacent decorative cleanup, and promotion gates for future imported city modules.

## Runtime Coverage

- `EnsureEntryAccessCorridorClear()` clears seven critical zones: entry, armory, safehouse, launch language marker, terminal, survivor, and helipad.
- The clearance pass disables collision and physics on route-blocking decorative static actors, tags them `EntryCorridorCollisionCleared`, and emits `[CodeRescueEntryAccess]`.
- `SpawnPurposeClarityLayer()` adds a navigation legend, objective pads, pylons, panels, and text-first route labels for ENTRY, ARMORY, PROTECTED CODING SAFEHOUSE, SURVIVOR, EXTRACTION, and OPTIONAL BOSS RISK.
- Route-adjacent decorative static actors become `CriticalPathNonBlockingArchitecture` so authored set dressing can read visually without blocking the player.
- `Content/CodeRescueData/human_scale_route_access_manifest.tsv` records each runtime surface, tag contract, player effect, and verifier.

## Source Guidance

- `WORLD_DEVELOPMENT_DEEPDIVE.md` calls for authored safe rooms, clear paths, readable landmarks, designed critical paths, encounter cover, and escape routes.
- `GAME_PHYSICS_DEEPDIVE.md` calls for simple, reliable collision discipline and cover surfaces that are promoted intentionally.
- `world_promotion_validation_contract.tsv` keeps future imported modules gated on human-scale proportions, walkable collision, AI navigation clarity, weapon-trace readability, and recovery-route accessibility.

## Boundaries

This slice does not replace final building art. It keeps the current procedural/fallback city playable and reviewable while future modular interiors, doors, stairs, cover, and collision assets move through world promotion validation.

## Validation

- `python3 Scripts/verify_human_scale_route_access_slice_pass.py`
- `python3 Scripts/verify_june19_playability_readability_fix_pass.py`
- `python3 Scripts/verify_world_promotion_validation_contract_pass.py`
- Packaged null/render smoke and runtime log contracts for `[CodeRescueEntryAccess]` and `[CodeRescueArchitectureClarity]`.
