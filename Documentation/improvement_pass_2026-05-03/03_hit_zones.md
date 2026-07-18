# Item 3: Hit Zones and Headshot Multipliers

## Summary
Added bone-name-based hit zone classification (Head=2.0x, Torso=1.0x, Limb=0.5x) to the Fire() function. Headshots now deal 2x damage and increment a HeadshotCount tracker. HUD displays transient "HEADSHOT!" feedback in red when a head hit occurs.

## What Changed
- **CodeRescueCharacter.h**: Added `EHitZone ClassifyHitZone(const FName&)` declaration; added private fields `FTimerHandle ReloadTimerHandle` and `float LastHeadshotTime` for tracking.
- **CodeRescueCharacter.cpp**: Implemented `ClassifyHitZone()` function that parses bone names to classify hits; updated `Fire()` to extract `Hit.BoneName`, classify zone, apply multiplier (Head 2.0x, Torso 1.0x, Limb 0.5x), and call `ApplyRescueDamage(FinalDamage, HitZone)`.
- **CodeRescueSaveGame.h**: Already updated with `int32 HeadshotCount` and `bool bHasHeadshotCount` backcompat flag.
- **CodeRescueHUDWidget.h**: Added `UTextBlock* HeadshotFeedbackText` to display transient headshot feedback.
- **CodeRescueHUDWidget.cpp**: Enhanced `RefreshHUD()` to display "HEADSHOT!" in red for 0.5 seconds after a head hit.

## Design Decisions
1. **Bone Name Parsing**: Performs case-insensitive string matching on bone names (e.g., "head", "neck", "spine", "pelvis", "arm", "leg", "hand", "foot") rather than relying on a pre-built bone table. Tolerates missing or mismatched bone names by defaulting to `Other`.
2. **Damage Multipliers**: Head=2.0x (headshots are high-value), Torso=1.0x (baseline), Limb=0.5x (reduced damage for appendages).
3. **Headshot Feedback**: Tracks `LastHeadshotTime` per-frame; HUD displays red "HEADSHOT!" text only if less than 0.5 seconds have elapsed, providing immediate visual confirmation.
4. **Backward Compat**: Save file includes `bHasHeadshotCount` flag; old saves skip restoration, new saves persist headshot count across sessions.

## Files Touched
- `Source/CodeRescueUnreal/CodeRescueCharacter.h` (+2 fields, +1 function decl)
- `Source/CodeRescueUnreal/CodeRescueCharacter.cpp` (+1 function, Fire() enhanced)
- `Source/CodeRescueUnreal/CodeRescueSaveGame.h` (backcompat fields, already in place)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.h` (+1 text widget)
- `Source/CodeRescueUnreal/CodeRescueHUDWidget.cpp` (RefreshHUD enhanced)

## Known Limitations
1. **Bone Names Vary by Mesh**: Different zombie packs use different bone naming conventions (e.g., "Head" vs "head", "Armature.Spine" vs "Spine"). The bone classifier uses fuzzy matching; edge cases may mis-classify.
2. **No Ragdoll Physics on Headshot**: Heads can be shot cleanly without special ragdoll or gore effects; death montage plays regardless.
3. **HeadshotCount Not Used in UI Yet**: Tracked in save game and memory, but no scoreboard or summary screen displays it. Item 15 (scoreboard) will integrate this stat.

## Follow-Up Work
1. Refine bone classification with data-driven bone-to-zone mapping (DataTable).
2. Add headshot-specific kill audio/VFX (headshot "ping" sound, screen flash, particle effects).
3. Integrate headshot count into leaderboard/stats screen (Item 15).
4. Tune damage multipliers based on zombie difficulty balancing.

## Compiler Notes
Requires linking against `Niagara` module for `UNiagaraFunctionLibrary` (already in use). No new external dependencies.
