#include "CodeRescueGameInstance.h"
#include "CodeRescueAchievementSystem.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueModLoader.h"
#include "CodeRescueSaveGame.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameMode.h"
#include "CodeRescueSkillTreeWidget.h"
#include "CaseFilePickupActor.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "CodingTerminalActor.h"
#include "SurvivorActor.h"
#include "CodeZombieActor.h"
#include "FriendlyNPCActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
constexpr int32 CodeRescueLanguageCount = 6;

void EnsureLanguageCounterSize(TArray<int32>& Counters)
{
    while (Counters.Num() < CodeRescueLanguageCount)
    {
        Counters.Add(0);
    }
}

FString LanguageToStableString(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java: return TEXT("Java");
    case ECodingLanguage::C: return TEXT("C");
    case ECodingLanguage::Python: return TEXT("Python");
    case ECodingLanguage::MATLAB: return TEXT("MATLAB");
    case ECodingLanguage::CPlus: return TEXT("C+");
    case ECodingLanguage::Cpp: return TEXT("C++");
    default: return TEXT("Unknown");
    }
}

void ResolveOperatorIdentityForLanguage(
    ECodingLanguage Language,
    FString& OutCallsign,
    FString& OutRoleTitle,
    FString& OutProfileNote)
{
    switch (Language)
    {
    case ECodingLanguage::Java:
        OutCallsign = TEXT("Rhea Calder");
        OutRoleTitle = TEXT("Rescue Operator");
        OutProfileNote = TEXT("frontline function-route lead");
        return;
    case ECodingLanguage::C:
        OutCallsign = TEXT("Ilan Cross");
        OutRoleTitle = TEXT("Systems Engineer");
        OutProfileNote = TEXT("memory-safe barricade and route discipline");
        return;
    case ECodingLanguage::Python:
        OutCallsign = TEXT("Noor Vance");
        OutRoleTitle = TEXT("Signal Analyst");
        OutProfileNote = TEXT("readable algorithm intel and scan support");
        return;
    case ECodingLanguage::MATLAB:
        OutCallsign = TEXT("Mika Stone");
        OutRoleTitle = TEXT("Triage Analyst");
        OutProfileNote = TEXT("arrays, models, and field medicine support");
        return;
    case ECodingLanguage::CPlus:
        OutCallsign = TEXT("Jules Ardent");
        OutRoleTitle = TEXT("Supply Strategist");
        OutProfileNote = TEXT("resource economy and quest-route planning");
        return;
    case ECodingLanguage::Cpp:
        OutCallsign = TEXT("Rhea Calder");
        OutRoleTitle = TEXT("Advanced Rescue Operator");
        OutProfileNote = TEXT("systems rescue, weapons, and survivor extraction");
        return;
    default:
        OutCallsign = TEXT("Rhea Calder");
        OutRoleTitle = TEXT("Rescue Operator");
        OutProfileNote = TEXT("frontline rescue route lead");
        return;
    }
}

FString EscapeNdjsonValue(FString Value)
{
    Value.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
    Value.ReplaceInline(TEXT("\""), TEXT("\\\""));
    Value.ReplaceInline(TEXT("\n"), TEXT("\\n"));
    Value.ReplaceInline(TEXT("\r"), TEXT(""));
    Value.ReplaceInline(TEXT("\t"), TEXT("\\t"));
    return Value;
}

FString EscapeJsonValue(FString Value)
{
    return EscapeNdjsonValue(MoveTemp(Value));
}

FString GetChallengeReplayVisibleGoal(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock")))
    {
        return TEXT("Visible goal: return true only when both rescue signals are true.");
    }
    if (ChallengeId.Contains(TEXT("reverse")))
    {
        return TEXT("Visible goal: reverse the visible rescue code exactly, preserving every character.");
    }
    if (ChallengeId.Contains(TEXT("palindrome")))
    {
        return TEXT("Visible goal: accept the mirror word and reject the impostor word.");
    }
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        return TEXT("Visible goal: output Fizz, Buzz, and FizzBuzz with the combined rule first.");
    }
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        return TEXT("Visible goal: keep only even values in their original order.");
    }
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("linked_list")) || ChallengeId.Contains(TEXT("traverse")))
    {
        return TEXT("Visible goal: follow current to next until the sentinel and return the visit count.");
    }
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("binarysearch")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        return TEXT("Visible goal: shrink low/high bounds until the target is found or ruled out.");
    }
    return TEXT("Visible goal: accumulate the visible route values and return the expected total.");
}

FString GetChallengeReplayHiddenNote(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock")))
    {
        return TEXT("Hidden-test replay: unsafe true/false combinations must stay locked.");
    }
    if (ChallengeId.Contains(TEXT("reverse")))
    {
        return TEXT("Hidden-test replay: mixed-case and city-specific packets must reverse exactly.");
    }
    if (ChallengeId.Contains(TEXT("palindrome")))
    {
        return TEXT("Hidden-test replay: one real mirror and one impostor verify both outcomes.");
    }
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        return TEXT("Hidden-test replay: longer sweeps confirm later 3, 5, and 15 multiples.");
    }
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        return TEXT("Hidden-test replay: odd-only and mixed lists verify order and empty results.");
    }
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("linked_list")) || ChallengeId.Contains(TEXT("traverse")))
    {
        return TEXT("Hidden-test replay: changed start nodes prove traversal is not hard-coded.");
    }
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("binarysearch")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        return TEXT("Hidden-test replay: first, middle, last, and not-found targets prove the bounds.");
    }
    return TEXT("Hidden-test replay: empty, single-value, and larger batches prove the loop shape.");
}

FString GetChallengeReplayPracticeAction(const FString& ChallengeId)
{
    if (ChallengeId.Contains(TEXT("lock")))
    {
        return TEXT("Practice action: write the truth table before touching the next terminal.");
    }
    if (ChallengeId.Contains(TEXT("reverse")))
    {
        return TEXT("Practice action: trace the first and last index before coding again.");
    }
    if (ChallengeId.Contains(TEXT("palindrome")))
    {
        return TEXT("Practice action: mark left/right pairs and stop only when they cross.");
    }
    if (ChallengeId.Contains(TEXT("fizzbuzz")))
    {
        return TEXT("Practice action: list 3, 5, and 15 outcomes before writing branches.");
    }
    if (ChallengeId.Contains(TEXT("filter")) || ChallengeId.Contains(TEXT("even")))
    {
        return TEXT("Practice action: predict which items survive a tiny mixed list.");
    }
    if (ChallengeId.Contains(TEXT("linkedlist")) || ChallengeId.Contains(TEXT("linked_list")) || ChallengeId.Contains(TEXT("traverse")))
    {
        return TEXT("Practice action: say current = next[current] out loud for each hop.");
    }
    if (ChallengeId.Contains(TEXT("binary_search")) || ChallengeId.Contains(TEXT("binarysearch")) || ChallengeId.Contains(TEXT("bsearch")))
    {
        return TEXT("Practice action: write low, mid, and high after one comparison.");
    }
    return TEXT("Practice action: name the accumulator, starting value, and loop end condition.");
}

FString BuildLanguageProfileReviewRecommendation(
    int32 LanguageAttempts,
    int32 LanguageSolves,
    int32 LanguageNoHintSolves,
    int32 SuccessRate,
    int32 PerfectSolves,
    int32 BestStreak,
    bool bCampaignComplete)
{
    if (bCampaignComplete)
    {
        return TEXT("Review recommendation: this language route is clear; use the start screen to begin another language or revisit saved debriefs before public-demo capture.");
    }
    if (LanguageAttempts <= 0)
    {
        return TEXT("Review recommendation: start with the active protected terminal, predict the visible test, then validate once before spending hints.");
    }
    if (LanguageSolves <= 0)
    {
        return TEXT("Review recommendation: reopen the challenge replay brief, trace the visible goal by hand, then retry the active terminal.");
    }
    if (SuccessRate < 50)
    {
        return TEXT("Review recommendation: replay the last debrief, use one practice trace, and choose Story or Easy from Pause if pressure is blocking learning.");
    }
    if (LanguageNoHintSolves < FMath::Max(1, LanguageSolves / 2))
    {
        return TEXT("Review recommendation: attempt the next terminal without hints to grow no-hint mastery and earn more Research Points.");
    }
    if (PerfectSolves < 3 || BestStreak < 3)
    {
        return TEXT("Review recommendation: aim for one first-try full-score solve by predicting hidden edge cases before validation.");
    }
    return TEXT("Review recommendation: advance to the next city stage and keep the streak alive; the journal will preserve each recap after Resume.");
}
}

UCodeRescueGameInstance::UCodeRescueGameInstance()
{
    SelectedLanguage = ECodingLanguage::Java;
    ResetRun();
}

void UCodeRescueGameInstance::Init()
{
    Super::Init();
    LoadPersistentRun();

    // #65: scan Saved/Mods/* for user-authored content. Pure data merge —
    // returns the parsed manifests for any future surfacing (modlist UI,
    // Workshop sync). The custom-challenge merge happens lazily inside
    // CodeRunnerLibrary::LoadCustomChallenges, so this just primes the cache
    // and logs how many mods were detected.
    const TArray<FCodeRescueModManifest> Mods = UCodeRescueModLoader::LoadAllMods();
    if (Mods.Num() > 0)
    {
        UE_LOG(LogTemp, Display, TEXT("[Mods] Loaded %d user mod(s) from Saved/Mods/"), Mods.Num());
    }
}

void UCodeRescueGameInstance::ResetRun()
{
    SurvivorsRescued = 0;
    ZombiesNeutralized = 0;
    CodingScore = 0;
    TerminalsSolved = 0;
    InitializeOperatorIdentityForLanguage(SelectedLanguage);
    CompletedMissionIds.Reset();
    MissionProgress.Reset();
    ConceptProgress.Reset();

    SolvedTerminalIds.Reset();
    RescuedSurvivorNames.Reset();
    NeutralizedZombieIds.Reset();
    CollectedCaseFileIds.Reset();
    LastCollectedCaseFileTitle.Reset();
    UsedFriendlyNPCServiceIds.Reset();
    bHasFriendlyNPCServiceState = false;
    SpawnedZombieVariants.Reset();
    CurrentObjectiveIndex = 0;
    LastPlayerLocation = FVector::ZeroVector;
    LastPlayerRotation = FRotator::ZeroRotator;
    LastPlayerHealth = 100.0f;
    LastPlayerAmmo = 150;
    LastActiveWeapon = EWeaponType::Pistol;
    LastWeaponMagazines.Reset();
    LastWeaponReserveAmmo.Reset();
    bHasWeaponQuickSlotState = false;
    LastPlayerMedkits = 8;
    LastPlayerArmorPlates = 4;
    LastPlayerFlares = 3;
    LastPlayerSmokes = 2;
    LastPlayerStims = 2;
    LastPlayerScrap = 0;
    LastPlayerRadioScannerCharges = 1;
    LastPlayerFlashlightBatteries = 1;
    LastPlayerBypassKits = 3;   // 2026-07-02: starting bypass allowance on a fresh run (accessibility + reachable terminal->horde loop)
    LastPlayerAmmoPouchCapacityBonus = 0;
    bHasPlayerTacticalGear = false;
    bHasPlayerResources = false;
    bHasWorldState = false;

    // #15 — Run scoreboard. Reset on Restart-Fresh; preserved across reload.
    KillCount = 0;
    RescueCount = 0;
    TerminalSolveCount = 0;
    RunSeconds = 0.0f;
    DeathCount = 0;
    HeadshotCount = 0;

    // #63 — companion gating flag.
    bHasCompanion = false;

    // Learning mastery state is run-scoped: a fresh restart should feel like a
    // clean classroom, while LoadPersistentRun restores the current profile.
    ResearchPoints = 3;
    SkillTreeUnlocked = 0;
    LanguageSolveCounts.Init(0, CodeRescueLanguageCount);
    LanguageAttemptCounts.Init(0, CodeRescueLanguageCount);
    LanguageNoHintSolveCounts.Init(0, CodeRescueLanguageCount);
    TotalValidationAttempts = 0;
    SuccessfulValidationAttempts = 0;
    FailedValidationAttempts = 0;
    CurrentLearningStreak = 0;
    BestLearningStreak = 0;
    NoHintSolveCount = 0;
    PerfectSolveCount = 0;
    LastLearningDebriefChallengeId.Reset();
    LastLearningDebriefConcept.Reset();
    LastLearningDebriefLanguage.Reset();
    LastLearningDebriefSummary.Reset();
    LastLearningDebriefScore = 0;
    bHasLearningDebriefState = false;
    LastSurvivorIntelTerminalId.Reset();
    LastSurvivorIntelSurvivorName.Reset();
    LastSurvivorIntelCityLabel.Reset();
    LastSurvivorIntelLanguage.Reset();
    LastSurvivorIntelStatus.Reset();
    LastSurvivorIntelSummary.Reset();
    LastSurvivorIntelScore = 0;
    bHasSurvivorIntelArchiveState = false;
    RewardChoiceEligibleTerminalIds.Reset();
    ClaimedTerminalRewardChoiceIds.Reset();
    LastTerminalRewardChoiceTerminalId.Reset();
    LastTerminalRewardChoiceId.Reset();
    LastTerminalRewardChoiceSummary.Reset();
    bHasTerminalRewardChoiceState = false;
}

