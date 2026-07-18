#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RescueRouteGuidanceDroneActor.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class CODERESCUEUNREAL_API ARescueRouteGuidanceDroneActor : public AActor
{
    GENERATED_BODY()

public:
    ARescueRouteGuidanceDroneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance")
    FVector RouteStartWorld = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance")
    FVector RouteEndWorld = FVector(500.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance")
    FLinearColor GuidanceTint = FLinearColor(0.1f, 0.96f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance", meta=(ClampMin="0.1", ClampMax="6.0"))
    float PatrolSpeed = 0.52f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance", meta=(ClampMin="20.0", ClampMax="600.0"))
    float HoverHeight = 210.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance", meta=(ClampMin="0.0", ClampMax="160.0"))
    float BobAmplitude = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance")
    float Phase = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Route Guidance")
    bool bReducedMotion = false;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Route Guidance")
    void ConfigureDrone(const FVector& InRouteStartWorld, const FVector& InRouteEndWorld, const FLinearColor& InTint, float InPhase, bool bInReducedMotion);

private:
    void ApplyTint();
    void ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);
    void TagGuidanceComponent(UStaticMeshComponent* Component);

    UPROPERTY()
    USceneComponent* Root = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Body = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Nose = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RotorArmA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* RotorArmB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* SignalPanel = nullptr;

    UPROPERTY()
    UPointLightComponent* GuidanceLight = nullptr;

    float MotionTime = 0.0f;
};
