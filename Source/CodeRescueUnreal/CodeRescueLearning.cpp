// CodeRescueLearning.cpp  -- see header for design intent and the Mac-compile DoD note.

#include "CodeRescueLearning.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    FString CurriculumPath()
    {
        return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("CodeRescueData"), TEXT("curriculum_database.json"));
    }

    void ReadStringArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, TArray<FString>& Out)
    {
        Out.Reset();
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (Obj.IsValid() && Obj->TryGetArrayField(Field, Values))
        {
            for (const TSharedPtr<FJsonValue>& V : *Values)
            {
                if (V.IsValid()) { Out.Add(V->AsString()); }
            }
        }
    }

    void ReadTestArray(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, TArray<FCodeRescueTestCase>& Out)
    {
        Out.Reset();
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (Obj.IsValid() && Obj->TryGetArrayField(Field, Values))
        {
            for (const TSharedPtr<FJsonValue>& V : *Values)
            {
                const TSharedPtr<FJsonObject> TestObj = V.IsValid() ? V->AsObject() : nullptr;
                if (TestObj.IsValid())
                {
                    FCodeRescueTestCase Test;
                    TestObj->TryGetStringField(TEXT("in"), Test.In);
                    TestObj->TryGetStringField(TEXT("out"), Test.Out);
                    Out.Add(Test);
                }
            }
        }
    }

    FString LanguageKey(const FString& Language)
    {
        FString Key = Language.ToLower();
        if (Key == TEXT("c++")) { return TEXT("cpp"); }
        if (Key == TEXT("c+"))  { return TEXT("cplus"); }
        return Key; // "python", "java", "c", "matlab"
    }

    // Resolve a starter for the desired language, falling back to any available starter.
    FString ResolveStarter(const FCodeRescueChallenge& C, const FString& Language)
    {
        const FString Key = LanguageKey(Language);
        if (const FString* Found = C.Starters.Find(Key)) { return *Found; }
        if (const FString* Py = C.Starters.Find(TEXT("python"))) { return *Py; }
        for (const TPair<FString, FString>& Pair : C.Starters) { return Pair.Value; }
        return FString();
    }
}

FString UCodeRescueLearningLibrary::GetLearningTelemetryPath()
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ClaudeLearning"), TEXT("telemetry.jsonl"));
}

bool UCodeRescueLearningLibrary::LoadChallenges(TArray<FCodeRescueChallenge>& OutChallenges, FString& OutError)
{
    OutChallenges.Reset();
    OutError.Empty();

    FString JsonText;
    const FString Path = CurriculumPath();
    if (!FFileHelper::LoadFileToString(JsonText, *Path))
    {
        OutError = FString::Printf(TEXT("Could not read curriculum database at %s"), *Path);
        return false;
    }

    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("Curriculum database JSON is invalid.");
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
    if (!Root->TryGetArrayField(TEXT("entries"), Entries))
    {
        OutError = TEXT("Curriculum database is missing an 'entries' array.");
        return false;
    }

    for (const TSharedPtr<FJsonValue>& EntryValue : *Entries)
    {
        const TSharedPtr<FJsonObject> E = EntryValue.IsValid() ? EntryValue->AsObject() : nullptr;
        if (!E.IsValid()) { continue; }

        FCodeRescueChallenge C;
        E->TryGetStringField(TEXT("id"), C.Id);
        E->TryGetStringField(TEXT("title"), C.Title);
        E->TryGetStringField(TEXT("language"), C.Language);
        E->TryGetStringField(TEXT("concept"), C.Concept);
        E->TryGetStringField(TEXT("validator"), C.Validator);
        E->TryGetNumberField(TEXT("tier"), C.Tier);
        E->TryGetNumberField(TEXT("difficulty"), C.Difficulty);
        E->TryGetStringField(TEXT("micro_lesson"), C.MicroLesson);
        E->TryGetStringField(TEXT("worked_example"), C.WorkedExample);
        E->TryGetStringField(TEXT("prompt"), C.Prompt);
        E->TryGetStringField(TEXT("post_solve"), C.PostSolve);
        E->TryGetStringField(TEXT("world_effect"), C.WorldEffect);

        ReadStringArray(E, TEXT("strategies"), C.Strategies);
        ReadStringArray(E, TEXT("common_mistakes"), C.Mistakes);
        ReadStringArray(E, TEXT("misconceptions"), C.Misconceptions);
        ReadTestArray(E, TEXT("visible_tests"), C.VisibleTests);
        ReadTestArray(E, TEXT("hidden_tests"), C.HiddenTests);

        const TSharedPtr<FJsonObject>* StarterObj = nullptr;
        if (E->TryGetObjectField(TEXT("starter"), StarterObj) && StarterObj && (*StarterObj).IsValid())
        {
            for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*StarterObj)->Values)
            {
                if (Pair.Value.IsValid())
                {
                    C.Starters.Add(Pair.Key.ToLower(), Pair.Value->AsString());
                }
            }
        }

        if (C.IsValid()) { OutChallenges.Add(C); }
    }

    if (OutChallenges.Num() == 0)
    {
        OutError = TEXT("Curriculum loaded but contained no valid entries.");
        return false;
    }
    return true;
}