void UCodeRescueGameInstance::IncrementKillCount(bool bWasHeadshot)
{
    ++KillCount;
    if (bWasHeadshot)
    {
        ++HeadshotCount;
    }
    SavePersistentRun();
    UCodeRescueAchievementSystem::EvaluateAll(GetWorld());   // #40
}

void UCodeRescueGameInstance::IncrementRescueCount()
{
    ++RescueCount;
    SavePersistentRun();
    UCodeRescueAchievementSystem::EvaluateAll(GetWorld());   // #40
}

void UCodeRescueGameInstance::IncrementTerminalSolveCount()
{
    ++TerminalSolveCount;
    SavePersistentRun();
    UCodeRescueAchievementSystem::EvaluateAll(GetWorld());   // #40
}

void UCodeRescueGameInstance::IncrementDeathCount()
{
    ++DeathCount;
    SaveDeathRecoveryCheckpoint(false);
}

void UCodeRescueGameInstance::AccumulateRunSeconds(float DeltaSeconds)
{
    if (DeltaSeconds > 0.0f)
    {
        RunSeconds += DeltaSeconds;
    }
}

FString UCodeRescueGameInstance::GetScoreboardSummary() const
{
    const int32 Hours = FMath::FloorToInt(RunSeconds / 3600.0f);
    const int32 Minutes = FMath::FloorToInt(FMath::Fmod(RunSeconds, 3600.0f) / 60.0f);
    const int32 Seconds = FMath::FloorToInt(FMath::Fmod(RunSeconds, 60.0f));
    return FString::Printf(
        TEXT("RUN SCOREBOARD\n  Cities cleared: %d\n  Survivors rescued: %d\n  Zombies neutralized: %d\n  Headshots: %d\n  Deaths: %d\n  Time played: %02d:%02d:%02d"),
        TerminalSolveCount, RescueCount, KillCount, HeadshotCount, DeathCount,
        Hours, Minutes, Seconds);
}

float UCodeRescueGameInstance::GetZombieHealthMultiplier() const
{
    switch (Difficulty)
    {
    case EGameDifficulty::Story:  return 0.45f;
    case EGameDifficulty::Easy:   return 0.65f;
    case EGameDifficulty::Survival: return 2.0f;
    case EGameDifficulty::Nightmare: return 2.55f;
    case EGameDifficulty::Hard:   return 1.6f;
    default:                      return 1.0f;
    }
}

float UCodeRescueGameInstance::GetZombieDamageMultiplier() const
{
    switch (Difficulty)
    {
    case EGameDifficulty::Story:  return 0.35f;
    case EGameDifficulty::Easy:   return 0.6f;
    case EGameDifficulty::Survival: return 2.15f;
    case EGameDifficulty::Nightmare: return 2.75f;
    case EGameDifficulty::Hard:   return 1.75f;
    default:                      return 1.0f;
    }
}

FString UCodeRescueGameInstance::GetDifficultyDisplayName() const
{
    switch (Difficulty)
    {
    case EGameDifficulty::Story:     return TEXT("Story");
    case EGameDifficulty::Easy:      return TEXT("Easy");
    case EGameDifficulty::Hard:      return TEXT("Hard");
    case EGameDifficulty::Survival:  return TEXT("Survival");
    case EGameDifficulty::Nightmare: return TEXT("Nightmare");
    default:                         return TEXT("Normal");
    }
}

FString UCodeRescueGameInstance::GetAccessibilitySummary() const
{
    const FString ColorMode =
        ColorblindMode == EColorblindMode::Deuteranope ? TEXT("Deuteranope") :
        ColorblindMode == EColorblindMode::Protanope ? TEXT("Protanope") :
        ColorblindMode == EColorblindMode::Tritanope ? TEXT("Tritanope") :
        TEXT("Standard");
    return FString::Printf(
        TEXT("Subtitles %s %.1fx | UI %.1fx | %s | Contrast %s | Motion %s | Mono %s | Sound cues %s | Aim %.1fx | Hints %s"),
        bSubtitlesEnabled ? TEXT("on") : TEXT("off"),
        FMath::Clamp(SubtitleScale, 0.75f, 1.75f),
        GetUITextScale(),
        *ColorMode,
        bHighContrastHUD ? TEXT("high") : TEXT("standard"),
        bReducedMotion ? TEXT("reduced") : TEXT("standard"),
        bMonoAudio ? TEXT("on") : TEXT("off"),
        bVisualizeSoundCues ? TEXT("visible") : TEXT("hidden"),
        FMath::Clamp(AimAssistScale, 0.0f, 2.0f),
        bSimplifiedInputHints ? TEXT("simplified") : TEXT("full"));
}

float UCodeRescueGameInstance::GetUITextScale() const
{
    return FMath::Clamp(UITextScale, 0.80f, 1.75f);
}

FString UCodeRescueGameInstance::GetUITextScaleSummary() const
{
    return FString::Printf(
        TEXT("UI text %.2fx | subtitles %.2fx"),
        GetUITextScale(),
        FMath::Clamp(SubtitleScale, 0.75f, 1.75f));
}

FString UCodeRescueGameInstance::GetMonoAudioSummary() const
{
    return bMonoAudio
        ? TEXT("Mono audio: on | positional cues centered | visual sound cues locked on")
        : TEXT("Mono audio: off | stereo spatial cues active");
}

FString UCodeRescueGameInstance::GetAudioMixSummary() const
{
    return FString::Printf(
        TEXT("Audio mix | Master %d%% | SFX %d%% | Music %d%% | Mono %s | Threat music %s %.0f%% | Ambient %s"),
        FMath::RoundToInt(FMath::Clamp(MasterVolume, 0.0f, 1.0f) * 100.0f),
        FMath::RoundToInt(FMath::Clamp(SfxVolume, 0.0f, 1.0f) * 100.0f),
        FMath::RoundToInt(FMath::Clamp(MusicVolume, 0.0f, 1.0f) * 100.0f),
        bMonoAudio ? TEXT("on") : TEXT("off"),
        *ReactiveThreatMusicState,
        FMath::Clamp(ReactiveThreatMusicIntensity, 0.0f, 1.0f) * 100.0f,
        *CityAmbientZoneLabel);
}

float UCodeRescueGameInstance::GetSfxVolumeScalar() const
{
    return FMath::Clamp(SfxVolume, 0.0f, 1.0f);
}

float UCodeRescueGameInstance::GetMusicVolumeScalar() const
{
    return FMath::Clamp(MusicVolume, 0.0f, 1.0f);
}

float UCodeRescueGameInstance::GetReactiveThreatMusicScalar() const
{
    if (!bReactiveThreatMusicEnabled)
    {
        return 1.0f;
    }

    return FMath::Lerp(0.82f, 1.22f, FMath::Clamp(ReactiveThreatMusicIntensity, 0.0f, 1.0f));
}

void UCodeRescueGameInstance::ApplyAudioMixSettings()
{
    MasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
    SfxVolume = FMath::Clamp(SfxVolume, 0.0f, 1.0f);
    MusicVolume = FMath::Clamp(MusicVolume, 0.0f, 1.0f);
    if (bMonoAudio)
    {
        bVisualizeSoundCues = true;
    }

    if (GEngine && GEngine->GetMainAudioDeviceRaw())
    {
        GEngine->GetMainAudioDeviceRaw()->SetTransientPrimaryVolume(MasterVolume);
    }
    if (MusicComponent)
    {
        RefreshReactiveThreatMusicVolume();
    }
}

void UCodeRescueGameInstance::RefreshReactiveThreatMusicVolume()
{
    if (MusicComponent)
    {
        MusicComponent->SetVolumeMultiplier(0.45f * GetMusicVolumeScalar() * GetReactiveThreatMusicScalar());
    }
}

void UCodeRescueGameInstance::UpdateReactiveThreatMusic(float ThreatIntensity, const FString& StateLabel)
{
    ReactiveThreatMusicIntensity = FMath::Clamp(ThreatIntensity, 0.0f, 1.0f);
    ReactiveThreatMusicState = StateLabel.IsEmpty() ? FString(TEXT("calm")) : StateLabel;
    RefreshReactiveThreatMusicVolume();
}

FString UCodeRescueGameInstance::GetReactiveThreatMusicSummary() const
{
    return FString::Printf(
        TEXT("Reactive threat music: %s %.0f%%, scalar %.2fx"),
        *ReactiveThreatMusicState,
        FMath::Clamp(ReactiveThreatMusicIntensity, 0.0f, 1.0f) * 100.0f,
        GetReactiveThreatMusicScalar());
}

void UCodeRescueGameInstance::UpdateCityAmbientZone(const FString& ZoneLabel, const FString& BedLabel, float ZoneIntensity)
{
    if (!bCityAmbientZoneDirectorEnabled)
    {
        CityAmbientZoneLabel = TEXT("ambient disabled");
        CityAmbientZoneBed = TEXT("ambient_disabled");
        CityAmbientZoneIntensity = 0.0f;
        return;
    }

    CityAmbientZoneLabel = ZoneLabel.IsEmpty() ? FString(TEXT("city street")) : ZoneLabel;
    CityAmbientZoneBed = BedLabel.IsEmpty() ? FString(TEXT("city_street_bed")) : BedLabel;
    CityAmbientZoneIntensity = FMath::Clamp(ZoneIntensity, 0.0f, 1.0f);
}

FString UCodeRescueGameInstance::GetCityAmbientZoneSummary() const
{
    return FString::Printf(
        TEXT("City ambient zone: %s | bed %s | intensity %.0f%%"),
        *CityAmbientZoneLabel,
        *CityAmbientZoneBed,
        FMath::Clamp(CityAmbientZoneIntensity, 0.0f, 1.0f) * 100.0f);
}

FString UCodeRescueGameInstance::GetVisualizedSoundCueSummary() const
{
    if (!bVisualizeSoundCues)
    {
        return TEXT("Sound cues hidden");
    }

    return FString::Printf(
        TEXT("Sound cues | threat %s %.0f%% | ambient %s %.0f%% | subtitles %s | mix SFX %d%% music %d%%"),
        *ReactiveThreatMusicState,
        FMath::Clamp(ReactiveThreatMusicIntensity, 0.0f, 1.0f) * 100.0f,
        *CityAmbientZoneLabel,
        FMath::Clamp(CityAmbientZoneIntensity, 0.0f, 1.0f) * 100.0f,
        bSubtitlesEnabled ? TEXT("on") : TEXT("off"),
        FMath::RoundToInt(FMath::Clamp(SfxVolume, 0.0f, 1.0f) * 100.0f),
        FMath::RoundToInt(FMath::Clamp(MusicVolume, 0.0f, 1.0f) * 100.0f));
}

FString UCodeRescueGameInstance::GetLanguageName() const
{
    switch (SelectedLanguage)
    {
    case ECodingLanguage::Java: return TEXT("Java");
    case ECodingLanguage::C: return TEXT("C");
    case ECodingLanguage::Python: return TEXT("Python");
    case ECodingLanguage::MATLAB: return TEXT("MATLAB");
    case ECodingLanguage::CPlus: return TEXT("C+");
    case ECodingLanguage::Cpp: return TEXT("C++");
    default: return TEXT("Unknown");
    }
}

void UCodeRescueGameInstance::InitializeOperatorIdentityForLanguage(ECodingLanguage Language)
{
    ResolveOperatorIdentityForLanguage(Language, OperatorCallsign, OperatorRoleTitle, OperatorProfileNote);
    bHasOperatorIdentityState = true;
}

FString UCodeRescueGameInstance::GetOperatorIdentitySummary() const
{
    const FString Callsign = OperatorCallsign.IsEmpty() ? TEXT("Rhea Calder") : OperatorCallsign;
    const FString Role = OperatorRoleTitle.IsEmpty() ? TEXT("Rescue Operator") : OperatorRoleTitle;
    const FString Note = OperatorProfileNote.IsEmpty() ? TEXT("frontline rescue route lead") : OperatorProfileNote;
    return FString::Printf(
        TEXT("%s | %s | %s | %s run"),
        *Callsign,
        *Role,
        *Note,
        *GetLanguageName());
}

FString UCodeRescueGameInstance::MakeLanguageSaveSlotName(ECodingLanguage Language)
{
    switch (Language)
    {
    case ECodingLanguage::Java:
        return TEXT("OperationCodeRescue_Language_Java");
    case ECodingLanguage::C:
        return TEXT("OperationCodeRescue_Language_C");
    case ECodingLanguage::Python:
        return TEXT("OperationCodeRescue_Language_Python");
    case ECodingLanguage::MATLAB:
        return TEXT("OperationCodeRescue_Language_MATLAB");
    case ECodingLanguage::CPlus:
        return TEXT("OperationCodeRescue_Language_CPlus");
    case ECodingLanguage::Cpp:
        return TEXT("OperationCodeRescue_Language_Cpp");
    default:
        return TEXT("OperationCodeRescue_Language_Java");
    }
}

