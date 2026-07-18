#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SecondaryMotionSignalActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class CODERESCUEUNREAL_API ASecondaryMotionSignalActor : public AActor
{
    GENERATED_BODY()

public:
    ASecondaryMotionSignalActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Secondary Motion", meta=(ClampMin="0.0", ClampMax="45.0"))
    float WindAmplitudeDegrees = 9.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Secondary Motion", meta=(ClampMin="0.0", ClampMax="12.0"))
    float WindSpeed = 2.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Secondary Motion")
    float FlutterPhase = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Secondary Motion")
    FLinearColor SignalTint = FLinearColor(0.1f, 0.8f, 1.0f);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Secondary Motion")
    void ConfigureSignal(const FLinearColor& InTint, float InPhase, float InAmplitudeDegrees, float InWindSpeed);

private:
    void ApplyTint();
    void ApplyComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale);

    UPROPERTY()
    USceneComponent* Root = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Mast = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Crossbar = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BannerA = nullptr;

    UPROPERTY()
    UStaticMeshComponent* BannerB = nullptr;

    UPROPERTY()
    UStaticMeshComponent* Cable = nullptr;

    float MotionTime = 0.0f;
};
