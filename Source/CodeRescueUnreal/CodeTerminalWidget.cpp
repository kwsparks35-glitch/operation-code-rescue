#include "CodeTerminalWidget.h"
#include "CodingTerminalActor.h"
#include "CodeRescueFacialExpressionComponent.h"
#include "CodeRescueCampaign.h"
#include "CodeRunnerLibrary.h"
#include "CodeRescueLearning.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueGameMode.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueUITheme.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/BackgroundBlur.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Algo/Reverse.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

static FString GetTerminalLanguageLabel(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java: return TEXT("Java");
    case ECodingLanguage::C: return TEXT("C");
    case ECodingLanguage::CPlus: return TEXT("C+");
    case ECodingLanguage::Cpp: return TEXT("C++");
    case ECodingLanguage::Python: return TEXT("Python");
    case ECodingLanguage::MATLAB: return TEXT("MATLAB");
    default: return TEXT("Unknown");
    }
}

static FString GetLearningStarterKeyForLanguage(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java: return TEXT("java");
    case ECodingLanguage::C: return TEXT("c");
    case ECodingLanguage::CPlus: return TEXT("cplus");
    case ECodingLanguage::Cpp: return TEXT("cpp");
    case ECodingLanguage::Python: return TEXT("python");
    case ECodingLanguage::MATLAB: return TEXT("matlab");
    default: return TEXT("");
    }
}

static FString NormalizeDataDrivenValidatorKey(const FString& RawKey)
{
    FString Key = RawKey.ToLower();
    Key.TrimStartAndEndInline();
    Key.ReplaceInline(TEXT("-"), TEXT("_"));
    Key.ReplaceInline(TEXT(" "), TEXT("_"));
    return Key;
}

static FString GetTerminalValidatorKey(const FString& ChallengeId)
{
    const FString Id = NormalizeDataDrivenValidatorKey(ChallengeId);
    if (Id.Contains(TEXT("filter")) || Id.Contains(TEXT("even")))
    {
        return TEXT("even_filter");
    }
    if (Id.Contains(TEXT("lock")))
    {
        return TEXT("boolean_lock");
    }
    if (Id.Contains(TEXT("reverse")))
    {
        return TEXT("reverse_string");
    }
    return FString();
}

static bool IsDataDrivenTerminalChallenge(const FString& ChallengeId)
{
    // 2026-07-04 R2: EVERY standard terminal now teaches from the curriculum
    // concept graph (previously only the three archetypes with curated
    // validator keys — 3 of 10 nodes). The generic validation layer in
    // CodeRunnerLibrary executes/structurally-checks any entry's declared tests.
    return !ChallengeId.IsEmpty();
}

static FString GetLearningChallengeValidatorKey(const FCodeRescueChallenge& Challenge)
{
    const FString DeclaredKey = NormalizeDataDrivenValidatorKey(Challenge.Validator);
    if (!DeclaredKey.IsEmpty())
    {
        return DeclaredKey;
    }

    const FString Id = NormalizeDataDrivenValidatorKey(Challenge.Id);
    const FString Concept = NormalizeDataDrivenValidatorKey(Challenge.Concept);
    if (Challenge.Tier == 4
        && (Id.Contains(TEXT("evac_even_order"))
            || Id.Contains(TEXT("filter"))
            || Id.Contains(TEXT("even"))
            || Concept.Contains(TEXT("filter"))))
    {
        return TEXT("even_filter");
    }
    if (Challenge.Tier == 2
        && (Id.Contains(TEXT("airlock"))
            || Id.Contains(TEXT("lock"))
            || Concept.Contains(TEXT("boolean"))))
    {
        return TEXT("boolean_lock");
    }
    if (Challenge.Tier == 5
        && (Id.Contains(TEXT("reverse"))
            || Concept.Contains(TEXT("strings_and_indexing"))))
    {
        return TEXT("reverse_string");
    }
    return FString();
}

static bool IsCompatibleLearningNodeForTerminal(const FCodeRescueChallenge& Challenge, const FString& ChallengeId)
{
    const FString TerminalKey = GetTerminalValidatorKey(ChallengeId);
    if (!TerminalKey.IsEmpty())
    {
        // Curated archetypes keep their exact concept mapping.
        return TerminalKey == GetLearningChallengeValidatorKey(Challenge);
    }
    // 2026-07-04 R2: all other terminals draw from the FULL concept graph —
    // any curriculum entry that declares test cases is eligible; the tier
    // ladder in SelectDataDrivenChallengeForTerminal orders the progression.
    return (Challenge.VisibleTests.Num() + Challenge.HiddenTests.Num()) > 0;
}

static bool LearningChallengeSupportsLanguage(const FCodeRescueChallenge& Challenge, ECodingLanguage Language)
{
    const FString LanguageLabel = GetTerminalLanguageLabel(Language);
    const bool bLanguageMatch = Challenge.Language.Equals(TEXT("All"), ESearchCase::IgnoreCase)
        || Challenge.Language.Equals(LanguageLabel, ESearchCase::IgnoreCase);
    return bLanguageMatch && Challenge.Starters.Contains(GetLearningStarterKeyForLanguage(Language));
}

static bool IsDataDrivenStarterSignatureCompatible(
    const FString& StarterCode,
    const FString& ChallengeId,
    ECodingLanguage Language)
{
    if (!IsDataDrivenTerminalChallenge(ChallengeId) || StarterCode.IsEmpty())
    {
        return false;
    }

    const FString ValidatorKey = GetTerminalValidatorKey(ChallengeId);
    if (ValidatorKey == TEXT("even_filter"))
    {
        switch (Language)
        {
        case ECodingLanguage::Java:
        case ECodingLanguage::C:
        case ECodingLanguage::CPlus:
        case ECodingLanguage::Cpp:
            return StarterCode.Contains(TEXT("evenNumbers"));
        case ECodingLanguage::Python:
        case ECodingLanguage::MATLAB:
            return StarterCode.Contains(TEXT("even_numbers")) || StarterCode.Contains(TEXT("evens"));
        default:
            return false;
        }
    }

    if (ValidatorKey == TEXT("boolean_lock"))
    {
        switch (Language)
        {
        case ECodingLanguage::Java:
        case ECodingLanguage::C:
        case ECodingLanguage::CPlus:
        case ECodingLanguage::Cpp:
            return StarterCode.Contains(TEXT("shouldUnlock"));
        case ECodingLanguage::Python:
        case ECodingLanguage::MATLAB:
            return StarterCode.Contains(TEXT("should_unlock")) || StarterCode.Contains(TEXT("shouldUnlock"));
        default:
            return false;
        }
    }

    if (ValidatorKey == TEXT("reverse_string"))
    {
        switch (Language)
        {
        case ECodingLanguage::Java:
        case ECodingLanguage::C:
        case ECodingLanguage::CPlus:
        case ECodingLanguage::Cpp:
            return StarterCode.Contains(TEXT("reverseString"));
        case ECodingLanguage::Python:
        case ECodingLanguage::MATLAB:
            return StarterCode.Contains(TEXT("reverse_string")) || StarterCode.Contains(TEXT("reverseString"));
        default:
            return false;
        }
    }

    // 2026-07-04 R2: entries without a curated validator key are generically
    // validated from their own starter — any non-empty starter is signature-
    // compatible by construction (the harness derives the name from it).
    return !StarterCode.IsEmpty();
}

static FCodeRescueChallenge SelectDataDrivenChallengeForTerminal(
    const ACodingTerminalActor* TerminalActor,
    ECodingLanguage Language)
{
    if (!TerminalActor || !IsDataDrivenTerminalChallenge(TerminalActor->Challenge.Id))
    {
        return FCodeRescueChallenge();
    }

    const FString TerminalId = NormalizeDataDrivenValidatorKey(TerminalActor->Challenge.Id);
    const bool bUsesCanonicalBuiltInHarness =
        TerminalId.Contains(TEXT("sum"))
        || TerminalId.Contains(TEXT("generator"))
        || TerminalId.Contains(TEXT("palindrome"))
        || TerminalId.Contains(TEXT("fizzbuzz"))
        || TerminalId.Contains(TEXT("linkedlist"))
        || TerminalId.Contains(TEXT("linked_list"))
        || TerminalId.Contains(TEXT("traverse"))
        || TerminalId.Contains(TEXT("binary_search"))
        || TerminalId.Contains(TEXT("binarysearch"))
        || TerminalId.Contains(TEXT("bsearch"));
    if (bUsesCanonicalBuiltInHarness)
    {
        // These shapes have complete six-language executable harnesses in
        // CodeRunnerLibrary. Do not replace their starter/signature with an
        // unrelated generic curriculum node selected only by city tier.
        return FCodeRescueChallenge();
    }

    TArray<FCodeRescueChallenge> AllChallenges;
    FString LoadError;
    if (!UCodeRescueLearningLibrary::LoadChallenges(AllChallenges, LoadError))
    {
        return FCodeRescueChallenge();
    }

    TArray<FCodeRescueChallenge> Compatible;
    for (const FCodeRescueChallenge& Candidate : AllChallenges)
    {
        if (Candidate.Prompt.IsEmpty()
            || !IsCompatibleLearningNodeForTerminal(Candidate, TerminalActor->Challenge.Id)
            || !LearningChallengeSupportsLanguage(Candidate, Language))
        {
            continue;
        }
        Compatible.Add(Candidate);
    }

    if (Compatible.Num() == 0)
    {
        return FCodeRescueChallenge();
    }

    // 2026-07-04 R2: tier ladder — early cities teach low tiers, later cities
    // climb the concept graph, wrapping so long campaigns keep rotating variety.
    Compatible.Sort([](const FCodeRescueChallenge& A, const FCodeRescueChallenge& B)
    {
        if (A.Tier != B.Tier)
        {
            return A.Tier < B.Tier;
        }
        return A.Id < B.Id;
    });

    // 2026-07-04 (top-50 item 29): SPACED REPETITION — every third city interleaves a
    // review of the player's weakest concept (success < 50% across >= 2 attempts)
    // instead of marching the ladder, so shaky ideas resurface before the campaign
    // moves on. Falls through to the ladder when nothing qualifies.
    if (TerminalActor->CityIndex % 3 == 2)
    {
        if (const UCodeRescueGameInstance* GI = TerminalActor->GetGameInstance<UCodeRescueGameInstance>())
        {
            FString WeakConcept;
            float WorstRate = 0.5f;
            for (const FCodeRescueConceptProgress& Progress : GI->ConceptProgress)
            {
                const int32 Attempts = Progress.SuccessfulValidations + Progress.FailedValidations;
                if (Attempts >= 2)
                {
                    const float Rate = static_cast<float>(Progress.SuccessfulValidations) / static_cast<float>(Attempts);
                    if (Rate < WorstRate)
                    {
                        WorstRate = Rate;
                        WeakConcept = Progress.ConceptId;
                    }
                }
            }
            if (!WeakConcept.IsEmpty())
            {
                for (const FCodeRescueChallenge& Candidate : Compatible)
                {
                    if (Candidate.Concept.Equals(WeakConcept, ESearchCase::IgnoreCase)
                        || Candidate.Concept.Contains(WeakConcept)
                        || WeakConcept.Contains(Candidate.Concept))
                    {
                        UE_LOG(LogTemp, Warning,
                            TEXT("[SpacedReview] city %d reviews weak concept '%s' (%.0f%% success)"),
                            TerminalActor->CityIndex, *WeakConcept, WorstRate * 100.0f);
                        return Candidate;
                    }
                }
            }
        }
    }

    const int32 Progress = FMath::Max(0, TerminalActor->CityIndex) / 2;   // ~2 cities per concept step
    const int32 Index = Progress % Compatible.Num();
    return Compatible[Index];
}

static FChallengeSpec BuildDataDrivenRuntimeChallengeForValidation(
    const FChallengeSpec& TerminalChallenge,
    const FCodeRescueChallenge& LearningChallenge)
{
    FChallengeSpec RuntimeChallenge = TerminalChallenge;
    if (!LearningChallenge.IsValid())
    {
        return RuntimeChallenge;
    }

    RuntimeChallenge.Id = LearningChallenge.Id;
    RuntimeChallenge.Title = LearningChallenge.Title;
    RuntimeChallenge.MissionBrief = LearningChallenge.Prompt;

    const FString StarterKey = GetLearningStarterKeyForLanguage(TerminalChallenge.Language);
    if (const FString* Starter = LearningChallenge.Starters.Find(StarterKey))
    {
        RuntimeChallenge.StarterCode = *Starter;
    }

    RuntimeChallenge.TestCases.Reset();
    auto AddTestCases = [&RuntimeChallenge](const TArray<FCodeRescueTestCase>& Tests, const TCHAR* Prefix)
    {
        for (int32 Index = 0; Index < Tests.Num(); ++Index)
        {
            FChallengeTestCase TestCase;
            TestCase.Label = FString::Printf(TEXT("%s %d"), Prefix, Index + 1);
            TestCase.Input = Tests[Index].In;
            TestCase.ExpectedOutput = Tests[Index].Out;
            RuntimeChallenge.TestCases.Add(TestCase);
        }
    };
    AddTestCases(LearningChallenge.VisibleTests, TEXT("visible"));
    AddTestCases(LearningChallenge.HiddenTests, TEXT("hidden"));
    return RuntimeChallenge;
}

static FString BuildDataDrivenValidationOracleSummary(const FCodeRescueChallenge& LearningChallenge)
{
    if (!LearningChallenge.IsValid())
    {
        return FString();
    }

    FString Text = FString::Printf(
        TEXT("\n\nDATA-DRIVEN VALIDATION PACK\nValidator key: %s\nDeclared visible tests:"),
        *GetLearningChallengeValidatorKey(LearningChallenge));

    if (LearningChallenge.VisibleTests.Num() == 0)
    {
        Text += TEXT("\n[NONE DECLARED]");
    }
    for (int32 Index = 0; Index < LearningChallenge.VisibleTests.Num(); ++Index)
    {
        Text += FString::Printf(
            TEXT("\n[%d] input %s -> expected %s"),
            Index + 1,
            *LearningChallenge.VisibleTests[Index].In,
            *LearningChallenge.VisibleTests[Index].Out);
    }

    Text += TEXT("\nDeclared hidden tests:");
    if (LearningChallenge.HiddenTests.Num() == 0)
    {
        Text += TEXT("\n[NONE DECLARED]");
    }
    for (int32 Index = 0; Index < LearningChallenge.HiddenTests.Num(); ++Index)
    {
        Text += FString::Printf(
            TEXT("\n[%d] input %s -> expected %s"),
            Index + 1,
            *LearningChallenge.HiddenTests[Index].In,
            *LearningChallenge.HiddenTests[Index].Out);
    }

    const FString ValidationMode = UCodeRescueLearningLibrary::IsExternalValidationEnabled()
        ? TEXT("External validation EXECUTES this challenge's declared test cases (2026-07-04: generic harness — every curriculum node, all languages).")
        : TEXT("External validation is off, so the in-engine fallback verifies the solution's structure against this challenge's own starter and reports the oracle for review.");
    Text += TEXT("\nExecution: ") + ValidationMode;
    return Text;
}

static FString BuildDataDrivenTeachApplySummary(
    const ACodingTerminalActor* TerminalActor,
    const UCodeRescueGameInstance* GI)
{
    if (!TerminalActor)
    {
        return FString();
    }

    const ECodingLanguage Language = TerminalActor->Challenge.Language;
    const FCodeRescueChallenge LearningChallenge = SelectDataDrivenChallengeForTerminal(TerminalActor, Language);
    if (!LearningChallenge.IsValid())
    {
        return FString();
    }

    const FString LanguageLabel = GetTerminalLanguageLabel(Language);
    const FCodeRescueTeachPayload TeachPayload = UCodeRescueLearningLibrary::BuildTeachPayload(
        LearningChallenge,
        LanguageLabel,
        GI && GI->bSimplifiedInputHints);
    const FCodeRescueLearningSummary Summary = UCodeRescueLearningLibrary::SummarizeConcept(LearningChallenge.Concept);
    const FString ValidationMode = UCodeRescueLearningLibrary::IsExternalValidationEnabled()
        ? TEXT("external compiler/interpreter validation can run on this trusted build")
        : TEXT("safe in-engine validation is active; external toolchains remain opt-in");

    FString Text = FString::Printf(
        TEXT("DATA-DRIVEN LESSON NODE\n%s | %s\nValidator: %s\nMicro-lesson: %s\nApply: %s\n%s\nWorld effect: %s\nLearning telemetry: %d attempts, %d solves, %.0f%% success for this concept.\nValidation: %s\nScaffold rule: after 3 repair attempts, a guided scaffold appears without changing the selected-language save contract."),
        *TeachPayload.ConceptLine,
        *TeachPayload.Title,
        *GetLearningChallengeValidatorKey(LearningChallenge),
        *TeachPayload.MicroLesson,
        *TeachPayload.Prompt,
        *TeachPayload.VisibleTestLine,
        *UCodeRescueLearningLibrary::GetWorldEffect(LearningChallenge),
        Summary.Attempts,
        Summary.Solves,
        Summary.SuccessRate * 100.0f,
        *ValidationMode);

    // 2026-07-04 (top-50 item 31): worked-example FADING — scaffolding recedes as this
    // concept's solve count grows (full example -> partial cue -> recall prompt), per
    // the worked-example-effect literature. Counts come from saved concept progress.
    int32 ConceptSolves = 0;
    if (GI)
    {
        for (const FCodeRescueConceptProgress& Progress : GI->ConceptProgress)
        {
            if (Progress.ConceptId.Equals(LearningChallenge.Concept, ESearchCase::IgnoreCase))
            {
                ConceptSolves = Progress.SuccessfulValidations;
                break;
            }
        }
    }
    if (!TeachPayload.WorkedExample.IsEmpty())
    {
        if (ConceptSolves <= 0)
        {
            Text += TEXT("\nWorked example: ") + TeachPayload.WorkedExample;
        }
        else if (ConceptSolves == 1)
        {
            FString Cue = TeachPayload.WorkedExample;
            int32 ArrowIdx = INDEX_NONE;
            if (Cue.FindChar(TEXT('>'), ArrowIdx) && ArrowIdx > 2)
            {
                Cue = Cue.Left(ArrowIdx + 1) + TEXT(" ...");
            }
            Text += TEXT("\nFading scaffold (1 solve in this concept): ") + Cue
                  + TEXT("  - finish the trace yourself before coding.");
        }
        else
        {
            Text += FString::Printf(
                TEXT("\nScaffold faded (%d solves in this concept): recall the worked pattern from memory - say the first step aloud, then code it."),
                ConceptSolves);
        }
    }
    return Text;
}