bool UCodeRescueGameInstance::DoesLanguageSaveExist(ECodingLanguage Language) const
{
    return UGameplayStatics::DoesSaveGameExist(MakeLanguageSaveSlotName(Language), 0);
}

FString UCodeRescueGameInstance::GetLanguageSaveSummary(ECodingLanguage Language) const
{
    const FString SlotName = MakeLanguageSaveSlotName(Language);
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return TEXT("No save yet");
    }

    const UCodeRescueSaveGame* Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!Save)
    {
        return TEXT("Save found, but could not be read");
    }

    const int32 LanguageIndex = FMath::Clamp(static_cast<int32>(Language), 0, CodeRescueLanguageCount - 1);
    const int32 LanguageSolves = Save->LanguageSolveCounts.IsValidIndex(LanguageIndex)
        ? Save->LanguageSolveCounts[LanguageIndex]
        : Save->TerminalsSolved;

    return FString::Printf(
        TEXT("%d terminals | %d survivors | score %d | %d track solves | %s %s"),
        Save->TerminalsSolved,
        Save->SurvivorsRescued,
        Save->CodingScore,
        LanguageSolves,
        Save->bHasOperatorIdentityState && !Save->OperatorCallsign.IsEmpty()
            ? *Save->OperatorCallsign
            : TEXT("Rhea Calder"),
        Save->bHasOperatorIdentityState && !Save->OperatorRoleTitle.IsEmpty()
            ? *Save->OperatorRoleTitle
            : TEXT("Rescue Operator"));
}

FString UCodeRescueGameInstance::GetFirstSessionRoutePreviewSummary(ECodingLanguage Language) const
{
    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    const FString LanguageName = LanguageToStableString(Language);
    const FString SlotName = MakeLanguageSaveSlotName(Language);
    const bool bSaveExists = DoesLanguageSaveExist(Language);
    const FString SaveState = bSaveExists
        ? FString::Printf(TEXT("Resume available: %s"), *GetLanguageSaveSummary(Language))
        : TEXT("No save yet: NEW creates a clean language-only profile.");

    if (Missions.Num() <= 0)
    {
        return FString::Printf(
            TEXT("FIRST-SESSION ROUTE PREVIEW\nTrack: %s only | Slot: %s | %s\nFirst route: protected terminal -> survivor marker -> extraction\nTuning: beginner/normal/challenge tuning band -> beginner onboarding\nStart-screen choice remains first: NEW starts this track; RESUME reloads only this language save."),
            *LanguageName,
            *SlotName,
            *SaveState);
    }

    const FCodeRescueCityMission& FirstMission = Missions[0];
    const FString DifficultyBand = FirstMission.DifficultyTier <= 1
        ? TEXT("beginner onboarding")
        : (FirstMission.DifficultyTier <= 3 ? TEXT("normal rescue pressure") : TEXT("challenge pressure"));

    return FString::Printf(
        TEXT("FIRST-SESSION ROUTE PREVIEW\nTrack: %s only | Slot: %s | %s\nFirst route: %03d %s, %s | protected terminal -> survivor marker -> extraction\nTerminal: %s | Stage: %s | Focus: %s\nLandmark: %s | Contact: %s | Tuning: beginner/normal/challenge tuning band -> %s\nStart-screen choice remains first: NEW creates a clean %s profile; RESUME reloads only this language save."),
        *LanguageName,
        *SlotName,
        *SaveState,
        FirstMission.Rank,
        *FirstMission.CityName,
        *FirstMission.StateName,
        *FirstMission.TerminalTitle,
        *FirstMission.CurriculumStageName,
        *FirstMission.CurriculumFocus,
        *FirstMission.LandmarkName,
        *FirstMission.SurvivorName,
        *DifficultyBand,
        *LanguageName);
}

FString UCodeRescueGameInstance::GetLaunchLanguageSaveRosterSummary() const
{
    auto MakeRosterEntry = [this](ECodingLanguage Language) -> FString
    {
        const FString State = DoesLanguageSaveExist(Language)
            ? TEXT("RESUME AVAILABLE")
            : TEXT("NEW RUN READY");
        return FString::Printf(
            TEXT("%s: %s"),
            *LanguageToStableString(Language),
            *State);
    };

    return FString::Printf(
        TEXT("LANGUAGE SAVE ROSTER\n%s | %s | %s\n%s | %s | %s"),
        *MakeRosterEntry(ECodingLanguage::Java),
        *MakeRosterEntry(ECodingLanguage::C),
        *MakeRosterEntry(ECodingLanguage::CPlus),
        *MakeRosterEntry(ECodingLanguage::Cpp),
        *MakeRosterEntry(ECodingLanguage::Python),
        *MakeRosterEntry(ECodingLanguage::MATLAB));
}

bool UCodeRescueGameInstance::StartFreshLanguageRun(ECodingLanguage Language)
{
    SaveSlotName = MakeLanguageSaveSlotName(Language);
    ResetRun();
    SelectedLanguage = Language;
    InitializeOperatorIdentityForLanguage(Language);
    bHasSelectedLaunchLanguageThisSession = true;
    UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
    return true;
}

bool UCodeRescueGameInstance::ResumeLanguageRun(ECodingLanguage Language)
{
    SaveSlotName = MakeLanguageSaveSlotName(Language);
    const bool bLoaded = LoadPersistentRun();
    SelectedLanguage = Language;
    bHasSelectedLaunchLanguageThisSession = true;
    if (!bLoaded)
    {
        ResetRun();
        SelectedLanguage = Language;
        InitializeOperatorIdentityForLanguage(Language);
    }
    return bLoaded;
}

bool UCodeRescueGameInstance::SavePersistentRun()
{
    return SavePersistentRunInternal(true);
}

bool UCodeRescueGameInstance::SaveDeathRecoveryCheckpoint(bool bCountDeath)
{
    if (bCountDeath)
    {
        ++DeathCount;
    }

    CaptureWorldStateFromLevel(GetWorld());

    const int32 RecoveryObjectiveIndex = FMath::Max(0, CurrentObjectiveIndex);
    LastPlayerLocation = FCodeRescueCampaign::GetPlayerStartLocation(RecoveryObjectiveIndex);
    LastPlayerRotation = FRotator(0.0f, 35.0f, 0.0f);
    bHasWorldState = true;

    if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        if (ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(Pawn))
        {
            const float RecoveryHealth = FMath::Max(35.0f, Character->MaxHealth * 0.45f);
            LastPlayerHealth = FMath::Clamp(RecoveryHealth, 1.0f, Character->MaxHealth);
            LastPlayerAmmo = FMath::Clamp(FMath::Max(LastPlayerAmmo, Character->MagazineSize), 0, FMath::Max(0, Character->MaxAmmo));
            LastPlayerMedkits = FMath::Clamp(FMath::Max(LastPlayerMedkits, 1), 0, FMath::Max(0, Character->MaxMedkits));
            LastPlayerArmorPlates = FMath::Clamp(FMath::Max(LastPlayerArmorPlates, 1), 0, FMath::Max(0, Character->MaxArmorPlates));
            LastPlayerStims = FMath::Max(LastPlayerStims, 1);
            bHasPlayerTacticalGear = true;
            bHasPlayerResources = true;
        }
    }

    if (LastPlayerHealth <= 0.0f)
    {
        LastPlayerHealth = 35.0f;
        LastPlayerMedkits = FMath::Max(LastPlayerMedkits, 1);
        bHasPlayerResources = true;
    }

    const bool bSaved = SavePersistentRunInternal(false);
    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueDeathRecovery] Saved playable %s recovery checkpoint slot=%s city=%d health=%.0f deaths=%d"),
        *GetLanguageName(),
        *SaveSlotName,
        RecoveryObjectiveIndex,
        LastPlayerHealth,
        DeathCount);
    return bSaved;
}

bool UCodeRescueGameInstance::SavePersistentRunInternal(bool bCaptureWorldState)
{
    // Pull live player transform + objective index out of the world before
    // writing, so callers do not have to remember to do it themselves.
    if (bCaptureWorldState)
    {
        CaptureWorldStateFromLevel(GetWorld());
    }
    TerminalsSolved = SolvedTerminalIds.Num();
    SurvivorsRescued = RescuedSurvivorNames.Num();
    ZombiesNeutralized = NeutralizedZombieIds.Num();

    UCodeRescueSaveGame* Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::CreateSaveGameObject(UCodeRescueSaveGame::StaticClass()));
    if (!Save) return false;

    Save->LastSelectedLanguage = SelectedLanguage;
    Save->Difficulty = Difficulty;
    Save->OperatorCallsign = OperatorCallsign;
    Save->OperatorRoleTitle = OperatorRoleTitle;
    Save->OperatorProfileNote = OperatorProfileNote;
    Save->bHasOperatorIdentityState = bHasOperatorIdentityState;
    Save->SurvivorsRescued = SurvivorsRescued;
    Save->ZombiesNeutralized = ZombiesNeutralized;
    Save->CodingScore = CodingScore;
    Save->TerminalsSolved = TerminalsSolved;
    Save->CompletedMissionIds = CompletedMissionIds;
    Save->MissionProgress = MissionProgress;
    Save->ConceptProgress = ConceptProgress;

    Save->SolvedTerminalIds = SolvedTerminalIds;
    Save->RescuedSurvivorNames = RescuedSurvivorNames;
    Save->NeutralizedZombieIds = NeutralizedZombieIds;
    Save->CollectedCaseFileIds = CollectedCaseFileIds;
    Save->LastCollectedCaseFileTitle = LastCollectedCaseFileTitle;
    Save->UsedFriendlyNPCServiceIds = UsedFriendlyNPCServiceIds;
    Save->bHasFriendlyNPCServiceState = bHasFriendlyNPCServiceState;
    Save->SpawnedZombieVariants = SpawnedZombieVariants;
    Save->CurrentObjectiveIndex = CurrentObjectiveIndex;
    Save->PlayerLocation = LastPlayerLocation;
    Save->PlayerRotation = LastPlayerRotation;
    Save->PlayerHealth = LastPlayerHealth;
    Save->PlayerAmmo = LastPlayerAmmo;
    Save->ActiveWeapon = LastActiveWeapon;
    Save->WeaponMagazines = LastWeaponMagazines;
    Save->WeaponReserveAmmo = LastWeaponReserveAmmo;
    Save->bHasWeaponQuickSlotState = bHasWeaponQuickSlotState;
    Save->PlayerMedkits = LastPlayerMedkits;
    Save->PlayerArmorPlates = LastPlayerArmorPlates;
    Save->PlayerFlares = LastPlayerFlares;
    Save->PlayerSmokes = LastPlayerSmokes;
    Save->PlayerStims = LastPlayerStims;
    Save->PlayerScrap = LastPlayerScrap;
    Save->PlayerRadioScannerCharges = LastPlayerRadioScannerCharges;
    Save->PlayerFlashlightBatteries = LastPlayerFlashlightBatteries;
    Save->PlayerBypassKits = LastPlayerBypassKits;
    Save->PlayerAmmoPouchCapacityBonus = LastPlayerAmmoPouchCapacityBonus;
    Save->bHasPlayerTacticalGear = bHasPlayerTacticalGear;
    Save->bHasPlayerResources = bHasPlayerResources;
    Save->bHasWorldState = bHasWorldState;

    // #15 — scoreboard
    Save->KillCount = KillCount;
    Save->RescueCount = RescueCount;
    Save->TerminalSolveCount = TerminalSolveCount;
    Save->RunSeconds = RunSeconds;
    Save->DeathCount = DeathCount;
    Save->HeadshotCount = HeadshotCount;
    Save->bHasRunScoreboard = true;
    Save->bHasShownTutorial = bHasShownTutorial;   // #17
    Save->bHasCompanion = bHasCompanion;           // support squad/companion gate
    Save->ResearchPoints = ResearchPoints;          // #31
    Save->LanguageSolveCounts = LanguageSolveCounts;   // #50
    Save->bSubtitlesEnabled = bSubtitlesEnabled;       // #44
    Save->ColorblindMode = ColorblindMode;             // #45
    Save->SubtitleScale = FMath::Clamp(SubtitleScale, 0.75f, 1.75f);
    Save->UITextScale = GetUITextScale();
    Save->bHighContrastHUD = bHighContrastHUD;
    Save->bReducedMotion = bReducedMotion;
    Save->bSimplifiedInputHints = bSimplifiedInputHints;
    Save->bVisualizeSoundCues = bVisualizeSoundCues;
    Save->bMonoAudio = bMonoAudio;
    Save->AimAssistScale = FMath::Clamp(AimAssistScale, 0.0f, 2.0f);
    Save->MasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
    Save->SfxVolume = FMath::Clamp(SfxVolume, 0.0f, 1.0f);
    Save->MusicVolume = FMath::Clamp(MusicVolume, 0.0f, 1.0f);
    Save->ControlProfileName = ControlProfileName.IsEmpty() ? TEXT("Default") : ControlProfileName;
    Save->ControlProfileExportCount = ControlProfileExportCount;
    Save->SkillTreeUnlocked = SkillTreeUnlocked;       // #55
    Save->TotalValidationAttempts = TotalValidationAttempts;
    Save->SuccessfulValidationAttempts = SuccessfulValidationAttempts;
    Save->FailedValidationAttempts = FailedValidationAttempts;
    Save->CurrentLearningStreak = CurrentLearningStreak;
    Save->BestLearningStreak = BestLearningStreak;
    Save->NoHintSolveCount = NoHintSolveCount;
    Save->PerfectSolveCount = PerfectSolveCount;
    Save->LanguageAttemptCounts = LanguageAttemptCounts;
    Save->LanguageNoHintSolveCounts = LanguageNoHintSolveCounts;
    Save->LastLearningDebriefChallengeId = LastLearningDebriefChallengeId;
    Save->LastLearningDebriefConcept = LastLearningDebriefConcept;
    Save->LastLearningDebriefLanguage = LastLearningDebriefLanguage;
    Save->LastLearningDebriefSummary = LastLearningDebriefSummary;
    Save->LastLearningDebriefScore = LastLearningDebriefScore;
    Save->bHasLearningDebriefState = bHasLearningDebriefState;
    Save->LastSurvivorIntelTerminalId = LastSurvivorIntelTerminalId;
    Save->LastSurvivorIntelSurvivorName = LastSurvivorIntelSurvivorName;
    Save->LastSurvivorIntelCityLabel = LastSurvivorIntelCityLabel;
    Save->LastSurvivorIntelLanguage = LastSurvivorIntelLanguage;
    Save->LastSurvivorIntelStatus = LastSurvivorIntelStatus;
    Save->LastSurvivorIntelSummary = LastSurvivorIntelSummary;
    Save->LastSurvivorIntelScore = LastSurvivorIntelScore;
    Save->bHasSurvivorIntelArchiveState = bHasSurvivorIntelArchiveState;
    Save->RewardChoiceEligibleTerminalIds = RewardChoiceEligibleTerminalIds;
    Save->ClaimedTerminalRewardChoiceIds = ClaimedTerminalRewardChoiceIds;
    Save->LastTerminalRewardChoiceTerminalId = LastTerminalRewardChoiceTerminalId;
    Save->LastTerminalRewardChoiceId = LastTerminalRewardChoiceId;
    Save->LastTerminalRewardChoiceSummary = LastTerminalRewardChoiceSummary;
    Save->bHasTerminalRewardChoiceState = bHasTerminalRewardChoiceState;

    const bool bOk = UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
    if (bOk)
    {
        // #19 — stamp wall-clock time so the HUD can show the autosave pip.
        if (UWorld* W = GetWorld())
        {
            LastSaveWallSeconds = W->GetTimeSeconds();
        }
    }
    return bOk;
}

