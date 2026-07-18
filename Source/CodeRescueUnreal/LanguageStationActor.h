#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CodeRescueTypes.h"
#include "LanguageStationActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class CODERESCUEUNREAL_API ALanguageStationActor : public AActor
{
    GENERATED_BODY()

public:
    ALanguageStationActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Language")
    ECodingLanguage Language = ECodingLanguage::Java;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Language")
    FString StationLabel = TEXT("Java Station");

    UFUNCTION(BlueprintCallable)
    void ActivateStation();

private:
    UPROPERTY()
    UStaticMeshComponent* StationMesh;

    UPROPERTY()
    UPointLightComponent* StationLight;
};