static const FCodeRescueCityMission* FindTerminalMissionForIntel(int32 CityIndex, const FString& TerminalId)
{
    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    if (Missions.IsValidIndex(CityIndex))
    {
        return &Missions[CityIndex];
    }

    for (const FCodeRescueCityMission& Mission : Missions)
    {
        if (Mission.TerminalId == TerminalId)
        {
            return &Mission;
        }
    }
    return nullptr;
}

static FString BuildSurvivorIntelArchiveText(
    const FCodeRescueCityMission* Mission,
    const FString& ChallengeId,
    ECodingLanguage Language,
    int32 Score,
    bool bAssisted,
    int32 CompletedChallenges)
{
    const FString LanguageLabel = GetTerminalLanguageLabel(Language);
    const bool bClearanceComplete = CompletedChallenges >= FCodeRescueCampaign::RequiredChallengesPerCity;
    if (!Mission)
    {
        return FString::Printf(
            TEXT("Coding station uploaded on the %s track. Terminal %s saved score %d; city clearance is %d/%d."),
            *LanguageLabel,
            *ChallengeId,
            Score,
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity);
    }

    const FCodeRescueSurvivorArchetypeProfile Archetype = FCodeRescueCampaign::GetSurvivorArchetypeProfile(*Mission);
    return FString::Printf(
        TEXT("Intel upload: %s for %s | Coding clearance %d/%d | %s.\nContact role: %s [%s] | Need: %s | Rescue value: %s\nLocation: %s / %s | Lesson payoff: %s\nValidation: %s score %d on %s; resume this language slot to keep progress intact.\nNext step: %s"),
        bAssisted ? TEXT("assisted field repair") : TEXT("clean terminal solve"),
        *Mission->SurvivorName,
        CompletedChallenges,
        FCodeRescueCampaign::RequiredChallengesPerCity,
        bClearanceComplete ? TEXT("survivor route open") : TEXT("survivor route remains locked"),
        *Archetype.Title,
        *Archetype.IconLabel,
        *Archetype.FieldNeed,
        *Archetype.RescueSkill,
        *Mission->LandmarkName,
        *Mission->DistrictStyle,
        *Mission->NovelGameplayDetail,
        bAssisted ? TEXT("bypass-kit") : TEXT("validator"),
        Score,
        *LanguageLabel,
        bClearanceComplete
            ? TEXT("follow the survivor marker, then return to extraction/debrief")
            : TEXT("complete the next protected coding station"));
}

static void RecordSurvivorIntelArchiveForTerminal(
    UCodeRescueGameInstance* GI,
    int32 CityIndex,
    const FString& ChallengeId,
    ECodingLanguage Language,
    int32 Score,
    bool bAssisted)
{
    if (!GI || ChallengeId.IsEmpty())
    {
        return;
    }

    const FCodeRescueCityMission* Mission = FindTerminalMissionForIntel(CityIndex, ChallengeId);
    const FString CityLabel = Mission
        ? FString::Printf(TEXT("%03d %s, %s"), Mission->Rank, *Mission->CityName, *Mission->StateName)
        : FString::Printf(TEXT("City route %d"), FMath::Max(1, CityIndex + 1));
    const FString SurvivorName = Mission
        ? Mission->SurvivorName
        : FString::Printf(TEXT("Survivor route %d"), FMath::Max(1, CityIndex + 1));
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    const bool bClearanceComplete = CompletedChallenges >= FCodeRescueCampaign::RequiredChallengesPerCity;

    GI->RecordSurvivorIntelDossier(
        ChallengeId,
        SurvivorName,
        CityLabel,
        GetTerminalLanguageLabel(Language),
        bClearanceComplete
            ? TEXT("ROUTE OPEN - survivor marker broadcasting")
            : *FString::Printf(TEXT("CODING CLEARANCE %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity),
        Score,
        BuildSurvivorIntelArchiveText(Mission, ChallengeId, Language, Score, bAssisted, CompletedChallenges));
}

static void MirrorTerminalThemeFromSettings(const UCodeRescueGameInstance* GI)
{
    if (!GI)
    {
        return;
    }

    CodeRescueUI::Theme().bHighContrast = GI->bHighContrastHUD;
    CodeRescueUI::Theme().bReducedMotion = GI->bReducedMotion;
    CodeRescueUI::Theme().TextScale = GI->GetUITextScale();
}

static void StyleTerminalButton(UButton* Button, UTextBlock* Label, const FString& Text, const FLinearColor& LabelColor, bool bPrimary)
{
    if (bPrimary)
    {
        CodeRescueUI::StylePrimaryButton(Button);
    }
    else
    {
        CodeRescueUI::StyleSecondaryButton(Button);
    }

    if (Label)
    {
        Label->SetText(FText::FromString(Text));
        Label->SetAutoWrapText(true);
        Label->SetJustification(ETextJustify::Center);
        CodeRescueUI::StyleText(Label, CodeRescueUI::EType::BodySmall, LabelColor);
    }
}

static void StyleTerminalCodeEditor(UMultiLineEditableTextBox* CodeBox)
{
    if (!CodeBox)
    {
        return;
    }

    const FLinearColor CodeForeground(0.96f, 0.985f, 1.0f, 1.0f);
    const FLinearColor CodeBackground(0.002f, 0.005f, 0.008f, 1.0f);
    const FLinearColor CodeBackgroundHover(0.004f, 0.010f, 0.014f, 1.0f);
    const FLinearColor CodeBackgroundFocus(0.006f, 0.015f, 0.020f, 1.0f);

    FTextBlockStyle CodeStyle = FTextBlockStyle::GetDefault();
    CodeStyle.SetFont(FCoreStyle::GetDefaultFontStyle(FName(TEXT("Mono")), CodeRescueUI::ScaledSize(CodeRescueUI::EType::BodySmall)));
    CodeStyle.SetColorAndOpacity(FSlateColor(CodeForeground));
    CodeStyle.SetShadowOffset(FVector2D(1.0f, 1.0f));
    CodeStyle.SetShadowColorAndOpacity(CodeRescueUI::Color::Shadow());
    CodeStyle.SetSelectedBackgroundColor(FSlateColor(CodeRescueUI::Resolve(CodeRescueUI::Color::AccentEmber())));
    CodeStyle.SetHighlightColor(FSlateColor(CodeRescueUI::Resolve(CodeRescueUI::Color::AccentAmber())));

    FEditableTextBoxStyle EditorStyle = CodeBox->WidgetStyle;
    EditorStyle.SetTextStyle(CodeStyle);
    EditorStyle.SetForegroundColor(FSlateColor(CodeForeground));
    EditorStyle.SetFocusedForegroundColor(FSlateColor(CodeForeground));
    EditorStyle.SetReadOnlyForegroundColor(FSlateColor(CodeForeground));
    EditorStyle.SetBackgroundColor(FSlateColor(FLinearColor::White));
    EditorStyle.SetBackgroundImageNormal(FSlateColorBrush(CodeBackground));
    EditorStyle.SetBackgroundImageHovered(FSlateColorBrush(CodeBackgroundHover));
    EditorStyle.SetBackgroundImageFocused(FSlateColorBrush(CodeBackgroundFocus));
    EditorStyle.SetBackgroundImageReadOnly(FSlateColorBrush(CodeBackground));
    CodeBox->WidgetStyle = EditorStyle;
    CodeBox->SetTextStyle(CodeStyle);
    CodeBox->SetForegroundColor(CodeForeground);
    CodeBox->SetMargin(FMargin(CodeRescueUI::Space::M, CodeRescueUI::Space::S));
    CodeBox->SynchronizeProperties();

    const auto RelativeLuminance = [](const FLinearColor& Color)
    {
        return Color.R * 0.2126f + Color.G * 0.7152f + Color.B * 0.0722f;
    };
    const float ContrastRatio =
        (RelativeLuminance(CodeForeground) + 0.05f) /
        (RelativeLuminance(CodeBackground) + 0.05f);
    UE_LOG(LogTemp, Display,
        TEXT("[TerminalContrastAudit] COMPLETE %s ratio=%.2f foreground=(%.3f,%.3f,%.3f) background=(%.3f,%.3f,%.3f) minimum=7.0"),
        ContrastRatio >= 7.0f ? TEXT("PASS") : TEXT("FAIL"),
        ContrastRatio,
        CodeForeground.R, CodeForeground.G, CodeForeground.B,
        CodeBackground.R, CodeBackground.G, CodeBackground.B);
}

static FLinearColor TerminalOutputColor(bool bSuccess)
{
    return bSuccess ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::DangerBright();
}

static FLinearColor TerminalPanelFill()
{
    return CodeRescueUI::Theme().bHighContrast
        ? FLinearColor(0.005f, 0.010f, 0.005f, 0.98f)
        : CodeRescueUI::Surface::Panel();
}

static FLinearColor TerminalEditorFill()
{
    return FLinearColor(0.002f, 0.005f, 0.008f, 1.0f);
}

static FString TerminalToolchainStateLine(ECodingLanguage Language)
{
    const bool bAvailable = UCodeRunnerLibrary::IsLanguageAvailable(Language);
    return FString::Printf(
        TEXT("Toolchain: %s | Validator: %s"),
        bAvailable ? TEXT("external compiler detected") : TEXT("safe in-engine fallback"),
        bAvailable ? TEXT("live toolchain + game harness") : TEXT("game harness only"));
}

static void SetTerminalOutput(UTextBlock* OutputText, const FString& Message, const FLinearColor& Color)
{
    if (!OutputText)
    {
        return;
    }

    CodeRescueUI::StyleText(OutputText, CodeRescueUI::EType::BodySmall, Color, false);
    OutputText->SetText(FText::FromString(Message));
}

static FString MakeStarterForLanguage(const FString& ChallengeId, ECodingLanguage Language)
{
    enum class EStarterChallengeKind
    {
        Sum,
        Lock,
        Reverse,
        Palindrome,
        FizzBuzz,
        EvenFilter,
        LinkedList,
        BinarySearch
    };

    const EStarterChallengeKind Kind =
        ChallengeId.Contains(TEXT("lock")) ? EStarterChallengeKind::Lock :
        ChallengeId.Contains(TEXT("reverse")) ? EStarterChallengeKind::Reverse :
        ChallengeId.Contains(TEXT("palindrome")) ? EStarterChallengeKind::Palindrome :
        ChallengeId.Contains(TEXT("fizzbuzz")) ? EStarterChallengeKind::FizzBuzz :
        (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) ? EStarterChallengeKind::EvenFilter :
        (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("linked_list")) || ChallengeId.Contains(TEXT("traverse"))) ? EStarterChallengeKind::LinkedList :
        (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("binarysearch")) || ChallengeId.Contains(TEXT("bsearch"))) ? EStarterChallengeKind::BinarySearch :
        EStarterChallengeKind::Sum;

    switch (Language)
    {
    case ECodingLanguage::Java:
        switch (Kind)
        {
        case EStarterChallengeKind::Lock:
            return TEXT("public static boolean shouldUnlock(boolean hasKey, boolean powerOn) {\n    return hasKey && powerOn;\n}\n");
        case EStarterChallengeKind::Reverse:
            return TEXT("public static String reverseString(String s) {\n    return new StringBuilder(s).reverse().toString();\n}\n");
        case EStarterChallengeKind::Palindrome:
            return TEXT("public static boolean isPalindrome(String s) {\n    return s.equals(new StringBuilder(s).reverse().toString());\n}\n");
        case EStarterChallengeKind::FizzBuzz:
            return TEXT("public static String[] fizzBuzz(int n) {\n    String[] result = new String[n];\n    for (int i = 1; i <= n; i++) {\n        if (i % 15 == 0) result[i - 1] = \"FizzBuzz\";\n        else if (i % 3 == 0) result[i - 1] = \"Fizz\";\n        else if (i % 5 == 0) result[i - 1] = \"Buzz\";\n        else result[i - 1] = Integer.toString(i);\n    }\n    return result;\n}\n");
        case EStarterChallengeKind::EvenFilter:
            return TEXT("public static int[] evenNumbers(int[] values) {\n    int count = 0;\n    for (int value : values) {\n        if (value % 2 == 0) count++;\n    }\n    int[] result = new int[count];\n    int index = 0;\n    for (int value : values) {\n        if (value % 2 == 0) result[index++] = value;\n    }\n    return result;\n}\n");
        case EStarterChallengeKind::LinkedList:
            return TEXT("public static int countNodes(int[] next, int start) {\n    int count = 0;\n    int current = start;\n    while (current != -1) {\n        count++;\n        current = next[current];\n    }\n    return count;\n}\n");
        case EStarterChallengeKind::BinarySearch:
            return TEXT("public static int binarySearch(int[] values, int target) {\n    int low = 0;\n    int high = values.length - 1;\n    while (low <= high) {\n        int mid = (low + high) / 2;\n        if (values[mid] == target) return mid;\n        if (values[mid] < target) low = mid + 1;\n        else high = mid - 1;\n    }\n    return -1;\n}\n");
        default:
            return TEXT("public static int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n");
        }
    case ECodingLanguage::C:
        switch (Kind)
        {
        case EStarterChallengeKind::Lock:
            return TEXT("int shouldUnlock(int hasKey, int powerOn) {\n    return hasKey && powerOn;\n}\n");
        case EStarterChallengeKind::Reverse:
            return TEXT("void reverseString(const char* input, char* output) {\n    int length = (int)strlen(input);\n    for (int i = 0; i < length; i++) {\n        output[i] = input[length - 1 - i];\n    }\n    output[length] = '\\0';\n}\n");
        case EStarterChallengeKind::Palindrome:
            return TEXT("int isPalindrome(const char* s) {\n    int left = 0;\n    int right = (int)strlen(s) - 1;\n    while (left < right) {\n        if (s[left++] != s[right--]) return 0;\n    }\n    return 1;\n}\n");
        case EStarterChallengeKind::FizzBuzz:
            return TEXT("void fizzBuzz(int n, char* output, int outputSize) {\n    output[0] = '\\0';\n    for (int i = 1; i <= n; i++) {\n        char piece[16];\n        if (i % 15 == 0) snprintf(piece, sizeof(piece), \"FizzBuzz\");\n        else if (i % 3 == 0) snprintf(piece, sizeof(piece), \"Fizz\");\n        else if (i % 5 == 0) snprintf(piece, sizeof(piece), \"Buzz\");\n        else snprintf(piece, sizeof(piece), \"%d\", i);\n        if (i > 1) strncat(output, \",\", outputSize - strlen(output) - 1);\n        strncat(output, piece, outputSize - strlen(output) - 1);\n    }\n}\n");
        case EStarterChallengeKind::EvenFilter:
            return TEXT("int evenNumbers(const int* input, int count, int* output) {\n    int outCount = 0;\n    for (int i = 0; i < count; i++) {\n        if (input[i] % 2 == 0) {\n            output[outCount++] = input[i];\n        }\n    }\n    return outCount;\n}\n");
        case EStarterChallengeKind::LinkedList:
            return TEXT("int countNodes(const int* next, int count, int start) {\n    int total = 0;\n    int current = start;\n    while (current != -1 && current >= 0 && current < count) {\n        total++;\n        current = next[current];\n    }\n    return total;\n}\n");
        case EStarterChallengeKind::BinarySearch:
            return TEXT("int binarySearch(const int* values, int count, int target) {\n    int low = 0;\n    int high = count - 1;\n    while (low <= high) {\n        int mid = (low + high) / 2;\n        if (values[mid] == target) return mid;\n        if (values[mid] < target) low = mid + 1;\n        else high = mid - 1;\n    }\n    return -1;\n}\n");
        default:
            return TEXT("int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n");
        }
    case ECodingLanguage::CPlus:
    case ECodingLanguage::Cpp:
        switch (Kind)
        {
        case EStarterChallengeKind::Lock:
            return TEXT("bool shouldUnlock(bool hasKey, bool powerOn) {\n    return hasKey && powerOn;\n}\n");
        case EStarterChallengeKind::Reverse:
            return TEXT("std::string reverseString(std::string s) {\n    std::reverse(s.begin(), s.end());\n    return s;\n}\n");
        case EStarterChallengeKind::Palindrome:
            return TEXT("bool isPalindrome(const std::string& s) {\n    std::string reversed = s;\n    std::reverse(reversed.begin(), reversed.end());\n    return s == reversed;\n}\n");
        case EStarterChallengeKind::FizzBuzz:
            return TEXT("std::vector<std::string> fizzBuzz(int n) {\n    std::vector<std::string> result;\n    for (int i = 1; i <= n; ++i) {\n        if (i % 15 == 0) result.push_back(\"FizzBuzz\");\n        else if (i % 3 == 0) result.push_back(\"Fizz\");\n        else if (i % 5 == 0) result.push_back(\"Buzz\");\n        else result.push_back(std::to_string(i));\n    }\n    return result;\n}\n");
        case EStarterChallengeKind::EvenFilter:
            return TEXT("std::vector<int> evenNumbers(const std::vector<int>& values) {\n    std::vector<int> result;\n    for (int value : values) {\n        if (value % 2 == 0) result.push_back(value);\n    }\n    return result;\n}\n");
        case EStarterChallengeKind::LinkedList:
            return TEXT("int countNodes(const std::vector<int>& next, int start) {\n    int count = 0;\n    int current = start;\n    while (current != -1 && current >= 0 && current < static_cast<int>(next.size())) {\n        ++count;\n        current = next[current];\n    }\n    return count;\n}\n");
        case EStarterChallengeKind::BinarySearch:
            return TEXT("int binarySearch(const std::vector<int>& values, int target) {\n    int low = 0;\n    int high = static_cast<int>(values.size()) - 1;\n    while (low <= high) {\n        int mid = (low + high) / 2;\n        if (values[mid] == target) return mid;\n        if (values[mid] < target) low = mid + 1;\n        else high = mid - 1;\n    }\n    return -1;\n}\n");
        default:
            return TEXT("int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n");
        }
    case ECodingLanguage::Python:
        switch (Kind)
        {
        case EStarterChallengeKind::Lock:
            return TEXT("def should_unlock(has_key, power_on):\n    return has_key and power_on\n");
        case EStarterChallengeKind::Reverse:
            return TEXT("def reverse_string(s):\n    return s[::-1]\n");
        case EStarterChallengeKind::Palindrome:
            return TEXT("def is_palindrome(s):\n    return s == s[::-1]\n");
        case EStarterChallengeKind::FizzBuzz:
            return TEXT("def fizz_buzz(n):\n    result = []\n    for i in range(1, n + 1):\n        if i % 15 == 0:\n            result.append(\"FizzBuzz\")\n        elif i % 3 == 0:\n            result.append(\"Fizz\")\n        elif i % 5 == 0:\n            result.append(\"Buzz\")\n        else:\n            result.append(str(i))\n    return result\n");
        case EStarterChallengeKind::EvenFilter:
            return TEXT("def even_numbers(values):\n    return [value for value in values if value % 2 == 0]\n");
        case EStarterChallengeKind::LinkedList:
            return TEXT("def count_nodes(next_indices, start):\n    total = 0\n    current = start\n    while current != -1:\n        total += 1\n        current = next_indices[current]\n    return total\n");
        case EStarterChallengeKind::BinarySearch:
            return TEXT("def binary_search(values, target):\n    low = 0\n    high = len(values) - 1\n    while low <= high:\n        mid = (low + high) // 2\n        if values[mid] == target:\n            return mid\n        if values[mid] < target:\n            low = mid + 1\n        else:\n            high = mid - 1\n    return -1\n");
        default:
            return TEXT("def total_power(a, b, c):\n    return a + b + c\n");
        }
    case ECodingLanguage::MATLAB:
        switch (Kind)
        {
        case EStarterChallengeKind::Lock:
            return TEXT("function result = should_unlock(has_key, power_on)\n    result = has_key && power_on;\nend\n");
        case EStarterChallengeKind::Reverse:
            return TEXT("function result = reverse_string(s)\n    result = fliplr(s);\nend\n");
        case EStarterChallengeKind::Palindrome:
            return TEXT("function result = is_palindrome(s)\n    result = strcmp(s, fliplr(s));\nend\n");
        case EStarterChallengeKind::FizzBuzz:
            return TEXT("function result = fizz_buzz(n)\n    result = strings(1, n);\n    for i = 1:n\n        if mod(i, 15) == 0\n            result(i) = \"FizzBuzz\";\n        elseif mod(i, 3) == 0\n            result(i) = \"Fizz\";\n        elseif mod(i, 5) == 0\n            result(i) = \"Buzz\";\n        else\n            result(i) = string(i);\n        end\n    end\nend\n");
        case EStarterChallengeKind::EvenFilter:
            return TEXT("function result = even_numbers(values)\n    result = values(mod(values, 2) == 0);\nend\n");
        case EStarterChallengeKind::LinkedList:
            return TEXT("function result = count_nodes(next_indices, start)\n    result = 0;\n    current = start;\n    while current ~= 0\n        result = result + 1;\n        current = next_indices(current);\n    end\nend\n");
        case EStarterChallengeKind::BinarySearch:
            return TEXT("function result = binary_search(values, target)\n    low = 1;\n    high = numel(values);\n    while low <= high\n        mid = floor((low + high) / 2);\n        if values(mid) == target\n            result = mid;\n            return;\n        elseif values(mid) < target\n            low = mid + 1;\n        else\n            high = mid - 1;\n        end\n    end\n    result = 0;\nend\n");
        default:
            return TEXT("function result = total_power(a, b, c)\n    result = a + b + c;\nend\n");
        }
    default:
        return TEXT("// choose a language from the launch menu first\n");
    }
}

