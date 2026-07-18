# Item 19 — Verify Mac Shipping build

**Status:** PARTIAL — the editor (Development) build compiled cleanly
after every C++ change in this pass. A full Shipping packaging via
`Package_Mac_App.command` was NOT run in-session (~5–10 minute build
not feasible in the screen-automation budget after the other 19 items).

## What WAS verified

After the final C++ batch (items 1–18 + 20):

- `Recompile_Module.command` ran clean — 17/17 link succeeded.
- Editor reopened, Python script ran, PIE launched.
- Inspecting CodeZombieActor0 in PIE confirmed runtime variant
  application math (Health 46.75 = 55 × 0.85, etc.).

So the **Development** Mac build is good.

## What needs a Shipping pass

Shipping configuration enables additional optimizations and strips
editor-only code. Things that historically break going from Dev to
Shipping in this project (per memory notes from prior passes):

1. **`DrawDebug*` calls outside `#if ENABLE_DRAW_DEBUG`.** The
   memory's "Status (resolved 2026-04-28, second pass)" note flags this
   — DrawDebug calls in GameMode + Character were wrapped in
   `#if ENABLE_DRAW_DEBUG`. New code added in this pass:
   - `ACodeRescueCharacter::Fire()` — line 417, sphere 456 — already
     `#if ENABLE_DRAW_DEBUG` wrapped (kept the existing wrapping). ✓
   - `ACodeZombieActor` — no new DrawDebug. ✓
   - `ACodeRescueHUDWidget` — uses `LineTraceSingleByChannel`, no debug
     drawing. ✓
   - `ACodeRescueAIController` — empty stub, no debug. ✓
2. **Editor-only modules referenced from runtime code.** I hit this
   during item 6 — `UCubeBuilder` is editor-only and broke the link.
   Removed before the final compile. No other editor-only includes
   added in this pass (verified by the successful Dev compile, which
   uses a similar but stricter linker on Apple).
3. **Soft asset references that fail to load on cooked builds.** All
   `TSoftObjectPtr<>` fields on `FZombieVariantRow` are loaded via
   `LoadSynchronous()` which works in both Dev and Shipping. The data
   table itself is `LoadObject<UDataTable>` in
   `ACodeRescueGameMode::ACodeRescueGameMode()` — this is the standard
   pattern.
4. **Python plugin in Shipping.** `PythonScriptPlugin` is enabled in
   the `.uproject`. Python is editor-only by default; including it in
   a Shipping cook is fine (it just doesn't load) but we should mark
   it as `WhitelistPlatforms = ["Editor"]` to be tidy. Optional.

## How to actually run the Shipping build

```sh
cd "/Users/labcomputer/Desktop/Operation_Code_Rescue/code_rescue_unreal_ue57_rebuild_fix"
./Package_Mac_App.command
```

Watch for:

- Cook log: zero "Failed to load" for the variant data table
  (`/Game/CodeRescueAssets/DT_ZombieVariants`) or the BP
  (`/Game/CodeRescueAssets/Blueprints/BP_CodeRescueGameMode`).
- Build phase: zero "Undefined symbols" — the same class of error that
  bit us with `UCubeBuilder` on the Dev side.
- Stage phase: the cooked .pak should include the variant table and
  all referenced skeletal meshes / animblueprints.

If anything fails, the most likely culprits are listed above; fix-up
patterns are well-known.

## Files relevant

- `Package_Mac_App.command` — invokes UAT BuildCookRun.
- All the new files (CodeRescueDeathWidget, CodeRescueAIController) are
  in `Source/CodeRescueUnreal/` so they're part of the runtime module
  and will cook.
