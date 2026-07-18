#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CodeRescueLeaderboards.generated.h"

UENUM(BlueprintType)
enum class ELeaderboardKind : uint8
{
    FastestFiveCity = 0   UMETA(DisplayName = "Fastest 5-City Clear (seconds)"),
    MostRescues     = 1   UMETA(DisplayName = "Most Rescues"),
    MostHeadshots   = 2   UMETA(DisplayName = "Most Headshots"),
    LongestNoResupply = 3 UMETA(DisplayName = "Longest No-Resupply Streak"),
};

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString PlayerName;
    UPROPERTY(BlueprintReadWrite) int64 Score = 0;
    UPROPERTY(BlueprintReadWrite) FString Timestamp;
};

/**
 * #58 — local-only leaderboards. Stored as JSON under
 * Saved/Leaderboards/<kind>.json. Top-10 lists per kind.
 *
 * Submit() inserts the score into the right list and trims to 10.
 * Fetch() returns the sorted list. UI shown from the main menu via
 * a new "Leaderboards" button (deferred — call TryShowFromMenu).
 */
UCLASS(BlueprintType)
class CODERESCUEUNREAL_API UCodeRescueLeaderboards : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Leaderboards")
    static bool Submit(ELeaderboardKind Kind, const FString& PlayerName, int64 Score);

    UFUNCTION(BlueprintCallable, Category="Leaderboards")
    static TArray<FLeaderboardEntry> Fetch(ELeaderboardKind Kind);

    UFUNCTION(BlueprintCallable, Category="Leaderboards")
    static FString PrettyPrint(ELeaderboardKind Kind);

private:
    static FString PathFor(ELeaderboardKind Kind);
};