FString UCodeTerminalWidget::GetCanonicalReferenceSolution(
    const FString& ChallengeId,
    ECodingLanguage Language)
{
    return MakeStarterForLanguage(ChallengeId, Language);
}

void UCodeTerminalWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidgetTreeNow();
    if (GEngine)
    {
        GEngine->ClearOnScreenDebugMessages();
    }
    // 2026-07-06: take keyboard focus on open so Escape-to-close and the
    // Ctrl hotkeys work immediately (playtest: Escape was dead because focus
    // stayed on the game viewport until the student clicked into the widget).
    SetKeyboardFocus();
}

TSharedRef<SWidget> UCodeTerminalWidget::RebuildWidget()
{
    // 2026-07-01 ROOT FIX for invisible UMG: build the tree BEFORE Slate assembly.
    BuildWidgetTreeNow();
    return Super::RebuildWidget();
}

void UCodeTerminalWidget::BuildWidgetTreeNow()
{
    if (bWidgetTreeBuilt || !WidgetTree)
    {
        return;
    }
    bWidgetTreeBuilt = true;
    SetIsFocusable(true);

    MirrorTerminalThemeFromSettings(GetGameInstance<UCodeRescueGameInstance>());

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TerminalRoot"));
    WidgetTree->RootWidget = Root;

    UBackgroundBlur* Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("BackgroundBlur"));
    Blur->SetBlurStrength(CodeRescueUI::Theme().bReducedMotion ? 3.0f : 5.0f);
    UCanvasPanelSlot* BlurSlot = Root->AddChildToCanvas(Blur);
    BlurSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BlurSlot->SetOffsets(FMargin(0, 0, 0, 0));

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeTerminalBackdrop"));
    Backdrop->SetBrushColor(CodeRescueUI::Resolve(FLinearColor(0.010f, 0.012f, 0.010f, 0.74f)));
    UCanvasPanelSlot* BackdropSlot = Root->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0, 0, 1, 1));
    BackdropSlot->SetOffsets(FMargin(0, 0, 0, 0));

    PanelFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BespokeTerminalPanelFrame"));
    CodeRescueUI::StylePanel(
        PanelFrame,
        TerminalPanelFill(),
        FMargin(CodeRescueUI::Space::XL, CodeRescueUI::Space::L));
    UCanvasPanelSlot* FrameSlot = Root->AddChildToCanvas(PanelFrame);
    FrameSlot->SetAnchors(FAnchors(0.10f, 0.06f, 0.90f, 0.94f));
    FrameSlot->SetOffsets(FMargin(0, 0, 0, 0));

    UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TerminalPanel"));
    // 2026-07-06 first-level completion: the panel column scrolls as a whole.
    // At short window heights a successful validation grew the diagnostics
    // block until COLLECT REWARD / CLOSE TERMINAL were pushed below the window
    // edge with no way to reach them (live-playtest stuck state). Wrapping the
    // column in a ScrollBox guarantees every control is reachable at ANY
    // resolution; at fullscreen the content fits and the scrollbox is inert.
    TerminalScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("TerminalScroll"));
    TerminalScroll->SetScrollBarVisibility(ESlateVisibility::Visible);
    TerminalScroll->AddChild(Box);
    PanelFrame->SetContent(TerminalScroll);

    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    TitleText->SetText(FText::FromString(TEXT("FIELD CODING TERMINAL")));
    Box->AddChildToVerticalBox(TitleText);

    TerminalStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TerminalStatusText"));
    TerminalStatusText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(TerminalStatusText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TerminalGreenBright(), false);
    TerminalStatusText->SetText(FText::FromString(TEXT("Safehouse compiler link initializing...")));
    UVerticalBoxSlot* StatusSlot = Box->AddChildToVerticalBox(TerminalStatusText);
    StatusSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::S));

    // 2026-07-05 first-level layout fix (IDE layout): everything a student READS
    // (brief, track lock, toolchain banner, learning status, checklist + coach)
    // lives in ONE scrollable region with a hard height cap, so the CODE EDITOR
    // and the action buttons are ALWAYS on screen. Previously these text zones
    // grew unbounded and starved the fill-sized editor to zero height — the very
    // first terminal a player opened had no visible code panel at common window
    // sizes.
    UVerticalBox* ReadingBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TerminalReadingBox"));

    BriefText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BriefText"));
    BriefText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(BriefText, CodeRescueUI::EType::Body, CodeRescueUI::Color::TextPrimary(), false);
    UVerticalBoxSlot* BriefSlot = ReadingBox->AddChildToVerticalBox(BriefText);
    BriefSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::S, 0.0f, CodeRescueUI::Space::S));

    LanguageLockText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LanguageLockText"));
    LanguageLockText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(LanguageLockText, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::TerminalGreenBright());
    ReadingBox->AddChildToVerticalBox(LanguageLockText);

    // Dependency banner: green when the external toolchain is found, amber
    // when we're falling back to the in-engine validator. Updated by
    // RefreshText() whenever the selected language changes.
    DependencyBanner = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DependencyBanner"));
    DependencyBanner->SetAutoWrapText(true);
    DependencyBanner->SetText(FText::FromString(TEXT("Checking compiler availability...")));
    CodeRescueUI::StyleText(DependencyBanner, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::Warning(), false);
    UVerticalBoxSlot* DependencySlot = ReadingBox->AddChildToVerticalBox(DependencyBanner);
    DependencySlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::S));

    LearningStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LearningStatusText"));
    LearningStatusText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(LearningStatusText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreen(), false);
    LearningStatusText->SetText(FText::FromString(TEXT("Learning profile loading...")));
    ReadingBox->AddChildToVerticalBox(LearningStatusText);

    ChecklistText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ChecklistText"));
    ChecklistText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(ChecklistText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary(), false);
    ChecklistText->SetText(FText::FromString(TEXT("Challenge checklist loading...")));
    UVerticalBoxSlot* ChecklistSlot = ReadingBox->AddChildToVerticalBox(ChecklistText);
    ChecklistSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::S));

    UScrollBox* ReadingScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("TerminalReadingScroll"));
    ReadingScroll->AddChild(ReadingBox);
    USizeBox* ReadingCap = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TerminalReadingCap"));
    ReadingCap->SetMaxDesiredHeight(280.0f);
    ReadingCap->SetContent(ReadingScroll);
    UVerticalBoxSlot* ReadingSlot = Box->AddChildToVerticalBox(ReadingCap);
    ReadingSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::S, 0.0f, CodeRescueUI::Space::S));

    CodeBox = WidgetTree->ConstructWidget<UMultiLineEditableTextBox>(UMultiLineEditableTextBox::StaticClass(), TEXT("CodeBox"));
    CodeBox->SetText(FText::FromString(TEXT("// code goes here")));
    StyleTerminalCodeEditor(CodeBox);
    CodeEditorFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TerminalCodeEditorFrame"));
    CodeRescueUI::StylePanel(CodeEditorFrame, TerminalEditorFill(), FMargin(2.0f, 2.0f));
    CodeEditorFrame->SetContent(CodeBox);
    // Auto-sized with a guaranteed minimum: the editor can never be squeezed out.
    USizeBox* CodeSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CodeEditorMinHeight"));
    CodeSize->SetMinDesiredHeight(210.0f);
    CodeSize->SetContent(CodeEditorFrame);
    UVerticalBoxSlot* CodeSlot = Box->AddChildToVerticalBox(CodeSize);
    CodeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    CodeSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::M, 0.0f, CodeRescueUI::Space::M));

    OutputFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TerminalDiagnosticsFrame"));
    CodeRescueUI::StylePanel(OutputFrame, TerminalEditorFill(), FMargin(CodeRescueUI::Space::M, CodeRescueUI::Space::S));
    UVerticalBox* DiagnosticsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TerminalDiagnosticsBox"));
    // 2026-07-06: a passing validation prints ~20 lines (score card + PASSED
    // list + takeaway + REFLECT debrief). Scroll it inside a capped frame so
    // results never shove the action buttons around — same IDE treatment as
    // the reading panel above.
    UScrollBox* DiagScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("DiagnosticsScroll"));
    DiagScroll->AddChild(DiagnosticsBox);
    USizeBox* DiagCap = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DiagnosticsMaxHeight"));
    DiagCap->SetMaxDesiredHeight(230.0f);
    DiagCap->SetContent(DiagScroll);
    OutputFrame->SetContent(DiagCap);

    DiagnosticsHeaderText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DiagnosticsHeaderText"));
    DiagnosticsHeaderText->SetText(FText::FromString(TEXT("DIAGNOSTICS | READY")));
    CodeRescueUI::StyleText(DiagnosticsHeaderText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::Warning(), false);
    DiagnosticsBox->AddChildToVerticalBox(DiagnosticsHeaderText);

    OutputText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OutputText"));
    OutputText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(OutputText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::Warning(), false);
    SetTerminalOutput(OutputText, TEXT("Practice run checks code without saving or opening the route. Validate code when ready to commit progress."), CodeRescueUI::Color::Warning());
    UVerticalBoxSlot* OutputTextSlot = DiagnosticsBox->AddChildToVerticalBox(OutputText);
    OutputTextSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, 0.0f));
    UVerticalBoxSlot* OutputSlot = Box->AddChildToVerticalBox(OutputFrame);
    OutputSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, CodeRescueUI::Space::S));

    // 2026-07-04 (item 27): predict-the-output drill row — question + three choices.
    // Populated per-challenge by SetupPredictionDrill(); collapsed when no tests.
    PredictionDrillRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("PredictionDrillRow"));
    PredictionQuestionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PredictionQuestionText"));
    PredictionQuestionText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(PredictionQuestionText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::AccentAmber(), false);
    UHorizontalBoxSlot* PredictQSlot = PredictionDrillRow->AddChildToHorizontalBox(PredictionQuestionText);
    PredictQSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    PredictQSlot->SetPadding(FMargin(0.0f, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    auto MakePredictChoice = [&](const TCHAR* BtnName, const TCHAR* LabelName, UButton*& OutBtn, UTextBlock*& OutLabel)
    {
        OutBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), BtnName);
        OutLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), LabelName);
        StyleTerminalButton(OutBtn, OutLabel, TEXT("?"), CodeRescueUI::Color::Stamina(), false);
        OutBtn->AddChild(OutLabel);
        UHorizontalBoxSlot* ChoiceSlot = PredictionDrillRow->AddChildToHorizontalBox(OutBtn);
        ChoiceSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        ChoiceSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, 0.0f, 0.0f));
    };
    MakePredictChoice(TEXT("PredictChoiceButtonA"), TEXT("PredictChoiceLabelA"), PredictChoiceButtonA, PredictChoiceLabelA);
    MakePredictChoice(TEXT("PredictChoiceButtonB"), TEXT("PredictChoiceLabelB"), PredictChoiceButtonB, PredictChoiceLabelB);
    MakePredictChoice(TEXT("PredictChoiceButtonC"), TEXT("PredictChoiceLabelC"), PredictChoiceButtonC, PredictChoiceLabelC);
    PredictChoiceButtonA->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnPredictChoiceA);
    PredictChoiceButtonB->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnPredictChoiceB);
    PredictChoiceButtonC->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnPredictChoiceC);
    PredictionDrillRow->SetVisibility(ESlateVisibility::Collapsed);
    UVerticalBoxSlot* PredictionSlot = Box->AddChildToVerticalBox(PredictionDrillRow);
    PredictionSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS));

    UHorizontalBox* PrimaryActionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TerminalPrimaryActionRow"));
    ValidateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ValidateButton"));
    UTextBlock* ValidateLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ValidateLabel"));
    StyleTerminalButton(ValidateButton, ValidateLabel, TEXT("VALIDATE CODE"), CodeRescueUI::Color::AccentAmber(), true);
    ValidateButton->AddChild(ValidateLabel);
    ValidateButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnValidateClicked);
    UHorizontalBoxSlot* ValidateSlot = PrimaryActionRow->AddChildToHorizontalBox(ValidateButton);
    ValidateSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ValidateSlot->SetPadding(FMargin(0.0f, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    PracticeButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PracticeRunButton"));
    UTextBlock* PracticeLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PracticeRunLabel"));
    StyleTerminalButton(PracticeButton, PracticeLabel, TEXT("PRACTICE RUN [Ctrl+P]"), CodeRescueUI::Color::Stamina(), false);
    PracticeButton->AddChild(PracticeLabel);
    PracticeButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnPracticeClicked);
    UHorizontalBoxSlot* PracticeSlot = PrimaryActionRow->AddChildToHorizontalBox(PracticeButton);
    PracticeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    PracticeSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    ResetButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ResetButton"));
    UTextBlock* ResetLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResetLabel"));
    StyleTerminalButton(ResetButton, ResetLabel, TEXT("RESET STARTER CODE [Ctrl+R]"), CodeRescueUI::Color::TerminalGreen(), false);
    ResetButton->AddChild(ResetLabel);
    ResetButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnResetStarterClicked);
    UHorizontalBoxSlot* ResetSlot = PrimaryActionRow->AddChildToHorizontalBox(ResetButton);
    ResetSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ResetSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    HintButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("HintButton"));
    UTextBlock* HintBtnLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HintBtnLabel"));
    StyleTerminalButton(HintButton, HintBtnLabel, TEXT("REVEAL HINT [Ctrl+H] (-1 ResearchPoint)"), CodeRescueUI::Color::Warning(), false);
    HintButton->AddChild(HintBtnLabel);
    HintButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnHintClicked);
    UHorizontalBoxSlot* HintButtonSlot = PrimaryActionRow->AddChildToHorizontalBox(HintButton);
    HintButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    HintButtonSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    BypassButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BypassKitButton"));
    UTextBlock* BypassLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BypassKitLabel"));
    StyleTerminalButton(BypassButton, BypassLabel, TEXT("USE BYPASS KIT [Ctrl+B]"), CodeRescueUI::Color::AccentEmber(), false);
    BypassButton->AddChild(BypassLabel);
    BypassButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnBypassClicked);
    UHorizontalBoxSlot* BypassSlot = PrimaryActionRow->AddChildToHorizontalBox(BypassButton);
    BypassSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    BypassSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, 0.0f, 0.0f));

    UVerticalBoxSlot* PrimaryActionSlot = Box->AddChildToVerticalBox(PrimaryActionRow);
    PrimaryActionSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS));

    UHorizontalBox* UtilityActionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TerminalUtilityActionRow"));
    MATLABButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("MATLABButton"));
    UTextBlock* MATLABLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MATLABLabel"));
    StyleTerminalButton(MATLABButton, MATLABLabel, TEXT("OPEN MATLAB DESKTOP"), CodeRescueUI::Color::Stamina(), false);
    MATLABButton->AddChild(MATLABLabel);
    MATLABButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnMATLABClicked);
    UHorizontalBoxSlot* MATLABSlot = UtilityActionRow->AddChildToHorizontalBox(MATLABButton);
    MATLABSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    MATLABSlot->SetPadding(FMargin(0.0f, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
    UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CloseLabel"));
    StyleTerminalButton(CloseButton, CloseLabel, TEXT("CLOSE TERMINAL"), CodeRescueUI::Color::DangerBright(), false);
    CloseButton->AddChild(CloseLabel);
    CloseButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnCloseClicked);
    UHorizontalBoxSlot* CloseSlot = UtilityActionRow->AddChildToHorizontalBox(CloseButton);
    CloseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    CloseSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, 0.0f, 0.0f));

    UVerticalBoxSlot* UtilityActionSlot = Box->AddChildToVerticalBox(UtilityActionRow);
    UtilityActionSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS));

    UHorizontalBox* RewardActionRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RewardChoiceActionRow"));
    RewardResearchButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardChoiceResearchButton"));
    UTextBlock* RewardResearchLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RewardChoiceResearchLabel"));
    StyleTerminalButton(RewardResearchButton, RewardResearchLabel, TEXT("REWARD: RESEARCH +2 RP"), CodeRescueUI::Color::TerminalGreenBright(), false);
    RewardResearchButton->AddChild(RewardResearchLabel);
    RewardResearchButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnRewardResearchClicked);
    UHorizontalBoxSlot* RewardResearchSlot = RewardActionRow->AddChildToHorizontalBox(RewardResearchButton);
    RewardResearchSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    RewardResearchSlot->SetPadding(FMargin(0.0f, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    RewardFieldKitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardChoiceFieldKitButton"));
    UTextBlock* RewardFieldKitLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RewardChoiceFieldKitLabel"));
    StyleTerminalButton(RewardFieldKitButton, RewardFieldKitLabel, TEXT("REWARD: FIELD KIT"), CodeRescueUI::Color::Stamina(), false);
    RewardFieldKitButton->AddChild(RewardFieldKitLabel);
    RewardFieldKitButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnRewardFieldKitClicked);
    UHorizontalBoxSlot* RewardFieldKitSlot = RewardActionRow->AddChildToHorizontalBox(RewardFieldKitButton);
    RewardFieldKitSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    RewardFieldKitSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS, 0.0f));

    RewardCraftingButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RewardChoiceCraftingButton"));
    UTextBlock* RewardCraftingLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RewardChoiceCraftingLabel"));
    StyleTerminalButton(RewardCraftingButton, RewardCraftingLabel, TEXT("REWARD: CRAFTING CACHE"), CodeRescueUI::Color::AccentAmber(), false);
    RewardCraftingButton->AddChild(RewardCraftingLabel);
    RewardCraftingButton->OnClicked.AddDynamic(this, &UCodeTerminalWidget::OnRewardCraftingClicked);
    UHorizontalBoxSlot* RewardCraftingSlot = RewardActionRow->AddChildToHorizontalBox(RewardCraftingButton);
    RewardCraftingSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    RewardCraftingSlot->SetPadding(FMargin(CodeRescueUI::Space::XS, 0.0f, 0.0f, 0.0f));

    UVerticalBoxSlot* RewardActionSlot = Box->AddChildToVerticalBox(RewardActionRow);
    RewardActionSlot->SetPadding(FMargin(0.0f, CodeRescueUI::Space::XS, 0.0f, CodeRescueUI::Space::XS));

    HintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HintText"));
    HintText->SetAutoWrapText(true);
    CodeRescueUI::StyleText(HintText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::Warning(), false);
    HintText->SetText(FText::FromString(TEXT("")));
    Box->AddChildToVerticalBox(HintText);

    RefreshText();
}