bool UCodeRescueGameInstance::LoadPersistentRun()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        return false;
    }

    UCodeRescueSaveGame* Save = Cast<UCodeRescueSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!Save) return false;

    SelectedLanguage = Save->LastSelectedLanguage;
    Difficulty = Save->Difficulty;
    if (Save->bHasOperatorIdentityState)
    {
        OperatorCallsign = Save->OperatorCallsign;
        OperatorRoleTitle = Save->OperatorRoleTitle;
        OperatorProfileNote = Save->OperatorProfileNote;
        bHasOperatorIdentityState = true;
    }
    else
    {
        InitializeOperatorIdentityForLanguage(SelectedLanguage);
    }
    SurvivorsRescued = Save->SurvivorsRescued;
    ZombiesNeutralized = Save->ZombiesNeutralized;
    CodingScore = Save->CodingScore;
    TerminalsSolved = Save->TerminalsSolved;
    CompletedMissionIds = Save->CompletedMissionIds;
    MissionProgress = Save->MissionProgress;
    ConceptProgress = Save->ConceptProgress;
    CompletedMissionIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    MissionProgress.RemoveAll([](const FCodeRescueMissionProgress& Progress) { return Progress.MissionId.IsEmpty(); });
    ConceptProgress.RemoveAll([](const FCodeRescueConceptProgress& Progress) { return Progress.ConceptId.IsEmpty(); });

    SolvedTerminalIds = Save->SolvedTerminalIds;
    RescuedSurvivorNames = Save->RescuedSurvivorNames;
    NeutralizedZombieIds = Save->NeutralizedZombieIds;
    CollectedCaseFileIds = Save->CollectedCaseFileIds;
    LastCollectedCaseFileTitle = Save->LastCollectedCaseFileTitle;
    UsedFriendlyNPCServiceIds = Save->UsedFriendlyNPCServiceIds;
    bHasFriendlyNPCServiceState = Save->bHasFriendlyNPCServiceState;
    SolvedTerminalIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    RescuedSurvivorNames.RemoveAll([](const FString& Name) { return Name.IsEmpty(); });
    NeutralizedZombieIds.RemoveAll([](int32 ZombieId) { return ZombieId < 0; });
    CollectedCaseFileIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    UsedFriendlyNPCServiceIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    TerminalsSolved = SolvedTerminalIds.Num();
    SurvivorsRescued = RescuedSurvivorNames.Num();
    ZombiesNeutralized = NeutralizedZombieIds.Num();
    SpawnedZombieVariants = Save->SpawnedZombieVariants;
    CurrentObjectiveIndex = Save->CurrentObjectiveIndex;
    LastPlayerLocation = Save->PlayerLocation;
    LastPlayerRotation = Save->PlayerRotation;
    LastPlayerHealth = Save->PlayerHealth;
    LastPlayerAmmo = Save->PlayerAmmo;
    LastActiveWeapon = Save->bHasWeaponQuickSlotState ? Save->ActiveWeapon : EWeaponType::Pistol;
    LastWeaponMagazines = Save->bHasWeaponQuickSlotState ? Save->WeaponMagazines : TArray<int32>();
    LastWeaponReserveAmmo = Save->bHasWeaponQuickSlotState ? Save->WeaponReserveAmmo : TArray<int32>();
    bHasWeaponQuickSlotState = Save->bHasWeaponQuickSlotState;
    LastPlayerMedkits = Save->PlayerMedkits;
    LastPlayerArmorPlates = Save->PlayerArmorPlates;
    LastPlayerFlares = Save->PlayerFlares;
    LastPlayerSmokes = Save->PlayerSmokes;
    LastPlayerStims = Save->PlayerStims;
    LastPlayerScrap = Save->PlayerScrap;
    LastPlayerRadioScannerCharges = Save->PlayerRadioScannerCharges;
    LastPlayerFlashlightBatteries = Save->PlayerFlashlightBatteries;
    LastPlayerBypassKits = Save->PlayerBypassKits;
    LastPlayerAmmoPouchCapacityBonus = Save->PlayerAmmoPouchCapacityBonus;
    bHasPlayerTacticalGear = Save->bHasPlayerTacticalGear;
    bHasPlayerResources = Save->bHasPlayerResources;
    bHasWorldState = Save->bHasWorldState;

    // #15 — scoreboard. Defaults to 0 if loading a pre-scoreboard save.
    if (Save->bHasRunScoreboard)
    {
        KillCount = Save->KillCount;
        RescueCount = Save->RescueCount;
        TerminalSolveCount = Save->TerminalSolveCount;
        RunSeconds = Save->RunSeconds;
        DeathCount = Save->DeathCount;
        HeadshotCount = Save->HeadshotCount;
    }
    bHasShownTutorial = Save->bHasShownTutorial;   // #17
    bHasCompanion = Save->bHasCompanion;
    ResearchPoints = Save->ResearchPoints;          // #31
    LanguageSolveCounts = Save->LanguageSolveCounts;   // #50
    bSubtitlesEnabled = Save->bSubtitlesEnabled;       // #44
    ColorblindMode = Save->ColorblindMode;             // #45
    SubtitleScale = FMath::Clamp(Save->SubtitleScale, 0.75f, 1.75f);
    UITextScale = FMath::Clamp(Save->UITextScale, 0.80f, 1.75f);
    bHighContrastHUD = Save->bHighContrastHUD;
    bReducedMotion = Save->bReducedMotion;
    bSimplifiedInputHints = Save->bSimplifiedInputHints;
    bVisualizeSoundCues = Save->bVisualizeSoundCues;
    bMonoAudio = Save->bMonoAudio;
    if (bMonoAudio)
    {
        bVisualizeSoundCues = true;
    }
    AimAssistScale = FMath::Clamp(Save->AimAssistScale, 0.0f, 2.0f);
    MasterVolume = FMath::Clamp(Save->MasterVolume, 0.0f, 1.0f);
    SfxVolume = FMath::Clamp(Save->SfxVolume, 0.0f, 1.0f);
    MusicVolume = FMath::Clamp(Save->MusicVolume, 0.0f, 1.0f);
    ApplyAudioMixSettings();
    ControlProfileName = Save->ControlProfileName.IsEmpty() ? TEXT("Default") : Save->ControlProfileName;
    ControlProfileExportCount = FMath::Max(0, Save->ControlProfileExportCount);
    SkillTreeUnlocked = Save->SkillTreeUnlocked;       // #55
    TotalValidationAttempts = Save->TotalValidationAttempts;
    SuccessfulValidationAttempts = Save->SuccessfulValidationAttempts;
    FailedValidationAttempts = Save->FailedValidationAttempts;
    CurrentLearningStreak = Save->CurrentLearningStreak;
    BestLearningStreak = Save->BestLearningStreak;
    NoHintSolveCount = Save->NoHintSolveCount;
    PerfectSolveCount = Save->PerfectSolveCount;
    LanguageAttemptCounts = Save->LanguageAttemptCounts;
    LanguageNoHintSolveCounts = Save->LanguageNoHintSolveCounts;
    LastLearningDebriefChallengeId = Save->LastLearningDebriefChallengeId;
    LastLearningDebriefConcept = Save->LastLearningDebriefConcept;
    LastLearningDebriefLanguage = Save->LastLearningDebriefLanguage;
    LastLearningDebriefSummary = Save->LastLearningDebriefSummary;
    LastLearningDebriefScore = FMath::Max(0, Save->LastLearningDebriefScore);
    bHasLearningDebriefState = Save->bHasLearningDebriefState && !LastLearningDebriefSummary.IsEmpty();
    LastSurvivorIntelTerminalId = Save->LastSurvivorIntelTerminalId;
    LastSurvivorIntelSurvivorName = Save->LastSurvivorIntelSurvivorName;
    LastSurvivorIntelCityLabel = Save->LastSurvivorIntelCityLabel;
    LastSurvivorIntelLanguage = Save->LastSurvivorIntelLanguage;
    LastSurvivorIntelStatus = Save->LastSurvivorIntelStatus;
    LastSurvivorIntelSummary = Save->LastSurvivorIntelSummary;
    LastSurvivorIntelScore = FMath::Max(0, Save->LastSurvivorIntelScore);
    bHasSurvivorIntelArchiveState = Save->bHasSurvivorIntelArchiveState
        && !LastSurvivorIntelTerminalId.IsEmpty()
        && !LastSurvivorIntelSummary.IsEmpty();
    RewardChoiceEligibleTerminalIds = Save->RewardChoiceEligibleTerminalIds;
    ClaimedTerminalRewardChoiceIds = Save->ClaimedTerminalRewardChoiceIds;
    RewardChoiceEligibleTerminalIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    ClaimedTerminalRewardChoiceIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    LastTerminalRewardChoiceTerminalId = Save->LastTerminalRewardChoiceTerminalId;
    LastTerminalRewardChoiceId = Save->LastTerminalRewardChoiceId;
    LastTerminalRewardChoiceSummary = Save->LastTerminalRewardChoiceSummary;
    bHasTerminalRewardChoiceState = Save->bHasTerminalRewardChoiceState
        && !LastTerminalRewardChoiceTerminalId.IsEmpty()
        && !LastTerminalRewardChoiceSummary.IsEmpty();
    EnsureLanguageCounterSize(LanguageSolveCounts);
    EnsureLanguageCounterSize(LanguageAttemptCounts);
    EnsureLanguageCounterSize(LanguageNoHintSolveCounts);
    return true;
}

FString UCodeRescueGameInstance::GetControlProfileSummary() const
{
    const FString Profile = ControlProfileName.IsEmpty() ? TEXT("Default") : ControlProfileName;
    return FString::Printf(
        TEXT("Control Profile: %s | Config axes + audited direct bindings | Exports: %d"),
        *Profile,
        ControlProfileExportCount);
}

