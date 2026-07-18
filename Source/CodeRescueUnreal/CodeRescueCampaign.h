#pragma once

#include "CoreMinimal.h"
#include "CodeRescueTypes.h"

class UCodeRescueGameInstance;

enum class ECampaignLessonKind : uint8
{
    Sum,
    Lock,
    Reverse,
    Palindrome,
    FizzBuzz,
    EvenFilter,
    LinkedListTraverse,
    BinarySearch
};

struct FCodeRescueCityMission
{
    int32 Rank = 0;
    FString CityName;
    FString StateName;
    FString Slug;
    FString TerminalId;
    FString TerminalTitle;
    FString MissionBrief;
    FString SurvivorName;
    FString RegionName;
    FString DistrictStyle;
    FString LandmarkName;
    FString ArtKitName;
    FString CurriculumFocus;
    FString CurriculumStageName;
    FString ArchitectureSignature;
    FString NovelGameplayDetail;
    FString LanguageTrackText;
    FString LearningSupportText;
    FString VisualDebuggerPlan;
    FString ProgressionPlan;
    FString CharacterStoryPlan;
    FString GameplayFlowPlan;
    FString AccessibilityPolishPlan;
    FString QAVerificationPlan;
    FString HintText;
    FString VisibleTestBrief;
    FString HiddenTestBrief;
    FString RadioBriefing;
    FString RadioVoiceName;
    ECampaignLessonKind LessonKind = ECampaignLessonKind::Sum;
    ECodingLanguage RecommendedLanguage = ECodingLanguage::Python;
    FLinearColor AccentColor = FLinearColor::White;
    FLinearColor SecondaryAccentColor = FLinearColor::White;
    int32 DifficultyTier = 1;
    float EncounterIntensity = 1.0f;
    int32 SkylineSeed = 0;
};

struct FCodeRescueSurvivorArchetypeProfile
{
    FString Title;
    FString IconLabel;
    FString FieldNeed;
    FString RescueSkill;
    FString DossierHook;
    FLinearColor AccentColor = FLinearColor(1.0f, 0.86f, 0.18f, 1.0f);
};

class FCodeRescueCampaign
{
public:
    static constexpr int32 USCityMissionCount = 342;
    static constexpr int32 CurrentCampaignMissionCount = 465;
    static constexpr int32 RequiredChallengesPerCity = 10;

    // Local-space arena contract. Values are scaled through GetCitySpanScale.
    // The outer edge retains a gap before the next streamed city while
    // enclosing every authored building, survivor, and extraction route.
    static constexpr float ArenaInnerHalfXLocal = 5250.0f;
    static constexpr float ArenaInnerHalfYLocal = 4550.0f;
    static constexpr float ArenaOuterHalfXLocal = 5500.0f;
    static constexpr float ArenaOuterHalfYLocal = 4800.0f;
    static constexpr float ArenaWallHalfXLocal = 5400.0f;
    static constexpr float ArenaWallHalfYLocal = 4700.0f;

    static const TArray<FCodeRescueCityMission>& GetMissions();
    static int32 GetMissionCount();
    static const FCodeRescueCityMission* GetMission(int32 Index);

    static float GetCitySpanScale();
    static FVector ScaleCityOffset(const FVector& Offset);
    static FVector ScaleCityExtent(const FVector& Extent);
    static FVector GetCityOrigin(int32 Index);
    static FVector GetPlayerStartLocation(int32 Index);
    static FString GetMissionLabel(int32 Index);
    static bool IsLocationInsideCityArenaXY(int32 Index, const FVector& Location, bool bUseOuterBounds = true);

    /** Ten deterministic, selected-language challenge ids for one city. The
     *  primary mission id remains stage one for old-save compatibility. */
    static TArray<FString> GetCityChallengeIds(int32 Index);
    static int32 GetCityChallengeProgress(const UCodeRescueGameInstance* GI, int32 Index);
    static bool HasCompletedCityChallengeSet(const UCodeRescueGameInstance* GI, int32 Index);
    static FString GetFirstUnsolvedCityChallengeId(const UCodeRescueGameInstance* GI, int32 Index);

    static bool IsCityCompleted(const UCodeRescueGameInstance* GI, int32 Index);
    static bool IsCityUnlocked(const UCodeRescueGameInstance* GI, int32 Index);
    static int32 GetFirstIncompleteCityIndex(const UCodeRescueGameInstance* GI);
    static FCodeRescueSurvivorArchetypeProfile GetSurvivorArchetypeProfile(const FCodeRescueCityMission& Mission);
};
