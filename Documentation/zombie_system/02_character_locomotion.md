# Item 2 — AnimBP velocity hookup (partial)

**Status:** PARTIAL — the per-frame velocity hookup landed; the full
`AActor → ACharacter` conversion is intentionally deferred.

## What landed

`ACodeZombieActor` now overrides `AActor::GetVelocity()` and returns a
per-tick computed velocity vector. Standard locomotion AnimBPs (the
`ThirdPerson_AnimBP` shipped with most marketplace packs is a textbook
case) read `Pawn.GetVelocity()` to drive their walk/run blendspaces. With
this in, those blendspaces actually transition between idle / walk / run
based on the zombie's real motion instead of staying frozen at the idle
pose.

Implementation:

- New private members `PreviousLocation` and `CachedVelocity` on
  `ACodeZombieActor` (`CodeZombieActor.h`).
- `BeginPlay` seeds `PreviousLocation = GetActorLocation()`.
- `Tick` computes `(Now - PreviousLocation) / DeltaSeconds` and smooths
  it via `FMath::VInterpTo` so single-frame collision spikes don't
  whiplash anims.
- The `bIsDying` early-out decays cached velocity to zero so the AnimBP
  blends back to idle (or whatever its zero-velocity pose is) before
  the death montage takes over.

## What's deferred

The bigger refactor: convert `ACodeZombieActor` from `AActor` to
`ACharacter`. That would give it a proper `UCharacterMovementComponent`,
capsule collision, and footstep notifies — and unlock things like:

- Real navmesh-based locomotion (just call `AAIController::MoveToActor`
  instead of the current direct `AddActorWorldOffset`).
- Walking up stairs / over small obstacles automatically.
- Footstep sounds via AnimNotify, no per-actor wiring.
- Air control (jump, fall) for variants that should leap.

Why deferred: it's a multi-touch refactor with risk:

- Every `Body` / `Head` / `SkeletalBody` reference in the existing
  constructor needs to be re-mapped onto `ACharacter`'s built-in
  `Mesh` and `CapsuleComponent`.
- The save system stores `ZombieId` and the spawn loop uses `SpawnActor`
  — both need re-checking.
- `ACharacter` defaults pose for collision, gravity, friction may not
  match what the procedurally-spawned cube floors expect; expect tuning.
- The AI controller logic in `ACodeZombieActor::Tick` becomes redundant
  and should move to `ACodeRescueAIController` (item 7's full version).

## Files touched

- `Source/CodeRescueUnreal/CodeZombieActor.h`
  — added `PreviousLocation`, `CachedVelocity`; override `GetVelocity()`.
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`
  — `BeginPlay`, `Tick` updates; `bIsDying` velocity decay.

## How to verify

The change is invisible without an AnimBP that reads velocity. Spawn a
zombie variant whose AnimBP exists (e.g. UrbanZombie4 or BusinessSuit),
press Play, and watch whether its mesh's legs actually animate as it
walks vs. T-pose with translation-only motion. If the legs animate, the
hookup is working.

## Next-session recipe for the full ACharacter conversion

1. Branch the project (or backup `Source/`).
2. Change `class ACodeZombieActor : public AActor` →
   `class ACodeZombieActor : public ACharacter`.
3. Remove the `Body`/`Head`/`SkeletalBody`/`Glow` member declarations
   and stop creating them in the constructor — `ACharacter` ships with
   a `CapsuleComponent` (root) and `Mesh` (USkeletalMeshComponent).
   Keep `Glow` if you still want the infection light.
4. In `InitializeFromVariant`, replace `SkeletalBody->SetSkeletalMesh(...)`
   with `GetMesh()->SetSkeletalMesh(...)` and the same for `SetAnimInstanceClass`.
5. In `Tick`, replace `AddActorWorldOffset(...)` with
   `AddMovementInput(Direction, 1.0)` (after setting
   `GetCharacterMovement()->MaxWalkSpeed = MoveSpeed` once at variant init).
6. Drop the `GetVelocity()` override — `ACharacter` already returns the
   movement component's velocity correctly.
7. Update spawn in `ACodeRescueGameMode::SpawnWorld` — same call works
   but the spawned actor will have a different bounding box; you may
   need to bump the spawn Z by ~80 units so the capsule isn't stuck in
   the floor.
8. Compile, PIE-test, fix whatever pops.
