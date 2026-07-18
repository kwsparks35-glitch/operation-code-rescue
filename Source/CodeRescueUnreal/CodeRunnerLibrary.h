#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CodeRescueTypes.h"
#include "CodeRunnerLibrary.generated.h"

UCLASS()
class CODERESCUEUNREAL_API UCodeRunnerLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Validation")
    static FCodeValidationResult ValidateChallenge(const FChallengeSpec& Challenge, const FString& UserCode);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Validation")
    static bool AreExternalValidatorsAllowed();

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Validation")
    static FString FindMATLABExecutable();

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Validation")
    static bool LaunchMATLABDesktop();

    /** Returns true when the external toolchain for the given language is
     *  present on this machine. Result is cached after the first call so
     *  this is cheap to query every time the terminal opens. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Validation")
    static bool IsLanguageAvailable(ECodingLanguage Language);

    /** Human-readable explanation of which command we looked for and how to
     *  install it. Returned regardless of availability so the UI can show
     *  "looking for: javac (install JDK 17+)" alongside a green check. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Validation")
    static FString GetLanguageDependencyMessage(ECodingLanguage Language);

    /** #46 — convert raw user code to RichText markup with per-language
     *  syntax highlighting. Keywords get <Keyword>...</Keyword>, strings
     *  get <Str>...</Str>, comments get <Cmt>...</Cmt>, numbers get
     *  <Num>...</Num>. Caller is responsible for setting up the matching
     *  text style set on the FRichTextBlock. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Editor")
    static FString HighlightCode(ECodingLanguage Language, const FString& UserCode);

    /** #47 — process a code-edit event: when the user just typed Enter,
     *  return the leading-whitespace prefix the editor should insert (plus
     *  4 extra spaces if the previous line ends in '{' or ':'). Returns
     *  empty string if no auto-indent is needed. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Editor")
    static FString ComputeAutoIndentForNewline(const FString& PrevLine);

    /** #47 — for a single typed character, return the matching closer if
     *  this is an open bracket. Empty string otherwise. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Editor")
    static FString GetAutoCloseFor(const FString& JustTyped);

    /** #49 — load custom challenges authored by teachers from
     *  Content/CodeRescueData/custom_challenges.json. Idempotent on Id. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Curriculum")
    static TArray<FChallengeSpec> LoadCustomChallenges();

private:
    static FCodeValidationResult ValidateJava(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir);
    static FCodeValidationResult ValidateC(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir);
    static FCodeValidationResult ValidateCpp(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir);
    static FCodeValidationResult ValidatePython(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir);
    static FCodeValidationResult ValidateMATLAB(const FChallengeSpec& Challenge, const FString& UserCode, const FString& SandboxDir);

    /** In-engine static-analysis fallback used when an external compiler is
     *  missing OR (optionally) as a last layer of defense. Pattern-matches
     *  the user's solution against known mission shapes (sum, lock, reverse,
     *  palindrome, fizzBuzz, even filtering, linked traversal, and binary
     *  search). */
    static FCodeValidationResult ValidateInEngine(const FChallengeSpec& Challenge, const FString& UserCode);

    static bool ExecProcess(const FString& Executable, const FString& Args, FString& OutStdOut, FString& OutStdErr, int32& OutReturnCode);
    static FString GetSandboxDir();
    static void AddCheck(FCodeValidationResult& Result, const FString& Label, bool bPassed);

    /** Probe the OS for a tool's presence by running it with --version (or
     *  similar) and checking for a successful exit. */
    static bool ProbeExecutable(const FString& Command, const FString& VersionArg);
};
