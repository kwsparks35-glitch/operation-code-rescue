#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CodeRescueTypes.generated.h"

class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;
class USoundBase;

UENUM(BlueprintType)
enum class ECodingLanguage : uint8
{
    Java    UMETA(DisplayName = "Java"),
    C       UMETA(DisplayName = "C"),
    Python  UMETA(DisplayName = "Python"),
    MATLAB  UMETA(DisplayName = "MATLAB"),
    CPlus   UMETA(DisplayName = "C+"),
    Cpp     UMETA(DisplayName = "C++")
};

UENUM(BlueprintType)
enum class EGameDifficulty : uint8
{
    Easy      = 0 UMETA(DisplayName = "Easy"),
    Normal    = 1 UMETA(DisplayName = "Normal"),
    Hard      = 2 UMETA(DisplayName = "Hard"),
    Story     = 3 UMETA(DisplayName = "Story"),
    Survival  = 4 UMETA(DisplayName = "Survival"),
    Nightmare = 5 UMETA(DisplayName = "Nightmare")
};


/** Hit zone for headshot damage multiplier and hit feedback. */
UENUM(BlueprintType)
enum class EHitZone : uint8
{
    Head = 0   UMETA(DisplayName = "Head"),
    Torso = 1  UMETA(DisplayName = "Torso"),
    Limb = 2   UMETA(DisplayName = "Limb"),
    Other = 3  UMETA(DisplayName = "Other")
};

/** #45 — colorblind accessibility modes. Each swaps per-zone post-process
 *  saturation matrices to a palette safe for that condition. */
UENUM(BlueprintType)
enum class EColorblindMode : uint8
{
    None         = 0  UMETA(DisplayName = "None"),
    Deuteranope  = 1  UMETA(DisplayName = "Deuteranope (red/green weak)"),
    Protanope    = 2  UMETA(DisplayName = "Protanope (red weak)"),
    Tritanope    = 3  UMETA(DisplayName = "Tritanope (blue weak)")
};

/** #26 - original survival-horror weapon classes available to the player. Pistol = default starter. */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Pistol             = 0  UMETA(DisplayName = "Balanced Handgun"),
    Shotgun            = 1  UMETA(DisplayName = "Pump Shotgun"),
    Rifle              = 2  UMETA(DisplayName = "Assault Rifle"),
    Grenade            = 3  UMETA(DisplayName = "Frag Grenade"),
    CombatKnife        = 4  UMETA(DisplayName = "Combat Knife"),
    HeavyHandgun       = 5  UMETA(DisplayName = "Heavy Handgun"),
    BurstHandgun       = 6  UMETA(DisplayName = "Burst Handgun"),
    TacticalShotgun    = 7  UMETA(DisplayName = "Tactical Shotgun"),
    AutoShotgun        = 8  UMETA(DisplayName = "Auto Shotgun"),
    SMG                = 9  UMETA(DisplayName = "SMG"),
    PrecisionRifle     = 10 UMETA(DisplayName = "Precision Rifle"),
    SemiAutoRifle      = 11 UMETA(DisplayName = "Semi-Auto Rifle"),
    Magnum             = 12 UMETA(DisplayName = "Magnum"),
    BoltLauncher       = 13 UMETA(DisplayName = "Bolt Launcher"),
    RocketLauncher     = 14 UMETA(DisplayName = "Rocket Launcher"),
    IncendiaryGrenade  = 15 UMETA(DisplayName = "Incendiary Grenade"),
    FlashGrenade       = 16 UMETA(DisplayName = "Flash Grenade")
};

/** #26 - per-weapon stats. Defaults are tuned for the current compact city scale. */
USTRUCT(BlueprintType)
struct FWeaponDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString TacticalRole;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MagazineSize = 12;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Damage = 35.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RefireDelay = 0.20f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReloadDuration = 1.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUsesAmmo = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StartingReserveAmmo = 60;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxReserveAmmo = 120;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AmmoCostPerShot = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BurstCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PierceCount = 0;
    /** Range in unreal units. Beyond this the trace doesn't register. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 30000.0f;
    /** Number of bullets fired per click (1 for hitscan, >1 for shotgun). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PelletsPerShot = 1;
    /** Half-angle in degrees of the shot spread. 0 = perfect accuracy. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpreadHalfAngleDeg = 0.0f;
    /** Grenade only: explode delay after fire. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ProjectileFuseSeconds = 0.0f;
    /** Grenade only: AoE radius and falloff for explosion damage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float ExplosionRadius = 0.0f;
};

/** #48 — declarative test case for a challenge. Each case has a label,
 *  an input string, and the expected output. The validator counts how
 *  many cases produced the expected output to drive a "12/15 passed"
 *  readout instead of binary success. */
USTRUCT(BlueprintType)
struct FChallengeTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Label;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Input;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString ExpectedOutput;
};

USTRUCT(BlueprintType)
struct FCodeValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    int32 Score = 0;

    UPROPERTY(BlueprintReadOnly)
    FString Summary;

    UPROPERTY(BlueprintReadOnly)
    FString StdOut;

    UPROPERTY(BlueprintReadOnly)
    FString StdErr;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> PassedChecks;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> FailedChecks;

    /** #48 — total declared test cases for the challenge. 0 means the
     *  validator ran in pure-checks mode (no test cases configured). */
    UPROPERTY(BlueprintReadOnly)
    int32 TotalTestCases = 0;

    /** #48 — number of test cases that produced the expected output. */
    UPROPERTY(BlueprintReadOnly)
    int32 PassedTestCases = 0;
};