FCodeRescueChallenge UCodeRescueLearningLibrary::SelectChallengeForCity(const FString& Language, int32 DesiredTier, int32 CityIndex)
{
    TArray<FCodeRescueChallenge> All;
    FString Error;
    if (!LoadChallenges(All, Error) || All.Num() == 0)
    {
        return FCodeRescueChallenge();
    }

    auto PickFrom = [CityIndex](const TArray<FCodeRescueChallenge>& Pool) -> FCodeRescueChallenge
    {
        const int32 Index = ((CityIndex % Pool.Num()) + Pool.Num()) % Pool.Num();
        return Pool[Index];
    };

    // Only real challenges (carry a prompt); the 6 originals without prompts are teaching notes.
    TArray<FCodeRescueChallenge> Playable;
    for (const FCodeRescueChallenge& C : All) { if (!C.Prompt.IsEmpty()) { Playable.Add(C); } }
    if (Playable.Num() == 0) { Playable = All; }

    const FString WantLang = Language.IsEmpty() ? TEXT("Python") : Language;

    TArray<FCodeRescueChallenge> LangTier, LangAny, TierAny;
    for (const FCodeRescueChallenge& C : Playable)
    {
        const bool bLang = C.Language.Equals(WantLang, ESearchCase::IgnoreCase) || C.Language.Equals(TEXT("All"), ESearchCase::IgnoreCase);
        const bool bTier = (DesiredTier <= 0) || (C.Tier == DesiredTier);
        if (bLang && bTier) { LangTier.Add(C); }
        if (bLang)          { LangAny.Add(C); }
        if (bTier)          { TierAny.Add(C); }
    }

    if (LangTier.Num() > 0) { return PickFrom(LangTier); }
    if (LangAny.Num()  > 0) { return PickFrom(LangAny);  }
    if (TierAny.Num()  > 0) { return PickFrom(TierAny);  }
    return PickFrom(Playable);
}

FCodeRescueTeachPayload UCodeRescueLearningLibrary::BuildTeachPayload(const FCodeRescueChallenge& Challenge, const FString& Language, bool bSimplified)
{
    FCodeRescueTeachPayload Payload;
    Payload.Title = Challenge.Title;
    Payload.ConceptLine = FString::Printf(TEXT("Tier %d - %s"), Challenge.Tier, *Challenge.Concept);
    Payload.MicroLesson = Challenge.MicroLesson;
    Payload.WorkedExample = Challenge.WorkedExample;
    Payload.Prompt = Challenge.Prompt;
    Payload.StarterCode = ResolveStarter(Challenge, Language);

    if (Challenge.VisibleTests.Num() > 0)
    {
        Payload.VisibleTestLine = FString::Printf(TEXT("Visible test: %s -> %s"),
            *Challenge.VisibleTests[0].In, *Challenge.VisibleTests[0].Out);
    }

    if (bSimplified)
    {
        // "simplified hints" flag: keep the doing-parts, drop the longer worked example.
        Payload.WorkedExample.Empty();
    }
    return Payload;
}

FString UCodeRescueLearningLibrary::BuildPostSolve(const FCodeRescueChallenge& Challenge)
{
    FString Text = Challenge.PostSolve;
    if (Challenge.HiddenTests.Num() > 0)
    {
        Text += FString::Printf(TEXT("\nHidden tests now revealed (%d): your solution also had to handle edge cases like %s -> %s."),
            Challenge.HiddenTests.Num(), *Challenge.HiddenTests[0].In, *Challenge.HiddenTests[0].Out);
    }
    return Text;
}

bool UCodeRescueLearningLibrary::ShouldOfferScaffold(int32 FailedAttempts, int32 AttemptThreshold)
{
    const int32 Threshold = FMath::Max(1, AttemptThreshold);
    return FailedAttempts >= Threshold;
}

