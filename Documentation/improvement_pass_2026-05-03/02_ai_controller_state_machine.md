# Item 2 — AI controller state machine

## What changed
`ACodeRescueAIController` was promoted from a stub to a real 5-state
machine driven from `Tick`:

```
   Patrol ─► Investigate ─► Chase ─► Attack
      ▲                                │
      └─────────── Stagger ◄───────────┘
```

State transitions:
- **Patrol → Chase** when player is within `SightRadius` (default 2500 u)
  AND `IsPlayerVisible()` returns true (line trace ECC_Visibility, ignoring
  self + pawn).
- **Chase → Attack** when within `AttackRange` (read from possessed zombie).
- **Attack → Chase** when player drifts out of attack range.
- *** → Stagger** when `EnterStagger()` is called externally (from
  `ACodeZombieActor::ApplyRescueDamage` when damage > 15). 0.4 s freeze.
- **Stagger → Chase** after the recovery timer.
- **Patrol** picks a random nearby waypoint every 4 s.

Visibility check is throttled to once every 0.5 s to avoid hot-pathing
line traces.

## Files touched
- `Source/CodeRescueUnreal/CodeRescueAIController.h`
- `Source/CodeRescueUnreal/CodeRescueAIController.cpp`
- `Source/CodeRescueUnreal/CodeRescueUnreal.Build.cs` —
  `AIModule`, `NavigationSystem` added to PublicDependencyModuleNames.

## Design decisions
- Uses navmesh-aware `SimpleMoveToActor` / `SimpleMoveToLocation` for
  pathfinding. The NavMeshBoundsVolume spawned per city by GameMode covers
  the whole 50×-scaled play area, so pathing should resolve.
- Doesn't use UAIPerceptionComponent — single line-trace check is cheaper
  and the prototype only needs sight, not hearing/touch.
- The chase logic in `ACodeZombieActor::Tick` is left as a fallback for
  when no controller possesses the pawn (e.g., variant where AutoPossessAI
  is overridden in a Blueprint subclass).

## Known limitations
- Patrol radius is fixed at 800 u — should probably scale with zone activity.
- No "give up after losing sight" behavior; once Chase is entered, the
  controller keeps chasing until the player dies or the zombie does.
- No squad coordination — every controller targets the player independently.

## Follow-up work
- Behavior Tree authored asset (this is the C++ scaffold; a `BT_Zombie` +
  `BB_Zombie` blackboard asset would make per-variant tuning easier).
- "Lose interest" timer: after 8 s without LoS, drop back to Patrol.
- Cross-zombie aggro broadcast so neighbors join a chase.