namespace
{
TArray<FString> GetHintsForChallenge(const FString& ChallengeId)
{
    // Per-shape pseudocode hint ladder. Each hint reveals one more line.
    if (ChallengeId.Contains(TEXT("sum")) || ChallengeId.Contains(TEXT("generator")))
    {
        return {
            TEXT("Hint 1: Define a function that takes three numeric arguments."),
            TEXT("Hint 2: Add all three arguments together (a + b + c)."),
            TEXT("Hint 3: Return the sum in the form your selected track expects."),
        };
    }
    if (ChallengeId.Contains(TEXT("lock")))
    {
        return {
            TEXT("Hint 1: Define shouldUnlock(keyA, keyB) that returns a boolean."),
            TEXT("Hint 2: Use boolean AND between the two parameters."),
            TEXT("Hint 3: Return the result of (keyA && keyB)."),
        };
    }
    if (ChallengeId.Contains(TEXT("reverse")))
    {
        return {
            TEXT("Hint 1: Define reverse(input) that returns the reversed input."),
            TEXT("Hint 2: Either iterate from end to start, or use a safe reverse helper for your selected track."),
            TEXT("Hint 3: Return the reversed string/array."),
        };
    }
    if (ChallengeId.Contains(TEXT("palindrome")))
    {
        return {
            TEXT("Hint 1: Define isPalindrome(s) that returns true/false."),
            TEXT("Hint 2: Compare s to its reverse, OR walk i from 0 and j from n-1 inward."),
            TEXT("Hint 3: Return the comparison result."),
        };
    }
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        return {
            TEXT("Hint 1: Loop from 1 to N."),
            TEXT("Hint 2: If n % 15 == 0 print 'FizzBuzz', elif n % 3 == 0 print 'Fizz', elif n % 5 == 0 print 'Buzz'."),
            TEXT("Hint 3: Else print n itself."),
        };
    }
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        return {
            TEXT("Hint 1: Define a filter function that takes a list."),
            TEXT("Hint 2: Iterate the list, keep only items where item % 2 == 0."),
            TEXT("Hint 3: Return the filtered list."),
        };
    }
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")))
    {
        return {
            TEXT("Hint 1: Initialize current to the given start index."),
            TEXT("Hint 2: While current is not the stop value, count it and move to next[current]."),
            TEXT("Hint 3: Return the total nodes visited before the sentinel."),
        };
    }
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        return {
            TEXT("Hint 1: Maintain lo = 0, hi = n - 1."),
            TEXT("Hint 2: While lo <= hi, compute mid = (lo + hi) / 2; compare arr[mid] to target."),
            TEXT("Hint 3: If arr[mid] < target: lo = mid + 1; else hi = mid - 1. Return mid on equality."),
        };
    }
    return { TEXT("Hint: try a small example by hand first, then generalize.") };
}

int32 CountNonWhitespaceCharacters(const FString& Code)
{
    int32 Count = 0;
    for (const TCHAR Ch : Code)
    {
        if (!FChar::IsWhitespace(Ch))
        {
            ++Count;
        }
    }
    return Count;
}

FString GetConceptLabelForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Boolean logic");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("String traversal");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Two-way comparison");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Loop branches");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("List filtering");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Linked-list traversal");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Binary search");
    return TEXT("Function design");
}

FString GetSignatureReminderForChallenge(const FString& ChallengeId, ECodingLanguage Language)
{
    enum class ESignatureShape
    {
        Sum,
        Lock,
        Reverse,
        Palindrome,
        FizzBuzz,
        EvenFilter,
        LinkedList,
        BinarySearch
    };

    const ESignatureShape Shape =
        ChallengeId.Contains(TEXT("lock")) ? ESignatureShape::Lock :
        ChallengeId.Contains(TEXT("reverse")) ? ESignatureShape::Reverse :
        ChallengeId.Contains(TEXT("palindrome")) ? ESignatureShape::Palindrome :
        ChallengeId.Contains(TEXT("fizzbuzz")) ? ESignatureShape::FizzBuzz :
        (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) ? ESignatureShape::EvenFilter :
        (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) ? ESignatureShape::LinkedList :
        (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) ? ESignatureShape::BinarySearch :
        ESignatureShape::Sum;

    switch (Language)
    {
    case ECodingLanguage::Java:
        switch (Shape)
        {
        case ESignatureShape::Lock: return TEXT("Signature: public static boolean shouldUnlock(boolean hasKey, boolean powerOn)");
        case ESignatureShape::Reverse: return TEXT("Signature: public static String reverseString(String s)");
        case ESignatureShape::Palindrome: return TEXT("Signature: public static boolean isPalindrome(String s)");
        case ESignatureShape::FizzBuzz: return TEXT("Signature: public static String[] fizzBuzz(int n)");
        case ESignatureShape::EvenFilter: return TEXT("Signature: public static int[] evenNumbers(int[] values)");
        case ESignatureShape::LinkedList: return TEXT("Signature: public static int countNodes(int[] next, int start)");
        case ESignatureShape::BinarySearch: return TEXT("Signature: public static int binarySearch(int[] values, int target)");
        default: return TEXT("Signature: public static int totalPower(int a, int b, int c)");
        }
    case ECodingLanguage::C:
        switch (Shape)
        {
        case ESignatureShape::Lock: return TEXT("Signature: int shouldUnlock(int hasKey, int powerOn)");
        case ESignatureShape::Reverse: return TEXT("Signature: void reverseString(const char* input, char* output)");
        case ESignatureShape::Palindrome: return TEXT("Signature: int isPalindrome(const char* s)");
        case ESignatureShape::FizzBuzz: return TEXT("Signature: void fizzBuzz(int n, char* output, int outputSize)");
        case ESignatureShape::EvenFilter: return TEXT("Signature: int evenNumbers(const int* input, int count, int* output)");
        case ESignatureShape::LinkedList: return TEXT("Signature: int countNodes(const int* next, int count, int start)");
        case ESignatureShape::BinarySearch: return TEXT("Signature: int binarySearch(const int* values, int count, int target)");
        default: return TEXT("Signature: int totalPower(int a, int b, int c)");
        }
    case ECodingLanguage::CPlus:
    case ECodingLanguage::Cpp:
        switch (Shape)
        {
        case ESignatureShape::Lock: return TEXT("Signature: bool shouldUnlock(bool hasKey, bool powerOn)");
        case ESignatureShape::Reverse: return TEXT("Signature: std::string reverseString(std::string s)");
        case ESignatureShape::Palindrome: return TEXT("Signature: bool isPalindrome(const std::string& s)");
        case ESignatureShape::FizzBuzz: return TEXT("Signature: std::vector<std::string> fizzBuzz(int n)");
        case ESignatureShape::EvenFilter: return TEXT("Signature: std::vector<int> evenNumbers(const std::vector<int>& values)");
        case ESignatureShape::LinkedList: return TEXT("Signature: int countNodes(const std::vector<int>& next, int start)");
        case ESignatureShape::BinarySearch: return TEXT("Signature: int binarySearch(const std::vector<int>& values, int target)");
        default: return TEXT("Signature: int totalPower(int a, int b, int c)");
        }
    case ECodingLanguage::Python:
        switch (Shape)
        {
        case ESignatureShape::Lock: return TEXT("Signature: def should_unlock(has_key, power_on)");
        case ESignatureShape::Reverse: return TEXT("Signature: def reverse_string(s)");
        case ESignatureShape::Palindrome: return TEXT("Signature: def is_palindrome(s)");
        case ESignatureShape::FizzBuzz: return TEXT("Signature: def fizz_buzz(n)");
        case ESignatureShape::EvenFilter: return TEXT("Signature: def even_numbers(values)");
        case ESignatureShape::LinkedList: return TEXT("Signature: def count_nodes(next_indices, start)");
        case ESignatureShape::BinarySearch: return TEXT("Signature: def binary_search(values, target)");
        default: return TEXT("Signature: def total_power(a, b, c)");
        }
    case ECodingLanguage::MATLAB:
        switch (Shape)
        {
        case ESignatureShape::Lock: return TEXT("Signature: function result = should_unlock(has_key, power_on)");
        case ESignatureShape::Reverse: return TEXT("Signature: function result = reverse_string(s)");
        case ESignatureShape::Palindrome: return TEXT("Signature: function result = is_palindrome(s)");
        case ESignatureShape::FizzBuzz: return TEXT("Signature: function result = fizz_buzz(n)");
        case ESignatureShape::EvenFilter: return TEXT("Signature: function result = even_numbers(values)");
        case ESignatureShape::LinkedList: return TEXT("Signature: function result = count_nodes(next_indices, start)");
        case ESignatureShape::BinarySearch: return TEXT("Signature: function result = binary_search(values, target)");
        default: return TEXT("Signature: function result = total_power(a, b, c)");
        }
    default:
        return TEXT("Signature: choose a launch language first");
    }
}