bool UCodeRescueGameInstance::ExportControlProfileReviewFile()
{
    const FString Dir = FPaths::ProjectSavedDir() / TEXT("Config/ControlProfiles");
    IFileManager::Get().MakeDirectory(*Dir, true);

    const FString Profile = ControlProfileName.IsEmpty() ? TEXT("Default") : ControlProfileName;
    const FString Timestamp = FDateTime::UtcNow().ToIso8601();
    const FString FileName = Dir / TEXT("runtime_controls_profile.json");

    const FString Json = FString::Printf(
        TEXT("{\n")
        TEXT("  \"generated_utc\": \"%s\",\n")
        TEXT("  \"profile_name\": \"%s\",\n")
        TEXT("  \"profile_status\": \"runtime-exported review profile\",\n")
        TEXT("  \"manifest\": \"Content/CodeRescueData/control_remap_manifest.tsv\",\n")
        TEXT("  \"implementation_note\": \"Gameplay remains bound through stable config axes and direct C++ bindings; this export gives reviewers the active control contract without mutating live combat input.\",\n")
        TEXT("  \"config_backed_controls\": [\"MoveForward\", \"MoveRight\", \"Turn\", \"LookUp\", \"Fire\", \"Interact\", \"UseMedkit\", \"ShowGuidance\", \"JumpObjective\"],\n")
        TEXT("  \"direct_cpp_controls\": [\"Reload\", \"RecoverToCityArena\", \"SquadRegroup\", \"SquadFormation\", \"SquadMedic\", \"SquadHoldFollow\", \"Journal\", \"Pause\", \"CameraCycle\", \"WeaponQuickSlots\", \"WeaponCycle\", \"ThrowActive\", \"PlaceBarricade\"],\n")
        TEXT("  \"next_safe_remap_step\": \"Refactor direct bindings through a shared action-to-key table before allowing arbitrary player-selected key capture.\"\n")
        TEXT("}\n"),
        *EscapeJsonValue(Timestamp),
        *EscapeJsonValue(Profile));

    const bool bWrote = FFileHelper::SaveStringToFile(Json, *FileName);
    if (bWrote)
    {
        ++ControlProfileExportCount;
        SavePersistentRun();
    }
    return bWrote;
}

int32 UCodeRescueGameInstance::GetAdaptiveDifficultyTier() const
{
    // #32 — read cumulative successful solves across all concepts. Threshold
    // ladder is intentionally generous: brand-new students stay on tier 1
    // until they prove out the basics.
    int32 TotalSolves = TerminalSolveCount;
    if (TotalSolves <= 1)   return 0;
    if (TotalSolves <= 8)   return 1;
    if (TotalSolves <= 25)  return 2;
    return 3;
}

int32 UCodeRescueGameInstance::GetLanguageProficiency(ECodingLanguage Language) const
{
    const int32 Idx = static_cast<int32>(Language);
    if (LanguageSolveCounts.IsValidIndex(Idx))
    {
        return LanguageSolveCounts[Idx];
    }
    return 0;
}

void UCodeRescueGameInstance::RecordLanguageSolve(ECodingLanguage Language)
{
    const int32 Idx = static_cast<int32>(Language);
    EnsureLanguageCounterSize(LanguageSolveCounts);
    LanguageSolveCounts[Idx] += 1;

    // #40 — Polyglot achievement: solve in every supported language track.
    bool bAllLanguages = true;
    for (int32 i = 0; i < CodeRescueLanguageCount; ++i)
    {
        if (!LanguageSolveCounts.IsValidIndex(i) || LanguageSolveCounts[i] == 0)
        {
            bAllLanguages = false;
            break;
        }
    }
    if (bAllLanguages)
    {
        UCodeRescueAchievementSystem::Award(GetWorld(), 3);   // Polyglot
    }
    SavePersistentRun();
}

void UCodeRescueGameInstance::LogCodeAttempt(const FString& ChallengeId, const FString& UserCode, bool bSuccess, int32 Score)
{
    LogCodeAttemptDetailed(ChallengeId, SelectedLanguage, UserCode, bSuccess, Score, 0, 0, FString());
}

void UCodeRescueGameInstance::LogCodeAttemptDetailed(
    const FString& ChallengeId,
    ECodingLanguage Language,
    const FString& UserCode,
    bool bSuccess,
    int32 Score,
    int32 AttemptNumber,
    int32 HintsUsed,
    const FString& FirstFailedCheck)
{
    // #34 — append to Saved/CodeAttempts/<ChallengeId>.ndjson. One JSON object
    // per line keeps the history append-only and easy to review.
    if (ChallengeId.IsEmpty()) return;
    const FString Dir = FPaths::ProjectSavedDir() / TEXT("CodeAttempts");
    IFileManager::Get().MakeDirectory(*Dir, true);
    FString SafeChallengeId = ChallengeId;
    SafeChallengeId.ReplaceInline(TEXT("/"), TEXT("_"));
    SafeChallengeId.ReplaceInline(TEXT("\\"), TEXT("_"));
    SafeChallengeId.ReplaceInline(TEXT(":"), TEXT("_"));
    const FString FileName = Dir / (SafeChallengeId + TEXT(".ndjson"));
    const FString TimeStamp = FDateTime::UtcNow().ToString();

    // Trim code length to keep the log readable (max ~10 KB per attempt).
    FString Trimmed = UserCode.Len() > 10000 ? UserCode.Left(10000) + TEXT("[truncated]") : UserCode;
    Trimmed = EscapeNdjsonValue(Trimmed);
    const FString EscapedFailedCheck = EscapeNdjsonValue(FirstFailedCheck);
    const FString Line = FString::Printf(
        TEXT("{\"ts\":\"%s\",\"challenge\":\"%s\",\"language\":\"%s\",\"attempt\":%d,\"hints\":%d,\"success\":%s,\"score\":%d,\"first_failed_check\":\"%s\",\"code\":\"%s\"}\n"),
        *TimeStamp,
        *EscapeNdjsonValue(ChallengeId),
        *LanguageToStableString(Language),
        AttemptNumber,
        HintsUsed,
        bSuccess ? TEXT("true") : TEXT("false"),
        Score,
        *EscapedFailedCheck,
        *Trimmed);
    FFileHelper::SaveStringToFile(Line, *FileName,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(),
        FILEWRITE_Append);
}

int32 UCodeRescueGameInstance::RecordValidationAttempt(
    ECodingLanguage Language,
    bool bSuccess,
    int32 Score,
    bool bUsedHint,
    bool bFirstTry)
{
    EnsureLanguageCounterSize(LanguageAttemptCounts);
    EnsureLanguageCounterSize(LanguageNoHintSolveCounts);

    const int32 Idx = FMath::Clamp(static_cast<int32>(Language), 0, CodeRescueLanguageCount - 1);
    ++TotalValidationAttempts;
    ++LanguageAttemptCounts[Idx];

    int32 BonusScore = 0;
    if (bSuccess)
    {
        ++SuccessfulValidationAttempts;
        ++CurrentLearningStreak;
        BestLearningStreak = FMath::Max(BestLearningStreak, CurrentLearningStreak);

        if (!bUsedHint)
        {
            ++NoHintSolveCount;
            ++LanguageNoHintSolveCounts[Idx];
            BonusScore += 5;
        }
        if (bFirstTry)
        {
            BonusScore += 8;
        }
        if (Score >= 100)
        {
            BonusScore += (!bUsedHint && bFirstTry) ? 12 : 4;
            if (!bUsedHint && bFirstTry)
            {
                ++PerfectSolveCount;
            }
        }
        if (CurrentLearningStreak >= 3)
        {
            BonusScore += FMath::Clamp(CurrentLearningStreak * 2, 6, 20);
        }
    }
    else
    {
        ++FailedValidationAttempts;
        CurrentLearningStreak = 0;
    }

    if (BonusScore > 0)
    {
        CodingScore += BonusScore;
    }
    SavePersistentRun();
    return BonusScore;
}

FString UCodeRescueGameInstance::GetLearningMasteryTitle() const
{
    if (PerfectSolveCount >= 12 && BestLearningStreak >= 8)
    {
        return TEXT("Algorithm Mentor");
    }
    if (SuccessfulValidationAttempts >= 20 || BestLearningStreak >= 6)
    {
        return TEXT("Debug Captain");
    }
    if (SuccessfulValidationAttempts >= 8 || BestLearningStreak >= 3)
    {
        return TEXT("Syntax Rescuer");
    }
    if (SuccessfulValidationAttempts >= 2)
    {
        return TEXT("Loop Scout");
    }
    return TEXT("New Coder");
}

FString UCodeRescueGameInstance::GetLearningProgressSummary() const
{
    const int32 SuccessRate = TotalValidationAttempts > 0
        ? FMath::RoundToInt((100.0f * SuccessfulValidationAttempts) / static_cast<float>(TotalValidationAttempts))
        : 0;
    return FString::Printf(
        TEXT("%s | Attempts %d | Success %d%% | Streak %d (best %d) | No-hint %d | Perfect %d"),
        *GetLearningMasteryTitle(),
        TotalValidationAttempts,
        SuccessRate,
        CurrentLearningStreak,
        BestLearningStreak,
        NoHintSolveCount,
        PerfectSolveCount);
}

FString UCodeRescueGameInstance::GetLanguageProgressSummary() const
{
    auto CountAt = [](const TArray<int32>& Counters, int32 Idx) -> int32
    {
        return Counters.IsValidIndex(Idx) ? Counters[Idx] : 0;
    };

    const int32 SelectedIndex = FMath::Clamp(static_cast<int32>(SelectedLanguage), 0, CodeRescueLanguageCount - 1);
    return FString::Printf(
        TEXT("%s track %d/%d"),
        *GetLanguageName(),
        CountAt(LanguageSolveCounts, SelectedIndex),
        CountAt(LanguageAttemptCounts, SelectedIndex));
}

FString UCodeRescueGameInstance::GetLanguageProfileRecapSummary() const
{
    auto CountAt = [](const TArray<int32>& Counters, int32 Idx) -> int32
    {
        return Counters.IsValidIndex(Idx) ? FMath::Max(0, Counters[Idx]) : 0;
    };

    const int32 SelectedIndex = FMath::Clamp(static_cast<int32>(SelectedLanguage), 0, CodeRescueLanguageCount - 1);
    const int32 LanguageSolves = CountAt(LanguageSolveCounts, SelectedIndex);
    const int32 LanguageAttempts = CountAt(LanguageAttemptCounts, SelectedIndex);
    const int32 LanguageNoHintSolves = CountAt(LanguageNoHintSolveCounts, SelectedIndex);
    const int32 LanguageSuccessRate = LanguageAttempts > 0
        ? FMath::Clamp(FMath::RoundToInt((100.0f * LanguageSolves) / static_cast<float>(LanguageAttempts)), 0, 100)
        : 0;

    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    const int32 FirstIncomplete = FCodeRescueCampaign::GetFirstIncompleteCityIndex(this);
    const bool bCampaignComplete = FirstIncomplete >= Missions.Num();
    const FCodeRescueCityMission* ActiveMission = Missions.IsValidIndex(FirstIncomplete) ? &Missions[FirstIncomplete] : nullptr;
    const FString StageLine = ActiveMission
        ? FString::Printf(
            TEXT("Stage recap: %s | Active city %03d %s, %s | Tier %d"),
            *ActiveMission->CurriculumStageName,
            ActiveMission->Rank,
            *ActiveMission->CityName,
            *ActiveMission->StateName,
            ActiveMission->DifficultyTier)
        : FString::Printf(TEXT("Stage recap: campaign complete | Cleared %d city route(s)"), LanguageSolves);
    const FString SlotName = MakeLanguageSaveSlotName(SelectedLanguage);
    const bool bSaveExists = DoesLanguageSaveExist(SelectedLanguage);
    const FString SaveSlotLine = FString::Printf(
        TEXT("Save-slot preview: %s | Start-screen Resume %s | %s"),
        *SlotName,
        *GetLanguageName(),
        bSaveExists ? TEXT("language save present") : TEXT("new run not saved yet"));
    const FString ReviewRecommendation = BuildLanguageProfileReviewRecommendation(
        LanguageAttempts,
        LanguageSolves,
        LanguageNoHintSolves,
        LanguageSuccessRate,
        PerfectSolveCount,
        BestLearningStreak,
        bCampaignComplete);

    return FString::Printf(
        TEXT("LANGUAGE PROFILE RECAP\nTrack: %s | Mastery: %s | Difficulty: %s\nProfile stats: solves %d | attempts %d | success %d%% | no-hint %d | perfect %d | streak %d/%d | RP %d | score %d\nRun stats: survivors %d | zombies %d | headshots %d | deaths %d | time %s\n%s\n%s\n%s"),
        *GetLanguageName(),
        *GetLearningMasteryTitle(),
        *GetDifficultyDisplayName(),
        LanguageSolves,
        LanguageAttempts,
        LanguageSuccessRate,
        LanguageNoHintSolves,
        PerfectSolveCount,
        CurrentLearningStreak,
        BestLearningStreak,
        ResearchPoints,
        CodingScore,
        RescueCount,
        KillCount,
        HeadshotCount,
        DeathCount,
        *FString::Printf(TEXT("%02d:%02d:%02d"),
            FMath::FloorToInt(RunSeconds / 3600.0f),
            FMath::FloorToInt(FMath::Fmod(RunSeconds, 3600.0f) / 60.0f),
            FMath::FloorToInt(FMath::Fmod(RunSeconds, 60.0f))),
        *StageLine,
        *SaveSlotLine,
        *ReviewRecommendation);
}

