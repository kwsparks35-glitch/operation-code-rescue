#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CodeRescueTypes.h"
#include "CodingTerminalActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class CODERESCUEUNREAL_API ACodingTerminalActor : public AActor
{
    GENERATED_BODY()

public:
    ACodingTerminalActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terminal")
    FChallengeSpec Challenge;

    UPROPERTY(BlueprintReadOnly)
    bool bSolved = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terminal")
    int32 CityIndex = 0;

    UFUNCTION(BlueprintCallable)
    void MarkSolved();

    void AddHelperActor(AActor* HelperActor);

private:
    void ClearHelperActors();

    UPROPERTY()
    UStaticMeshComponent* ConsoleMesh;

    UPROPERTY()
    UPointLightComponent* TerminalLight;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> HelperActors;
};
