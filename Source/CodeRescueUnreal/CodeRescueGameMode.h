#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CodeRunnerLibrary.h"
#include "CodeRescueGameMode.generated.h"

class UStaticMesh;
class UUserWidget;
class UCodeRescueMainMenuWidget;
class AActor;
class APostProcessVolume;
class ADirectionalLight;

UCLASS()
class CODERESCUEUNREAL_API ACodeRescueGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACodeRescueGameMode();

    /** Procedural streaming stand-in for World Partition: keeps only the
     *  active campaign city loaded while the full world-major-city catalog
     *  remains available to the journal, HUD, and progression gates. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Campaign")
    void EnsureCampaignCityLoaded(int32 CityIndex);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Campaign")
    int32 GetActiveCampaignCityIndex() const { return ActiveCampaignCityIndex; }

    /** Authoritative launch-selector instance. The GameMode owns this widget
     *  across packaged Slate rebuilds until a language run is deployed. */
    UCodeRescueMainMenuWidget* GetLaunchLanguageMenu() const { return LaunchLanguageMenu; }

    /** Runtime safety contract for coding spaces: enemy AI, damage, and QA
     *  can query the same tagged protected-learning bounds used by the world
     *  dressing layer. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Safety")
    static bool IsLocationInsideProtectedLearningZone(const UObject* WorldContextObject, FVector Location, float Expansion = 260.0f);

    /** Optional Blueprint override for the victory widget. Defaults to the
     *  C++ class if unset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|UI")
    TSubclassOf<UUserWidget> VictoryWidgetClass;

    /** Optional override for the Survivor actor class spawned by SpawnWorld.
     *  Drop in a Blueprint subclass of ASurvivorActor that wraps a MetaHuman
     *  (or any custom skeletal-mesh setup) and the procedural cube survivor
     *  is replaced wholesale — no code edit required. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Photoreal")
    TSubclassOf<class ASurvivorActor> SurvivorActorClass;

    /** Optional override for the Zombie actor class. Same pattern as
     *  SurvivorActorClass — drop in a Blueprint subclass derived from
     *  ACodeZombieActor that uses your purchased zombie pack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Photoreal")
    TSubclassOf<class ACodeZombieActor> ZombieActorClass;

    /** Optional data table mapping EZombieVariant to FZombieVariantRow.
     *  When set, world spawning can pick per-location variants via weighted
     *  random selection and call AZombieCharacter::InitializeFromVariant.
     *  When unset, every zombie spawns as the procedural-cube fallback
     *  (existing behavior preserved). Hot-loaded from the
     *  /Game/CodeRescueAssets/DT_ZombieVariants table by default. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Photoreal", meta=(RequiredAssetDataTags="RowStructure=/Script/CodeRescueUnreal.ZombieVariantRow"))
    class UDataTable* ZombieVariantTable = nullptr;

    /** Speaks the active city's radio briefing through macOS system TTS.
     *  This gives every generated city an immediate voiced briefing while
     *  authored/imported WAV assets are still optional. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    bool bEnableSystemRadioVoice = true;

    /** The generated Maple/sinister cue set is kept for future review, but the
     *  default runtime narrator is intentionally plain macOS speech because it
     *  is intelligible during play. Enable this only after human audio QA. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    bool bPreferCookedRadioBriefingCues = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Campaign Tuning", meta=(ClampMin="1", ClampMax="80"))
    int32 CityBuildingBaseCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Campaign Tuning", meta=(ClampMin="0", ClampMax="20"))
    int32 CityBuildingPerDifficultyTier = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Campaign Tuning", meta=(ClampMin="0.0", ClampMax="5.0"))
    float CityBuildingHeightTierBonus = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Pickup Tuning", meta=(ClampMin="0", ClampMax="500"))
    int32 AmmoPickupBaseAmount = 35;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Pickup Tuning", meta=(ClampMin="0", ClampMax="100"))
    int32 AmmoPickupCityCycleBonus = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Pickup Tuning", meta=(ClampMin="1", ClampMax="20"))
    int32 AmmoPickupCityCycleLength = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Pickup Tuning", meta=(ClampMin="1", ClampMax="20"))
    int32 MedkitPickupCityInterval = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Pickup Tuning", meta=(ClampMin="1", ClampMax="10"))
    int32 MedkitPickupAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0", ClampMax="20"))
    int32 ZombieBaseCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1", ClampMax="20"))
    int32 ZombieCountTierDivisor = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0", ClampMax="20"))
    int32 ZombieMinCount = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1", ClampMax="400"))
    int32 ZombieMaxCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="50", ClampMax="1000"))
    int32 ZombieToLivingPresenceRatio = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="10", ClampMax="400"))
    int32 MaxActiveAIZombiesPerCity = 120;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1", ClampMax="100"))
    int32 BackgroundHordeClusterSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1.0", ClampMax="500.0"))
    float ZombieBaseHealth = 42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.0", ClampMax="100.0"))
    float ZombieHealthPerDifficultyTier = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.0", ClampMax="100.0"))
    float ZombieHealthCityCycleBonus = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1", ClampMax="30"))
    int32 ZombieHealthCityCycleLength = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1.0", ClampMax="100.0"))
    float ZombieBaseAttackDamage = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.0", ClampMax="50.0"))
    float ZombieAttackDamagePerDifficultyTier = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="20.0", ClampMax="1000.0"))
    float ZombieBaseMoveSpeed = 78.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.0", ClampMax="200.0"))
    float ZombieMoveSpeedPerDifficultyTier = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.0", ClampMax="200.0"))
    float ZombieMoveSpeedCityCycleBonus = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="40.0", ClampMax="1000.0"))
    float ZombieAttackRange = 130.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="1000.0", ClampMax="30000.0"))
    float ZombieBaseActivationRange = 5600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.0", ClampMax="3000.0"))
    float ZombieActivationRangePerDifficultyTier = 550.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.1", ClampMax="5.0"))
    float MinEncounterIntensityScale = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Enemy Tuning", meta=(ClampMin="0.1", ClampMax="5.0"))
    float MaxEncounterIntensityScale = 1.65f;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** #35 — sun-rotation pivot. Created at world origin in BeginPlay. */
    UPROPERTY()
    class ADirectionalLight* SunLight = nullptr;

    UPROPERTY()
    class ASkyLight* WorldSkyLight = nullptr;

    /** Strong ownership for the launch selector. AddToViewport alone does not
     *  keep a programmatically built UUserWidget alive through packaged GC. */
    UPROPERTY(Transient)
    UCodeRescueMainMenuWidget* LaunchLanguageMenu = nullptr;

    /** 2026-07-04: night-sky layer — star dome + moon follow the player, visible only at night. */
    UPROPERTY()
    class AActor* NightSkyDome = nullptr;
    UPROPERTY()
    class AActor* NightSkyMoon = nullptr;

    /** #37 — spawn one secret terminal per city at a deliberately
     *  off-the-beaten-path location. Bonus 5x score reward. */
    void SpawnSecretTerminalForCity(int32 CityIndex, const FVector& Origin);

    /** #36 — spawn the per-zone weather Niagara emitter. */
    void SpawnWeatherForCity(int32 CityIndex, const FVector& Origin);
    void SpawnWeatherLightingIdentityLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** Periodic check: if all terminals solved AND all survivors rescued,
     *  spawn the victory widget once. */
    UFUNCTION()
    void CheckVictoryCondition();

    void SpawnWorld();
    void SpawnLaunchLanguageSelectionScene();
    AActor* SpawnBlock(const FVector& Location, const FVector& Scale, const FLinearColor& Color, const FString& Name, bool bEnableCollision = true);
    AActor* SpawnRotatedBlock(const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color, const FString& Name, bool bEnableCollision = true);
    // 2026-07-01: spawn one of the authored /Game/CodeRescueArt kit meshes (real art, not a primitive).
    AActor* SpawnKitMesh(const FString& MeshObjectPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FString& Name, bool bEnableCollision = true, const TCHAR* MaterialPath = nullptr);
    /** Compact Blender-authored, non-colliding ground ring that follows its
     * zombie. Replaces the former human-height marker cubes. */
    AActor* SpawnZombieReadabilityMarker(class ACodeZombieActor* Zombie, const FLinearColor& AccentColor, const FString& Name, float Scale = 1.0f);
    void SpawnAuthoredCityKitLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    // 2026-07-04: authored streetscape (roads/sidewalks/crosswalk/vehicles/trees/signals) + night sky.
    void SpawnStreetscapeLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    // 2026-07-06: v3 completed street walls — closed buildings w/ lit windows,
    // streetlights, hydrants, bus stop, power poles, curb-parked vehicles.
    void SpawnCityBlockV3Layer(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** Blender-authored structures and combat-readability props used only by
     * the first level; kept out of every later campaign city. */
    void SpawnFirstLevelCombatArtPass(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** Open-door Blender buildings, functional interiors, and flat traversal
     * anchors for the first-level production profile. */
    void SpawnFirstLevelTraversalArtPass(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** Three Blender-authored, symbol-led first-level destinations that turn
     * formerly ambiguous open regions into useful gameplay districts. */
    void SpawnFirstLevelPurposeDistrictPass(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** 2026-07-06 mood pass: exponential height fog + filmic post volume. */
    void SpawnCityMoodLayer();
    /** 2026-07-07: auto-run the solidity fix (grounds floating collision
     *  meshes) after every city spawn — no more elevated regions. */
    void GroundFloatingMeshes(int32 CityIndex);
    /** 2026-07-11 floating-character fix: after ALL world geometry is final
     *  (including the slab-lowering GroundFloatingMeshes pass), re-ground
     *  every character in the city. AActor-based characters (survivors, NPCs,
     *  decorative civilians) have no gravity, so a missed BeginPlay snap
     *  otherwise floats forever. */
    void GroundSpawnedCharacters(int32 CityIndex, const FVector& Origin, const FString& CityLabel);

public:
    /** 2026-07-11: shared robust ground snap for character-like actors.
     *  Ignores fall-recovery catch floors and other characters, traces
     *  ECC_Visibility then ECC_WorldStatic, and rejects hits below
     *  MinGroundZ. On a miss: optionally snaps the actor's base to
     *  FallbackGroundZ (city plane), otherwise leaves the actor untouched.
     *  Returns true when the actor was moved. */
    static bool SnapCharacterBaseToGround(AActor* Actor, float MinGroundZ = -1.0e9f, bool bSnapToFallbackOnMiss = false, float FallbackGroundZ = 0.0f);
    /** Aligns the visible skeletal feet with the bottom of a grounded capsule.
     * Mesh bounds can change after a runtime mesh/animation swap even when the
     * actor capsule itself is already at the correct elevation. */
    static bool AlignCharacterVisualFeetToCapsule(class ACharacter* Character, float ContactLift = 1.5f);
    /** Distance from the actor's location down to its visual base (capsule
     *  half-height for ACharacter, lowest visible mesh-bounds bottom otherwise). */
    static float ComputeCharacterBottomOffset(const AActor* Actor);

protected:
    UPROPERTY() AActor* CityMoodFog = nullptr;
    UPROPERTY() AActor* CityMoodPost = nullptr;
    UPROPERTY() class ACodeRescueWeatherFieldActor* ActiveWeatherField = nullptr;
    /** 2026-07-07: night key light — the sun points up at night, so a fixed
     *  cool-blue moon directional keeps the world readable after dark. */
    UPROPERTY() ADirectionalLight* MoonLight = nullptr;
    void SpawnNightSkyLayer(const FVector& Origin);
    void UpdateNightSkyVisibility();
    bool RunFirstLevelWorldAccessAudit();
    /** Verifies that the canonical mission floor physically supports all four
     *  arena edges, including the full east/right perimeter reported by the
     *  player. The fall-recovery floor beneath the city is never accepted. */
    bool AuditCampaignPerimeterGround(int32 CityIndex);
    bool RunFirstLevelChallengeAudit();

    /** 2026-07-11 pass 4 (Kenny's multi-level ground screenshots): snap every
     *  first-level ground slab's WALKABLE TOP to the street datum, tone the
     *  blinding orientation-plaza material, and report intentional elevation
     *  changes. Idempotent; runs post-assembly and again before the audit. */
    void UnifyFirstLevelGroundTops();

    /** 2026-07-11 pass 4: one ambient wind manager per world (foliage sway). */
    bool bWindSwayManagerSpawned = false;
    void StartFirstLevelSkyAudit();
    void StartWorldLootWeatherVisualReview();
    bool AuditCampaignCityPopulation(int32 CityIndex);
    void StartCampaignGroundRecoveryAudit();
    void StartTerminalContrastReview();
    /** Line-trace ground snap: returns the ground Z under Probe, or DefaultZ when nothing is hit. */
    float GroundZAt(const FVector& Probe, float DefaultZ) const;
    AActor* SpawnTexturedBlock(const FVector& Location, const FVector& Scale, const FLinearColor& FallbackColor, const FString& Name, const TCHAR* MaterialPath, bool bEnableCollision = true);
    AActor* SpawnStaticMeshProp(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& BlockScale, const FString& Name, bool bEnableCollision = true, const TCHAR* MaterialOverride = nullptr);
    AActor* SpawnDecorativeCivilian(const FVector& Location, const FRotator& Rotation, bool bUseQuinn, const FLinearColor& BadgeColor, const FString& Name, const FString& DisplayLabel = FString());
    void SpawnZone(const FString& ZoneName, const FVector& Origin, const FLinearColor& AccentColor);
    void SpawnCampaignCity(const struct FCodeRescueCityMission& Mission, int32 CityIndex);
    void SpawnCityLandmark(const struct FCodeRescueCityMission& Mission, const FVector& Origin, const FString& CityLabel);
    void SpawnCityArtKit(const struct FCodeRescueCityMission& Mission, const FVector& Origin, const FString& CityLabel);
    void SpawnMajorCityUrbanIdentityLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnUSCitySpecificIdentityLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnRegionalCityKitIdentityLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    // ---- improvement_pass_2026-06-12 #45 — whole-city U.S. realization -----
    /** Applies the per-city sky: sun tint/intensity members consumed by the
     *  day/night Tick, a streamed exponential height fog, and a cloud deck
     *  matched to the city's weather family (marine layer, overcast, desert
     *  clear, humid gulf glow, ...). */
    void ApplyUSCitySkyRealization(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** City-wide terrain: ground tint plates, coastal/lake/river water with
     *  shoreline and bridge decks, perimeter backdrop (mountain ring, mesas,
     *  evergreen ridge, palm shore, prairie horizon, hill terraces), and
     *  street vegetation matched to the city's region. */
    void SpawnUSCityLandscapeRealizationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** Residential districts built from the city's home archetype (brownstone
     *  rows, triple-deckers, painted Victorians, craftsman bungalows, adobe
     *  ranches, brick two-flats, shotgun porches, sunbelt ranches, deco
     *  pastels, mountain cabins, capes). */
    void SpawnUSCityResidentialDistrictLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    /** Curbside vehicle population along the authored street grid using the
     *  city's fleet mix (taxis, pickups, SUVs, sedans, compacts/EVs, vans,
     *  buses, convertibles, plow trucks). */
    void SpawnUSCityVehiclePopulationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnWorldMajorCitySignatureLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnCityLandscapeDetails(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnCinematicStreetLifeLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnMissionObjectiveRoute(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnWorldCompositionLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnCharacterIdentityCourt(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnEnterableCivicSafehouse(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnInteriorMissionSpacesForCity(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnPhysicsTraversalYard(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnMissionDioramas(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnAccountLinkedAssetShowcase(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnImmediateGameImprovementLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnCodingLearningGamificationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnGraduatedCurriculumCityIdentityLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnCharacterWorldRealizationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnFirstMinuteOrientationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnNext100DevelopmentLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnBespokeSurvivalHorrorArtLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnBespokeAuthoredAssetRefinementLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnProductionTrackCompletionLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnFirstViewAestheticArrivalLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnGameplayArenaConfinementLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnUniversalEntryAccessLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnTacticalArmoryLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnUnrealSystemsCharacterWorldLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnPublicDemoFabDetailLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnProtectedCodingChallengeHub(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnChallengeRoomConceptArtLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnEncounterDirectorLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, class ASurvivorActor* Survivor);
    void SpawnCreativeRecommendationImplementationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnRuntimeDataLayerMigrationLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnPurposeClarityLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void EnsureEntryAccessCorridorClear(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    bool ShouldSpawnDevelopmentShowcaseLayers() const;
    void ApplyProductionPresentationCleanup(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnRescueSupportTeamForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnSurvivorReliefCamp(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, class ASurvivorActor* Survivor);
    void SpawnSecondaryMotionSignalLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, class ASurvivorActor* Survivor);
    void SpawnTerminal(const FVector& Location, const FString& Id, const FString& Title, const FString& Brief, int32 CityIndex = 0);
    void SpawnLanguageStation(const FVector& Location, ECodingLanguage Language, const FString& Label);
    AActor* SpawnGuideText(const FString& Text, const FVector& Location, const FColor& Color, float Size = 80.0f);
    void ApplyRuntimeDataLayerTags(AActor* Actor, const TArray<FName>& LayerTags) const;
    void RegisterStreamedActor(AActor* Actor);
    void ClearStreamedCampaignActors();
    int32 EstimateLivingPresenceCountForCity(const struct FCodeRescueCityMission& Mission, int32 CityIndex);
    int32 ComputeTargetZombiePresence(const struct FCodeRescueCityMission& Mission, int32 CityIndex);
    void SpawnBackgroundHordePopulation(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, int32 LivingPresenceCount, int32 ActiveZombieCount, int32 TargetZombiePresence);
    bool TryGetRecordedZombieVariant(int32 ZombieId, EZombieVariant& OutVariant) const;
    const struct FZombieVariantRow* FindZombieVariantRow(EZombieVariant Variant) const;
    const struct FZombieVariantRow* SelectZombieVariantRow(int32 CityIndex, int32 ZombieSlot, int32 ZombieId, EZombieVariant& OutVariant) const;
    void ApplyZombieFamilyVariant(class ACodeZombieActor* Zombie, EZombieVariant Variant, int32 ZombieId, FName ContextTag, bool bPersistAssignment) const;
    void ApplyCityZombieFamilyVariant(class ACodeZombieActor* Zombie, int32 CityIndex, int32 ZombieSlot, int32 ZombieId, FName ContextTag, bool bPersistAssignment) const;
    void SpeakRadioBriefing(const struct FCodeRescueCityMission& Mission);
    void StopActiveRadioBriefing();

    // ---- Improvement pass 2026-05-03 helpers ------------------------------
    /** #7 — Helicopter fast-travel pad spawned per city. */
    void SpawnHelipadForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** Expanded, city-variant extraction staging around the helipad. */
    void SpawnExpandedExtractionSetPieceForCity(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** #62 — Drivable jeep spawned next to each helipad. */
    void SpawnJeepForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** #62 — Per-city boss zombie. Health/damage scale with the city's
     *  difficulty tier; spawned in the deep northeast quadrant so the player
     *  has to fight inward to find it. */
    void SpawnBossForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel, const struct FCodeRescueCityMission& Mission);
    void SpawnEliteWardenMiniBossStagingLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** #67 — Novel world set-pieces. Each city gets a procedurally-decided
     *  themed event (lab vault / radio tower / dog ambush / hospital triage /
     *  drone wreckage) that adds optional gameplay flavor. */
    void SpawnSetPieceForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel, const struct FCodeRescueCityMission& Mission);

    /** #68 — Ambient friendly NPCs (Engineer / Medic / Scientist / Trader)
     *  scattered around the city's safe quadrant. Adds non-combat depth +
     *  resource trickle without changing the rescue/coding loop. */
    void SpawnFriendlyNPCsForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** Collectible case files that connect city story, survivor stakes, and
     *  coding concepts to the selected-language save run. */
    void SpawnCollectibleCaseFilesForCity(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnEnvironmentalStorytellingLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);
    void SpawnWorldBibleLoreLayer(const struct FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel);

    /** #8 — Authored prop kit-bash for a hero city. Prefers inspectable
     *  imported/static-mesh props and keeps block fallbacks only if a local
     *  asset is unavailable. */
    void SpawnAuthoredPropsForCity(int32 CityIndex, const FVector& Origin, const FLinearColor& Accent);

    /** #9 — Per-zone color-grading volume. */
    void SpawnPerZonePostProcessVolume(int32 CityIndex, const FVector& Origin, const FLinearColor& Accent);
    void ConfigurePerZonePostProcessVolume(APostProcessVolume* PPV, int32 CityIndex, const FString& GradeToken, EColorblindMode ColorblindMode) const;

    /** #10 — Per-zone ambient bed (USoundBase soft ref). */
    void SpawnAmbientSoundForCity(int32 CityIndex, const FVector& Origin);

public:
    /** #14 — Spawns a 30-second horde rush around the just-solved terminal.
     *  Called from UCodeTerminalWidget after RecordTerminalSolved. Triggers
     *  ZombieMaxCount+4 zombies in a ring, with extra Hard-variant weighting. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Encounters")
    void TriggerBossHorde(const FVector& Center, int32 CityIndex);

    /** World-development feedback when a coding terminal is solved: reveal
     *  the survivor route, pulse rescue beacons, and drop a small route cache.
     *  Also called during save restore so solved terminals reconstruct their
     *  world-side cause/effect without re-opening the terminal UI. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|World")
    void RevealSolvedTerminalRescueRoute(const FString& TerminalId, int32 CityIndex, const FVector& TerminalLocation, bool bFromLoad = false);

    /** First-time coding passes and real zombie defeats place collectible,
     *  ground-snapped supplies into the active encounter. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Rewards")
    int32 SpawnChallengeCompletionSupplyCache(const FString& ChallengeId, int32 CityIndex, const FVector& TerminalLocation);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Rewards")
    int32 SpawnZombieDeathSupply(int32 ZombieId, const FVector& DeathLocation);

    UFUNCTION(BlueprintCallable, Category="Code Rescue|Audio")
    int32 RefreshMonoAudioSpatialization(bool bMonoAudioEnabled);

    /** Re-applies the saved color-vision correction to active streamed city
     *  post-process volumes so Settings changes are visible immediately. */
    UFUNCTION(BlueprintCallable, Category="Code Rescue|Accessibility")
    int32 RefreshActiveColorVisionPostProcess(EColorblindMode NewMode);

    /** Per-city radio briefing soft refs (#12). Index parallels campaign
     *  missions. Runtime also falls back to slug-based SoundWave loads for
     *  imported Maple cues when this array is not populated. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    TArray<TSoftObjectPtr<class USoundBase>> CityRadioBriefingCues;

    /** #10 — Per-zone ambient sound cues. Same null-until-import pattern. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Audio")
    TArray<TSoftObjectPtr<class USoundBase>> ZoneAmbientCues;

    /** #33 — when true, SpawnWorld skips zombie spawns and only places the
     *  launch-locked language marker + a single hub terminal. ASandboxGameMode flips this
     *  in its constructor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Sandbox")
    bool bSandboxMode = false;

    /** #36 — soft refs to per-zone weather Niagara emitters. Index parallels
     *  PostProcess/Ambient zone cycle (0=Anchorage/snow, 1=Seattle/rain, 2=Tokyo/fog). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|Weather")
    TArray<TSoftObjectPtr<class UNiagaraSystem>> ZoneWeatherSystems;

    /** #35 — 30-minute full solar cycle. Day and night each remain readable
     *  for roughly fifteen minutes instead of flashing by during one mission. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Code Rescue|TimeOfDay", meta=(ClampMin="60.0", ClampMax="3600.0"))
    float DayNightPeriodSeconds = 1800.0f;

    /** #35 — current normalized solar cycle in [0,1): 0=noon,
     * .25=sunset, .5=midnight, .75=sunrise. Starts at .12 in late morning. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|TimeOfDay")
    float TimeOfDay = 0.12f;

    /** #35 — when true, zombie spawn density gets the night multiplier. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Code Rescue|TimeOfDay")
    bool bIsNight = false;

    int32 LastLoggedSkyPhase = INDEX_NONE;

    TArray<TWeakObjectPtr<AActor>> FirstLevelGroundSurfaceActors;
    TArray<FVector> FirstLevelAccessDoorwayOutsidePoints;
    TArray<FVector> FirstLevelAccessDoorwayInsidePoints;

protected:

    UPROPERTY()
    UStaticMesh* CubeMesh = nullptr;

    // ---- improvement_pass_2026-06-12 #45 — active-city realization state ---
    /** Sun key-light tints/intensities for the streamed city. Defaults match
     *  the long-standing readable day/night values; ApplyUSCitySkyRealization
     *  overrides them per city and Tick consumes them every frame. */
    FLinearColor CityDaySunColor = FLinearColor(1.0f, 0.96f, 0.86f);
    FLinearColor CityNightSunColor = FLinearColor(0.55f, 0.64f, 0.95f);
    float CityDaySunIntensity = 7.0f;
    float CityNightSunIntensity = 3.2f;
    /** Wardrobe palette + accessory token for the streamed city. When the
     *  palette is non-empty, SpawnDecorativeCivilian dresses every civilian
     *  in city-appropriate clothing colors and headwear/carry accessories. */
    TArray<FLinearColor> ActiveCityWardrobePalette;
    FString ActiveCityWardrobeAccessory;
    /** Color-grade family token consumed by SpawnPerZonePostProcessVolume so
     *  U.S. cities grade by climate (cool overcast, warm desert, crisp
     *  mountain, humid gulf, tropical bright, golden basin, neutral metro). */
    FString ActiveCityRealizationGradeToken;

private:
    /** Set once the victory widget has been shown so we don't spawn it
     *  every tick of the timer after the win condition latches. */
    bool bVictoryShown = false;

    FTimerHandle VictoryCheckTimer;
    TArray<TWeakObjectPtr<AActor>> StreamedCampaignActors;
    int32 ActiveCampaignCityIndex = INDEX_NONE;
    bool bCollectStreamedCampaignActors = false;
    int32 LastSpokenRadioCityIndex = INDEX_NONE;
    int32 PendingRadioCityIndex = INDEX_NONE;
    FTimerHandle RadioBriefingDelayTimer;
    UPROPERTY(Transient) class UAudioComponent* ActiveRadioBriefingComponent = nullptr;
    FProcHandle ActiveSystemRadioProcess;
};