USTRUCT(BlueprintType)
struct FChallengeSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MissionBrief;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECodingLanguage Language = ECodingLanguage::Python;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StarterCode;

    /** #48 — declarative test cases. Empty array means binary success only. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FChallengeTestCase> TestCases;
};

/** Identifier for one of the zombie packs landed under Content/. Order is
 *  load-bearing for save serialization — append new entries, never reorder. */
UENUM(BlueprintType)
enum class EZombieVariant : uint8
{
    /** Procedural cube/sphere fallback. Always available, no asset deps. */
    Default            UMETA(DisplayName = "Default (procedural)"),
    /** PxItiger Dog Zombie — quadruped, Animations folder ships. */
    DogZombie          UMETA(DisplayName = "Dog Zombie (PxItiger)"),
    /** Andryuha1981 Urban Zombie 4 — humanoid, comes with Blueprint + anims. */
    UrbanZombie4       UMETA(DisplayName = "Urban Zombie 4 (Andryuha1981)"),
    /** Yarrawah ZombieM04 — Business Suit, "9000+ unique variations, 34 anims". */
    BusinessSuit       UMETA(DisplayName = "Zombie - Business Suit (Yarrawah)"),
    /** Yarrawah ZombieF01 — Bloated Female, "150,000+ variations, 34 anims". */
    BloatedFemale      UMETA(DisplayName = "Zombie - Bloated Female (Yarrawah)"),
    /** RamsterZ Zombie Female: Nurse — free sample, lighter surface area. */
    NurseFemale        UMETA(DisplayName = "Zombie Female: Nurse (RamsterZ)"),
    /** rivai Zombie — base mesh only, NO animations. Use sparingly. */
    BaseMesh           UMETA(DisplayName = "Zombie (rivai, base mesh only)"),

    // ---- #29 elite variants. Append-only — never reorder existing entries. ----
    /** Spitter — ranged-acid zombie. Higher cooldown, projectile attack. */
    EliteSpitter       UMETA(DisplayName = "Elite — Spitter (ranged acid)"),
    /** Charger — sprints in a straight line, knocks back the player. */
    EliteCharger       UMETA(DisplayName = "Elite — Charger (sprint+knockdown)"),
    /** Boomer — explodes on death, AoE damage + spawns 3 small zombies. */
    EliteBoomer        UMETA(DisplayName = "Elite — Boomer (explodes on death)")
};

/** Row driving per-variant visuals + stats. Looked up at spawn time by
 *  ACodeZombieActor::InitializeFromVariant. Soft references avoid pulling
 *  all 5.7 GB of pack content into memory on startup — only the variant
 *  actually spawned is async-loaded. */
USTRUCT(BlueprintType)
struct FZombieVariantRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Which enum entry this row backs. Required so spawn code can pick
     *  rows by EZombieVariant rather than relying on row-name conventions. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EZombieVariant Variant = EZombieVariant::Default;

    /** Human-readable name (used in HUD / debug draw). FString rather than
     *  FText so DataTable JSON import accepts a plain string for this field;
     *  FText forces structured JSON which the bulk add-row API rejects. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    /** Skeletal mesh asset shipped by the pack. Soft so it's only loaded on
     *  demand. Leave unset for Default to keep the procedural cube fallback. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowedClasses="/Script/Engine.SkeletalMesh"))
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    /** AnimBP class shipped by the pack. Without this the mesh stands in T-pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<UAnimInstance> AnimBPClass;

    /** Multipliers applied on top of the base zombie stats set in
     *  ACodeRescueGameMode::SpawnWorld. Lets fast variants be ~1.4x speed,
     *  heavy variants be ~1.6x health, etc., without touching spawn code. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", ClampMax="5.0"))
    float HealthMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", ClampMax="5.0"))
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", ClampMax="5.0"))
    float SpeedMultiplier = 1.0f;

    /** Per-location weight for spawn distribution. Indexes map to whichever
     *  campaign or zone ordering the active spawner uses. A weight of 0 means
     *  this variant will never be picked for that location. Higher values are
     *  more common. Empty map means available everywhere at 1.0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<int32, float> ZoneWeights;

    /** Optional fixed scale override for the skeletal mesh (DogZombie is
     *  much smaller than humanoids; BloatedFemale is taller). Defaults to 1. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1", ClampMax="5.0"))
    float MeshScale = 1.0f;

    /** Plays once on a non-fatal hit. Optional — primitive-cube fallback
     *  zombies never have one set. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> HitReactMontage;

    /** Plays once when Health reaches 0. The actor is destroyed after the
     *  montage's playback length elapses (or 2.0 sec if there's no montage). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> DeathMontage;

    /** Plays once when the zombie commits an attack swing. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> AttackMontage;

    /** Periodic ambient growl. Played at random intervals while alive and
     *  within hearing distance of the player. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> GrowlCue;

    /** Plays at attack-impact moment. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> AttackCue;

    /** Plays once on death. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> DeathCue;
};

/** Per-zombie variant assignment recorded by GameMode at spawn time and
 *  persisted in the SaveGame. Lives in CodeRescueTypes.h (rather than
 *  CodeRescueSaveGame.h) so both UCodeRescueGameInstance and
 *  UCodeRescueSaveGame can reference it without cross-including. */
USTRUCT(BlueprintType)
struct FCodeRescueZombieVariantRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 ZombieId = -1;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    EZombieVariant Variant = EZombieVariant::Default;
};
