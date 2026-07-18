#pragma once

// 2026-07-11 pass 4 (environment physics): a real swinging door for the
// enterable buildings. E-interact toggles it (tag-dispatched from
// ACodeRescueCharacter::Interact like Helipad/MessageMarker); it swings
// smoothly, auto-closes after a few seconds, and BLOCKS zombies and gunfire
// while closed. Mesh: Blender SM_Door_Steel (hinge edge at the origin).

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class CODERESCUEUNREAL_API ADoorActor : public AActor
{
    GENERATED_BODY()

public:
    ADoorActor();

    virtual void Tick(float DeltaSeconds) override;

    /** Called via ProcessEvent from the character's tag-dispatch interact. */
    UFUNCTION(BlueprintCallable, Category="Door")
    void ToggleDoor();

    UFUNCTION(BlueprintCallable, Category="Door")
    bool IsOpen() const { return bOpen; }

    /** Audit support: snap the leaf fully open/closed with no swing animation
     *  (the doorway-clearance sweeps must test the OPENED doorway). */
    UFUNCTION(BlueprintCallable, Category="Door")
    void SetDoorOpenInstant(bool bNewOpen);

    /** Leaf sizing for a doorway: width/height scale on the authored 1.04 x
     *  2.06 m leaf; bMirrorSwing flips the opening direction (double doors). */
    void ConfigureLeaf(float WidthScale, float HeightScale, bool bMirrorSwing);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    USceneComponent* HingeRoot = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Leaf = nullptr;

    bool bOpen = false;
    float OpenYawDelta = 105.0f;
    float AutoCloseRemaining = 0.0f;

    /** Seconds an opened door waits before swinging shut again. */
    UPROPERTY(EditAnywhere, Category="Door")
    float AutoCloseSeconds = 8.0f;
};
