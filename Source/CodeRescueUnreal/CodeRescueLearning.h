// CodeRescueLearning.h
//
// Self-contained "learning vertical" API for Operation Code Rescue.
//
// Design intent (see Documentation/improvement_pass_2026-07-01/ and CLAUDE_OVERSIGHT/):
//   * Additive first — the runtime terminal/GameMode wiring calls into this module while preserving
//     the legacy challenge flow as fallback if the data pack is unavailable.
//   * Data-driven — challenges come from Content/CodeRescueData/curriculum_database.json (the
//     expanded concept graph), not from a hard-coded 8-value enum.
//   * Teach -> apply -> reinforce — BuildTeachPayload() fronts each terminal with a micro-lesson
//     and worked example; BuildPostSolve() explains why it worked.
//   * Never wall-block — ShouldOfferScaffold()/BuildScaffold() give a guided path after N attempts.
//   * Intrinsic integration — GetWorldEffect() returns the fiction the player's OUTPUT drives.
//   * Measurable — RecordAttempt()/SummarizeConcept() write learning telemetry to Saved/.
//
// NOTE: authored without an on-device UE compile. Mac compile + playtest is the Definition-of-Done
// gate (see GOVERNANCE_PROTOCOL.md). Kept deliberately idiomatic and conservative.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CodeRescueLearning.generated.h"

USTRUCT(BlueprintType)
struct FCodeRescueTestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString In;
    UPROPERTY(BlueprintReadOnly) FString Out;
};

/** One data-driven coding challenge from the curriculum concept graph. */
USTRUCT(BlueprintType)
struct FCodeRescueChallenge
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString Id;
    UPROPERTY(BlueprintReadOnly) FString Title;
    UPROPERTY(BlueprintReadOnly) FString Language;     // primary display language (e.g. "Python")
    UPROPERTY(BlueprintReadOnly) FString Concept;
    UPROPERTY(BlueprintReadOnly) FString Validator;    // runtime validator family (e.g. "even_filter")
    UPROPERTY(BlueprintReadOnly) int32   Tier = 1;
    UPROPERTY(BlueprintReadOnly) int32   Difficulty = 1;

    UPROPERTY(BlueprintReadOnly) FString MicroLesson;
    UPROPERTY(BlueprintReadOnly) FString WorkedExample;
    UPROPERTY(BlueprintReadOnly) FString Prompt;
    UPROPERTY(BlueprintReadOnly) FString PostSolve;
    UPROPERTY(BlueprintReadOnly) FString WorldEffect;

    /** language key (lowercase, e.g. "python","java") -> starter code */
    UPROPERTY(BlueprintReadOnly) TMap<FString, FString> Starters;

    UPROPERTY(BlueprintReadOnly) TArray<FCodeRescueTestCase> VisibleTests;
    UPROPERTY(BlueprintReadOnly) TArray<FCodeRescueTestCase> HiddenTests;
    UPROPERTY(BlueprintReadOnly) TArray<FString> Misconceptions;
    UPROPERTY(BlueprintReadOnly) TArray<FString> Strategies;
    UPROPERTY(BlueprintReadOnly) TArray<FString> Mistakes;

    bool IsValid() const { return !Id.IsEmpty(); }
};

/** Everything a terminal needs to TEACH before it tests. */
USTRUCT(BlueprintType)
struct FCodeRescueTeachPayload
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString Title;
    UPROPERTY(BlueprintReadOnly) FString ConceptLine;     // "Tier 4 - arrays / filtering"
    UPROPERTY(BlueprintReadOnly) FString MicroLesson;
    UPROPERTY(BlueprintReadOnly) FString WorkedExample;
    UPROPERTY(BlueprintReadOnly) FString Prompt;
    UPROPERTY(BlueprintReadOnly) FString StarterCode;     // resolved for the requested language
    UPROPERTY(BlueprintReadOnly) FString VisibleTestLine; // human-readable first visible test
};

/** Rolling learning summary for one concept (from telemetry). */
USTRUCT(BlueprintType)
struct FCodeRescueLearningSummary
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString Concept;
    UPROPERTY(BlueprintReadOnly) int32 Attempts = 0;
    UPROPERTY(BlueprintReadOnly) int32 Solves = 0;
    UPROPERTY(BlueprintReadOnly) int32 NoHintSolves = 0;
    UPROPERTY(BlueprintReadOnly) int32 ScaffoldUses = 0;
    UPROPERTY(BlueprintReadOnly) float SuccessRate = 0.0f;
};

UCLASS()
class CODERESCUEUNREAL_API UCodeRescueLearningLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Load all rich challenges from the curriculum concept graph. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static bool LoadChallenges(TArray<FCodeRescueChallenge>& OutChallenges, FString& OutError);

    /**
     * Pick a challenge for a city. Prefers the requested language and the given difficulty tier
     * (1..10), then relaxes: same language any tier -> any language same tier -> first available.
     * Deterministic by CityIndex so a city always shows the same challenge.
     */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FCodeRescueChallenge SelectChallengeForCity(const FString& Language, int32 DesiredTier, int32 CityIndex);

    /** Build the teach-then-apply payload; bSimplified trims text for the "simplified hints" flag. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FCodeRescueTeachPayload BuildTeachPayload(const FCodeRescueChallenge& Challenge, const FString& Language, bool bSimplified);

    /** Post-solve explanation ("why it works") plus the hidden-test reveal. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FString BuildPostSolve(const FCodeRescueChallenge& Challenge);

    /** Never wall-block: after AttemptThreshold failures, offer a guided scaffold. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static bool ShouldOfferScaffold(int32 FailedAttempts, int32 AttemptThreshold);

    /** A guided, fill-in-the-blank version derived from the starter + a misconception nudge. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FString BuildScaffold(const FCodeRescueChallenge& Challenge, const FString& Language);

    /** The in-world fiction the player's OUTPUT drives (intrinsic integration). */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FString GetWorldEffect(const FCodeRescueChallenge& Challenge);

    /** True if the real external compiler/interpreter path is enabled (mirrors the runner CVar). */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static bool IsExternalValidationEnabled();

    /** Append one attempt to the learning telemetry log (Saved/ClaudeLearning/telemetry.jsonl). */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static void RecordAttempt(const FString& Concept, const FString& ChallengeId, bool bSuccess,
                              bool bUsedHint, bool bUsedScaffold, float SecondsTaken, const FString& ErrorKind);

    /** Roll up telemetry for one concept into a summary (attempts, solves, no-hint, success rate). */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FCodeRescueLearningSummary SummarizeConcept(const FString& Concept);

    /** Absolute path to the telemetry log. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Learning")
    static FString GetLearningTelemetryPath();
};