FString GetChecklistForChallenge(const FString& ChallengeId, ECodingLanguage Language)
{
    FString Checklist = GetSignatureReminderForChallenge(ChallengeId, Language);
    Checklist += TEXT("\nChecklist: ");
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        Checklist += TEXT("loop 1..n | test 15 before 3/5 | return every item");
    }
    else if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        Checklist += TEXT("lo/hi bounds | midpoint | shrink range | return not-found value");
    }
    else if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")))
    {
        Checklist += TEXT("start at head | visit current | advance current | stop at null");
    }
    else if (ChallengeId.Contains(TEXT("palindrome")))
    {
        Checklist += TEXT("compare ends | stop at center | false on mismatch | true when all match");
    }
    else if (ChallengeId.Contains(TEXT("reverse")))
    {
        Checklist += TEXT("preserve length | copy from end to start | return reversed value");
    }
    else if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        Checklist += TEXT("iterate inputs | keep evens only | preserve order | return collection/count");
    }
    else if (ChallengeId.Contains(TEXT("lock")))
    {
        Checklist += TEXT("read both booleans | combine with AND | return true only when both pass");
    }
    else
    {
        Checklist += TEXT("read inputs | compute total | return exact expected value");
    }
    return Checklist;
}

FString GetNextMicroGoal(const FString& ChallengeId, int32 ConsecutiveFailures)
{
    const FString FirstStep =
        ChallengeId.Contains(TEXT("fizzbuzz")) ? TEXT("Write only the loop and one branch, then validate the shape.") :
        ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")) ? TEXT("Trace one search by hand and verify lo/hi shrink each pass.") :
        ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")) ? TEXT("Write the current = current.next step before anything else.") :
        ChallengeId.Contains(TEXT("palindrome")) ? TEXT("Test one short word with left/right indexes on paper.") :
        ChallengeId.Contains(TEXT("reverse")) ? TEXT("Confirm the first output character comes from the last input character.") :
        ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")) ? TEXT("Make one tiny list and keep only numbers divisible by 2.") :
        ChallengeId.Contains(TEXT("lock")) ? TEXT("List the truth table and make only true/true unlock.") :
        TEXT("Run a tiny example and match the return value exactly.");

    if (ConsecutiveFailures >= 3)
    {
        return FirstStep + TEXT(" Then use a hint if the same check fails again.");
    }
    if (ConsecutiveFailures >= 2)
    {
        return FirstStep + TEXT(" Change one thing at a time.");
    }
    return FirstStep;
}

FString FormatStreamForTerminal(const FString& Stream)
{
    return Stream.TrimStartAndEnd().IsEmpty() ? TEXT("[empty]") : Stream;
}

FString GetRewardPreview(const UCodeRescueGameInstance* GI, bool bUsedHintThisAttempt)
{
    const int32 CurrentStreak = GI ? GI->CurrentLearningStreak : 0;
    return FString::Printf(
        TEXT("Reward Preview: no-hint +1 RP, first-try perfect +1 RP, every 5-streak +1 RP | clean status: %s | current streak: %d"),
        bUsedHintThisAttempt ? TEXT("hint used") : TEXT("clean"),
        CurrentStreak);
}

FString GetLanguageTechniqueTip(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java:
        return TEXT("Java tip: keep the public static method signature exactly as the harness expects.");
    case ECodingLanguage::C:
        return TEXT("C tip: initialize output buffers and be careful with null terminators.");
    case ECodingLanguage::CPlus:
        return TEXT("C+ tip: keep the C-style logic clear, then use std::string or std::vector where the prompt asks for collections.");
    case ECodingLanguage::Cpp:
        return TEXT("C++ tip: prefer std::vector and std::string return values so the validator can compare results directly.");
    case ECodingLanguage::Python:
        return TEXT("Python tip: small helper variables often make the algorithm easier to read.");
    case ECodingLanguage::MATLAB:
        return TEXT("MATLAB tip: vectorized expressions are welcome, but the function name must match the file.");
    default:
        return TEXT("Tip: keep the function name and return shape aligned with the prompt.");
    }
}

FString GetWhyThisMattersForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Why it matters: boolean decisions keep gates, settings, permissions, and rescue rules predictable.");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("Why it matters: reverse traversal teaches index control and safe movement through strings.");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Why it matters: mirror checks teach you to prove both acceptance and rejection cases.");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Why it matters: ordered branches are the heart of validators, quests, and game-state scripts.");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("Why it matters: filters are how programs clean data, search results, and mission queues.");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Why it matters: linked traversal teaches current, next, and sentinel state.");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Why it matters: binary search shows how a strong invariant beats brute force.");
    return TEXT("Why it matters: returning one exact value is the first contract between your code and the game world.");
}

FString GetPredictionPromptForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Predict before coding: which true/false pairs should open the route?");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("Predict before coding: what are the first and last output characters?");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Predict before coding: name one mirror code that passes and one impostor that fails.");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Predict before coding: what should 3, 5, and 15 output?");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("Predict before coding: which values survive a short odd/even list?");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Predict before coding: list the node visit order from start to sentinel.");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Predict before coding: write low, mid, and high after the first comparison.");
    return TEXT("Predict before coding: add three tiny inputs by hand and write the expected return.");
}

FString GetWorkedExampleForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Worked example: true && false stays locked; true && true opens.");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("Worked example: rescue -> eucser by reading from the end.");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Worked example: level passes because paired characters match inward.");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Worked example: 15 is FizzBuzz because the combined rule comes first.");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("Worked example: [1,2,3,4] keeps [2,4] in the same order.");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Worked example: next=[1,2,-1], start=0 visits three nodes.");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Worked example: target 6 in [2,4,6,8] checks the middle before shrinking.");
    return TEXT("Worked example: 20, 15, and 10 return 45.");
}

FString GetVisualDebuggerCueForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Visual debugger: truth-table lamps show both inputs and final gate state.");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("Visual debugger: packet tiles light from right to left as each character moves.");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Visual debugger: mirrored posts show left/right comparisons together.");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Visual debugger: numbered beacon pylons separate 3, 5, and 15 decisions.");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("Visual debugger: triage lanes split kept even values from rejected odd values.");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Visual debugger: linked posts show current, next, count, and sentinel.");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Visual debugger: shrinking bands mark low, mid, high, and discarded ranges.");
    return TEXT("Visual debugger: three power cells light as inputs combine into one return value.");
}

FString GetMistakeGlossaryForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Mistake glossary: OR opens unsafe pairs; no return leaves the harness undecided.");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("Mistake glossary: off-by-one indexes drop the first or last character.");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Mistake glossary: returning true too early accepts impostor codes.");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Mistake glossary: checking 3 before 15 turns FizzBuzz into only Fizz.");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("Mistake glossary: appending every value or changing order breaks the filter.");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Mistake glossary: forgetting current = next[current] creates an infinite loop.");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Mistake glossary: stale bounds repeat the same midpoint forever.");
    return TEXT("Mistake glossary: printing instead of returning hides the answer from the validator.");
}

FString GetHiddenTestDebriefForChallenge(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock"))) return TEXT("Hidden-test debrief: unsafe true/false combinations must all stay locked.");
    if (ChallengeId.Contains(TEXT("reverse"))) return TEXT("Hidden-test debrief: mixed-case and city-specific packets must reverse exactly.");
    if (ChallengeId.Contains(TEXT("palindrome"))) return TEXT("Hidden-test debrief: one real mirror and one impostor verify both outcomes.");
    if (ChallengeId.Contains(TEXT("fizzbuzz"))) return TEXT("Hidden-test debrief: longer sweeps make sure later 3, 5, and 15 multiples still work.");
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even"))) return TEXT("Hidden-test debrief: odd-only and mixed lists verify order and empty-result behavior.");
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse"))) return TEXT("Hidden-test debrief: changed start nodes prove traversal is not hard-coded.");
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch"))) return TEXT("Hidden-test debrief: first, middle, last, and not-found targets prove the bounds.");
    return TEXT("Hidden-test debrief: zeros and smaller inputs prove the sum is general.");
}

FString GetTerminalLearningCoach(const FString& ChallengeId)
{
    return GetWhyThisMattersForChallenge(ChallengeId)
        + TEXT("\n") + GetPredictionPromptForChallenge(ChallengeId)
        + TEXT("\n") + GetWorkedExampleForChallenge(ChallengeId)
        + TEXT("\nCode-trace mini-game: step through one visible input before pressing Validate.")
        + TEXT("\n") + GetVisualDebuggerCueForChallenge(ChallengeId)
        + TEXT("\n") + GetMistakeGlossaryForChallenge(ChallengeId)
        + TEXT("\n") + GetHiddenTestDebriefForChallenge(ChallengeId);
}

FString GetMasteryGrade(const FCodeValidationResult& Result, int32 Attempts, int32 HintsUsed)
{
    if (!Result.bSuccess)
    {
        return Result.Score >= 70 ? TEXT("Retry-Ready") : TEXT("Needs Debugging");
    }
    if (Result.Score >= 100 && Attempts == 1 && HintsUsed == 0)
    {
        return TEXT("S - First Try");
    }
    if (Result.Score >= 95 && HintsUsed == 0)
    {
        return TEXT("A - Independent");
    }
    if (Result.Score >= 85)
    {
        return TEXT("B - Mission Ready");
    }
    return TEXT("C - Passing");
}

FString GetConceptProofForChallenge(const FString& ChallengeId, const FCodeValidationResult& Result)
{
    const FString TestShape = FString::Printf(
        TEXT("validated %d/%d declared checks"),
        Result.PassedTestCases,
        Result.TotalTestCases);

    if (ChallengeId.Contains(TEXT("lock")))
    {
        return TestShape + TEXT("; every unsafe boolean pair stayed closed while the safe pair opened.");
    }
    if (ChallengeId.Contains(TEXT("reverse")))
    {
        return TestShape + TEXT("; the output preserved length while moving each character from the opposite end.");
    }
    if (ChallengeId.Contains(TEXT("palindrome")))
    {
        return TestShape + TEXT("; mirror pairs were checked before accepting or rejecting the code.");
    }
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        return TestShape + TEXT("; the combined 15 rule was handled before the simpler 3 and 5 branches.");
    }
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        return TestShape + TEXT("; only even values survived and their original order stayed intact.");
    }
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")))
    {
        return TestShape + TEXT("; current advanced through each node and stopped at the sentinel.");
    }
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        return TestShape + TEXT("; low and high bounds shrank without losing the target or not-found case.");
    }
    return TestShape + TEXT("; the function returned the exact value the rescue harness expected.");
}

FString GetLanguageTransferForChallenge(const FString& ChallengeId, ECodingLanguage Language)
{
    FString Pattern;
    if (ChallengeId.Contains(TEXT("lock")))
    {
        Pattern = TEXT("carry the truth-table habit into every permission, door, and squad-safety check.");
    }
    else if (ChallengeId.Contains(TEXT("reverse")))
    {
        Pattern = TEXT("reuse clear index direction whenever mission data must be transformed safely.");
    }
    else if (ChallengeId.Contains(TEXT("palindrome")))
    {
        Pattern = TEXT("reuse paired comparisons when a route code must prove both match and mismatch cases.");
    }
    else if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        Pattern = TEXT("reuse strongest-condition-first branching when multiple rescue rules overlap.");
    }
    else if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        Pattern = TEXT("reuse keep/reject loops to clean survivor queues, loot lists, and route scans.");
    }
    else if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")))
    {
        Pattern = TEXT("reuse current-next-sentinel thinking for chained objectives and path segments.");
    }
    else if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        Pattern = TEXT("reuse invariant checks whenever sorted intel can narrow the search space.");
    }
    else
    {
        Pattern = TEXT("reuse exact return contracts whenever the game harness asks for one clear result.");
    }

    return Pattern + TEXT(" ") + GetLanguageTechniqueTip(Language);
}

FString GetNextPracticeRepForChallenge(const FString& ChallengeId, bool bUsedHintThisAttempt, int32 ResultScore)
{
    const FString CleanFocus = bUsedHintThisAttempt
        ? TEXT("repeat this shape once without hints to bank independent mastery")
        : TEXT("try the next terminal with the same no-hint discipline");

    if (ResultScore < 100)
    {
        return TEXT("raise the same solution to a perfect score by checking edge cases before moving on.");
    }
    if (ChallengeId.Contains(TEXT("lock")))
    {
        return CleanFocus + TEXT("; write the false/false, true/false, false/true, true/true cases first.");
    }
    if (ChallengeId.Contains(TEXT("reverse")) || ChallengeId.Contains(TEXT("palindrome")))
    {
        return CleanFocus + TEXT("; trace a short string by hand before validating.");
    }
    if (ChallengeId.Contains(TEXT("fizzbuzz")) || ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        return CleanFocus + TEXT("; predict a mixed input list before running the validator.");
    }
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")) || ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        return CleanFocus + TEXT("; keep a tiny table of state changes beside the code.");
    }
    return CleanFocus + TEXT("; keep the signature exact and return the value directly.");
}

FString BuildPostSolveAfterActionDebrief(
    const FString& ChallengeId,
    const FCodeValidationResult& Result,
    ECodingLanguage Language,
    const UCodeRescueGameInstance* GI,
    int32 CityIndex,
    bool bUsedHintThisAttempt,
    int32 ResearchReward)
{
    const FString LanguageLabel = GetTerminalLanguageLabel(Language);
    const FString SaveSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(Language);
    const FString RewardLine = ResearchReward > 0
        ? FString::Printf(TEXT("ResearchPoints awarded: +%d for clean learning momentum."), ResearchReward)
        : TEXT("ResearchPoints: no bonus this solve; clean no-hint and first-try perfect solves can earn extra RP.");
    const FString LanguageProgress = GI
        ? GI->GetLanguageProgressSummary()
        : TEXT("Language progress unavailable until the save profile is loaded.");
    const int32 DisplayCityRoute = FMath::Max(1, CityIndex + 1);
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    const bool bClearanceComplete = CompletedChallenges >= FCodeRescueCampaign::RequiredChallengesPerCity;
    const FString WorldFollowUp = bClearanceComplete
        ? FString::Printf(
            TEXT("Coding clearance for city route %d is %d/%d; survivor marker and rescue handoff are live."),
            DisplayCityRoute,
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity)
        : FString::Printf(
            TEXT("Coding clearance for city route %d is %d/%d; collect the supply cache and complete the next protected station."),
            DisplayCityRoute,
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity);

    return FString::Printf(
        TEXT("\n\nPOST-SOLVE DEBRIEF\nConcept proof: %s\nLanguage transfer (%s): %s\nWorld follow-up: %s\nSave continuity: %s run saved in %s with attempts, solves, hints, and ResearchPoints intact.\nNext practice: %s\n%s\n%s"),
        *GetConceptProofForChallenge(ChallengeId, Result),
        *LanguageLabel,
        *GetLanguageTransferForChallenge(ChallengeId, Language),
        *WorldFollowUp,
        *LanguageLabel,
        *SaveSlot,
        *GetNextPracticeRepForChallenge(ChallengeId, bUsedHintThisAttempt, Result.Score),
        *RewardLine,
        *LanguageProgress);
}

FString BuildRepairDebrief(
    const FString& ChallengeId,
    const FCodeValidationResult& Result,
    ECodingLanguage Language,
    int32 ConsecutiveFailures)
{
    const FString LanguageLabel = GetTerminalLanguageLabel(Language);
    const FString ActiveTarget = Result.FailedChecks.Num() > 0
        ? Result.FailedChecks[0]
        : TEXT("the validator did not receive a matching return value");

    return FString::Printf(
        TEXT("\n\nREPAIR DEBRIEF\nActive target: %s\nLanguage tactic (%s): %s\nNext validation move: %s\nWorld state: combat remains paused in the safehouse until the survivor route unlocks."),
        *ActiveTarget,
        *LanguageLabel,
        *GetLanguageTechniqueTip(Language),
        *GetNextMicroGoal(ChallengeId, ConsecutiveFailures));
}

FString GetRepairAdvice(const FString& ChallengeId, const FCodeValidationResult& Result, ECodingLanguage Language)
{
    if (Result.bSuccess)
    {
        return FString::Printf(
            TEXT("Takeaway: %s mastered. Apply the same pattern in the next rescue."),
            *GetConceptLabelForChallenge(ChallengeId));
    }

    const FString FirstFailed = Result.FailedChecks.Num() > 0
        ? Result.FailedChecks[0]
        : TEXT("the validator did not receive the expected output");

    FString ShapeAdvice;
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        ShapeAdvice = TEXT("Check the branch order: 15 must be tested before 3 or 5.");
    }
    else if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        ShapeAdvice = TEXT("Check lo, hi, mid updates; every loop must shrink the search range.");
    }
    else if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("traverse")))
    {
        ShapeAdvice = TEXT("Make sure current advances to current.next inside the loop.");
    }
    else if (ChallengeId.Contains(TEXT("palindrome")))
    {
        ShapeAdvice = TEXT("Compare the same characters from both ends before returning true.");
    }
    else if (ChallengeId.Contains(TEXT("reverse")))
    {
        ShapeAdvice = TEXT("Walk from the last character back to the first, or use a safe reverse helper.");
    }
    else if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        ShapeAdvice = TEXT("Keep only values where value % 2 equals 0.");
    }
    else if (ChallengeId.Contains(TEXT("lock")))
    {
        ShapeAdvice = TEXT("Return true only when every required boolean is true.");
    }
    else
    {
        ShapeAdvice = TEXT("Run the smallest example by hand, then match the returned value exactly.");
    }

    return FString::Printf(
        TEXT("Next repair: %s\nFirst failed check: %s\n%s"),
        *ShapeAdvice,
        *FirstFailed,
        *GetLanguageTechniqueTip(Language));
}
}