FString UCodeRescueGameInstance::GetFailSafeObjectiveBoardSummary() const
{
    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    const int32 FirstIncomplete = FCodeRescueCampaign::GetFirstIncompleteCityIndex(this);
    const FCodeRescueCityMission* ActiveMission = Missions.IsValidIndex(FirstIncomplete) ? &Missions[FirstIncomplete] : nullptr;
    const bool bCampaignComplete = FirstIncomplete >= Missions.Num();
    const FString SlotName = MakeLanguageSaveSlotName(SelectedLanguage);
    const bool bSaveExists = DoesLanguageSaveExist(SelectedLanguage);

    int32 CompletedCount = 0;
    for (int32 i = 0; i < Missions.Num(); ++i)
    {
        if (FCodeRescueCampaign::IsCityCompleted(this, i))
        {
            ++CompletedCount;
        }
    }

    if (!ActiveMission || bCampaignComplete)
    {
        return FString::Printf(
            TEXT("FAIL-SAFE OBJECTIVE BOARD\nTrack: %s only | Slot: %s | Start-screen Resume: %s\nActive route: campaign complete | Cities graduated %d/%d | extraction/debrief complete\nReturn markers: T objective jump | Backspace safe recovery | J journal | Esc pause/save\nSafety: route markers, minimap, journal, and saved-language profile remain text-first if you relaunch.\nNext action: review victory/debrief, choose another saved language from the start screen, or start a new language run."),
            *GetLanguageName(),
            *SlotName,
            bSaveExists ? TEXT("available") : TEXT("new run not saved yet"),
            CompletedCount,
            Missions.Num());
    }

    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(this, FirstIncomplete);
    const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(this, FirstIncomplete);
    const bool bSurvivorRescued = RescuedSurvivorNames.Contains(ActiveMission->SurvivorName);
    const FString Phase = !bTerminalSolved
        ? FString::Printf(TEXT("protected coding concourse %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity)
        : (!bSurvivorRescued ? TEXT("survivor route") : TEXT("extraction/debrief"));
    const FString ObjectiveState = FString::Printf(
        TEXT("%s | %s"),
        bTerminalSolved
            ? TEXT("coding clearance complete")
            : *FString::Printf(TEXT("coding clearance %d/%d"), CompletedChallenges, FCodeRescueCampaign::RequiredChallengesPerCity),
        bSurvivorRescued ? TEXT("survivor rescued") : TEXT("survivor rescue pending"));
    const FString NextAction = !bTerminalSolved
        ? FString::Printf(
            TEXT("Next action: press T for objective jump to protected %s coding station %d/%d; combat pressure is suppressed while learning."),
            *GetLanguageName(),
            CompletedChallenges + 1,
            FCodeRescueCampaign::RequiredChallengesPerCity)
        : (!bSurvivorRescued
            ? FString::Printf(
                TEXT("Next action: follow the cyan survivor marker to %s, then confirm the rescue handoff."),
                *ActiveMission->SurvivorName)
            : TEXT("Next action: return to helipad extraction/debrief and save before switching languages."));

    return FString::Printf(
        TEXT("FAIL-SAFE OBJECTIVE BOARD\nTrack: %s only | Slot: %s | Start-screen Resume: %s\nActive route: %03d %s, %s | %s | %s | %s\nReturn markers: T objective jump | Backspace safe recovery | J journal | Esc pause/save\nSafety: protected terminal suppresses combat pressure; route markers, minimap, and journal stay text-first if you lose the path.\n%s"),
        *GetLanguageName(),
        *SlotName,
        bSaveExists ? TEXT("available") : TEXT("new run not saved yet"),
        ActiveMission->Rank,
        *ActiveMission->CityName,
        *ActiveMission->StateName,
        *Phase,
        *ActiveMission->TerminalTitle,
        *ObjectiveState,
        *NextAction);
}

void UCodeRescueGameInstance::RecordLearningDebrief(
    const FString& ChallengeId,
    const FString& ConceptLabel,
    const FString& LanguageLabel,
    int32 Score,
    const FString& Summary)
{
    if (ChallengeId.IsEmpty() || Summary.IsEmpty())
    {
        return;
    }

    FString CompactSummary = Summary;
    CompactSummary.ReplaceInline(TEXT("\r"), TEXT(""));
    CompactSummary.ReplaceInline(TEXT("\n\n"), TEXT("\n"));
    if (CompactSummary.Len() > 900)
    {
        CompactSummary = CompactSummary.Left(897) + TEXT("...");
    }

    LastLearningDebriefChallengeId = ChallengeId;
    LastLearningDebriefConcept = ConceptLabel.IsEmpty() ? TEXT("Concept review") : ConceptLabel;
    LastLearningDebriefLanguage = LanguageLabel.IsEmpty() ? GetLanguageName() : LanguageLabel;
    LastLearningDebriefScore = FMath::Clamp(Score, 0, 100);
    LastLearningDebriefSummary = CompactSummary;
    bHasLearningDebriefState = true;
    SavePersistentRun();
}

FString UCodeRescueGameInstance::GetLearningDebriefJournalSummary() const
{
    if (!bHasLearningDebriefState || LastLearningDebriefSummary.IsEmpty())
    {
        return FString::Printf(
            TEXT("LAST LEARNING DEBRIEF\nNo solved terminal debrief saved yet for this %s run. Validate a terminal to pin the concept proof, language transfer, and next practice here."),
            *GetLanguageName());
    }

    return FString::Printf(
        TEXT("LAST LEARNING DEBRIEF\nTrack: %s | Challenge: %s | Concept: %s | Score: %d | Slot: %s\n%s"),
        *LastLearningDebriefLanguage,
        *LastLearningDebriefChallengeId,
        *LastLearningDebriefConcept,
        LastLearningDebriefScore,
        *SaveSlotName,
        *LastLearningDebriefSummary);
}

FString UCodeRescueGameInstance::GetChallengeReplayJournalSummary() const
{
    if (!bHasLearningDebriefState || LastLearningDebriefChallengeId.IsEmpty())
    {
        return FString::Printf(
            TEXT("CHALLENGE REPLAY BRIEF\nNo replay brief saved yet for this %s run. Solve or bypass a terminal, then reopen J to review the visible goal, hidden-test replay note, and next practice action from the saved language profile."),
            *GetLanguageName());
    }

    const bool bAssisted = LastLearningDebriefSummary.Contains(TEXT("ASSISTED LEARNING DEBRIEF"))
        || LastLearningDebriefSummary.Contains(TEXT("Bypass kit"));
    const FString ReplayStatus = bAssisted
        ? TEXT("ASSISTED ROUTE OPEN - replay clean solve before chasing perfect rewards")
        : TEXT("CLEAN SOLVE SAVED - replay notes ready for spaced practice");
    const FString ConceptLabel = LastLearningDebriefConcept.IsEmpty()
        ? TEXT("Concept review")
        : LastLearningDebriefConcept;
    const FString LanguageLabel = LastLearningDebriefLanguage.IsEmpty()
        ? GetLanguageName()
        : LastLearningDebriefLanguage;

    return FString::Printf(
        TEXT("CHALLENGE REPLAY BRIEF\nTrack: %s | Challenge: %s | Concept: %s | Score: %d | Slot: %s\nReplay status: %s\n%s\n%s\n%s\nJournal hook: this survives start-screen Resume; use it to rehearse before the next live terminal."),
        *LanguageLabel,
        *LastLearningDebriefChallengeId,
        *ConceptLabel,
        LastLearningDebriefScore,
        *SaveSlotName,
        *ReplayStatus,
        *GetChallengeReplayVisibleGoal(LastLearningDebriefChallengeId),
        *GetChallengeReplayHiddenNote(LastLearningDebriefChallengeId),
        *GetChallengeReplayPracticeAction(LastLearningDebriefChallengeId));
}

void UCodeRescueGameInstance::RecordSurvivorIntelDossier(
    const FString& TerminalId,
    const FString& SurvivorName,
    const FString& CityLabel,
    const FString& LanguageLabel,
    const FString& StatusLabel,
    int32 Score,
    const FString& Summary)
{
    if (TerminalId.IsEmpty() || SurvivorName.IsEmpty() || Summary.IsEmpty())
    {
        return;
    }

    FString CompactSummary = Summary;
    CompactSummary.ReplaceInline(TEXT("\r"), TEXT(""));
    CompactSummary.ReplaceInline(TEXT("\n\n"), TEXT("\n"));
    if (CompactSummary.Len() > 720)
    {
        CompactSummary = CompactSummary.Left(717) + TEXT("...");
    }

    LastSurvivorIntelTerminalId = TerminalId;
    LastSurvivorIntelSurvivorName = SurvivorName;
    LastSurvivorIntelCityLabel = CityLabel.IsEmpty() ? TEXT("Active city route") : CityLabel;
    LastSurvivorIntelLanguage = LanguageLabel.IsEmpty() ? GetLanguageName() : LanguageLabel;
    LastSurvivorIntelStatus = StatusLabel.IsEmpty() ? TEXT("ROUTE OPEN - survivor marker broadcasting") : StatusLabel;
    LastSurvivorIntelScore = FMath::Clamp(Score, 0, 100);
    LastSurvivorIntelSummary = CompactSummary;
    bHasSurvivorIntelArchiveState = true;
    SavePersistentRun();
}

void UCodeRescueGameInstance::MarkTerminalRewardChoiceEligible(const FString& TerminalId)
{
    if (TerminalId.IsEmpty() || HasClaimedTerminalRewardChoice(TerminalId))
    {
        return;
    }

    RewardChoiceEligibleTerminalIds.AddUnique(TerminalId);
    SavePersistentRun();
}

bool UCodeRescueGameInstance::HasClaimedTerminalRewardChoice(const FString& TerminalId) const
{
    return !TerminalId.IsEmpty() && ClaimedTerminalRewardChoiceIds.Contains(TerminalId);
}

bool UCodeRescueGameInstance::IsTerminalRewardChoiceAvailable(const FString& TerminalId) const
{
    return !TerminalId.IsEmpty()
        && SolvedTerminalIds.Contains(TerminalId)
        && RewardChoiceEligibleTerminalIds.Contains(TerminalId)
        && !HasClaimedTerminalRewardChoice(TerminalId);
}

bool UCodeRescueGameInstance::ClaimTerminalRewardChoice(
    const FString& TerminalId,
    const FString& ChoiceId,
    ACodeRescueCharacter* Player)
{
    if (!IsTerminalRewardChoiceAvailable(TerminalId))
    {
        return false;
    }

    FString ChoiceKey = ChoiceId.TrimStartAndEnd().ToLower();
    FString ChoiceLabel;
    FString RewardSummary;

    if (ChoiceKey == TEXT("research") || ChoiceKey == TEXT("rp"))
    {
        ResearchPoints += 2;
        ChoiceKey = TEXT("research");
        ChoiceLabel = TEXT("Research Boost");
        RewardSummary = FString::Printf(
            TEXT("Reward choice claimed: Research Boost | +2 ResearchPoints | total RP %d"),
            ResearchPoints);
    }
    else if (ChoiceKey == TEXT("fieldkit") || ChoiceKey == TEXT("field"))
    {
        if (!Player)
        {
            return false;
        }
        const int32 AmmoGranted = Player->AddAmmo(45);
        const int32 MedkitsGranted = Player->AddMedkits(1);
        const int32 ArmorGranted = Player->AddArmorPlates(1);
        ChoiceKey = TEXT("fieldkit");
        ChoiceLabel = TEXT("Field Kit");
        RewardSummary = FString::Printf(
            TEXT("Reward choice claimed: Field Kit | ammo +%d | medkit +%d | armor +%d"),
            AmmoGranted,
            MedkitsGranted,
            ArmorGranted);
    }
    else if (ChoiceKey == TEXT("crafting") || ChoiceKey == TEXT("scrap"))
    {
        if (!Player)
        {
            return false;
        }
        const int32 ScrapTotal = Player->GrantScrap(5);
        const int32 BypassGranted = Player->AddBypassKits(1);
        ChoiceKey = TEXT("crafting");
        ChoiceLabel = TEXT("Crafting Cache");
        RewardSummary = FString::Printf(
            TEXT("Reward choice claimed: Crafting Cache | scrap total %d | bypass kit +%d"),
            ScrapTotal,
            BypassGranted);
    }
    else
    {
        return false;
    }

    RewardChoiceEligibleTerminalIds.Remove(TerminalId);
    ClaimedTerminalRewardChoiceIds.AddUnique(TerminalId);
    LastTerminalRewardChoiceTerminalId = TerminalId;
    LastTerminalRewardChoiceId = ChoiceLabel;
    LastTerminalRewardChoiceSummary = FString::Printf(
        TEXT("REWARD CHOICE KIOSK\nTrack: %s | Terminal: %s | Slot: %s\n%s\nStart-screen Resume keeps this one-time choice in the selected-language profile."),
        *GetLanguageName(),
        *TerminalId,
        *SaveSlotName,
        *RewardSummary);
    bHasTerminalRewardChoiceState = true;
    SavePersistentRun();
    return true;
}

FString UCodeRescueGameInstance::GetTerminalRewardChoiceSummary(const FString& TerminalId) const
{
    if (!TerminalId.IsEmpty() && HasClaimedTerminalRewardChoice(TerminalId))
    {
        if (bHasTerminalRewardChoiceState && LastTerminalRewardChoiceTerminalId == TerminalId)
        {
            return LastTerminalRewardChoiceSummary;
        }
        return FString::Printf(
            TEXT("REWARD CHOICE KIOSK\nTrack: %s | Terminal: %s | Slot: %s\nReward already claimed for this terminal in the selected-language profile."),
            *GetLanguageName(),
            *TerminalId,
            *SaveSlotName);
    }
    if (!TerminalId.IsEmpty() && IsTerminalRewardChoiceAvailable(TerminalId))
    {
        return FString::Printf(
            TEXT("REWARD CHOICE KIOSK\nTrack: %s | Terminal: %s | Slot: %s\nChoose one: Research Boost (+2 RP), Field Kit (ammo, medkit, armor), or Crafting Cache (scrap, bypass kit). One choice saves to this language run."),
            *GetLanguageName(),
            *TerminalId,
            *SaveSlotName);
    }
    return FString::Printf(
        TEXT("REWARD CHOICE KIOSK\nTrack: %s | Slot: %s\nNo live-solve reward choice is pending. Practice runs and bypass-kit solves do not unlock the kiosk."),
        *GetLanguageName(),
        *SaveSlotName);
}

FString UCodeRescueGameInstance::GetSurvivorIntelArchiveSummary() const
{
    if (!bHasSurvivorIntelArchiveState || LastSurvivorIntelSummary.IsEmpty())
    {
        return FString::Printf(
            TEXT("SURVIVOR INTEL ARCHIVE\nNo survivor intel has been uploaded yet for this %s run. Solve the active terminal to pin the latest contact, route, score, and rescue next step here."),
            *GetLanguageName());
    }

    return FString::Printf(
        TEXT("SURVIVOR INTEL ARCHIVE\nTrack: %s | Status: %s | Score: %d | Slot: %s\nContact: %s | Terminal: %s | Route: %s\n%s"),
        *LastSurvivorIntelLanguage,
        *LastSurvivorIntelStatus,
        LastSurvivorIntelScore,
        *SaveSlotName,
        *LastSurvivorIntelSurvivorName,
        *LastSurvivorIntelTerminalId,
        *LastSurvivorIntelCityLabel,
        *LastSurvivorIntelSummary);
}

bool UCodeRescueGameInstance::DeletePersistentRun()
{
    ResetRun();
    return UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
}

void UCodeRescueGameInstance::RecordTerminalSolved(const FString& MissionId, int32 Score)
{
    if (MissionId.IsEmpty())
    {
        return;
    }

    bool bFoundProgress = false;
    for (FCodeRescueMissionProgress& Progress : MissionProgress)
    {
        if (Progress.MissionId == MissionId)
        {
            Progress.Attempts = FMath::Max(Progress.Attempts, 1);
            Progress.BestScore = FMath::Max(Progress.BestScore, Score);
            Progress.bCompleted = Progress.bCompleted || Score > 0;
            bFoundProgress = true;
            break;
        }
    }
    if (!bFoundProgress)
    {
        FCodeRescueMissionProgress NewProgress;
        NewProgress.MissionId = MissionId;
        NewProgress.Attempts = 1;
        NewProgress.BestScore = Score;
        NewProgress.bCompleted = Score > 0;
        MissionProgress.Add(NewProgress);
    }

    if (SolvedTerminalIds.Contains(MissionId))
    {
        SavePersistentRun();
        return;
    }

    // Persist *which* terminal was solved so reload skips it instead of
    // re-spawning it as interactable.
    MarkTerminalSolved(MissionId);
    CompletedMissionIds.AddUnique(MissionId);
    CodingScore += Score;
    TerminalsSolved = SolvedTerminalIds.Num();
    SavePersistentRun();
}

void UCodeRescueGameInstance::RecordTerminalAttempt(const FString& MissionId, int32 Score, bool bCompleted)
{
    if (MissionId.IsEmpty())
    {
        return;
    }

    bool bFoundProgress = false;
    for (FCodeRescueMissionProgress& Progress : MissionProgress)
    {
        if (Progress.MissionId == MissionId)
        {
            ++Progress.Attempts;
            Progress.BestScore = FMath::Max(Progress.BestScore, Score);
            Progress.bCompleted = Progress.bCompleted || bCompleted;
            bFoundProgress = true;
            break;
        }
    }
    if (!bFoundProgress)
    {
        FCodeRescueMissionProgress NewProgress;
        NewProgress.MissionId = MissionId;
        NewProgress.Attempts = 1;
        NewProgress.BestScore = Score;
        NewProgress.bCompleted = bCompleted;
        MissionProgress.Add(NewProgress);
    }

    SavePersistentRun();
}

void UCodeRescueGameInstance::RecordConceptAttempt(const FString& ConceptId, bool bSuccess, int32 ScoreDelta)
{
    if (!ConceptId.IsEmpty())
    {
        bool bFoundProgress = false;
        for (FCodeRescueConceptProgress& Progress : ConceptProgress)
        {
            if (Progress.ConceptId == ConceptId)
            {
                if (bSuccess)
                {
                    ++Progress.SuccessfulValidations;
                    Progress.MasteryScore = FMath::Clamp(Progress.MasteryScore + FMath::Max(1, ScoreDelta), 0, 100);
                }
                else
                {
                    ++Progress.FailedValidations;
                    Progress.MasteryScore = FMath::Clamp(Progress.MasteryScore + FMath::Min(0, ScoreDelta), 0, 100);
                }
                bFoundProgress = true;
                break;
            }
        }
        if (!bFoundProgress)
        {
            FCodeRescueConceptProgress NewProgress;
            NewProgress.ConceptId = ConceptId;
            NewProgress.MasteryScore = bSuccess ? FMath::Clamp(FMath::Max(1, ScoreDelta), 0, 100) : 0;
            NewProgress.SuccessfulValidations = bSuccess ? 1 : 0;
            NewProgress.FailedValidations = bSuccess ? 0 : 1;
            ConceptProgress.Add(NewProgress);
        }
    }
    CodingScore = FMath::Max(0, CodingScore + ScoreDelta);
    SavePersistentRun();
}

void UCodeRescueGameInstance::MarkTerminalSolved(const FString& TerminalId)
{
    if (TerminalId.IsEmpty()) return;
    SolvedTerminalIds.AddUnique(TerminalId);
    TerminalsSolved = SolvedTerminalIds.Num();
}

void UCodeRescueGameInstance::MarkSurvivorRescued(const FString& SurvivorName)
{
    if (SurvivorName.IsEmpty()) return;
    RescuedSurvivorNames.AddUnique(SurvivorName);
    SurvivorsRescued = RescuedSurvivorNames.Num();
    if (bHasSurvivorIntelArchiveState && LastSurvivorIntelSurvivorName == SurvivorName)
    {
        LastSurvivorIntelStatus = TEXT("RESCUED - extraction debrief ready");
        if (!LastSurvivorIntelSummary.Contains(TEXT("Archive update: survivor rescued")))
        {
            LastSurvivorIntelSummary += TEXT("\nArchive update: survivor rescued; extraction and debrief route are ready.");
        }
    }
}

void UCodeRescueGameInstance::MarkZombieNeutralized(int32 ZombieId)
{
    // -1 sentinel means "no ID assigned" — skip rather than persist garbage.
    if (ZombieId < 0) return;
    NeutralizedZombieIds.AddUnique(ZombieId);
    ZombiesNeutralized = NeutralizedZombieIds.Num();
}

bool UCodeRescueGameInstance::IsPersistentStoryZombieId(int32 ZombieId)
{
    // 2,000,000-2,099,999 are city bosses; 2,100,000-2,199,999 are the
    // authored named minibosses. Other ID bands describe renewable combat
    // populations whose individual deaths remain useful statistics only.
    return ZombieId >= 2000000 && ZombieId < 2200000;
}

bool UCodeRescueGameInstance::HasCollectedCaseFile(const FString& CaseFileId) const
{
    return !CaseFileId.IsEmpty() && CollectedCaseFileIds.Contains(CaseFileId);
}

bool UCodeRescueGameInstance::RecordCaseFileCollected(const FString& CaseFileId, const FString& CaseFileTitle)
{
    if (CaseFileId.IsEmpty())
    {
        return false;
    }

    const int32 BeforeCount = CollectedCaseFileIds.Num();
    CollectedCaseFileIds.AddUnique(CaseFileId);
    if (!CaseFileTitle.IsEmpty())
    {
        LastCollectedCaseFileTitle = CaseFileTitle;
    }
    return CollectedCaseFileIds.Num() > BeforeCount;
}

FString UCodeRescueGameInstance::GetCaseFileCollectionSummary() const
{
    const FString LastTitle = LastCollectedCaseFileTitle.IsEmpty()
        ? FString(TEXT("none yet"))
        : LastCollectedCaseFileTitle;
    return FString::Printf(
        TEXT("Case files: %d collected | Last: %s"),
        CollectedCaseFileIds.Num(),
        *LastTitle);
}

bool UCodeRescueGameInstance::HasFriendlyNPCServiceCooldown(const FString& ServiceId) const
{
    return !ServiceId.IsEmpty() && UsedFriendlyNPCServiceIds.Contains(ServiceId);
}

bool UCodeRescueGameInstance::MarkFriendlyNPCServiceUsed(const FString& ServiceId)
{
    if (ServiceId.IsEmpty())
    {
        return false;
    }

    const int32 BeforeCount = UsedFriendlyNPCServiceIds.Num();
    UsedFriendlyNPCServiceIds.AddUnique(ServiceId);
    UsedFriendlyNPCServiceIds.RemoveAll([](const FString& Id) { return Id.IsEmpty(); });
    bHasFriendlyNPCServiceState = true;
    SavePersistentRun();
    return UsedFriendlyNPCServiceIds.Num() > BeforeCount;
}

bool UCodeRescueGameInstance::ClearFriendlyNPCServiceCooldown(const FString& ServiceId)
{
    if (ServiceId.IsEmpty())
    {
        return false;
    }

    const int32 RemovedCount = UsedFriendlyNPCServiceIds.Remove(ServiceId);
    bHasFriendlyNPCServiceState = true;
    if (RemovedCount > 0)
    {
        SavePersistentRun();
    }
    return RemovedCount > 0;
}

void UCodeRescueGameInstance::ResetFriendlyNPCServiceCooldowns()
{
    const bool bHadCooldowns = UsedFriendlyNPCServiceIds.Num() > 0;
    UsedFriendlyNPCServiceIds.Reset();
    bHasFriendlyNPCServiceState = true;
    if (bHadCooldowns)
    {
        SavePersistentRun();
    }
}

FString UCodeRescueGameInstance::GetFriendlyNPCServiceSummary() const
{
    return FString::Printf(
        TEXT("Support services: %d used this day-cycle | Language save: %s"),
        UsedFriendlyNPCServiceIds.Num(),
        *GetLanguageName());
}

void UCodeRescueGameInstance::RecordZombieVariant(int32 ZombieId, EZombieVariant Variant)
{
    if (ZombieId < 0) return;
    // Update existing record if present, else append. Idempotent on ZombieId
    // so SpawnWorld can safely call this even if a future change re-runs spawn.
    for (FCodeRescueZombieVariantRecord& R : SpawnedZombieVariants)
    {
        if (R.ZombieId == ZombieId)
        {
            R.Variant = Variant;
            return;
        }
    }
    FCodeRescueZombieVariantRecord NewRec;
    NewRec.ZombieId = ZombieId;
    NewRec.Variant = Variant;
    SpawnedZombieVariants.Add(NewRec);
}

EZombieVariant UCodeRescueGameInstance::GetRecordedZombieVariant(int32 ZombieId) const
{
    for (const FCodeRescueZombieVariantRecord& R : SpawnedZombieVariants)
    {
        if (R.ZombieId == ZombieId)
        {
            return R.Variant;
        }
    }
    return EZombieVariant::Default;
}

void UCodeRescueGameInstance::CaptureWorldStateFromLevel(UWorld* World)
{
    if (!World)
    {
        return;
    }

    if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        LastPlayerLocation = Pawn->GetActorLocation();
        // Prefer the controller's view rotation so first-person look pitch is
        // preserved; fall back to the pawn rotation if no PC is available.
        if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
        {
            LastPlayerRotation = PC->GetControlRotation();
        }
        else
        {
            LastPlayerRotation = Pawn->GetActorRotation();
        }

        if (ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(Pawn))
        {
            CurrentObjectiveIndex = Character->GetObjectiveIndex();
            LastPlayerHealth = Character->Health;
            LastPlayerAmmo = Character->Ammo;
            LastActiveWeapon = Character->ActiveWeapon;
            LastWeaponMagazines = Character->WeaponMagazines;
            LastWeaponReserveAmmo = Character->WeaponReserveAmmo;
            bHasWeaponQuickSlotState = true;
            LastPlayerMedkits = Character->Medkits;
            LastPlayerArmorPlates = Character->ArmorPlates;
            LastPlayerFlares = Character->FlareCount;
            LastPlayerSmokes = Character->SmokeCount;
            LastPlayerStims = Character->StimCount;
            LastPlayerScrap = Character->Scrap;
            LastPlayerRadioScannerCharges = Character->RadioScannerCharges;
            LastPlayerFlashlightBatteries = Character->FlashlightBatteries;
            LastPlayerBypassKits = Character->BypassKits;
            LastPlayerAmmoPouchCapacityBonus = Character->AmmoPouchCapacityBonus;
            bHasPlayerTacticalGear = true;
            bHasPlayerResources = true;
        }

        // Once we have a real player transform, the apply path is allowed to
        // teleport on subsequent loads.
        bHasWorldState = true;
    }
}

void UCodeRescueGameInstance::ApplyWorldStateToLevel(UWorld* World)
{
    if (!World)
    {
        return;
    }

    ApplyObjectiveStateToLevel(World);

    // Player transform + objective index: only restore on a save that has
    // actually captured world state, so a brand-new save file does not
    // teleport the pawn to (0,0,0).
    if (bHasWorldState)
    {
        if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            Pawn->SetActorLocation(LastPlayerLocation);
            Pawn->SetActorRotation(LastPlayerRotation);
            if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
            {
                PC->SetControlRotation(LastPlayerRotation);
            }
            if (ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(Pawn))
            {
                Character->SetObjectiveIndex(CurrentObjectiveIndex);
                if (bHasPlayerResources)
                {
                    if (bHasPlayerTacticalGear)
                    {
                        Character->RestorePlayerResourcesDetailed(
                            LastPlayerHealth,
                            LastPlayerAmmo,
                            LastPlayerMedkits,
                            LastPlayerArmorPlates,
                            LastPlayerFlares,
                            LastPlayerSmokes,
                            LastPlayerStims,
                            LastPlayerScrap,
                            LastPlayerRadioScannerCharges,
                            LastPlayerFlashlightBatteries,
                            LastPlayerBypassKits,
                            LastPlayerAmmoPouchCapacityBonus);
                    }
                    else
                    {
                        Character->RestorePlayerResources(LastPlayerHealth, LastPlayerAmmo, LastPlayerMedkits);
                    }
                    if (bHasWeaponQuickSlotState)
                    {
                        Character->RestoreWeaponQuickSlotState(
                            LastActiveWeapon,
                            LastWeaponMagazines,
                            LastWeaponReserveAmmo);
                    }
                }
            }
        }
    }
}

