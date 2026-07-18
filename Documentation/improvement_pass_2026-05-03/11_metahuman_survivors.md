# Item 11 — MetaHuman survivor scaffolding

## What changed
`ASurvivorActor` got two new soft-ref UPROPERTYs:
- `RescueVoCue` (`TSoftObjectPtr<USoundBase>`) — voice line played on rescue.
- `IdleBarkCue` (`TSoftObjectPtr<USoundBase>`) — distress bark played on a
  20-30s random idle timer (so trapped survivors can be triangulated by ear).

`Rescue()` now plays `RescueVoCue` (if set) at the survivor's location and
clears the idle-bark timer. `BeginPlay` arms the first idle bark.

The existing `ProfessionalSurvivorMesh` + `ProfessionalSurvivorAnimClass`
fields already supported MetaHuman skeletal mesh hookup — see the BeginPlay
body for the swap that hides the procedural cube/sphere body when a real
mesh is assigned. This pass added the audio half of the puzzle.

## Files touched
- `Source/CodeRescueUnreal/SurvivorActor.h/.cpp`

## Design decisions
- Soft refs so unimported audio leaves the build correct (just silent).
- Idle bark on a 20-30s random interval rather than a fixed cadence so
  multiple survivors don't accidentally sync their barks.
- Rescue VO plays from the survivor location (not 2D) so positional context
  reads correctly when the player turns away mid-rescue.

## Known limitations
- No MetaHuman is actually assigned. All survivors render as procedural
  cube + sphere by default.
- Idle bark plays only when ProfessionalSurvivorMesh is unset too — we did
  not add a "stop barking when player is within X units" behavior.

## Follow-up work — MetaHuman import flow
1. Open Quixel Bridge inside the UE editor (Window → Quixel Bridge).
2. Sign in to your Epic account.
3. Browse MetaHumans → pick an "office worker" or "civilian" preset.
4. Click "Add" — Bridge downloads and adds to `Content/MetaHumans/`.
5. In `BP_SurvivorActor` (create if absent — Blueprint subclass of
   ASurvivorActor), set:
   - `ProfessionalSurvivorMesh` → `Body` skeletal mesh from MetaHuman folder.
   - `ProfessionalSurvivorAnimClass` → `ABP_MetaHuman` (the supplied AnimBP).
6. Assign `BP_SurvivorActor` to `ACodeRescueGameMode::SurvivorActorClass`.
7. Import 1-2 voice WAVs per survivor archetype (or use macOS `say` via the
   radio script). Assign to `RescueVoCue` / `IdleBarkCue`.
