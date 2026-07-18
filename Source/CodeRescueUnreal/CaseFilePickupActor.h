#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CaseFilePickupActor.generated.h"

class ACodeRescueCharacter;
class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class CODERESCUEUNREAL_API ACaseFilePickupActor : public AActor
{
    GENERATED_BODY()

public:
    ACaseFilePickupActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File")
    FString CaseFileId = TEXT("case_file");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File")
    FString CaseFileTitle = TEXT("Field Case File");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File", meta=(MultiLine="true"))
    FString CaseFileBody = TEXT("A field note connecting the city, survivor, and coding lesson.");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File")
    int32 CityIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File")
    FLinearColor CaseFileTint = FLinearColor(0.35f, 0.86f, 1.0f);

    UPROPERTY(BlueprintReadOnly, Category="Code Rescue|Case File")
    bool bCollected = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File")
    bool bSnapToGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Case File", meta=(ClampMin="8.0", ClampMax="160.0"))
    float GroundClearance = 38.0f;

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Case File")
    bool Collect(ACodeRescueCharacter* Character);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                          bool bFromSweep, const FHitResult& SweepResult);

private:
    void SnapToGround();
    void ApplyVisualState();
    void SetCollectedVisualState();

    UPROPERTY()
    UStaticMeshComponent* MeshComp = nullptr;

    UPROPERTY()
    USphereComponent* TriggerComp = nullptr;

    UPROPERTY()
    UPointLightComponent* GlowComp = nullptr;
};
