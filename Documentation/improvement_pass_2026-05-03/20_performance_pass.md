# Item 20 — Performance pass

## What changed (this pass)
**Distant-zombie tick throttle.** `ACodeZombieActor::Tick` now skips the
chase + attack logic when the player is more than 10000 units away,
running at ~0.5 Hz instead of full frame rate. A `DistantTickAccumulator`
field counts elapsed seconds and resets when the player is back within
range.

This is meaningful on the 50× cities since a 342-city campaign can have
dozens of off-screen zombies idling per frame.

## Files touched
- `Source/CodeRescueUnreal/CodeZombieActor.h` — `DistantTickAccumulator`
  field.
- `Source/CodeRescueUnreal/CodeZombieActor.cpp` — throttle gate at top of
  `Tick`.

## Other items in this pass (not yet implemented; each is small)

### A. HISM for repeated kit-bash meshes
The `SpawnBlock` calls in `SpawnCampaignCity::SpawnSkylineBlock` create one
`AStaticMeshActor` per building. For ~20 buildings × 342 cities (worst case
if every city were resident) that's a lot of actors. Convert to a single
`UHierarchicalInstancedStaticMeshComponent` on the GameMode and add one
instance per building.

```cpp
// In SpawnCampaignCity, replace the building loop with:
if (!CityHISM)
{
    CityHISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
    CityHISM->SetStaticMesh(CubeMesh);
    CityHISM->RegisterComponent();
}
for (int32 i = 0; i < BuildingCount; ++i)
{
    FTransform T(FRotator::ZeroRotator, BuildingLoc, Scale);
    CityHISM->AddInstance(T);
}
```

### B. Benchmark baseline
On the user's Mac (M4 Pro, what the user's running):
1. Boot PIE on the largest city (CityIndex 341).
2. Open `~` console: `stat unit`, then `stat scenerendering`, then `stat gpu`.
3. Hold Shift+W and walk for 30 s to get a stable reading.
4. Save `Saved/Profiling/baseline_2026-05-03.csv` via the editor's stat HUD.

Record the baseline numbers in this file under "Recorded baselines:" so
future passes can compare. (No baseline numbers recorded in this commit.)

### C. Distance culling on procedural cubes
City building blocks have shadow casting off via `MeshComp->SetCastShadow(false)`,
which already helps. Adding `CullDistanceVolume` actors per city would let
the engine drop sub-1000-unit cubes once you're 4000 units away.

## Known limitations
- The throttle is pure tick-throttle; CPU-side animation update for
  ACharacter still ticks every frame. To go further, set
  `GetMesh()->VisibilityBasedAnimTickOption = OnlyTickWhenRendered` per
  zombie.

## Follow-up work
- Implement A and B (above).
- Add `OnlyTickWhenRendered` to `GetMesh()` in zombie BeginPlay.
- Profile horde fight (8-12 zombies in a 1000-unit ring) and see if the
  throttle hurts there.
