// CodeRescueFacialExpressionComponent.h  (2026-07-04 pass)
//
// Drives the v2 characters' facial morph targets (exported as FBX shape keys from
// Scripts/BlenderArt/build_characters_v2.py: Blink, BrowRaise, BrowAngry, Smile, Grimace,
// JawOpen, Alarm) so characters visibly REACT: solving a terminal earns a smile, taking
// damage a grimace, a horde spawn an alarmed face, plus an autonomous blink cycle.
//
// Degrades gracefully: if the owner's skeletal mesh has no morph targets (mannequin or
// zombie-pack meshes), every call is a safe no-op. Cook-safe; no assets referenced.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CodeRescueFacialExpressionComponent.generated.h"

class USkeletalMeshComponent;

USTRUCT()
struct FActiveExpression
{
    GENERATED_BODY()
    FName Morph;
    float Weight = 1.0f;
    float HoldSeconds = 2.0f;
    float Elapsed = 0.0f;
    float FadeSeconds = 0.5f;
};

UCLASS(ClassGroup=(CodeRescue), meta=(BlueprintSpawnableComponent))
class CODERESCUEUNREAL_API UCodeRescueFacialExpressionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCodeRescueFacialExpressionComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Play a named expression (morph target) at Weight for HoldSeconds, then fade out. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Face")
    void SetExpression(FName MorphName, float Weight = 1.0f, float HoldSeconds = 2.0f);

    /** Convenience: find the component on any actor and play an expression. Safe no-op. */
    static void TriggerOnActor(AActor* Actor, FName MorphName, float Weight = 1.0f, float HoldSeconds = 2.0f);

    /** Autonomous blinking (disable for zombies if desired). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Face")
    bool bAutoBlink = true;

private:
    UPROPERTY() TArray<USkeletalMeshComponent*> Faces;

    TArray<FActiveExpression> Active;
    float NextBlinkIn = 3.0f;
    float BlinkPhase = -1.0f;   // <0 idle, otherwise 0..BlinkDuration
    static constexpr float BlinkDuration = 0.22f;

    void RefreshFaces();
    void ApplyMorph(FName Morph, float Weight);
};