void UCodeRescueGameInstance::ApplyObjectiveStateToLevel(UWorld* World)
{
    if (!World)
    {
        return;
    }

    // Solved coding terminals: hide and disable collision so they no longer
    // appear as interactable. MarkSolved already does both, plus sets bSolved
    // (which FindNearestInteractable filters on).
    if (SolvedTerminalIds.Num() > 0)
    {
        for (TActorIterator<ACodingTerminalActor> It(World); It; ++It)
        {
            ACodingTerminalActor* Terminal = *It;
            if (IsValid(Terminal) && SolvedTerminalIds.Contains(Terminal->Challenge.Id))
            {
                // SpawnTerminal reconstructs the city route once from the
                // primary station after all ten required challenges are done.
                // Replaying this response for every saved station created
                // duplicate geometry, overlapping narration, and interactive
                // "CODE ACCEPTED" readers during ordinary walking.
                Terminal->MarkSolved();
            }
        }
    }

    // Rescued survivors: set bRescued silently and hide. We don't call
    // Rescue() here because that would re-increment SurvivorsRescued and
    // re-trigger SavePersistentRun.
    if (RescuedSurvivorNames.Num() > 0)
    {
        for (TActorIterator<ASurvivorActor> It(World); It; ++It)
        {
            ASurvivorActor* Survivor = *It;
            if (IsValid(Survivor) && RescuedSurvivorNames.Contains(Survivor->SurvivorName))
            {
                Survivor->bRescued = true;
                Survivor->SetActorHiddenInGame(true);
                Survivor->SetActorEnableCollision(false);
            }
        }
    }

    // Only authored story threats stay gone. Ambient IDs remain in the save
    // for aggregate combat statistics, but must not empty future sessions.
    if (NeutralizedZombieIds.Num() > 0)
    {
        TArray<ACodeZombieActor*> ToDestroy;
        for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
        {
            ACodeZombieActor* Zombie = *It;
            if (IsValid(Zombie)
                && IsPersistentStoryZombieId(Zombie->ZombieId)
                && NeutralizedZombieIds.Contains(Zombie->ZombieId))
            {
                ToDestroy.Add(Zombie);
            }
        }
        for (ACodeZombieActor* Zombie : ToDestroy)
        {
            Zombie->Destroy();
        }
    }

    if (CollectedCaseFileIds.Num() > 0)
    {
        for (TActorIterator<ACaseFilePickupActor> It(World); It; ++It)
        {
            ACaseFilePickupActor* CaseFile = *It;
            if (IsValid(CaseFile) && CollectedCaseFileIds.Contains(CaseFile->CaseFileId))
            {
                CaseFile->bCollected = true;
                CaseFile->SetActorHiddenInGame(true);
                CaseFile->SetActorEnableCollision(false);
            }
        }
    }

    for (TActorIterator<AFriendlyNPCActor> It(World); It; ++It)
    {
        AFriendlyNPCActor* NPC = *It;
        if (IsValid(NPC))
        {
            NPC->ApplySavedServiceState();
        }
    }
}

