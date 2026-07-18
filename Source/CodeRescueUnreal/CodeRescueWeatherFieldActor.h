#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CodeRescueWeatherFieldActor.generated.h"

class AExponentialHeightFog;
class UInstancedStaticMeshComponent;
class USceneComponent;

UENUM()
enum class ECodeRescueWeatherPhase : uint8
{
    Wind,
    Rain,
    Fog
};

/** Runtime weather with visible and gameplay consequences. Authored rain and
 * wind instances follow the player; fog drives both atmospheric visibility
 * and AI sight range; rain applies a restrained traction change. */
UCLASS()
class CODERESCUEUNREAL_API ACodeRescueWeatherFieldActor : public AActor
{
    GENERATED_BODY()

public:
    ACodeRescueWeatherFieldActor();

    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    void ConfigureForCity(int32 InCityIndex, const FVector& InCityOrigin);
    bool RunAcceptanceAudit();
    /** Deterministic visual-QA control. Normal gameplay still advances phases
     * on its 120-second timer. */
    void SetVisualReviewPhase(ECodeRescueWeatherPhase Phase);

    ECodeRescueWeatherPhase GetActivePhase() const { return ActivePhase; }
    int32 GetRainInstanceCount() const;
    int32 GetWindDebrisInstanceCount() const;

protected:
    virtual void BeginPlay() override;

private:
    void BuildWeatherInstances();
    void SetWeatherPhase(ECodeRescueWeatherPhase NewPhase, bool bAuditTransition = false);
    void ApplyFogForPhase();
    void ApplyPlayerInfluence();
    void UpdateWeatherInstances(float DeltaSeconds);
    void FindOrCreateFogActor();
    static const TCHAR* PhaseLabel(ECodeRescueWeatherPhase Phase);

    UPROPERTY()
    USceneComponent* SceneRoot = nullptr;

    UPROPERTY()
    UInstancedStaticMeshComponent* RainInstances = nullptr;

    UPROPERTY()
    UInstancedStaticMeshComponent* WindDebrisInstances = nullptr;

    UPROPERTY()
    AExponentialHeightFog* ControlledFogActor = nullptr;

    TArray<FVector> RainLocalPositions;
    TArray<FVector> DebrisLocalPositions;
    TArray<FRotator> DebrisLocalRotations;
    FRandomStream WeatherStream;

    FVector CityOrigin = FVector::ZeroVector;
    int32 CityIndex = 0;
    ECodeRescueWeatherPhase ActivePhase = ECodeRescueWeatherPhase::Rain;
    float PhaseElapsedSeconds = 0.0f;
    float PhaseDurationSeconds = 120.0f;
    float CachedGroundFriction = 0.0f;
    float CachedBrakingDeceleration = 0.0f;
    bool bCachedMovement = false;
    bool bOwnsFogActor = false;
    bool bFogApplied = false;
    bool bInstancesBuilt = false;
};
