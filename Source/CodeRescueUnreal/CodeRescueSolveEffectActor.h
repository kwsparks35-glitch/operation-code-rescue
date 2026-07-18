// CodeRescueSolveEffectActor.h
//
// Cook-safe "your code rescued the world" effect (intrinsic integration, World Deep-Dive 7.1).
// When a terminal is solved, the game spawns one of these at the terminal. It is parameterized by
// the SOLUTION'S OUTPUT (OutputMagnitude) -- e.g. how many units passed the player's filter -- so the
// player's actual answer drives the world response, not merely the fact that they solved it.
//
// Built from engine primitives only (no bespoke assets), reduced-motion aware, self-destroying.
// OnSolveEffectStarted is a Blueprint hook for later authored Niagara / sound / camera work.
//
// Additive runtime hook: ACodeRescueGameMode now spawns it on successful validate. Authored without an on-device UE compile; the Mac
// compile + playtest is the Definition-of-Done gate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CodeRescueSolveEffectActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UTextRenderComponent;

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueSolveEffectActor : public AActor
{
    GENERATED_BODY()

public:
    ACodeRescueSolveEffectActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** The rescue fiction the solution drives (from the challenge's world_effect). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Solve Effect")
    FString WorldEffectText;

    /** Accent color for the light + label (per language track / difficulty). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Solve Effect")
    FLinearColor AccentColor = FLinearColor(0.10f, 0.92f, 0.55f);

    /** How much the player's OUTPUT accomplished (e.g. count of units the filter returned). Drives intensity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Solve Effect")
    int32 OutputMagnitude = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Solve Effect")
    bool bReducedMotion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Solve Effect")
    float EffectLifetimeSeconds = 6.0f;

    /** Configure and start the effect. Call right after a successful validate. */
    UFUNCTION(BlueprintCallable, Category="Solve Effect")
    void ConfigureSolveEffect(const FString& InWorldEffectText, const FLinearColor& InAccentColor,
                              int32 InOutputMagnitude, bool bInReducedMotion, float InLifetimeSeconds);

    /** Blueprint hook so authored FX can replace/augment the primitive fallback without code changes. */
    UFUNCTION(BlueprintImplementableEvent, Category="Solve Effect")
    void OnSolveEffectStarted(int32 InOutputMagnitude);

private:
    UPROPERTY() USceneComponent* SceneRoot = nullptr;
    UPROPERTY() UStaticMeshComponent* BaseRing = nullptr;
    UPROPERTY() UStaticMeshComponent* RouteColumn = nullptr;
    UPROPERTY() TArray<UStaticMeshComponent*> PulseNodes;
    UPROPERTY() UPointLightComponent* EffectLight = nullptr;
    UPROPERTY() UTextRenderComponent* EffectLabel = nullptr;

    float MotionTime = 0.0f;

    void ApplyAccent();
};
