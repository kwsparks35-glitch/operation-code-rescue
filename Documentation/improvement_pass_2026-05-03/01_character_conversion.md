# Item 1: Convert ACodeZombieActor from AActor to ACharacter

**Status:** Complete (code changes finalized, pending compilation verification on macOS)  
**Date:** 2026-05-03  
**Files Modified:**
- `Source/CodeRescueUnreal/CodeZombieActor.h`
- `Source/CodeRescueUnreal/CodeZombieActor.cpp`

## Summary

Converted `ACodeZombieActor` from `AActor` to `ACharacter` to leverage Unreal's built-in `CharacterMovementComponent` for proper locomotion, collision, and velocity handling. This eliminates custom velocity tracking and prepares the zombie actor for AI controller-driven movement in Item 2.

## Changes Made

### Header (CodeZombieActor.h)

1. **Base Class Change**
   - From: `class CODERESCUEUNREAL_API ACodeZombieActor : public AActor`
   - To: `class CODERESCUEUNREAL_API ACodeZombieActor : public ACharacter`

2. **Removed Overrides**
   - Deleted `virtual FVector GetVelocity() const override`
   - This was a workaround for custom velocity tracking; CharacterMovementComponent provides built-in velocity

3. **Removed Member Variables**
   - Deleted `FVector PreviousLocation` (no longer needed for velocity calculation)
   - Deleted `FVector CachedVelocity` (CharacterMovementComponent handles velocity)
   - Deleted `USkeletalMeshComponent* SkeletalBody` (inherited from ACharacter as `GetMesh()`)

### Constructor (CodeZombieActor.cpp, lines 20–65)

1. **Capsule & Collision Setup**
   - Now calls `GetCapsuleComponent()->SetCollisionProfileName(TEXT("BlockAllDynamic"))`
   - Calls `GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision)` to prevent mesh collision

2. **Component Attachment**
   - `Body` (procedural fallback): attaches to `GetCapsuleComponent()` instead of being root
   - `Head`: attaches to `Body` (unchanged logic)
   - `Glow`: attaches to `GetCapsuleComponent()` instead of `Body`
   - `GrowlAudio`: attaches to `GetCapsuleComponent()` instead of `Body`

3. **CharacterMovementComponent Configuration**
   ```cpp
   GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
   GetCharacterMovement()->RotationRate.Yaw = 540.0f;
   GetCharacterMovement()->bUseControllerDesiredRotation = true;
   ```

4. **AI Controller Assignment**
   ```cpp
   AIControllerClass = ACodeRescueAIController::StaticClass();
   AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
   ```
   This ensures the zombie is automatically possessed by an AI controller upon spawn.

### BeginPlay() (lines 67–118)

1. **Mesh Switching Logic**
   - Changed all `SkeletalBody->` calls to `GetMesh()->`
   - Logic remains identical: use professional mesh if available, else fall back to procedural primitives

2. **Removed Velocity Tracking**
   - Deleted `PreviousLocation = GetActorLocation()` seed
   - CharacterMovementComponent tracks velocity internally

3. **VFX Attachment**
   - Infection aura still spawns attached to `RootComponent` (now the capsule)

### Tick() (lines 141–205)

1. **Dying State Cleanup**
   - Changed from: `CachedVelocity = FMath::VInterpTo(...)`
   - Changed to: `GetCharacterMovement()->StopMovementImmediately()`

2. **Out-of-Range Handling**
   - Changed from: velocity decay + location tracking
   - Changed to: `GetCharacterMovement()->StopMovementImmediately()`

3. **Movement Implementation**
   - Changed from: `AddActorWorldOffset(Direction * MoveSpeed * DeltaSeconds, true)`
   - Changed to: `AddMovementInput(Direction, 1.0f)`
   - This lets CharacterMovementComponent handle acceleration, velocity, and collision

4. **Animation Montage Playback**
   - Changed all `SkeletalBody->GetAnimInstance()` to `GetMesh()->GetAnimInstance()`

5. **Removed Velocity Calculation Loop**
   - Deleted the end-of-Tick velocity calculation (lines 197–204 in original)
   - CharacterMovementComponent provides `GetCharacterMovement()->Velocity`

### InitializeFromVariant() (lines 207–274)

1. **Mesh Scaling**
   - Changed `SkeletalBody->SetRelativeScale3D(...)` to `GetMesh()->SetRelativeScale3D(...)`
   - All asset loading logic remains unchanged

### ApplyRescueDamage() (lines 276–355)

1. **Hit-React Montage**
   - Changed `SkeletalBody->GetAnimInstance()` to `GetMesh()->GetAnimInstance()`

2. **Death Montage Playback**
   - Changed `SkeletalBody->GetAnimInstance()` to `GetMesh()->GetAnimInstance()`
   - All timing and delay logic unchanged

## Architecture Benefits

1. **Collision & Movement**
   - CharacterMovementComponent handles capsule movement, collision resolution, and ground friction automatically
   - Zombies can now walk up slight slopes, around obstacles, and down stairs without custom code

2. **Velocity for Animations**
   - AnimBP can read from `GetCharacterMovement()->Velocity` directly
   - No manual velocity tracking or frame-dependent calculations

3. **AI Controller Integration**
   - `AutoPossessAI` setup allows the controller to drive movement via `AddMovementInput()`
   - Prepares for Item 2 (AI state machine)

4. **Simplified Code**
   - Removed ~50 lines of velocity tracking boilerplate
   - Consistent with engine conventions (matches how `ACodeRescueCharacter` is structured)

## Verification Checklist

- [x] Header: `ACharacter` base class
- [x] Header: Removed `GetVelocity()` override
- [x] Header: Removed velocity member variables
- [x] Header: Removed SkeletalBody member
- [x] Cpp: Constructor capsule/mesh setup
- [x] Cpp: CharacterMovementComponent configuration
- [x] Cpp: AIControllerClass assignment
- [x] Cpp: BeginPlay() mesh switching via `GetMesh()`
- [x] Cpp: Tick() movement via `AddMovementInput()`
- [x] Cpp: Tick() dying state via `StopMovementImmediately()`
- [x] Cpp: Tick() montage playback via `GetMesh()`
- [x] Cpp: InitializeFromVariant() mesh scaling via `GetMesh()`
- [x] Cpp: ApplyRescueDamage() montages via `GetMesh()`
- [x] Code: No remaining references to `SkeletalBody`, `PreviousLocation`, `CachedVelocity`

## Next Steps

- Compile on macOS and resolve any link errors
- Run PIE smoke test:
  - Spawn a default zombie (procedural mesh)
  - Spawn a professional zombie (from content pack)
  - Verify movement toward player
  - Verify attack montage playback
  - Verify death sequence
- Proceed to Item 2 (AI controller state machine)

## Notes

- The inherited `GetMesh()` is a `USkeletalMeshComponent*` by default on `ACharacter`
- Procedural fallback (Body/Head) remains as static mesh overlays for visual clarity
- `Glow` (point light) attachment to capsule ensures it tracks zombie center regardless of mesh state
- `GrowlAudio` attachment to capsule ensures 3D spatial audio emits from the zombie's body center
