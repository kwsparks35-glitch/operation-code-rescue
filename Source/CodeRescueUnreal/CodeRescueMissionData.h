#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CodeRescueTypes.h"
#include "CodeRescueMissionData.generated.h"

USTRUCT(BlueprintType)
struct FCodeRescueObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredCount = 1;
};

USTRUCT(BlueprintType)
struct FCodeRescueCurriculumNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ConceptId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECodingLanguage Language = ECodingLanguage::Java;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Difficulty = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> StrategyTips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CommonMistakes;
};

UCLASS(BlueprintType)
class CODERESCUEUNREAL_API UCodeRescueMissionData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mission")
    FString MissionId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mission")
    FString RealWorldInspiredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mission")
    FString NarrativeBriefing;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mission")
    TArray<FCodeRescueObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Curriculum")
    TArray<FCodeRescueCurriculumNode> CurriculumNodes;
};
