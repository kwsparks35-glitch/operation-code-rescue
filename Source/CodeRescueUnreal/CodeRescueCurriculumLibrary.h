#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CodeRescueCurriculumLibrary.generated.h"

USTRUCT(BlueprintType)
struct FCodeRescueCurriculumEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Id;

    UPROPERTY(BlueprintReadOnly)
    FString Title;

    UPROPERTY(BlueprintReadOnly)
    FString Language;

    UPROPERTY(BlueprintReadOnly)
    FString Concept;

    UPROPERTY(BlueprintReadOnly)
    int32 Difficulty = 1;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Strategies;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Mistakes;
};

USTRUCT(BlueprintType)
struct FCodeRescueCampaignAuditEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Rank = 0;

    UPROPERTY(BlueprintReadOnly)
    FString CityName;

    UPROPERTY(BlueprintReadOnly)
    FString StateName;

    UPROPERTY(BlueprintReadOnly)
    FString Slug;

    UPROPERTY(BlueprintReadOnly)
    FString TerminalId;

    UPROPERTY(BlueprintReadOnly)
    FString TerminalTitle;

    UPROPERTY(BlueprintReadOnly)
    FString LessonKind;

    UPROPERTY(BlueprintReadOnly)
    FString CurriculumStageName;

    UPROPERTY(BlueprintReadOnly)
    FString CurriculumFocus;

    UPROPERTY(BlueprintReadOnly)
    FString MissionBrief;

    UPROPERTY(BlueprintReadOnly)
    FString RegionName;

    UPROPERTY(BlueprintReadOnly)
    FString DistrictStyle;

    UPROPERTY(BlueprintReadOnly)
    FString LandmarkName;

    UPROPERTY(BlueprintReadOnly)
    FString ArtKitName;

    UPROPERTY(BlueprintReadOnly)
    FString ArchitectureSignature;

    UPROPERTY(BlueprintReadOnly)
    FString NovelGameplayDetail;

    UPROPERTY(BlueprintReadOnly)
    FString LanguageTrackText;

    UPROPERTY(BlueprintReadOnly)
    FString LearningSupportText;

    UPROPERTY(BlueprintReadOnly)
    FString VisualDebuggerPlan;

    UPROPERTY(BlueprintReadOnly)
    FString ProgressionPlan;

    UPROPERTY(BlueprintReadOnly)
    FString CharacterStoryPlan;

    UPROPERTY(BlueprintReadOnly)
    FString GameplayFlowPlan;

    UPROPERTY(BlueprintReadOnly)
    FString AccessibilityPolishPlan;

    UPROPERTY(BlueprintReadOnly)
    FString QAVerificationPlan;

    UPROPERTY(BlueprintReadOnly)
    FString HintText;

    UPROPERTY(BlueprintReadOnly)
    FString VisibleTestBrief;

    UPROPERTY(BlueprintReadOnly)
    FString HiddenTestBrief;

    UPROPERTY(BlueprintReadOnly)
    FString RadioBriefing;

    UPROPERTY(BlueprintReadOnly)
    FString RadioVoiceName;

    UPROPERTY(BlueprintReadOnly)
    int32 DifficultyTier = 1;

    UPROPERTY(BlueprintReadOnly)
    float EncounterIntensity = 1.0f;
};

UCLASS()
class CODERESCUEUNREAL_API UCodeRescueCurriculumLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Curriculum")
    static FString GetCurriculumDatabasePath();

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Curriculum")
    static bool LoadCurriculumEntries(TArray<FCodeRescueCurriculumEntry>& OutEntries, FString& OutError);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Campaign")
    static TArray<FCodeRescueCampaignAuditEntry> GetCampaignAuditEntries();
};