void UCodeTerminalWidget::OnHintClicked()
{
    if (!TerminalActor) return;
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI) return;
    if (GI->ResearchPoints <= 0)
    {
        if (HintText)
        {
            HintText->SetText(FText::FromString(TEXT("No ResearchPoints remaining. Solve a terminal without hints to earn one.")));
        }
        SetDiagnosticsState(TEXT("HINTS | no ResearchPoints remaining"), CodeRescueUI::Color::Warning());
        return;
    }

    const TArray<FString> Hints = GetHintsForChallenge(TerminalActor->Challenge.Id);
    if (HintsRevealed >= Hints.Num())
    {
        if (HintText)
        {
            HintText->SetText(FText::FromString(TEXT("All hints already revealed for this challenge.")));
        }
        SetDiagnosticsState(TEXT("HINTS | all revealed"), CodeRescueUI::Color::Warning());
        return;
    }

    GI->ResearchPoints -= 1;
    bUsedHintThisAttempt = true;

    FString Combined;
    for (int32 i = 0; i <= HintsRevealed; ++i)
    {
        Combined += Hints[i] + TEXT("\n");
    }
    HintsRevealed += 1;
    if (HintText)
    {
        HintText->SetText(FText::FromString(Combined));
    }
    SetDiagnosticsState(TEXT("HINTS | support uploaded"), CodeRescueUI::Color::Warning());
    GI->SavePersistentRun();
}

void UCodeTerminalWidget::OnBypassClicked()
{
    if (!TerminalActor)
    {
        return;
    }

    if (TerminalActor->bSolved)
    {
        SetDiagnosticsState(TEXT("SOLVED | bypass disabled"), CodeRescueUI::Color::TerminalGreenBright());
        SetTerminalOutput(OutputText, TEXT("This terminal is already solved; no bypass kit consumed."), CodeRescueUI::Color::Warning());
        return;
    }

    ACodeRescueCharacter* Player = Cast<ACodeRescueCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player || !Player->TrySpendBypassKit(1))
    {
        SetDiagnosticsState(TEXT("BYPASS KIT | none available"), CodeRescueUI::Color::Warning());
        SetTerminalOutput(OutputText, TEXT("No bypass kits available. Find the orange rescue-kit pickup, then return to this terminal."), CodeRescueUI::Color::Warning());
        return;
    }

    constexpr int32 BypassScore = 60;
    const FString ChallengeId = TerminalActor->Challenge.Id;
    const ECodingLanguage Language = TerminalActor->Challenge.Language;
    const FVector SolvedTerminalLocation = TerminalActor->GetActorLocation();
    bUsedHintThisAttempt = true;
    LastScore = BypassScore;
    SessionBestScore = FMath::Max(SessionBestScore, BypassScore);
    ++SessionAttemptCount;

    TerminalActor->MarkSolved();
    if (ValidateButton)
    {
        ValidateButton->SetIsEnabled(false);
    }
    if (ResetButton)
    {
        ResetButton->SetIsEnabled(false);
    }
    if (BypassButton)
    {
        BypassButton->SetIsEnabled(false);
    }
    UpdateRewardChoiceButtons();
    if (CodeBox)
    {
        CodeBox->SetIsReadOnly(true);
    }

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (GI)
    {
        GI->RecordTerminalAttempt(ChallengeId, BypassScore, true);
        GI->RecordConceptAttempt(GetConceptLabelForChallenge(ChallengeId), true, 0);
        GI->RecordTerminalSolved(ChallengeId, BypassScore);
        // 2026-07-04: success shows on the player's face (v2 morphs; no-op otherwise).
        UCodeRescueFacialExpressionComponent::TriggerOnActor(GetOwningPlayerPawn(), FName(TEXT("Smile")), 1.0f, 3.0f);
        GI->IncrementTerminalSolveCount();
        GI->RecordLanguageSolve(Language);
        GI->RecordLearningDebrief(
            ChallengeId,
            GetConceptLabelForChallenge(ChallengeId),
            GetTerminalLanguageLabel(Language),
            BypassScore,
            FString::Printf(
                TEXT("ASSISTED LEARNING DEBRIEF\nBypass kit completed one protected station on the selected %s track.\nConcept review: %s\nLanguage transfer: %s\nNext practice: return to this pattern later for a clean solve; bypass progress is saved, but clean-solve rewards are disabled for this station."),
                *GetTerminalLanguageLabel(Language),
                *GetConceptLabelForChallenge(ChallengeId),
                *GetLanguageTransferForChallenge(ChallengeId, Language)));
        RecordSurvivorIntelArchiveForTerminal(
            GI,
            TerminalActor->CityIndex,
            ChallengeId,
            Language,
            BypassScore,
            true);
        GI->SavePersistentRun();
    }

    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, TerminalActor->CityIndex);
    const bool bClearanceComplete = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, TerminalActor->CityIndex);
    const bool bRequiredCityChallenge =
        FCodeRescueCampaign::GetCityChallengeIds(TerminalActor->CityIndex).Contains(ChallengeId);
    if (bClearanceComplete && bRequiredCityChallenge)
    {
        if (ACodeRescueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACodeRescueGameMode>() : nullptr)
        {
            GameMode->RevealSolvedTerminalRescueRoute(
                ChallengeId,
                TerminalActor->CityIndex,
                SolvedTerminalLocation);
            GameMode->TriggerBossHorde(SolvedTerminalLocation, TerminalActor->CityIndex);
        }
    }

    SetDiagnosticsState(
        bClearanceComplete ? TEXT("BYPASS KIT | survivor route unlocked") : TEXT("BYPASS KIT | station complete"),
        CodeRescueUI::Color::TerminalGreenBright());
    SetTerminalOutput(OutputText, FString::Printf(
        TEXT("Bypass kit consumed on the selected %s track.\nStation accepted a field repair score of %d. Coding clearance: %d/%d.\n%s\nClean-solve supply rewards are disabled for this station because a rescue-kit assist was used."),
        *GetTerminalLanguageLabel(Language),
        BypassScore,
        CompletedChallenges,
        FCodeRescueCampaign::RequiredChallengesPerCity,
        bClearanceComplete
            ? TEXT("All stations complete: survivor route unlocked.")
            : TEXT("Survivor route remains locked until all ten stations are complete.")),
        CodeRescueUI::Color::TerminalGreenBright());
}

void UCodeTerminalWidget::InitializeTerminal(ACodingTerminalActor* InTerminal)
{
    TerminalActor = InTerminal;
    HintsRevealed = 0;
    bUsedHintThisAttempt = false;
    SessionAttemptCount = 0;
    PracticeRunCount = 0;
    SessionBestScore = 0;
    LastScore = 0;
    ConsecutiveFailureCount = 0;
    LastFailedCheck.Reset();
    RefreshText();
}

void UCodeTerminalWidget::SetDiagnosticsState(const FString& Label, const FLinearColor& Color)
{
    if (DiagnosticsHeaderText)
    {
        CodeRescueUI::StyleText(DiagnosticsHeaderText, CodeRescueUI::EType::Caption, Color, false);
        DiagnosticsHeaderText->SetText(FText::FromString(FString::Printf(TEXT("DIAGNOSTICS | %s"), *Label)));
    }
    if (OutputFrame)
    {
        CodeRescueUI::StylePanel(OutputFrame, TerminalEditorFill(), FMargin(CodeRescueUI::Space::M, CodeRescueUI::Space::S));
    }
}

void UCodeTerminalWidget::RefreshText()
{
    if (!TerminalActor || !TitleText || !BriefText || !CodeBox || !MATLABButton) return;

    SetupPredictionDrill();   // 2026-07-04 item 27: challenge-specific predict-the-output row

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        MirrorTerminalThemeFromSettings(GI);
        if (PanelFrame)
        {
            CodeRescueUI::StylePanel(
                PanelFrame,
                TerminalPanelFill(),
                FMargin(CodeRescueUI::Space::XL, CodeRescueUI::Space::L));
        }
        if (CodeEditorFrame)
        {
            CodeRescueUI::StylePanel(CodeEditorFrame, TerminalEditorFill(), FMargin(2.0f, 2.0f));
        }
        StyleTerminalCodeEditor(CodeBox);
        TerminalActor->Challenge.Language = GI->SelectedLanguage;
        TerminalActor->Challenge.StarterCode = MakeStarterForLanguage(TerminalActor->Challenge.Id, GI->SelectedLanguage);
        const FCodeRescueChallenge LearningChallenge = SelectDataDrivenChallengeForTerminal(TerminalActor, GI->SelectedLanguage);
        if (LearningChallenge.IsValid())
        {
            const FCodeRescueTeachPayload TeachPayload = UCodeRescueLearningLibrary::BuildTeachPayload(
                LearningChallenge,
                GetTerminalLanguageLabel(GI->SelectedLanguage),
                GI->bSimplifiedInputHints);
            if (IsDataDrivenStarterSignatureCompatible(
                TeachPayload.StarterCode,
                TerminalActor->Challenge.Id,
                GI->SelectedLanguage))
            {
                TerminalActor->Challenge.StarterCode = TeachPayload.StarterCode;
            }
        }
        const FString LanguageLabel = GetTerminalLanguageLabel(GI->SelectedLanguage);
        const FString LanguageSaveSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(GI->SelectedLanguage);
        const bool bToolchainAvailable = UCodeRunnerLibrary::IsLanguageAvailable(GI->SelectedLanguage);
        const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, TerminalActor->CityIndex);
        const bool bClearanceComplete = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, TerminalActor->CityIndex);
        if (TerminalStatusText)
        {
            TerminalStatusText->SetText(FText::FromString(FString::Printf(
                TEXT("PROTECTED CODING CONCOURSE | Track %s | Station %s | Clearance %d/%d\n%s | %s"),
                *LanguageLabel,
                TerminalActor->bSolved ? TEXT("solved") : TEXT("active"),
                CompletedChallenges,
                FCodeRescueCampaign::RequiredChallengesPerCity,
                bClearanceComplete ? TEXT("survivor route unlocked") : TEXT("survivor route locked"),
                *TerminalToolchainStateLine(GI->SelectedLanguage))));
            CodeRescueUI::StyleText(
                TerminalStatusText,
                CodeRescueUI::EType::Caption,
                bToolchainAvailable ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::Warning(),
                false);
        }
        if (LanguageLockText)
        {
            LanguageLockText->SetText(FText::FromString(FString::Printf(
                TEXT("LOCKED TRACK: %s | Save profile: %s\nProgress, hints, attempts, and solves remain in this language. Resume it from the start screen next launch."),
                *LanguageLabel,
                *LanguageSaveSlot)));
            CodeRescueUI::StyleText(LanguageLockText, CodeRescueUI::EType::Subheading, CodeRescueUI::Color::TerminalGreenBright());
        }
        if (LearningStatusText)
        {
            CodeRescueUI::StyleText(LearningStatusText, CodeRescueUI::EType::BodySmall, CodeRescueUI::Color::TerminalGreen(), false);
            LearningStatusText->SetText(FText::FromString(FString::Printf(
                TEXT("Concept: %s | %s\n%s\n%s"),
                *GetConceptLabelForChallenge(TerminalActor->Challenge.Id),
                *GI->GetLearningProgressSummary(),
                *GI->GetLanguageProgressSummary(),
                *GetRewardPreview(GI, bUsedHintThisAttempt))));
        }
    }

    // 2026-07-05 first-level coherence: when a data-driven lesson is active, the WHOLE
    // panel presents THAT challenge (title, brief, checklist). Previously the header
    // shouted the legacy archetype ("write totalPower…") while the lesson node below
    // taught a different function — the very first terminal a player ever opens
    // contradicted itself, and the redundant archetype wall pushed the code panel
    // off-screen at smaller window heights.
    const FCodeRescueChallenge ActiveLearning = SelectDataDrivenChallengeForTerminal(
        TerminalActor, TerminalActor->Challenge.Language);
    const bool bLearningActive = ActiveLearning.IsValid();

    if (bLearningActive)
    {
        FString CityPrefix = TerminalActor->Challenge.Title;
        int32 ColonIdx = INDEX_NONE;
        if (CityPrefix.FindChar(TEXT(':'), ColonIdx) && ColonIdx > 0)
        {
            CityPrefix = CityPrefix.Left(ColonIdx);
        }
        TitleText->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *CityPrefix, *ActiveLearning.Title)));
    }
    else
    {
        TitleText->SetText(FText::FromString(TerminalActor->Challenge.Title));
    }
    CodeRescueUI::StyleText(TitleText, CodeRescueUI::EType::Title, CodeRescueUI::Color::AccentAmber());
    // 2026-07-01 pedagogy readability: the campaign MissionBrief carries internal dev-plan
    // paragraphs (progression/character/flow/accessibility/QA). Students only need the
    // fiction, the lesson, the worked example, and the tests - filter the rest out here.
    if (bLearningActive)
    {
        TArray<FString> BriefLines;
        TerminalActor->Challenge.MissionBrief.ParseIntoArrayLines(BriefLines, false);
        FString Fiction = BriefLines.Num() > 0 ? BriefLines[0].TrimStartAndEnd() : FString();
        const int32 WriteIdx = Fiction.Find(TEXT(". Write"));
        if (WriteIdx != INDEX_NONE)
        {
            Fiction = Fiction.Left(WriteIdx + 1);   // keep the city hook, drop the archetype instruction
        }
        FString StageLine;
        for (const FString& BriefLine : BriefLines)
        {
            const FString Trimmed = BriefLine.TrimStartAndEnd();
            if (Trimmed.StartsWith(TEXT("Stage ")))
            {
                StageLine = Trimmed;
                break;
            }
        }
        FString StudentBrief = Fiction;
        if (!StageLine.IsEmpty())
        {
            StudentBrief += TEXT("\n") + StageLine;
        }
        StudentBrief += FString::Printf(
            TEXT("\n\nTHIS TERMINAL'S CODING TASK — %s\n%s\nThe DATA-DRIVEN LESSON NODE below teaches exactly this challenge, and the code panel already holds its starter."),
            *ActiveLearning.Title,
            *ActiveLearning.Prompt);
        BriefText->SetText(FText::FromString(StudentBrief));
    }
    else
    {
        TArray<FString> BriefLines;
        TerminalActor->Challenge.MissionBrief.ParseIntoArrayLines(BriefLines, false);
        FString StudentBrief;
        for (const FString& BriefLine : BriefLines)
        {
            const FString Trimmed = BriefLine.TrimStartAndEnd();
            if (Trimmed.StartsWith(TEXT("Progression plan:")) ||
                Trimmed.StartsWith(TEXT("Character plan:")) ||
                Trimmed.StartsWith(TEXT("Flow plan:")) ||
                Trimmed.StartsWith(TEXT("Accessibility and polish plan:")) ||
                Trimmed.StartsWith(TEXT("QA plan:")) ||
                Trimmed.StartsWith(TEXT("Architecture:")))
            {
                continue;
            }
            StudentBrief += BriefLine + TEXT("\n");
        }
        BriefText->SetText(FText::FromString(StudentBrief.TrimEnd()));
    }
    CodeRescueUI::StyleText(BriefText, CodeRescueUI::EType::Body, CodeRescueUI::Color::TextPrimary(), false);
    FString StarterCode = TerminalActor->Challenge.StarterCode;
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        const FCodeRescueChallenge& LearningChallenge = ActiveLearning;
        if (LearningChallenge.IsValid())
        {
            const FCodeRescueTeachPayload TeachPayload = UCodeRescueLearningLibrary::BuildTeachPayload(
                LearningChallenge,
                GetTerminalLanguageLabel(TerminalActor->Challenge.Language),
                GI->bSimplifiedInputHints);
            if (IsDataDrivenStarterSignatureCompatible(
                TeachPayload.StarterCode,
                TerminalActor->Challenge.Id,
                TerminalActor->Challenge.Language))
            {
                StarterCode = TeachPayload.StarterCode;
            }
        }
    }

    CodeBox->SetText(FText::FromString(StarterCode));
    if (ChecklistText)
    {
        const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
        const FString DataDrivenCoach = BuildDataDrivenTeachApplySummary(TerminalActor, GI);
        CodeRescueUI::StyleText(ChecklistText, CodeRescueUI::EType::Caption, CodeRescueUI::Color::TextSecondary(), false);
        // 2026-07-05 first-level coherence: with an active lesson, the archetype
        // checklist/coach lines (they name the WRONG function) give way to one
        // universal checklist + the data-driven coach section.
        ChecklistText->SetText(FText::FromString(
            bLearningActive
            ? (TEXT("Checklist: read the micro-lesson | trace the visible test by hand | keep the exact signature | RETURN the value (never just print it)")
               + (DataDrivenCoach.IsEmpty() ? FString() : TEXT("\n\n") + DataDrivenCoach))
            : (GetChecklistForChallenge(TerminalActor->Challenge.Id, TerminalActor->Challenge.Language)
               + TEXT("\n") + GetTerminalLearningCoach(TerminalActor->Challenge.Id)
               + (DataDrivenCoach.IsEmpty() ? FString() : TEXT("\n\n") + DataDrivenCoach))));
    }

    // Dependency banner: probe the toolchain and color-code the result. Green
    // = external compiler available. Amber = falling back to in-engine
    // static-analysis validator (still scores + completes the mission).
    if (DependencyBanner)
    {
        const ECodingLanguage Lang = TerminalActor->Challenge.Language;
        const bool bAvailable = UCodeRunnerLibrary::IsLanguageAvailable(Lang);
        DependencyBanner->SetText(FText::FromString(UCodeRunnerLibrary::GetLanguageDependencyMessage(Lang)));
        CodeRescueUI::StyleText(
            DependencyBanner,
            CodeRescueUI::EType::BodySmall,
            bAvailable ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::Warning(),
            false);
    }

    if (TerminalActor->Challenge.Language == ECodingLanguage::MATLAB)
    {
        MATLABButton->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        MATLABButton->SetVisibility(ESlateVisibility::Collapsed);
    }

    const bool bAlreadySolved = TerminalActor->bSolved;
    if (ValidateButton)
    {
        ValidateButton->SetIsEnabled(!bAlreadySolved);
    }
    if (PracticeButton)
    {
        PracticeButton->SetIsEnabled(!bAlreadySolved);
    }
    if (ResetButton)
    {
        ResetButton->SetIsEnabled(!bAlreadySolved);
    }
    if (BypassButton)
    {
        BypassButton->SetIsEnabled(!bAlreadySolved);
    }
    if (CodeBox)
    {
        CodeBox->SetIsReadOnly(bAlreadySolved);
    }
    UpdateRewardChoiceButtons();
    SetDiagnosticsState(
        bAlreadySolved ? TEXT("SOLVED | survivor intel uploaded") : TEXT("READY | awaiting validation"),
        bAlreadySolved ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::Warning());
}

