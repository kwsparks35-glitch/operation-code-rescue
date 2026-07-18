// CodeRescueBeaconMarkerActor.h  (2026-07-04 pass)
//
// Kenny's directive: "correct anything longer than a single word to a beaming symbol that sits
// above the thing it is intending to draw attention toward." This actor IS that beaming symbol:
// a vertical light beam + one large category glyph, color-coded by what it marks (supply "+",
// survivor "++", threat "!", terminal "</>", etc.).
//
// It subclasses ACodeRescueMessageMarkerActor so every beacon keeps the proven read-on-demand
// contract: tag "MessageMarker" + OpenMessageReader() means the player can still press E near the
// beacon to read the full original text in the scrollable reader. Zero changes to the character's
// Interact() dispatch. Cook-safe (engine primitives only), reduced-motion aware.

#pragma once

#include "CoreMinimal.h"
#include "CodeRescueMessageMarkerActor.h"
#include "CodeRescueBeaconMarkerActor.generated.h"

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueBeaconMarkerActor : public ACodeRescueMessageMarkerActor
{
    GENERATED_BODY()

public:
    ACodeRescueBeaconMarkerActor();
    virtual void Tick(float DeltaSeconds) override;

    /** Set the single-glyph symbol ("+", "!", "</>", ...) and the beam accent color. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Messages")
    void ConfigureBeacon(const FString& InGlyph, const FLinearColor& InAccent);

    /** Beam height in uu (beam is centered on the actor; half extends down toward the target). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beacon") float BeamHeight = 900.0f;

private:
    UPROPERTY() UStaticMeshComponent* Beam = nullptr;
    UPROPERTY() UTextRenderComponent* Glyph = nullptr;

    float PulseTime = 0.0f;
};
