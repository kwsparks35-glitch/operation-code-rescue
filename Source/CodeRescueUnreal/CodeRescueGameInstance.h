#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CodeRescueSaveGame.h"
#include "CodeRescueTypes.h"
#include "CodeRescueGameInstance.generated.h"

UCLASS()
class CODERESCUEUNREAL_API UCodeRescueGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UCodeRescueGameInstance();
    virtual void Init() override;

    UPROPERTY(BlueprintReadWrite)
    ECodingLanguage SelectedLanguage;

    /** Runtime-only launch gate. A fresh app session must show the language
     *  chooser before active play; the selected language sets this flag before
     *  reloading the campaign entry map. It is intentionally not saved. */
    UPROPERTY(BlueprintReadWrite, Transient, Category="Launch")
    bool bHasSelectedLaunchLanguageThisSession = false;

    /** Selected difficulty. Scales zombie health and attack damage at spawn.
     *  Persisted in the SaveGame so the player resumes on the same setting. */
    UPROPERTY(BlueprintReadWrite, Category="Difficulty")
    EGameDifficulty Difficulty = EGameDifficulty::Normal;

    UPROPERTY(BlueprintReadWrite, Category="Operator")
    FString OperatorCallsign = TEXT("Rhea Calder");

    UPROPERTY(BlueprintReadWrite, Category="Operator")
    FString OperatorRoleTitle = TEXT("Rescue Operator");

    UPROPERTY(BlueprintReadWrite, Category="Operator")
    FString OperatorProfileNote = TEXT("frontline rescue route lead");

    UPROPERTY(BlueprintReadWrite, Category="Operator")
    bool bHasOperatorIdentityState = false;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Operator")
    FString GetOperatorIdentitySummary() const;

    UFUNCTION(BlueprintCallable, Category="Operator")
    void InitializeOperatorIdentityForLanguage(ECodingLanguage Language);

    /** Multiplier applied to zombie max health at spawn. */
    UFUNCTION(BlueprintCallable, Category="Difficulty")
    float GetZombieHealthMultiplier() const;

    /** Multiplier applied to zombie melee damage. */
    UFUNCTION(BlueprintCallable, Category="Difficulty")
    float GetZombieDamageMultiplier() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Difficulty")
    FString GetDifficultyDisplayName() const;

    UPROPERTY(BlueprintReadWrite)
    int32 SurvivorsRescued;

    UPROPERTY(BlueprintReadWrite)
    int32 ZombiesNeutralized;

    UPROPERTY(BlueprintReadWrite)
    int32 CodingScore;

    UPROPERTY(BlueprintReadWrite)
    int32 TerminalsSolved;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<FString> CompletedMissionIds;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<FCodeRescueMissionProgress> MissionProgress;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<FCodeRescueConceptProgress> ConceptProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
    FString SaveSlotName = TEXT("OperationCodeRescue_Profile0");

    // ---- World-state persistence ----------------------------------------
    /** Challenge.Id of every coding terminal already solved this run. */
    UPROPERTY(BlueprintReadWrite, Category="Save")
    TArray<FString> SolvedTerminalIds;

    /** SurvivorName of every survivor already rescued this run. */
    UPROPERTY(BlueprintReadWrite, Category="Save")
    TArray<FString> RescuedSurvivorNames;

    /** ZombieId of every zombie already neutralized this run. */
    UPROPERTY(BlueprintReadWrite, Category="Save")
    TArray<int32> NeutralizedZombieIds;

    /** CaseFileId of every narrative collectible gathered in this language run. */
    UPROPERTY(BlueprintReadWrite, Category="Save|Narrative")
    TArray<FString> CollectedCaseFileIds;

    /** Last collected case-file title for journal and end-state summaries. */
    UPROPERTY(BlueprintReadWrite, Category="Save|Narrative")
    FString LastCollectedCaseFileTitle;

    /** Safehouse NPC services already used in the current day/night cycle.
     *  This is saved inside the selected-language profile. */
    UPROPERTY(BlueprintReadWrite, Category="Save|NPC")
    TArray<FString> UsedFriendlyNPCServiceIds;

    UPROPERTY(BlueprintReadWrite, Category="Save|NPC")
    bool bHasFriendlyNPCServiceState = false;

    /** Variant chosen for each ZombieId at spawn time. Recorded by
     *  RecordZombieVariant (called from GameMode::SpawnWorld) and serialized
     *  so reloads reproduce the same lineup. Empty on a fresh run. */
    UPROPERTY(BlueprintReadWrite, Category="Save")
    TArray<FCodeRescueZombieVariantRecord> SpawnedZombieVariants;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 CurrentObjectiveIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    FRotator LastPlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    float LastPlayerHealth = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerAmmo = 150;

    UPROPERTY(BlueprintReadWrite, Category="Save|Weapons")
    EWeaponType LastActiveWeapon = EWeaponType::Pistol;

    UPROPERTY(BlueprintReadWrite, Category="Save|Weapons")
    TArray<int32> LastWeaponMagazines;

    UPROPERTY(BlueprintReadWrite, Category="Save|Weapons")
    TArray<int32> LastWeaponReserveAmmo;

    UPROPERTY(BlueprintReadWrite, Category="Save|Weapons")
    bool bHasWeaponQuickSlotState = false;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerMedkits = 8;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerArmorPlates = 4;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerFlares = 3;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerSmokes = 2;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerStims = 2;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerScrap = 0;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerRadioScannerCharges = 1;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerFlashlightBatteries = 1;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerBypassKits = 0;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    int32 LastPlayerAmmoPouchCapacityBonus = 0;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    bool bHasPlayerTacticalGear = false;

    UPROPERTY(BlueprintReadWrite, Category="Save")
    bool bHasPlayerResources = false;

    /** True once a save has captured the world. ApplyWorldStateToLevel uses
     *  this to decide whether teleporting the player is safe. */
    UPROPERTY(BlueprintReadWrite, Category="Save")
    bool bHasWorldState = false;

    // ---- Run scoreboard (#15) ------------------------------------------
    UPROPERTY(BlueprintReadWrite, Category="Scoreboard")
    int32 KillCount = 0;

    UPROPERTY(BlueprintReadWrite, Category="Scoreboard")
    int32 RescueCount = 0;

    UPROPERTY(BlueprintReadWrite, Category="Scoreboard")
    int32 TerminalSolveCount = 0;

    UPROPERTY(BlueprintReadWrite, Category="Scoreboard")
    float RunSeconds = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category="Scoreboard")
    int32 DeathCount = 0;

    UPROPERTY(BlueprintReadWrite, Category="Scoreboard")
    int32 HeadshotCount = 0;

    /** #17 — once true, tutorial overlay is suppressed on subsequent runs. */
    UPROPERTY(BlueprintReadWrite, Category="Tutorial")
    bool bHasShownTutorial = false;

    /** #63 — set to true the first time a survivor is rescued; gates the
     *  companion spawn so subsequent rescues don't spawn duplicate companions.
     *  Reset on Restart-Fresh via ResetRun(). */
    UPROPERTY(BlueprintReadWrite, Category="Companion")
    bool bHasCompanion = false;

    /** #19 — wall-clock seconds of the last successful save. HUD widget reads
     *  this to flash a small "Saving..." pip when a write just completed. */
    UPROPERTY(BlueprintReadWrite, Category="Save")
    float LastSaveWallSeconds = -99.0f;

    /** #44 — global toggle: when false, subtitles widget no-ops the Push call. */
    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    bool bSubtitlesEnabled = true;

    /** #45 — colorblind palette. Read by GameMode when authoring per-zone PPVs. */
    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    EColorblindMode ColorblindMode = EColorblindMode::None;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    float SubtitleScale = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    float UITextScale = 0.90f;   // 2026-07-01: calmer default HUD type; raisable in Settings.

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    bool bHighContrastHUD = false;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    bool bReducedMotion = false;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    bool bSimplifiedInputHints = false;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    bool bVisualizeSoundCues = true;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    bool bMonoAudio = false;

    UPROPERTY(BlueprintReadWrite, Category="Accessibility")
    float AimAssistScale = 1.0f;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Accessibility")
    FString GetAccessibilitySummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Accessibility")
    float GetUITextScale() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Accessibility")
    FString GetUITextScaleSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Accessibility")
    FString GetMonoAudioSummary() const;

    UPROPERTY(BlueprintReadWrite, Category="Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category="Audio")
    float SfxVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category="Audio")
    float MusicVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category="Audio")
    bool bReactiveThreatMusicEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category="Audio")
    float ReactiveThreatMusicIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Audio")
    FString ReactiveThreatMusicState = TEXT("calm");

    UPROPERTY(BlueprintReadWrite, Category="Audio")
    bool bCityAmbientZoneDirectorEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category="Audio")
    FString CityAmbientZoneLabel = TEXT("entry approach");

    UPROPERTY(BlueprintReadOnly, Category="Audio")
    FString CityAmbientZoneBed = TEXT("city_entry_bed");

    UPROPERTY(BlueprintReadOnly, Category="Audio")
    float CityAmbientZoneIntensity = 0.0f;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    FString GetAudioMixSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    float GetSfxVolumeScalar() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    float GetMusicVolumeScalar() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    float GetReactiveThreatMusicScalar() const;

    UFUNCTION(BlueprintCallable, Category="Audio")
    void ApplyAudioMixSettings();

    UFUNCTION(BlueprintCallable, Category="Audio")
    void UpdateReactiveThreatMusic(float ThreatIntensity, const FString& StateLabel);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    FString GetReactiveThreatMusicSummary() const;

    UFUNCTION(BlueprintCallable, Category="Audio")
    void UpdateCityAmbientZone(const FString& ZoneLabel, const FString& BedLabel, float ZoneIntensity);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    FString GetCityAmbientZoneSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Audio")
    FString GetVisualizedSoundCueSummary() const;

    /** Current audited control profile. The active runtime bindings are still
     *  conservative and C++-owned; this names the reviewable profile exported
     *  from Settings for accessibility/control-remap follow-up. */
    UPROPERTY(BlueprintReadWrite, Category="Controls")
    FString ControlProfileName = TEXT("Default");

    UPROPERTY(BlueprintReadWrite, Category="Controls")
    int32 ControlProfileExportCount = 0;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Controls")
    FString GetControlProfileSummary() const;

    UFUNCTION(BlueprintCallable, Category="Controls")
    bool ExportControlProfileReviewFile();

    /** #31 — research points earned by solving terminals without hints.
     *  Spent on hints when stuck. */
    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 ResearchPoints = 3;   // 3 starter hints

    /** #34 — log a code attempt to Saved/CodeAttempts/<ChallengeId>.json. */
    UFUNCTION(BlueprintCallable, Category="Curriculum")
    void LogCodeAttempt(const FString& ChallengeId, const FString& UserCode, bool bSuccess, int32 Score);

    /** Richer attempt log used by the terminal after the 2026-05-24 learning pass. */
    UFUNCTION(BlueprintCallable, Category="Curriculum")
    void LogCodeAttemptDetailed(
        const FString& ChallengeId,
        ECodingLanguage Language,
        const FString& UserCode,
        bool bSuccess,
        int32 Score,
        int32 AttemptNumber,
        int32 HintsUsed,
        const FString& FirstFailedCheck);

    /** Tracks validation attempts, learning streaks, and bonus score. Returns
     *  the bonus score awarded for the attempt. */
    UFUNCTION(BlueprintCallable, Category="Curriculum")
    int32 RecordValidationAttempt(
        ECodingLanguage Language,
        bool bSuccess,
        int32 Score,
        bool bUsedHint,
        bool bFirstTry);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetLearningProgressSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetLearningMasteryTitle() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetLanguageProgressSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetLanguageProfileRecapSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Launch")
    FString GetFirstSessionRoutePreviewSummary(ECodingLanguage Language) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Launch")
    FString GetLaunchLanguageSaveRosterSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Objectives")
    FString GetFailSafeObjectiveBoardSummary() const;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastLearningDebriefChallengeId;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastLearningDebriefConcept;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastLearningDebriefLanguage;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastLearningDebriefSummary;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 LastLearningDebriefScore = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    bool bHasLearningDebriefState = false;

    UFUNCTION(BlueprintCallable, Category="Curriculum")
    void RecordLearningDebrief(
        const FString& ChallengeId,
        const FString& ConceptLabel,
        const FString& LanguageLabel,
        int32 Score,
        const FString& Summary);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetLearningDebriefJournalSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetChallengeReplayJournalSummary() const;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastSurvivorIntelTerminalId;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastSurvivorIntelSurvivorName;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastSurvivorIntelCityLabel;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastSurvivorIntelLanguage;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastSurvivorIntelStatus;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastSurvivorIntelSummary;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 LastSurvivorIntelScore = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    bool bHasSurvivorIntelArchiveState = false;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<FString> RewardChoiceEligibleTerminalIds;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<FString> ClaimedTerminalRewardChoiceIds;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastTerminalRewardChoiceTerminalId;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastTerminalRewardChoiceId;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    FString LastTerminalRewardChoiceSummary;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    bool bHasTerminalRewardChoiceState = false;

    UFUNCTION(BlueprintCallable, Category="Curriculum")
    void MarkTerminalRewardChoiceEligible(const FString& TerminalId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    bool IsTerminalRewardChoiceAvailable(const FString& TerminalId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    bool HasClaimedTerminalRewardChoice(const FString& TerminalId) const;

    UFUNCTION(BlueprintCallable, Category="Curriculum")
    bool ClaimTerminalRewardChoice(const FString& TerminalId, const FString& ChoiceId, class ACodeRescueCharacter* Player);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetTerminalRewardChoiceSummary(const FString& TerminalId) const;

    UFUNCTION(BlueprintCallable, Category="Curriculum")
    void RecordSurvivorIntelDossier(
        const FString& TerminalId,
        const FString& SurvivorName,
        const FString& CityLabel,
        const FString& LanguageLabel,
        const FString& StatusLabel,
        int32 Score,
        const FString& Summary);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    FString GetSurvivorIntelArchiveSummary() const;

    // ---- #43 background music system --------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Music")
    TSoftObjectPtr<class USoundBase> MenuMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Music")
    TSoftObjectPtr<class USoundBase> AmbientCityMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Music")
    TSoftObjectPtr<class USoundBase> BossHordeStinger;

    /** Play a music cue, fading out the previous track if any. Pass nullptr
     *  to fade out only. */
    UFUNCTION(BlueprintCallable, Category="Music")
    void PlayMusic(class USoundBase* NewTrack, float FadeInDuration = 1.5f);

    /** Convenience: PlayMusic on one of the three named slots. */
    UFUNCTION(BlueprintCallable, Category="Music")
    void PlayMenuMusic();
    UFUNCTION(BlueprintCallable, Category="Music")
    void PlayCityMusic();
    UFUNCTION(BlueprintCallable, Category="Music")
    void PlayHordeStinger();

private:
    /** Persistent audio component for music playback. Created lazily. */
    UPROPERTY()
    class UAudioComponent* MusicComponent = nullptr;

    void RefreshReactiveThreatMusicVolume();

public:
    /** #32 — adaptive difficulty. Returns 0..3:
     *    0 = student is new (no concept progress yet)
     *    1 = standard difficulty
     *    2 = advanced (offer harder challenge variants)
     *    3 = mastery (offer expert variants + bonus stars)
     *  Reads cumulative SuccessfulValidations across all concepts. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    int32 GetAdaptiveDifficultyTier() const;

    /** #32 — track per-language proficiency so adaptive tier can specialize. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Curriculum")
    int32 GetLanguageProficiency(ECodingLanguage Language) const;

    /** #50 — increment per-language counters on a successful solve. */
    UFUNCTION(BlueprintCallable, Category="Curriculum")
    void RecordLanguageSolve(ECodingLanguage Language);

    /** #50 — per-language solve counters, persisted in SaveGame. Index by
     *  static_cast<int32>(ECodingLanguage). Six entries by convention. */
    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<int32> LanguageSolveCounts;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 TotalValidationAttempts = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 SuccessfulValidationAttempts = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 FailedValidationAttempts = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 CurrentLearningStreak = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 BestLearningStreak = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 NoHintSolveCount = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    int32 PerfectSolveCount = 0;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<int32> LanguageAttemptCounts;

    UPROPERTY(BlueprintReadWrite, Category="Curriculum")
    TArray<int32> LanguageNoHintSolveCounts;

    /** #54 — crafting recipes. Each call returns true on success and
     *  deducts the cost. */
    UFUNCTION(BlueprintCallable, Category="Crafting")
    bool CraftFlare(class ACodeRescueCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Crafting")
    bool CraftStim(class ACodeRescueCharacter* Player);

    UFUNCTION(BlueprintCallable, Category="Crafting")
    bool CraftGrenade(class ACodeRescueCharacter* Player);

    /** #55 — skill tree. 8 nodes addressed by index 0..7:
     *    0 = +20 Max Stamina
     *    1 = -25% Reload Time
     *    2 = +6 Pistol Mag Size
     *    3 = +1 Max Flares
     *    4 = +1 Max Smokes
     *    5 = +1 Max Stims
     *    6 = +25 Max Health
     *    7 = -1 Scrap cost on Barricades
     *  Each node costs 2 ResearchPoints to unlock. Bitmap on SaveGame. */
    UPROPERTY(BlueprintReadWrite, Category="Skill Tree")
    int64 SkillTreeUnlocked = 0;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Skill Tree")
    bool IsSkillUnlocked(int32 NodeIndex) const;

    UFUNCTION(BlueprintCallable, Category="Skill Tree")
    bool TryUnlockSkill(int32 NodeIndex);

    /** Apply the unlocked skill effects to the player pawn. Call once at
     *  player BeginPlay and again after every TryUnlockSkill. */
    UFUNCTION(BlueprintCallable, Category="Skill Tree")
    void ApplySkillTreeToPlayer(class ACodeRescueCharacter* Player);

    /** Spawn the skill tree widget. Call from pause menu. */
    UFUNCTION(BlueprintCallable, Category="Skill Tree")
    void OpenSkillTreeWidget();

    /** Spawn the crafting widget. Call from pause menu. */
    UFUNCTION(BlueprintCallable, Category="Crafting")
    void OpenCraftingWidget();

    /** #56 — local co-op (split-screen). Spawns a second LocalPlayer when
     *  called; the engine handles the second viewport automatically. */
    UFUNCTION(BlueprintCallable, Category="Co-op")
    bool EnableSecondPlayer();

    UFUNCTION(BlueprintCallable, Category="Co-op")
    bool DisableSecondPlayer();

    /** Increment scoreboard counters and persist immediately. The kill counter
     *  uses the existing MarkZombieNeutralized for world-state purposes; this
     *  is purely for the run summary widget. */
    UFUNCTION(BlueprintCallable, Category="Scoreboard")
    void IncrementKillCount(bool bWasHeadshot = false);

    UFUNCTION(BlueprintCallable, Category="Scoreboard")
    void IncrementRescueCount();

    UFUNCTION(BlueprintCallable, Category="Scoreboard")
    void IncrementTerminalSolveCount();

    UFUNCTION(BlueprintCallable, Category="Scoreboard")
    void IncrementDeathCount();

    UFUNCTION(BlueprintCallable, Category="Scoreboard")
    void AccumulateRunSeconds(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Scoreboard")
    FString GetScoreboardSummary() const;

    UFUNCTION(BlueprintCallable)
    void ResetRun();

    UFUNCTION(BlueprintCallable)
    FString GetLanguageName() const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool SavePersistentRun();

    /** Save the selected-language run after death as a playable recovery
     *  checkpoint. This preserves progress and death count without storing a
     *  zero-health pawn that would immediately re-open the death screen. */
    UFUNCTION(BlueprintCallable, Category="Save")
    bool SaveDeathRecoveryCheckpoint(bool bCountDeath);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadPersistentRun();

    UFUNCTION(BlueprintCallable, Category="Save")
    bool DeletePersistentRun();

    static FString MakeLanguageSaveSlotName(ECodingLanguage Language);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save")
    bool DoesLanguageSaveExist(ECodingLanguage Language) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save")
    FString GetLanguageSaveSummary(ECodingLanguage Language) const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool StartFreshLanguageRun(ECodingLanguage Language);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool ResumeLanguageRun(ECodingLanguage Language);

    UFUNCTION(BlueprintCallable, Category="Save")
    void RecordTerminalSolved(const FString& MissionId, int32 Score);

    UFUNCTION(BlueprintCallable, Category="Save")
    void RecordTerminalAttempt(const FString& MissionId, int32 Score, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category="Save")
    void RecordConceptAttempt(const FString& ConceptId, bool bSuccess, int32 ScoreDelta);

    // ---- World-state helpers --------------------------------------------
    /** Record that a specific terminal is solved (by Challenge.Id). Idempotent. */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void MarkTerminalSolved(const FString& TerminalId);

    /** Record that a specific survivor is rescued (by SurvivorName). Idempotent. */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void MarkSurvivorRescued(const FString& SurvivorName);

    /** Record that a specific zombie is neutralized (by ZombieId). Idempotent. */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void MarkZombieNeutralized(int32 ZombieId);

    /** Boss and named miniboss defeats persist across sessions. Ambient,
     *  director, horde, and dog populations repopulate when a run resumes so
     *  an experienced save can never load into an empty city. */
    static bool IsPersistentStoryZombieId(int32 ZombieId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|Narrative")
    bool HasCollectedCaseFile(const FString& CaseFileId) const;

    UFUNCTION(BlueprintCallable, Category="Save|Narrative")
    bool RecordCaseFileCollected(const FString& CaseFileId, const FString& CaseFileTitle);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|Narrative")
    FString GetCaseFileCollectionSummary() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|NPC")
    bool HasFriendlyNPCServiceCooldown(const FString& ServiceId) const;

    UFUNCTION(BlueprintCallable, Category="Save|NPC")
    bool MarkFriendlyNPCServiceUsed(const FString& ServiceId);

    UFUNCTION(BlueprintCallable, Category="Save|NPC")
    bool ClearFriendlyNPCServiceCooldown(const FString& ServiceId);

    UFUNCTION(BlueprintCallable, Category="Save|NPC")
    void ResetFriendlyNPCServiceCooldowns();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Save|NPC")
    FString GetFriendlyNPCServiceSummary() const;

    /** Record (or update) the variant assigned to a ZombieId at spawn time.
     *  Called from ACodeRescueGameMode::SpawnWorld so reload reproduces the
     *  same mesh/AnimBP for each zombie. Idempotent on (ZombieId). */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void RecordZombieVariant(int32 ZombieId, EZombieVariant Variant);

    /** Look up a previously-recorded variant for a ZombieId. Returns Default
     *  if no record exists (pre-0.5.0 saves, or the slot was never spawned). */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    EZombieVariant GetRecordedZombieVariant(int32 ZombieId) const;

    /** Walk the level and capture the player transform + objective index into
     *  the live GI fields. Called by SavePersistentRun before serialization. */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void CaptureWorldStateFromLevel(UWorld* World);

    /** Apply solved-terminal / rescued-survivor / neutralized-zombie state to
     *  every matching actor in the world, then teleport the player if a
     *  transform was captured. Call from GameMode::BeginPlay after SpawnWorld. */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void ApplyWorldStateToLevel(UWorld* World);

    /** Apply only mission actor state (terminal/survivor/zombie), without
     *  moving the player. Used when the active campaign city is streamed in
     *  after BeginPlay. */
    UFUNCTION(BlueprintCallable, Category="Save|World")
    void ApplyObjectiveStateToLevel(UWorld* World);

private:
    bool SavePersistentRunInternal(bool bCaptureWorldState);
};