void UCodeTerminalWidget::ResetToStarterCode()
{
    if (!TerminalActor || !CodeBox)
    {
        return;
    }
    if (TerminalActor->bSolved)
    {
        SetDiagnosticsState(TEXT("SOLVED | reset disabled"), CodeRescueUI::Color::TerminalGreenBright());
        SetTerminalOutput(OutputText, TEXT("This terminal is already solved; starter reset is disabled."), CodeRescueUI::Color::Warning());
        return;
    }

    CodeBox->SetText(FText::FromString(TerminalActor->Challenge.StarterCode));
    LastFailedCheck.Reset();
    SetDiagnosticsState(TEXT("READY | starter restored"), CodeRescueUI::Color::Warning());
    SetTerminalOutput(OutputText, FString::Printf(
        TEXT("Starter code restored. Session attempts remain at %d so the learning log stays honest.\n%s"),
        SessionAttemptCount,
        *GetChecklistForChallenge(TerminalActor->Challenge.Id, TerminalActor->Challenge.Language)),
        CodeRescueUI::Color::Warning());
}

void UCodeTerminalWidget::OnValidateClicked()
{
    RunValidation(false);
}

void UCodeTerminalWidget::OnPracticeClicked()
{
    RunValidation(true);
}

void UCodeTerminalWidget::RunValidation(bool bPracticeOnly)
{
    if (!TerminalActor || !CodeBox) return;
    if (TerminalActor->bSolved)
    {
        const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
        const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, TerminalActor->CityIndex);
        SetDiagnosticsState(
            FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, TerminalActor->CityIndex)
                ? TEXT("SOLVED | survivor route unlocked")
                : TEXT("SOLVED | station progress saved"),
            CodeRescueUI::Color::TerminalGreenBright());
        FString SolvedOutput = FString::Printf(
            TEXT("This station is already solved.\nCoding clearance: %d/%d\nSession best score: %d\nNext growth step: tackle the next protected station."),
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity,
            SessionBestScore);
        if (GI)
        {
            SolvedOutput += TEXT("\n\n") + GI->GetTerminalRewardChoiceSummary(TerminalActor->Challenge.Id);
        }
        SetTerminalOutput(OutputText, SolvedOutput, CodeRescueUI::Color::TerminalGreenBright());
        if (ValidateButton)
        {
            ValidateButton->SetIsEnabled(false);
        }
        if (PracticeButton)
        {
            PracticeButton->SetIsEnabled(false);
        }
        UpdateRewardChoiceButtons();
        CodeBox->SetIsReadOnly(true);
        return;
    }

    const FString UserCode = CodeBox->GetText().ToString();
    const int32 NonWhitespaceCharacters = CountNonWhitespaceCharacters(UserCode);
    if (NonWhitespaceCharacters == 0)
    {
        SetDiagnosticsState(TEXT("READY | no code detected"), CodeRescueUI::Color::Warning());
        SetTerminalOutput(OutputText, FString::Printf(
            TEXT("No code detected yet.\nMicro-goal: start from the required signature, then solve the smallest visible example.\n%s\n%s"),
            *GetChecklistForChallenge(TerminalActor->Challenge.Id, TerminalActor->Challenge.Language),
            *GetPredictionPromptForChallenge(TerminalActor->Challenge.Id)),
            CodeRescueUI::Color::Warning());
        return;
    }
    if (UserCode.Len() > 20000)
    {
        SetDiagnosticsState(TEXT("LIMIT | code too long"), CodeRescueUI::Color::DangerBright());
        SetTerminalOutput(OutputText, TEXT("Code is over the 20,000 character terminal limit. Trim experiments or reset to starter before validating."), CodeRescueUI::Color::DangerBright());
        return;
    }

    if (bPracticeOnly)
    {
        ++PracticeRunCount;
    }
    else
    {
        ++SessionAttemptCount;
    }
    const int32 DisplayAttemptCount = bPracticeOnly ? PracticeRunCount : SessionAttemptCount;
    const bool bFirstTry = !bPracticeOnly && SessionAttemptCount == 1;
    const FCodeRescueChallenge LearningChallenge = SelectDataDrivenChallengeForTerminal(
        TerminalActor,
        TerminalActor->Challenge.Language);
    const FChallengeSpec ValidationChallenge = BuildDataDrivenRuntimeChallengeForValidation(
        TerminalActor->Challenge,
        LearningChallenge);
    FCodeValidationResult Result = UCodeRunnerLibrary::ValidateChallenge(ValidationChallenge, UserCode);
    const FString FirstFailedCheck = Result.FailedChecks.Num() > 0 ? Result.FailedChecks[0] : FString();
    const int32 PreviousScore = LastScore;
    LastScore = Result.Score;
    SessionBestScore = FMath::Max(SessionBestScore, Result.Score);
    if (Result.bSuccess)
    {
        ConsecutiveFailureCount = 0;
        LastFailedCheck.Reset();
    }
    else
    {
        ++ConsecutiveFailureCount;
        LastFailedCheck = FirstFailedCheck;
    }
    const bool bLearningScaffoldOffered = LearningChallenge.IsValid()
        && !Result.bSuccess
        && UCodeRescueLearningLibrary::ShouldOfferScaffold(ConsecutiveFailureCount, 3);
    if (LearningChallenge.IsValid())
    {
        UCodeRescueLearningLibrary::RecordAttempt(
            LearningChallenge.Concept,
            LearningChallenge.Id,
            Result.bSuccess,
            bUsedHintThisAttempt,
            bLearningScaffoldOffered,
            0.0f,
            FirstFailedCheck.IsEmpty() ? Result.Summary : FirstFailedCheck);
    }

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    int32 BonusScore = 0;
    if (GI && !bPracticeOnly)
    {
        BonusScore = GI->RecordValidationAttempt(
            TerminalActor->Challenge.Language,
            Result.bSuccess,
            Result.Score,
            bUsedHintThisAttempt,
            bFirstTry);
        GI->LogCodeAttemptDetailed(
            TerminalActor->Challenge.Id,
            TerminalActor->Challenge.Language,
            CodeBox->GetText().ToString(),
            Result.bSuccess,
            Result.Score,
            DisplayAttemptCount,
            HintsRevealed,
            FirstFailedCheck);
        GI->RecordTerminalAttempt(TerminalActor->Challenge.Id, Result.Score, Result.bSuccess);
        GI->RecordConceptAttempt(GetConceptLabelForChallenge(TerminalActor->Challenge.Id), Result.bSuccess, 0);
    }

    int32 ResearchReward = 0;
    int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, TerminalActor->CityIndex);
    int32 SpawnedSupplyPickups = 0;
    bool bFirstTimeChallengeCompletion = false;
    bool bClearanceComplete = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, TerminalActor->CityIndex);

    if (Result.bSuccess && !bPracticeOnly)
    {
        const FVector SolvedTerminalLocation = TerminalActor->GetActorLocation();
        bFirstTimeChallengeCompletion = !GI || !GI->SolvedTerminalIds.Contains(TerminalActor->Challenge.Id);
        TerminalActor->MarkSolved();
        if (GI)
        {
            GI->RecordTerminalSolved(TerminalActor->Challenge.Id, Result.Score);
            // 2026-07-04: success shows on the player's face (v2 morphs; no-op otherwise).
            UCodeRescueFacialExpressionComponent::TriggerOnActor(GetOwningPlayerPawn(), FName(TEXT("Smile")), 1.0f, 3.0f);
            GI->IncrementTerminalSolveCount();   // #15 scoreboard
            GI->RecordLanguageSolve(TerminalActor->Challenge.Language);   // #50
            GI->MarkTerminalRewardChoiceEligible(TerminalActor->Challenge.Id);
            // Reward independent learning without making hints feel punitive.
            if (!bUsedHintThisAttempt)
            {
                ResearchReward += 1;
            }
            if (!bUsedHintThisAttempt && bFirstTry && Result.Score >= 100)
            {
                ResearchReward += 1;
            }
            if (GI->CurrentLearningStreak > 0 && (GI->CurrentLearningStreak % 5) == 0)
            {
                ResearchReward += 1;
            }
            if (ResearchReward > 0)
            {
                GI->ResearchPoints += ResearchReward;
                GI->SavePersistentRun();
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
                        FString::Printf(TEXT("+%d ResearchPoint%s for clean learning"),
                            ResearchReward,
                            ResearchReward == 1 ? TEXT("") : TEXT("s")));
                }
            }
            CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, TerminalActor->CityIndex);
            bClearanceComplete = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, TerminalActor->CityIndex);
        }
        if (ACodeRescueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACodeRescueGameMode>() : nullptr)
        {
            if (bFirstTimeChallengeCompletion)
            {
                SpawnedSupplyPickups = GameMode->SpawnChallengeCompletionSupplyCache(
                    TerminalActor->Challenge.Id,
                    TerminalActor->CityIndex,
                    SolvedTerminalLocation);
            }
            const bool bRequiredCityChallenge = FCodeRescueCampaign::GetCityChallengeIds(
                TerminalActor->CityIndex).Contains(TerminalActor->Challenge.Id);
            if (bFirstTimeChallengeCompletion && bClearanceComplete && bRequiredCityChallenge)
            {
                GameMode->RevealSolvedTerminalRescueRoute(
                    TerminalActor->Challenge.Id,
                    TerminalActor->CityIndex,
                    SolvedTerminalLocation,
                    false);
                GameMode->TriggerBossHorde(SolvedTerminalLocation, TerminalActor->CityIndex);
            }
        }
        if (ValidateButton)
        {
            ValidateButton->SetIsEnabled(false);
        }
        if (PracticeButton)
        {
            PracticeButton->SetIsEnabled(false);
        }
        UpdateRewardChoiceButtons();
        CodeBox->SetIsReadOnly(true);
    }
    SetDiagnosticsState(
        bPracticeOnly
            ? (Result.bSuccess ? TEXT("PRACTICE PASS | no route advanced") : TEXT("PRACTICE REPAIR | no save penalty"))
            : (Result.bSuccess
                ? (bClearanceComplete ? TEXT("PASS | survivor route unlocked") : TEXT("PASS | coding station saved"))
                : TEXT("REPAIR | failed checks detected")),
        TerminalOutputColor(Result.bSuccess));

    FString Output = FString::Printf(
        TEXT("%s%s\nScore: %d   Best this session: %d   Previous score: %d\nAttempt: %d   Practice runs: %d   Hints used: %d   Mastery Grade: %s\nConcept: %s\nDeclared tests: %d total, %d passed\n\nPASSED:\n"),
        bPracticeOnly ? TEXT("PRACTICE RUN - NO SAVE ADVANCE\n") : TEXT(""),
        *Result.Summary,
        Result.Score,
        SessionBestScore,
        PreviousScore,
        DisplayAttemptCount,
        PracticeRunCount,
        HintsRevealed,
        *GetMasteryGrade(Result, DisplayAttemptCount, HintsRevealed),
        *GetConceptLabelForChallenge(TerminalActor->Challenge.Id),
        Result.TotalTestCases,
        Result.PassedTestCases);
    for (const FString& Check : Result.PassedChecks)
    {
        Output += TEXT("[PASS] ") + Check + TEXT("\n");
    }
    Output += TEXT("\nFAILED:\n");
    if (Result.FailedChecks.Num() == 0)
    {
        Output += TEXT("[NONE]\n");
    }
    else
    {
        for (const FString& Check : Result.FailedChecks)
        {
            Output += TEXT("[FIX] ") + Check + TEXT("\n");
        }
    }

    Output += TEXT("\n") + GetRepairAdvice(TerminalActor->Challenge.Id, Result, TerminalActor->Challenge.Language);
    if (LearningChallenge.IsValid())
    {
        Output += FString::Printf(
            TEXT("\n\nDATA-DRIVEN CURRICULUM NODE\nNode: %s | %s\nTelemetry: attempt recorded to %s\nWorld effect: %s"),
            *LearningChallenge.Id,
            *LearningChallenge.Concept,
            *UCodeRescueLearningLibrary::GetLearningTelemetryPath(),
            *UCodeRescueLearningLibrary::GetWorldEffect(LearningChallenge));
        Output += BuildDataDrivenValidationOracleSummary(LearningChallenge);
    }
    if (Result.bSuccess && bPracticeOnly)
    {
        Output += FString::Printf(
            TEXT("\n\nPRACTICE-ONLY DEBRIEF\nNo station solve, route progress, ResearchPoints, coding score, attempts, streaks, survivor intel archive, or save profile were advanced.\nUse VALIDATE CODE when ready to commit this selected-language %s solution toward the ten-station clearance.\nNext practice: %s"),
            *GetTerminalLanguageLabel(TerminalActor->Challenge.Language),
            *GetNextPracticeRepForChallenge(TerminalActor->Challenge.Id, bUsedHintThisAttempt, Result.Score));
    }
    else if (Result.bSuccess)
    {
        const FString PostSolveDebrief = BuildPostSolveAfterActionDebrief(
            TerminalActor->Challenge.Id,
            Result,
            TerminalActor->Challenge.Language,
            GI,
            TerminalActor->CityIndex,
            bUsedHintThisAttempt,
            ResearchReward);
        Output += PostSolveDebrief;
        if (LearningChallenge.IsValid())
        {
            Output += TEXT("\n\nDATA-DRIVEN REINFORCEMENT\n")
                + UCodeRescueLearningLibrary::BuildPostSolve(LearningChallenge);
        }
        if (GI)
        {
            GI->RecordLearningDebrief(
                TerminalActor->Challenge.Id,
                GetConceptLabelForChallenge(TerminalActor->Challenge.Id),
                GetTerminalLanguageLabel(TerminalActor->Challenge.Language),
                Result.Score,
                PostSolveDebrief);
            RecordSurvivorIntelArchiveForTerminal(
                GI,
                TerminalActor->CityIndex,
                TerminalActor->Challenge.Id,
                TerminalActor->Challenge.Language,
                Result.Score,
                false);
        }
    }
    else
    {
        Output += BuildRepairDebrief(
            TerminalActor->Challenge.Id,
            Result,
            TerminalActor->Challenge.Language,
            ConsecutiveFailureCount);
        if (bLearningScaffoldOffered)
        {
            Output += TEXT("\n\nADAPTIVE SCAFFOLD (never wall-block)\n");
            Output += UCodeRescueLearningLibrary::BuildScaffold(
                LearningChallenge,
                GetTerminalLanguageLabel(TerminalActor->Challenge.Language));
        }
    }
    if (!Result.bSuccess)
    {
        Output += TEXT("\nMicro-goal: ") + GetNextMicroGoal(TerminalActor->Challenge.Id, ConsecutiveFailureCount);
        if (!LastFailedCheck.IsEmpty())
        {
            Output += TEXT("\nLast failed check this session: ") + LastFailedCheck;
        }
    }
    if (BonusScore > 0 || ResearchReward > 0)
    {
        Output += FString::Printf(
            TEXT("\n\nRewards: +%d coding score bonus, +%d ResearchPoint%s"),
            BonusScore,
            ResearchReward,
            ResearchReward == 1 ? TEXT("") : TEXT("s"));
    }
    if (Result.bSuccess && !bPracticeOnly)
    {
        Output += FString::Printf(
            TEXT("\n\nCoding Clearance: %d/%d | Supply pickups dropped: %d\n%s"),
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity,
            SpawnedSupplyPickups,
            bClearanceComplete
                ? TEXT("All stations complete. Survivor whereabouts and the rescue route are now live.")
                : TEXT("Progress saved to this language. Complete the next protected station before approaching the survivor."));
        if (GI)
        {
            Output += TEXT("\n") + GI->GetTerminalRewardChoiceSummary(TerminalActor->Challenge.Id);
        }
    }
    if (GI)
    {
        const FString ActiveLanguageLabel = GetTerminalLanguageLabel(TerminalActor->Challenge.Language);
        const FString ActiveLanguageSaveSlot = UCodeRescueGameInstance::MakeLanguageSaveSlotName(TerminalActor->Challenge.Language);
        if (bPracticeOnly)
        {
            Output += FString::Printf(
                TEXT("\n\nPractice Run Lock:\nTrack: %s only\nSave profile unchanged: %s\nStart screen resume keeps this same %s profile; practice-only terminal option does not advance campaign progress."),
                *ActiveLanguageLabel,
                *ActiveLanguageSaveSlot,
                *ActiveLanguageLabel);
        }
        else
        {
            Output += FString::Printf(
                TEXT("\n\nLanguage Run Lock:\nTrack: %s only\nSave profile updated: %s\nStart screen resume uses this same %s profile."),
                *ActiveLanguageLabel,
                *ActiveLanguageSaveSlot,
                *ActiveLanguageLabel);
        }
        Output += TEXT("\n\nLearning Profile:\n") + GI->GetLearningProgressSummary();
        Output += TEXT("\nLanguage Practice:\n") + GI->GetLanguageProgressSummary();
        Output += TEXT("\n") + GetRewardPreview(GI, bUsedHintThisAttempt);
        if (Result.bSuccess && !bPracticeOnly)
        {
            // 2026-07-04 (top-50 item 33): one-line reflective debrief — metacognition
            // after every solve. Rotates deterministically so repeats stay fresh.
            static const TCHAR* DebriefPrompts[] = {
                TEXT("In one sentence: what was the KEY idea that made this work?"),
                TEXT("Which test would have caught your first mistake earliest?"),
                TEXT("If a teammate hit this challenge, what single hint would you give?"),
                TEXT("What would break first if the input doubled in size?"),
                TEXT("Name the concept you just used - where else in the city could it apply?"),
            };
            const int32 PromptIdx = FMath::Abs(static_cast<int32>(GetTypeHash(TerminalActor->Challenge.Id))
                + SessionAttemptCount) % UE_ARRAY_COUNT(DebriefPrompts);
            Output += FString::Printf(TEXT("\n\nREFLECT (say it out loud or jot it): %s"), DebriefPrompts[PromptIdx]);
        }
        if (LearningStatusText)
        {
            LearningStatusText->SetText(FText::FromString(FString::Printf(
                TEXT("Concept: %s | %s\n%s\n%s"),
                *GetConceptLabelForChallenge(TerminalActor->Challenge.Id),
                *GI->GetLearningProgressSummary(),
                *GI->GetLanguageProgressSummary(),
                *GetRewardPreview(GI, bUsedHintThisAttempt))));
        }
    }
    Output += FString::Printf(
        TEXT("\n\nTerminal Session:\nAttempts: %d   Practice runs: %d   Best: %d   Consecutive fixes needed: %d   Clean solve still available: %s"),
        SessionAttemptCount,
        PracticeRunCount,
        SessionBestScore,
        ConsecutiveFailureCount,
        bUsedHintThisAttempt ? TEXT("no") : TEXT("yes"));
    if (bPracticeOnly)
    {
        Output += TEXT("\nPractice safety: no route opened, no save write, no ResearchPoints awarded, and live validation remains available.");
    }
    Output += TEXT("\n") + GetHiddenTestDebriefForChallenge(TerminalActor->Challenge.Id);
    Output += TEXT("\n") + GetVisualDebuggerCueForChallenge(TerminalActor->Challenge.Id);
    Output += TEXT("\n\nSTDOUT:\n") + FormatStreamForTerminal(Result.StdOut) + TEXT("\nSTDERR:\n") + FormatStreamForTerminal(Result.StdErr);

    SetTerminalOutput(OutputText, Output, TerminalOutputColor(Result.bSuccess));

    // 2026-07-06: keep the terminal driveable from the keyboard after a
    // validation click (Escape closes, Ctrl hotkeys work) and bring the fresh
    // results panel to the top of the scrolling column so the score card,
    // PASSED/FIX list, and the action buttons right below it are all in view.
    SetKeyboardFocus();
    if (TerminalScroll && OutputFrame)
    {
        TerminalScroll->ScrollWidgetIntoView(OutputFrame, true);
    }
}

