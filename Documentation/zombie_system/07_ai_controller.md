# Item 7 — Real `AAIController` + Behavior Tree

**Status:** PARTIAL — `ACodeRescueAIController` C++ class shipped as a
stub. The actual chase logic still lives in `ACodeZombieActor::Tick`
(direct linear movement). The conversion to BT-driven AI is gated on
item 2's full `AActor → ACharacter` refactor.

## What landed

`Source/CodeRescueUnreal/CodeRescueAIController.{h,cpp}` —
empty `AAIController` subclass with TODO docstrings pointing at the
follow-up plan. Compiles into the runtime module so a future Behavior
Tree asset can target it without another module change.

```cpp
UCLASS()
class CODERESCUEUNREAL_API ACodeRescueAIController : public AAIController
{
public:
    ACodeRescueAIController();
protected:
    virtual void OnPossess(APawn* InPawn) override;
};
```

## Why it's a stub

`AAIController` only possesses `APawn` subclasses. `ACodeZombieActor`
extends `AActor`, not `APawn`. Until item 2's full conversion, the
controller has nothing to possess.

## Follow-up plan once `ACodeZombieActor` is an `ACharacter`

1. **Wire the controller as default.** In `ACodeZombieActor`'s
   constructor, set
   `AIControllerClass = ACodeRescueAIController::StaticClass();` and
   `AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;`.
2. **Author a Behavior Tree.** Editor: Content Browser →
   `/Game/CodeRescueAssets/AI/` → Right-click → AI → Behavior Tree.
   Save as `BT_ZombieDefault`. Open and add:
   - Selector at the root.
   - Sequence under it: `Wait 0.2s` → `MoveToTarget` → `AttackTarget`.
   - `MoveToTarget` is `BTTask_MoveTo` configured to use a Blackboard
     vector key `TargetLocation`.
   - `AttackTarget` is a custom `BTTask_BP` that calls into
     `ACodeZombieActor::PerformAttack` (a method to extract from the
     existing Tick logic).
3. **Author a Blackboard.** Add a `BB_Zombie` blackboard with one
   `Vector` key `TargetLocation` and one `Object` key `TargetActor`.
4. **Bind in the controller.** In `OnPossess`:

   ```cpp
   void ACodeRescueAIController::OnPossess(APawn* InPawn)
   {
       Super::OnPossess(InPawn);
       if (BehaviorTreeAsset)
       {
           UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp);
           RunBehaviorTree(BehaviorTreeAsset);
           // Seed the target — for now just lock onto the local player.
           if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
           {
               BlackboardComp->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);
           }
       }
   }
   ```

5. **Add an `AIPerception` component** (sight + hearing) so target
   acquisition is data-driven rather than hard-coded "the player".
6. **Strip the chase logic out of `ACodeZombieActor::Tick`** once the BT
   is reliably driving movement.

## Files touched

- `Source/CodeRescueUnreal/CodeRescueAIController.h` (new)
- `Source/CodeRescueUnreal/CodeRescueAIController.cpp` (new)
