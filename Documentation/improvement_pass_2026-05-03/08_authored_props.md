# Item 8 — Authored environment kit-bash for Anchorage

## What changed
`ACodeRescueGameMode::SpawnAuthoredPropsForCity(CityIndex, Origin, Accent)`
spawns 8 prop instances (cars / dumpsters / shipping containers) at
deterministic offsets in Anchorage (CityIndex == 0). Other cities get a
3-prop subset to populate the open spaces without overwhelming.

All props are tagged "AuthoredProp" so they can be enumerated for later
swap-in of authored Megascans meshes via Blueprint subclass overrides.

Currently uses Engine basic shapes (cube/cylinder via `SpawnBlock`) with
distinct color tints per prop role:
- Cars: 3 colors (charcoal, red, blue) at NW/NE/SW offsets
- Dumpster: brown at SE
- Shipping containers: 4 different tints in the corners

## Files touched
- `Source/CodeRescueUnreal/CodeRescueGameMode.h/.cpp`

## Design decisions
- Hardcoded prop table inside the function so behavior is deterministic
  across runs and no data file needs updating to start playtesting.
- Uses the existing `SpawnBlock` helper so collision + materials work the
  same way as the rest of the procedural geometry.
- Anchorage gets the full 8-prop kit; other cities get 3 to keep the
  342-city campaign performant.

## Known limitations
- These are not real authored meshes; they're colored cubes shaped roughly
  like cars/containers. The visual upgrade is gated on Megascans/PCG
  imports — see the follow-up.
- Props block player movement but don't break line-of-sight in any
  meaningful way (no occluder geometry).

## Follow-up work
1. Import a small Quixel set (vehicle, dumpster, shipping container, debris
   pile) via Megascans Bridge.
2. Author a `BP_AnchoragePropKit` Blueprint with `TArray<TSubclassOf<AStaticMeshActor>>`
   referencing those imports.
3. Swap the inline `SpawnBlock` calls in this function for the BP-driven
   version when the assets land.
4. Add a PCG graph for distributing skylines + props across a wider area
   for the cities past Anchorage.
