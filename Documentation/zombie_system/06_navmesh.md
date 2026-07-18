# Item 6 — NavMesh

**Status:** DONE (code) — runtime NavMesh bounds volume spawned;
NavigationSystem auto-generates the mesh. Pathfinding is unused yet —
item 7's controller will switch to it once the BT lands.

## What landed

1. **`Config/DefaultEngine.ini`** — added the `NavigationSystemV1` and
   `RecastNavMesh` blocks:

   ```ini
   [/Script/NavigationSystem.NavigationSystemV1]
   bAutoCreateNavigationData=True
   bSpawnNavDataInNavBoundsLevel=True

   [/Script/NavigationSystem.RecastNavMesh]
   RuntimeGeneration=Dynamic
   TilePoolSize=1024
   TileSizeUU=2000
   CellSize=15.0
   CellHeight=10.0
   AgentRadius=42.0
   AgentHeight=192.0
   AgentMaxStepHeight=35.0
   AgentMaxSlope=44.0
   ```

   `RuntimeGeneration=Dynamic` is the key — without it the navmesh only
   builds when you explicitly bake from the editor, which doesn't help
   a procedurally-spawned world.

2. **`ACodeRescueGameMode::SpawnWorld`** — spawns a single
   `ANavMeshBoundsVolume` covering all three zones plus the road
   connectors after the zone geometry is in:

   ```cpp
   ANavMeshBoundsVolume* NavBounds = W->SpawnActor<ANavMeshBoundsVolume>(...);
   NavBounds->SetActorScale3D(FVector(800.0f, 320.0f, 40.0f));
   NavBounds->ReregisterAllComponents();
   ```

   The default volume has a 1×1×1 cube brush; `SetActorScale3D` makes
   it ~80,000 × 32,000 × 4,000 units which is enough for the current
   map. Editor-only `UCubeBuilder` is unavailable at runtime, hence the
   transform-based approach.

## What's NOT used yet

`ACodeZombieActor::Tick` still uses direct `AddActorWorldOffset` chase.
Hooking it up to the navmesh requires either:

- Converting the actor to `ACharacter` (item 2 full version) so an
  `AAIController` can drive `MoveToActor` / `MoveToLocation`, OR
- Manually pathfinding via `UNavigationSystemV1::FindPathToLocationSynchronously`
  and walking the result yourself.

Both options live in item 7's `ACodeRescueAIController` once it grows
past being a stub.

## Files touched

- `Config/DefaultEngine.ini`
- `Source/CodeRescueUnreal/CodeRescueGameMode.cpp` — added NavMesh block
  in `SpawnWorld`; new `#include "NavMesh/NavMeshBoundsVolume.h"`,
  `Components/BrushComponent.h`, `Components/BoxComponent.h`

## How to verify

1. PIE.
2. Console (` ` ` key): `show navigation`. Green tiled mesh should
   draw on top of the floor planes spanning all three zones.
3. Or: console `RecastNavMesh.bDrawNavMesh 1` for the Recast-specific
   debug view.

If the navmesh is empty ("no navigation tiles found"), check that:

- `RuntimeGeneration=Dynamic` is in `DefaultEngine.ini` (case-sensitive).
- The bounds volume actually spawned — search `NavMeshBounds` in the
  Outliner.
- The floor planes have collision enabled (the procedural cube floors
  do; just listing the things that go wrong).

## Why a single volume instead of one per zone

Three small volumes give marginally faster initial generation but
introduce gaps where navmesh tiles don't bridge. A single volume that
covers everything is easier to reason about and the UE 5.7 Recast tiler
is fast enough that we don't need to optimize that.
