// CodeRescueMessageMarkerActor.h
//
// Declutters the world: instead of dumping a full paragraph as world text (which competes for the
// player's attention), each message becomes a compact, unique-ID marker hovering over its location.
// The player reads the full paragraph in a separate scrollable screen (UCodeRescueMessageReaderWidget)
// by looking at / standing near the marker and pressing the existing interact key (E / Enter / Tab / G).
//
// Interaction is wired through the project's existing pattern: the marker is tagged "MessageMarker"
// and exposes OpenMessageReader() as a UFUNCTION, which ACodeRescueCharacter::Interact() calls by name
// (same decoupled approach as the Helipad). No new input bindings required.
//
// Cook-safe (engine primitives + text only), reduced-motion aware. Authored without an on-device UE
// compile; Mac compile + playtest is the Definition-of-Done gate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CodeRescueMessageMarkerActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UPointLightComponent;

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueMessageMarkerActor : public AActor
{
    GENERATED_BODY()

public:
    ACodeRescueMessageMarkerActor();
    virtual void Tick(float DeltaSeconds) override;

    /** Configure the marker's short id, reader title/body, accent, and motion setting. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Messages")
    void ConfigureMessage(const FString& InMessageId, const FString& InTitle, const FString& InBody,
                          const FLinearColor& InAccent, bool bInReducedMotion);

    /** Called by ACodeRescueCharacter::Interact() (by name) to open the scrollable reader. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Messages")
    void OpenMessageReader();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Message") FString MessageId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Message") FString Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Message") FString Body;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Message") FLinearColor Accent = FLinearColor(0.35f, 0.85f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Message") bool bReducedMotion = false;

    /** Distance (uu) at which the "READ" prompt appears. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Message") float PromptRange = 900.0f;

private:
    UPROPERTY() USceneComponent* SceneRoot = nullptr;
    UPROPERTY() UStaticMeshComponent* Plate = nullptr;
    UPROPERTY() UTextRenderComponent* IdLabel = nullptr;
    UPROPERTY() UTextRenderComponent* ReadPrompt = nullptr;
    UPROPERTY() UPointLightComponent* Glow = nullptr;

    float MotionTime = 0.0f;

protected:
    // 2026-07-16 freeze pass: true while the player is too far for the
    // facing/bob animation to matter. Subclasses (beacons) must skip their
    // own per-frame presentation work when set — the transform propagation
    // through the TextRender children was a measurable slice of a dense
    // late-campaign city's frame.
    bool bMarkerAnimationCulled = false;

private:
    void ApplyAccent();
};
