#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CodeRescueAchievementSystem.generated.h"

/** #40 — local achievement entry. */
USTRUCT(BlueprintType)
struct FAchievementDef
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString Id;
    UPROPERTY(BlueprintReadWrite) FString DisplayName;
    UPROPERTY(BlueprintReadWrite) FString Description;
    /** Bit position in UCodeRescueSaveGame::AchievementsUnlocked (0..63). */
    UPROPERTY(BlueprintReadWrite) int32 BitIndex = 0;
};

/** #57 — pluggable achievement backend. The local implementation is the
 *  only backend that ships in v1; the Steam stub is selected when
 *  GConfig has [/Script/CodeRescueUnreal.OSS] bSteamEnabled=true. */
class CODERESCUEUNREAL_API IAchievementBackend
{
public:
    virtual ~IAchievementBackend() = default;
    virtual bool Award(class UWorld* World, const FAchievementDef& Def) = 0;
    virtual bool IsUnlocked(class UWorld* World, int32 BitIndex) = 0;
};

class CODERESCUEUNREAL_API FLocalAchievementBackend : public IAchievementBackend
{
public:
    virtual bool Award(class UWorld* World, const FAchievementDef& Def) override;
    virtual bool IsUnlocked(class UWorld* World, int32 BitIndex) override;
};

class CODERESCUEUNREAL_API FSteamAchievementBackend : public IAchievementBackend
{
public:
    virtual bool Award(class UWorld* World, const FAchievementDef& Def) override;
    virtual bool IsUnlocked(class UWorld* World, int32 BitIndex) override;
};

/**
 * UCodeRescueAchievementSystem — local-only achievement tracker.
 *
 * Holds the static achievement catalog (FAchievementDef array) and provides
 * Award(BitIndex) / IsUnlocked(BitIndex) helpers backed by the SaveGame
 * bitmap. On Award, also spawns a toast widget on the local viewport.
 *
 * No Steam / Epic / GOG integration in v1 — this is purely local.
 */
UCLASS(BlueprintType)
class CODERESCUEUNREAL_API UCodeRescueAchievementSystem : public UObject
{
    GENERATED_BODY()

public:
    static const TArray<FAchievementDef>& GetCatalog();

    /** Returns true on first-time award (false if already unlocked). */
    UFUNCTION(BlueprintCallable, Category="Achievements")
    static bool Award(class UWorld* World, int32 BitIndex);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Achievements")
    static bool IsUnlocked(class UWorld* World, int32 BitIndex);

    /** Convenience: check post-event conditions and award all that fit. Call
     *  this after every kill/rescue/solve for cheap event-driven evaluation. */
    UFUNCTION(BlueprintCallable, Category="Achievements")
    static void EvaluateAll(class UWorld* World);

private:
    static void ShowToast(class UWorld* World, const FAchievementDef& Def);
};
