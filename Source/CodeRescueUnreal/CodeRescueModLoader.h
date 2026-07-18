#pragma once

#include "CoreMinimal.h"
#include "CodeRescueTypes.h"
#include "UObject/NoExportTypes.h"
#include "CodeRescueModLoader.generated.h"

USTRUCT(BlueprintType)
struct FCodeRescueModManifest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString Name;
    UPROPERTY(BlueprintReadWrite) FString Author;
    UPROPERTY(BlueprintReadWrite) FString Version;
    UPROPERTY(BlueprintReadWrite) FString Description;
    UPROPERTY(BlueprintReadWrite) TArray<FChallengeSpec> CustomChallenges;
};

/**
 * #60 — local mod loader. Scans Saved/Mods/<modname>/manifest.json on
 * startup and merges declarative content (custom challenges, future
 * variant rows, future radio briefings) into the live game pools.
 *
 * Pure data merge — no code execution. This is the surface a future
 * Steam Workshop integration would target.
 */
UCLASS(BlueprintType)
class CODERESCUEUNREAL_API UCodeRescueModLoader : public UObject
{
    GENERATED_BODY()

public:
    /** Walk Saved/Mods/ and load every manifest.json found.
     *  Returns the parsed manifests for further use. */
    UFUNCTION(BlueprintCallable, Category="Mods")
    static TArray<FCodeRescueModManifest> LoadAllMods();

    /** Convenience: pretty-print all mod summaries. */
    UFUNCTION(BlueprintCallable, Category="Mods")
    static FString PrettyPrintLoadedMods();

private:
    static bool ParseManifest(const FString& JsonPath, FCodeRescueModManifest& Out);
};