void UCodeTerminalWidget::OnResetStarterClicked()
{
    ResetToStarterCode();
}

void UCodeTerminalWidget::OnRewardResearchClicked()
{
    ClaimRewardChoice(TEXT("research"));
}

void UCodeTerminalWidget::OnRewardFieldKitClicked()
{
    ClaimRewardChoice(TEXT("fieldkit"));
}

void UCodeTerminalWidget::OnRewardCraftingClicked()
{
    ClaimRewardChoice(TEXT("crafting"));
}

void UCodeTerminalWidget::ClaimRewardChoice(const FString& ChoiceId)
{
    if (!TerminalActor)
    {
        return;
    }

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        SetDiagnosticsState(TEXT("REWARD CHOICE | profile unavailable"), CodeRescueUI::Color::Warning());
        SetTerminalOutput(OutputText, TEXT("Reward choice unavailable: selected-language profile is not loaded."), CodeRescueUI::Color::Warning());
        return;
    }

    ACodeRescueCharacter* Player = Cast<ACodeRescueCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    const bool bClaimed = GI->ClaimTerminalRewardChoice(TerminalActor->Challenge.Id, ChoiceId, Player);
    UpdateRewardChoiceButtons();
    if (bClaimed)
    {
        SetDiagnosticsState(TEXT("REWARD CHOICE | saved"), CodeRescueUI::Color::TerminalGreenBright());
        SetTerminalOutput(OutputText, GI->GetTerminalRewardChoiceSummary(TerminalActor->Challenge.Id), CodeRescueUI::Color::TerminalGreenBright());
    }
    else
    {
        SetDiagnosticsState(TEXT("REWARD CHOICE | unavailable"), CodeRescueUI::Color::Warning());
        SetTerminalOutput(OutputText, GI->GetTerminalRewardChoiceSummary(TerminalActor->Challenge.Id), CodeRescueUI::Color::Warning());
    }
}

void UCodeTerminalWidget::UpdateRewardChoiceButtons()
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bRewardAvailable = TerminalActor
        && GI
        && TerminalActor->bSolved
        && GI->IsTerminalRewardChoiceAvailable(TerminalActor->Challenge.Id);

    if (RewardResearchButton)
    {
        RewardResearchButton->SetIsEnabled(bRewardAvailable);
    }
    if (RewardFieldKitButton)
    {
        RewardFieldKitButton->SetIsEnabled(bRewardAvailable);
    }
    if (RewardCraftingButton)
    {
        RewardCraftingButton->SetIsEnabled(bRewardAvailable);
    }
}

void UCodeTerminalWidget::OnMATLABClicked()
{
    const bool bLaunched = UCodeRunnerLibrary::LaunchMATLABDesktop();
    SetDiagnosticsState(
        bLaunched ? TEXT("MATLAB | launch requested") : TEXT("MATLAB | launch unavailable"),
        bLaunched ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::DangerBright());
    SetTerminalOutput(
        OutputText,
        bLaunched ? TEXT("MATLAB desktop launch requested.") : TEXT("MATLAB could not be launched. Set MATLAB_BIN, MATLABROOT, or ensure 'matlab' is on PATH."),
        bLaunched ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::DangerBright());
}

void UCodeTerminalWidget::OnCloseClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }
    // Always release the global UI lock so polled gameplay keys resume,
    // even if we somehow got here without a valid PlayerController.
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    RemoveFromParent();
}

FReply UCodeTerminalWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const bool bCommandModifier = InKeyEvent.IsControlDown() || InKeyEvent.IsCommandDown();
    // #31 — Ctrl+H reveals the next hint. (Plain H would conflict with code typing.)
    if (InKeyEvent.GetKey() == EKeys::H && bCommandModifier)
    {
        OnHintClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::B && bCommandModifier)
    {
        OnBypassClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::P && bCommandModifier)
    {
        OnPracticeClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Enter && bCommandModifier)
    {
        OnValidateClicked();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::R && bCommandModifier)
    {
        ResetToStarterCode();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        OnCloseClicked();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCodeTerminalWidget::NativeDestruct()
{
    // Final safety: if the widget tears down for any reason (level change,
    // owning PC reset, etc.) we MUST release the global UI lock or polled
    // gameplay input will stay disabled forever.
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::SetGamePaused(GetWorld(), false);
    Super::NativeDestruct();
}

// ---------------------------------------------------------------------------
// 2026-07-04 (top-50 item 27): interactive predict-the-output drill.

void UCodeTerminalWidget::SetupPredictionDrill()
{
    bPredictionAnswered = false;
    PredictionCorrectIndex = -1;
    if (!PredictionDrillRow || !TerminalActor)
    {
        return;
    }
    const FCodeRescueChallenge Learning = SelectDataDrivenChallengeForTerminal(
        TerminalActor, TerminalActor->Challenge.Language);
    if (!Learning.IsValid() || Learning.VisibleTests.Num() == 0 || TerminalActor->bSolved)
    {
        PredictionDrillRow->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    const FCodeRescueTestCase& First = Learning.VisibleTests[0];
    PredictionConcept = Learning.Concept;
    PredictionChallengeId = Learning.Id;

    // Distractors: plausible near-misses derived from the correct literal's shape.
    const FString Correct = First.Out.TrimStartAndEnd();
    FString AltA, AltB;
    const FString Lower = Correct.ToLower();
    if (Lower == TEXT("true") || Lower == TEXT("false"))
    {
        AltA = (Lower == TEXT("true")) ? TEXT("false") : TEXT("true");
        AltB = TEXT("no output");
    }
    else if (Correct.StartsWith(TEXT("[")))
    {
        FString Reversed = Correct.Mid(1, Correct.Len() - 2).TrimStartAndEnd();
        TArray<FString> Parts;
        Reversed.ParseIntoArray(Parts, TEXT(","), true);
        Algo::Reverse(Parts);
        for (FString& P : Parts) { P.TrimStartAndEndInline(); }
        AltA = TEXT("[") + FString::Join(Parts, TEXT(", ")) + TEXT("]");
        if (AltA == Correct) { AltA = TEXT("[]"); }
        AltB = Parts.Num() > 0 ? (TEXT("[") + Parts[0] + TEXT("]")) : TEXT("[0]");
        if (AltB == Correct) { AltB = TEXT("[-1]"); }
    }
    else if ((Correct.StartsWith(TEXT("\"")) && Correct.EndsWith(TEXT("\""))) || (Correct.StartsWith(TEXT("'")) && Correct.EndsWith(TEXT("'"))))
    {
        const FString Inner = Correct.Mid(1, Correct.Len() - 2);
        FString InnerReversed;
        for (int32 CharIdx = Inner.Len() - 1; CharIdx >= 0; --CharIdx)
        {
            InnerReversed.AppendChar(Inner[CharIdx]);
        }
        AltA = TEXT("\"") + InnerReversed + TEXT("\"");
        if (AltA == Correct) { AltA = TEXT("\"\""); }
        AltB = Inner.Len() > 1 ? (TEXT("\"") + Inner.Left(1) + TEXT("\"")) : TEXT("\"?\"");
        if (AltB == Correct) { AltB = TEXT("\"!\""); }
    }
    else if (Correct.IsNumeric() || (Correct.StartsWith(TEXT("-")) && Correct.Mid(1).IsNumeric()) || Correct.Contains(TEXT(".")))
    {
        const double V = FCString::Atod(*Correct);
        const bool bFloat = Correct.Contains(TEXT("."));
        AltA = bFloat ? FString::SanitizeFloat(V * 2.0) : FString::FromInt(FMath::RoundToInt(V) + 1);
        AltB = bFloat ? FString::SanitizeFloat(V / 2.0) : FString::FromInt(FMath::RoundToInt(V) - 1);
        if (AltA == Correct) { AltA = FString::FromInt(FMath::RoundToInt(V) + 2); }
        if (AltB == Correct) { AltB = FString::FromInt(FMath::RoundToInt(V) - 2); }
    }
    else
    {
        AltA = TEXT("0");
        AltB = TEXT("error");
    }

    // Deterministic slot assignment per challenge so replays don't memorize A/B/C.
    PredictionCorrectIndex = static_cast<int32>(GetTypeHash(Learning.Id) % 3u);
    FString Labels[3];
    Labels[PredictionCorrectIndex] = Correct;
    Labels[(PredictionCorrectIndex + 1) % 3] = AltA;
    Labels[(PredictionCorrectIndex + 2) % 3] = AltB;

    PredictionQuestionText->SetText(FText::FromString(FString::Printf(
        TEXT("PREDICT IT: for input %s this returns ..."), *First.In)));
    if (PredictChoiceLabelA) { PredictChoiceLabelA->SetText(FText::FromString(Labels[0])); }
    if (PredictChoiceLabelB) { PredictChoiceLabelB->SetText(FText::FromString(Labels[1])); }
    if (PredictChoiceLabelC) { PredictChoiceLabelC->SetText(FText::FromString(Labels[2])); }
    for (UButton* B : { PredictChoiceButtonA, PredictChoiceButtonB, PredictChoiceButtonC })
    {
        if (B) { B->SetIsEnabled(true); }
    }
    PredictionDrillRow->SetVisibility(ESlateVisibility::Visible);
}

void UCodeTerminalWidget::AnswerPrediction(int32 ChoiceIndex)
{
    if (bPredictionAnswered || PredictionCorrectIndex < 0 || !PredictionDrillRow ||
        PredictionDrillRow->GetVisibility() != ESlateVisibility::Visible)
    {
        return;
    }
    bPredictionAnswered = true;
    const bool bCorrect = (ChoiceIndex == PredictionCorrectIndex);
    for (UButton* B : { PredictChoiceButtonA, PredictChoiceButtonB, PredictChoiceButtonC })
    {
        if (B) { B->SetIsEnabled(false); }
    }
    UTextBlock* CorrectLabel = (PredictionCorrectIndex == 0) ? PredictChoiceLabelA
                              : (PredictionCorrectIndex == 1) ? PredictChoiceLabelB
                              : PredictChoiceLabelC;
    const FString CorrectShown = CorrectLabel ? CorrectLabel->GetText().ToString() : FString();
    if (PredictionQuestionText)
    {
        PredictionQuestionText->SetText(FText::FromString(bCorrect
            ? TEXT("PREDICTION LOCKED ✓ — now prove it in code.")
            : FString::Printf(TEXT("PREDICTION ✗ — it returns %s. Re-read the worked example, then code it."), *CorrectShown)));
        PredictionQuestionText->SetColorAndOpacity(FSlateColor(bCorrect
            ? CodeRescueUI::Color::TerminalGreenBright()
            : CodeRescueUI::Color::Warning()));
    }
    SetDiagnosticsState(bCorrect ? TEXT("PREDICT | correct — mental model confirmed")
                                 : TEXT("PREDICT | miss — trace the worked example first"),
                        bCorrect ? CodeRescueUI::Color::TerminalGreenBright() : CodeRescueUI::Color::Warning());
    // Telemetry: predictions feed the same concept stream as attempts, tagged distinctly.
    UCodeRescueLearningLibrary::RecordAttempt(
        PredictionConcept,
        PredictionChallengeId + TEXT("#predict"),
        bCorrect,
        /*bUsedHint=*/false,
        /*bScaffoldOffered=*/false,
        0.0f,
        TEXT("prediction-drill"));
    // 2026-07-06: clicking a choice moves focus to the (now disabled) button;
    // hand it back to the widget so Escape/Ctrl hotkeys keep working.
    SetKeyboardFocus();
}

void UCodeTerminalWidget::OnPredictChoiceA() { AnswerPrediction(0); }
void UCodeTerminalWidget::OnPredictChoiceB() { AnswerPrediction(1); }
void UCodeTerminalWidget::OnPredictChoiceC() { AnswerPrediction(2); }
