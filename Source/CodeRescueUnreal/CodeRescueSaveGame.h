#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CodeRescueTypes.h"
#include "CodeRescueSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FCodeRescueMissionProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString MissionId;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bCompleted = false;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 BestScore = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 Attempts = 0;
};

USTRUCT(BlueprintType)
struct FCodeRescueConceptProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString ConceptId;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 MasteryScore = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 SuccessfulValidations = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 FailedValidations = 0;
};

// FCodeRescueZombieVariantRecord lives in CodeRescueTypes.h so both this
// SaveGame class and UCodeRescueGameInstance can reference it safely under
// UHT (Unreal Header Tool requires the type be reachable without
// cross-including the SaveGame header from the GameInstance).

UCLASS()
class CODERESCUEUNREAL_API UCodeRescueSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString SaveVersion = TEXT("0.8.0-demo-readiness");

    UPROPERTY(BlueprintReadWrite, SaveGame)
    ECodingLanguage LastSelectedLanguage = ECodingLanguage::Java;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    EGameDifficulty Difficulty = EGameDifficulty::Normal;

    /** Playable rescue operator identity tied to the selected language run. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString OperatorCallsign = TEXT("Rhea Calder");

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString OperatorRoleTitle = TEXT("Rescue Operator");

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString OperatorProfileNote = TEXT("frontline rescue route lead");

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasOperatorIdentityState = false;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 SurvivorsRescued = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 ZombiesNeutralized = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 CodingScore = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 TerminalsSolved = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> CompletedMissionIds;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FCodeRescueMissionProgress> MissionProgress;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FCodeRescueConceptProgress> ConceptProgress;

    // ---- World-state persistence (added in 0.4.0) -------------------------
    // Per-actor IDs of objects that have been resolved this run. On level
    // load these are applied by UCodeRescueGameInstance::ApplyWorldStateToLevel
    // so the world reflects past progress instead of starting fresh.

    /** Challenge.Id of every coding terminal already solved. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> SolvedTerminalIds;

    /** SurvivorName of every survivor already rescued. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> RescuedSurvivorNames;

    /** ZombieId of every zombie already neutralized. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<int32> NeutralizedZombieIds;

    /** CaseFileId of every narrative collectible gathered in this language run. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> CollectedCaseFileIds;

    /** Last collected case-file title, used by journal/death/victory summaries. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastCollectedCaseFileTitle;

    /** Friendly safehouse NPC service IDs already used during the current
     *  day/night cycle. Saved per selected-language slot so closing the game
     *  cannot refresh Engineer/Medic/Scientist/Trader support early. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> UsedFriendlyNPCServiceIds;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasFriendlyNPCServiceState = false;

    /** Variant assignment per spawned zombie, captured by GameInstance after
     *  SpawnWorld and replayed on reload so the same i-th zombie keeps the
     *  same mesh/AnimBP/stats. Empty on saves predating 0.5.0. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FCodeRescueZombieVariantRecord> SpawnedZombieVariants;

    /** Player's last objective-jump target, used to seed T-key cycling. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 CurrentObjectiveIndex = 0;

    /** Player transform at last save. Only applied when bHasWorldState is true. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    float PlayerHealth = 100.0f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerAmmo = 150;

    /** 2026-06-30 quick-slot armory pass: preserve selected weapon and
     *  per-slot magazine/reserve state inside each language save. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    EWeaponType ActiveWeapon = EWeaponType::Pistol;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<int32> WeaponMagazines;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<int32> WeaponReserveAmmo;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasWeaponQuickSlotState = false;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerMedkits = 8;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerArmorPlates = 4;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerFlares = 3;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerSmokes = 2;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerStims = 2;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerScrap = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerRadioScannerCharges = 1;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerFlashlightBatteries = 1;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerBypassKits = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerAmmoPouchCapacityBonus = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasPlayerTacticalGear = false;

    /** Magazine ammo persisted across saves (added in improvement pass 2026-05-03). */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PlayerMagazineAmmo = 30;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasPlayerMagazineAmmo = false;

    /** Headshot kill count (added in improvement pass 2026-05-03). */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 HeadshotCount = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasHeadshotCount = false;

    /** Stamina system persistence (added in improvement pass 2026-05-03). */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    float PlayerStamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasPlayerStamina = false;

    /** False on saves predating 0.6.0. Prevents old saves from restoring
     *  default resource fields over the C++ character defaults. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasPlayerResources = false;

    /** False on a brand-new save file. Prevents teleporting the pawn to
     *  (0,0,0) on first launch when no transform has been captured yet. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasWorldState = false;

    // ---- Run scoreboard (added in improvement_pass_2026-05-03 #15) ---------
    /** Total zombies killed across all sessions. Aggregates if loading,
     *  resets if Restart-Fresh. Surfaced on Victory + Death widgets. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 KillCount = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 RescueCount = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 TerminalSolveCount = 0;

    /** Cumulative seconds of gameplay across resumed runs. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    float RunSeconds = 0.0f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 DeathCount = 0;

    /** Backcompat flag: false on saves predating the scoreboard fields above. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasRunScoreboard = false;

    /** #17 — sticky tutorial-shown flag. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasShownTutorial = false;

    /** Companion/support-team deployment state. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasCompanion = false;

    /** #31 — research points (hint currency). */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 ResearchPoints = 3;

    /** #40 — bitmap of unlocked achievements (1 << index). 64 max for now. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int64 AchievementsUnlocked = 0;

    /** #50 — per-language solve counts by ECodingLanguage enum index. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<int32> LanguageSolveCounts;

    /** #55 — bitmap of unlocked skill-tree nodes. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int64 SkillTreeUnlocked = 0;

    /** 2026-05-24 learning pass — total validate button presses. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 TotalValidationAttempts = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 SuccessfulValidationAttempts = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 FailedValidationAttempts = 0;

    /** Current and best consecutive successful terminal validations. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 CurrentLearningStreak = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 BestLearningStreak = 0;

    /** Solves completed without spending a hint on that terminal attempt. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 NoHintSolveCount = 0;

    /** First-try, no-hint, full-score solves. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 PerfectSolveCount = 0;

    /** Per-language validation attempts and no-hint solves by ECodingLanguage index. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<int32> LanguageAttemptCounts;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<int32> LanguageNoHintSolveCounts;

    /** Latest terminal learning debrief saved into this selected-language run. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastLearningDebriefChallengeId;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastLearningDebriefConcept;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastLearningDebriefLanguage;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastLearningDebriefSummary;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 LastLearningDebriefScore = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasLearningDebriefState = false;

    /** Latest survivor intel dossier unlocked in this selected-language run. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastSurvivorIntelTerminalId;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastSurvivorIntelSurvivorName;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastSurvivorIntelCityLabel;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastSurvivorIntelLanguage;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastSurvivorIntelStatus;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastSurvivorIntelSummary;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 LastSurvivorIntelScore = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasSurvivorIntelArchiveState = false;

    /** Terminal IDs with a live-solve reward choice waiting to be claimed. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> RewardChoiceEligibleTerminalIds;

    /** Terminal IDs whose one-time reward choice was already claimed. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    TArray<FString> ClaimedTerminalRewardChoiceIds;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastTerminalRewardChoiceTerminalId;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastTerminalRewardChoiceId;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString LastTerminalRewardChoiceSummary;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHasTerminalRewardChoiceState = false;

    /** #45 — accessibility settings persisted alongside game data. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bSubtitlesEnabled = true;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    EColorblindMode ColorblindMode = EColorblindMode::None;

    /** 2026-06-18 demo-readiness accessibility fields. Defaults preserve
     *  behavior for older saves and are applied only after a player changes
     *  settings. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    float SubtitleScale = 1.0f;

    /** 2026-06-30 UI readability pass — separate menu/HUD text scale so
     *  subtitles can be tuned independently from the shared UI theme. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    float UITextScale = 1.0f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bHighContrastHUD = false;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bReducedMotion = false;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bSimplifiedInputHints = false;

    /** 2026-06-30 audio accessibility pass — when true the HUD mirrors
     *  important threat and ambience sound states as readable visual cues. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bVisualizeSoundCues = true;

    /** 2026-06-30 mono audio accessibility pass — centers project-owned
     *  positional cues and keeps visual sound cues enabled for directional
     *  information. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    bool bMonoAudio = false;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    float AimAssistScale = 1.0f;

    /** 2026-06-30 settings polish — audio mix values persist so the menu,
     *  runtime music, radio, and SFX playback restore the player's mix. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    float SfxVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    float MusicVolume = 1.0f;

    /** 2026-06-24 no-human improvement pass — reviewable control profile
     *  metadata. Runtime gameplay bindings remain conservative, but the
     *  player can export the active control contract from Settings for future
     *  human/accessibility review. */
    UPROPERTY(BlueprintReadWrite, SaveGame)
    FString ControlProfileName = TEXT("Default");

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 ControlProfileExportCount = 0;
};