// ---- #43 background music system ------------------------------------------
void UCodeRescueGameInstance::PlayMusic(USoundBase* NewTrack, float FadeInDuration)
{
    if (!MusicComponent)
    {
        MusicComponent = NewObject<UAudioComponent>(this, TEXT("MusicAudio"));
        if (MusicComponent)
        {
            MusicComponent->bAutoActivate = false;
            MusicComponent->bAllowSpatialization = false;
            MusicComponent->VolumeMultiplier = 0.45f * GetMusicVolumeScalar() * GetReactiveThreatMusicScalar();
            MusicComponent->RegisterComponentWithWorld(GetWorld());
        }
    }
    if (!MusicComponent) return;

    if (!NewTrack)
    {
        MusicComponent->FadeOut(1.0f, 0.0f);
        return;
    }
    MusicComponent->Stop();
    MusicComponent->SetSound(NewTrack);
    MusicComponent->FadeIn(FMath::Max(0.0f, FadeInDuration), 0.45f * GetMusicVolumeScalar() * GetReactiveThreatMusicScalar());
}

void UCodeRescueGameInstance::PlayMenuMusic()
{
    if (USoundBase* T = MenuMusic.LoadSynchronous()) PlayMusic(T);
}

void UCodeRescueGameInstance::PlayCityMusic()
{
    if (USoundBase* T = AmbientCityMusic.LoadSynchronous()) PlayMusic(T);
}

void UCodeRescueGameInstance::PlayHordeStinger()
{
    if (USoundBase* T = BossHordeStinger.LoadSynchronous()) PlayMusic(T, 0.2f);
}

// ============================================================================
// #54 crafting + #55 skill tree
// ============================================================================

#include "CodeRescueCharacter.h"

bool UCodeRescueGameInstance::CraftFlare(ACodeRescueCharacter* Player)
{
    if (!Player || Player->Scrap < 2) return false;
    Player->Scrap -= 2;
    Player->FlareCount += 1;
    SavePersistentRun();
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Crafted: 1 Flare"));
    return true;
}

bool UCodeRescueGameInstance::CraftStim(ACodeRescueCharacter* Player)
{
    if (!Player || Player->Scrap < 5 || Player->Medkits < 1) return false;
    Player->Scrap -= 5;
    Player->Medkits -= 1;
    Player->StimCount += 1;
    SavePersistentRun();
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Crafted: 1 Stim"));
    return true;
}

bool UCodeRescueGameInstance::CraftGrenade(ACodeRescueCharacter* Player)
{
    if (!Player || Player->Scrap < 8) return false;
    Player->Scrap -= 8;
    Player->EnsureWeaponStateInitialized();
    Player->AddAmmoToWeaponIndex(static_cast<int32>(EWeaponType::Grenade), 1);
    Player->RefreshLegacyAmmoFromWeaponReserves();
    SavePersistentRun();
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Crafted: 1 Grenade"));
    return true;
}

bool UCodeRescueGameInstance::IsSkillUnlocked(int32 NodeIndex) const
{
    if (NodeIndex < 0 || NodeIndex >= 64) return false;
    return (SkillTreeUnlocked & ((int64)1 << NodeIndex)) != 0;
}

bool UCodeRescueGameInstance::TryUnlockSkill(int32 NodeIndex)
{
    if (NodeIndex < 0 || NodeIndex >= 8) return false;
    if (IsSkillUnlocked(NodeIndex)) return false;
    if (ResearchPoints < 2) return false;
    ResearchPoints -= 2;
    SkillTreeUnlocked |= ((int64)1 << NodeIndex);
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
        FString::Printf(TEXT("Skill unlocked: node %d (-2 RP)"), NodeIndex));
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ACodeRescueCharacter* Player = Cast<ACodeRescueCharacter>(PC->GetPawn()))
        {
            ApplySkillTreeToPlayer(Player);
        }
    }
    SavePersistentRun();
    return true;
}

void UCodeRescueGameInstance::ApplySkillTreeToPlayer(ACodeRescueCharacter* Player)
{
    if (!Player) return;
    const int64 PendingMask = SkillTreeUnlocked & ~Player->AppliedSkillTreeUnlockedMask;
    if (PendingMask == 0)
    {
        return;
    }

    auto HasPendingSkill = [PendingMask](int32 NodeIndex)
    {
        return (PendingMask & ((int64)1 << NodeIndex)) != 0;
    };

    if (HasPendingSkill(0))
    {
        Player->MaxStamina += 20.0f;
        Player->Stamina = FMath::Min(Player->MaxStamina, Player->Stamina + 20.0f);
    }
    if (HasPendingSkill(1))
    {
        for (FWeaponDef& Weapon : Player->WeaponLoadout)
        {
            Weapon.ReloadDuration *= 0.75f;
        }
    }
    if (HasPendingSkill(2) && Player->WeaponLoadout.IsValidIndex(static_cast<int32>(EWeaponType::Pistol)))
    {
        Player->WeaponLoadout[static_cast<int32>(EWeaponType::Pistol)].MagazineSize += 6;
    }
    if (HasPendingSkill(3)) Player->FlareCount = FMath::Max(Player->FlareCount, 4);
    if (HasPendingSkill(4)) Player->SmokeCount = FMath::Max(Player->SmokeCount, 3);
    if (HasPendingSkill(5)) Player->StimCount  = FMath::Max(Player->StimCount,  3);
    if (HasPendingSkill(6))
    {
        Player->MaxHealth += 25.0f;
        if (Player->Health > 0.0f)
        {
            Player->Health = FMath::Min(Player->MaxHealth, Player->Health + 25.0f);
        }
    }
    if (HasPendingSkill(7))
    {
        Player->BarricadeScrapCost = FMath::Max(1, Player->BarricadeScrapCost - 1);
    }

    Player->AppliedSkillTreeUnlockedMask |= PendingMask;
    Player->SyncActiveWeaponStateFromLoadout();
}

void UCodeRescueGameInstance::OpenSkillTreeWidget()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (UCodeRescueSkillTreeWidget* Widget = CreateWidget<UCodeRescueSkillTreeWidget>(PC, UCodeRescueSkillTreeWidget::StaticClass()))
        {
            Widget->AddToViewport(175);
        }
    }
}

void UCodeRescueGameInstance::OpenCraftingWidget()
{
    if (!GEngine) return;
    FString Text = TEXT("CRAFTING WORKBENCH\n");
    Text += TEXT("  2 scrap            -> 1 Flare    (call CraftFlare)\n");
    Text += TEXT("  5 scrap + 1 medkit -> 1 Stim     (call CraftStim)\n");
    Text += TEXT("  8 scrap            -> 1 Grenade  (call CraftGrenade)\n");
    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, Text);
}

// ============================================================================
// #56 local co-op (split-screen)
// ============================================================================
bool UCodeRescueGameInstance::EnableSecondPlayer()
{
    FString ErrorMsg;
    ULocalPlayer* P2 = CreateLocalPlayer(1, ErrorMsg, true);
    if (!P2)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red,
            FString::Printf(TEXT("CreateLocalPlayer failed: %s"), *ErrorMsg));
        return false;
    }
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Second player enabled."));
    return true;
}

bool UCodeRescueGameInstance::DisableSecondPlayer()
{
    if (ULocalPlayer* P2 = GetLocalPlayerByIndex(1))
    {
        return RemoveLocalPlayer(P2);
    }
    return false;
}