FString UCodeRescueLearningLibrary::BuildScaffold(const FCodeRescueChallenge& Challenge, const FString& Language)
{
    FString Scaffold = ResolveStarter(Challenge, Language);
    Scaffold += TEXT("\n\n// Guided path (you still learn, and the rescue still happens):");
    for (int32 i = 0; i < Challenge.Strategies.Num() && i < 3; ++i)
    {
        Scaffold += FString::Printf(TEXT("\n//   step %d: %s"), i + 1, *Challenge.Strategies[i]);
    }
    if (Challenge.Misconceptions.Num() > 0)
    {
        Scaffold += FString::Printf(TEXT("\n//   watch out: %s"), *Challenge.Misconceptions[0]);
    }
    if (Challenge.VisibleTests.Num() > 0)
    {
        Scaffold += FString::Printf(TEXT("\n//   target: %s must produce %s"),
            *Challenge.VisibleTests[0].In, *Challenge.VisibleTests[0].Out);
    }
    return Scaffold;
}

FString UCodeRescueLearningLibrary::GetWorldEffect(const FCodeRescueChallenge& Challenge)
{
    return Challenge.WorldEffect.IsEmpty()
        ? TEXT("The solved terminal reopens the rescue route.")
        : Challenge.WorldEffect;
}

bool UCodeRescueLearningLibrary::IsExternalValidationEnabled()
{
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("CodeRescue.AllowExternalCodeValidation")))
    {
        if (CVar->GetInt() != 0) { return true; }
    }
    return FParse::Param(FCommandLine::Get(), TEXT("AllowExternalCodeValidation"));
}

void UCodeRescueLearningLibrary::RecordAttempt(const FString& Concept, const FString& ChallengeId, bool bSuccess,
                                               bool bUsedHint, bool bUsedScaffold, float SecondsTaken, const FString& ErrorKind)
{
    const FString Path = GetLearningTelemetryPath();
    const FString Dir = FPaths::GetPath(Path);
    IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();
    if (!PF.DirectoryExists(*Dir))
    {
        PF.CreateDirectoryTree(*Dir);
    }

    const FString Escaped = ErrorKind.Replace(TEXT("\""), TEXT("'"));
    const FString Line = FString::Printf(
        TEXT("{\"t\":\"%s\",\"concept\":\"%s\",\"id\":\"%s\",\"success\":%s,\"hint\":%s,\"scaffold\":%s,\"seconds\":%.1f,\"error\":\"%s\"}\n"),
        *FDateTime::UtcNow().ToIso8601(),
        *Concept, *ChallengeId,
        bSuccess ? TEXT("true") : TEXT("false"),
        bUsedHint ? TEXT("true") : TEXT("false"),
        bUsedScaffold ? TEXT("true") : TEXT("false"),
        SecondsTaken, *Escaped);

    FFileHelper::SaveStringToFile(Line, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
                                  &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

FCodeRescueLearningSummary UCodeRescueLearningLibrary::SummarizeConcept(const FString& Concept)
{
    FCodeRescueLearningSummary Summary;
    Summary.Concept = Concept;

    TArray<FString> Lines;
    if (!FFileHelper::LoadFileToStringArray(Lines, *GetLearningTelemetryPath()))
    {
        return Summary;
    }

    for (const FString& Raw : Lines)
    {
        if (Raw.IsEmpty()) { continue; }
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
        TSharedPtr<FJsonObject> Obj;
        if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid()) { continue; }

        FString RowConcept;
        Obj->TryGetStringField(TEXT("concept"), RowConcept);
        if (!RowConcept.Equals(Concept, ESearchCase::IgnoreCase)) { continue; }

        Summary.Attempts++;
        bool bSuccess = false, bHint = false, bScaffold = false;
        Obj->TryGetBoolField(TEXT("success"), bSuccess);
        Obj->TryGetBoolField(TEXT("hint"), bHint);
        Obj->TryGetBoolField(TEXT("scaffold"), bScaffold);
        if (bSuccess)
        {
            Summary.Solves++;
            if (!bHint && !bScaffold) { Summary.NoHintSolves++; }
        }
        if (bScaffold) { Summary.ScaffoldUses++; }
    }

    Summary.SuccessRate = (Summary.Attempts > 0)
        ? static_cast<float>(Summary.Solves) / static_cast<float>(Summary.Attempts)
        : 0.0f;
    return Summary;
}
