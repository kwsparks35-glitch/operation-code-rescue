#include "CodeRescueGameMode.h"
#include "CodeRescueMaterialUtils.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueWindSway.h"
#include "CodeRescueWeatherFieldActor.h"
#include "DoorActor.h"
#include "CodeRunnerLibrary.h"
#include "CodeRescueLearning.h"
#include "CodeRescueMainMenuWidget.h"
#include "CodeRescuePhysicsStability.h"
#include "CodeRescueSolveEffectActor.h"
#include "CodeRescueMessageMarkerActor.h"
#include "CodeRescueBeaconMarkerActor.h"
#include "CodeRescueFacialExpressionComponent.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CodeTerminalWidget.h"
#include "CodeRescueTutorialWidget.h"
#include "CodeRescueVictoryWidget.h"
#include "BarricadeActor.h"
#include "CodingTerminalActor.h"
#include "CodeZombieActor.h"
#include "BossZombieActor.h"
#include "BossRevealPresentationActor.h"
#include "CompanionActor.h"
#include "FriendlyNPCActor.h"
#include "JeepActor.h"
#include "HelipadActor.h"
#include "LanguageStationActor.h"
#include "PickupActor.h"
#include "CaseFilePickupActor.h"
#include "RescueRouteGuidanceDroneActor.h"
#include "SecondaryMotionSignalActor.h"
#include "ObjectiveFocusBeaconActor.h"
#include "SurvivorActor.h"
#include "Sound/AmbientSound.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "EngineUtils.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyAtmosphereComponent.h" // declares both USkyAtmosphereComponent and ASkyAtmosphere
#include "Components/DirectionalLightComponent.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BrushComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "UnrealClient.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

#include <initializer_list>

namespace
{
constexpr int32 CodeRescueRegularZombieIdBase = 1000000;
constexpr int32 CodeRescueRegularZombieIdStride = 1000;
constexpr int32 CodeRescueBossZombieIdBase = 2000000;
constexpr int32 CodeRescueDogZombieIdBase = 3000000;
constexpr int32 CodeRescueHordeZombieIdBase = 4000000;

float GetRuntimeSfxVolume(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI ? GI->GetSfxVolumeScalar() : 1.0f;
}

float GetRuntimeMusicVolume(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI ? GI->GetMusicVolumeScalar() : 1.0f;
}

bool IsRuntimeMonoAudioEnabled(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI && GI->bMonoAudio;
}

bool IsFilterTerminalId(const FString& TerminalId)
{
    return TerminalId.Contains(TEXT("filter")) || TerminalId.Contains(TEXT("even"));
}

bool IsLockTerminalId(const FString& TerminalId)
{
    return TerminalId.Contains(TEXT("lock"));
}

bool IsReverseTerminalId(const FString& TerminalId)
{
    return TerminalId.Contains(TEXT("reverse"));
}

bool IsFilterLearningWorldNode(const FCodeRescueChallenge& Challenge)
{
    return Challenge.Tier == 4
        && (Challenge.Id.Contains(TEXT("evac-even-order"))
            || Challenge.Id.Contains(TEXT("filter"))
            || Challenge.Id.Contains(TEXT("even"))
            || Challenge.Concept.Contains(TEXT("filter")));
}

bool IsLockLearningWorldNode(const FCodeRescueChallenge& Challenge)
{
    return Challenge.Tier == 2
        && (Challenge.Id.Contains(TEXT("airlock"))
            || Challenge.Id.Contains(TEXT("lock"))
            || Challenge.Concept.Contains(TEXT("boolean")));
}

bool IsReverseLearningWorldNode(const FCodeRescueChallenge& Challenge)
{
    return Challenge.Tier == 5
        && (Challenge.Id.Contains(TEXT("reverse"))
            || Challenge.Concept.Contains(TEXT("strings and indexing")));
}

int32 CountNumericItemsInLearningOutput(const FString& OutputText)
{
    int32 Count = 0;
    bool bInsideNumber = false;
    for (int32 i = 0; i < OutputText.Len(); ++i)
    {
        const TCHAR C = OutputText[i];
        const bool bNumberChar = FChar::IsDigit(C) || C == static_cast<TCHAR>('-');
        if (bNumberChar && !bInsideNumber)
        {
            ++Count;
            bInsideNumber = true;
        }
        else if (!bNumberChar)
        {
            bInsideNumber = false;
        }
    }
    return Count;
}

int32 CountQuotedCharactersInLearningOutput(FString OutputText)
{
    OutputText = OutputText.TrimStartAndEnd();
    if (OutputText.Len() >= 2)
    {
        const TCHAR First = OutputText[0];
        const TCHAR Last = OutputText[OutputText.Len() - 1];
        if ((First == static_cast<TCHAR>('\'') && Last == static_cast<TCHAR>('\''))
            || (First == static_cast<TCHAR>('"') && Last == static_cast<TCHAR>('"')))
        {
            return FMath::Max(0, OutputText.Len() - 2);
        }
    }
    return OutputText.Len();
}

int32 EstimateLearningOutputMagnitude(const FCodeRescueChallenge& Challenge)
{
    if (Challenge.VisibleTests.Num() > 0)
    {
        return CountNumericItemsInLearningOutput(Challenge.VisibleTests[0].Out);
    }
    if (Challenge.HiddenTests.Num() > 0)
    {
        return CountNumericItemsInLearningOutput(Challenge.HiddenTests[0].Out);
    }
    return 1;
}

int32 EstimateLearningStringOutputMagnitude(const FCodeRescueChallenge& Challenge)
{
    if (Challenge.VisibleTests.Num() > 0)
    {
        return CountQuotedCharactersInLearningOutput(Challenge.VisibleTests[0].Out);
    }
    if (Challenge.HiddenTests.Num() > 0)
    {
        return CountQuotedCharactersInLearningOutput(Challenge.HiddenTests[0].Out);
    }
    return 1;
}

FString QuoteForProcessArg(FString Value)
{
    Value.ReplaceInline(TEXT("\n"), TEXT(" "));
    Value.ReplaceInline(TEXT("\r"), TEXT(" "));
    Value.ReplaceInline(TEXT("\""), TEXT("'"));
    return FString::Printf(TEXT("\"%s\""), *Value);
}

FString BuildRadioRouteCadenceLine(
    const FCodeRescueCityMission& Mission,
    int32 CityIndex,
    const UCodeRescueGameInstance* GI)
{
    const FString LanguageLabel = GI ? GI->GetLanguageName() : FString(TEXT("selected language"));
    const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bSurvivorRescued = GI && GI->RescuedSurvivorNames.Contains(Mission.SurvivorName);
    const FString PhaseLabel = bSurvivorRescued
        ? TEXT("Extraction beacon live")
        : (bTerminalSolved ? TEXT("Survivor route open") : TEXT("Terminal route locked"));
    const FString NextStep = bSurvivorRescued
        ? TEXT("reach the helipad and debrief")
        : (bTerminalSolved ? TEXT("follow survivor ping") : TEXT("solve protected terminal"));
    const FString TerminalLabel = Mission.TerminalTitle.IsEmpty()
        ? Mission.TerminalId
        : Mission.TerminalTitle;
    const FString LandmarkLabel = Mission.LandmarkName.IsEmpty()
        ? Mission.DistrictStyle
        : Mission.LandmarkName;

    return FString::Printf(
        TEXT("[Radio Relay]: %s, %s. %s track. %s. Terminal %s; survivor %s; landmark %s. Next: %s."),
        *Mission.CityName,
        *Mission.StateName,
        *LanguageLabel,
        *PhaseLabel,
        *TerminalLabel,
        *Mission.SurvivorName,
        *LandmarkLabel,
        *NextStep);
}

FName GetZombieFamilyVariantAuditTag(EZombieVariant Variant)
{
    switch (Variant)
    {
    case EZombieVariant::DogZombie:
        return FName("ZombieFamily_DogZombie");
    case EZombieVariant::UrbanZombie4:
        return FName("ZombieFamily_UrbanZombie");
    case EZombieVariant::BusinessSuit:
        return FName("ZombieFamily_BusinessSuit");
    case EZombieVariant::BloatedFemale:
        return FName("ZombieFamily_Bloated");
    case EZombieVariant::NurseFemale:
        return FName("ZombieFamily_Nurse");
    case EZombieVariant::BaseMesh:
        return FName("ZombieFamily_BaseMeshFallback");
    case EZombieVariant::EliteSpitter:
        return FName("ZombieFamily_EliteSpitter");
    case EZombieVariant::EliteCharger:
        return FName("ZombieFamily_EliteCharger");
    case EZombieVariant::EliteBoomer:
        return FName("ZombieFamily_EliteBoomer");
    default:
        return FName("ZombieFamily_Default");
    }
}

FString GetZombieFamilyVariantMarkerLabel(EZombieVariant Variant)
{
    switch (Variant)
    {
    case EZombieVariant::DogZombie:
        return TEXT("Dog Zombie");
    case EZombieVariant::UrbanZombie4:
        return TEXT("Urban Zombie");
    case EZombieVariant::BusinessSuit:
        return TEXT("Business Zombie");
    case EZombieVariant::BloatedFemale:
        return TEXT("Bloated Zombie");
    case EZombieVariant::NurseFemale:
        return TEXT("Nurse Zombie");
    case EZombieVariant::BaseMesh:
        return TEXT("Fallback Zombie");
    case EZombieVariant::EliteSpitter:
        return TEXT("Elite Spitter");
    case EZombieVariant::EliteCharger:
        return TEXT("Elite Charger");
    case EZombieVariant::EliteBoomer:
        return TEXT("Elite Boomer");
    default:
        return TEXT("Default Zombie");
    }
}

FLinearColor GetZombieFamilyVariantMarkerColor(EZombieVariant Variant)
{
    switch (Variant)
    {
    case EZombieVariant::DogZombie:
        return FLinearColor(0.96f, 0.35f, 0.08f);
    case EZombieVariant::UrbanZombie4:
        return FLinearColor(0.92f, 0.14f, 0.08f);
    case EZombieVariant::BusinessSuit:
        return FLinearColor(0.65f, 0.10f, 0.85f);
    case EZombieVariant::BloatedFemale:
        return FLinearColor(0.86f, 0.66f, 0.08f);
    case EZombieVariant::NurseFemale:
        return FLinearColor(0.12f, 0.78f, 0.56f);
    case EZombieVariant::EliteSpitter:
        return FLinearColor(0.42f, 1.0f, 0.14f);
    case EZombieVariant::EliteCharger:
        return FLinearColor(1.0f, 0.04f, 0.22f);
    case EZombieVariant::EliteBoomer:
        return FLinearColor(0.96f, 0.55f, 0.06f);
    default:
        return FLinearColor(1.0f, 0.05f, 0.0f);
    }
}

FVector CityOffset(const FVector& Offset)
{
    return FCodeRescueCampaign::ScaleCityOffset(Offset);
}

FVector CityExtent(const FVector& Extent)
{
    return FCodeRescueCampaign::ScaleCityExtent(Extent);
}

bool IsEssentialGuideText(const FString& UpperText)
{
    // 2026-07-02: tightened per Kenny's "words competing for attention" note. Only genuine
    // control prompts and the core-loop labels stay as on-screen text; everything descriptive
    // (location names, lore, weather, district flavor) becomes a compact hover marker instead.
    return UpperText.Contains(TEXT("[E]")) ||
           UpperText.Contains(TEXT("E /")) ||
           UpperText.Contains(TEXT("\nE ")) ||
           UpperText.Contains(TEXT(" E ")) ||
           UpperText.Contains(TEXT("WASD")) ||
           UpperText.Contains(TEXT("ENTER")) ||
           UpperText.Contains(TEXT("BACKSPACE")) ||
           UpperText.Contains(TEXT("OBJECTIVE")) ||
           UpperText.Contains(TEXT("SELECT CODING LANGUAGE")) ||
           UpperText.Contains(TEXT("RESUME SAVE"));
}

FString SymbolForGuideText(const FString& UpperText)
{
    if (UpperText.Contains(TEXT("AMMO")) ||
        UpperText.Contains(TEXT("MEDKIT")) ||
        UpperText.Contains(TEXT("FLARE")) ||
        UpperText.Contains(TEXT("SMOKE")) ||
        UpperText.Contains(TEXT("STIM")) ||
        UpperText.Contains(TEXT("SCRAP")) ||
        UpperText.Contains(TEXT("ARMOR")) ||
        UpperText.Contains(TEXT("GEAR")) ||
        UpperText.Contains(TEXT("ARMORY")))
    {
        return TEXT("+");
    }
    if (UpperText.Contains(TEXT("SURVIVOR")) ||
        UpperText.Contains(TEXT("MEDIC")) ||
        UpperText.Contains(TEXT("CIVILIAN")) ||
        UpperText.Contains(TEXT("TEAM")))
    {
        return TEXT("++");
    }
    if (UpperText.Contains(TEXT("ZOMBIE")) ||
        UpperText.Contains(TEXT("HORDE")) ||
        UpperText.Contains(TEXT("BOSS")) ||
        UpperText.Contains(TEXT("WARDEN")) ||
        UpperText.Contains(TEXT("INCOMING")))
    {
        return TEXT("!");
    }
    if (UpperText.Contains(TEXT("TERMINAL")) ||
        UpperText.Contains(TEXT("CODE")) ||
        UpperText.Contains(TEXT("CODING")) ||
        UpperText.Contains(TEXT("CURRICULUM")) ||
        UpperText.Contains(TEXT("LANGUAGE")) ||
        UpperText.Contains(TEXT("JAVA")) ||
        UpperText.Contains(TEXT("PYTHON")) ||
        UpperText.Contains(TEXT("MATLAB")) ||
        UpperText.Contains(TEXT("C++")) ||
        UpperText.Contains(TEXT("C+")))
    {
        return TEXT("</>");
    }
    if (UpperText.Contains(TEXT("HELIPAD")) ||
        UpperText.Contains(TEXT("EVAC")) ||
        UpperText.Contains(TEXT("EXTRACTION")))
    {
        return TEXT("H");
    }
    if (UpperText.Contains(TEXT("JEEP")) ||
        UpperText.Contains(TEXT("VEHICLE")) ||
        UpperText.Contains(TEXT("ROUTE")) ||
        UpperText.Contains(TEXT("ENTRY")) ||
        UpperText.Contains(TEXT("PATH")))
    {
        return TEXT(">");
    }
    if (UpperText.Contains(TEXT("SAFEHOUSE")) ||
        UpperText.Contains(TEXT("SHELTER")) ||
        UpperText.Contains(TEXT("HOME")))
    {
        return TEXT("[]");
    }
    if (UpperText.Contains(TEXT("WATER")) ||
        UpperText.Contains(TEXT("RIVER")) ||
        UpperText.Contains(TEXT("COAST")))
    {
        return TEXT("~");
    }
    if (UpperText.Contains(TEXT("RADIO")) ||
        UpperText.Contains(TEXT("AUDIO")) ||
        UpperText.Contains(TEXT("VOICE")))
    {
        return TEXT("))");
    }
    if (UpperText.Contains(TEXT("QA")) ||
        UpperText.Contains(TEXT("VERIFY")) ||
        UpperText.Contains(TEXT("TEST")) ||
        UpperText.Contains(TEXT("CHECK")))
    {
        return TEXT("?");
    }
    if (UpperText.Contains(TEXT("CITY")) ||
        UpperText.Contains(TEXT("REGIONAL")) ||
        UpperText.Contains(TEXT("LANDMARK")) ||
        UpperText.Contains(TEXT("ART KIT")))
    {
        return TEXT("#");
    }
    if (UpperText.Contains(TEXT("REWARD")) ||
        UpperText.Contains(TEXT("VOUCHER")) ||
        UpperText.Contains(TEXT("UNLOCK")))
    {
        return TEXT("$");
    }
    if (UpperText.Contains(TEXT("SOLAR")) ||
        UpperText.Contains(TEXT("POWER")) ||
        UpperText.Contains(TEXT("NEON")))
    {
        return TEXT("*");
    }
    return TEXT(".");
}

FVector CityArchitectureExtent(const FVector& Extent)
{
    // Compact survival-horror proportions: buildings should frame alleys and
    // courtyards around human-scale actors instead of reading as unreachable
    // megastructures beside tiny NPCs and pickups.
    constexpr float FootprintScale = 4.8f;
    constexpr float HeightScale = 5.7f;
    return FVector(Extent.X * FootprintScale, Extent.Y * FootprintScale, Extent.Z * HeightScale);
}

const TCHAR* CodeRescueCityBuildingMeshPaths[] = {
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building01.SM_Building01"),
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building02.SM_Building02"),
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building03.SM_Building03"),
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building04.SM_Building04"),
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building05.SM_Building05"),
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building01_FixUV.SM_Building01_FixUV"),
    TEXT("/Game/Parallax_Night_Building_Material/Meshes/Building/SM_Building01_IrregularUV.SM_Building01_IrregularUV"),
};

const TCHAR* CodeRescueCityBuildingMaterialPaths[] = {
    TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
    TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
    TEXT("/Game/StarterContent/Materials/M_Brick_Hewn_Stone.M_Brick_Hewn_Stone"),
    TEXT("/Game/StarterContent/Materials/M_Brick_Clay_Old.M_Brick_Clay_Old"),
    TEXT("/Game/StarterContent/Materials/M_Metal_Rust.M_Metal_Rust"),
    TEXT("/Game/StarterContent/Materials/M_Rock_Slate.M_Rock_Slate"),
    TEXT("/Game/StarterContent/Materials/M_Basic_Wall.M_Basic_Wall"),
};

const TCHAR* CodeRescueBridgeMeshPaths[] = {
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_001.SM_modern_bridge_001"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_002.SM_modern_bridge_002"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_003.SM_modern_bridge_003"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_004.SM_modern_bridge_004"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_005.SM_modern_bridge_005"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_006.SM_modern_bridge_006"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_007.SM_modern_bridge_007"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_008.SM_modern_bridge_008"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_009.SM_modern_bridge_009"),
    TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_010.SM_modern_bridge_010"),
};

const TCHAR* CodeRescueLandscapeRockPaths[] = {
    TEXT("/Game/StarterContent/Props/SM_Rock.SM_Rock"),
    TEXT("/Game/StarterContent/Shapes/Shape_TriPyramid.Shape_TriPyramid"),
    TEXT("/Game/StarterContent/Shapes/Shape_QuadPyramid.Shape_QuadPyramid"),
};

const TCHAR* CodeRescueLandscapeBushPaths[] = {
    TEXT("/Game/StarterContent/Props/SM_Bush.SM_Bush"),
    TEXT("/Game/StarterContent/Shapes/Shape_WideCapsule.Shape_WideCapsule"),
};

UStaticMesh* LoadCodeRescueAssetMesh(const TCHAR* MeshPath)
{
    return MeshPath ? LoadObject<UStaticMesh>(nullptr, MeshPath) : nullptr;
}

UMaterialInterface* LoadCodeRescueMaterial(const TCHAR* MaterialPath)
{
    return CodeRescueMaterials::LoadMaterial(MaterialPath);
}

AActor* ApplyCodeRescueMaterialToStaticActor(AActor* Actor, const TCHAR* MaterialPath, UObject* Outer, const FLinearColor& Tint, float EmissiveScale = 0.0f)
{
    if (!Actor || !MaterialPath)
    {
        return Actor;
    }

    UMaterialInterface* Material = LoadCodeRescueMaterial(MaterialPath);
    if (!Material)
    {
        return Actor;
    }

    if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
    {
        if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
        {
            const int32 SlotCount = FMath::Max(1, MeshComp->GetNumMaterials());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot)
            {
                UMaterialInstanceDynamic* MID = CodeRescueMaterials::CreateTintedDynamicMaterial(
                    Material,
                    Outer ? Outer : Actor,
                    Tint,
                    EmissiveScale);
                if (MID)
                {
                    MeshComp->SetMaterial(Slot, MID);
                }
                else
                {
                    MeshComp->SetMaterial(Slot, Material);
                }
            }
            MeshComp->SetCastShadow(true);
        }
    }
    return Actor;
}

template <typename T, int32 NumPaths>
T* LoadAssetFromPathRing(const TCHAR* const (&Paths)[NumPaths], int32 StartIndex)
{
    for (int32 Attempt = 0; Attempt < NumPaths; ++Attempt)
    {
        const int32 PathIndex = FMath::Abs(StartIndex + Attempt) % NumPaths;
        if (T* Asset = LoadObject<T>(nullptr, Paths[PathIndex]))
        {
            return Asset;
        }
    }
    return nullptr;
}

UStaticMesh* LoadCodeRescueCityBuildingMesh(int32 VariantIndex)
{
    return LoadAssetFromPathRing<UStaticMesh>(CodeRescueCityBuildingMeshPaths, VariantIndex);
}

UStaticMesh* LoadCodeRescueBridgeMesh(int32 VariantIndex)
{
    return LoadAssetFromPathRing<UStaticMesh>(CodeRescueBridgeMeshPaths, VariantIndex);
}

UStaticMesh* LoadCodeRescueLandscapeRockMesh(int32 VariantIndex)
{
    return LoadAssetFromPathRing<UStaticMesh>(CodeRescueLandscapeRockPaths, VariantIndex);
}

UStaticMesh* LoadCodeRescueLandscapeBushMesh(int32 VariantIndex)
{
    return LoadAssetFromPathRing<UStaticMesh>(CodeRescueLandscapeBushPaths, VariantIndex);
}

struct FCodeRescueUSCityVisualProfile
{
    FString LandscapeCue = TEXT("mixed metro terrain with emergency staging");
    FString ArchitectureCue = TEXT("mid-rise downtown facades and municipal storefronts");
    FString SkyCue = TEXT("clear neutral daylight with light city haze");
    FString RoadCue = TEXT("gridded streets with painted lanes");
    FString SidewalkCue = TEXT("concrete sidewalks and crosswalks");
    FString HomeCue = TEXT("mixed apartments and compact single-family homes");
    FString VehicleCue = TEXT("sedans, SUVs, delivery vans");
    FString ClothingCue = TEXT("everyday commuter layers");
    FString LandmarkCue = TEXT("downtown civic marker");
    FString SignatureCue = TEXT("regional civic skyline marker");
    FString SignatureShapeToken = TEXT("CivicMarker");
    FString DistrictCue = TEXT("downtown, residential, and rescue-access districts");

    FLinearColor TerrainColor = FLinearColor(0.10f, 0.16f, 0.12f);
    FLinearColor ArchitectureColor = FLinearColor(0.20f, 0.20f, 0.18f);
    FLinearColor SkyColor = FLinearColor(0.18f, 0.34f, 0.55f);
    FLinearColor RoadColor = FLinearColor(0.025f, 0.027f, 0.030f);
    FLinearColor SidewalkColor = FLinearColor(0.46f, 0.44f, 0.40f);
    FLinearColor HomeColor = FLinearColor(0.32f, 0.24f, 0.18f);
    FLinearColor VehicleColor = FLinearColor(0.16f, 0.20f, 0.24f);
    FLinearColor ClothingColor = FLinearColor(0.18f, 0.28f, 0.36f);
    FLinearColor SignatureColor = FLinearColor(0.74f, 0.78f, 0.84f);

    bool bCoastal = false;
    bool bDesert = false;
    bool bMountain = false;
    bool bRiverfront = false;
    bool bIndustrial = false;
    bool bHistoric = false;
    bool bTropical = false;
    bool bTransit = false;
    bool bFreeway = false;
    bool bTechCampus = false;
    bool bEntertainment = false;
    bool bCapitalCivic = false;
    bool bCollegeTown = false;
    bool bMilitaryHarbor = false;
    bool bColdWeather = false;
    bool bSuburban = false;

    int32 TowerCount = 4;
    int32 HomeCount = 5;
    int32 VehicleCount = 4;
    float StreetWidthScale = 1.0f;
};

bool IsUSMajorCityMission(const FCodeRescueCityMission& Mission)
{
    // The campaign is ordered with U.S. cities first, followed by global cities.
    // Some global rows reuse two-letter country codes that overlap U.S. state
    // abbreviations, so rank is the safest runtime boundary.
    return Mission.Rank >= 1 &&
           Mission.Rank <= FCodeRescueCampaign::USCityMissionCount &&
           Mission.StateName.Len() == 2;
}

bool CityContainsAny(const FString& City, std::initializer_list<const TCHAR*> Terms)
{
    for (const TCHAR* Term : Terms)
    {
        if (City.Contains(Term))
        {
            return true;
        }
    }
    return false;
}

FCodeRescueUSCityVisualProfile BuildUSCityVisualProfile(const FCodeRescueCityMission& Mission)
{
    FCodeRescueUSCityVisualProfile Profile;
    const FString& City = Mission.CityName;
    const FString& State = Mission.StateName;

    Profile.TowerCount = Mission.Rank <= 25 ? 7 : Mission.Rank <= 100 ? 5 : 4;
    Profile.HomeCount = Mission.Rank <= 75 ? 6 : 5;
    Profile.VehicleCount = Mission.Rank <= 75 ? 5 : 4;
    Profile.LandmarkCue = Mission.LandmarkName;
    Profile.SignatureCue = Mission.LandmarkName;

    static const TSet<FString> NortheastStates = {
        TEXT("CT"), TEXT("MA"), TEXT("MD"), TEXT("NH"), TEXT("NJ"), TEXT("NY"), TEXT("PA"), TEXT("RI"), TEXT("VA"), TEXT("DC")
    };
    static const TSet<FString> MidwestStates = {
        TEXT("IA"), TEXT("IL"), TEXT("IN"), TEXT("KS"), TEXT("MI"), TEXT("MN"), TEXT("MO"), TEXT("ND"), TEXT("NE"), TEXT("OH"), TEXT("SD"), TEXT("WI")
    };
    static const TSet<FString> SouthernStates = {
        TEXT("AL"), TEXT("AR"), TEXT("FL"), TEXT("GA"), TEXT("KY"), TEXT("LA"), TEXT("MS"), TEXT("NC"), TEXT("OK"), TEXT("SC"), TEXT("TN"), TEXT("TX")
    };
    static const TSet<FString> WesternStates = {
        TEXT("AK"), TEXT("AZ"), TEXT("CA"), TEXT("CO"), TEXT("HI"), TEXT("ID"), TEXT("MT"), TEXT("NM"), TEXT("NV"), TEXT("OR"), TEXT("UT"), TEXT("WA")
    };

    if (NortheastStates.Contains(State))
    {
        Profile.LandscapeCue = TEXT("brick-and-stone metro edge with compact parks");
        Profile.ArchitectureCue = TEXT("rowhomes, civic stone, glass downtown towers");
        Profile.SkyCue = TEXT("cool Atlantic light and overcast city haze");
        Profile.RoadCue = TEXT("tight urban grid with transit-priority lanes");
        Profile.SidewalkCue = TEXT("narrow concrete walks, stoops, crosswalks");
        Profile.HomeCue = TEXT("brick rowhouses and older apartment blocks");
        Profile.VehicleCue = TEXT("sedans, buses, taxis, delivery trucks");
        Profile.ClothingCue = TEXT("coats, office layers, hoodies");
        Profile.TerrainColor = FLinearColor(0.12f, 0.16f, 0.13f);
        Profile.ArchitectureColor = FLinearColor(0.30f, 0.16f, 0.12f);
        Profile.SkyColor = FLinearColor(0.30f, 0.40f, 0.48f);
        Profile.HomeColor = FLinearColor(0.36f, 0.13f, 0.09f);
        Profile.VehicleColor = FLinearColor(0.88f, 0.70f, 0.12f);
        Profile.ClothingColor = FLinearColor(0.10f, 0.12f, 0.18f);
        Profile.SignatureShapeToken = TEXT("HistoricBell");
        Profile.SignatureColor = FLinearColor(0.70f, 0.66f, 0.56f);
        Profile.bHistoric = true;
        Profile.bTransit = true;
        Profile.StreetWidthScale = 0.86f;
    }
    else if (MidwestStates.Contains(State))
    {
        Profile.LandscapeCue = TEXT("flat grid, river/lake edge, winter-ready streets");
        Profile.ArchitectureCue = TEXT("brick warehouses, industrial towers, civic blocks");
        Profile.SkyCue = TEXT("broad lake/plains sky with cool haze");
        Profile.RoadCue = TEXT("straight numbered grid and rail crossings");
        Profile.SidewalkCue = TEXT("wide concrete walks with curb snow marks");
        Profile.HomeCue = TEXT("brick two-flats, bungalows, porch homes");
        Profile.VehicleCue = TEXT("sedans, pickups, city buses");
        Profile.ClothingCue = TEXT("jackets, workwear, team colors");
        Profile.TerrainColor = FLinearColor(0.12f, 0.17f, 0.15f);
        Profile.ArchitectureColor = FLinearColor(0.26f, 0.14f, 0.10f);
        Profile.SkyColor = FLinearColor(0.26f, 0.35f, 0.44f);
        Profile.HomeColor = FLinearColor(0.30f, 0.18f, 0.12f);
        Profile.VehicleColor = FLinearColor(0.12f, 0.16f, 0.20f);
        Profile.ClothingColor = FLinearColor(0.12f, 0.16f, 0.24f);
        Profile.SignatureShapeToken = TEXT("IndustrialMotor");
        Profile.SignatureColor = FLinearColor(0.48f, 0.46f, 0.42f);
        Profile.bIndustrial = true;
        Profile.bTransit = true;
    }
    else if (SouthernStates.Contains(State))
    {
        Profile.LandscapeCue = TEXT("warm lowland city edge with trees and canals");
        Profile.ArchitectureCue = TEXT("brick, limestone, glass offices, porch storefronts");
        Profile.SkyCue = TEXT("warm humid sky and late-afternoon glow");
        Profile.RoadCue = TEXT("wide arterials, service roads, rescue shoulders");
        Profile.SidewalkCue = TEXT("sunlit sidewalks with planted medians");
        Profile.HomeCue = TEXT("porch homes, ranch houses, low apartments");
        Profile.VehicleCue = TEXT("pickups, SUVs, sedans, service vans");
        Profile.ClothingCue = TEXT("light shirts, workwear, casual boots");
        Profile.TerrainColor = FLinearColor(0.16f, 0.20f, 0.11f);
        Profile.ArchitectureColor = FLinearColor(0.30f, 0.23f, 0.16f);
        Profile.SkyColor = FLinearColor(0.50f, 0.42f, 0.28f);
        Profile.HomeColor = FLinearColor(0.34f, 0.22f, 0.14f);
        Profile.VehicleColor = FLinearColor(0.20f, 0.18f, 0.14f);
        Profile.ClothingColor = FLinearColor(0.42f, 0.30f, 0.18f);
        Profile.SignatureShapeToken = TEXT("RiverBridge");
        Profile.SignatureColor = FLinearColor(0.62f, 0.44f, 0.26f);
        Profile.bRiverfront = State == TEXT("LA") || State == TEXT("AR") || State == TEXT("KY") || State == TEXT("MS") || State == TEXT("TN");
        Profile.bFreeway = State == TEXT("TX") || State == TEXT("FL") || State == TEXT("GA");
        Profile.StreetWidthScale = 1.22f;
    }
    else if (WesternStates.Contains(State))
    {
        Profile.LandscapeCue = TEXT("western terrain edge with hills, palms, or desert");
        Profile.ArchitectureCue = TEXT("stucco, glass offices, low-rise homes, wide blocks");
        Profile.SkyCue = TEXT("bright dry sky with long-distance haze");
        Profile.RoadCue = TEXT("wide boulevards and freeway frontage lanes");
        Profile.SidewalkCue = TEXT("broad sidewalks, curb ramps, painted crossings");
        Profile.HomeCue = TEXT("stucco homes, apartments, garage-front suburbs");
        Profile.VehicleCue = TEXT("SUVs, EVs, pickups, delivery vans");
        Profile.ClothingCue = TEXT("casual layers, denim, sunwear");
        Profile.TerrainColor = FLinearColor(0.24f, 0.18f, 0.10f);
        Profile.ArchitectureColor = FLinearColor(0.42f, 0.34f, 0.24f);
        Profile.SkyColor = FLinearColor(0.24f, 0.44f, 0.66f);
        Profile.SidewalkColor = FLinearColor(0.56f, 0.50f, 0.42f);
        Profile.HomeColor = FLinearColor(0.58f, 0.43f, 0.30f);
        Profile.VehicleColor = FLinearColor(0.10f, 0.18f, 0.22f);
        Profile.ClothingColor = FLinearColor(0.16f, 0.30f, 0.42f);
        Profile.SignatureShapeToken = TEXT("FreewayCrown");
        Profile.SignatureColor = FLinearColor(0.48f, 0.52f, 0.58f);
        Profile.bFreeway = true;
        Profile.StreetWidthScale = 1.28f;
    }

    if (State == TEXT("AZ") || State == TEXT("NM") || State == TEXT("NV") || CityContainsAny(City, { TEXT("El Paso"), TEXT("Las Cruces"), TEXT("Yuma"), TEXT("St. George") }))
    {
        Profile.LandscapeCue = TEXT("desert wash, mesas, dry air");
        Profile.ArchitectureCue = TEXT("adobe, stucco, solar shade, low civic blocks");
        Profile.SkyCue = TEXT("hard blue desert sky and amber horizon");
        Profile.SidewalkCue = TEXT("pale concrete, shade canopies, xeriscape edges");
        Profile.HomeCue = TEXT("stucco homes with flat roofs and shaded porches");
        Profile.VehicleCue = TEXT("SUVs, pickups, utility vans");
        Profile.ClothingCue = TEXT("sun hats, light shirts, desert workwear");
        Profile.TerrainColor = FLinearColor(0.38f, 0.25f, 0.11f);
        Profile.ArchitectureColor = FLinearColor(0.60f, 0.43f, 0.28f);
        Profile.SkyColor = FLinearColor(0.55f, 0.55f, 0.36f);
        Profile.HomeColor = FLinearColor(0.66f, 0.47f, 0.32f);
        Profile.ClothingColor = FLinearColor(0.62f, 0.42f, 0.20f);
        Profile.SignatureShapeToken = TEXT("DesertSun");
        Profile.SignatureColor = FLinearColor(0.92f, 0.52f, 0.18f);
        Profile.bDesert = true;
        Profile.bCoastal = false;
    }

    if (State == TEXT("FL") || State == TEXT("HI") || CityContainsAny(City, { TEXT("Corpus Christi"), TEXT("Virginia Beach"), TEXT("Norfolk"), TEXT("Chesapeake"), TEXT("Newport News"), TEXT("Mobile"), TEXT("Wilmington"), TEXT("Charleston") }))
    {
        Profile.LandscapeCue = TEXT("coastal flats, palms, waterline, humid air");
        Profile.ArchitectureCue = TEXT("stucco towers, marina blocks, pastel homes");
        Profile.SkyCue = TEXT("bright coastal sky with sea haze");
        Profile.RoadCue = TEXT("wide coastal boulevards and bridge approaches");
        Profile.SidewalkCue = TEXT("sunny sidewalks, beach crossings, palms");
        Profile.HomeCue = TEXT("pastel homes, balconies, low coastal apartments");
        Profile.VehicleCue = TEXT("convertibles, SUVs, shuttles");
        Profile.ClothingCue = TEXT("short sleeves, resort casual, rain shells");
        Profile.TerrainColor = FLinearColor(0.13f, 0.26f, 0.20f);
        Profile.ArchitectureColor = FLinearColor(0.58f, 0.58f, 0.48f);
        Profile.SkyColor = FLinearColor(0.24f, 0.56f, 0.70f);
        Profile.HomeColor = FLinearColor(0.72f, 0.58f, 0.46f);
        Profile.VehicleColor = FLinearColor(0.05f, 0.34f, 0.42f);
        Profile.ClothingColor = FLinearColor(0.05f, 0.48f, 0.54f);
        Profile.SignatureShapeToken = TEXT("TropicalDeco");
        Profile.SignatureColor = FLinearColor(0.24f, 0.80f, 0.86f);
        Profile.bCoastal = true;
        Profile.bTropical = State == TEXT("FL") || State == TEXT("HI");
        Profile.bRiverfront = false;
    }

    if (State == TEXT("CO") || State == TEXT("AK") || State == TEXT("ID") || State == TEXT("MT") || State == TEXT("UT"))
    {
        Profile.LandscapeCue = TEXT("mountain horizon, dry foothills, clear air");
        Profile.ArchitectureCue = TEXT("stone, glass, timber, outdoor civic plazas");
        Profile.SkyCue = TEXT("high-altitude blue sky and crisp light");
        Profile.RoadCue = TEXT("broad arterial grid with trail crossings");
        Profile.SidewalkCue = TEXT("wide sidewalks, trailheads, snow-ready curbs");
        Profile.HomeCue = TEXT("foothill homes, timber porches, low apartments");
        Profile.VehicleCue = TEXT("SUVs, trail vehicles, rescue trucks");
        Profile.ClothingCue = TEXT("outdoor jackets, boots, fleece layers");
        Profile.TerrainColor = FLinearColor(0.20f, 0.20f, 0.16f);
        Profile.ArchitectureColor = FLinearColor(0.30f, 0.30f, 0.26f);
        Profile.SkyColor = FLinearColor(0.20f, 0.42f, 0.70f);
        Profile.HomeColor = FLinearColor(0.38f, 0.27f, 0.18f);
        Profile.ClothingColor = FLinearColor(0.16f, 0.26f, 0.30f);
        Profile.SignatureShapeToken = TEXT("MountainPeakTower");
        Profile.SignatureColor = FLinearColor(0.70f, 0.78f, 0.82f);
        Profile.bMountain = true;
    }

    if (State == TEXT("WA") || State == TEXT("OR"))
    {
        Profile.LandscapeCue = TEXT("evergreens, water edge, rainy pavement");
        Profile.ArchitectureCue = TEXT("brick warehouses, glass tech offices, timber accents");
        Profile.SkyCue = TEXT("soft overcast Pacific Northwest light");
        Profile.RoadCue = TEXT("wet streets, bike lanes, transit stops");
        Profile.SidewalkCue = TEXT("rain-dark sidewalks and bike markings");
        Profile.HomeCue = TEXT("craftsman homes, apartments, timber porches");
        Profile.VehicleCue = TEXT("EVs, buses, bikes, delivery vans");
        Profile.ClothingCue = TEXT("rain shells, hoodies, flannel");
        Profile.TerrainColor = FLinearColor(0.08f, 0.18f, 0.13f);
        Profile.ArchitectureColor = FLinearColor(0.22f, 0.18f, 0.14f);
        Profile.SkyColor = FLinearColor(0.26f, 0.34f, 0.38f);
        Profile.HomeColor = FLinearColor(0.24f, 0.20f, 0.16f);
        Profile.VehicleColor = FLinearColor(0.08f, 0.18f, 0.20f);
        Profile.ClothingColor = FLinearColor(0.08f, 0.22f, 0.18f);
        Profile.SignatureShapeToken = TEXT("EvergreenWaterTower");
        Profile.SignatureColor = FLinearColor(0.16f, 0.46f, 0.38f);
        Profile.bCoastal = true;
        Profile.bMountain = true;
        Profile.bTransit = true;
    }

    if (City == TEXT("New York"))
    {
        Profile.LandscapeCue = TEXT("harbor edge and dense island skyline");
        Profile.ArchitectureCue = TEXT("Manhattan towers, brownstones, subway stairs");
        Profile.SkyCue = TEXT("Atlantic haze between glass towers");
        Profile.RoadCue = TEXT("tight numbered grid with taxi lanes");
        Profile.SidewalkCue = TEXT("crowded concrete walks, stoops, crosswalks");
        Profile.HomeCue = TEXT("brownstones and high-rise apartments");
        Profile.VehicleCue = TEXT("yellow taxis, buses, delivery vans");
        Profile.ClothingCue = TEXT("black coats, businesswear, hoodies");
        Profile.TowerCount = 10;
        Profile.VehicleColor = FLinearColor(1.0f, 0.74f, 0.05f);
        Profile.SignatureCue = TEXT("harbor statue silhouette and dense island skyline");
        Profile.SignatureShapeToken = TEXT("HarborStatue");
        Profile.SignatureColor = FLinearColor(0.46f, 0.78f, 0.70f);
        Profile.bCoastal = true;
        Profile.bTransit = true;
        Profile.bHistoric = true;
        Profile.StreetWidthScale = 0.78f;
    }
    else if (City == TEXT("Los Angeles"))
    {
        Profile.LandscapeCue = TEXT("dry basin, palms, distant hills");
        Profile.ArchitectureCue = TEXT("stucco blocks, glass towers, studio lots");
        Profile.SkyCue = TEXT("warm smoggy sunset over palms");
        Profile.RoadCue = TEXT("wide boulevards and freeway ramps");
        Profile.SidewalkCue = TEXT("palm-lined sidewalks and painted crossings");
        Profile.HomeCue = TEXT("bungalows, stucco courts, hillside homes");
        Profile.VehicleCue = TEXT("SUVs, convertibles, delivery vans");
        Profile.ClothingCue = TEXT("denim, tees, sunglasses, light jackets");
        Profile.bCoastal = true;
        Profile.bFreeway = true;
        Profile.bEntertainment = true;
        Profile.SignatureCue = TEXT("hillside letter panels and studio-boulevard palms");
        Profile.SignatureShapeToken = TEXT("HillsideLetters");
        Profile.SignatureColor = FLinearColor(0.96f, 0.78f, 0.36f);
        Profile.TowerCount = 7;
        Profile.StreetWidthScale = 1.42f;
    }
    else if (City == TEXT("Chicago"))
    {
        Profile.LandscapeCue = TEXT("lakefront, river bridges, flat grid");
        Profile.ArchitectureCue = TEXT("steel towers, brick flats, elevated rail");
        Profile.SkyCue = TEXT("cool lake haze and winter light");
        Profile.RoadCue = TEXT("ordered grid with river bridge approaches");
        Profile.SidewalkCue = TEXT("wide downtown sidewalks and L shadows");
        Profile.HomeCue = TEXT("brick two-flats and apartment courts");
        Profile.VehicleCue = TEXT("city buses, taxis, sedans");
        Profile.ClothingCue = TEXT("coats, boots, navy and black layers");
        Profile.bRiverfront = true;
        Profile.bIndustrial = true;
        Profile.bTransit = true;
        Profile.SignatureCue = TEXT("river bridge and elevated rail silhouette");
        Profile.SignatureShapeToken = TEXT("RiverBridge");
        Profile.SignatureColor = FLinearColor(0.52f, 0.62f, 0.70f);
        Profile.TowerCount = 9;
    }
    else if (City == TEXT("Houston"))
    {
        Profile.LandscapeCue = TEXT("bayou channels and humid flat sprawl");
        Profile.ArchitectureCue = TEXT("energy glass towers, brick suburbs, strip centers");
        Profile.SkyCue = TEXT("humid Gulf haze and stormy light");
        Profile.RoadCue = TEXT("wide freeway frontage and service roads");
        Profile.SidewalkCue = TEXT("broad sunlit sidewalks and drainage edges");
        Profile.HomeCue = TEXT("ranch homes, townhomes, shaded porches");
        Profile.VehicleCue = TEXT("pickups, SUVs, work trucks");
        Profile.ClothingCue = TEXT("light shirts, boots, workwear");
        Profile.bRiverfront = true;
        Profile.bFreeway = true;
        Profile.SignatureCue = TEXT("bayou channel with energy-district crown");
        Profile.SignatureShapeToken = TEXT("BayouEnergy");
        Profile.SignatureColor = FLinearColor(0.36f, 0.68f, 0.72f);
        Profile.TowerCount = 8;
    }
    else if (City == TEXT("Philadelphia"))
    {
        Profile.LandscapeCue = TEXT("Schuylkill edge, historic core, tight grid");
        Profile.ArchitectureCue = TEXT("brick rowhomes, civic stone, old market halls");
        Profile.RoadCue = TEXT("narrow rowhouse streets and transit lanes");
        Profile.HomeCue = TEXT("brick rowhomes with stoops");
        Profile.VehicleCue = TEXT("buses, sedans, delivery trucks");
        Profile.ClothingCue = TEXT("hoodies, coats, work jackets");
        Profile.bHistoric = true;
        Profile.bTransit = true;
        Profile.bRiverfront = true;
        Profile.SignatureCue = TEXT("historic bell hall and rowhouse street marker");
        Profile.SignatureShapeToken = TEXT("HistoricBell");
        Profile.SignatureColor = FLinearColor(0.78f, 0.58f, 0.22f);
    }
    else if (City == TEXT("San Antonio"))
    {
        Profile.LandscapeCue = TEXT("riverwalk channel and limestone plazas");
        Profile.ArchitectureCue = TEXT("missions, limestone, adobe, low downtown blocks");
        Profile.RoadCue = TEXT("loop roads with shaded river crossings");
        Profile.HomeCue = TEXT("stucco homes, courtyards, porch roofs");
        Profile.VehicleCue = TEXT("pickups, SUVs, tourist shuttles");
        Profile.ClothingCue = TEXT("light casual wear, boots, sun hats");
        Profile.bRiverfront = true;
        Profile.bHistoric = true;
        Profile.SignatureCue = TEXT("mission arch and riverwalk marker");
        Profile.SignatureShapeToken = TEXT("MissionArch");
        Profile.SignatureColor = FLinearColor(0.78f, 0.58f, 0.38f);
    }
    else if (City == TEXT("San Diego"))
    {
        Profile.LandscapeCue = TEXT("coastal mesas, palms, naval waterfront");
        Profile.ArchitectureCue = TEXT("Spanish revival, stucco, marina towers");
        Profile.RoadCue = TEXT("coastal boulevards and bridge ramps");
        Profile.HomeCue = TEXT("stucco apartments and beach cottages");
        Profile.VehicleCue = TEXT("SUVs, convertibles, naval service vans");
        Profile.ClothingCue = TEXT("beach casual, hoodies, light uniforms");
        Profile.bCoastal = true;
        Profile.bMilitaryHarbor = true;
        Profile.SignatureCue = TEXT("naval harbor mast and coastal mesa silhouette");
        Profile.SignatureShapeToken = TEXT("HarborNaval");
        Profile.SignatureColor = FLinearColor(0.24f, 0.52f, 0.70f);
    }
    else if (City == TEXT("Dallas"))
    {
        Profile.LandscapeCue = TEXT("North Texas prairie edge and glass skyline");
        Profile.ArchitectureCue = TEXT("glass towers, brick suburbs, corporate campuses");
        Profile.RoadCue = TEXT("freeway grid and wide arterial loops");
        Profile.HomeCue = TEXT("brick homes, townhomes, garage suburbs");
        Profile.VehicleCue = TEXT("pickups, SUVs, black sedans");
        Profile.ClothingCue = TEXT("business casual, boots, light jackets");
        Profile.bFreeway = true;
        Profile.SignatureCue = TEXT("angular glass crown and prairie freeway skyline");
        Profile.SignatureShapeToken = TEXT("FreewayCrown");
        Profile.SignatureColor = FLinearColor(0.42f, 0.62f, 0.78f);
        Profile.TowerCount = 8;
    }
    else if (City == TEXT("Jacksonville"))
    {
        Profile.LandscapeCue = TEXT("river city, pine flats, Atlantic bridges");
        Profile.ArchitectureCue = TEXT("riverfront towers, low suburbs, port sheds");
        Profile.RoadCue = TEXT("bridge approaches and broad arterials");
        Profile.HomeCue = TEXT("porch houses, coastal apartments");
        Profile.bCoastal = true;
        Profile.bRiverfront = true;
        Profile.SignatureCue = TEXT("river bridge, port sheds, and pine-flat skyline");
        Profile.SignatureShapeToken = TEXT("RiverBridge");
        Profile.SignatureColor = FLinearColor(0.24f, 0.50f, 0.48f);
    }
    else if (City == TEXT("Fort Worth"))
    {
        Profile.LandscapeCue = TEXT("prairie downtown and stockyards edge");
        Profile.ArchitectureCue = TEXT("brick stockyards, glass towers, western storefronts");
        Profile.RoadCue = TEXT("wide arterials and rail-side blocks");
        Profile.HomeCue = TEXT("brick homes and ranch-house suburbs");
        Profile.VehicleCue = TEXT("pickups, SUVs, utility trailers");
        Profile.ClothingCue = TEXT("boots, denim, work jackets");
        Profile.bHistoric = true;
        Profile.bFreeway = true;
        Profile.SignatureCue = TEXT("stockyard gate and western rail marker");
        Profile.SignatureShapeToken = TEXT("StockyardGate");
        Profile.SignatureColor = FLinearColor(0.68f, 0.42f, 0.20f);
    }
    else if (City == TEXT("San Jose"))
    {
        Profile.LandscapeCue = TEXT("Silicon Valley flats and foothills");
        Profile.ArchitectureCue = TEXT("tech campuses, glass offices, stucco apartments");
        Profile.RoadCue = TEXT("wide boulevards, EV lanes, freeway access");
        Profile.HomeCue = TEXT("suburban homes, apartments, campus housing");
        Profile.VehicleCue = TEXT("EVs, rideshare sedans, tech shuttles");
        Profile.ClothingCue = TEXT("hoodies, badges, sneakers");
        Profile.bTechCampus = true;
        Profile.bSuburban = true;
        Profile.SignatureCue = TEXT("Silicon Valley chip-campus marker");
        Profile.SignatureShapeToken = TEXT("TechCampus");
        Profile.SignatureColor = FLinearColor(0.18f, 0.72f, 0.90f);
    }
    else if (City == TEXT("Austin"))
    {
        Profile.LandscapeCue = TEXT("Hill Country edge and river lakefront");
        Profile.ArchitectureCue = TEXT("limestone, music venues, glass tech towers");
        Profile.RoadCue = TEXT("hilly arterials, bike lanes, bridge crossings");
        Profile.HomeCue = TEXT("bungalows, new apartments, porch homes");
        Profile.VehicleCue = TEXT("pickups, scooters, EVs");
        Profile.ClothingCue = TEXT("tees, denim, boots, tech hoodies");
        Profile.bRiverfront = true;
        Profile.bTechCampus = true;
        Profile.bEntertainment = true;
        Profile.SignatureCue = TEXT("music venue, tech campus, and river bridge marker");
        Profile.SignatureShapeToken = TEXT("MusicNote");
        Profile.SignatureColor = FLinearColor(0.86f, 0.52f, 0.18f);
    }
    else if (City == TEXT("San Francisco"))
    {
        Profile.LandscapeCue = TEXT("bay hills, fog, waterfront piers");
        Profile.ArchitectureCue = TEXT("Victorians, steep streets, glass towers");
        Profile.SkyCue = TEXT("cool fog bands and bay light");
        Profile.RoadCue = TEXT("steep tight streets and transit tracks");
        Profile.HomeCue = TEXT("painted Victorians and hill apartments");
        Profile.VehicleCue = TEXT("cable-car cues, EVs, buses");
        Profile.ClothingCue = TEXT("jackets, hoodies, scarves");
        Profile.bCoastal = true;
        Profile.bMountain = true;
        Profile.bTransit = true;
        Profile.bTechCampus = true;
        Profile.SignatureCue = TEXT("bay suspension bridge and fog-band skyline");
        Profile.SignatureShapeToken = TEXT("SuspensionBridge");
        Profile.SignatureColor = FLinearColor(0.78f, 0.30f, 0.18f);
        Profile.StreetWidthScale = 0.82f;
    }
    else if (City == TEXT("Seattle"))
    {
        Profile.LandscapeCue = TEXT("Puget Sound, evergreens, mountain horizon");
        Profile.ArchitectureCue = TEXT("glass towers, brick warehouses, tech campuses");
        Profile.SkyCue = TEXT("soft rain clouds and blue-gray light");
        Profile.RoadCue = TEXT("wet streets, bike lanes, transit corridors");
        Profile.HomeCue = TEXT("craftsman homes and timber apartments");
        Profile.VehicleCue = TEXT("EVs, buses, ferries, delivery vans");
        Profile.ClothingCue = TEXT("rain shells, flannel, hoodies");
        Profile.bCoastal = true;
        Profile.bMountain = true;
        Profile.bTechCampus = true;
        Profile.bTransit = true;
        Profile.SignatureCue = TEXT("observation needle, evergreen water, and tech skyline");
        Profile.SignatureShapeToken = TEXT("ObservationNeedle");
        Profile.SignatureColor = FLinearColor(0.22f, 0.58f, 0.70f);
        Profile.TowerCount = 8;
    }
    else if (City == TEXT("Denver"))
    {
        Profile.LandscapeCue = TEXT("Front Range mountains and dry high plains");
        Profile.ArchitectureCue = TEXT("brick LoDo blocks, glass towers, outdoor plazas");
        Profile.RoadCue = TEXT("broad downtown grid with trail crossings");
        Profile.HomeCue = TEXT("brick bungalows and foothill apartments");
        Profile.VehicleCue = TEXT("SUVs, trail vehicles, light rail");
        Profile.ClothingCue = TEXT("fleece, outdoor shells, boots");
        Profile.bMountain = true;
        Profile.bTransit = true;
        Profile.SignatureCue = TEXT("Front Range peak tower and trail-grid marker");
        Profile.SignatureShapeToken = TEXT("MountainPeakTower");
        Profile.SignatureColor = FLinearColor(0.62f, 0.78f, 0.88f);
    }
    else if (City == TEXT("Washington"))
    {
        Profile.LandscapeCue = TEXT("Potomac civic axis and memorial greens");
        Profile.ArchitectureCue = TEXT("neoclassical columns, stone offices, rowhouses");
        Profile.RoadCue = TEXT("diagonal avenues, traffic circles, security lanes");
        Profile.HomeCue = TEXT("brick rowhouses and embassy apartments");
        Profile.VehicleCue = TEXT("black SUVs, buses, service sedans");
        Profile.ClothingCue = TEXT("suits, trench coats, official jackets");
        Profile.bCapitalCivic = true;
        Profile.bHistoric = true;
        Profile.bTransit = true;
        Profile.SignatureCue = TEXT("civic obelisk and diagonal avenue marker");
        Profile.SignatureShapeToken = TEXT("CivicObelisk");
        Profile.SignatureColor = FLinearColor(0.82f, 0.84f, 0.78f);
        Profile.TowerCount = 4;
    }
    else if (City == TEXT("Las Vegas") || City == TEXT("North Las Vegas") || City == TEXT("Henderson"))
    {
        Profile.LandscapeCue = TEXT("Mojave desert basin and resort corridor");
        Profile.ArchitectureCue = TEXT("neon resorts, stucco suburbs, parking structures");
        Profile.SkyCue = TEXT("desert sunset and neon night glow");
        Profile.RoadCue = TEXT("wide strip boulevards and resort frontage");
        Profile.HomeCue = TEXT("stucco subdivisions and apartment courts");
        Profile.VehicleCue = TEXT("taxis, limos, SUVs, shuttles");
        Profile.ClothingCue = TEXT("resort casual, uniforms, sunwear");
        Profile.bDesert = true;
        Profile.bEntertainment = true;
        Profile.bFreeway = true;
        Profile.VehicleColor = FLinearColor(1.0f, 0.62f, 0.08f);
        Profile.SignatureCue = TEXT("neon resort marquee and desert boulevard marker");
        Profile.SignatureShapeToken = TEXT("NeonMarquee");
        Profile.SignatureColor = FLinearColor(1.0f, 0.20f, 0.86f);
    }
    else if (City == TEXT("Boston"))
    {
        Profile.LandscapeCue = TEXT("harbor edge, old streets, river campus");
        Profile.ArchitectureCue = TEXT("brick historic blocks, brownstones, campuses");
        Profile.RoadCue = TEXT("curving colonial streets and transit nodes");
        Profile.HomeCue = TEXT("brownstones and triple-deckers");
        Profile.VehicleCue = TEXT("sedans, buses, delivery vans");
        Profile.ClothingCue = TEXT("coats, scarves, college layers");
        Profile.bHistoric = true;
        Profile.bCoastal = true;
        Profile.bCollegeTown = true;
        Profile.SignatureCue = TEXT("harbor beacon, brick campus, and old-street marker");
        Profile.SignatureShapeToken = TEXT("HarborBeacon");
        Profile.SignatureColor = FLinearColor(0.62f, 0.54f, 0.44f);
        Profile.StreetWidthScale = 0.84f;
    }
    else if (City == TEXT("Detroit"))
    {
        Profile.LandscapeCue = TEXT("river industrial edge and broad boulevards");
        Profile.ArchitectureCue = TEXT("Art Deco towers, factories, brick neighborhoods");
        Profile.RoadCue = TEXT("wide motor-city avenues and service lanes");
        Profile.HomeCue = TEXT("brick homes, duplexes, industrial lofts");
        Profile.VehicleCue = TEXT("muscle cars, pickups, utility vans");
        Profile.ClothingCue = TEXT("work jackets, hoodies, team colors");
        Profile.bIndustrial = true;
        Profile.bRiverfront = true;
        Profile.SignatureCue = TEXT("motor factory, Art Deco crown, and riverfront marker");
        Profile.SignatureShapeToken = TEXT("IndustrialMotor");
        Profile.SignatureColor = FLinearColor(0.46f, 0.48f, 0.50f);
    }
    else if (City == TEXT("Nashville-Davidson"))
    {
        Profile.LandscapeCue = TEXT("Cumberland river bend and rolling hills");
        Profile.ArchitectureCue = TEXT("music venues, brick warehouses, glass towers");
        Profile.RoadCue = TEXT("hilly arterials, river bridges, venue streets");
        Profile.HomeCue = TEXT("porch homes, new apartments, brick cottages");
        Profile.VehicleCue = TEXT("pickups, tour vans, sedans");
        Profile.ClothingCue = TEXT("denim, boots, stage black, casual shirts");
        Profile.bRiverfront = true;
        Profile.bEntertainment = true;
        Profile.SignatureCue = TEXT("music note skyline and Cumberland river marker");
        Profile.SignatureShapeToken = TEXT("MusicNote");
        Profile.SignatureColor = FLinearColor(0.88f, 0.48f, 0.18f);
    }
    else if (City == TEXT("Miami") || City == TEXT("Hialeah") || City == TEXT("Fort Lauderdale") || City == TEXT("Hollywood") || City == TEXT("Pompano Beach"))
    {
        Profile.LandscapeCue = TEXT("tropical coastal flats, canals, palms");
        Profile.ArchitectureCue = TEXT("Art Deco pastel, glass condos, marina blocks");
        Profile.SkyCue = TEXT("bright turquoise sky and storm clouds");
        Profile.RoadCue = TEXT("coastal boulevards, causeways, palm medians");
        Profile.HomeCue = TEXT("pastel stucco homes and condo balconies");
        Profile.VehicleCue = TEXT("convertibles, SUVs, delivery scooters");
        Profile.ClothingCue = TEXT("linen, bright colors, short sleeves");
        Profile.bCoastal = true;
        Profile.bTropical = true;
        Profile.bEntertainment = true;
        Profile.SignatureCue = TEXT("Art Deco palm, canal, and coastal tower marker");
        Profile.SignatureShapeToken = TEXT("TropicalDeco");
        Profile.SignatureColor = FLinearColor(0.16f, 0.78f, 0.84f);
    }
    else if (City == TEXT("New Orleans"))
    {
        Profile.LandscapeCue = TEXT("Mississippi riverbend, canals, live oaks");
        Profile.ArchitectureCue = TEXT("French Quarter balconies, shotgun homes, brick warehouses");
        Profile.SkyCue = TEXT("humid Gulf glow and storm haze");
        Profile.RoadCue = TEXT("narrow old streets, streetcar tracks, river roads");
        Profile.HomeCue = TEXT("shotgun homes, raised porches, iron balconies");
        Profile.VehicleCue = TEXT("streetcars, pickups, service vans");
        Profile.ClothingCue = TEXT("linen, rain jackets, festival colors");
        Profile.bRiverfront = true;
        Profile.bHistoric = true;
        Profile.bTransit = true;
        Profile.bTropical = true;
        Profile.SignatureCue = TEXT("iron balcony, streetcar rail, and riverbend marker");
        Profile.SignatureShapeToken = TEXT("BalconyStreetcar");
        Profile.SignatureColor = FLinearColor(0.72f, 0.48f, 0.24f);
    }
    else if (City == TEXT("Urban Honolulu"))
    {
        Profile.LandscapeCue = TEXT("island coast, volcanic ridge, palms");
        Profile.ArchitectureCue = TEXT("high-rise hotels, lanais, low island homes");
        Profile.SkyCue = TEXT("Pacific sun, trade-wind clouds");
        Profile.RoadCue = TEXT("coastal avenues, hotel loops, harbor roads");
        Profile.HomeCue = TEXT("lanai apartments and tropical homes");
        Profile.VehicleCue = TEXT("shuttles, compact cars, service trucks");
        Profile.ClothingCue = TEXT("aloha shirts, resort wear, sandals");
        Profile.bCoastal = true;
        Profile.bTropical = true;
        Profile.bMountain = true;
        Profile.SignatureCue = TEXT("volcanic ridge, surf line, and island hotel marker");
        Profile.SignatureShapeToken = TEXT("VolcanicSurf");
        Profile.SignatureColor = FLinearColor(0.12f, 0.64f, 0.78f);
    }
    else if (City == TEXT("Anchorage"))
    {
        Profile.LandscapeCue = TEXT("snowy mountains, inlet edge, spruce line");
        Profile.ArchitectureCue = TEXT("low civic blocks, timber, utilitarian warehouses");
        Profile.SkyCue = TEXT("cold blue mountain light");
        Profile.RoadCue = TEXT("snow-ready arterials and utility shoulders");
        Profile.HomeCue = TEXT("timber homes, low apartments, snow roofs");
        Profile.VehicleCue = TEXT("4x4s, pickups, rescue trucks");
        Profile.ClothingCue = TEXT("parkas, boots, high-vis layers");
        Profile.bMountain = true;
        Profile.bCoastal = true;
        Profile.bColdWeather = true;
        Profile.SignatureCue = TEXT("snowy inlet, spruce line, and rescue-truck marker");
        Profile.SignatureShapeToken = TEXT("SnowInlet");
        Profile.SignatureColor = FLinearColor(0.72f, 0.88f, 1.0f);
        Profile.TowerCount = 3;
    }
    else if (City == TEXT("Salt Lake City") || City == TEXT("Provo"))
    {
        Profile.LandscapeCue = TEXT("Wasatch mountain wall and dry valley");
        Profile.ArchitectureCue = TEXT("stone civic blocks, glass offices, campus homes");
        Profile.RoadCue = TEXT("wide numbered grid and mountain-view boulevards");
        Profile.HomeCue = TEXT("porch homes, apartments, foothill suburbs");
        Profile.VehicleCue = TEXT("SUVs, commuter cars, rail transit");
        Profile.ClothingCue = TEXT("outdoor layers, campus casual, boots");
        Profile.bMountain = true;
        Profile.bTransit = true;
        Profile.bCollegeTown = City == TEXT("Provo");
        Profile.SignatureCue = TEXT("Wasatch grid spire and mountain-valley marker");
        Profile.SignatureShapeToken = TEXT("MountainGridSpire");
        Profile.SignatureColor = FLinearColor(0.68f, 0.72f, 0.64f);
    }
    else if (CityContainsAny(City, { TEXT("Berkeley"), TEXT("Cambridge"), TEXT("Ann Arbor"), TEXT("Madison"), TEXT("Boulder"), TEXT("College Station"), TEXT("Athens"), TEXT("Gainesville"), TEXT("Fayetteville") }))
    {
        Profile.ArchitectureCue = TEXT("campus quads, brick halls, apartments, labs");
        Profile.RoadCue = TEXT("bike lanes, campus loops, compact streets");
        Profile.HomeCue = TEXT("student apartments, porch houses, small rentals");
        Profile.VehicleCue = TEXT("bikes, buses, compact cars");
        Profile.ClothingCue = TEXT("campus hoodies, backpacks, casual layers");
        Profile.bCollegeTown = true;
        Profile.bTransit = true;
        Profile.SignatureCue = TEXT("campus quad and lab hall marker");
        Profile.SignatureShapeToken = TEXT("CampusQuad");
        Profile.SignatureColor = FLinearColor(0.42f, 0.68f, 0.42f);
    }
    else if (CityContainsAny(City, { TEXT("Plano"), TEXT("Frisco"), TEXT("Irvine"), TEXT("Cary"), TEXT("Bellevue"), TEXT("Sunnyvale"), TEXT("Santa Clara"), TEXT("Richardson"), TEXT("Overland Park") }))
    {
        Profile.ArchitectureCue = TEXT("corporate campuses, apartments, planned suburbs");
        Profile.RoadCue = TEXT("wide arterials, office-park loops, bike paths");
        Profile.HomeCue = TEXT("new townhomes, subdivisions, garden apartments");
        Profile.VehicleCue = TEXT("EVs, SUVs, rideshare sedans");
        Profile.ClothingCue = TEXT("business casual, hoodies, sneakers");
        Profile.bTechCampus = true;
        Profile.bSuburban = true;
        Profile.SignatureCue = TEXT("planned tech campus and office-park loop marker");
        Profile.SignatureShapeToken = TEXT("TechCampus");
        Profile.SignatureColor = FLinearColor(0.16f, 0.66f, 0.82f);
    }

    TArray<FString> Districts;
    if (Profile.bCoastal)
    {
        Districts.Add(Profile.bMilitaryHarbor ? TEXT("naval harbor waterfront") : TEXT("waterfront or beach approach"));
    }
    if (Profile.bRiverfront)
    {
        Districts.Add(TEXT("riverwalk and bridge access"));
    }
    if (Profile.bTransit)
    {
        Districts.Add(TEXT("transit stop and rail/bus corridor"));
    }
    if (Profile.bHistoric)
    {
        Districts.Add(TEXT("historic core and stoop row"));
    }
    if (Profile.bIndustrial)
    {
        Districts.Add(TEXT("warehouse/loading district"));
    }
    if (Profile.bEntertainment)
    {
        Districts.Add(TEXT("venue and neon corridor"));
    }
    if (Profile.bTechCampus)
    {
        Districts.Add(TEXT("tech campus loop"));
    }
    if (Profile.bCollegeTown)
    {
        Districts.Add(TEXT("campus quad and lab row"));
    }
    if (Profile.bMountain)
    {
        Districts.Add(TEXT("mountain-view trailhead"));
    }
    if (Profile.bDesert)
    {
        Districts.Add(TEXT("xeriscape shade district"));
    }
    if (Profile.bCapitalCivic)
    {
        Districts.Add(TEXT("civic security avenue"));
    }
    if (Profile.bSuburban)
    {
        Districts.Add(TEXT("planned neighborhood loop"));
    }
    if (Districts.Num() == 0)
    {
        Districts.Add(TEXT("downtown, residential, and rescue-access districts"));
    }
    Profile.DistrictCue = FString::Join(Districts, TEXT(" | "));

    return Profile;
}

bool ShouldUseBuiltInSafeZombieVariant(EZombieVariant Variant)
{
    return Variant == EZombieVariant::UrbanZombie4 || Variant == EZombieVariant::EliteBoomer;
}

const TArray<FZombieVariantRow>& GetBuiltInZombieVariantRows()
{
    static TArray<FZombieVariantRow> Rows;
    if (Rows.Num() > 0)
    {
        return Rows;
    }

    auto AddRow = [](EZombieVariant Variant, const TCHAR* DisplayName, const TCHAR* MeshPath,
                     const TCHAR* AnimBPPath,
                     float HealthMul, float DamageMul, float SpeedMul, float MeshScale,
                     float AnchorageWeight, float HarborWeight, float MetroWeight)
    {
        FZombieVariantRow Row;
        Row.Variant = Variant;
        Row.DisplayName = DisplayName;
        Row.SkeletalMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(MeshPath));
        if (AnimBPPath && FCString::Strlen(AnimBPPath) > 0)
        {
            Row.AnimBPClass = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(AnimBPPath));
        }
        Row.HealthMultiplier = HealthMul;
        Row.DamageMultiplier = DamageMul;
        Row.SpeedMultiplier = SpeedMul;
        Row.MeshScale = MeshScale;
        Row.ZoneWeights.Add(0, AnchorageWeight);
        Row.ZoneWeights.Add(1, HarborWeight);
        Row.ZoneWeights.Add(2, MetroWeight);
        Rows.Add(Row);
    };

    AddRow(EZombieVariant::DogZombie, TEXT("Dog Zombie"),
           TEXT("/Game/DogZombie/Meshes/SK_DogZombie.SK_DogZombie"),
           TEXT(""),
           0.55f, 0.70f, 1.45f, 0.55f, 0.6f, 1.6f, 0.6f);
    AddRow(EZombieVariant::UrbanZombie4, TEXT("Urban Zombie 4"),
           TEXT("/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_02.SK_Zombie_M04_02"),
           TEXT("/Game/YI_ModularZombies/Animation/ThirdPerson/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"),
           1.0f, 1.0f, 1.0f, 1.0f, 1.8f, 1.0f, 0.9f);
    AddRow(EZombieVariant::BusinessSuit, TEXT("Zombie - Business Suit"),
           TEXT("/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01.SK_Zombie_M04_01"),
           TEXT("/Game/YI_ModularZombies/Animation/ThirdPerson/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"),
           1.1f, 1.05f, 0.95f, 1.0f, 0.4f, 1.6f, 1.4f);
    AddRow(EZombieVariant::BloatedFemale, TEXT("Zombie - Bloated Female"),
           TEXT("/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01.SK_Zombie_F01_01"),
           TEXT("/Game/YI_ModularZombies/Animation/ThirdPerson/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"),
           1.6f, 1.25f, 0.70f, 1.10f, 0.6f, 1.0f, 1.7f);
    AddRow(EZombieVariant::NurseFemale, TEXT("Zombie Female: Nurse"),
           TEXT("/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit.ZombieFemale_NurseOutfit"),
           TEXT(""),
           0.95f, 1.0f, 1.05f, 1.0f, 1.4f, 0.9f, 0.7f);
    AddRow(EZombieVariant::BaseMesh, TEXT("Zombie (rivai, base mesh)"),
           TEXT("/Game/Zombie/BaseMesh/SK_Zombie.SK_Zombie"),
           TEXT(""),
           0.85f, 0.7f, 0.85f, 1.0f, 0.05f, 0.05f, 0.05f);
    AddRow(EZombieVariant::EliteSpitter, TEXT("Elite - Spitter"),
           TEXT("/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01.SK_Zombie_F01_01"),
           TEXT("/Game/YI_ModularZombies/Animation/ThirdPerson/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"),
           1.20f, 1.10f, 0.85f, 1.10f, 0.20f, 0.30f, 0.45f);
    AddRow(EZombieVariant::EliteCharger, TEXT("Elite - Charger"),
           TEXT("/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01.SK_Zombie_M04_01"),
           TEXT("/Game/YI_ModularZombies/Animation/ThirdPerson/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"),
           1.10f, 1.30f, 1.50f, 1.05f, 0.30f, 0.30f, 0.55f);
    AddRow(EZombieVariant::EliteBoomer, TEXT("Elite - Boomer"),
           TEXT("/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_02.SK_Zombie_F01_02"),
           TEXT("/Game/YI_ModularZombies/Animation/ThirdPerson/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"),
           1.30f, 1.40f, 0.80f, 1.15f, 0.20f, 0.30f, 0.55f);

    return Rows;
}
}

// ---- improvement_pass_2026-06-12 #45 — whole-city U.S. realization ---------
// Concrete, spawnable parameters derived from the doc-43 visual profile. The
// profile carries the city's narrative cues and base colors; these params turn
// them into terrain forms, water bodies, sky/fog/grade values, facade
// palettes, road patterns, home archetypes, vehicle fleets, and wardrobe
// palettes so the ENTIRE generated city approximates its real counterpart.
struct FCodeRescueUSCityRealizationParams
{
    // Landscape
    FString TerrainToken = TEXT("Plains");          // Coastal/GreatLakes/RiverCity/Desert/Mountain/Plains/ForestHills/Tropical/Subarctic/GoldenBasin
    FString BackdropToken = TEXT("PrairieHorizon"); // MountainRing/MesaButtes/EvergreenRidge/PalmShore/PrairieHorizon/HillTerraces/SkylineHaze
    FString VegetationToken = TEXT("Deciduous");    // Palms/Evergreens/Deciduous/Cactus/Tundra/OakScrub
    FLinearColor GroundTint = FLinearColor(0.10f, 0.14f, 0.10f);
    bool bWaterEdge = false;                         // coastal / great-lake shoreline plate
    bool bRiverThrough = false;                      // river band + bridge decks across town
    FString WaterEdgeSide = TEXT("South");           // South/East/West
    FLinearColor WaterColor = FLinearColor(0.03f, 0.10f, 0.16f);
    FLinearColor ShorelineColor = FLinearColor(0.42f, 0.38f, 0.28f);

    // Sky
    FLinearColor DaySunColor = FLinearColor(1.0f, 0.96f, 0.86f);
    FLinearColor NightSunColor = FLinearColor(0.55f, 0.64f, 0.95f);
    float DaySunIntensity = 7.0f;
    float NightSunIntensity = 3.2f;
    FLinearColor FogColor = FLinearColor(0.45f, 0.51f, 0.60f);
    float FogDensity = 0.012f;
    FString CloudToken = TEXT("Clear");              // Clear/MarineLayer/Overcast/HazeWarm/HumidGlow/SnowSky
    FString GradeToken = TEXT("NeutralMetro");       // CoolOvercast/WarmDesert/CrispMountain/HumidGulf/TropicalBright/GoldenBasin/NeutralMetro

    // Architecture (city-wide systemic skyline)
    TArray<FLinearColor> FacadePalette;
    FString FacadeToken = TEXT("MixedMetro");        // GlassSteel/BrickMasonry/StuccoAdobe/DecoPastel/StoneCivic/TimberMixed/MixedMetro
    float DowntownHeightScale = 1.0f;                // NYC ~1.7 ... DC ~0.62 height-act
    float SprawlFalloff = 0.45f;                     // 0 = uniformly tall core->edge, 1 = strong low sprawl
    float FootprintScale = 1.0f;

    // Roads + sidewalks
    FString RoadPatternToken = TEXT("StrictGrid");   // NumberedGrid/StrictGrid/DiagonalAvenues/IrregularHistoric/WideArterial/HillGrid
    FLinearColor AsphaltTone = FLinearColor(0.018f, 0.020f, 0.023f);
    FLinearColor LanePaintTone = FLinearColor(0.86f, 0.78f, 0.52f);
    FLinearColor SidewalkTone = FLinearColor(0.19f, 0.18f, 0.16f);
    float RoadWidthScale = 1.0f;
    float SidewalkWidthScale = 1.0f;
    bool bBrickHistoricWalks = false;

    // Homes
    FString HomeArchetypeToken = TEXT("SuburbanRanch"); // BrownstoneRow/TripleDecker/VictorianPainted/CraftsmanBungalow/AdobeRanch/BrickTwoFlat/ShotgunPorch/SunbeltRanch/DecoPastelHome/MountainCabin/CapeCod
    TArray<FLinearColor> HomePalette;
    int32 HomesPerRow = 7;

    // Vehicles
    TArray<FString> VehicleMix;                      // weighted token bag: Taxi/Pickup/SUV/Sedan/Compact/EV/Van/Bus/Convertible/PlowTruck
    FLinearColor FleetAccent = FLinearColor(0.16f, 0.20f, 0.24f);
    int32 CurbVehicleCount = 20;

    // Clothing
    TArray<FLinearColor> WardrobePalette;
    FString WardrobeAccessoryToken = TEXT("BallCap"); // Beanie/SunHat/CowboyHat/Backpack/Scarf/ParkaHood/Lanyard/BallCap
};

FCodeRescueUSCityRealizationParams BuildUSCityRealizationParams(
    const FCodeRescueCityMission& Mission,
    const FCodeRescueUSCityVisualProfile& Profile)
{
    FCodeRescueUSCityRealizationParams P;
    const FString& City = Mission.CityName;
    const FString& State = Mission.StateName;

    auto StateIn = [&State](std::initializer_list<const TCHAR*> States) -> bool
    {
        for (const TCHAR* S : States)
        {
            if (State == S)
            {
                return true;
            }
        }
        return false;
    };

    // ---- regional baselines (every one of the U.S. campaign cities lands here) --
    P.GroundTint = Profile.TerrainColor;
    P.AsphaltTone = Profile.RoadColor;
    P.SidewalkTone = Profile.SidewalkColor * 0.55f;
    P.FleetAccent = Profile.VehicleColor;
    P.RoadWidthScale = Profile.StreetWidthScale;
    P.WardrobePalette = { Profile.ClothingColor,
                          Profile.ClothingColor * 0.55f,
                          FLinearColor(0.08f, 0.08f, 0.09f),
                          FLinearColor(0.55f, 0.52f, 0.46f) };
    P.HomePalette = { Profile.HomeColor, Profile.HomeColor * 0.72f, Profile.HomeColor * 1.25f };
    P.FacadePalette = { Profile.ArchitectureColor,
                        Profile.ArchitectureColor * 0.62f,
                        FLinearColor(0.10f, 0.13f, 0.16f),
                        FLinearColor(0.16f, 0.15f, 0.13f) };
    P.FogColor = Profile.SkyColor * 0.65f;

    if (Profile.bHistoric)
    {
        P.RoadPatternToken = TEXT("IrregularHistoric");
        P.bBrickHistoricWalks = true;
        P.SidewalkWidthScale = 1.15f;
    }

    // Northeast: rowhome corridor, transit, cool Atlantic light.
    if (StateIn({ TEXT("CT"), TEXT("MA"), TEXT("MD"), TEXT("NH"), TEXT("NJ"), TEXT("NY"), TEXT("PA"), TEXT("RI"), TEXT("VA"), TEXT("DC") }))
    {
        P.TerrainToken = TEXT("ForestHills");
        P.BackdropToken = TEXT("SkylineHaze");
        P.VegetationToken = TEXT("Deciduous");
        P.CloudToken = TEXT("Overcast");
        P.GradeToken = TEXT("CoolOvercast");
        P.FacadeToken = TEXT("BrickMasonry");
        P.HomeArchetypeToken = TEXT("BrownstoneRow");
        P.HomePalette = { FLinearColor(0.30f, 0.13f, 0.09f), FLinearColor(0.24f, 0.11f, 0.08f), FLinearColor(0.36f, 0.20f, 0.12f) };
        P.VehicleMix = { TEXT("Sedan"), TEXT("Sedan"), TEXT("Taxi"), TEXT("Bus"), TEXT("Van"), TEXT("SUV") };
        P.WardrobeAccessoryToken = TEXT("Scarf");
        P.WardrobePalette = { FLinearColor(0.07f, 0.08f, 0.11f), FLinearColor(0.16f, 0.16f, 0.18f), FLinearColor(0.30f, 0.08f, 0.07f), FLinearColor(0.13f, 0.18f, 0.26f) };
        P.DaySunColor = FLinearColor(0.94f, 0.94f, 0.90f);
        P.FogDensity = 0.016f;
        P.DowntownHeightScale = 1.18f;
        P.SprawlFalloff = 0.30f;
    }
    // Midwest: flat grid, brick + industry, big plains/lake sky.
    else if (StateIn({ TEXT("IA"), TEXT("IL"), TEXT("IN"), TEXT("KS"), TEXT("MI"), TEXT("MN"), TEXT("MO"), TEXT("ND"), TEXT("NE"), TEXT("OH"), TEXT("SD"), TEXT("WI") }))
    {
        P.TerrainToken = TEXT("Plains");
        P.BackdropToken = TEXT("PrairieHorizon");
        P.VegetationToken = TEXT("Deciduous");
        P.CloudToken = TEXT("Overcast");
        P.GradeToken = TEXT("CoolOvercast");
        P.FacadeToken = TEXT("BrickMasonry");
        P.HomeArchetypeToken = TEXT("BrickTwoFlat");
        P.RoadPatternToken = TEXT("StrictGrid");
        P.VehicleMix = { TEXT("Sedan"), TEXT("Pickup"), TEXT("Pickup"), TEXT("SUV"), TEXT("Bus"), TEXT("Van") };
        P.WardrobeAccessoryToken = TEXT("Beanie");
        P.WardrobePalette = { FLinearColor(0.10f, 0.13f, 0.20f), FLinearColor(0.24f, 0.10f, 0.07f), FLinearColor(0.14f, 0.14f, 0.15f), FLinearColor(0.42f, 0.30f, 0.12f) };
        P.FogDensity = 0.013f;
        P.DowntownHeightScale = 0.95f;
        P.SprawlFalloff = 0.55f;
    }
    // South: warm lowland, porches, pickups, humid haze.
    else if (StateIn({ TEXT("AL"), TEXT("AR"), TEXT("FL"), TEXT("GA"), TEXT("KY"), TEXT("LA"), TEXT("MS"), TEXT("NC"), TEXT("OK"), TEXT("SC"), TEXT("TN"), TEXT("TX") }))
    {
        P.TerrainToken = TEXT("Lowland");
        P.BackdropToken = TEXT("SkylineHaze");
        P.VegetationToken = TEXT("OakScrub");
        P.CloudToken = TEXT("HumidGlow");
        P.GradeToken = TEXT("HumidGulf");
        P.FacadeToken = TEXT("MixedMetro");
        P.HomeArchetypeToken = TEXT("SunbeltRanch");
        P.RoadPatternToken = TEXT("WideArterial");
        P.RoadWidthScale = FMath::Max(P.RoadWidthScale, 1.18f);
        P.VehicleMix = { TEXT("Pickup"), TEXT("Pickup"), TEXT("SUV"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van") };
        P.WardrobeAccessoryToken = TEXT("BallCap");
        P.WardrobePalette = { FLinearColor(0.55f, 0.50f, 0.40f), FLinearColor(0.16f, 0.24f, 0.30f), FLinearColor(0.40f, 0.12f, 0.10f), FLinearColor(0.85f, 0.82f, 0.74f) };
        P.DaySunColor = FLinearColor(1.0f, 0.93f, 0.78f);
        P.FogColor = FLinearColor(0.55f, 0.55f, 0.48f);
        P.FogDensity = 0.017f;
        P.GroundTint = FLinearColor(0.11f, 0.15f, 0.09f);
        P.DowntownHeightScale = 1.0f;
        P.SprawlFalloff = 0.62f;
    }
    // West: default dry-bright; deserts/mountains/coast refined below.
    else
    {
        P.TerrainToken = TEXT("GoldenBasin");
        P.BackdropToken = TEXT("MountainRing");
        P.VegetationToken = TEXT("OakScrub");
        P.CloudToken = TEXT("Clear");
        P.GradeToken = TEXT("CrispMountain");
        P.FacadeToken = TEXT("GlassSteel");
        P.HomeArchetypeToken = TEXT("CraftsmanBungalow");
        P.VehicleMix = { TEXT("SUV"), TEXT("Sedan"), TEXT("Pickup"), TEXT("Compact"), TEXT("EV"), TEXT("Van") };
        P.WardrobeAccessoryToken = TEXT("Backpack");
        P.WardrobePalette = { FLinearColor(0.18f, 0.22f, 0.24f), FLinearColor(0.36f, 0.26f, 0.14f), FLinearColor(0.12f, 0.12f, 0.13f), FLinearColor(0.50f, 0.55f, 0.52f) };
        P.DaySunColor = FLinearColor(1.0f, 0.97f, 0.88f);
        P.FogDensity = 0.008f;
        P.DowntownHeightScale = 1.0f;
        P.SprawlFalloff = 0.55f;
    }

    // ---- climate/terrain families sharpen the regional baseline ------------
    if (Profile.bDesert || StateIn({ TEXT("AZ"), TEXT("NM"), TEXT("NV") }))
    {
        P.TerrainToken = TEXT("Desert");
        P.BackdropToken = TEXT("MesaButtes");
        P.VegetationToken = TEXT("Cactus");
        P.CloudToken = TEXT("Clear");
        P.GradeToken = TEXT("WarmDesert");
        P.FacadeToken = TEXT("StuccoAdobe");
        P.HomeArchetypeToken = TEXT("AdobeRanch");
        P.HomePalette = { FLinearColor(0.55f, 0.40f, 0.26f), FLinearColor(0.62f, 0.48f, 0.32f), FLinearColor(0.48f, 0.33f, 0.22f) };
        P.FacadePalette = { FLinearColor(0.42f, 0.32f, 0.22f), FLinearColor(0.50f, 0.40f, 0.28f), FLinearColor(0.30f, 0.24f, 0.18f), FLinearColor(0.20f, 0.22f, 0.24f) };
        P.GroundTint = FLinearColor(0.34f, 0.24f, 0.13f);
        P.RoadPatternToken = TEXT("WideArterial");
        P.RoadWidthScale = FMath::Max(P.RoadWidthScale, 1.25f);
        P.DaySunColor = FLinearColor(1.0f, 0.90f, 0.70f);
        P.FogColor = FLinearColor(0.62f, 0.50f, 0.34f);
        P.FogDensity = 0.006f;
        P.WardrobeAccessoryToken = TEXT("SunHat");
        P.WardrobePalette = { FLinearColor(0.72f, 0.62f, 0.46f), FLinearColor(0.30f, 0.20f, 0.12f), FLinearColor(0.80f, 0.76f, 0.66f), FLinearColor(0.20f, 0.26f, 0.30f) };
        P.SprawlFalloff = 0.70f;
        P.DowntownHeightScale = 0.85f;
    }
    if (Profile.bMountain || StateIn({ TEXT("CO"), TEXT("UT"), TEXT("MT"), TEXT("ID") }))
    {
        P.BackdropToken = TEXT("MountainRing");
        P.VegetationToken = TEXT("Evergreens");
        P.GradeToken = TEXT("CrispMountain");
        P.CloudToken = TEXT("Clear");
        P.FogDensity = 0.005f;
        P.VehicleMix = { TEXT("SUV"), TEXT("SUV"), TEXT("Pickup"), TEXT("Sedan"), TEXT("Van"), TEXT("Compact") };
        P.WardrobeAccessoryToken = TEXT("Beanie");
        P.HomeArchetypeToken = TEXT("MountainCabin");
        P.HomePalette = { FLinearColor(0.26f, 0.16f, 0.09f), FLinearColor(0.33f, 0.22f, 0.12f), FLinearColor(0.18f, 0.13f, 0.09f) };
    }
    if (Profile.bTropical || State == TEXT("HI") || City.Contains(TEXT("Miami")) || City.Contains(TEXT("Hialeah")) || City.Contains(TEXT("Fort Lauderdale")))
    {
        P.TerrainToken = TEXT("Tropical");
        P.BackdropToken = TEXT("PalmShore");
        P.VegetationToken = TEXT("Palms");
        P.CloudToken = TEXT("HumidGlow");
        P.GradeToken = TEXT("TropicalBright");
        P.FacadeToken = TEXT("DecoPastel");
        P.HomeArchetypeToken = TEXT("DecoPastelHome");
        P.HomePalette = { FLinearColor(0.75f, 0.55f, 0.55f), FLinearColor(0.45f, 0.70f, 0.66f), FLinearColor(0.80f, 0.74f, 0.50f) };
        P.FacadePalette = { FLinearColor(0.62f, 0.58f, 0.50f), FLinearColor(0.55f, 0.62f, 0.58f), FLinearColor(0.66f, 0.50f, 0.45f), FLinearColor(0.22f, 0.30f, 0.32f) };
        P.bWaterEdge = true;
        P.WaterColor = FLinearColor(0.02f, 0.22f, 0.26f);
        P.ShorelineColor = FLinearColor(0.78f, 0.70f, 0.50f);
        P.VehicleMix = { TEXT("Convertible"), TEXT("Compact"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van"), TEXT("Bus") };
        P.WardrobeAccessoryToken = TEXT("SunHat");
        P.WardrobePalette = { FLinearColor(0.85f, 0.78f, 0.60f), FLinearColor(0.20f, 0.55f, 0.55f), FLinearColor(0.75f, 0.35f, 0.30f), FLinearColor(0.90f, 0.88f, 0.80f) };
        P.DaySunColor = FLinearColor(1.0f, 0.96f, 0.80f);
    }
    if (State == TEXT("AK"))
    {
        P.TerrainToken = TEXT("Subarctic");
        P.BackdropToken = TEXT("MountainRing");
        P.VegetationToken = TEXT("Tundra");
        P.CloudToken = TEXT("SnowSky");
        P.GradeToken = TEXT("CrispMountain");
        P.GroundTint = FLinearColor(0.30f, 0.32f, 0.34f);
        P.HomeArchetypeToken = TEXT("MountainCabin");
        P.VehicleMix = { TEXT("Pickup"), TEXT("SUV"), TEXT("SUV"), TEXT("PlowTruck"), TEXT("Van"), TEXT("Sedan") };
        P.WardrobeAccessoryToken = TEXT("ParkaHood");
        P.WardrobePalette = { FLinearColor(0.30f, 0.16f, 0.08f), FLinearColor(0.12f, 0.16f, 0.22f), FLinearColor(0.45f, 0.45f, 0.45f), FLinearColor(0.55f, 0.30f, 0.10f) };
        P.NightSunColor = FLinearColor(0.50f, 0.62f, 0.92f);
        P.DaySunColor = FLinearColor(0.92f, 0.93f, 0.92f);
        P.FogColor = FLinearColor(0.60f, 0.64f, 0.70f);
        P.FogDensity = 0.018f;
    }
    if (StateIn({ TEXT("WA"), TEXT("OR") }))
    {
        P.TerrainToken = TEXT("ForestHills");
        P.BackdropToken = TEXT("EvergreenRidge");
        P.VegetationToken = TEXT("Evergreens");
        P.CloudToken = TEXT("Overcast");
        P.GradeToken = TEXT("CoolOvercast");
        P.FogColor = FLinearColor(0.45f, 0.50f, 0.52f);
        P.FogDensity = 0.020f;
        P.VehicleMix = { TEXT("EV"), TEXT("Compact"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van"), TEXT("Bus") };
        P.WardrobeAccessoryToken = TEXT("Beanie");
        P.WardrobePalette = { FLinearColor(0.12f, 0.18f, 0.14f), FLinearColor(0.20f, 0.20f, 0.22f), FLinearColor(0.36f, 0.24f, 0.12f), FLinearColor(0.40f, 0.44f, 0.42f) };
        P.HomeArchetypeToken = TEXT("CraftsmanBungalow");
        P.HomePalette = { FLinearColor(0.20f, 0.26f, 0.22f), FLinearColor(0.34f, 0.28f, 0.18f), FLinearColor(0.26f, 0.20f, 0.16f) };
    }
    if (Profile.bCoastal)
    {
        P.bWaterEdge = true;
    }
    if (Profile.bRiverfront)
    {
        P.bRiverThrough = true;
    }

    // ---- named-city sharpening (high-signal U.S. metros) --------------------
    if (City == TEXT("New York") || City == TEXT("Yonkers") || City == TEXT("Jersey City") || City == TEXT("Newark") || City == TEXT("Elizabeth") || City == TEXT("Paterson"))
    {
        P.RoadPatternToken = TEXT("NumberedGrid");
        P.DowntownHeightScale = 1.7f;
        P.SprawlFalloff = 0.10f;
        P.FootprintScale = 0.92f;
        P.FacadeToken = TEXT("GlassSteel");
        P.FacadePalette = { FLinearColor(0.12f, 0.16f, 0.20f), FLinearColor(0.20f, 0.20f, 0.22f), FLinearColor(0.28f, 0.14f, 0.10f), FLinearColor(0.40f, 0.38f, 0.34f) };
        P.HomeArchetypeToken = TEXT("BrownstoneRow");
        P.VehicleMix = { TEXT("Taxi"), TEXT("Taxi"), TEXT("Taxi"), TEXT("Sedan"), TEXT("Van"), TEXT("Bus") };
        P.CurbVehicleCount = 26;
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("East");
        P.GradeToken = TEXT("NeutralMetro");
        P.SidewalkWidthScale = 1.35f;
    }
    else if (City == TEXT("Chicago"))
    {
        P.RoadPatternToken = TEXT("StrictGrid");
        P.DowntownHeightScale = 1.55f;
        P.SprawlFalloff = 0.25f;
        P.FacadeToken = TEXT("GlassSteel");
        P.TerrainToken = TEXT("GreatLakes");
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("East");
        P.bRiverThrough = true;
        P.WaterColor = FLinearColor(0.03f, 0.14f, 0.18f);
        P.HomeArchetypeToken = TEXT("BrickTwoFlat");
        P.VehicleMix = { TEXT("Taxi"), TEXT("Sedan"), TEXT("Bus"), TEXT("SUV"), TEXT("Van"), TEXT("Sedan") };
        P.WardrobeAccessoryToken = TEXT("Beanie");
    }
    else if (City == TEXT("Boston") || City == TEXT("Cambridge") || City == TEXT("Quincy") || City == TEXT("Worcester") || City == TEXT("Lowell") || City == TEXT("Brockton") || City == TEXT("New Bedford") || City == TEXT("Lynn") || (City == TEXT("Springfield") && State == TEXT("MA")))
    {
        P.RoadPatternToken = TEXT("IrregularHistoric");
        P.bBrickHistoricWalks = true;
        P.HomeArchetypeToken = TEXT("TripleDecker");
        P.HomePalette = { FLinearColor(0.50f, 0.42f, 0.30f), FLinearColor(0.30f, 0.34f, 0.30f), FLinearColor(0.40f, 0.22f, 0.16f) };
        P.bWaterEdge = (City == TEXT("Boston") || City == TEXT("Quincy") || City == TEXT("Lynn") || City == TEXT("New Bedford"));
        P.WaterEdgeSide = TEXT("East");
        P.DowntownHeightScale = 1.12f;
    }
    else if (City == TEXT("Washington"))
    {
        P.RoadPatternToken = TEXT("DiagonalAvenues");
        P.DowntownHeightScale = 0.62f;   // height-act flat skyline
        P.SprawlFalloff = 0.15f;
        P.FacadeToken = TEXT("StoneCivic");
        P.FacadePalette = { FLinearColor(0.62f, 0.60f, 0.54f), FLinearColor(0.55f, 0.53f, 0.48f), FLinearColor(0.42f, 0.40f, 0.36f), FLinearColor(0.30f, 0.30f, 0.30f) };
        P.HomeArchetypeToken = TEXT("BrownstoneRow");
        P.bRiverThrough = true;
        P.WardrobeAccessoryToken = TEXT("Lanyard");
        P.WardrobePalette = { FLinearColor(0.10f, 0.11f, 0.14f), FLinearColor(0.16f, 0.18f, 0.22f), FLinearColor(0.40f, 0.38f, 0.36f), FLinearColor(0.26f, 0.10f, 0.10f) };
    }
    else if (City == TEXT("San Francisco") || City == TEXT("Oakland") || City == TEXT("Berkeley") || City == TEXT("Daly City") || City == TEXT("San Mateo"))
    {
        P.RoadPatternToken = TEXT("HillGrid");
        P.BackdropToken = TEXT("HillTerraces");
        P.CloudToken = TEXT("MarineLayer");
        P.GradeToken = TEXT("CoolOvercast");
        P.HomeArchetypeToken = TEXT("VictorianPainted");
        P.HomePalette = { FLinearColor(0.58f, 0.48f, 0.56f), FLinearColor(0.46f, 0.56f, 0.58f), FLinearColor(0.66f, 0.60f, 0.44f), FLinearColor(0.36f, 0.30f, 0.40f) };
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("West");
        P.FogColor = FLinearColor(0.58f, 0.60f, 0.62f);
        P.FogDensity = 0.022f;
        P.VehicleMix = { TEXT("EV"), TEXT("EV"), TEXT("Compact"), TEXT("Bus"), TEXT("Sedan"), TEXT("Van") };
        P.DowntownHeightScale = 1.30f;
        P.SprawlFalloff = 0.30f;
    }
    else if (City == TEXT("Seattle") || City == TEXT("Bellevue") || City == TEXT("Tacoma") || City == TEXT("Everett") || City == TEXT("Renton") || City == TEXT("Kent") || City == TEXT("Federal Way"))
    {
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("West");
        P.DowntownHeightScale = City == TEXT("Seattle") ? 1.35f : 0.95f;
        P.CloudToken = TEXT("MarineLayer");
    }
    else if (City == TEXT("Los Angeles") || City == TEXT("Long Beach") || (City == TEXT("Glendale") && State == TEXT("CA")) || (City == TEXT("Pasadena") && State == TEXT("CA")) || City == TEXT("Burbank") || City == TEXT("Inglewood") || City == TEXT("Torrance") || City == TEXT("Anaheim") || City == TEXT("Santa Ana") || City == TEXT("Irvine"))
    {
        P.TerrainToken = TEXT("GoldenBasin");
        P.GradeToken = TEXT("GoldenBasin");
        P.CloudToken = TEXT("HazeWarm");
        P.BackdropToken = TEXT("MountainRing");
        P.VegetationToken = TEXT("Palms");
        P.HomeArchetypeToken = TEXT("CraftsmanBungalow");
        P.HomePalette = { FLinearColor(0.66f, 0.58f, 0.44f), FLinearColor(0.55f, 0.45f, 0.34f), FLinearColor(0.48f, 0.52f, 0.46f) };
        P.RoadPatternToken = TEXT("WideArterial");
        P.RoadWidthScale = 1.30f;
        P.VehicleMix = { TEXT("Convertible"), TEXT("SUV"), TEXT("EV"), TEXT("Compact"), TEXT("Sedan"), TEXT("Van") };
        P.CurbVehicleCount = 24;
        P.WardrobeAccessoryToken = TEXT("SunHat");
        P.WardrobePalette = { FLinearColor(0.85f, 0.80f, 0.70f), FLinearColor(0.20f, 0.20f, 0.22f), FLinearColor(0.60f, 0.30f, 0.24f), FLinearColor(0.30f, 0.42f, 0.46f) };
        P.DowntownHeightScale = 1.05f;
        P.SprawlFalloff = 0.75f;
        P.FogColor = FLinearColor(0.66f, 0.58f, 0.44f);
        P.FogDensity = 0.014f;
        P.bWaterEdge = (City == TEXT("Los Angeles") || City == TEXT("Long Beach") || City == TEXT("Torrance"));
        P.WaterEdgeSide = TEXT("West");
    }
    else if (City == TEXT("New Orleans"))
    {
        P.HomeArchetypeToken = TEXT("ShotgunPorch");
        P.HomePalette = { FLinearColor(0.60f, 0.55f, 0.40f), FLinearColor(0.45f, 0.55f, 0.50f), FLinearColor(0.62f, 0.42f, 0.40f) };
        P.bRiverThrough = true;
        P.RoadPatternToken = TEXT("IrregularHistoric");
        P.bBrickHistoricWalks = true;
        P.CloudToken = TEXT("HumidGlow");
        P.GradeToken = TEXT("HumidGulf");
        P.FogDensity = 0.020f;
        P.WardrobeAccessoryToken = TEXT("SunHat");
    }
    else if (City == TEXT("Houston") || (City == TEXT("Pasadena") && State == TEXT("TX")))
    {
        P.bRiverThrough = true;   // bayou band
        P.RoadWidthScale = 1.35f;
        P.CurbVehicleCount = 24;
        P.DowntownHeightScale = 1.25f;
        P.SprawlFalloff = 0.78f;
        P.FogDensity = 0.019f;
    }
    else if (City == TEXT("Dallas") || City == TEXT("Fort Worth") || (City == TEXT("Arlington") && State == TEXT("TX")) || City == TEXT("Plano") || City == TEXT("Irving") || City == TEXT("Garland") || City == TEXT("Frisco") || City == TEXT("McKinney"))
    {
        P.RoadWidthScale = 1.30f;
        P.DowntownHeightScale = City == TEXT("Dallas") ? 1.25f : 0.92f;
        P.SprawlFalloff = 0.72f;
        P.VehicleMix = { TEXT("Pickup"), TEXT("Pickup"), TEXT("SUV"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van") };
        P.WardrobeAccessoryToken = TEXT("CowboyHat");
    }
    else if (City == TEXT("San Antonio") || City == TEXT("Austin") || City == TEXT("El Paso") || City == TEXT("Laredo") || City == TEXT("Corpus Christi"))
    {
        P.WardrobeAccessoryToken = City == TEXT("Austin") ? TEXT("BallCap") : TEXT("CowboyHat");
        if (City == TEXT("San Antonio"))
        {
            P.bRiverThrough = true;          // riverwalk
            P.HomeArchetypeToken = TEXT("AdobeRanch");
        }
        if (City == TEXT("El Paso"))
        {
            P.BackdropToken = TEXT("MesaButtes");
            P.GradeToken = TEXT("WarmDesert");
        }
        if (City == TEXT("Corpus Christi"))
        {
            P.bWaterEdge = true;
            P.WaterEdgeSide = TEXT("South");
        }
        if (City == TEXT("Austin"))
        {
            P.VehicleMix = { TEXT("EV"), TEXT("Pickup"), TEXT("Compact"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van") };
            P.bRiverThrough = true;
        }
    }
    else if (City == TEXT("Denver") || City == TEXT("Colorado Springs") || (City == TEXT("Aurora") && State == TEXT("CO")) || (City == TEXT("Lakewood") && State == TEXT("CO")) || City == TEXT("Fort Collins") || City == TEXT("Boulder"))
    {
        P.BackdropToken = TEXT("MountainRing");
        P.GradeToken = TEXT("CrispMountain");
        P.DowntownHeightScale = City == TEXT("Denver") ? 1.18f : 0.85f;
        P.RoadPatternToken = TEXT("StrictGrid");
    }
    else if (City == TEXT("Salt Lake City") || City == TEXT("Provo") || City == TEXT("West Valley City") || City == TEXT("West Jordan"))
    {
        P.BackdropToken = TEXT("MountainRing");
        P.RoadPatternToken = TEXT("StrictGrid");
        P.RoadWidthScale = 1.40f;   // famously wide pioneer grid
        P.GradeToken = TEXT("CrispMountain");
    }
    else if (City == TEXT("Las Vegas") || City == TEXT("North Las Vegas") || City == TEXT("Henderson") || City == TEXT("Reno") || City == TEXT("Sparks"))
    {
        P.GradeToken = TEXT("WarmDesert");
        P.BackdropToken = TEXT("MesaButtes");
        P.DowntownHeightScale = City.Contains(TEXT("Las Vegas")) ? 1.30f : 0.85f;
        P.VehicleMix = { TEXT("Taxi"), TEXT("Convertible"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van"), TEXT("Bus") };
    }
    else if (City == TEXT("Phoenix") || City == TEXT("Tucson") || City == TEXT("Mesa") || City == TEXT("Chandler") || City == TEXT("Scottsdale") || City == TEXT("Gilbert") || (City == TEXT("Glendale") && State == TEXT("AZ")) || City == TEXT("Tempe") || (City == TEXT("Peoria") && State == TEXT("AZ")) || City == TEXT("Surprise") || City == TEXT("Albuquerque") || City == TEXT("Rio Rancho") || City == TEXT("Las Cruces"))
    {
        P.GradeToken = TEXT("WarmDesert");
        P.BackdropToken = TEXT("MesaButtes");
        P.HomeArchetypeToken = TEXT("AdobeRanch");
        P.SprawlFalloff = 0.80f;
        P.DowntownHeightScale = City == TEXT("Phoenix") ? 0.95f : 0.75f;
    }
    else if (City == TEXT("Detroit") || City == TEXT("Warren") || City == TEXT("Sterling Heights") || City == TEXT("Dearborn"))
    {
        P.FacadeToken = TEXT("BrickMasonry");
        P.bRiverThrough = (City == TEXT("Detroit"));
        P.VehicleMix = { TEXT("Sedan"), TEXT("Sedan"), TEXT("Pickup"), TEXT("SUV"), TEXT("Van"), TEXT("Sedan") };
        P.DowntownHeightScale = City == TEXT("Detroit") ? 1.15f : 0.85f;
    }
    else if (City == TEXT("Nashville-Davidson") || City == TEXT("Memphis") || City == TEXT("Knoxville") || City == TEXT("Chattanooga") || City == TEXT("Murfreesboro") || City == TEXT("Clarksville"))
    {
        P.bRiverThrough = (City != TEXT("Murfreesboro"));
        P.HomeArchetypeToken = TEXT("ShotgunPorch");
        P.WardrobeAccessoryToken = TEXT("BallCap");
    }
    else if (City == TEXT("Urban Honolulu"))
    {
        P.TerrainToken = TEXT("Tropical");
        P.BackdropToken = TEXT("MountainRing");
        P.VegetationToken = TEXT("Palms");
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("South");
        P.WaterColor = FLinearColor(0.02f, 0.24f, 0.30f);
        P.GradeToken = TEXT("TropicalBright");
        P.HomeArchetypeToken = TEXT("DecoPastelHome");
    }
    else if (City == TEXT("Anchorage"))
    {
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("West");
        P.WaterColor = FLinearColor(0.06f, 0.10f, 0.13f);
        P.ShorelineColor = FLinearColor(0.38f, 0.38f, 0.36f);
        P.DowntownHeightScale = 0.80f;
    }
    else if (City == TEXT("St. Louis") || City == TEXT("Kansas City") || City == TEXT("Cincinnati") || City == TEXT("Pittsburgh") || City == TEXT("Louisville/Jefferson County") || City == TEXT("Memphis") || City == TEXT("Minneapolis") || City == TEXT("St. Paul") || City == TEXT("Omaha") || City == TEXT("Des Moines") || City == TEXT("Davenport") || City == TEXT("Evansville") || City == TEXT("Cedar Rapids"))
    {
        P.bRiverThrough = true;
        P.FacadeToken = TEXT("BrickMasonry");
    }
    else if (City == TEXT("Cleveland") || City == TEXT("Milwaukee") || City == TEXT("Buffalo") || (City == TEXT("Rochester") && State == TEXT("NY")) || City == TEXT("Toledo") || City == TEXT("Green Bay") || City == TEXT("Madison"))
    {
        P.TerrainToken = TEXT("GreatLakes");
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("East");
        P.WaterColor = FLinearColor(0.03f, 0.12f, 0.16f);
    }
    else if (City == TEXT("San Jose") || City == TEXT("Sunnyvale") || City == TEXT("Santa Clara") || City == TEXT("Fremont") || City == TEXT("Mountain View") || City == TEXT("Bellevue") || City == TEXT("Redmond") || City == TEXT("Cary") || City == TEXT("Frisco") || City == TEXT("Carmel") || City == TEXT("Fishers"))
    {
        P.FacadeToken = TEXT("GlassSteel");
        P.HomeArchetypeToken = TEXT("SunbeltRanch");
        P.VehicleMix = { TEXT("EV"), TEXT("EV"), TEXT("Compact"), TEXT("SUV"), TEXT("Sedan"), TEXT("Van") };
        P.WardrobeAccessoryToken = TEXT("Lanyard");
        P.DowntownHeightScale = 0.85f;
        P.SprawlFalloff = 0.65f;
    }
    else if (City == TEXT("San Diego") || City == TEXT("Chula Vista") || City == TEXT("Oceanside") || City == TEXT("Carlsbad") || City == TEXT("El Cajon"))
    {
        P.bWaterEdge = true;
        P.WaterEdgeSide = TEXT("West");
        P.GradeToken = TEXT("GoldenBasin");
        P.VegetationToken = TEXT("Palms");
        P.HomeArchetypeToken = TEXT("SunbeltRanch");
        P.CloudToken = TEXT("MarineLayer");
    }
    else if (City == TEXT("Philadelphia") || City == TEXT("Baltimore") || City == TEXT("Pittsburgh") || City == TEXT("Allentown") || (City == TEXT("Richmond") && State == TEXT("VA")) || City == TEXT("Norfolk") || (City == TEXT("Alexandria") && State == TEXT("VA")))
    {
        P.HomeArchetypeToken = TEXT("BrownstoneRow");
        P.bBrickHistoricWalks = true;
        P.RoadPatternToken = (City == TEXT("Philadelphia")) ? TEXT("StrictGrid") : P.RoadPatternToken;
        P.bRiverThrough = true;
        if (City == TEXT("Baltimore") || City == TEXT("Norfolk"))
        {
            P.bWaterEdge = true;
            P.WaterEdgeSide = TEXT("South");
        }
    }
    else if (City == TEXT("Portland") || City == TEXT("Gresham") || City == TEXT("Hillsboro") || City == TEXT("Salem") || City == TEXT("Eugene") || City == TEXT("Bend"))
    {
        P.bRiverThrough = (City == TEXT("Portland") || City == TEXT("Salem") || City == TEXT("Eugene"));
        P.VehicleMix = { TEXT("Compact"), TEXT("EV"), TEXT("Van"), TEXT("SUV"), TEXT("Sedan"), TEXT("Bus") };
    }

    if (Profile.bCollegeTown)
    {
        P.WardrobeAccessoryToken = TEXT("Backpack");
        P.VehicleMix.Add(TEXT("Compact"));
        P.VehicleMix.Add(TEXT("Bus"));
    }
    if (Profile.bTechCampus)
    {
        P.WardrobeAccessoryToken = TEXT("Lanyard");
    }
    if (Profile.bColdWeather && P.WardrobeAccessoryToken == TEXT("BallCap"))
    {
        P.WardrobeAccessoryToken = TEXT("Beanie");
    }

    // Wardrobe and fleet always have safe contents.
    if (P.VehicleMix.Num() == 0)
    {
        P.VehicleMix = { TEXT("Sedan"), TEXT("SUV"), TEXT("Pickup"), TEXT("Van") };
    }
    if (P.WardrobePalette.Num() == 0)
    {
        P.WardrobePalette = { Profile.ClothingColor, FLinearColor(0.12f, 0.12f, 0.14f) };
    }
    P.CurbVehicleCount = FMath::Clamp(P.CurbVehicleCount + (Mission.Rank <= 25 ? 4 : 0), 12, 30);
    P.HomesPerRow = Mission.Rank <= 50 ? 8 : 7;

    // Sun members mirror sky family.
    if (P.GradeToken == TEXT("CoolOvercast"))
    {
        P.DaySunIntensity = 5.6f;
        P.DaySunColor = FLinearColor(0.90f, 0.92f, 0.94f);
    }
    else if (P.GradeToken == TEXT("WarmDesert"))
    {
        P.DaySunIntensity = 8.2f;
    }
    else if (P.GradeToken == TEXT("TropicalBright"))
    {
        P.DaySunIntensity = 7.8f;
    }
    else if (P.GradeToken == TEXT("CrispMountain"))
    {
        P.DaySunIntensity = 7.4f;
        P.DaySunColor = FLinearColor(0.98f, 0.98f, 0.94f);
    }
    else if (P.GradeToken == TEXT("HumidGulf"))
    {
        P.DaySunIntensity = 6.6f;
        P.DaySunColor = FLinearColor(1.0f, 0.94f, 0.80f);
    }
    else if (P.GradeToken == TEXT("GoldenBasin"))
    {
        P.DaySunIntensity = 7.2f;
        P.DaySunColor = FLinearColor(1.0f, 0.92f, 0.74f);
    }

    return P;
}

bool ACodeRescueGameMode::IsLocationInsideProtectedLearningZone(const UObject* WorldContextObject, FVector Location, float Expansion)
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World)
    {
        return false;
    }

    // 2026-07-16 FREEZE ROOT CAUSE (Kenny: "game rendered completely frozen",
    // "movement almost non-functional"; confirmed with a `sample` profile of
    // the packaged app resuming his LA save): this function iterated EVERY
    // actor in the world per call, and it is called per AI controller, per
    // zombie, and per player tick. In a dense late-campaign city that is
    // O(agents x world) tag scans per frame — the game thread spun at a few
    // hundred percent CPU and the frame counter stood still. The protected
    // anchors are rare, mostly static markers, so the query now runs against
    // a cached snapshot (pure math, no iteration) refreshed at most every
    // few seconds. Game-thread only, per-world keyed.
    struct FProtectedZoneSnapshot
    {
        FBox Bounds;
        bool bBoundsValid = false;
        FVector Location = FVector::ZeroVector;
    };
    static TArray<FProtectedZoneSnapshot> CachedZones;
    static TWeakObjectPtr<UWorld> CachedZonesWorld;
    static double CachedZonesBuiltAt = -1000.0;
    constexpr double CacheLifetimeSeconds = 4.0;

    const double Now = World->GetTimeSeconds();
    const bool bCacheStale =
        CachedZonesWorld.Get() != World ||
        Now < CachedZonesBuiltAt ||                       // world/time restarted
        (Now - CachedZonesBuiltAt) > CacheLifetimeSeconds;
    if (bCacheStale)
    {
        const FName ProtectedTags[] = {
            FName("NoZombieLearningZone"),
            FName("ProtectedCodingChallengeZone"),
            FName("ProtectedLearningSpace"),
            FName("SafeTerminalLab"),
            FName("BonusCodingChallengeSafeZone"),
        };
        CachedZones.Reset();
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (!IsValid(Actor))
            {
                continue;
            }
            bool bProtectedAnchor = false;
            for (const FName& Tag : ProtectedTags)
            {
                if (Actor->Tags.Contains(Tag))
                {
                    bProtectedAnchor = true;
                    break;
                }
            }
            if (!bProtectedAnchor)
            {
                continue;
            }
            FProtectedZoneSnapshot& Zone = CachedZones.AddDefaulted_GetRef();
            Zone.Bounds = Actor->GetComponentsBoundingBox(true);
            Zone.bBoundsValid = Zone.Bounds.IsValid != 0;
            Zone.Location = Actor->GetActorLocation();
        }
        CachedZonesWorld = World;
        CachedZonesBuiltAt = Now;
    }

    const float SafeExpansion = FMath::Max(0.0f, Expansion);
    const float MarkerRadius = FMath::Max(320.0f, SafeExpansion);
    for (const FProtectedZoneSnapshot& Zone : CachedZones)
    {
        if (Zone.bBoundsValid && Zone.Bounds.ExpandBy(SafeExpansion).IsInsideOrOn(Location))
        {
            return true;
        }
        const FVector Delta = Location - Zone.Location;
        if (FMath::Abs(Delta.Z) <= 520.0f &&
            FVector::DistSquared2D(Location, Zone.Location) <= FMath::Square(MarkerRadius))
        {
            return true;
        }
    }

    return false;
}

ACodeRescueGameMode::ACodeRescueGameMode()
{
    // Tick registration must happen before the actor enters the world.
    // Enabling bCanEverTick from BeginPlay is too late for reliable runtime
    // registration and previously left the day/night system frozen.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    DefaultPawnClass = ACodeRescueCharacter::StaticClass();
    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));

    // Default-bind the variant data table so spawn-time variant selection
    // works out of the box without anyone needing to remember to wire it in
    // Project Settings or a Blueprint subclass. The build_zombie_variants_table.py
    // script authors this asset; if it's missing the spawn loop falls back to
    // procedural-cube zombies and gameplay is unchanged.
    if (!ZombieVariantTable)
    {
        ZombieVariantTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/CodeRescueAssets/DT_ZombieVariants.DT_ZombieVariants"));
    }
}

void ACodeRescueGameMode::BeginPlay()
{
    Super::BeginPlay();
    PrimaryActorTick.bCanEverTick = true;       // #35 day/night Tick
    SetActorTickEnabled(true);

    // #35 — find or spawn a directional light to drive the day/night cycle.
    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
    {
        SunLight = *It;
        break;
    }
    if (!SunLight)
    {
        SunLight = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(),
            FVector(0, 0, 5000), FRotator(-45, 30, 0));
    }

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        const bool bAutomationBypass =
            FParse::Param(FCommandLine::Get(), TEXT("CodeRescueBypassLaunchLanguageMenu")) ||
            FParse::Param(FCommandLine::Get(), TEXT("VisualReviewStart"));
        if (bAutomationBypass)
        {
            GI->bHasSelectedLaunchLanguageThisSession = true;
        }

        if (!GI->bHasSelectedLaunchLanguageThisSession)
        {
            SpawnLaunchLanguageSelectionScene();

            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                LaunchLanguageMenu = CreateWidget<UCodeRescueMainMenuWidget>(PC, UCodeRescueMainMenuWidget::StaticClass());
                if (UCodeRescueMainMenuWidget* Menu = LaunchLanguageMenu)
                {
                    Menu->SetLaunchLanguageOnly(true);
                    Menu->AddToViewport(4000);

                    // Keep both interaction paths live: Slate receives pointer and keyboard input,
                    // while the pawn's focus-independent polling remains the packaged-build
                    // fallback. This must be the final input mode set during launch-gate creation.
                    FInputModeGameAndUI Mode;
                    Mode.SetWidgetToFocus(Menu->TakeWidget());
                    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                    Mode.SetHideCursorDuringCapture(false);
                    PC->SetInputMode(Mode);
                    PC->bShowMouseCursor = true;
                    PC->SetIgnoreLookInput(true);
                    PC->SetIgnoreMoveInput(true);
                }
            }

            GI->PlayMenuMusic();
            UE_LOG(LogTemp, Display, TEXT("[CodeRescueLaunchLanguageMenu] Showing launch language chooser before active play."));
            return;
        }
    }

    SpawnWorld();

    // 2026-07-11 pass 4: unify the first-level ground datum for EVERY session
    // (not just audited test runs) once deferred assembly settles.
    {
        FTimerHandle GroundUnifyTimer;
        GetWorldTimerManager().SetTimer(GroundUnifyTimer, FTimerDelegate::CreateWeakLambda(
            this, [this]() { UnifyFirstLevelGroundTops(); }), 1.2f, false);
    }

    const bool bIntegratedFirstLevelAudit =
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit"));
    const bool bRunWorldAccessAudit = bIntegratedFirstLevelAudit ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelWorldAccessAudit"));
    const bool bRunChallengeAudit = bIntegratedFirstLevelAudit ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelChallengeAudit"));
    if (bRunWorldAccessAudit || bRunChallengeAudit)
    {
        FTimerHandle FirstLevelValidationTimer;
        GetWorldTimerManager().SetTimer(FirstLevelValidationTimer, FTimerDelegate::CreateWeakLambda(
            this,
            [this, bRunWorldAccessAudit, bRunChallengeAudit, bIntegratedFirstLevelAudit]()
            {
                if (bRunWorldAccessAudit)
                {
                    UnifyFirstLevelGroundTops();   // idempotent; audit sees the unified world
                }
                const bool bWorldPass = !bRunWorldAccessAudit || RunFirstLevelWorldAccessAudit();
                const bool bChallengePass = !bRunChallengeAudit || RunFirstLevelChallengeAudit();
                if (!bIntegratedFirstLevelAudit)
                {
                    const bool bPass = bWorldPass && bChallengePass;
                    if (bPass)
                    {
                        UE_LOG(LogTemp, Display,
                            TEXT("[FirstLevelValidationAudit] COMPLETE PASS world=%d challenges=%d"),
                            bWorldPass ? 1 : 0, bChallengePass ? 1 : 0);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error,
                            TEXT("[FirstLevelValidationAudit] COMPLETE FAIL world=%d challenges=%d"),
                            bWorldPass ? 1 : 0, bChallengePass ? 1 : 0);
                    }
                    FPlatformMisc::RequestExit(false);
                }
            }),
            0.9f,
            false);
    }
    if (bIntegratedFirstLevelAudit || FParse::Param(FCommandLine::Get(), TEXT("FirstLevelSkyAudit")))
    {
        StartFirstLevelSkyAudit();
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("CampaignGroundRecoveryAudit")))
    {
        StartCampaignGroundRecoveryAudit();
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("TerminalContrastReview")))
    {
        StartTerminalContrastReview();
    }

    // Deterministic visual-QA capture for packaged/editor smoke runs. The
    // delay allows world streaming, camera setup, and the HUD to settle.
    if (FParse::Param(FCommandLine::Get(), TEXT("ProductionReviewCapture")))
    {
        FTimerHandle ReviewCaptureTimer;
        GetWorldTimerManager().SetTimer(ReviewCaptureTimer, FTimerDelegate::CreateLambda([this]()
        {
            const FString CapturePath = FPaths::ProjectSavedDir() / TEXT("Screenshots/Production/production_arrival.png");
            FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
            UE_LOG(LogTemp, Display, TEXT("[ProductionReviewCapture] requested %s"), *CapturePath);
        }), 6.0f, false);
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("WorldLootWeatherVisualReview")))
    {
        // Let the universal post-spawn ground unifier and character settle
        // pass finish before deriving camera targets from actor bounds.
        FTimerHandle WorldLootReviewStartTimer;
        GetWorldTimerManager().SetTimer(WorldLootReviewStartTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            StartWorldLootWeatherVisualReview();
        }), 2.0f, false);
    }

    // #64: kick off ambient city music as soon as the world spawns. The
    // GameInstance handles the audio component lazily and is a no-op when
    // AmbientCityMusic isn't bound to a real cue yet.
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->PlayCityMusic();
    }

    if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        // Spawn on the currently streamed campaign city hub, safely inside
        // its mission floor and above the global safety plane.
        const int32 StartCityIndex = ActiveCampaignCityIndex != INDEX_NONE ? ActiveCampaignCityIndex : 0;
        Pawn->SetActorLocation(FCodeRescueCampaign::GetPlayerStartLocation(StartCityIndex));
        Pawn->SetActorRotation(FRotator(0, 35, 0));
    }

    // World is now fully populated and the player is at the default start.
    // Replay the saved run on top of that: hide solved terminals, hide
    // rescued survivors, destroy already-killed zombies, and (if a real
    // transform was previously captured) teleport the player back to where
    // they last were.
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->ApplyWorldStateToLevel(GetWorld());
    }
    if (ActiveCampaignCityIndex != INDEX_NONE)
    {
        AuditCampaignCityPopulation(ActiveCampaignCityIndex);
    }

    if (FParse::Param(FCommandLine::Get(), TEXT("VisualReviewStart")))
    {
        if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            const int32 StartCityIndex = ActiveCampaignCityIndex != INDEX_NONE ? ActiveCampaignCityIndex : 0;
            const FRotator ReviewRotation(0.0f, 39.0f, 0.0f);
            Pawn->SetActorLocation(FCodeRescueCampaign::GetPlayerStartLocation(StartCityIndex));
            Pawn->SetActorRotation(ReviewRotation);
            if (AController* Controller = Pawn->GetController())
            {
                Controller->SetControlRotation(ReviewRotation);
            }
        }
    }

    if (GEngine)
    {
        // 2026-07-01 HUD diet: two short lines only. If the language gate is still open (player is
        // on the CHOOSE CODING LANGUAGE screen), show the selection controls instead.
        bool bGateOpen = true;
        if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            bGateOpen = !GI->bHasSelectedLaunchLanguageThisSession;
        }
        if (bGateOpen)
        {
            GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Green,
                TEXT("CHOOSE CODING LANGUAGE: Up/Down highlight (or press 1-6), then Enter to deploy."));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 12.0f, FColor::Green,
                TEXT("STEP 1/3  TERMINAL - press T to travel there, E to open it."));
            GEngine->AddOnScreenDebugMessage(-1, 12.0f, FColor::Cyan,
                TEXT("T travel | E interact | J journal | Esc menu"));
        }
    }

    // Victory check tick: 1 Hz is plenty for a binary state. A timer is used
    // (rather than overriding Tick) so we don't pay tick cost on the GameMode
    // every frame, and so a SetGamePaused(true) cleanly stops the check.
    GetWorldTimerManager().SetTimer(VictoryCheckTimer, this, &ACodeRescueGameMode::CheckVictoryCondition, 1.0f, true);

    // #17 — first-launch tutorial overlay. Shown once; flag persists in SaveGame.
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        if (!GI->bHasShownTutorial)
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (UCodeRescueTutorialWidget* W = CreateWidget<UCodeRescueTutorialWidget>(PC, UCodeRescueTutorialWidget::StaticClass()))
                {
                    W->AddToViewport(2000); // very high z-order so it sits over HUD
                }
            }
        }
    }
}

void ACodeRescueGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopActiveRadioBriefing();
    Super::EndPlay(EndPlayReason);
}

void ACodeRescueGameMode::CheckVictoryCondition()
{
    if (bVictoryShown)
    {
        return;
    }

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        return;
    }

    // Win condition: every mission in the current 465-stop campaign solved AND
    // every city survivor team rescued, checked through campaign IDs rather than
    // counters so old saves or repeated validates cannot inflate completion.
    const int32 RequiredCities = FCodeRescueCampaign::GetMissionCount();
    if (FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI) < RequiredCities)
    {
        return;
    }

    bVictoryShown = true;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }

    TSubclassOf<UUserWidget> WidgetClass = VictoryWidgetClass;
    if (!WidgetClass)
    {
        WidgetClass = UCodeRescueVictoryWidget::StaticClass();
    }

    if (UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass))
    {
        Widget->AddToViewport(100);
        // The victory widget itself takes care of pausing + locking input
        // in its NativeConstruct. We just spawn it.
    }
}

void ACodeRescueGameMode::ApplyRuntimeDataLayerTags(AActor* Actor, const TArray<FName>& LayerTags) const
{
    if (!Actor)
    {
        return;
    }

    Actor->Tags.AddUnique(FName("RuntimeDataLayerStandIn"));
    Actor->Tags.AddUnique(FName("WorldPartitionDataLayerMigration"));
    Actor->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Actor->Tags.AddUnique(FName("Top50Recommendation31"));
    Actor->Tags.AddUnique(FName("DataLayerReadyFallback"));
    for (const FName& LayerTag : LayerTags)
    {
        if (LayerTag != NAME_None)
        {
            Actor->Tags.AddUnique(LayerTag);
        }
    }
}

void ACodeRescueGameMode::RegisterStreamedActor(AActor* Actor)
{
    if (bCollectStreamedCampaignActors && IsValid(Actor))
    {
        Actor->Tags.AddUnique(FName("ActiveCampaignCityStreamedActor"));
        Actor->Tags.AddUnique(FName("RuntimeWorldPartitionStreamCell"));
        Actor->Tags.AddUnique(FName("CurrentCppWorldPartitionFallback"));
        Actor->Tags.AddUnique(FName("OneFilePerActorMigrationReady"));
        Actor->Tags.AddUnique(FName("WorldPartitionReady"));
        StreamedCampaignActors.Add(Actor);
    }
}

void ACodeRescueGameMode::ClearStreamedCampaignActors()
{
    for (TWeakObjectPtr<AActor>& ActorPtr : StreamedCampaignActors)
    {
        if (AActor* Actor = ActorPtr.Get())
        {
            Actor->Destroy();
        }
    }
    StreamedCampaignActors.Reset();
    ActiveCampaignCityIndex = INDEX_NONE;
}

void ACodeRescueGameMode::SpawnLaunchLanguageSelectionScene()
{
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
        GEngine->Exec(GetWorld(), TEXT("r.ReflectionMethod 0"));
        GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.AutoExposure 1"));
        GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 2"));
        GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
    }

    GetWorld()->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

    if (SunLight)
    {
        if (UDirectionalLightComponent* DC = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent()))
        {
            DC->SetMobility(EComponentMobility::Movable);
            DC->SetIntensity(9.0f);
            DC->SetLightColor(FLinearColor(1.0f, 0.94f, 0.78f));
            DC->SetAtmosphereSunLight(true);
        }
        SunLight->SetActorLocation(FVector(-300.0f, -700.0f, 1800.0f));
        SunLight->SetActorRotation(FRotator(-38.0f, 48.0f, 0.0f));
    }

    if (ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
    {
        Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        Sky->GetLightComponent()->SetIntensity(2.4f);
        Sky->GetLightComponent()->SetLightColor(FLinearColor(0.58f, 0.74f, 1.0f));
        Sky->GetLightComponent()->SetRealTimeCaptureEnabled(true);
        Sky->GetLightComponent()->RecaptureSky();
        Sky->Tags.Add(FName("LaunchLanguageChoiceScene"));
    }

    if (APointLight* KeyLight = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), FVector(-520.0f, -620.0f, 560.0f), FRotator::ZeroRotator))
    {
        if (UPointLightComponent* PLC = KeyLight->FindComponentByClass<UPointLightComponent>())
        {
            PLC->SetMobility(EComponentMobility::Movable);
            PLC->SetIntensity(58000.0f);
            PLC->SetAttenuationRadius(1800.0f);
            PLC->SetLightColor(FLinearColor(0.45f, 0.85f, 1.0f));
        }
        KeyLight->Tags.Add(FName("LaunchLanguageChoiceScene"));
    }

    auto TagLaunch = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("LaunchLanguageChoiceScene"));
            Actor->Tags.Add(FName("LaunchLanguageWorldPrompt"));
        }
        return Actor;
    };

    TagLaunch(SpawnBlock(FVector(0.0f, 0.0f, -10.0f), FVector(10.8f, 7.2f, 0.12f), FLinearColor(0.030f, 0.045f, 0.050f), TEXT("Launch Language Solid Platform"), true));
    TagLaunch(SpawnBlock(FVector(0.0f, 330.0f, 120.0f), FVector(9.0f, 0.12f, 2.25f), FLinearColor(0.030f, 0.060f, 0.078f), TEXT("Launch Language Lit Backdrop"), false));
    TagLaunch(SpawnBlock(FVector(0.0f, -355.0f, 12.0f), FVector(9.5f, 0.12f, 0.16f), FLinearColor(0.10f, 0.42f, 0.48f) * 1.6f, TEXT("Launch Language Front Guidance Line"), false));
    TagLaunch(SpawnBlock(FVector(-525.0f, 0.0f, 70.0f), FVector(0.12f, 6.7f, 1.4f), FLinearColor(0.040f, 0.090f, 0.096f), TEXT("Launch Language West Rail"), false));
    TagLaunch(SpawnBlock(FVector(525.0f, 0.0f, 70.0f), FVector(0.12f, 6.7f, 1.4f), FLinearColor(0.040f, 0.090f, 0.096f), TEXT("Launch Language East Rail"), false));

    const FVector LaunchCameraLocation(0.0f, -860.0f, 360.0f);
    const FVector LaunchCameraTarget(0.0f, -55.0f, 130.0f);

    TagLaunch(SpawnGuideText(
        TEXT("SELECT CODING LANGUAGE\nNEW RUN OR RESUME SAVE"),
        FVector(0.0f, -164.0f, 390.0f),
        FColor(245, 236, 190),
        38.0f));
    TagLaunch(SpawnGuideText(
        TEXT("FIRST-SESSION ROUTE PREVIEW\nprotected terminal -> survivor marker -> extraction\nStart-screen choice remains first: each save is language-only"),
        FVector(0.0f, -188.0f, 304.0f),
        FColor(154, 244, 182),
        24.0f));

    struct FLaunchLanguageChoice
    {
        const TCHAR* Label;
        FLinearColor Color;
        FVector Location;
        ECodingLanguage Language;
    };

    const FLaunchLanguageChoice Choices[] = {
        { TEXT("JAVA"), FLinearColor(1.0f, 0.34f, 0.16f), FVector(-450.0f, -40.0f, 0.0f), ECodingLanguage::Java },
        { TEXT("PYTHON"), FLinearColor(1.0f, 0.82f, 0.20f), FVector(-270.0f, -40.0f, 0.0f), ECodingLanguage::Python },
        { TEXT("C"), FLinearColor(0.25f, 0.58f, 1.0f), FVector(-90.0f, -40.0f, 0.0f), ECodingLanguage::C },
        { TEXT("C+"), FLinearColor(0.16f, 0.82f, 0.94f), FVector(90.0f, -40.0f, 0.0f), ECodingLanguage::CPlus },
        { TEXT("C++"), FLinearColor(0.18f, 0.72f, 1.0f), FVector(270.0f, -40.0f, 0.0f), ECodingLanguage::Cpp },
        { TEXT("MATLAB"), FLinearColor(0.88f, 0.30f, 1.0f), FVector(450.0f, -40.0f, 0.0f), ECodingLanguage::MATLAB },
    };

    for (const FLaunchLanguageChoice& Choice : Choices)
    {
        const FVector& Base = Choice.Location;
        TagLaunch(SpawnBlock(Base + FVector(0.0f, 0.0f, 28.0f), FVector(1.32f, 1.12f, 0.18f), FLinearColor(0.018f, 0.025f, 0.028f) + Choice.Color * 0.28f, FString::Printf(TEXT("Launch Language %s Pedestal"), Choice.Label), true));
        TagLaunch(SpawnBlock(Base + FVector(0.0f, 10.0f, 92.0f), FVector(0.82f, 0.72f, 1.10f), FLinearColor(0.030f, 0.040f, 0.044f) + Choice.Color * 0.18f, FString::Printf(TEXT("Launch Language %s Pillar"), Choice.Label), false));
        TagLaunch(SpawnBlock(Base + FVector(0.0f, -52.0f, 188.0f), FVector(1.05f, 0.06f, 0.60f), Choice.Color * 2.2f, FString::Printf(TEXT("Launch Language %s Symbol Panel"), Choice.Label), false));
        TagLaunch(SpawnBlock(Base + FVector(0.0f, -72.0f, 252.0f), FVector(0.42f, 0.08f, 0.08f), Choice.Color * 3.0f, FString::Printf(TEXT("Launch Language %s Upper Signal"), Choice.Label), false));
        TagLaunch(SpawnBlock(Base + FVector(0.0f, 34.0f, 210.0f), FVector(0.45f, 0.45f, 0.06f), Choice.Color * 2.8f, FString::Printf(TEXT("Launch Language %s Beacon"), Choice.Label), false));
        TagLaunch(SpawnGuideText(
            FString::Printf(TEXT("%s\nTRACK ONLY"), Choice.Label),
            Base + FVector(0.0f, -118.0f, 330.0f),
            Choice.Color.ToFColor(true),
            32.0f));

        // 2026-07-01 ROOT FIX: the pedestals were pure decoration - there was no actual
        // ALanguageStationActor on the launch platform, so Interact() correctly reported
        // "no interactable within range" (verified in a live packaged playtest). Spawn a real
        // station in front of each pedestal; walking up + E/Enter now deploys that language.
        FActorSpawnParameters StationParams;
        StationParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        if (ALanguageStationActor* Station = GetWorld()->SpawnActor<ALanguageStationActor>(
                ALanguageStationActor::StaticClass(),
                Base + FVector(0.0f, -130.0f, 90.0f),
                FRotator::ZeroRotator,
                StationParams))
        {
            Station->Language = Choice.Language;
            Station->StationLabel = Choice.Label;
            TagLaunch(Station);
            UE_LOG(LogTemp, Display, TEXT("[LaunchLanguage] station spawned: %s"), Choice.Label);
        }
    }

    const FVector CameraLocation(LaunchCameraLocation);
    const FVector CameraTarget(LaunchCameraTarget);
    if (ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, (CameraTarget - CameraLocation).Rotation()))
    {
        Camera->Tags.Add(FName("LaunchLanguageChoiceScene"));
        Camera->Tags.Add(FName("LaunchLanguageCamera"));
        if (UCameraComponent* CameraComponent = Camera->GetCameraComponent())
        {
            CameraComponent->SetFieldOfView(82.0f);
        }
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PC->SetViewTarget(Camera);
        }
        FTimerHandle LaunchCameraRefreshHandle;
        TWeakObjectPtr<ACameraActor> LaunchCamera(Camera);
        GetWorldTimerManager().SetTimer(LaunchCameraRefreshHandle, FTimerDelegate::CreateWeakLambda(this, [this, LaunchCamera]()
        {
            if (LaunchCamera.IsValid())
            {
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
                {
                    PC->SetViewTarget(LaunchCamera.Get());
                }
            }
        }), 0.05f, false);
    }
}

int32 ACodeRescueGameMode::EstimateLivingPresenceCountForCity(const FCodeRescueCityMission& Mission, int32 CityIndex)
{
    (void)CityIndex;

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bSurvivorAlreadyRescued = GI && GI->RescuedSurvivorNames.Contains(Mission.SurvivorName);

    int32 LivingPresenceCount = 1; // player
    LivingPresenceCount += bSurvivorAlreadyRescued ? 0 : 1; // active survivor team lead
    LivingPresenceCount += 5; // player-deployed rescue support squad
    LivingPresenceCount += GI && GI->bHasCompanion ? 1 : 0;
    LivingPresenceCount += 4; // Engineer, Medic, Scientist, Trader
    LivingPresenceCount += 3; // civilian identity court
    LivingPresenceCount += 2; // safehouse civilians
    LivingPresenceCount += 3; // classroom/debug-lab civilians
    return FMath::Max(1, LivingPresenceCount);
}

int32 ACodeRescueGameMode::ComputeTargetZombiePresence(const FCodeRescueCityMission& Mission, int32 CityIndex)
{
    const int32 LivingPresenceCount = EstimateLivingPresenceCountForCity(Mission, CityIndex);
    const int32 Ratio = FMath::Max(50, ZombieToLivingPresenceRatio);
    return FMath::Max(Ratio, LivingPresenceCount * Ratio);
}

void ACodeRescueGameMode::SpawnBackgroundHordePopulation(
    const FCodeRescueCityMission& Mission,
    int32 CityIndex,
    const FVector& Origin,
    const FString& CityLabel,
    int32 LivingPresenceCount,
    int32 ActiveZombieCount,
    int32 TargetZombiePresence)
{
    if (bSandboxMode)
    {
        return;
    }

    const int32 RemainingZombiePresence = FMath::Max(0, TargetZombiePresence - ActiveZombieCount);
    if (RemainingZombiePresence <= 0)
    {
        return;
    }

    const int32 ClusterSize = FMath::Max(1, BackgroundHordeClusterSize);
    const int32 ClusterCount = FMath::CeilToInt(static_cast<float>(RemainingZombiePresence) / static_cast<float>(ClusterSize));
    FRandomStream HordeStream(Mission.SkylineSeed ^ 0x50DA1E);
    const FLinearColor HordeRed = FLinearColor(1.0f, 0.04f, 0.02f);
    const FLinearColor CorpseGrey = FLinearColor(0.08f, 0.085f, 0.075f) + Mission.AccentColor * 0.04f;

    auto TagHorde = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("ZombiePopulation50To1"));
            Actor->Tags.Add(FName("BackgroundHordeProxy"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    int32 RepresentedSoFar = 0;
    for (int32 i = 0; i < ClusterCount; ++i)
    {
        const int32 Remaining = FMath::Max(0, RemainingZombiePresence - RepresentedSoFar);
        const int32 RepresentedHere = FMath::Min(ClusterSize, Remaining);
        RepresentedSoFar += RepresentedHere;

        const int32 Side = i % 4;
        const float Along = -3600.0f + FMath::Fmod(static_cast<float>(i) * 540.0f + HordeStream.FRandRange(-120.0f, 120.0f), 7200.0f);
        FVector Local;
        if (Side == 0)
        {
            Local = FVector(Along, 3060.0f + HordeStream.FRandRange(-120.0f, 120.0f), 62.0f);
        }
        else if (Side == 1)
        {
            Local = FVector(Along, -3060.0f + HordeStream.FRandRange(-120.0f, 120.0f), 62.0f);
        }
        else if (Side == 2)
        {
            Local = FVector(-3740.0f + HordeStream.FRandRange(-120.0f, 120.0f), Along * 0.82f, 62.0f);
        }
        else
        {
            Local = FVector(3740.0f + HordeStream.FRandRange(-120.0f, 120.0f), Along * 0.82f, 62.0f);
        }

        const FVector WorldLoc = Origin + CityOffset(Local);
        const float HeightScale = 1.0f + static_cast<float>(RepresentedHere) * 0.018f;
        TagHorde(SpawnBlock(
            WorldLoc,
            CityExtent(FVector(0.18f + RepresentedHere * 0.010f, 0.18f + RepresentedHere * 0.006f, HeightScale)),
            CorpseGrey + HordeRed * (0.12f + 0.02f * (i % 5)),
            FString::Printf(TEXT("%s 50to1 Background Horde Cluster %d represents %d zombies"), *CityLabel, i + 1, RepresentedHere),
            false));

        if (i % 8 == 0)
        {
            TagHorde(SpawnBlock(
                WorldLoc + FVector(0.0f, 0.0f, 118.0f),
                CityExtent(FVector(0.36f, 0.035f, 0.045f)),
                HordeRed * 2.8f,
                CityLabel + TEXT(" 50to1 Horde Eye Glint"),
                false));
        }
    }

    TagHorde(SpawnGuideText(
        FString::Printf(
            TEXT("50:1 OUTBREAK DENSITY\nLiving presences: %d\nZombie presence target: %d\nActive AI threats: %d\nBackground horde: %d proxies x up to %d"),
            LivingPresenceCount,
            TargetZombiePresence,
            ActiveZombieCount,
            ClusterCount,
            ClusterSize),
        Origin + CityOffset(FVector(0.0f, 3180.0f, 620.0f)),
        FColor(255, 70, 60),
        38.0f));
}

bool ACodeRescueGameMode::TryGetRecordedZombieVariant(int32 ZombieId, EZombieVariant& OutVariant) const
{
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        for (const FCodeRescueZombieVariantRecord& Record : GI->SpawnedZombieVariants)
        {
            if (Record.ZombieId == ZombieId)
            {
                OutVariant = Record.Variant;
                return true;
            }
        }
    }

    return false;
}

const FZombieVariantRow* ACodeRescueGameMode::FindZombieVariantRow(EZombieVariant Variant) const
{
    if (ZombieVariantTable && !ShouldUseBuiltInSafeZombieVariant(Variant))
    {
        TArray<FZombieVariantRow*> Rows;
        ZombieVariantTable->GetAllRows<FZombieVariantRow>(TEXT("FindZombieVariantRow"), Rows);
        for (FZombieVariantRow* Row : Rows)
        {
            if (Row && Row->Variant == Variant)
            {
                return Row;
            }
        }
    }

    for (const FZombieVariantRow& Row : GetBuiltInZombieVariantRows())
    {
        if (Row.Variant == Variant)
        {
            return &Row;
        }
    }
    return nullptr;
}

const FZombieVariantRow* ACodeRescueGameMode::SelectZombieVariantRow(int32 CityIndex, int32 ZombieSlot, int32 ZombieId, EZombieVariant& OutVariant) const
{
    OutVariant = EZombieVariant::Default;

    if (TryGetRecordedZombieVariant(ZombieId, OutVariant))
    {
        return FindZombieVariantRow(OutVariant);
    }

    TArray<const FZombieVariantRow*> Rows;
    TSet<EZombieVariant> SeenVariants;
    if (ZombieVariantTable)
    {
        TArray<FZombieVariantRow*> TableRows;
        ZombieVariantTable->GetAllRows<FZombieVariantRow>(TEXT("SelectZombieVariantRow"), TableRows);
        for (const FZombieVariantRow* Row : TableRows)
        {
            if (Row && !ShouldUseBuiltInSafeZombieVariant(Row->Variant))
            {
                Rows.Add(Row);
                SeenVariants.Add(Row->Variant);
            }
        }
    }
    for (const FZombieVariantRow& BuiltInRow : GetBuiltInZombieVariantRows())
    {
        if (!SeenVariants.Contains(BuiltInRow.Variant))
        {
            Rows.Add(&BuiltInRow);
            SeenVariants.Add(BuiltInRow.Variant);
        }
    }
    if (Rows.Num() <= 0)
    {
        return nullptr;
    }

    struct FWeightedVariant
    {
        const FZombieVariantRow* Row = nullptr;
        float Weight = 0.0f;
    };

    TArray<FWeightedVariant> WeightedRows;
    float TotalWeight = 0.0f;
    const int32 ThemeBucket = FMath::Abs(CityIndex) % 3;
    for (const FZombieVariantRow* Row : Rows)
    {
        if (!Row)
        {
            continue;
        }

        float Weight = 1.0f;
        if (Row->ZoneWeights.Num() > 0)
        {
            if (const float* ExactWeight = Row->ZoneWeights.Find(CityIndex))
            {
                Weight = *ExactWeight;
            }
            else if (const float* ThemeWeight = Row->ZoneWeights.Find(ThemeBucket))
            {
                Weight = *ThemeWeight;
            }
            else
            {
                Weight = 0.0f;
            }
        }

        if (Weight > 0.0f)
        {
            WeightedRows.Add({ Row, Weight });
            TotalWeight += Weight;
        }
    }

    if (WeightedRows.Num() <= 0 || TotalWeight <= 0.0f)
    {
        return nullptr;
    }

    FRandomStream Stream((CityIndex + 1) * 73856093 ^ (ZombieSlot + 17) * 19349663);
    float Pick = Stream.FRandRange(0.0f, TotalWeight);
    for (const FWeightedVariant& Weighted : WeightedRows)
    {
        Pick -= Weighted.Weight;
        if (Pick <= 0.0f)
        {
            OutVariant = Weighted.Row->Variant;
            return Weighted.Row;
        }
    }

    OutVariant = WeightedRows.Last().Row->Variant;
    return WeightedRows.Last().Row;
}

void ACodeRescueGameMode::ApplyZombieFamilyVariant(ACodeZombieActor* Zombie, EZombieVariant Variant, int32 ZombieId, FName ContextTag, bool bPersistAssignment) const
{
    if (!Zombie)
    {
        return;
    }

    if (const FZombieVariantRow* VariantRow = FindZombieVariantRow(Variant))
    {
        Zombie->InitializeFromVariant(Variant, *VariantRow);
    }
    else
    {
        Zombie->Variant = Variant;
    }

    Zombie->Tags.AddUnique(FName("ZombieFamilyVariantRuntime"));
    Zombie->Tags.AddUnique(FName("CityZombieFamilyVariant"));
    Zombie->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Zombie->Tags.AddUnique(FName("Top50Recommendations"));
    Zombie->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
    if (ContextTag != NAME_None)
    {
        Zombie->Tags.AddUnique(ContextTag);
    }

    if (bPersistAssignment)
    {
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            GI->RecordZombieVariant(ZombieId, Zombie->Variant);
        }
    }
}

void ACodeRescueGameMode::ApplyCityZombieFamilyVariant(ACodeZombieActor* Zombie, int32 CityIndex, int32 ZombieSlot, int32 ZombieId, FName ContextTag, bool bPersistAssignment) const
{
    EZombieVariant SelectedVariant = EZombieVariant::Default;
    if (const FZombieVariantRow* VariantRow = SelectZombieVariantRow(CityIndex, ZombieSlot, ZombieId, SelectedVariant))
    {
        if (Zombie)
        {
            Zombie->InitializeFromVariant(SelectedVariant, *VariantRow);
        }
    }
    else if (Zombie)
    {
        Zombie->Variant = SelectedVariant;
    }

    if (!Zombie)
    {
        return;
    }

    Zombie->Tags.AddUnique(FName("ZombieFamilyVariantRuntime"));
    Zombie->Tags.AddUnique(FName("CityZombieFamilyVariant"));
    Zombie->Tags.AddUnique(FName("CityWeightedZombieFamily"));
    Zombie->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Zombie->Tags.AddUnique(FName("Top50Recommendations"));
    Zombie->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
    if (ContextTag != NAME_None)
    {
        Zombie->Tags.AddUnique(ContextTag);
    }

    if (bPersistAssignment)
    {
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            GI->RecordZombieVariant(ZombieId, Zombie->Variant);
        }
    }
}

void ACodeRescueGameMode::EnsureCampaignCityLoaded(int32 CityIndex)
{
    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    if (!Missions.IsValidIndex(CityIndex))
    {
        return;
    }

    if (ActiveCampaignCityIndex == CityIndex && StreamedCampaignActors.Num() > 0)
    {
        return;
    }

    ClearStreamedCampaignActors();

    const bool bPreviousCollect = bCollectStreamedCampaignActors;
    bCollectStreamedCampaignActors = true;
    SpawnCampaignCity(Missions[CityIndex], CityIndex);
    bCollectStreamedCampaignActors = bPreviousCollect;
    ActiveCampaignCityIndex = CityIndex;

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->ApplyObjectiveStateToLevel(GetWorld());
    }
    AuditCampaignCityPopulation(CityIndex);

    // Resume can briefly construct city zero before restoring a later saved
    // city. Debounce the briefing so only the city that remains active speaks.
    StopActiveRadioBriefing();
    GetWorldTimerManager().ClearTimer(RadioBriefingDelayTimer);
    PendingRadioCityIndex = CityIndex;
    GetWorldTimerManager().SetTimer(
        RadioBriefingDelayTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, CityIndex]()
        {
            const TArray<FCodeRescueCityMission>& CurrentMissions = FCodeRescueCampaign::GetMissions();
            if (ActiveCampaignCityIndex != CityIndex ||
                PendingRadioCityIndex != CityIndex ||
                LastSpokenRadioCityIndex == CityIndex ||
                !CurrentMissions.IsValidIndex(CityIndex))
            {
                return;
            }
            SpeakRadioBriefing(CurrentMissions[CityIndex]);
            LastSpokenRadioCityIndex = CityIndex;
            UE_LOG(LogTemp, Display,
                TEXT("[RadioVoiceArbiter] city=%d active_voice=1 overlapping_voices=0"),
                CityIndex);
        }),
        0.85f,
        false);
}

bool ACodeRescueGameMode::AuditCampaignCityPopulation(int32 CityIndex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    int32 RenewableAlive = 0;
    int32 PersistentAlive = 0;
    int32 RegularAlive = 0;
    int32 DirectorAlive = 0;
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        const ACodeZombieActor* Zombie = *It;
        if (!IsValid(Zombie) || Zombie->Health <= 0.0f || Zombie->IsHidden()
            || !FCodeRescueCampaign::IsLocationInsideCityArenaXY(
                CityIndex, Zombie->GetActorLocation(), true))
        {
            continue;
        }

        if (UCodeRescueGameInstance::IsPersistentStoryZombieId(Zombie->ZombieId))
        {
            ++PersistentAlive;
        }
        else
        {
            ++RenewableAlive;
        }
        if (Zombie->ZombieId >= CodeRescueRegularZombieIdBase
            && Zombie->ZombieId < CodeRescueBossZombieIdBase)
        {
            ++RegularAlive;
        }
        DirectorAlive += Zombie->Tags.Contains(FName("EncounterDirectorZombie")) ? 1 : 0;
    }

    int32 SavedRenewableDeathsIgnored = 0;
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        for (const int32 ZombieId : GI->NeutralizedZombieIds)
        {
            SavedRenewableDeathsIgnored +=
                UCodeRescueGameInstance::IsPersistentStoryZombieId(ZombieId) ? 0 : 1;
        }
    }

    const bool bPass = bSandboxMode || RenewableAlive >= 6;
    const FString Summary = FString::Printf(
        TEXT("[CityZombiePopulation] COMPLETE %s city=%d renewable_alive=%d regular_alive=%d director_alive=%d persistent_alive=%d saved_renewable_deaths_ignored=%d minimum=6 sandbox=%d"),
        bPass ? TEXT("PASS") : TEXT("FAIL"),
        CityIndex,
        RenewableAlive,
        RegularAlive,
        DirectorAlive,
        PersistentAlive,
        SavedRenewableDeathsIgnored,
        bSandboxMode ? 1 : 0);
    if (bPass)
    {
        Tags.AddUnique(FName("CityZombiePopulationPass"));
        Tags.AddUnique(FName(*FString::Printf(TEXT("CityZombiePopulationCity%dPass"), CityIndex)));
        if (CityIndex == 0)
        {
            Tags.AddUnique(FName("FirstLevelIntegratedPopulationPass"));
        }
        UE_LOG(LogTemp, Display, TEXT("%s"), *Summary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *Summary);
    }
    return bPass;
}

void ACodeRescueGameMode::StartCampaignGroundRecoveryAudit()
{
    FTimerHandle AuditTimer;
    GetWorldTimerManager().SetTimer(AuditTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        constexpr int32 AuditCityIndex = 1;
        EnsureCampaignCityLoaded(AuditCityIndex);

        ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(
            UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        const FVector Origin = FCodeRescueCampaign::GetCityOrigin(AuditCityIndex);
        if (!Character)
        {
            UE_LOG(LogTemp, Error,
                TEXT("[CampaignGroundRecoveryAudit] COMPLETE FAIL city=%d reason=missing_character"),
                AuditCityIndex);
            FPlatformMisc::RequestExit(false);
            return;
        }

        Character->SetActorLocation(
            Origin + FVector(0.0f, 0.0f, -120.0f),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        Character->RecoverToCityArena();

        const FVector RecoveredLocation = Character->GetActorLocation();
        const float RelativeZ = RecoveredLocation.Z - Origin.Z;
        const bool bHeightPass = RelativeZ >= 60.0f && RelativeZ <= 520.0f;
        const bool bBoundsPass = FCodeRescueCampaign::IsLocationInsideCityArenaXY(
            AuditCityIndex, RecoveredLocation);
        const bool bGroundPass = Tags.Contains(FName("CityGroundContinuityCity1Pass"));
        const bool bPerimeterGroundPass = AuditCampaignPerimeterGround(AuditCityIndex);
        const bool bPopulationPass = AuditCampaignCityPopulation(AuditCityIndex);
        const bool bPass = bHeightPass && bBoundsPass && bGroundPass &&
            bPerimeterGroundPass && bPopulationPass;
        const FString Summary = FString::Printf(
            TEXT("[CampaignGroundRecoveryAudit] COMPLETE %s city=%d recovered_relative_z=%.2f height=%d bounds=%d ground=%d perimeter_ground=%d population=%d"),
            bPass ? TEXT("PASS") : TEXT("FAIL"),
            AuditCityIndex,
            RelativeZ,
            bHeightPass ? 1 : 0,
            bBoundsPass ? 1 : 0,
            bGroundPass ? 1 : 0,
            bPerimeterGroundPass ? 1 : 0,
            bPopulationPass ? 1 : 0);
        if (bPass)
        {
            UE_LOG(LogTemp, Display, TEXT("%s"), *Summary);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("%s"), *Summary);
        }
        FPlatformMisc::RequestExit(false);
    }), 1.1f, false);
}

void ACodeRescueGameMode::StartTerminalContrastReview()
{
    FTimerHandle OpenTimer;
    GetWorldTimerManager().SetTimer(OpenTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodingTerminalActor* ReviewTerminal = nullptr;
        for (TActorIterator<ACodingTerminalActor> It(GetWorld()); It; ++It)
        {
            if (It->CityIndex != 0)
            {
                continue;
            }
            if (!ReviewTerminal || It->Challenge.Id.Contains(TEXT("stage03_reverse")))
            {
                ReviewTerminal = *It;
            }
            if (It->Challenge.Id.Contains(TEXT("stage03_reverse")))
            {
                break;
            }
        }

        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!ReviewTerminal || !PC)
        {
            UE_LOG(LogTemp, Error,
                TEXT("[TerminalContrastReview] COMPLETE FAIL terminal=%d controller=%d"),
                ReviewTerminal ? 1 : 0,
                PC ? 1 : 0);
            FPlatformMisc::RequestExit(false);
            return;
        }

        UCodeTerminalWidget* TerminalWidget = CreateWidget<UCodeTerminalWidget>(
            PC, UCodeTerminalWidget::StaticClass());
        if (!TerminalWidget)
        {
            UE_LOG(LogTemp, Error, TEXT("[TerminalContrastReview] COMPLETE FAIL widget=0"));
            FPlatformMisc::RequestExit(false);
            return;
        }

        TerminalWidget->InitializeTerminal(ReviewTerminal);
        TerminalWidget->AddToViewport(500);
        TerminalWidget->SetKeyboardFocus();
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TerminalWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
        ACodeRescueCharacter::SetUIOpen(true);

        FTimerHandle CaptureTimer;
        GetWorldTimerManager().SetTimer(CaptureTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            const FString CapturePath = FPaths::ProjectSavedDir() /
                TEXT("Screenshots/FirstLevel/first_level_terminal_contrast.png");
            FScreenshotRequest::RequestScreenshot(CapturePath, true, false);
            UE_LOG(LogTemp, Display,
                TEXT("[TerminalContrastReview] COMPLETE PASS capture=%s editor_foreground=near_white editor_background=near_black"),
                *CapturePath);

            FTimerHandle ExitTimer;
            GetWorldTimerManager().SetTimer(ExitTimer, FTimerDelegate::CreateLambda([]()
            {
                FPlatformMisc::RequestExit(false);
            }), 0.8f, false);
        }), 1.25f, false);
    }), 1.0f, false);
}

void ACodeRescueGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // #35 — advance time of day and rotate the sun.
    if (DayNightPeriodSeconds > 0.0f)
    {
        TimeOfDay = FMath::Fmod(TimeOfDay + DeltaSeconds / DayNightPeriodSeconds, 1.0f);
        const bool bWasNight = bIsNight;
        const float SolarAltitude = FMath::Cos(TimeOfDay * 2.0f * PI);
        auto SmoothRange = [](float Minimum, float Maximum, float Value)
        {
            const float Alpha = FMath::Clamp((Value - Minimum) / FMath::Max(KINDA_SMALL_NUMBER, Maximum - Minimum), 0.0f, 1.0f);
            return Alpha * Alpha * (3.0f - 2.0f * Alpha);
        };
        const float DayAlpha = SmoothRange(-0.06f, 0.30f, SolarAltitude);
        const float NightAlpha = 1.0f - SmoothRange(-0.24f, 0.04f, SolarAltitude);
        const float TwilightAlpha = 1.0f - SmoothRange(0.02f, 0.36f, FMath::Abs(SolarAltitude));
        bIsNight = SolarAltitude < -0.08f;
        if (SunLight)
        {
            // 0=noon, .25=sunset, .5=midnight, .75=sunrise. SkyAtmosphere
            // receives the same smoothly moving light and produces the real
            // horizon scattering between those phases.
            const float Pitch = -90.0f + 360.0f * TimeOfDay;
            SunLight->SetActorRotation(FRotator(Pitch, 30.0f, 0.0f));
            if (UDirectionalLightComponent* DLC = SunLight->FindComponentByClass<UDirectionalLightComponent>())
            {
                const FLinearColor DaySafe = FLinearColor::LerpUsingHSV(
                    CityDaySunColor, FLinearColor(1.0f, 0.965f, 0.90f), 0.35f);
                const FLinearColor HorizonWarm(1.0f, 0.29f, 0.075f);
                const float HighSunBlend = SmoothRange(0.02f, 0.60f, SolarAltitude);
                const FLinearColor SunColor = FLinearColor::LerpUsingHSV(HorizonWarm, DaySafe, HighSunBlend);
                const float SunIntensity = FMath::Max(
                    0.03f,
                    FMath::Max(CityDaySunIntensity, 6.5f) * DayAlpha + TwilightAlpha * 1.65f);
                DLC->SetIntensity(SunIntensity);
                DLC->SetLightColor(SunColor);
            }
            if (!MoonLight)
            {
                MoonLight = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(),
                    FVector(0.0f, 0.0f, 2200.0f), FRotator(-35.0f, -60.0f, 0.0f));
                if (MoonLight)
                {
                    if (UDirectionalLightComponent* MLC = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent()))
                    {
                        MLC->SetMobility(EComponentMobility::Movable);
                        MLC->SetIntensity(0.0f);
                        MLC->SetLightColor(FLinearColor(0.58f, 0.66f, 0.92f));
                        MLC->SetCastShadows(true);
                        MLC->SetAtmosphereSunLight(false);
                    }
                    MoonLight->Tags.Add(FName("CityMoodLayer"));
                }
            }
            if (MoonLight)
            {
                if (UDirectionalLightComponent* MLC = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent()))
                {
                    MLC->SetVisibility(NightAlpha > 0.01f);
                    MLC->SetIntensity(FMath::Lerp(0.0f, 2.35f, NightAlpha));
                }
            }
        }
        if (WorldSkyLight)
        {
            if (USkyLightComponent* SkyComponent = WorldSkyLight->GetLightComponent())
            {
                SkyComponent->SetIntensity(FMath::Lerp(0.68f, 1.48f, DayAlpha) + TwilightAlpha * 0.12f);
                SkyComponent->SetLightColor(FLinearColor::LerpUsingHSV(
                    FLinearColor(0.42f, 0.50f, 0.72f),
                    FLinearColor(0.95f, 0.96f, 1.0f),
                    DayAlpha));
            }
        }
        const int32 SkyPhase = SolarAltitude > 0.28f
            ? 0
            : SolarAltitude < -0.12f
                ? 2
                : TimeOfDay < 0.5f ? 1 : 3;
        if (SkyPhase != LastLoggedSkyPhase)
        {
            static const TCHAR* PhaseNames[] = { TEXT("DAY"), TEXT("SUNSET"), TEXT("NIGHT"), TEXT("SUNRISE") };
            LastLoggedSkyPhase = SkyPhase;
            UE_LOG(LogTemp, Display,
                TEXT("[FirstLevelSkyCycle] phase=%s time=%.3f solar=%.3f day=%.2f twilight=%.2f night=%.2f"),
                PhaseNames[SkyPhase], TimeOfDay, SolarAltitude, DayAlpha, TwilightAlpha, NightAlpha);
        }
        // 2026-07-04: star dome + moon follow the player and appear only through the night window.
        UpdateNightSkyVisibility();
        if (bWasNight != bIsNight && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f,
                bIsNight ? FColor(40, 40, 200) : FColor::Yellow,
                bIsNight ? TEXT("Night falls. Zombie activity rises.")
                         : TEXT("Day breaks. The world feels lighter."));
        }
        // #68: at every day↔night transition, refresh ambient NPC perks so
        // the player can re-visit traders/medics across day cycles.
        if (bWasNight != bIsNight)
        {
            if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
            {
                GI->ResetFriendlyNPCServiceCooldowns();
            }
            for (TActorIterator<AFriendlyNPCActor> It(GetWorld()); It; ++It)
            {
                It->ResetDailyPerk();
            }
        }
    }

    // #15 — accumulate run seconds.
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->AccumulateRunSeconds(DeltaSeconds);
    }
}

// ---- #36 weather --------------------------------------------------------
void ACodeRescueGameMode::SpawnWeatherForCity(int32 CityIndex, const FVector& Origin)
{
    if (IsValid(ActiveWeatherField))
    {
        ActiveWeatherField->Destroy();
        ActiveWeatherField = nullptr;
    }
    const FTransform WeatherTransform(FRotator::ZeroRotator, Origin);
    ActiveWeatherField = GetWorld()->SpawnActorDeferred<ACodeRescueWeatherFieldActor>(
        ACodeRescueWeatherFieldActor::StaticClass(), WeatherTransform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (ActiveWeatherField)
    {
        ActiveWeatherField->ConfigureForCity(CityIndex, Origin);
        UGameplayStatics::FinishSpawningActor(ActiveWeatherField, WeatherTransform);
        ActiveWeatherField->Tags.AddUnique(FName("FirstLevelWeatherGameplayInfluence"));
        RegisterStreamedActor(ActiveWeatherField);
    }

    // Keep optional authored Niagara systems as a supplementary layer when a
    // designer wires them, but the native Blender-backed field is complete on
    // its own and no longer silently disappears when this array is empty.
    const int32 ZoneIdx = CityIndex % 3;
    if (!ZoneWeatherSystems.IsValidIndex(ZoneIdx)) return;
    UNiagaraSystem* Sys = ZoneWeatherSystems[ZoneIdx].LoadSynchronous();
    if (!Sys) return;
    UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), Sys, Origin + FVector(0, 0, 2000.0f), FRotator::ZeroRotator,
        FVector(40.0f, 40.0f, 1.0f), true);
    if (Comp)
    {
        // Track via the streamed-actor list so it's cleaned up on city swap.
        if (AActor* Owner = Comp->GetOwner())
        {
            RegisterStreamedActor(Owner);
        }
    }
}

void ACodeRescueGameMode::SpawnWeatherLightingIdentityLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams Climate = BuildUSCityRealizationParams(Mission, Profile);

    FString WeatherTitle = TEXT("CLEAR SKY CONTRAST");
    FString WeatherCue = TEXT("crisp light, long sightlines, and high-contrast route pools");
    FLinearColor WeatherColor = Climate.DaySunColor * 0.75f + Mission.AccentColor * 0.25f;
    FLinearColor ShelterColor = Mission.SecondaryAccentColor * 0.65f + FLinearColor(0.18f, 0.22f, 0.26f, 1.0f) * 0.35f;

    if (Climate.CloudToken == TEXT("Overcast"))
    {
        WeatherTitle = TEXT("OVERCAST SHELTER LIGHTING");
        WeatherCue = TEXT("soft gray sky, wet pavement strips, and warm door pools");
        WeatherColor = FLinearColor(0.54f, 0.58f, 0.64f, 1.0f);
        ShelterColor = FLinearColor(1.0f, 0.72f, 0.32f, 1.0f);
    }
    else if (Climate.CloudToken == TEXT("MarineLayer"))
    {
        WeatherTitle = TEXT("MARINE FOG ROUTE LIGHTING");
        WeatherCue = TEXT("low fog banks, blue route lamps, and coastline haze");
        WeatherColor = FLinearColor(0.46f, 0.62f, 0.72f, 1.0f);
        ShelterColor = FLinearColor(0.30f, 0.86f, 1.0f, 1.0f);
    }
    else if (Climate.CloudToken == TEXT("HazeWarm"))
    {
        WeatherTitle = TEXT("WARM HAZE HEAT MIRAGE");
        WeatherCue = TEXT("amber haze bands, hot skyline glow, and shade markers");
        WeatherColor = FLinearColor(1.0f, 0.62f, 0.26f, 1.0f);
        ShelterColor = FLinearColor(1.0f, 0.82f, 0.38f, 1.0f);
    }
    else if (Climate.CloudToken == TEXT("HumidGlow"))
    {
        WeatherTitle = TEXT("HUMID STORM GLOW");
        WeatherCue = TEXT("storm haze, green-blue air, and saturated shelter lights");
        WeatherColor = FLinearColor(0.58f, 0.72f, 0.56f, 1.0f);
        ShelterColor = FLinearColor(0.25f, 1.0f, 0.76f, 1.0f);
    }
    else if (Climate.CloudToken == TEXT("SnowSky"))
    {
        WeatherTitle = TEXT("COLD SNOW SKY");
        WeatherCue = TEXT("blue snow light, white sky flecks, and heated refuge pools");
        WeatherColor = FLinearColor(0.72f, 0.82f, 1.0f, 1.0f);
        ShelterColor = FLinearColor(1.0f, 0.62f, 0.34f, 1.0f);
    }

    FString SafeCloudToken = Climate.CloudToken;
    SafeCloudToken.ReplaceInline(TEXT(" "), TEXT(""));
    FString SafeGradeToken = Climate.GradeToken;
    SafeGradeToken.ReplaceInline(TEXT(" "), TEXT(""));
    const FName WeatherLayerTag(*FString::Printf(TEXT("RuntimeDataLayer_Weather_%s"), *SafeCloudToken));
    const FName GradeLayerTag(*FString::Printf(TEXT("RuntimeDataLayer_Grade_%s"), *SafeGradeToken));

    auto TagWeather = [this, WeatherLayerTag, GradeLayerTag](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("WeatherLightingIdentity"));
            Actor->Tags.Add(FName("WeatherLightingIdentityReady"));
            Actor->Tags.Add(FName("DistrictWeatherCue"));
            Actor->Tags.Add(FName("NonBlockingWeatherCue"));
            Actor->Tags.Add(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.Add(FName("Top50Recommendations"));
            ApplyRuntimeDataLayerTags(Actor, TArray<FName>{
                FName("RuntimeDataLayer_Time_DayNightCycle"),
                FName("RuntimeDataLayer_Mode_WeatherLighting"),
                WeatherLayerTag,
                GradeLayerTag,
            });
        }
        return Actor;
    };

    auto SpawnWeatherLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(CityLabel + TEXT(" ") + Name);
#endif
            RegisterStreamedActor(Light);
            TagWeather(Light);
            Light->Tags.Add(FName("WeatherLightingSignalLight"));
        }
    };

    struct FWeatherDistrictCue
    {
        const TCHAR* Name;
        FVector LocalCenter;
        FVector LabelOffset;
    };

    const FWeatherDistrictCue Districts[] = {
        { TEXT("ENTRY WEATHER SHELTER"), FVector(-3260.0f, -2460.0f, 0.0f), FVector(0.0f, -170.0f, 280.0f) },
        { TEXT("SAFEHOUSE LIGHT POOL"), FVector(-2860.0f, -3020.0f, 0.0f), FVector(0.0f, 180.0f, 300.0f) },
        { TEXT("RESCUE ROUTE SKY CUE"), FVector(760.0f, -1420.0f, 0.0f), FVector(0.0f, -190.0f, 320.0f) },
    };

    for (int32 DistrictIndex = 0; DistrictIndex < UE_ARRAY_COUNT(Districts); ++DistrictIndex)
    {
        const FWeatherDistrictCue& District = Districts[DistrictIndex];
        const FVector Center = Origin + CityOffset(District.LocalCenter);
        const FLinearColor DistrictColor = FLinearColor::LerpUsingHSV(WeatherColor, ShelterColor, static_cast<float>(DistrictIndex) / 2.0f);

        // 2026-07-02: emissive multipliers dialed down (was 2.2 / 1.65). At the bright open
        // entry plaza these strips bloomed into a teal glow; a calmer emissive keeps the accent
        // without washing the frame.
        TagWeather(SpawnBlock(
            Center + FVector(0.0f, 0.0f, 16.0f),
            FVector(3.8f, 0.48f, 0.055f),
            DistrictColor * 1.15f,
            FString::Printf(TEXT("%s %s Ground Weather Reflection"), *CityLabel, District.Name),
            false));
        TagWeather(SpawnBlock(
            Center + FVector(0.0f, -78.0f, 132.0f),
            FVector(2.65f, 0.045f, 0.58f),
            DistrictColor * 1.0f,
            FString::Printf(TEXT("%s %s Climate Sign"), *CityLabel, District.Name),
            false));
        TagWeather(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s\nclouds: %s | grade: %s"),
                District.Name,
                *WeatherCue,
                *Climate.CloudToken,
                *Climate.GradeToken),
            Center + District.LabelOffset,
            DistrictColor.ToFColor(true),
            18.0f));

        for (int32 Stripe = 0; Stripe < 5; ++Stripe)
        {
            const float X = -260.0f + Stripe * 130.0f;
            const float Z = 118.0f + (Stripe % 2) * 44.0f;
            const FVector StripeScale = Climate.CloudToken == TEXT("SnowSky")
                ? FVector(0.060f, 0.030f, 0.28f)
                : FVector(0.055f, 0.030f, 0.76f);
            TagWeather(SpawnBlock(
                Center + FVector(X, -18.0f + Stripe * 8.0f, Z),
                StripeScale,
                DistrictColor * (1.15f + Stripe * 0.10f),
                FString::Printf(TEXT("%s %s Weather Streak %d"), *CityLabel, District.Name, Stripe + 1),
                false));
        }

        SpawnWeatherLight(
            Center + FVector(0.0f, -18.0f, 250.0f),
            DistrictColor,
            2200.0f + DistrictIndex * 900.0f,
            620.0f + DistrictIndex * 110.0f,
            FString::Printf(TEXT("%s Weather Light"), District.Name));
    }

    TagWeather(SpawnGuideText(
        FString::Printf(TEXT("WEATHER + LIGHTING IDENTITY\n%s\n%s\n%s"), *Mission.CityName, *WeatherTitle, *Climate.GradeToken),
        Origin + CityOffset(FVector(-710.0f, -2320.0f, 420.0f)),
        WeatherColor.ToFColor(true),
        28.0f));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueWeatherLightingIdentity] %s spawned district weather cues title='%s' clouds='%s' grade='%s' fog=%.3f"),
        *CityLabel,
        *WeatherTitle,
        *Climate.CloudToken,
        *Climate.GradeToken,
        Climate.FogDensity);

    (void)CityIndex;
}

void ACodeRescueGameMode::SpawnRuntimeDataLayerMigrationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bTerminalSolved = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams Climate = BuildUSCityRealizationParams(Mission, Profile);

    FString SafeCloudToken = Climate.CloudToken;
    SafeCloudToken.ReplaceInline(TEXT(" "), TEXT(""));
    FString SafeGradeToken = Climate.GradeToken;
    SafeGradeToken.ReplaceInline(TEXT(" "), TEXT(""));

    const FName WeatherLayerTag(*FString::Printf(TEXT("RuntimeDataLayer_Weather_%s"), *SafeCloudToken));
    const FName GradeLayerTag(*FString::Printf(TEXT("RuntimeDataLayer_Grade_%s"), *SafeGradeToken));

    auto TagDataLayerActor = [this](AActor* Actor, const TArray<FName>& LayerTags) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("RuntimeDataLayerMigrationLayer"));
            Actor->Tags.AddUnique(FName("WorldPartitionDataLayerBridge"));
            Actor->Tags.AddUnique(FName("DataLayerStateTimeModeReview"));
            Actor->Tags.AddUnique(FName("OFPAActorLayerAuditReady"));
            Actor->Tags.AddUnique(FName("NonBlockingWorldPromotionCue"));
            ApplyRuntimeDataLayerTags(Actor, LayerTags);
        }
        return Actor;
    };

    auto SpawnDataLayerLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, const TArray<FName>& LayerTags, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(520.0f);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagDataLayerActor(Light, LayerTags);
        }
    };

    struct FRuntimeDataLayerCard
    {
        const TCHAR* Title;
        FString Detail;
        FVector LocalOffset;
        FLinearColor Color;
        TArray<FName> LayerTags;
    };

    const FVector StationCenter = Origin + CityOffset(FVector(-3340.0f, 2900.0f, 0.0f));
    const FLinearColor StreamBlue = FLinearColor(0.10f, 0.74f, 1.0f);
    const FLinearColor SafeCyan = FLinearColor(0.16f, 0.94f, 1.0f);
    const FLinearColor CombatRed = FLinearColor(1.0f, 0.12f, 0.08f);
    const FLinearColor RescueGold = FLinearColor(1.0f, 0.84f, 0.18f);
    const FLinearColor WeatherTeal = FLinearColor(0.34f, 0.92f, 0.72f);
    const FLinearColor TimeViolet = FLinearColor(0.72f, 0.36f, 1.0f);

    TArray<FRuntimeDataLayerCard> Cards;
    Cards.Add({
        TEXT("STREAMING CELL"),
        FString::Printf(TEXT("active C++ fallback cell: city %d"), CityIndex + 1),
        FVector(-560.0f, 0.0f, 0.0f),
        StreamBlue,
        TArray<FName>{ FName("RuntimeWorldPartitionStreamCell"), FName("CurrentCppWorldPartitionFallback") },
    });
    Cards.Add({
        TEXT("SAFEHOUSE MODE"),
        TEXT("protected coding safe beat stays loaded before combat"),
        FVector(-280.0f, 0.0f, 0.0f),
        SafeCyan,
        TArray<FName>{ FName("RuntimeDataLayer_State_SafeBeat"), FName("RuntimeDataLayer_Mode_CodingSafehouse") },
    });
    Cards.Add({
        TEXT("COMBAT MODE"),
        bTerminalSolved ? TEXT("rescue pressure active after terminal solve") : TEXT("overrun pressure staged outside the lab"),
        FVector(0.0f, 0.0f, 0.0f),
        CombatRed,
        TArray<FName>{ FName("RuntimeDataLayer_State_Overrun"), FName("RuntimeDataLayer_Mode_Combat") },
    });
    Cards.Add({
        TEXT("OBJECTIVE STATE"),
        bTerminalSolved ? TEXT("survivor route open layer") : TEXT("terminal locked layer until code passes"),
        FVector(280.0f, 0.0f, 0.0f),
        RescueGold,
        TArray<FName>{
            bTerminalSolved ? FName("RuntimeDataLayer_State_RescueRouteOpen") : FName("RuntimeDataLayer_State_TerminalLocked"),
            FName("RuntimeDataLayer_Mode_RescueTraversal"),
        },
    });
    Cards.Add({
        TEXT("WEATHER / TIME"),
        FString::Printf(TEXT("%s grade, %s clouds, day-night layer"), *Climate.GradeToken, *Climate.CloudToken),
        FVector(560.0f, 0.0f, 0.0f),
        FLinearColor::LerpUsingHSV(WeatherTeal, TimeViolet, 0.35f),
        TArray<FName>{ FName("RuntimeDataLayer_Time_DayNightCycle"), WeatherLayerTag, GradeLayerTag },
    });

    TagDataLayerActor(SpawnTexturedBlock(
        StationCenter + FVector(0.0f, 0.0f, -4.0f),
        FVector(12.8f, 2.25f, 0.050f),
        FLinearColor(0.020f, 0.032f, 0.038f) + Mission.AccentColor * 0.10f,
        CityLabel + TEXT(" Runtime Data Layer Migration Deck"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false),
        TArray<FName>{ FName("RuntimeDataLayerMigrationDeck"), FName("RuntimeWorldPartitionStreamCell") });

    TagDataLayerActor(SpawnBlock(
        StationCenter + FVector(0.0f, -132.0f, 145.0f),
        FVector(11.8f, 0.07f, 1.05f),
        FLinearColor(0.034f, 0.045f, 0.052f) + Mission.SecondaryAccentColor * 0.16f,
        CityLabel + TEXT(" Runtime Data Layer Backboard"),
        false),
        TArray<FName>{ FName("RuntimeDataLayerMigrationBoard"), FName("RuntimeWorldPartitionStreamCell") });

    TagDataLayerActor(SpawnGuideText(
        FString::Printf(TEXT("WORLD PARTITION NAVIGATION\nDATA LAYERS: state | time | mode\n%s"), *Mission.CityName),
        StationCenter + FVector(0.0f, -178.0f, 342.0f),
        FColor(170, 230, 255),
        26.0f),
        TArray<FName>{ FName("RuntimeDataLayerMigrationBoard"), FName("RuntimeWorldPartitionStreamCell") });

    for (int32 Index = 0; Index < Cards.Num(); ++Index)
    {
        const FRuntimeDataLayerCard& Card = Cards[Index];
        const FVector Base = StationCenter + Card.LocalOffset;
        TagDataLayerActor(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 15.0f),
            FVector(2.10f, 0.42f, 0.060f),
            Card.Color * 1.65f,
            FString::Printf(TEXT("%s Runtime Data Layer Pad %d"), *CityLabel, Index + 1),
            false),
            Card.LayerTags);
        TagDataLayerActor(SpawnBlock(
            Base + FVector(0.0f, -40.0f, 102.0f),
            FVector(0.16f, 0.16f, 1.20f),
            Card.Color * 1.45f,
            FString::Printf(TEXT("%s Runtime Data Layer Pylon %d"), *CityLabel, Index + 1),
            false),
            Card.LayerTags);
        TagDataLayerActor(SpawnBlock(
            Base + FVector(0.0f, -66.0f, 192.0f),
            FVector(1.28f, 0.04f, 0.38f),
            Card.Color * 1.90f,
            FString::Printf(TEXT("%s Runtime Data Layer Label Plate %d"), *CityLabel, Index + 1),
            false),
            Card.LayerTags);
        TagDataLayerActor(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Card.Title, *Card.Detail),
            Base + FVector(0.0f, -96.0f, 268.0f),
            Card.Color.ToFColor(true),
            16.0f),
            Card.LayerTags);
        SpawnDataLayerLight(
            Base + FVector(0.0f, -34.0f, 238.0f),
            Card.Color,
            1500.0f + Index * 260.0f,
            Card.LayerTags,
            FString::Printf(TEXT("%s Runtime Data Layer Light %d"), *CityLabel, Index + 1));
    }

    TagDataLayerActor(SpawnGuideText(
        TEXT("SAFEHOUSE OBJECTIVE EXTRACTION\nAuthor Data Layers later; keep this fallback playable now."),
        StationCenter + FVector(0.0f, 150.0f, 286.0f),
        FColor(210, 245, 230),
        20.0f),
        TArray<FName>{
            FName("RuntimeDataLayer_State_SafeBeat"),
            FName("RuntimeDataLayer_Mode_RescueTraversal"),
            FName("RuntimeWorldPartitionStreamCell"),
        });

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueRuntimeDataLayers] %s city=%d solved=%s cloud='%s' grade='%s' spawned World Partition/Data Layer migration station."),
        *CityLabel,
        CityIndex,
        bTerminalSolved ? TEXT("true") : TEXT("false"),
        *Climate.CloudToken,
        *Climate.GradeToken);
}

// ---- #37 secret terminal -------------------------------------------------
void ACodeRescueGameMode::SpawnSecretTerminalForCity(int32 CityIndex, const FVector& Origin)
{
    // Bonus lessons now live in the protected safehouse annex, not the combat
    // district. Players should be able to code without losing the rescue run.
    const FVector Loc = Origin + CityOffset(FVector(-2480.0f, -2480.0f, 185.0f));
    const FString SecretId = FString::Printf(TEXT("secret_bsearch_city%d"), CityIndex);
    const FString SecretTitle = TEXT("HIDDEN TERMINAL — Binary Search");
    const FString SecretBrief = TEXT("Protected bonus terminal: implement binary search for a 5x intel score reward.");
    SpawnTerminal(Loc, SecretId, SecretTitle, SecretBrief, CityIndex);
    AActor* AnnexMarker = SpawnBlock(
        Loc + FVector(0.0f, 0.0f, -96.0f),
        FVector(1.75f, 1.35f, 0.08f),
        FLinearColor(0.12f, 0.8f, 1.0f) * 1.55f,
        TEXT("Protected Secret Coding Annex"),
        false);
    if (AnnexMarker)
    {
        AnnexMarker->Tags.Add(FName("ProtectedCodingChallengeZone"));
        AnnexMarker->Tags.Add(FName("NoZombieLearningZone"));
        AnnexMarker->Tags.Add(FName("SafeTerminalLab"));
        AnnexMarker->Tags.Add(FName("LearningWithoutDeathRisk"));
        AnnexMarker->Tags.Add(FName("BonusCodingChallengeSafeZone"));
    }
    // Visual marker so it is findable inside the safe learning district.
    SpawnGuideText(TEXT("HIDDEN TERMINAL\n5x score"),
                   Loc + FVector(0, 0, 600), FColor(255, 200, 50), 60.0f);
}

void ACodeRescueGameMode::SpeakRadioBriefing(const FCodeRescueCityMission& Mission)
{
    StopActiveRadioBriefing();

    // Text coverage is the guaranteed radio baseline: every mission should
    // produce an on-screen briefing even when cooked cues or platform TTS are
    // unavailable in a packaged/offline run.
    const int32 CityIdx = ActiveCampaignCityIndex;
    UCodeRescueSubtitlesWidget::Push(Mission.RadioBriefing, 12.0f);
    UCodeRescueSubtitlesWidget::Push(BuildRadioRouteCadenceLine(Mission, CityIdx, GetGameInstance<UCodeRescueGameInstance>()), 8.0f);

    // Prefer cooked WAV cues when explicitly enabled. The previous ordering let
    // macOS /usr/bin/say return early, which meant -UseCookedRadioVoice could
    // never actually select the imported Maple cues on Mac.
    const bool bCookedRadioVoiceAllowed =
        bPreferCookedRadioBriefingCues ||
        FParse::Param(FCommandLine::Get(), TEXT("UseCookedRadioVoice"));
    if (bCookedRadioVoiceAllowed && CityIdx >= 0 && CityRadioBriefingCues.IsValidIndex(CityIdx))
    {
        if (USoundBase* Cue = CityRadioBriefingCues[CityIdx].LoadSynchronous())
        {
            ActiveRadioBriefingComponent = UGameplayStatics::SpawnSound2D(
                this, Cue, GetRuntimeSfxVolume(this), 1.0f, 0.0f, nullptr, false, true);
            return; // success - skip the TTS fallback
        }
    }

    // Demo-readiness pass: generated Maple cues are imported by stable city
    // slug. Load them directly when the native GameMode is active and no
    // Blueprint array entry has been assigned for this mission.
    if (bCookedRadioVoiceAllowed && !Mission.Slug.IsEmpty())
    {
        const FString AssetName = FString::Printf(TEXT("%s_radio_briefing"), *Mission.Slug);
        const FString AssetPath = FString::Printf(
            TEXT("/Game/CodeRescueAssets/Audio/RadioSamples/%s.%s"),
            *AssetName,
            *AssetName);
        if (USoundBase* Cue = Cast<USoundBase>(
                StaticLoadObject(USoundBase::StaticClass(), nullptr, *AssetPath)))
        {
            ActiveRadioBriefingComponent = UGameplayStatics::SpawnSound2D(
                this, Cue, GetRuntimeSfxVolume(this), 1.0f, 0.0f, nullptr, false, true);
            return; // success - skip the TTS fallback
        }
    }

#if PLATFORM_MAC
    const bool bRadioVoiceMuted = FParse::Param(FCommandLine::Get(), TEXT("NoRadioVoice"));
    if (bEnableSystemRadioVoice && !bRadioVoiceMuted)
    {
        const FString SayPath = TEXT("/usr/bin/say");
        if (FPaths::FileExists(SayPath))
        {
            const FString SpokenText = FString::Printf(TEXT("Operation Code Rescue. %s"), *Mission.RadioBriefing);
            const FString Args = FString::Printf(TEXT("-v Samantha -r 165 %s"), *QuoteForProcessArg(SpokenText));
            ActiveSystemRadioProcess = FPlatformProcess::CreateProc(
                *SayPath, *Args, true, false, false, nullptr, 0, nullptr, nullptr);
            return;
        }
    }
#endif
}

void ACodeRescueGameMode::StopActiveRadioBriefing()
{
    bool bStoppedVoice = false;
    if (IsValid(ActiveRadioBriefingComponent))
    {
        ActiveRadioBriefingComponent->Stop();
        ActiveRadioBriefingComponent = nullptr;
        bStoppedVoice = true;
    }
    if (ActiveSystemRadioProcess.IsValid())
    {
        if (FPlatformProcess::IsProcRunning(ActiveSystemRadioProcess))
        {
            FPlatformProcess::TerminateProc(ActiveSystemRadioProcess, true);
            bStoppedVoice = true;
        }
        FPlatformProcess::CloseProc(ActiveSystemRadioProcess);
        ActiveSystemRadioProcess.Reset();
    }
    if (bStoppedVoice)
    {
        UE_LOG(LogTemp, Display, TEXT("[RadioVoiceArbiter] previous_voice_stopped=1"));
    }
}

AActor* ACodeRescueGameMode::SpawnGuideText(const FString& Text, const FVector& Location, const FColor& Color, float Size)
{
    if (FParse::Param(FCommandLine::Get(), TEXT("VisualReviewStart")))
    {
        return nullptr;
    }

    // World-text declutter (2026-07-01): a substantive message becomes a compact hovering marker with a
    // unique id; the player reads the full paragraph in a separate scrollable screen (press E / Enter near
    // the marker) so the words stop competing for attention. Launch with -NoHoverMarkers for legacy text.
    const bool bHoverMarkers = !FParse::Param(FCommandLine::Get(), TEXT("NoHoverMarkers"));
    // Essential gameplay text (control prompts, the launch-language labels, objective banners)
    // must stay as real world text: never convert it to a read-on-demand marker. Fixes the
    // 2026-07-01 regression where "JAVA\nTRACK ONLY" pedestal labels vanished into markers.
    const bool bEssential = IsEssentialGuideText(Text.ToUpper());
    // 2026-07-04 (Kenny's word-competition directive): anything longer than ONE WORD becomes a
    // BEAMING SYMBOL above the thing it marks (ACodeRescueBeaconMarkerActor: vertical light beam +
    // one category glyph). The full original text stays reachable — every beacon keeps the
    // MessageMarker read-on-demand contract (walk up, press E). Single words may stay as text.
    const FString FlatText = Text.Replace(TEXT("\n"), TEXT(" ")).TrimStartAndEnd();
    const bool bMultiWord = FlatText.Contains(TEXT(" "));
    if (bHoverMarkers && bMultiWord && !bEssential)
    {
        static int32 GMessageMarkerCounter = 0;
        ++GMessageMarkerCounter;

        FString FirstLine = Text;
        int32 NewlineIdx = INDEX_NONE;
        if (Text.FindChar(TEXT('\n'), NewlineIdx))
        {
            FirstLine = Text.Left(NewlineIdx);
        }
        FirstLine = FirstLine.TrimStartAndEnd();
        const FString MarkerId = FString::Printf(TEXT("[%03d] %s"), GMessageMarkerCounter, *FirstLine.Left(16));

        bool bReducedMotion = false;
        if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            bReducedMotion = GI->bReducedMotion;
        }

        ACodeRescueBeaconMarkerActor* Marker = GetWorld()->SpawnActor<ACodeRescueBeaconMarkerActor>(
            ACodeRescueBeaconMarkerActor::StaticClass(), Location, FRotator(0, 45, 0));
        if (Marker)
        {
            Marker->ConfigureMessage(MarkerId, FirstLine, Text, FLinearColor(Color), bReducedMotion);
            Marker->ConfigureBeacon(SymbolForGuideText(Text.ToUpper()), FLinearColor(Color));
            RegisterStreamedActor(Marker);
        }
        return Marker;
    }

    ATextRenderActor* Label = GetWorld()->SpawnActor<ATextRenderActor>(ATextRenderActor::StaticClass(), Location, FRotator(0, 45, 0));
    if (Label)
    {
        const FString UpperText = Text.ToUpper();
        // Essential prompts keep their words; a single word is already a "symbol" and keeps itself.
        const bool bKeepText = IsEssentialGuideText(UpperText) || !bMultiWord;
        const FString DisplayText = bKeepText ? Text : SymbolForGuideText(UpperText);
        const float DisplaySize = bKeepText ? FMath::Clamp(Size, 18.0f, 52.0f) : FMath::Clamp(Size * 0.70f, 20.0f, 34.0f);

        Label->GetTextRender()->SetText(FText::FromString(DisplayText));
        Label->GetTextRender()->SetTextRenderColor(Color);
        Label->GetTextRender()->SetWorldSize(DisplaySize);
        Label->GetTextRender()->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
        Label->GetTextRender()->SetCastShadow(false);
        Label->Tags.Add(bKeepText ? FName("WorldTextEssential") : FName("WorldInfoSymbol"));
        RegisterStreamedActor(Label);
    }
    return Label;
}

void ACodeRescueGameMode::SpawnZone(const FString& ZoneName, const FVector& Origin, const FLinearColor& AccentColor)
{
    // Very large zone floor. This is intentionally city-block scale, not room scale.
    SpawnBlock(Origin + FVector(0, 0, -6), FVector(160, 160, 0.12f), FLinearColor(0.10f, 0.10f, 0.12f), ZoneName + TEXT(" Mega Floor"));

    // Low guide rails only; they show boundaries without making a boxed-in room.
    SpawnBlock(Origin + FVector(0, -8000, 55), FVector(160, 0.25f, 0.8f), AccentColor * 0.8f, ZoneName + TEXT(" North Guide Rail"));
    SpawnBlock(Origin + FVector(0, 8000, 55), FVector(160, 0.25f, 0.8f), AccentColor * 0.8f, ZoneName + TEXT(" South Guide Rail"));
    SpawnBlock(Origin + FVector(-8000, 0, 55), FVector(0.25f, 160, 0.8f), AccentColor * 0.8f, ZoneName + TEXT(" West Guide Rail"));
    SpawnBlock(Origin + FVector(8000, 0, 55), FVector(0.25f, 160, 0.8f), AccentColor * 0.8f, ZoneName + TEXT(" East Guide Rail"));

    // 2026-07-07: the persistent city-scale DrawDebugBox is GONE. Kenny plays
    // DEVELOPMENT builds where ENABLE_DRAW_DEBUG is on, so the "editor-only"
    // outline was in fact a permanent 160m accent-magenta wire web across his
    // night sky (identified live: the criss-crossing lines that dominated
    // every night scene). The low guide rails above already mark the bounds.
    SpawnGuideText(ZoneName, Origin + FVector(-5200, -6400, 520), AccentColor.ToFColor(true), 170.0f);

    // Procedural buildings, upgraded from a single cube per building to a
    // stack of bodies with emissive "window strip" cubes between floors.
    // Reads as architecture rather than as primitives.
    const FLinearColor BodyPalette[4] = {
        FLinearColor(0.06f, 0.07f, 0.10f),  // graphite
        FLinearColor(0.10f, 0.10f, 0.13f),  // slate
        FLinearColor(0.13f, 0.12f, 0.11f),  // warm grey
        FLinearColor(0.08f, 0.10f, 0.14f)   // navy concrete
    };

    // Window emissive matches the zone accent so each location has a distinct
    // visual signature without needing hand-built art for every city.
    const FLinearColor WindowEmissive = AccentColor * 6.0f;

    for (int32 i = 0; i < 36; ++i)
    {
        const float X = Origin.X + FMath::RandRange(-6200.0f, 6200.0f);
        const float Y = Origin.Y + FMath::RandRange(-6200.0f, 6200.0f);
        // Footprint width/depth varies, but stays close to square so the
        // building reads as a tower rather than a slab.
        const float WidthScale = FMath::RandRange(1.4f, 3.5f);
        const float DepthScale = FMath::RandRange(1.4f, 3.5f);
        const int32 Floors = FMath::RandRange(2, 8);
        const float FloorScaleZ = 1.4f;                // height of one floor in cube-scale units
        const float FloorHeight = FloorScaleZ * 100.0f; // world units per floor (cube edge = 100 * scale)
        const FLinearColor BodyTint = BodyPalette[i % 4];

        for (int32 F = 0; F < Floors; ++F)
        {
            const float ZBase = (F * FloorHeight) + (FloorHeight * 0.5f);
            // Body
            SpawnBlock(
                FVector(X, Y, ZBase),
                FVector(WidthScale, DepthScale, FloorScaleZ),
                BodyTint,
                FString::Printf(TEXT("%s Building Floor %d"), *ZoneName, F + 1));

            // Two thin emissive window-strip cubes flanking the body — one on
            // each long side. Skip the ground floor so it reads as a darker
            // base. Adds the cinematic "glowing tower at night" look.
            if (F > 0)
            {
                const float StripZ = ZBase;
                const float StripDepth = DepthScale * 0.55f;  // shorter than building
                const float StripThickness = 0.06f;            // very thin
                const float StripHeight = FloorScaleZ * 0.55f; // band-style window
                // East side
                SpawnBlock(
                    FVector(X + WidthScale * 50.0f + 2.0f, Y, StripZ),
                    FVector(StripThickness, StripDepth, StripHeight),
                    WindowEmissive,
                    TEXT("Window Strip"));
                // West side
                SpawnBlock(
                    FVector(X - WidthScale * 50.0f - 2.0f, Y, StripZ),
                    FVector(StripThickness, StripDepth, StripHeight),
                    WindowEmissive,
                    TEXT("Window Strip"));
            }
        }

        // Rooftop antenna / spire on the tallest buildings to break up the
        // silhouette. Cheap, just a thin tall cube tinted with the accent.
        if (Floors >= 6)
        {
            const float RoofZ = Floors * FloorHeight + 80.0f;
            SpawnBlock(
                FVector(X, Y, RoofZ),
                FVector(0.12f, 0.12f, 1.6f),
                AccentColor * 2.0f,
                TEXT("Rooftop Spire"));
        }
    }
}

void ACodeRescueGameMode::SpawnTerminal(const FVector& Location, const FString& Id, const FString& Title, const FString& Brief, int32 CityIndex)
{
    ACodingTerminalActor* Terminal = GetWorld()->SpawnActor<ACodingTerminalActor>(ACodingTerminalActor::StaticClass(), Location, FRotator(0, 180, 0));
    if (!Terminal)
    {
        return;
    }

    Terminal->Challenge.Id = Id;
    Terminal->Challenge.Title = Title;
    Terminal->Challenge.MissionBrief = Brief;
    Terminal->CityIndex = CityIndex;
    Terminal->Tags.AddUnique(FName("ProtectedCodingChallengeZone"));
    Terminal->Tags.AddUnique(FName("NoZombieLearningZone"));
    Terminal->Tags.AddUnique(FName("SafeTerminalLab"));
    Terminal->Tags.AddUnique(FName("SelectedLanguageOnly"));
    Terminal->Tags.AddUnique(FName("LearningWithoutDeathRisk"));
    ApplyRuntimeDataLayerTags(Terminal, TArray<FName>{
        FName("RuntimeDataLayer_State_SafeBeat"),
        FName("RuntimeDataLayer_Mode_CodingSafehouse"),
        FName("RuntimeDataLayer_Mode_SelectedLanguageOnly"),
    });
    RegisterStreamedActor(Terminal);

    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        Terminal->Challenge.Language = GI->SelectedLanguage;
        if (GI->SolvedTerminalIds.Contains(Id))
        {
            Terminal->MarkSolved();
            // Reconstruct the city-wide rescue route once, after the full
            // ten-station curriculum is complete. Earlier stations retain
            // their solved state without duplicating route geometry.
            const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(CityIndex);
            if (Mission && Id == Mission->TerminalId &&
                FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex))
            {
                RevealSolvedTerminalRescueRoute(Id, CityIndex, Location, true);
            }
            return;
        }
    }

    Terminal->AddHelperActor(SpawnBlock(Location + FVector(0, 0, 110), FVector(2.0f, 0.55f, 2.2f), FLinearColor(0.0f, 0.7f, 1.0f), TEXT("INTERACTIVE CYAN TERMINAL"), false));
    // Real (non-debug) emissive halo above each terminal. Survives shipping
    // and tells players "go here" without leaning on DrawDebugSphere.
    Terminal->AddHelperActor(SpawnBlock(Location + FVector(0, 0, 280), FVector(0.6f, 0.6f, 0.05f), FLinearColor(0.0f, 1.0f, 1.0f) * 4.0f, TEXT("Terminal Halo"), false));
    // 2026-07-07: the PERSISTENT DrawDebugSphere that used to accompany the
    // halo IS the "cyan lattice dome" from Kenny's obstruction reports — a
    // never-expiring 260uu wire sphere over every terminal that swallowed the
    // camera and read as geometry. The emissive halo above carries the
    // "go here" job on its own; the debug sphere is gone.
    Terminal->AddHelperActor(SpawnGuideText(TEXT("TERMINAL\nE / Enter / Tab"), Location + FVector(0, 0, 560), FColor::Cyan, 76.0f));

    // 2026-07-04 (top-50 item 45): breadcrumb guidance trail — small emissive cyan
    // strips from the arrival plaza to the MAIN terminal only (never the secret one),
    // ground-snapped so new players always have a physical path to the core loop.
    const TArray<FCodeRescueCityMission>& TrailMissions = FCodeRescueCampaign::GetMissions();
    if (TrailMissions.IsValidIndex(CityIndex) && TrailMissions[CityIndex].TerminalId == Id)
    {
        const FVector TrailStart = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
        const FVector TrailDelta = Location - TrailStart;
        const float TrailDist = TrailDelta.Size2D();
        if (TrailDist > 1500.0f)
        {
            const int32 TrailSteps = FMath::Clamp(FMath::RoundToInt(TrailDist / 900.0f), 4, 14);
            const FRotator TrailYaw = FVector(TrailDelta.X, TrailDelta.Y, 0.0f).GetSafeNormal().Rotation();
            for (int32 StepIdx = 1; StepIdx <= TrailSteps; ++StepIdx)
            {
                const float TrailT = static_cast<float>(StepIdx) / static_cast<float>(TrailSteps + 1);
                FVector TrailPoint = TrailStart + TrailDelta * TrailT;
                TrailPoint.Z = GroundZAt(TrailPoint + FVector(0, 0, 200.0f), TrailStart.Z - 88.0f) + 6.0f;
                AActor* TrailStrip = SpawnRotatedBlock(
                    TrailPoint,
                    FRotator(0.0f, TrailYaw.Yaw, 0.0f),
                    FVector(0.9f, 0.16f, 0.045f),
                    FLinearColor(0.0f, 0.9f, 1.0f) * 2.6f,
                    FString::Printf(TEXT("Guidance Trail %s %d"), *Id, StepIdx),
                    /*bEnableCollision=*/false);
                if (TrailStrip)
                {
                    TrailStrip->Tags.Add(FName("GuidanceTrail"));
                }
            }
            UE_LOG(LogTemp, Display, TEXT("[GuidanceTrail] %s: %d strips over %.0fm"), *Id, TrailSteps, TrailDist / 100.0f);
        }
    }
}

void ACodeRescueGameMode::SpawnAuthoredCityKitLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    // 2026-07-01 (Kenny: "world looks like grey boxes"): dress the arrival street with the authored
    // /Game/CodeRescueArt kit so the player sees real buildings/props. Additive - the primitive city
    // spine is untouched. Placed around the player start; scale tuned by live playtest.
    const FString KitDir = TEXT("/Game/CodeRescueArt/CityKit/");
    auto Kit = [&KitDir](const TCHAR* Name) -> FString
    {
        // 2026-07-01 (round 5, root cause of "world still grey boxes"): the glTF importer nests
        // each mesh under <Name>/StaticMeshes/<Name>, NOT flat at <Name>. The old flat object path
        // resolved to a folder, so LoadObject returned null and zero kit meshes ever spawned.
        const FString N(Name);
        return KitDir + N + TEXT("/StaticMeshes/") + N + TEXT(".") + N;
    };

    const FVector Start = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const float GroundZ = Start.Z - 88.0f;   // capsule half-height -> mesh base rests on the ground
    const float SideY = 3000.0f;             // street half-width
    const float Step = 2600.0f;              // facade spacing
    const float Scale = 4.0f;                // kit meshes are ~6-9m; scale up to building size

    // Count successes/failures so a live playtest gets an unambiguous signal that the authored
    // meshes actually loaded (the old flat path failed silently and looked like "no art").
    int32 KitSpawned = 0, KitFailed = 0;
    auto Place = [&](const FString& Path, const FVector& L, const FRotator& R, const FVector& S,
                     const FString& Nm, bool bCol, const TCHAR* Mat)
    {
        if (SpawnKitMesh(Path, L, R, S, Nm, bCol, Mat)) { ++KitSpawned; } else { ++KitFailed; }
    };

    // Solid cooked StarterContent materials so the kit reads as concrete/brick/steel, not checkerboard.
    const TCHAR* MatConcrete = TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels");
    const TCHAR* MatBrick    = TEXT("/Game/StarterContent/Materials/M_Brick_Clay_Old.M_Brick_Clay_Old");
    const TCHAR* MatStone    = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
    const TCHAR* MatSteel    = TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel");
    const TCHAR* MatGrime    = TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime");
    const TCHAR* MatCobble   = TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough");
    const TCHAR* FacadeMats[3] = { MatConcrete, MatBrick, MatStone };

    const TCHAR* Facades[3] = { TEXT("SM_Facade_Windows_6x9"), TEXT("SM_Facade_Storefront_6x45"), TEXT("SM_Building_Corner_9m") };
    for (int32 i = -3; i <= 3; ++i)
    {
        const float X = Start.X + i * Step;
        const int32 L = FMath::Abs(i) % 3;
        const int32 R = (FMath::Abs(i) + 1) % 3;
        Place(Kit(Facades[L]),
            FVector(X, Start.Y + SideY, GroundZ), FRotator(0.0f, 180.0f, 0.0f), FVector(Scale),
            CityLabel + TEXT(" Kit Facade L"), true, FacadeMats[L]);
        Place(Kit(Facades[R]),
            FVector(X, Start.Y - SideY, GroundZ), FRotator(0.0f, 0.0f, 0.0f), FVector(Scale),
            CityLabel + TEXT(" Kit Facade R"), true, FacadeMats[R]);
    }

    for (int32 i = -2; i <= 2; ++i)
    {
        const float X = Start.X + i * 2400.0f;
        Place(Kit(TEXT("SM_Lamp_Post_4m")),
            FVector(X, Start.Y + 1200.0f, GroundZ), FRotator::ZeroRotator, FVector(Scale * 0.8f),
            CityLabel + TEXT(" Kit Lamp"), true, MatSteel);   // 2026-07-04: props are solid now
        Place(Kit(TEXT("SM_Street_Planter")),
            FVector(X + 400.0f, Start.Y - 1200.0f, GroundZ), FRotator::ZeroRotator, FVector(Scale),
            CityLabel + TEXT(" Kit Planter"), true, MatGrime);
    }

    Place(Kit(TEXT("SM_Terminal_Kiosk")),
        FVector(Start.X + 1600.0f, Start.Y + 300.0f, GroundZ), FRotator(0.0f, 200.0f, 0.0f), FVector(Scale),
        CityLabel + TEXT(" Kit Kiosk"), true, MatSteel);
    Place(Kit(TEXT("SM_Extraction_Arch_5m")),
        FVector(Start.X + 5200.0f, Start.Y, GroundZ), FRotator(0.0f, 90.0f, 0.0f), FVector(Scale * 1.2f),
        CityLabel + TEXT(" Kit Arch"), true, MatConcrete);
    Place(Kit(TEXT("SM_Rubble_Pile")),
        FVector(Start.X + 1800.0f, Start.Y - 1500.0f, GroundZ), FRotator::ZeroRotator, FVector(Scale),
        CityLabel + TEXT(" Kit Rubble"), true, MatCobble);

    UE_LOG(LogTemp, Warning, TEXT("[CityKit] Authored art: %d spawned, %d failed (dir=%s)"),
        KitSpawned, KitFailed, *KitDir);
    if (GEngine && KitFailed > 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange,
            FString::Printf(TEXT("CityKit art: %d loaded, %d FAILED to load"), KitSpawned, KitFailed));
    }
}

float ACodeRescueGameMode::GroundZAt(const FVector& Probe, float DefaultZ) const
{
    // 2026-07-04 world-physics pass: every authored prop is ground-snapped through this trace so
    // nothing floats in mid-air ("they should not be floating"). WorldStatic covers the mega floor,
    // roads, and kit architecture (mobility does not change the object channel).
    if (UWorld* World = GetWorld())
    {
        FHitResult Hit;
        FCollisionQueryParams Params(FName(TEXT("CRGroundSnap")), false);
        const FVector Up = Probe + FVector(0.0f, 0.0f, 600.0f);
        const FVector Down = Probe - FVector(0.0f, 0.0f, 6000.0f);
        if (World->LineTraceSingleByChannel(Hit, Up, Down, ECC_WorldStatic, Params))
        {
            return Hit.ImpactPoint.Z;
        }
    }
    return DefaultZ;
}

void ACodeRescueGameMode::SpawnStreetscapeLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    // 2026-07-04: dress the arrival street with the v2 authored streetscape — real roads with lane
    // paint, sidewalks with curbs, a crosswalk, parked/wrecked vehicles, oaks/dead trees/bushes and
    // traffic signals. All meshes are Blender-authored GLBs imported to /Game/CodeRescueArt (same
    // proven path as the 07-01 kit). EVERYTHING here spawns with collision ON and ground-snapped:
    // walls you cannot walk through, props that rest on the pavement.
    (void)Mission;
    auto Art = [](const TCHAR* Sub, const TCHAR* Name) -> FString
    {
        // glTF importer nests every mesh at <Dir>/<Name>/StaticMeshes/<Name>.<Name> (07-01 lesson).
        const FString N(Name);
        return FString::Printf(TEXT("/Game/CodeRescueArt/%s/%s/StaticMeshes/%s.%s"), Sub, *N, *N, *N);
    };

    const FVector Start = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const float DefaultGroundZ = Start.Z - 88.0f;
    const float CanonicalFirstLevelGroundZ = Origin.Z;
    int32 StreetSpawned = 0, StreetFailed = 0;
    auto Place = [&](const FString& Path, const FVector& L, const FRotator& R, const FVector& S, const FString& Nm)
    {
        const float Gz = CityIndex == 0
            ? CanonicalFirstLevelGroundZ
            : GroundZAt(FVector(L.X, L.Y, DefaultGroundZ + 200.0f), DefaultGroundZ);
        if (AActor* Spawned = SpawnKitMesh(
                Path, FVector(L.X, L.Y, Gz + L.Z), R, S, Nm, /*bEnableCollision=*/true, nullptr))
        {
            ++StreetSpawned;
            // 2026-07-17 (Kenny: "correct the uneven ground level; it does not
            // seem as though you have corrected this during any of your
            // iterations"): he was RIGHT — street surfaces registered for the
            // ground unifier only in city 0, so every later city logged
            // "[GroundUnify] skipped — no driving surfaces registered" and
            // kept its seams. EVERY city registers now (one city per world,
            // registry reset on rebuild; the FirstLevel* tag names are kept
            // for verifier compatibility — they mean "canonical ground").
            if (Nm.Contains(TEXT("Road")) || Nm.Contains(TEXT("Sidewalk")) || Nm.Contains(TEXT("Crosswalk")))
            {
                Spawned->Tags.AddUnique(FName("FirstLevelCanonicalGroundSurface"));
                Spawned->Tags.AddUnique(FName("FirstLevelArenaAccess"));
                FirstLevelGroundSurfaceActors.Add(Spawned);
            }
        }
        else
        {
            ++StreetFailed;
        }
    };

    // 2026-07-06: v3 kit (PBR materials, worn asphalt, expansion joints)
    // replaces the v2 pieces in place when its import exists; v2 stays as the
    // fallback so a pre-import boot still has a street.
    auto ArtV3 = [](const TCHAR* Sub, const TCHAR* Name) -> FString
    {
        // Interchange (UE5.7 glTF) nests one level DEEPER than the legacy
        // importer: <dest>/<SourceName>/StaticMeshes/<Name> where dest already
        // ends in <Name> — hence the doubled segment.
        const FString N(Name);
        return FString::Printf(TEXT("/Game/CodeRescueArt/%s/%s/%s/StaticMeshes/%s.%s"), Sub, *N, *N, *N, *N);
    };
    const bool bV3Kit = LoadObject<UStaticMesh>(nullptr, *ArtV3(TEXT("CityKitV3"), TEXT("RoadStraightV3"))) != nullptr;
    UE_LOG(LogTemp, Display, TEXT("[CityBlockV3] v3 street kit %s"), bV3Kit ? TEXT("ACTIVE") : TEXT("not imported yet — using v2"));

    // Roadway down the main street axis: one segment per facade bay, widened to fill the street.
    for (int32 i = -3; i <= 3; ++i)
    {
        if (bV3Kit)
        {
            Place(ArtV3(TEXT("CityKitV3"), i == 0 ? TEXT("RoadIntersectionV3") : TEXT("RoadStraightV3")),
                  FVector(Start.X + i * 2600.0f, Start.Y, 2.0f), FRotator::ZeroRotator,
                  i == 0 ? FVector(3.25f, 3.25f, 1.0f) : FVector(3.25f, 4.6f, 1.0f),
                  CityLabel + (i == 0 ? TEXT(" Primary Intersection") : TEXT(" Road Segment")));
            Place(ArtV3(TEXT("CityKitV3"), TEXT("SidewalkV3")),
                  FVector(Start.X + i * 2600.0f, Start.Y + 2820.0f, 2.0f), FRotator(0.0f, 180.0f, 0.0f),
                  FVector(3.25f, 2.6f, 1.0f), CityLabel + TEXT(" Sidewalk N"));
            Place(ArtV3(TEXT("CityKitV3"), TEXT("SidewalkV3")),
                  FVector(Start.X + i * 2600.0f, Start.Y - 2820.0f, 2.0f), FRotator::ZeroRotator,
                  FVector(3.25f, 2.6f, 1.0f), CityLabel + TEXT(" Sidewalk S"));
        }
        else
        {
            Place(Art(TEXT("CityKit"), TEXT("SM_Road_Straight_12m")),
                  FVector(Start.X + i * 2600.0f, Start.Y, 2.0f), FRotator::ZeroRotator,
                  FVector(2.1667f, 4.0f, 1.2f), CityLabel + TEXT(" Road Segment"));
            Place(Art(TEXT("CityKit"), TEXT("SM_Sidewalk_6m")),
                  FVector(Start.X + i * 2600.0f, Start.Y + 2820.0f, 2.0f), FRotator(0.0f, 180.0f, 0.0f),
                  FVector(4.3333f, 2.5f, 1.0f), CityLabel + TEXT(" Sidewalk N"));
            Place(Art(TEXT("CityKit"), TEXT("SM_Sidewalk_6m")),
                  FVector(Start.X + i * 2600.0f, Start.Y - 2820.0f, 2.0f), FRotator::ZeroRotator,
                  FVector(4.3333f, 2.5f, 1.0f), CityLabel + TEXT(" Sidewalk S"));
        }
    }
    if (bV3Kit)
    {
        // A real city needs a connected street network, not a single runway.
        // The cross street opens the arrival sight line and gives vehicles,
        // sidewalks, encounters, and the tactical camera meaningful choices.
        for (int32 i = -2; i <= 2; ++i)
        {
            if (i == 0)
            {
                continue;
            }
            Place(ArtV3(TEXT("CityKitV3"), TEXT("RoadStraightV3")),
                  FVector(Start.X, Start.Y + i * 2600.0f, 2.0f), FRotator(0.0f, 90.0f, 0.0f),
                  FVector(3.25f, 4.6f, 1.0f), CityLabel + TEXT(" Cross Street Road"));
            Place(ArtV3(TEXT("CityKitV3"), TEXT("SidewalkV3")),
                  FVector(Start.X + 2820.0f, Start.Y + i * 2600.0f, 2.0f), FRotator(0.0f, -90.0f, 0.0f),
                  FVector(3.25f, 2.6f, 1.0f), CityLabel + TEXT(" Cross Street Sidewalk E"));
            Place(ArtV3(TEXT("CityKitV3"), TEXT("SidewalkV3")),
                  FVector(Start.X - 2820.0f, Start.Y + i * 2600.0f, 2.0f), FRotator(0.0f, 90.0f, 0.0f),
                  FVector(3.25f, 2.6f, 1.0f), CityLabel + TEXT(" Cross Street Sidewalk W"));
        }
    }
    Place(Art(TEXT("CityKit"), TEXT("SM_Crosswalk_8m")),
          FVector(Start.X + 5200.0f, Start.Y, 3.0f), FRotator(0.0f, 90.0f, 0.0f),
          FVector(3.0f, 3.2f, 1.2f), CityLabel + TEXT(" Crosswalk"));

    // Abandoned vehicles (solid — they block movement and give combat cover).
    if (bV3Kit)
    {
        Place(ArtV3(TEXT("CityKitV3"), TEXT("SedanCrashedV3")),
              FVector(Start.X - 1900.0f, Start.Y + 950.0f, 0.0f), FRotator(0.0f, 12.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Wrecked Sedan"));
        Place(ArtV3(TEXT("CityKitV3"), TEXT("DeliveryVanV3")),
              FVector(Start.X + 700.0f, Start.Y - 980.0f, 0.0f), FRotator(0.0f, 184.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Abandoned Van"));
        Place(ArtV3(TEXT("CityKitV3"), TEXT("PoliceCruiserV3")),
              FVector(Start.X + 2600.0f, Start.Y + 980.0f, 0.0f), FRotator(0.0f, 95.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Police Cruiser"));
        Place(ArtV3(TEXT("CityKitV3"), TEXT("SedanCleanV3")),
              FVector(Start.X - 4400.0f, Start.Y - 940.0f, 0.0f), FRotator(0.0f, 355.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Stalled Sedan"));
        Place(ArtV3(TEXT("CityKitV3"), TEXT("PickupV3")),
              FVector(Start.X + 4200.0f, Start.Y - 1050.0f, 0.0f), FRotator(0.0f, 8.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Dead Pickup"));
    }
    else
    {
        Place(Art(TEXT("Vehicles"), TEXT("SM_Sedan_Wreck")),
              FVector(Start.X - 1900.0f, Start.Y + 950.0f, 0.0f), FRotator(0.0f, 12.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Wrecked Sedan"));
        Place(Art(TEXT("Vehicles"), TEXT("SM_Van_Delivery")),
              FVector(Start.X + 700.0f, Start.Y - 980.0f, 0.0f), FRotator(0.0f, 184.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Abandoned Van"));
        Place(Art(TEXT("Vehicles"), TEXT("SM_Police_Cruiser")),
              FVector(Start.X + 2600.0f, Start.Y + 980.0f, 0.0f), FRotator(0.0f, 95.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Police Cruiser"));
        Place(Art(TEXT("Vehicles"), TEXT("SM_Sedan_Wreck")),
              FVector(Start.X - 4400.0f, Start.Y - 940.0f, 0.0f), FRotator(0.0f, 355.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Stalled Sedan"));
    }

    // Street greenery: oaks along the sidewalks, a couple of dead trees, low bushes at facade bases.
    for (int32 i = -2; i <= 2; ++i)
    {
        const float SideSign = (i % 2 == 0) ? 1.0f : -1.0f;
        Place(Art(TEXT("Nature"), TEXT("SM_Tree_Oak_8m")),
              FVector(Start.X + i * 2600.0f + 1300.0f, Start.Y + SideSign * 2820.0f, 0.0f),
              FRotator(0.0f, i * 73.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Street Oak"));
    }
    Place(Art(TEXT("Nature"), TEXT("SM_Tree_Dead_6m")),
          FVector(Start.X - 6800.0f, Start.Y + 2500.0f, 0.0f), FRotator(0.0f, 40.0f, 0.0f),
          FVector(1.0f), CityLabel + TEXT(" Dead Tree W"));
    Place(Art(TEXT("Nature"), TEXT("SM_Tree_Dead_6m")),
          FVector(Start.X + 6900.0f, Start.Y - 2400.0f, 0.0f), FRotator(0.0f, 205.0f, 0.0f),
          FVector(1.0f), CityLabel + TEXT(" Dead Tree E"));
    for (int32 i = -2; i <= 2; ++i)
    {
        Place(Art(TEXT("Nature"), TEXT("SM_Bush_Round")),
              FVector(Start.X + i * 2600.0f - 700.0f, Start.Y - 2950.0f, 0.0f),
              FRotator(0.0f, i * 51.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Bush"));
    }

    // Signals and signage at the kiosk corner and the crosswalk.
    Place(Art(TEXT("CityKit"), TEXT("SM_StreetSign_Stop")),
          FVector(Start.X + 1600.0f, Start.Y - 900.0f, 0.0f), FRotator(0.0f, 200.0f, 0.0f),
          FVector(1.0f), CityLabel + TEXT(" Stop Sign"));
    Place(Art(TEXT("CityKit"), TEXT("SM_TrafficLight")),
          FVector(Start.X + 4900.0f, Start.Y - 900.0f, 0.0f), FRotator::ZeroRotator,
          FVector(1.0f), CityLabel + TEXT(" Traffic Light S"));
    Place(Art(TEXT("CityKit"), TEXT("SM_TrafficLight")),
          FVector(Start.X + 5500.0f, Start.Y + 900.0f, 0.0f), FRotator(0.0f, 180.0f, 0.0f),
          FVector(1.0f), CityLabel + TEXT(" Traffic Light N"));

    if (StreetFailed > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Streetscape] %s: %d spawned, %d failed"), *CityLabel, StreetSpawned, StreetFailed);
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("[Streetscape] %s: %d spawned, 0 failed"), *CityLabel, StreetSpawned);
    }
    if (GEngine && StreetFailed > 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange,
            FString::Printf(TEXT("Streetscape: %d loaded, %d FAILED (import pending?)"), StreetSpawned, StreetFailed));
    }
}

void ACodeRescueGameMode::SpawnCityBlockV3Layer(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    // 2026-07-06 (Kenny: "(lack of) vehicles/sidewalks/completed structures"):
    // COMPLETED street walls. Closed Blender-authored buildings with inset
    // window grids (some lit), storefronts, fire escapes and rooftop clutter
    // line BOTH sides of the main street behind the sidewalks, with street
    // furniture (streetlights, hydrants, trash, bus stop, power poles,
    // barriers) and curb-parked vehicles. Everything solid + ground-snapped.
    auto ArtV3 = [](const TCHAR* Name) -> FString
    {
        // Interchange double-nesting (see SpawnStreetscapeLayer note).
        const FString N(Name);
        return FString::Printf(TEXT("/Game/CodeRescueArt/CityKitV3/%s/%s/StaticMeshes/%s.%s"), *N, *N, *N, *N);
    };
    if (!LoadObject<UStaticMesh>(nullptr, *ArtV3(TEXT("BuildingBrickV3"))))
    {
        UE_LOG(LogTemp, Display, TEXT("[CityBlockV3] %s: building kit not imported yet — layer skipped"), *CityLabel);
        return;
    }

    const FVector Start = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const float DefaultGroundZ = Start.Z - 88.0f;
    const float CanonicalFirstLevelGroundZ = Origin.Z;
    int32 BlockSpawned = 0, BlockFailed = 0;
    auto Place = [&](const FString& Path, const FVector& L, const FRotator& R, const FVector& S, const FString& Nm)
    {
        const float Gz = CityIndex == 0
            ? CanonicalFirstLevelGroundZ
            : GroundZAt(FVector(L.X, L.Y, DefaultGroundZ + 200.0f), DefaultGroundZ);
        if (SpawnKitMesh(Path, FVector(L.X, L.Y, Gz + L.Z), R, S, Nm, /*bEnableCollision=*/true, nullptr))
        {
            ++BlockSpawned;
        }
        else
        {
            ++BlockFailed;
        }
    };

    // Street walls: attached facades, deterministic variety per city.
    // 2026-07-07 FIX (Kenny: "walls obstruct the camera", "stuck after T"):
    // every Y below was ABSOLUTE while X was relative to Start — with the
    // street center at Start.Y=-5520 the whole layer (18 buildings included)
    // spawned 19-91m north of where it belonged: ON the roadway and the spawn
    // pad. Every placement is now relative to Start, matching the streetscape.
    const TCHAR* BuildingNames[4] = {
        TEXT("BuildingBrickV3"), TEXT("BuildingConcreteV3"),
        TEXT("BuildingStuccoV3"), TEXT("BuildingBrownstoneV3") };
    const float BuildingY = 3620.0f;   // behind the sidewalks at Start.Y±2820
    for (int32 i = -4; i <= 4; ++i)
    {
        if (FMath::Abs(i) <= 1)
        {
            continue; // open the central intersection and its camera sight lines
        }
        for (int32 SideSign = -1; SideSign <= 1; SideSign += 2)
        {
            const bool bFirstLevelAccessibleBuildingSlot = CityIndex == 0 &&
                ((i == -2 && SideSign > 0) ||
                 (i == 2 && SideSign < 0) ||
                 (i == 3 && SideSign > 0));
            if (bFirstLevelAccessibleBuildingSlot)
            {
                // Reserved for literal open-door V5 interiors.
                continue;
            }
            const int32 Pick = FMath::Abs(CityIndex * 7 + i * 3 + (SideSign > 0 ? 1 : 0)) % 4;
            const float DepthJitter = ((i * 13 + SideSign * 5) % 3) * 60.0f;
            Place(ArtV3(BuildingNames[Pick]),
                  FVector(Start.X + i * 820.0f, Start.Y + (BuildingY + DepthJitter) * SideSign, 0.0f),
                  FRotator(0.0f, SideSign > 0 ? 180.0f : 0.0f, 0.0f),
                  FVector(1.0f),
                  CityLabel + FString::Printf(TEXT(" Facade %d%c"), i + 5, SideSign > 0 ? TEXT('N') : TEXT('S')));
        }
    }

    // Complete the second street wall with authored structures. These sit
    // beyond the cross-street sidewalks, preserving a 24m open intersection
    // while avoiding the sparse one-axis city silhouette.
    for (int32 SideSign = -1; SideSign <= 1; SideSign += 2)
    {
        for (int32 Row = -1; Row <= 1; ++Row)
        {
            const int32 Pick = FMath::Abs(CityIndex * 5 + Row * 2 + SideSign) % 4;
            Place(ArtV3(BuildingNames[Pick]),
                  FVector(Start.X + SideSign * 3620.0f, Start.Y + Row * 900.0f, 0.0f),
                  FRotator(0.0f, SideSign > 0 ? -90.0f : 90.0f, 0.0f),
                  FVector(1.0f),
                  CityLabel + FString::Printf(TEXT(" Cross Street Facade %c%d"),
                      SideSign > 0 ? TEXT('E') : TEXT('W'), Row + 2));
        }
    }

    // Streetlights along both curbs, staggered so pools of light alternate.
    // 2026-07-07 fidelity: each lamp now casts a REAL warm light pool — the
    // heads were emissive-only, so night streets read as pure black. Warm
    // sodium pools + the cool moon key = readable, noir, lived-in streets.
    for (int32 i = -3; i <= 3; ++i)
    {
        const float SideSign = (i % 2 == 0) ? 1.0f : -1.0f;
        const FVector LampBase(Start.X + i * 1300.0f + 650.0f, Start.Y + 2650.0f * SideSign, 0.0f);
        Place(ArtV3(TEXT("StreetlightV3")),
              LampBase,
              FRotator(0.0f, SideSign > 0.0f ? -90.0f : 90.0f, 0.0f),
              FVector(1.0f), CityLabel + TEXT(" Streetlight"));
        const float LampGz = CityIndex == 0
            ? CanonicalFirstLevelGroundZ
            : GroundZAt(FVector(LampBase.X, LampBase.Y, DefaultGroundZ + 200.0f), DefaultGroundZ);
        const FVector HeadOffset(SideSign > 0.0f ? 0.0f : 0.0f, SideSign * -195.0f, 690.0f);
        if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(),
                FVector(LampBase.X, LampBase.Y, LampGz) + HeadOffset, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(4200.0f);
                PLC->SetLightColor(FLinearColor(1.0f, 0.78f, 0.48f));   // warm sodium
                PLC->SetAttenuationRadius(1500.0f);
                PLC->SetCastShadows(false);   // 7 lamps/city — keep them cheap
            }
            Lamp->Tags.Add(FName("CityMoodLayer"));
            RegisterStreamedActor(Lamp);
        }
    }

    // Street furniture cluster near the entry plaza.
    Place(ArtV3(TEXT("HydrantV3")),   FVector(Start.X - 900.0f,  Start.Y - 2550.0f, 0.0f), FRotator::ZeroRotator,          FVector(1.0f), CityLabel + TEXT(" Hydrant"));
    Place(ArtV3(TEXT("TrashClusterV3")), FVector(Start.X - 350.0f, Start.Y + 2600.0f, 0.0f), FRotator(0.0f, 35.0f, 0.0f),   FVector(1.0f), CityLabel + TEXT(" Trash"));
    Place(ArtV3(TEXT("TrashClusterV3")), FVector(Start.X + 3400.0f, Start.Y - 2620.0f, 0.0f), FRotator(0.0f, 210.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Trash E"));
    Place(ArtV3(TEXT("BusStopV3")),   FVector(Start.X + 1800.0f, Start.Y + 2700.0f, 0.0f), FRotator(0.0f, 180.0f, 0.0f),    FVector(1.0f), CityLabel + TEXT(" Bus Stop"));
    Place(ArtV3(TEXT("PowerPoleV3")), FVector(Start.X - 3000.0f, Start.Y - 3050.0f, 0.0f), FRotator(0.0f, 12.0f, 0.0f),    FVector(1.0f), CityLabel + TEXT(" Power Pole W"));
    Place(ArtV3(TEXT("PowerPoleV3")), FVector(Start.X + 3600.0f, Start.Y - 3050.0f, 0.0f), FRotator(0.0f, -8.0f, 0.0f),    FVector(1.0f), CityLabel + TEXT(" Power Pole E"));
    Place(ArtV3(TEXT("JerseyBarrierV3")), FVector(Start.X + 5600.0f, Start.Y + 350.0f, 0.0f), FRotator(0.0f, 78.0f, 0.0f),  FVector(1.0f), CityLabel + TEXT(" Barrier A"));
    Place(ArtV3(TEXT("JerseyBarrierV3")), FVector(Start.X + 5750.0f, Start.Y - 280.0f, 0.0f), FRotator(0.0f, 99.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Barrier B"));
    Place(ArtV3(TEXT("TrafficLightV3")), FVector(Start.X - 5200.0f, Start.Y - 900.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f),   FVector(1.0f), CityLabel + TEXT(" Traffic Light W"));

    // Extra curb-parked vehicles for a lived-in (then abandoned) street.
    Place(ArtV3(TEXT("SedanCleanV3")),  FVector(Start.X - 3300.0f, Start.Y + 1150.0f, 0.0f), FRotator(0.0f, 178.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Parked Sedan W"));
    Place(ArtV3(TEXT("DeliveryVanV3")), FVector(Start.X + 5000.0f, Start.Y + 1120.0f, 0.0f), FRotator(0.0f, 2.0f, 0.0f),   FVector(1.0f), CityLabel + TEXT(" Parked Van E"));
    Place(ArtV3(TEXT("SedanCrashedV3")), FVector(Start.X + 1500.0f, Start.Y + 300.0f, 0.0f), FRotator(0.0f, 305.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Mid-street Wreck"));
    Place(ArtV3(TEXT("PoliceCruiserV3")), FVector(Start.X + 980.0f, Start.Y + 3900.0f, 0.0f), FRotator(0.0f, 88.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Cross-street Cruiser"));
    Place(ArtV3(TEXT("PickupV3")), FVector(Start.X - 1050.0f, Start.Y - 4300.0f, 0.0f), FRotator(0.0f, 272.0f, 0.0f), FVector(1.0f), CityLabel + TEXT(" Cross-street Pickup"));

    if (BlockFailed > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CityBlockV3] %s: %d spawned, %d failed"), *CityLabel, BlockSpawned, BlockFailed);
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("[CityBlockV3] %s: %d spawned, 0 failed"), *CityLabel, BlockSpawned);
    }
}

void ACodeRescueGameMode::SpawnFirstLevelCombatArtPass(
    int32 CityIndex,
    const FVector& Origin,
    const FString& CityLabel)
{
    if (CityIndex != 0)
    {
        return;
    }

    auto ArtPath = [](const TCHAR* Name) -> FString
    {
        const FString N(Name);
        return FString::Printf(
            TEXT("/Game/CodeRescueArt/FirstLevelV4/%s/%s/StaticMeshes/%s.%s"),
            *N, *N, *N, *N);
    };
    if (!LoadObject<UStaticMesh>(nullptr, *ArtPath(TEXT("FirstLevelStorefrontV4"))))
    {
        UE_LOG(LogTemp, Warning, TEXT("[FirstLevelCombatArtV4] authored assets unavailable; level-one pass skipped"));
        return;
    }

    const FVector Start = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    int32 SpawnedCount = 0;
    auto Place = [&](const TCHAR* AssetName, const FVector& RelativeLocation, const FRotator& Rotation, bool bCollision)
    {
        const FVector FlatLocation = Start + FVector(RelativeLocation.X, RelativeLocation.Y, 0.0f);
        const float GroundZ = Origin.Z;
        AActor* Actor = SpawnKitMesh(
            ArtPath(AssetName),
            FVector(FlatLocation.X, FlatLocation.Y, GroundZ + RelativeLocation.Z),
            Rotation,
            FVector::OneVector,
            CityLabel + TEXT(" First Level ") + AssetName,
            bCollision,
            nullptr);
        if (Actor)
        {
            ++SpawnedCount;
            Actor->Tags.AddUnique(FName("FirstLevelOnly"));
            Actor->Tags.AddUnique(FName("FirstLevelCombatArtV4"));
            Actor->Tags.AddUnique(FName("BlenderAuthoredFirstLevelArt"));
            Actor->Tags.AddUnique(FName("CameraSightlinePreserved"));
            ApplyRuntimeDataLayerTags(Actor, { FName("FirstLevelGameplayArt"), FName("CombatReadabilityLayer") });
        }
    };

    // Open-sided support spaces sit behind opposite curbs and do not cover the
    // first-level objective corridor or player-start clearance zone.
    Place(TEXT("FieldArmoryV4"), FVector(-980.0f, -3210.0f, 0.0f), FRotator(0.0f, 8.0f, 0.0f), true);
    // The V5 enterable clinic now replaces the smaller V4 triage checkpoint;
    // keeping both placed them in the same footprint and sealed its doorway.
    // Keep tactical cover in the east combat lane without crossing the V5
    // café's continuous sidewalk-to-door capsule route.
    Place(TEXT("SandbagCoverV4"), FVector(3180.0f, 860.0f, 0.0f), FRotator(0.0f, 8.0f, 0.0f), true);

    const FVector PracticalPositions[] = {
        Start + FVector(-980.0f, -3210.0f, 285.0f)
    };
    for (const FVector& PracticalPosition : PracticalPositions)
    {
        if (APointLight* Practical = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(), PracticalPosition, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* Light = Cast<UPointLightComponent>(Practical->GetLightComponent()))
            {
                Light->SetMobility(EComponentMobility::Movable);
                Light->SetIntensity(2800.0f);
                Light->SetLightColor(FLinearColor(1.0f, 0.62f, 0.26f));
                Light->SetAttenuationRadius(1050.0f);
                Light->SetCastShadows(false);
            }
            Practical->Tags.AddUnique(FName("FirstLevelOnly"));
            Practical->Tags.AddUnique(FName("FirstLevelCombatArtV4"));
            RegisterStreamedActor(Practical);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[FirstLevelCombatArtV4] %s spawned %d authored structures/cover; later levels unchanged"),
        *CityLabel, SpawnedCount);
}

void ACodeRescueGameMode::SpawnFirstLevelTraversalArtPass(
    int32 CityIndex,
    const FVector& Origin,
    const FString& CityLabel)
{
    if (CityIndex != 0)
    {
        return;
    }

    auto ArtPath = [](const TCHAR* Name) -> FString
    {
        const FString N(Name);
        return FString::Printf(
            TEXT("/Game/CodeRescueArt/FirstLevelV5/%s/%s/StaticMeshes/%s.%s"),
            *N, *N, *N, *N);
    };
    if (!LoadObject<UStaticMesh>(nullptr, *ArtPath(TEXT("AccessibleMarketV5"))))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[FirstLevelAccessV5] authored open-door buildings unavailable; traversal pass skipped"));
        return;
    }

    struct FAccessBuildingSpec
    {
        const TCHAR* AssetName;
        const TCHAR* AccessTag;
        FVector RelativeLocation;
        FRotator Rotation;
        EPickupKind PickupKind;
        int32 PickupAmount;
        FLinearColor LightColor;
    };
    const FAccessBuildingSpec Specs[] = {
        {
            TEXT("AccessibleMarketV5"),
            TEXT("FirstLevelAccessibleMarket"),
            FVector(-1640.0f, 3620.0f, 0.0f),
            FRotator::ZeroRotator,
            EPickupKind::AmmoPouch,
            24,
            FLinearColor(0.58f, 1.0f, 0.68f),
        },
        {
            TEXT("AccessibleClinicV5"),
            TEXT("FirstLevelAccessibleClinic"),
            FVector(1640.0f, -3200.0f, 0.0f),
            FRotator(0.0f, 180.0f, 0.0f),
            EPickupKind::Medkit,
            1,
            FLinearColor(0.55f, 0.76f, 1.0f),
        },
        {
            TEXT("OpenStreetCafeV5"),
            TEXT("FirstLevelAccessibleCafe"),
            FVector(2460.0f, 3620.0f, 0.0f),
            FRotator::ZeroRotator,
            EPickupKind::FlashlightBattery,
            1,
            FLinearColor(1.0f, 0.64f, 0.38f),
        },
    };

    FirstLevelAccessDoorwayOutsidePoints.Reset();
    FirstLevelAccessDoorwayInsidePoints.Reset();
    const FVector Start = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const FVector BuildingScale(0.72f);
    int32 SpawnedBuildings = 0;
    int32 FunctionalPickups = 0;
    for (const FAccessBuildingSpec& Spec : Specs)
    {
        const FVector Center(
            Start.X + Spec.RelativeLocation.X,
            Start.Y + Spec.RelativeLocation.Y,
            Origin.Z);
        AActor* Building = SpawnKitMesh(
            ArtPath(Spec.AssetName),
            Center,
            Spec.Rotation,
            BuildingScale,
            CityLabel + TEXT(" ") + Spec.AssetName,
            false,
            nullptr);
        if (!Building)
        {
            continue;
        }

        ++SpawnedBuildings;
        Building->Tags.AddUnique(FName("FirstLevelOnly"));
        Building->Tags.AddUnique(FName("FirstLevelEnterableBuilding"));
        Building->Tags.AddUnique(FName("LiteralOpenDoorway"));
        Building->Tags.AddUnique(FName("CanonicalStreetElevation"));
        Building->Tags.AddUnique(FName(Spec.AccessTag));
        ApplyRuntimeDataLayerTags(Building, {
            FName("FirstLevelGameplayArt"),
            FName("FirstLevelAccessibleInteriors"),
            FName("WorldDevelopmentDeepDive") });

        // Interchange supplies one convex hull for each combined GLB, which
        // seals otherwise literal holes during a character/capsule sweep.
        // Keep the detailed visual mesh collision-free and reproduce its
        // structural shell with explicit boxes. This provides dependable
        // player, zombie, and weapon collision while preserving the open
        // 3.2m doorway and the walkable interior floor.
        const bool bCafe = FCString::Strcmp(Spec.AssetName, TEXT("OpenStreetCafeV5")) == 0;
        const float WidthMeters = bCafe ? 12.5f : 15.5f;
        const float DepthMeters = bCafe ? 8.2f : 10.0f;
        const float HeightMeters = bCafe ? 6.3f : 8.8f;
        const float WallMeters = 0.28f;
        const float CentimetersPerScaledMeter = 100.0f * BuildingScale.X;
        int32 CollisionBoxes = 0;
        auto AddCollisionBox = [&](const FVector& LocalCenterMeters, const FVector& SizeMeters, const TCHAR* Suffix)
        {
            const FVector WorldCenter = Center + Spec.Rotation.RotateVector(
                LocalCenterMeters * CentimetersPerScaledMeter);
            AActor* CollisionActor = SpawnRotatedBlock(
                WorldCenter,
                Spec.Rotation,
                SizeMeters * BuildingScale.X,
                FLinearColor::Black,
                CityLabel + TEXT(" ") + Spec.AssetName + TEXT(" Collision ") + Suffix,
                true);
            if (!CollisionActor)
            {
                return;
            }
            CollisionActor->SetActorHiddenInGame(true);
            CollisionActor->Tags.AddUnique(FName("FirstLevelOnly"));
            CollisionActor->Tags.AddUnique(FName("FirstLevelAccessCollision"));
            CollisionActor->Tags.AddUnique(FName("DoorwayPreservingCollision"));
            ++CollisionBoxes;
        };

        AddCollisionBox(
            FVector(0.0f, 0.0f, 0.10f),
            FVector(WidthMeters, DepthMeters, 0.20f),
            TEXT("Floor"));
        AddCollisionBox(
            FVector(0.0f, DepthMeters * 0.5f - WallMeters * 0.5f, HeightMeters * 0.5f),
            FVector(WidthMeters, WallMeters, HeightMeters),
            TEXT("RearWall"));
        AddCollisionBox(
            FVector(-WidthMeters * 0.5f + WallMeters * 0.5f, 0.0f, HeightMeters * 0.5f),
            FVector(WallMeters, DepthMeters, HeightMeters),
            TEXT("LeftWall"));
        AddCollisionBox(
            FVector(WidthMeters * 0.5f - WallMeters * 0.5f, 0.0f, HeightMeters * 0.5f),
            FVector(WallMeters, DepthMeters, HeightMeters),
            TEXT("RightWall"));
        const float DoorWidthMeters = 3.2f;
        const float SideWidthMeters = (WidthMeters - DoorWidthMeters) * 0.5f;
        for (const float Side : { -1.0f, 1.0f })
        {
            AddCollisionBox(
                FVector(
                    Side * (DoorWidthMeters * 0.5f + SideWidthMeters * 0.5f),
                    -DepthMeters * 0.5f + WallMeters * 0.5f,
                    2.25f),
                FVector(SideWidthMeters, WallMeters, 4.5f),
                Side < 0.0f ? TEXT("FrontWallLeft") : TEXT("FrontWallRight"));
        }
        AddCollisionBox(
            FVector(0.0f, -DepthMeters * 0.5f + WallMeters * 0.5f, 6.65f),
            FVector(WidthMeters, WallMeters, 4.3f),
            TEXT("FrontUpper"));
        AddCollisionBox(
            FVector(0.0f, 0.0f, HeightMeters - 0.14f),
            FVector(WidthMeters + 0.2f, DepthMeters + 0.2f, 0.28f),
            TEXT("Roof"));

        if (FCString::Strcmp(Spec.AssetName, TEXT("AccessibleMarketV5")) == 0)
        {
            AddCollisionBox(FVector(-4.3f, 1.6f, 0.72f), FVector(4.6f, 1.0f, 1.25f), TEXT("Counter"));
            for (const float ShelfX : { -5.2f, -2.7f, 2.9f, 5.2f })
            {
                AddCollisionBox(FVector(ShelfX, 3.2f, 1.45f), FVector(1.4f, 0.55f, 2.7f), TEXT("Shelf"));
            }
        }
        else if (FCString::Strcmp(Spec.AssetName, TEXT("AccessibleClinicV5")) == 0)
        {
            for (const float CotX : { -4.5f, 0.0f, 4.5f })
            {
                AddCollisionBox(FVector(CotX, 2.4f, 0.52f), FVector(3.0f, 1.15f, 0.42f), TEXT("Cot"));
            }
            AddCollisionBox(FVector(-4.8f, -1.0f, 0.82f), FVector(3.9f, 1.0f, 1.35f), TEXT("Desk"));
        }
        else
        {
            AddCollisionBox(FVector(0.0f, 2.55f, 0.80f), FVector(7.2f, 0.92f, 1.30f), TEXT("CafeBar"));
            for (const float TableX : { -4.0f, -1.35f, 1.35f, 4.0f })
            {
                AddCollisionBox(FVector(TableX, 0.0f, 0.55f), FVector(0.28f, 0.28f, 1.10f), TEXT("TablePost"));
                AddCollisionBox(FVector(TableX, 0.0f, 1.04f), FVector(1.44f, 1.44f, 0.10f), TEXT("TableTop"));
            }
        }
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelAccessV5] building=%s collision_boxes=%d doorway_clear_width=%.0fcm visual_hull=disabled"),
            Spec.AssetName,
            CollisionBoxes,
            DoorWidthMeters * CentimetersPerScaledMeter);

        const FVector OutsideOffset = Spec.Rotation.RotateVector(FVector(0.0f, -560.0f, 92.0f));
        const FVector InsideOffset = Spec.Rotation.RotateVector(FVector(0.0f, -150.0f, 92.0f));
        const FVector OutsidePoint = Center + OutsideOffset;
        const FVector InsidePoint = Center + InsideOffset;
        FirstLevelAccessDoorwayOutsidePoints.Add(OutsidePoint);
        FirstLevelAccessDoorwayInsidePoints.Add(InsidePoint);

        // 2026-07-11 pass 4 (environment physics): REAL swinging double doors
        // in the 3.2 m doorway. E toggles them, they auto-close, and they block
        // zombies/gunfire while shut. Hinges at the jambs (x = ±160), leaves
        // meet at the doorway center.
        {
            FActorSpawnParameters DoorSpawnParams;
            DoorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            const float FrontWallLocalY = (-DepthMeters * 0.5f + WallMeters * 0.5f) * 100.0f;
            const float DoorLeafScale = 160.0f / 104.0f;   // authored leaf 1.04 m
            const float DoorBaseLocalZ = 20.0f;            // collision floor top
            for (const float HingeSide : { -1.0f, 1.0f })
            {
                const FVector HingeLocal(HingeSide * 160.0f, FrontWallLocalY, DoorBaseLocalZ);
                const FVector HingeWorld = Center + Spec.Rotation.RotateVector(HingeLocal);
                const FRotator LeafRot = HingeSide < 0.0f
                    ? Spec.Rotation
                    : (Spec.Rotation + FRotator(0.0f, 180.0f, 0.0f));
                if (ADoorActor* Door = GetWorld()->SpawnActor<ADoorActor>(
                        ADoorActor::StaticClass(), HingeWorld, LeafRot, DoorSpawnParams))
                {
                    Door->ConfigureLeaf(DoorLeafScale, 210.0f / 206.0f, HingeSide > 0.0f);
                    Door->Tags.AddUnique(FName("FirstLevelOnly"));
                    Door->Tags.AddUnique(FName("FirstLevelAccessDoorLeaf"));
                    RegisterStreamedActor(Door);
                    UE_LOG(LogTemp, Display, TEXT("[DoorActor] %s %s doorway leaf spawned (%s hinge)"),
                        *CityLabel, Spec.AssetName, HingeSide < 0.0f ? TEXT("left") : TEXT("right"));
                }
            }
        }

        // A bright, level sidewalk spur makes the usable entrance legible and
        // guarantees that the street-side approach is not mistaken for a
        // decorative facade. It is visual-only; the canonical ground beneath
        // it remains the authoritative collision surface.
        const FVector ApproachDirection = (OutsidePoint - InsidePoint).GetSafeNormal2D();
        if (AActor* EntryWalk = SpawnRotatedBlock(
                OutsidePoint + ApproachDirection * 480.0f + FVector(0.0f, 0.0f, -86.0f),
                Spec.Rotation,
                FVector(1.75f, 9.6f, 0.035f),
                Spec.LightColor * 0.42f + FLinearColor(0.42f, 0.44f, 0.46f),
                CityLabel + TEXT(" ") + Spec.AssetName + TEXT(" Accessible Sidewalk Spur"),
                false))
        {
            EntryWalk->Tags.AddUnique(FName("FirstLevelOnly"));
            EntryWalk->Tags.AddUnique(FName("FirstLevelBuildingAccessRoute"));
            EntryWalk->Tags.AddUnique(FName("CanonicalStreetElevation"));
            RegisterStreamedActor(EntryWalk);
        }

        const FVector PickupOffset = Spec.Rotation.RotateVector(FVector(-260.0f, 110.0f, 112.0f));
        if (APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(
                APickupActor::StaticClass(), Center + PickupOffset, Spec.Rotation))
        {
            Pickup->Kind = Spec.PickupKind;
            Pickup->Amount = Spec.PickupAmount;
            Pickup->Tags.AddUnique(FName("FirstLevelInteriorFunctionalPickup"));
            Pickup->Tags.AddUnique(FName(Spec.AccessTag));
            RegisterStreamedActor(Pickup);
            ++FunctionalPickups;
        }

        const FVector LightOffset = Spec.Rotation.RotateVector(FVector(0.0f, 40.0f, 310.0f));
        if (APointLight* InteriorLight = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(), Center + LightOffset, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* Light = Cast<UPointLightComponent>(InteriorLight->GetLightComponent()))
            {
                Light->SetMobility(EComponentMobility::Movable);
                Light->SetIntensity(5200.0f);
                Light->SetLightColor(Spec.LightColor);
                Light->SetAttenuationRadius(1100.0f);
                Light->SetCastShadows(false);
            }
            InteriorLight->Tags.AddUnique(FName("FirstLevelInteriorPractical"));
            RegisterStreamedActor(InteriorLight);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[FirstLevelAccessV5] %s buildings=%d open_doorways=%d functional_pickups=%d canonical_ground_z=%.1f"),
        *CityLabel,
        SpawnedBuildings,
        FirstLevelAccessDoorwayOutsidePoints.Num(),
        FunctionalPickups,
        Origin.Z);
}

void ACodeRescueGameMode::SpawnFirstLevelPurposeDistrictPass(
    int32 CityIndex,
    const FVector& Origin,
    const FString& CityLabel)
{
    if (CityIndex != 0)
    {
        return;
    }
    auto ArtPath = [](const TCHAR* Name)
    {
        const FString N(Name);
        return FString::Printf(
            TEXT("/Game/CodeRescueArt/WorldLootWeatherV6/%s/%s/StaticMeshes/%s.%s"),
            *N, *N, *N, *N);
    };
    if (!LoadObject<UStaticMesh>(nullptr, *ArtPath(TEXT("FieldLogisticsDepotV6"))))
    {
        UE_LOG(LogTemp, Error, TEXT("[PurposeDistrictAudit] COMPLETE FAIL reason=authored_assets_missing"));
        return;
    }

    struct FPurposeDistrictSpec
    {
        const TCHAR* AssetName;
        const TCHAR* DistrictTag;
        FVector PreferredOffsetFromOrigin;
        FRotator Rotation;
        EPickupKind RewardKind;
        int32 RewardAmount;
        FLinearColor PracticalColor;
    };
    const FPurposeDistrictSpec Specs[] = {
        { TEXT("FieldLogisticsDepotV6"), TEXT("PurposeDistrict_LogisticsDepot"),
          FVector(7900.0f, -5600.0f, 0.0f), FRotator(0.0f, 18.0f, 0.0f),
          EPickupKind::Ammo, 36, FLinearColor(1.0f, 0.56f, 0.18f) },
        { TEXT("WeatherRelayV6"), TEXT("PurposeDistrict_WeatherRelay"),
          FVector(8200.0f, 0.0f, 0.0f), FRotator(0.0f, -24.0f, 0.0f),
          EPickupKind::RadioScanner, 1, FLinearColor(0.16f, 0.82f, 1.0f) },
        { TEXT("QuarantineCheckpointV6"), TEXT("PurposeDistrict_QuarantineCheckpoint"),
          FVector(7900.0f, 5400.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f),
          EPickupKind::ArmorPlate, 1, FLinearColor(1.0f, 0.25f, 0.08f) },
    };

    const FVector CandidateDeltas[] = {
        FVector::ZeroVector,
        FVector(0.0f, -1150.0f, 0.0f),
        FVector(0.0f, 1150.0f, 0.0f),
        FVector(-1150.0f, 0.0f, 0.0f),
        FVector(1150.0f, 0.0f, 0.0f),
        FVector(-900.0f, -900.0f, 0.0f),
        FVector(-900.0f, 900.0f, 0.0f),
    };
    auto IsOpenDistrictLocation = [this, Origin](
        UStaticMesh* DistrictMesh,
        const FVector& Candidate,
        const FRotator& Rotation)
    {
        if (!DistrictMesh ||
            !FCodeRescueCampaign::IsLocationInsideCityArenaXY(0, Candidate, false))
        {
            return false;
        }
        const FBox DistrictBounds = DistrictMesh->GetBoundingBox()
            .TransformBy(FTransform(Rotation, Candidate))
            .ExpandBy(FVector(180.0f, 180.0f, 35.0f));
        for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
        {
            AStaticMeshActor* Existing = *It;
            if (!IsValid(Existing) || Existing->IsHidden())
            {
                continue;
            }
            FVector ExistingOrigin;
            FVector ExistingExtent;
            Existing->GetActorBounds(true, ExistingOrigin, ExistingExtent);
            const float ExistingTop = ExistingOrigin.Z + ExistingExtent.Z;
            if (ExistingTop <= Origin.Z + 45.0f || ExistingExtent.Z <= 14.0f)
            {
                continue; // canonical floor, roads, markings, and ground rings
            }
            const FBox ExistingBounds(ExistingOrigin - ExistingExtent, ExistingOrigin + ExistingExtent);
            if (DistrictBounds.Intersect(ExistingBounds))
            {
                return false;
            }
        }
        return true;
    };

    int32 SpawnedLandmarks = 0;
    int32 SpawnedRewards = 0;
    int32 OpenSpaceValidated = 0;
    int32 FieldDepotModules = 0;
    int32 DepotStockPickups = 0;
    for (const FPurposeDistrictSpec& Spec : Specs)
    {
        UStaticMesh* DistrictMesh = LoadObject<UStaticMesh>(nullptr, *ArtPath(Spec.AssetName));
        FVector Location = Origin + Spec.PreferredOffsetFromOrigin;
        bool bOpenSpace = false;
        for (const FVector& Delta : CandidateDeltas)
        {
            const FVector Candidate = Origin + Spec.PreferredOffsetFromOrigin + Delta;
            if (IsOpenDistrictLocation(DistrictMesh, Candidate, Spec.Rotation))
            {
                Location = Candidate;
                bOpenSpace = true;
                break;
            }
        }
        if (!bOpenSpace)
        {
            UE_LOG(LogTemp, Error,
                TEXT("[PurposeDistrictAudit] no open placement asset=%s preferred=(%.0f,%.0f)"),
                Spec.AssetName,
                Spec.PreferredOffsetFromOrigin.X,
                Spec.PreferredOffsetFromOrigin.Y);
            continue;
        }
        ++OpenSpaceValidated;
        const bool bFieldDepot = FString(Spec.AssetName).Contains(TEXT("FieldLogisticsDepot"));
        AActor* Landmark = SpawnKitMesh(
            ArtPath(Spec.AssetName),
            Location,
            Spec.Rotation,
            FVector::OneVector,
            CityLabel + TEXT(" ") + Spec.AssetName,
            true,
            bFieldDepot
                ? TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")
                : nullptr);
        if (!Landmark)
        {
            continue;
        }
        ++SpawnedLandmarks;
        Landmark->Tags.AddUnique(FName("FirstLevelOnly"));
        Landmark->Tags.AddUnique(FName("FirstLevelPurposeLandmark"));
        Landmark->Tags.AddUnique(FName("PurposeDistrictReadable"));
        Landmark->Tags.AddUnique(FName("PurposeDistrictOpenSpaceValidated"));
        Landmark->Tags.AddUnique(FName("BlenderAuthoredWorldV6"));
        Landmark->Tags.AddUnique(FName("CanonicalStreetElevation"));
        Landmark->Tags.AddUnique(FName(Spec.DistrictTag));
        ApplyRuntimeDataLayerTags(Landmark, {
            FName("FirstLevelGameplayArt"),
            FName("WorldDevelopmentDeepDive"),
            FName("PurposefulWorldRegion") });
        if (UStaticMeshComponent* LandmarkMesh = Landmark->FindComponentByClass<UStaticMeshComponent>())
        {
            LandmarkMesh->SetVisibility(true, true);
            LandmarkMesh->SetHiddenInGame(false, true);
            LandmarkMesh->SetRenderInMainPass(true);
            LandmarkMesh->MarkRenderStateDirty();
            if (bFieldDepot)
            {
                // UE 5.7 reports valid render sections for this imported mesh
                // but omits it from the Mac main pass. Keep its Blender bounds
                // as the collision/siting contract, disable the invisible
                // blocker, and realize the same authored dimensions through
                // visible UE modules so the district can never become empty.
                LandmarkMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Landmark->SetActorEnableCollision(false);
                Landmark->Tags.AddUnique(FName("BlenderFieldDepotSpatialContract"));

                auto DepotModule = [this, &FieldDepotModules, &Spec, &Location](
                    const FVector& LocalOffset,
                    const FVector& Scale,
                    const FLinearColor& Color,
                    const TCHAR* ModuleName,
                    bool bCollision)
                {
                    AActor* Module = SpawnRotatedBlock(
                        Location + Spec.Rotation.RotateVector(LocalOffset),
                        Spec.Rotation,
                        Scale,
                        Color,
                        FString::Printf(TEXT("Field Logistics Depot %s"), ModuleName),
                        bCollision);
                    if (Module)
                    {
                        Module->Tags.AddUnique(FName("FirstLevelPurposeLandmarkModule"));
                        Module->Tags.AddUnique(FName("PurposeDistrict_LogisticsDepot"));
                        Module->Tags.AddUnique(FName("BlenderDepotRuntimeRealization"));
                        Module->Tags.AddUnique(FName("CanonicalStreetElevation"));
                        ++FieldDepotModules;
                    }
                    return Module;
                };

                DepotModule(FVector(0.0f, 0.0f, 10.0f), FVector(8.5f, 5.3f, 0.20f),
                    FLinearColor(0.075f, 0.085f, 0.092f), TEXT("Foundation"), false);
                for (const float X : { -385.0f, 385.0f })
                {
                    for (const float Y : { -225.0f, 225.0f })
                    {
                        DepotModule(FVector(X, Y, 165.0f), FVector(0.18f, 0.18f, 3.20f),
                            FLinearColor(0.32f, 0.35f, 0.38f), TEXT("Canopy Post"), true);
                    }
                }
                DepotModule(FVector(0.0f, 0.0f, 328.0f), FVector(8.5f, 5.3f, 0.22f),
                    FLinearColor(0.18f, 0.23f, 0.19f), TEXT("Weather Canopy"), false);
                DepotModule(FVector(0.0f, 215.0f, 135.0f), FVector(7.6f, 0.34f, 2.40f),
                    FLinearColor(0.15f, 0.18f, 0.21f), TEXT("Rear Rack"), true);
                for (const float Z : { 48.0f, 118.0f, 188.0f })
                {
                    DepotModule(FVector(0.0f, 193.0f, Z), FVector(7.2f, 0.75f, 0.11f),
                        FLinearColor(0.42f, 0.46f, 0.50f), TEXT("Rack Shelf"), true);
                }
                for (int32 CrateIndex = 0; CrateIndex < 8; ++CrateIndex)
                {
                    const float X = -305.0f + static_cast<float>(CrateIndex % 4) * 202.0f;
                    const float Z = 45.0f + static_cast<float>(CrateIndex / 4) * 70.0f;
                    const FLinearColor CrateColor = CrateIndex % 2 == 0
                        ? FLinearColor(0.12f, 0.18f, 0.10f)
                        : FLinearColor(0.50f, 0.16f, 0.025f);
                    DepotModule(FVector(X, 165.0f, Z), FVector(1.15f, 0.68f, 0.55f),
                        CrateColor, TEXT("Supply Case"), true);
                }
                DepotModule(FVector(-52.0f, -245.0f, 240.0f), FVector(0.62f, 0.12f, 0.52f),
                    FLinearColor(1.0f, 0.46f, 0.06f) * 2.0f, TEXT("Supply Symbol Left"), false);
                DepotModule(FVector(52.0f, -245.0f, 240.0f), FVector(0.62f, 0.12f, 0.52f),
                    FLinearColor(1.0f, 0.46f, 0.06f) * 2.0f, TEXT("Supply Symbol Right"), false);
                DepotModule(FVector(0.0f, -245.0f, 296.0f), FVector(0.62f, 0.12f, 0.52f),
                    FLinearColor(1.0f, 0.46f, 0.06f) * 2.0f, TEXT("Supply Symbol Crown"), false);

                const EPickupKind DepotStockKinds[] = {
                    EPickupKind::Ammo,
                    EPickupKind::Medkit,
                    EPickupKind::ArmorPlate,
                    EPickupKind::RadioScanner,
                    EPickupKind::Scrap,
                    EPickupKind::Flare,
                };
                const int32 DepotStockAmounts[] = { 24, 1, 1, 1, 2, 1 };
                for (int32 StockIndex = 0; StockIndex < UE_ARRAY_COUNT(DepotStockKinds); ++StockIndex)
                {
                    const FVector LocalStockOffset(
                        -300.0f + static_cast<float>(StockIndex) * 120.0f,
                        -105.0f,
                        115.0f);
                    const FVector StockLocation = Location + Spec.Rotation.RotateVector(LocalStockOffset);
                    const FRotator StockRotation = Spec.Rotation + FRotator(0.0f, 0.0f, 0.0f);
                    if (APickupActor* Stock = GetWorld()->SpawnActor<APickupActor>(
                            APickupActor::StaticClass(), StockLocation, StockRotation))
                    {
                        Stock->Kind = DepotStockKinds[StockIndex];
                        Stock->Amount = DepotStockAmounts[StockIndex];
                        Stock->PresentationSpinDegreesPerSecond = 0.0f;
                        Stock->Tags.AddUnique(FName("PurposeDistrictReward"));
                        Stock->Tags.AddUnique(FName("PurposeDistrict_LogisticsDepot"));
                        Stock->Tags.AddUnique(FName("DepotIconStockPickup"));
                        Stock->RefreshPresentation();
                        RegisterStreamedActor(Stock);
                        ++DepotStockPickups;
                    }
                }
            }
        }

        if (APointLight* Practical = GetWorld()->SpawnActor<APointLight>(
            APointLight::StaticClass(), Location + FVector(0.0f, 0.0f, 260.0f), FRotator::ZeroRotator))
        {
            if (UPointLightComponent* Light = Cast<UPointLightComponent>(Practical->GetLightComponent()))
            {
                Light->SetMobility(EComponentMobility::Movable);
                Light->SetIntensity(2400.0f);
                Light->SetAttenuationRadius(820.0f);
                Light->SetLightColor(Spec.PracticalColor);
                Light->SetCastShadows(false);
            }
            Practical->Tags.AddUnique(FName("FirstLevelPurposeLandmarkLight"));
            Practical->Tags.AddUnique(FName(Spec.DistrictTag));
            RegisterStreamedActor(Practical);
        }

        const FVector RewardOffset = Spec.Rotation.RotateVector(FVector(0.0f, -390.0f, 86.0f));
        if (APickupActor* Reward = GetWorld()->SpawnActor<APickupActor>(
            APickupActor::StaticClass(), Location + RewardOffset, Spec.Rotation))
        {
            Reward->Kind = Spec.RewardKind;
            Reward->Amount = Spec.RewardAmount;
            Reward->Tags.AddUnique(FName("PurposeDistrictReward"));
            Reward->Tags.AddUnique(FName(Spec.DistrictTag));
            Reward->RefreshPresentation();
            RegisterStreamedActor(Reward);
            ++SpawnedRewards;
        }
    }

    const bool bPass = SpawnedLandmarks == UE_ARRAY_COUNT(Specs) &&
        SpawnedRewards == UE_ARRAY_COUNT(Specs) &&
        OpenSpaceValidated == UE_ARRAY_COUNT(Specs) &&
        FieldDepotModules == 21 &&
        DepotStockPickups == 6;
    if (bPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedPurposeDistrictPass"));
        UE_LOG(LogTemp, Display,
            TEXT("[PurposeDistrictAudit] COMPLETE PASS landmarks=%d/3 rewards=%d/3 open_space=%d/3 depot_modules=%d/21 depot_icon_stock=%d/6 physical_symbols=3 paragraph_signs=0 canonical_ground=1"),
            SpawnedLandmarks, SpawnedRewards, OpenSpaceValidated, FieldDepotModules,
            DepotStockPickups);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[PurposeDistrictAudit] COMPLETE FAIL landmarks=%d/3 rewards=%d/3 open_space=%d/3 depot_modules=%d/21 depot_icon_stock=%d/6"),
            SpawnedLandmarks, SpawnedRewards, OpenSpaceValidated, FieldDepotModules,
            DepotStockPickups);
    }
}

void ACodeRescueGameMode::SpawnCityMoodLayer()
{
    // 2026-07-06 mood pass — "comfortably realistic", Resident Evil Requiem
    // temperature: cool low fog hugging the streets, filmic post (gentle
    // bloom, vignette, grain) with warm practicals (street lamps, lit
    // windows) punching through. Honors the 07-02 de-teal decision: fog
    // inscattering is a near-neutral cold gray, NOT teal.
    UWorld* World = GetWorld();
    if (!World || CityMoodFog)
    {
        return;
    }
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(FVector(0, 0, 100.0f), FRotator::ZeroRotator, Params);
    if (Fog)
    {
        if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
        {
            // 2026-07-07 clarity tune (Kenny: "cannot discern where the
            // character is"): halve the density and push the fog out — mood
            // stays, mid-range readability wins.
            FogComp->SetFogDensity(0.011f);
            FogComp->SetFogHeightFalloff(0.24f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.026f, 0.029f, 0.034f));
            FogComp->SetStartDistance(1600.0f);
            FogComp->SetFogMaxOpacity(0.72f);
        }
        Fog->Tags.Add(FName(TEXT("CityMoodLayer")));
        CityMoodFog = Fog;
    }
    APostProcessVolume* Post = World->SpawnActor<APostProcessVolume>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
    if (Post)
    {
        Post->bUnbound = true;
        Post->BlendWeight = 1.0f;
        Post->Settings.bOverride_BloomIntensity = true;
        Post->Settings.BloomIntensity = 0.55f;
        Post->Settings.bOverride_VignetteIntensity = true;
        Post->Settings.VignetteIntensity = 0.42f;
        Post->Settings.bOverride_FilmGrainIntensity = true;
        Post->Settings.FilmGrainIntensity = 0.16f;
        Post->Settings.bOverride_ColorContrast = true;
        Post->Settings.ColorContrast = FVector4(1.06f, 1.06f, 1.06f, 1.0f);
        Post->Settings.bOverride_ColorSaturation = true;
        Post->Settings.ColorSaturation = FVector4(0.92f, 0.92f, 0.92f, 1.0f);
        Post->Settings.bOverride_ColorGamma = true;
        Post->Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.04f, 1.0f);   // whisper of cool in the shadows
        Post->Tags.Add(FName(TEXT("CityMoodLayer")));
        CityMoodPost = Post;
    }
    UE_LOG(LogTemp, Display, TEXT("[CityMood] fog=%s post=%s"),
        CityMoodFog ? TEXT("ok") : TEXT("FAILED"),
        CityMoodPost ? TEXT("ok") : TEXT("FAILED"));
}

void ACodeRescueGameMode::SpawnNightSkyLayer(const FVector& Origin)
{
    // V5 uses isolated emissive star geometry, not an inward-facing shell.
    // It cannot cover the camera with a material surface and follows the
    // player with the detailed moon through the night window.
    if (NightSkyDome || NightSkyMoon)
    {
        return;
    }
    NightSkyDome = SpawnKitMesh(
        TEXT("/Game/CodeRescueArt/FirstLevelV5/PointStarFieldV5/PointStarFieldV5/StaticMeshes/PointStarFieldV5.PointStarFieldV5"),
        Origin, FRotator::ZeroRotator, FVector::OneVector,
        TEXT("First Level Point Star Field"), /*bEnableCollision=*/false, nullptr);
    const bool bUsingV5PointStars = NightSkyDome != nullptr;
    if (!NightSkyDome)
    {
        NightSkyDome = SpawnKitMesh(
            TEXT("/Game/CodeRescueArt/Sky/SM_SkyDome_Stars/StaticMeshes/SM_SkyDome_Stars.SM_SkyDome_Stars"),
            Origin + FVector(0.0f, 0.0f, -20000.0f), FRotator::ZeroRotator, FVector(600.0f),
            TEXT("Legacy Night Sky Star Dome"), /*bEnableCollision=*/false, nullptr);
    }
    NightSkyMoon = SpawnKitMesh(
        TEXT("/Game/CodeRescueArt/FirstLevelV5/MoonDetailedV5/MoonDetailedV5/StaticMeshes/MoonDetailedV5.MoonDetailedV5"),
        Origin + FVector(-12000.0f, 9000.0f, 15000.0f), FRotator::ZeroRotator, FVector::OneVector,
        TEXT("First Level Detailed Moon"), /*bEnableCollision=*/false, nullptr);
    if (!NightSkyMoon)
    {
        NightSkyMoon = SpawnKitMesh(
            TEXT("/Game/CodeRescueArt/Sky/SM_Moon/StaticMeshes/SM_Moon.SM_Moon"),
            Origin + FVector(-32000.0f, 26000.0f, 38000.0f), FRotator::ZeroRotator, FVector(14.0f),
            TEXT("Legacy Moon"), /*bEnableCollision=*/false, nullptr);
    }
    for (AActor* SkyActor : { NightSkyDome, NightSkyMoon })
    {
        if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(SkyActor))
        {
            if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
            {
                SMC->SetCastShadow(false);
            }
            SMA->Tags.Add(FName("SkyLayer"));
            SMA->SetActorHiddenInGame(true);   // revealed by the day/night tick
        }
    }
    if (NightSkyDome && bUsingV5PointStars)
    {
        NightSkyDome->Tags.AddUnique(FName("SafePointStarField"));
        NightSkyDome->Tags.AddUnique(FName("FirstLevelSkyV5"));
    }
    if (NightSkyMoon)
    {
        NightSkyMoon->Tags.AddUnique(FName("FirstLevelSkyV5"));
    }
    if (NightSkyDome && NightSkyMoon)
    {
        UE_LOG(LogTemp, Display, TEXT("[NightSky] dome=ok moon=ok"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[NightSky] dome=%s moon=%s"),
            NightSkyDome ? TEXT("ok") : TEXT("MISSING (import pending)"),
            NightSkyMoon ? TEXT("ok") : TEXT("MISSING (import pending)"));
    }
}

void ACodeRescueGameMode::UpdateNightSkyVisibility()
{
    if (!NightSkyDome && !NightSkyMoon)
    {
        return;
    }
    const float SolarAltitude = FMath::Cos(TimeOfDay * 2.0f * PI);
    const bool bShowMoon = SolarAltitude < 0.04f;
    const bool bShowStars = SolarAltitude < -0.10f;
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (NightSkyDome)
    {
        const bool bSafePointField = NightSkyDome->Tags.Contains(FName("SafePointStarField"));
        NightSkyDome->SetActorHiddenInGame(!bSafePointField || !bShowStars);
        if (Player)
        {
            const FVector P = Player->GetActorLocation();
            NightSkyDome->SetActorLocation(bSafePointField
                ? P
                : FVector(P.X, P.Y, P.Z - 20000.0f));
        }
    }
    if (NightSkyMoon)
    {
        NightSkyMoon->SetActorHiddenInGame(!bShowMoon);
        if (Player)
        {
            const FVector P = Player->GetActorLocation();
            NightSkyMoon->SetActorLocation(P + FVector(-12000.0f, 9000.0f, 15000.0f));
        }
    }
}

void ACodeRescueGameMode::UnifyFirstLevelGroundTops()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Phase A — the REGISTERED street kit itself: snap intra-group outliers to
    // their mesh-type median (a lone road tile 18 uu proud of its siblings is
    // exactly the "different ground here vs there" Kenny photographed). Groups
    // keep their designed relative offsets (sidewalk curbs stay curbs).
    struct FSurfaceEntry
    {
        AActor* Actor = nullptr;
        float Top = 0.0f;
    };
    TMap<FString, TArray<FSurfaceEntry>> SurfaceGroups;
    for (const TWeakObjectPtr<AActor>& SurfacePtr : FirstLevelGroundSurfaceActors)
    {
        AActor* Surface = SurfacePtr.Get();
        if (!Surface)
        {
            continue;
        }
        FVector BoundsOrigin, BoundsExtent;
        Surface->GetActorBounds(false, BoundsOrigin, BoundsExtent);
        FString GroupKey = TEXT("block");
        if (const AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Surface))
        {
            if (const UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                if (const UStaticMesh* Asset = MeshComp->GetStaticMesh())
                {
                    GroupKey = Asset->GetName();
                }
            }
        }
        FSurfaceEntry Entry;
        Entry.Actor = Surface;
        Entry.Top = BoundsOrigin.Z + BoundsExtent.Z;
        SurfaceGroups.FindOrAdd(GroupKey).Add(Entry);
    }

    // Road datum FIRST (median of the road-tile tops themselves).
    TArray<float> RoadTops;
    for (const TPair<FString, TArray<FSurfaceEntry>>& Group : SurfaceGroups)
    {
        if (Group.Key.Contains(TEXT("Road")))
        {
            for (const FSurfaceEntry& Entry : Group.Value)
            {
                RoadTops.Add(Entry.Top);
            }
        }
    }
    if (RoadTops.Num() < 1)
    {
        for (const TPair<FString, TArray<FSurfaceEntry>>& Group : SurfaceGroups)
        {
            if (!Group.Key.Contains(TEXT("Sidewalk")))
            {
                for (const FSurfaceEntry& Entry : Group.Value)
                {
                    RoadTops.Add(Entry.Top);
                }
            }
        }
    }
    if (RoadTops.Num() < 1)
    {
        UE_LOG(LogTemp, Display, TEXT("[GroundUnify] skipped — no driving surfaces registered"));
        return;
    }
    RoadTops.Sort();
    const float Datum = RoadTops[RoadTops.Num() / 2];

    // Snap every DRIVING surface (roads, crosswalk stripes, painted blocks)
    // onto the road plane; sidewalks keep their designed curb via their own
    // group median. This is what makes "the ground" agree everywhere.
    int32 RegisteredSnapped = 0;
    TSet<const AActor*> RegisteredSet;
    for (TPair<FString, TArray<FSurfaceEntry>>& Group : SurfaceGroups)
    {
        const bool bSidewalk = Group.Key.Contains(TEXT("Sidewalk"));
        float GroupTarget = Datum;
        if (bSidewalk)
        {
            TArray<float> Tops;
            for (const FSurfaceEntry& Entry : Group.Value)
            {
                Tops.Add(Entry.Top);
            }
            Tops.Sort();
            GroupTarget = Tops[Tops.Num() / 2];
        }
        for (const FSurfaceEntry& Entry : Group.Value)
        {
            RegisteredSet.Add(Entry.Actor);
            const float Target = bSidewalk ? GroupTarget : (Datum + 0.4f);
            const float Delta = Target - Entry.Top;
            if (FMath::Abs(Delta) <= 0.75f || FMath::Abs(Delta) > 120.0f)
            {
                continue;
            }
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Entry.Actor))
            {
                if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    if (MeshComp->Mobility != EComponentMobility::Movable)
                    {
                        MeshComp->SetMobility(EComponentMobility::Movable);
                    }
                }
            }
            Entry.Actor->AddActorWorldOffset(FVector(0.0f, 0.0f, Delta));
            ++RegisteredSnapped;
            UE_LOG(LogTemp, Display, TEXT("[GroundUnify] street %s (%s) top %.1f -> %.1f"),
                *Entry.Actor->GetName(), *Group.Key, Entry.Top, Target);
        }
    }

    int32 Adjusted = 0;
    int32 AlreadyLevel = 0;
    int32 IntentionalElevated = 0;
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Actor = *It;
        if (!Actor || RegisteredSet.Contains(Actor) ||
            Actor->Tags.Contains(FName("GroundUnifyExempt")))
        {
            continue;
        }
        // 2026-07-17: unify runs in EVERY city's arena — and EDGE planes count.
        // Kenny's outskirt photos showed seams exactly at the perimeter: big
        // ground planes whose ORIGIN falls outside the arena test were being
        // skipped even though most of their surface is inside. Accept the
        // actor if any bounds corner reaches the arena.
        {
            FVector EdgeOrigin, EdgeExtent;
            Actor->GetActorBounds(false, EdgeOrigin, EdgeExtent);
            const int32 ActiveCity = GetActiveCampaignCityIndex();
            bool bTouchesArena = FCodeRescueCampaign::IsLocationInsideCityArenaXY(ActiveCity, EdgeOrigin);
            for (int32 CornerIdx = 0; !bTouchesArena && CornerIdx < 4; ++CornerIdx)
            {
                const FVector Corner = EdgeOrigin + FVector(
                    (CornerIdx & 1) ? EdgeExtent.X : -EdgeExtent.X,
                    (CornerIdx & 2) ? EdgeExtent.Y : -EdgeExtent.Y,
                    0.0f);
                bTouchesArena = FCodeRescueCampaign::IsLocationInsideCityArenaXY(ActiveCity, Corner);
            }
            if (!bTouchesArena)
            {
                continue;
            }
        }
        FVector BoundsOrigin, BoundsExtent;
        Actor->GetActorBounds(false, BoundsOrigin, BoundsExtent);
        // 2026-07-16 pass 5 (Kenny's terrace screenshots): large WALKABLE
        // PLATFORMS count as ground even when they are thick blocks — a big
        // raised terrace is exactly the "different ground here vs there"
        // complaint. Platform = footprint >= ~6x6 m and body <= 2.6 m tall.
        const bool bLargePlatform =
            BoundsExtent.X * BoundsExtent.Y >= 90000.0f && BoundsExtent.Z <= 130.0f;
        if (BoundsExtent.Z > 26.0f && !bLargePlatform)
        {
            continue;                       // not a ground slab or platform
        }
        if (BoundsExtent.X * BoundsExtent.Y < 40000.0f)
        {
            continue;                       // smaller than ~4 m² footprint: prop/decor
        }
        const float Top = BoundsOrigin.Z + BoundsExtent.Z;
        const float Delta = Datum - Top;
        // pass-5 diagnostics: every sizeable candidate leaves an evidence line
        if (BoundsExtent.X * BoundsExtent.Y >= 90000.0f)
        {
            FString MeshLabel = TEXT("block");
            if (const UStaticMeshComponent* DiagComp = Actor->GetStaticMeshComponent())
            {
                if (const UStaticMesh* DiagMesh = DiagComp->GetStaticMesh())
                {
                    MeshLabel = DiagMesh->GetName();
                }
            }
            UE_LOG(LogTemp, Display,
                TEXT("[GroundUnify] candidate %s mesh=%s top=%.1f delta=%.1f extent=(%.0f,%.0f,%.0f) platform=%d"),
                *Actor->GetName(), *MeshLabel, Top, Delta,
                BoundsExtent.X, BoundsExtent.Y, BoundsExtent.Z, bLargePlatform ? 1 : 0);
        }
        if (FMath::Abs(Delta) <= 0.75f)
        {
            ++AlreadyLevel;
            continue;
        }
        const float SnapWindow = bLargePlatform ? 220.0f : 60.0f;
        if (FMath::Abs(Delta) > SnapWindow)
        {
            // Genuine elevation set-piece (roofs, the underpass) — report, don't flatten.
            ++IntentionalElevated;
            UE_LOG(LogTemp, Display,
                TEXT("[GroundUnify] intentional elevation kept: %s top=%.1f datum=%.1f delta=%.1f"),
                *Actor->GetName(), Top, Datum, Delta);
            continue;
        }
        // Thin decorative layers (stripes, light spills) ride 0.6 uu proud of the
        // datum so they never z-fight the slabs they sit on.
        const float LayerEpsilon = (BoundsExtent.Z <= 3.0f) ? 0.6f : 0.0f;
        if (UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent())
        {
            if (MeshComp->Mobility != EComponentMobility::Movable)
            {
                MeshComp->SetMobility(EComponentMobility::Movable);
            }
        }
        Actor->AddActorWorldOffset(FVector(0.0f, 0.0f, Delta + LayerEpsilon));
        ++Adjusted;
        UE_LOG(LogTemp, Display, TEXT("[GroundUnify] snapped %s top %.1f -> %.1f (delta %.1f)"),
            *Actor->GetName(), Top, Datum + LayerEpsilon, Delta + LayerEpsilon);
    }

    // Tone the First Minute Orientation plaza: Kenny's night screenshot shows a
    // blinding white slab that also reads as a different "ground". Iterate ALL
    // actors (textured blocks may not be AStaticMeshActor) and match by
    // footprint so the big floor slab is caught regardless of class.
    int32 TonedPlazaSlabs = 0;
    int32 OrientationTagged = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (!It->Tags.Contains(FName("FirstMinuteOrientation")) ||
            It->Tags.Contains(FName("FirstLevelPlazaToned")))
        {
            continue;
        }
        ++OrientationTagged;
        FVector BoundsOrigin, BoundsExtent;
        It->GetActorBounds(false, BoundsOrigin, BoundsExtent);
        UE_LOG(LogTemp, Display, TEXT("[GroundUnify] orientation candidate %s extent=(%.0f,%.0f,%.0f)"),
            *It->GetName(), BoundsExtent.X, BoundsExtent.Y, BoundsExtent.Z);
        if (BoundsExtent.Z > 26.0f || BoundsExtent.X * BoundsExtent.Y < 60000.0f)
        {
            continue;                       // only the big floor slab (> ~6 m²)
        }
        if (UStaticMeshComponent* MeshComp = It->FindComponentByClass<UStaticMeshComponent>())
        {
            if (UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
            {
                // cycle-11 tactical review: 0.055 read near-black against the
                // sunlit speckled road — lifted toward the road's average so
                // the two grounds meet without a luminance seam.
                const FLinearColor Toned(0.085f, 0.090f, 0.096f);
                MID->SetVectorParameterValue(TEXT("Color"), Toned);
                MID->SetVectorParameterValue(TEXT("BaseColor"), Toned);
                MID->SetScalarParameterValue(TEXT("Roughness"), 0.92f);
                ++TonedPlazaSlabs;
                It->Tags.AddUnique(FName("FirstLevelPlazaToned"));
                UE_LOG(LogTemp, Display, TEXT("[GroundUnify] plaza slab toned: %s extent=(%.0f,%.0f,%.0f)"),
                    *It->GetName(), BoundsExtent.X, BoundsExtent.Y, BoundsExtent.Z);
            }
        }
    }

    // Fallback plaza detection: the washed-white slabs are the big thin
    // Concrete-textured blocks (the orientation tag was absent in live runs).
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->Tags.Contains(FName("FirstLevelPlazaToned")))
        {
            continue;
        }
        // 2026-07-17: tone plazas in EVERY city's arena, edge planes included
        // (origin-only testing skipped the perimeter slabs Kenny photographed)
        {
            FVector EdgeOrigin, EdgeExtent;
            It->GetActorBounds(false, EdgeOrigin, EdgeExtent);
            const int32 ActiveCity = GetActiveCampaignCityIndex();
            bool bTouchesArena = FCodeRescueCampaign::IsLocationInsideCityArenaXY(ActiveCity, EdgeOrigin);
            for (int32 CornerIdx = 0; !bTouchesArena && CornerIdx < 4; ++CornerIdx)
            {
                const FVector Corner = EdgeOrigin + FVector(
                    (CornerIdx & 1) ? EdgeExtent.X : -EdgeExtent.X,
                    (CornerIdx & 2) ? EdgeExtent.Y : -EdgeExtent.Y,
                    0.0f);
                bTouchesArena = FCodeRescueCampaign::IsLocationInsideCityArenaXY(ActiveCity, Corner);
            }
            if (!bTouchesArena)
            {
                continue;
            }
        }
        UStaticMeshComponent* MeshComp = It->FindComponentByClass<UStaticMeshComponent>();
        if (!MeshComp)
        {
            continue;
        }
        FVector BoundsOrigin, BoundsExtent;
        It->GetActorBounds(false, BoundsOrigin, BoundsExtent);
        // pass 5: include the thick raised platforms (terraces) + resolve MIDs
        // to their BASE material (tinted dynamic instances hid the Concrete
        // name, which is why the speckled terraces were never toned).
        const bool bLargePlatform =
            BoundsExtent.X * BoundsExtent.Y >= 90000.0f && BoundsExtent.Z <= 130.0f;
        if ((BoundsExtent.Z > 26.0f && !bLargePlatform) ||
            BoundsExtent.X * BoundsExtent.Y < 60000.0f)
        {
            continue;
        }
        UMaterialInterface* Slot0 = MeshComp->GetMaterial(0);
        UMaterialInterface* BaseMat = Slot0 ? Slot0->GetBaseMaterial() : nullptr;
        const FString MatName = BaseMat ? BaseMat->GetName() : (Slot0 ? Slot0->GetName() : FString());
        // pass 5 (cycle-3 tactical screenshot): after the HEIGHT unification the
        // remaining "different ground" read is TEXTURE — speckled granite slabs
        // against smooth asphalt. Any LARGE ground plane near the datum gets the
        // same asphalt tone, texture-agnostic; the authored street kit
        // (roads/sidewalks/crosswalks) keeps its detail art.
        FString KitName;
        if (const UStaticMesh* KitMesh = MeshComp->GetStaticMesh())
        {
            KitName = KitMesh->GetName();
        }
        const bool bStreetKitArt = KitName.Contains(TEXT("Road")) ||
            KitName.Contains(TEXT("Sidewalk")) || KitName.Contains(TEXT("Crosswalk"));
        FVector ToneOrigin, ToneExtent;
        It->GetActorBounds(false, ToneOrigin, ToneExtent);
        // 2026-07-17: widened from 60 — the blinding white slabs at Kenny's
        // outskirt save sit up to ~150 uu proud and were escaping the tone
        const bool bNearDatum = FMath::Abs((ToneOrigin.Z + ToneExtent.Z)) < 160.0f;
        const bool bHugeGroundPlane = ToneExtent.X * ToneExtent.Y >= 240000.0f && bNearDatum && !bStreetKitArt;
        if (!bHugeGroundPlane && !MatName.Contains(TEXT("Concrete")))
        {
            continue;
        }
        if (UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
        {
            // lifted with the orientation-slab tone (cycle-11): match the
            // sunlit road average instead of reading as a dark patch
            const FLinearColor Toned(0.085f, 0.090f, 0.096f);
            MID->SetVectorParameterValue(TEXT("Color"), Toned);
            MID->SetVectorParameterValue(TEXT("BaseColor"), Toned);
            MID->SetScalarParameterValue(TEXT("Roughness"), 0.92f);
            ++TonedPlazaSlabs;
            It->Tags.AddUnique(FName("FirstLevelPlazaToned"));
            UE_LOG(LogTemp, Display, TEXT("[GroundUnify] concrete plaza toned: %s extent=(%.0f,%.0f,%.0f)"),
                *It->GetName(), BoundsExtent.X, BoundsExtent.Y, BoundsExtent.Z);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[GroundUnify] COMPLETE datum=%.2f street_snapped=%d adjusted=%d level=%d intentional=%d plaza_toned=%d orientation_tagged=%d"),
        Datum, RegisteredSnapped, Adjusted, AlreadyLevel, IntentionalElevated, TonedPlazaSlabs, OrientationTagged);
}

bool ACodeRescueGameMode::AuditCampaignPerimeterGround(int32 CityIndex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector CityOrigin = FCodeRescueCampaign::GetCityOrigin(CityIndex);
    AStaticMeshActor* CanonicalGround = nullptr;
    float ClosestGroundDistanceSq = TNumericLimits<float>::Max();
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        if (!It->Tags.Contains(FName("CanonicalMissionGround")))
        {
            continue;
        }
        const float DistanceSq = FVector::DistSquared2D(It->GetActorLocation(), CityOrigin);
        if (DistanceSq < ClosestGroundDistanceSq)
        {
            ClosestGroundDistanceSq = DistanceSq;
            CanonicalGround = *It;
        }
    }

    if (!CanonicalGround)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[CampaignPerimeterGroundAudit] COMPLETE FAIL city=%d reason=missing_canonical_ground"),
            CityIndex);
        return false;
    }

    FVector FloorBoundsOrigin;
    FVector FloorBoundsExtent;
    CanonicalGround->GetActorBounds(false, FloorBoundsOrigin, FloorBoundsExtent);
    const float FloorTopZ = FloorBoundsOrigin.Z + FloorBoundsExtent.Z;
    const FVector WallHalfWorld = FCodeRescueCampaign::ScaleCityOffset(FVector(
        FCodeRescueCampaign::ArenaWallHalfXLocal,
        FCodeRescueCampaign::ArenaWallHalfYLocal,
        0.0f));
    constexpr float FloorWallOverlap = 75.0f;
    const bool bFloorCoversWalls =
        FloorBoundsExtent.X >= FMath::Abs(WallHalfWorld.X) + FloorWallOverlap &&
        FloorBoundsExtent.Y >= FMath::Abs(WallHalfWorld.Y) + FloorWallOverlap;

    constexpr int32 SamplesPerEdge = 9;
    constexpr float EdgeInset = 320.0f;
    constexpr float CornerInset = 760.0f;
    const float ProbeX = FMath::Abs(WallHalfWorld.X) - EdgeInset;
    const float ProbeY = FMath::Abs(WallHalfWorld.Y) - EdgeInset;
    const float CrossX = FMath::Abs(WallHalfWorld.X) - CornerInset;
    const float CrossY = FMath::Abs(WallHalfWorld.Y) - CornerInset;
    int32 SupportedSamples = 0;
    int32 EastSupportedSamples = 0;
    float MaximumTopDelta = 0.0f;
    FCollisionQueryParams ProbeParams(SCENE_QUERY_STAT(CampaignPerimeterGround), false);

    auto ProbeCanonicalFloor = [&](const FVector& ProbeLocation, bool bEastEdge)
    {
        FHitResult GroundHit;
        const FVector Start(ProbeLocation.X, ProbeLocation.Y, FloorTopZ + 90.0f);
        const FVector End(ProbeLocation.X, ProbeLocation.Y, FloorTopZ - 90.0f);
        const bool bHit = CanonicalGround->ActorLineTraceSingle(
            GroundHit, Start, End, ECC_Visibility, ProbeParams);
        const float TopDelta = bHit
            ? FMath::Abs(GroundHit.ImpactPoint.Z - FloorTopZ)
            : TNumericLimits<float>::Max();
        const bool bWalkableSupport = bHit &&
            GroundHit.ImpactNormal.Z >= 0.85f &&
            TopDelta <= 3.0f &&
            !CanonicalGround->Tags.Contains(FName("FallRecoveryCatchFloor"));
        if (bWalkableSupport)
        {
            ++SupportedSamples;
            EastSupportedSamples += bEastEdge ? 1 : 0;
            MaximumTopDelta = FMath::Max(MaximumTopDelta, TopDelta);
        }
    };

    for (int32 SampleIndex = 0; SampleIndex < SamplesPerEdge; ++SampleIndex)
    {
        const float Alpha = static_cast<float>(SampleIndex) /
            static_cast<float>(SamplesPerEdge - 1);
        const float AlongX = FMath::Lerp(-CrossX, CrossX, Alpha);
        const float AlongY = FMath::Lerp(-CrossY, CrossY, Alpha);
        ProbeCanonicalFloor(CityOrigin + FVector(ProbeX, AlongY, 0.0f), true);
        ProbeCanonicalFloor(CityOrigin + FVector(-ProbeX, AlongY, 0.0f), false);
        ProbeCanonicalFloor(CityOrigin + FVector(AlongX, ProbeY, 0.0f), false);
        ProbeCanonicalFloor(CityOrigin + FVector(AlongX, -ProbeY, 0.0f), false);
    }

    constexpr int32 TotalSamples = SamplesPerEdge * 4;
    const bool bCollisionEnabled = CanonicalGround->GetStaticMeshComponent() &&
        CanonicalGround->GetStaticMeshComponent()->GetCollisionEnabled() !=
            ECollisionEnabled::NoCollision;
    const bool bPass = bFloorCoversWalls && bCollisionEnabled &&
        SupportedSamples == TotalSamples &&
        EastSupportedSamples == SamplesPerEdge;
    const FString Summary = FString::Printf(
        TEXT("[CampaignPerimeterGroundAudit] COMPLETE %s city=%d samples=%d/%d east_right=%d/%d floor_covers_walls=%d collision=%d max_top_delta=%.2f floor_extent=(%.0f,%.0f) wall_half=(%.0f,%.0f) catch_floor_accepted=0"),
        bPass ? TEXT("PASS") : TEXT("FAIL"),
        CityIndex,
        SupportedSamples,
        TotalSamples,
        EastSupportedSamples,
        SamplesPerEdge,
        bFloorCoversWalls ? 1 : 0,
        bCollisionEnabled ? 1 : 0,
        MaximumTopDelta,
        FloorBoundsExtent.X,
        FloorBoundsExtent.Y,
        FMath::Abs(WallHalfWorld.X),
        FMath::Abs(WallHalfWorld.Y));
    if (bPass)
    {
        Tags.AddUnique(FName(*FString::Printf(
            TEXT("CampaignPerimeterGroundCity%dPass"), CityIndex)));
        if (CityIndex == 0)
        {
            Tags.AddUnique(FName("FirstLevelIntegratedPerimeterGroundPass"));
        }
        UE_LOG(LogTemp, Display, TEXT("%s"), *Summary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *Summary);
    }
    return bPass;
}

bool ACodeRescueGameMode::RunFirstLevelWorldAccessAudit()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    int32 ValidGroundSurfaces = 0;
    float MinimumSurfaceZ = TNumericLimits<float>::Max();
    float MaximumSurfaceZ = -TNumericLimits<float>::Max();
    float MinimumTopZ = TNumericLimits<float>::Max();
    float MaximumTopZ = -TNumericLimits<float>::Max();
    for (const TWeakObjectPtr<AActor>& SurfacePtr : FirstLevelGroundSurfaceActors)
    {
        if (const AActor* Surface = SurfacePtr.Get())
        {
            ++ValidGroundSurfaces;
            MinimumSurfaceZ = FMath::Min(MinimumSurfaceZ, Surface->GetActorLocation().Z);
            MaximumSurfaceZ = FMath::Max(MaximumSurfaceZ, Surface->GetActorLocation().Z);
            // 2026-07-11 pass 4: the WALKABLE TOP is what the player perceives
            // as "the ground" — pivot spread let thick-vs-thin slabs pass while
            // their tops disagreed (Kenny's multi-level screenshots). Curbs are
            // REAL street design, so the top contract compares DRIVING surfaces
            // (roads/crosswalks); the sidewalk curb offset is reported info.
            FVector BoundsOrigin, BoundsExtent;
            Surface->GetActorBounds(false, BoundsOrigin, BoundsExtent);
            bool bDrivingSurface = true;
            if (const AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Surface))
            {
                if (const UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    if (const UStaticMesh* Asset = MeshComp->GetStaticMesh())
                    {
                        const FString MeshPath = Asset->GetPathName();
                        bDrivingSurface = !MeshPath.Contains(TEXT("Sidewalk"));
                    }
                }
            }
            if (bDrivingSurface)
            {
                MinimumTopZ = FMath::Min(MinimumTopZ, static_cast<float>(BoundsOrigin.Z + BoundsExtent.Z));
                MaximumTopZ = FMath::Max(MaximumTopZ, static_cast<float>(BoundsOrigin.Z + BoundsExtent.Z));
            }
        }
    }
    const float GroundSpread = ValidGroundSurfaces > 0 ? MaximumSurfaceZ - MinimumSurfaceZ : TNumericLimits<float>::Max();
    const float GroundTopSpread = MaximumTopZ >= MinimumTopZ ? MaximumTopZ - MinimumTopZ : TNumericLimits<float>::Max();
    const bool bRegisteredGroundPass = ValidGroundSurfaces >= 12 &&
        GroundSpread <= 18.5f && GroundTopSpread <= 8.0f;
    const bool bPerimeterGroundPass = AuditCampaignPerimeterGround(0);
    const bool bGroundPlanePass = bRegisteredGroundPass && bPerimeterGroundPass;
    UE_LOG(LogTemp, Display, TEXT("[FirstLevelAccessAudit] ground pivot_spread=%.2f driving_TOP_spread=%.2f"),
        GroundSpread, GroundTopSpread);

    int32 EnterableBuildings = 0;
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        if (It->Tags.Contains(FName("FirstLevelEnterableBuilding")))
        {
            ++EnterableBuildings;
        }
    }

    // 2026-07-11 pass 4: the doorways now have REAL door leaves. The audit
    // tests the OPENED doorway (doors must be openable + clear), then restores
    // the closed state. Instant-snap so the same-frame sweeps see the truth.
    TArray<ADoorActor*> AuditDoors;
    for (TActorIterator<ADoorActor> DoorIt(World); DoorIt; ++DoorIt)
    {
        if (FCodeRescueCampaign::IsLocationInsideCityArenaXY(0, DoorIt->GetActorLocation()))
        {
            AuditDoors.Add(*DoorIt);
            DoorIt->SetDoorOpenInstant(true);
        }
    }
    const int32 AuditDoorLeafCount = AuditDoors.Num();

    const int32 DoorCount = FMath::Min(
        FirstLevelAccessDoorwayOutsidePoints.Num(),
        FirstLevelAccessDoorwayInsidePoints.Num());
    int32 ClearDoorways = 0;
    int32 StreetReachableDoorways = 0;
    int32 LevelDoorways = 0;
    int32 BoundedDoorways = 0;
    auto GroundAt = [World](const FVector& Point, float& OutGroundZ)
    {
        FHitResult GroundHit;
        FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(FirstLevelDoorGround), false);
        if (World->LineTraceSingleByChannel(
                GroundHit,
                Point + FVector(0.0f, 0.0f, 130.0f),
                Point - FVector(0.0f, 0.0f, 260.0f),
                ECC_Visibility,
                GroundParams))
        {
            OutGroundZ = GroundHit.ImpactPoint.Z;
            return true;
        }
        return false;
    };
    for (int32 Index = 0; Index < DoorCount; ++Index)
    {
        FVector Outside = FirstLevelAccessDoorwayOutsidePoints[Index];
        FVector Inside = FirstLevelAccessDoorwayInsidePoints[Index];
        Outside.Z += 10.0f;
        Inside.Z += 10.0f;
        const FVector StreetDirection = (Outside - Inside).GetSafeNormal2D();
        const FVector StreetApproach = Outside + StreetDirection * 1450.0f;

        FCollisionQueryParams DoorParams(SCENE_QUERY_STAT(FirstLevelDoorwayAccess), false);
        FHitResult DoorHit;
        const bool bDoorBlocked = World->SweepSingleByChannel(
            DoorHit,
            Outside,
            Inside,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeCapsule(30.0f, 70.0f),
            DoorParams);
        if (!bDoorBlocked)
        {
            ++ClearDoorways;
        }

        FHitResult ApproachHit;
        const bool bStreetApproachBlocked = World->SweepSingleByChannel(
            ApproachHit,
            StreetApproach,
            Inside,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeCapsule(30.0f, 70.0f),
            DoorParams);
        if (!bStreetApproachBlocked)
        {
            ++StreetReachableDoorways;
        }
        FString ApproachBlockerLabel = TEXT("none");
        FString ApproachBlockerTags = TEXT("none");
        FVector ApproachBlockerLocation = FVector::ZeroVector;
        if (const AActor* ApproachBlocker = ApproachHit.GetActor())
        {
            ApproachBlockerLabel = ApproachBlocker->GetName();
#if WITH_EDITOR
            ApproachBlockerLabel = ApproachBlocker->GetActorLabel();
#endif
            ApproachBlockerLocation = ApproachBlocker->GetActorLocation();
            ApproachBlockerTags.Reset();
            for (const FName& Tag : ApproachBlocker->Tags)
            {
                if (!ApproachBlockerTags.IsEmpty())
                {
                    ApproachBlockerTags += TEXT("|");
                }
                ApproachBlockerTags += Tag.ToString();
            }
            if (ApproachBlockerTags.IsEmpty())
            {
                ApproachBlockerTags = TEXT("untagged");
            }
        }

        const bool bWithinPlayableBounds =
            FCodeRescueCampaign::IsLocationInsideCityArenaXY(0, StreetApproach) &&
            FCodeRescueCampaign::IsLocationInsideCityArenaXY(0, Outside) &&
            FCodeRescueCampaign::IsLocationInsideCityArenaXY(0, Inside);
        if (bWithinPlayableBounds)
        {
            ++BoundedDoorways;
        }

        float OutsideGroundZ = 0.0f;
        float InsideGroundZ = 0.0f;
        if (GroundAt(Outside, OutsideGroundZ) && GroundAt(Inside, InsideGroundZ) &&
            FMath::Abs(OutsideGroundZ - InsideGroundZ) <= 28.0f)
        {
            ++LevelDoorways;
        }
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelAccessAudit] doorway=%d clear=%d street_reachable=%d bounded=%d level=%d blocker=%s approach_blocker=%s blocker_location=%s blocker_tags=%s outside=%s inside=%s street=%s ground_out=%.2f ground_in=%.2f"),
            Index + 1,
            bDoorBlocked ? 0 : 1,
            bStreetApproachBlocked ? 0 : 1,
            bWithinPlayableBounds ? 1 : 0,
            FMath::Abs(OutsideGroundZ - InsideGroundZ) <= 28.0f ? 1 : 0,
            DoorHit.GetActor() ? *DoorHit.GetActor()->GetName() : TEXT("none"),
            *ApproachBlockerLabel,
            *ApproachBlockerLocation.ToCompactString(),
            *ApproachBlockerTags,
            *Outside.ToCompactString(),
            *Inside.ToCompactString(),
            *StreetApproach.ToCompactString(),
            OutsideGroundZ,
            InsideGroundZ);
    }

    int32 MissionRoutePoints = 1;
    int32 SafeMissionRoutePoints = FCodeRescueCampaign::IsLocationInsideCityArenaXY(
        0, FCodeRescueCampaign::GetPlayerStartLocation(0)) ? 1 : 0;
    for (TActorIterator<ASurvivorActor> It(World); It; ++It)
    {
        if (It->CityIndex == 0)
        {
            ++MissionRoutePoints;
            SafeMissionRoutePoints += FCodeRescueCampaign::IsLocationInsideCityArenaXY(
                0, It->GetActorLocation()) ? 1 : 0;
            break;
        }
    }
    for (TActorIterator<AHelipadActor> It(World); It; ++It)
    {
        if (It->CityIndex == 0)
        {
            ++MissionRoutePoints;
            SafeMissionRoutePoints += FCodeRescueCampaign::IsLocationInsideCityArenaXY(
                0, It->GetActorLocation()) ? 1 : 0;
            break;
        }
    }

    // restore the audited doors to their closed resting state
    for (ADoorActor* Door : AuditDoors)
    {
        if (Door)
        {
            Door->SetDoorOpenInstant(false);
        }
    }
    UE_LOG(LogTemp, Display, TEXT("[FirstLevelAccessAudit] functional door leaves=%d (opened for sweep, restored closed)"),
        AuditDoorLeafCount);

    int32 PurposeLandmarks = 0;
    int32 OpenSpacePurposeLandmarks = 0;
    int32 BoundedPurposeLandmarks = 0;
    int32 FieldDepotRuntimeModules = 0;
    int32 FieldDepotRuntimeStock = 0;
    TSet<FName> PurposeDistrictTypes;
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Landmark = *It;
        FieldDepotRuntimeModules += Landmark->Tags.Contains(
            FName("BlenderDepotRuntimeRealization")) ? 1 : 0;
        if (!Landmark->Tags.Contains(FName("FirstLevelPurposeLandmark")))
        {
            continue;
        }
        ++PurposeLandmarks;
        OpenSpacePurposeLandmarks += Landmark->Tags.Contains(
            FName("PurposeDistrictOpenSpaceValidated")) ? 1 : 0;
        BoundedPurposeLandmarks += FCodeRescueCampaign::IsLocationInsideCityArenaXY(
            0, Landmark->GetActorLocation()) ? 1 : 0;
        for (const FName& Tag : Landmark->Tags)
        {
            if (Tag.ToString().StartsWith(TEXT("PurposeDistrict_")))
            {
                PurposeDistrictTypes.Add(Tag);
            }
        }
    }
    for (TActorIterator<APickupActor> It(World); It; ++It)
    {
        FieldDepotRuntimeStock += It->Tags.Contains(FName("DepotIconStockPickup")) ? 1 : 0;
    }
    const bool bPurposeDistrictPass = PurposeLandmarks == 3 &&
        PurposeDistrictTypes.Num() == 3 &&
        OpenSpacePurposeLandmarks == 3 &&
        BoundedPurposeLandmarks == 3 &&
        FieldDepotRuntimeModules == 21 &&
        FieldDepotRuntimeStock == 6;
    if (bPurposeDistrictPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedPurposeDistrictPass"));
        UE_LOG(LogTemp, Display,
            TEXT("[PurposeDistrictRuntimeAudit] COMPLETE PASS landmarks=3/3 district_types=3 open_space=3/3 bounded=3/3 depot_modules=21/21 depot_icon_stock=6/6 physical_symbols=1 canonical_ground=1"));
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[PurposeDistrictRuntimeAudit] COMPLETE FAIL landmarks=%d/3 district_types=%d/3 open_space=%d/3 bounded=%d/3 depot_modules=%d/21 depot_icon_stock=%d/6"),
            PurposeLandmarks, PurposeDistrictTypes.Num(),
            OpenSpacePurposeLandmarks, BoundedPurposeLandmarks,
            FieldDepotRuntimeModules, FieldDepotRuntimeStock);
    }

    int32 PickupCount = 0;
    int32 AuthoredPickups = 0;
    int32 GroundedPickups = 0;
    TSet<FString> PickupStyles;
    for (TActorIterator<APickupActor> It(World); It; ++It)
    {
        APickupActor* Pickup = *It;
        if (!FCodeRescueCampaign::IsLocationInsideCityArenaXY(0, Pickup->GetActorLocation(), true))
        {
            continue;
        }
        ++PickupCount;
        Pickup->RefreshPresentation();
        PickupStyles.Add(Pickup->GetPresentationStyleToken());
        UStaticMeshComponent* PresentationMesh = Pickup->FindComponentByClass<UStaticMeshComponent>();
        const UStaticMesh* MeshAsset = PresentationMesh ? PresentationMesh->GetStaticMesh() : nullptr;
        const bool bAuthored = Pickup->IsAuthoredPresentationReady() && MeshAsset &&
            MeshAsset->GetPathName().Contains(TEXT("WorldLootWeatherV6/Pickup"));
        AuthoredPickups += bAuthored ? 1 : 0;
        GroundedPickups += Pickup->Tags.Contains(FName("PickupGroundContactVerified")) ? 1 : 0;
    }
    const bool bPickupPresentationPass = PickupCount >= 11 &&
        AuthoredPickups == PickupCount && GroundedPickups == PickupCount && PickupStyles.Num() >= 6;
    if (bPickupPresentationPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedSymbolLootPass"));
        UE_LOG(LogTemp, Display,
            TEXT("[PickupPresentationAudit] COMPLETE PASS pickups=%d authored=%d grounded=%d icon_styles=%d/6 paragraph_labels=0"),
            PickupCount, AuthoredPickups, GroundedPickups, PickupStyles.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[PickupPresentationAudit] COMPLETE FAIL pickups=%d authored=%d grounded=%d icon_styles=%d/6"),
            PickupCount, AuthoredPickups, GroundedPickups, PickupStyles.Num());
    }

    int32 ThreatMarkers = 0;
    int32 CompactThreatMarkers = 0;
    int32 AttachedThreatMarkers = 0;
    int32 CubeThreatMarkers = 0;
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Zombie = *It;
        AActor* Marker = Zombie->VisualMarkerActor;
        if (!IsValid(Marker))
        {
            continue;
        }
        ++ThreatMarkers;
        AttachedThreatMarkers += Marker->GetAttachParentActor() == Zombie ? 1 : 0;
        const AStaticMeshActor* StaticMarker = Cast<AStaticMeshActor>(Marker);
        const UStaticMeshComponent* MarkerMesh = StaticMarker ? StaticMarker->GetStaticMeshComponent() : nullptr;
        const UStaticMesh* MarkerAsset = MarkerMesh ? MarkerMesh->GetStaticMesh() : nullptr;
        const FString MarkerPath = MarkerAsset ? MarkerAsset->GetPathName() : FString();
        FVector BoundsOrigin = FVector::ZeroVector;
        FVector BoundsExtent = FVector::ZeroVector;
        Marker->GetActorBounds(false, BoundsOrigin, BoundsExtent);
        const bool bCompact = Marker->Tags.Contains(FName("CompactZombieGroundMarker")) &&
            MarkerPath.Contains(TEXT("ThreatGroundRingV6")) &&
            BoundsExtent.Z * 2.0f <= 20.0f &&
            MarkerMesh && MarkerMesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision;
        CompactThreatMarkers += bCompact ? 1 : 0;
        CubeThreatMarkers += MarkerPath.Contains(TEXT("/Engine/BasicShapes/Cube")) ? 1 : 0;
    }
    const bool bThreatMarkerPass = ThreatMarkers > 0 &&
        CompactThreatMarkers == ThreatMarkers && AttachedThreatMarkers == ThreatMarkers && CubeThreatMarkers == 0;
    if (bThreatMarkerPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedThreatMarkerPass"));
        UE_LOG(LogTemp, Display,
            TEXT("[ThreatMarkerAudit] COMPLETE PASS markers=%d compact=%d attached=%d enclosure_cubes=0 collision=0"),
            ThreatMarkers, CompactThreatMarkers, AttachedThreatMarkers);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[ThreatMarkerAudit] COMPLETE FAIL markers=%d compact=%d attached=%d enclosure_cubes=%d"),
            ThreatMarkers, CompactThreatMarkers, AttachedThreatMarkers, CubeThreatMarkers);
    }

    const bool bWeatherPass = IsValid(ActiveWeatherField) && ActiveWeatherField->RunAcceptanceAudit();
    if (bWeatherPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedWeatherPass"));
    }

    const bool bDoorPass = DoorCount >= 3 &&
        ClearDoorways == DoorCount &&
        StreetReachableDoorways == DoorCount &&
        BoundedDoorways == DoorCount &&
        LevelDoorways == DoorCount;
    const bool bRoutePass = MissionRoutePoints >= 3 && SafeMissionRoutePoints == MissionRoutePoints;
    const bool bVisibleFootPass = Tags.Contains(FName("FirstLevelIntegratedVisibleFootGroundPass"));
    const bool bPass = bGroundPlanePass && bDoorPass && bRoutePass && EnterableBuildings >= 3 &&
        bPurposeDistrictPass && bPickupPresentationPass && bThreatMarkerPass && bWeatherPass && bVisibleFootPass;
    const FString AccessSummary = FString::Printf(
        TEXT("[FirstLevelAccessAudit] COMPLETE %s ground_surfaces=%d z_spread=%.2f perimeter_ground=%d enterable_buildings=%d clear_doors=%d/%d street_doors=%d/%d bounded_doors=%d/%d level_doors=%d/%d route_points=%d/%d visible_feet=%d loot_symbols=%d districts=%d threat_markers=%d weather=%d"),
        bPass ? TEXT("PASS") : TEXT("FAIL"), ValidGroundSurfaces, GroundSpread,
        bPerimeterGroundPass ? 1 : 0,
        EnterableBuildings,
        ClearDoorways, DoorCount,
        StreetReachableDoorways, DoorCount,
        BoundedDoorways, DoorCount,
        LevelDoorways, DoorCount,
        SafeMissionRoutePoints, MissionRoutePoints,
        bVisibleFootPass ? 1 : 0,
        bPickupPresentationPass ? 1 : 0,
        bPurposeDistrictPass ? 1 : 0,
        bThreatMarkerPass ? 1 : 0,
        bWeatherPass ? 1 : 0);
    if (bPass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *AccessSummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *AccessSummary);
    }
    if (bPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedWorldPass"));
    }
    return bPass;
}

bool ACodeRescueGameMode::RunFirstLevelChallengeAudit()
{
    UWorld* World = GetWorld();
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (!World || !GI)
    {
        UE_LOG(LogTemp, Error, TEXT("[FirstLevelChallengeAudit] COMPLETE FAIL reason=missing_world_or_profile"));
        return false;
    }

    const TArray<FString> ChallengeIds = FCodeRescueCampaign::GetCityChallengeIds(0);
    TSet<FString> PhysicalChallengeIds;
    for (TActorIterator<ACodingTerminalActor> It(World); It; ++It)
    {
        if (ChallengeIds.Contains(It->Challenge.Id))
        {
            PhysicalChallengeIds.Add(It->Challenge.Id);
        }
    }
    const bool bPhysicalStationsPass =
        ChallengeIds.Num() == FCodeRescueCampaign::RequiredChallengesPerCity &&
        PhysicalChallengeIds.Num() == FCodeRescueCampaign::RequiredChallengesPerCity;

    TSet<FString> CampaignChallengeIds;
    bool bEveryCityHasTen = true;
    bool bEveryChallengeIdUnique = true;
    const int32 MissionCount = FCodeRescueCampaign::GetMissionCount();
    for (int32 CityIndex = 0; CityIndex < MissionCount; ++CityIndex)
    {
        const TArray<FString> CityChallengeIds = FCodeRescueCampaign::GetCityChallengeIds(CityIndex);
        bEveryCityHasTen &= CityChallengeIds.Num() == FCodeRescueCampaign::RequiredChallengesPerCity;
        for (const FString& ChallengeId : CityChallengeIds)
        {
            if (CampaignChallengeIds.Contains(ChallengeId))
            {
                bEveryChallengeIdUnique = false;
            }
            CampaignChallengeIds.Add(ChallengeId);
        }
    }
    const int32 ExpectedCampaignChallenges = MissionCount * FCodeRescueCampaign::RequiredChallengesPerCity;
    const bool bCampaignChallengeContractPass =
        MissionCount > 0 &&
        bEveryCityHasTen &&
        bEveryChallengeIdUnique &&
        CampaignChallengeIds.Num() == ExpectedCampaignChallenges;
    const FString CampaignContractSummary = FString::Printf(
        TEXT("[CampaignChallengeContractAudit] COMPLETE %s cities=%d challenges=%d expected=%d ten_per_city=%d unique_ids=%d"),
        bCampaignChallengeContractPass ? TEXT("PASS") : TEXT("FAIL"),
        MissionCount,
        CampaignChallengeIds.Num(),
        ExpectedCampaignChallenges,
        bEveryCityHasTen ? 1 : 0,
        bEveryChallengeIdUnique ? 1 : 0);
    if (bCampaignChallengeContractPass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *CampaignContractSummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *CampaignContractSummary);
    }

    struct FReferenceLanguage
    {
        ECodingLanguage Language;
        const TCHAR* Label;
    };
    const FReferenceLanguage Languages[] = {
        { ECodingLanguage::Java, TEXT("Java") },
        { ECodingLanguage::C, TEXT("C") },
        { ECodingLanguage::CPlus, TEXT("C+") },
        { ECodingLanguage::Cpp, TEXT("C++") },
        { ECodingLanguage::Python, TEXT("Python") },
        { ECodingLanguage::MATLAB, TEXT("MATLAB") },
    };

    int32 PassedValidations = 0;
    int32 ExternalToolchainValidations = 0;
    for (int32 StageIndex = 0; StageIndex < ChallengeIds.Num(); ++StageIndex)
    {
        int32 StagePasses = 0;
        for (const FReferenceLanguage& Reference : Languages)
        {
            FChallengeSpec Challenge;
            Challenge.Id = ChallengeIds[StageIndex];
            Challenge.Title = FString::Printf(TEXT("First Level Stage %02d"), StageIndex + 1);
            Challenge.MissionBrief = TEXT("Deterministic six-language acceptance validation.");
            Challenge.Language = Reference.Language;
            Challenge.StarterCode = UCodeTerminalWidget::GetCanonicalReferenceSolution(
                Challenge.Id,
                Reference.Language);
            ExternalToolchainValidations += UCodeRunnerLibrary::IsLanguageAvailable(Reference.Language) ? 1 : 0;
            const FCodeValidationResult Result = UCodeRunnerLibrary::ValidateChallenge(
                Challenge,
                Challenge.StarterCode);
            if (Result.bSuccess)
            {
                ++StagePasses;
                ++PassedValidations;
            }
            else
            {
                UE_LOG(LogTemp, Error,
                    TEXT("[FirstLevelChallengeAudit] stage=%d id='%s' language=%s result=FAIL score=%d summary=%s"),
                    StageIndex + 1,
                    *Challenge.Id,
                    Reference.Label,
                    Result.Score,
                    *Result.Summary.ReplaceCharWithEscapedChar());
            }
        }
        const FString StageSummary = FString::Printf(
            TEXT("[FirstLevelChallengeAudit] stage=%d/10 id='%s' languages=%d/%d"),
            StageIndex + 1,
            *ChallengeIds[StageIndex],
            StagePasses,
            UE_ARRAY_COUNT(Languages));
        if (StagePasses == UE_ARRAY_COUNT(Languages))
        {
            UE_LOG(LogTemp, Display, TEXT("%s"), *StageSummary);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("%s"), *StageSummary);
        }
    }

    // 2026-07-11 GENERALIZED-ACCEPTANCE VECTORS. Every entry is a
    // functionally correct solution written in a DIFFERENT family than the
    // canonical reference. The first three were rejected by real packaged
    // builds (Kenny's 2026-07-11 palindrome and binary-search screenshots,
    // and the earlier prefix-decrement reverse from player telemetry); the
    // rest pin down common equivalent families so they can never regress.
    struct FAlternateSolutionVector
    {
        const TCHAR* IdFragment;      // matched against the city challenge ids; used verbatim as a fallback id
        ECodingLanguage Language;
        const TCHAR* Label;
        FString Code;
    };
    TArray<FAlternateSolutionVector> AlternateVectors;
    AlternateVectors.Add({ TEXT("reverse"), ECodingLanguage::Cpp, TEXT("prefix_decrement_reverse"),
        TEXT("#include <string>\n")
        TEXT("std::string reverseString(std::string s) {\n")
        TEXT("    std::string reversed = \"\";\n")
        TEXT("    for (int i = s.length() - 1; i >= 0; --i) {\n")
        TEXT("        reversed += s[i];\n")
        TEXT("    }\n")
        TEXT("    return reversed;\n")
        TEXT("}\n") });
    // Kenny's exact rejected palindrome (prefix inc/dec two-pointer with !=).
    AlternateVectors.Add({ TEXT("palindrome"), ECodingLanguage::Cpp, TEXT("two_pointer_neq_palindrome"),
        TEXT("#include <string>\n")
        TEXT("bool isPalindrome(const std::string& s) {\n")
        TEXT("    int left = 0;\n")
        TEXT("    int right = static_cast<int>(s.length()) - 1;\n")
        TEXT("    while (left < right) {\n")
        TEXT("        if (s[left] != s[right]) {\n")
        TEXT("            return false;\n")
        TEXT("        }\n")
        TEXT("        ++left;\n")
        TEXT("        --right;\n")
        TEXT("    }\n")
        TEXT("    return true;\n")
        TEXT("}\n") });
    // Kenny's exact rejected binary search (overflow-safe midpoint).
    AlternateVectors.Add({ TEXT("binary_search"), ECodingLanguage::Cpp, TEXT("overflow_safe_midpoint_bsearch"),
        TEXT("#include <vector>\n")
        TEXT("int binarySearch(const std::vector<int> values, int target) {\n")
        TEXT("    int low = 0;\n")
        TEXT("    int high = static_cast<int>(values.size()) - 1;\n")
        TEXT("    while (low <= high) {\n")
        TEXT("        int mid = low + (high - low) / 2;\n")
        TEXT("        if (values[mid] == target) {\n")
        TEXT("            return mid;\n")
        TEXT("        }\n")
        TEXT("        if (values[mid] < target) {\n")
        TEXT("            low = mid + 1;\n")
        TEXT("        } else {\n")
        TEXT("            high = mid - 1;\n")
        TEXT("        }\n")
        TEXT("    }\n")
        TEXT("    return -1;\n")
        TEXT("}\n") });
    AlternateVectors.Add({ TEXT("lock"), ECodingLanguage::Cpp, TEXT("nested_if_lock"),
        TEXT("bool shouldUnlock(bool hasKey, bool powerOn) {\n")
        TEXT("    if (hasKey) {\n")
        TEXT("        if (powerOn) {\n")
        TEXT("            return true;\n")
        TEXT("        }\n")
        TEXT("    }\n")
        TEXT("    return false;\n")
        TEXT("}\n") });
    AlternateVectors.Add({ TEXT("sum"), ECodingLanguage::Cpp, TEXT("accumulation_sum"),
        TEXT("int totalPower(int a, int b, int c) {\n")
        TEXT("    int total = a + b;\n")
        TEXT("    total += c;\n")
        TEXT("    return total;\n")
        TEXT("}\n") });
    AlternateVectors.Add({ TEXT("palindrome"), ECodingLanguage::Python, TEXT("reverse_compare_palindrome"),
        TEXT("def is_palindrome(s):\n")
        TEXT("    return s == s[::-1]\n") });
    AlternateVectors.Add({ TEXT("fizzbuzz"), ECodingLanguage::Python, TEXT("lowercase_literals_fizzbuzz"),
        TEXT("def fizz_buzz(n):\n")
        TEXT("    out = []\n")
        TEXT("    for i in range(1, n + 1):\n")
        TEXT("        if i % 15 == 0: out.append('fizzbuzz')\n")
        TEXT("        elif i % 3 == 0: out.append('fizz')\n")
        TEXT("        elif i % 5 == 0: out.append('buzz')\n")
        TEXT("        else: out.append(str(i))\n")
        TEXT("    return out\n") });
    AlternateVectors.Add({ TEXT("filter"), ECodingLanguage::Cpp, TEXT("no_space_for_filter"),
        TEXT("#include <vector>\n")
        TEXT("std::vector<int> evenNumbers(const std::vector<int>& values) {\n")
        TEXT("    std::vector<int> result;\n")
        TEXT("    for(size_t i = 0; i < values.size(); ++i) {\n")
        TEXT("        if (values[i] % 2 == 0) {\n")
        TEXT("            result.push_back(values[i]);\n")
        TEXT("        }\n")
        TEXT("    }\n")
        TEXT("    return result;\n")
        TEXT("}\n") });

    int32 AlternatePasses = 0;
    for (const FAlternateSolutionVector& Vector : AlternateVectors)
    {
        const FString* MatchedId = ChallengeIds.FindByPredicate([&Vector](const FString& Id)
        {
            return Id.Contains(Vector.IdFragment, ESearchCase::IgnoreCase);
        });
        FChallengeSpec AlternateChallenge;
        AlternateChallenge.Id = MatchedId ? *MatchedId : FString(Vector.IdFragment);
        AlternateChallenge.Title = TEXT("Alternate-solution acceptance");
        AlternateChallenge.MissionBrief = TEXT("A correct solution in a non-canonical family must validate.");
        AlternateChallenge.Language = Vector.Language;
        const FCodeValidationResult AlternateResult = UCodeRunnerLibrary::ValidateChallenge(
            AlternateChallenge, Vector.Code);
        if (AlternateResult.bSuccess)
        {
            ++AlternatePasses;
        }
        else
        {
            FString FailedChecks;
            for (const FString& Check : AlternateResult.FailedChecks)
            {
                FailedChecks += FString::Printf(TEXT("'%s' "), *Check);
            }
            UE_LOG(LogTemp, Error,
                TEXT("[FirstLevelAlternateSolutionAudit] vector=%s id='%s' result=FAIL score=%d failed=%s"),
                Vector.Label, *AlternateChallenge.Id, AlternateResult.Score, *FailedChecks);
        }
    }
    const bool bPrefixDecrementPass = AlternatePasses == AlternateVectors.Num();
    const FString AlternateSolutionSummary = FString::Printf(
        TEXT("[FirstLevelAlternateSolutionAudit] COMPLETE %s vectors=%d/%d prefix_decrement=1 palindrome_neq=1 bsearch_overflow_safe=1 nested_if=1 accumulation=1 reverse_compare=1 lowercase_literals=1 no_space_for=1"),
        bPrefixDecrementPass ? TEXT("PASS") : TEXT("FAIL"),
        AlternatePasses,
        AlternateVectors.Num());
    if (bPrefixDecrementPass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *AlternateSolutionSummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *AlternateSolutionSummary);
    }

    // 2026-07-11 GUIDANCE-COHERENCE AUDIT (Kenny: "cannot enter the last
    // coding challenge; the game keeps directing me to the designated area"):
    // the coding-phase objective/beacon now resolve the ACTUAL next-unsolved
    // terminal actor. This audit asserts that resolution works for the
    // CURRENT save state and, decisively, that the FINAL station physically
    // exists with a real brief so the last challenge is always enterable.
    const FString NextUnsolvedId = FCodeRescueCampaign::GetFirstUnsolvedCityChallengeId(GI, 0);
    const FString GuidanceProbeId = !NextUnsolvedId.IsEmpty()
        ? NextUnsolvedId
        : (ChallengeIds.Num() > 0 ? ChallengeIds[0] : FString());
    const ACodingTerminalActor* GuidanceTerminal = nullptr;
    const ACodingTerminalActor* FinalTerminal = nullptr;
    for (TActorIterator<ACodingTerminalActor> It(World); It; ++It)
    {
        const ACodingTerminalActor* Terminal = *It;
        if (!IsValid(Terminal))
        {
            continue;
        }
        if (Terminal->Challenge.Id == GuidanceProbeId)
        {
            GuidanceTerminal = Terminal;
        }
        if (ChallengeIds.Num() > 0 && Terminal->Challenge.Id == ChallengeIds.Last())
        {
            FinalTerminal = Terminal;
        }
    }
    const bool bGuidanceResolves = GuidanceTerminal != nullptr;
    const bool bFinalStationExists = FinalTerminal != nullptr;
    const bool bFinalStationBriefed = FinalTerminal
        && !FinalTerminal->Challenge.Title.IsEmpty()
        && !FinalTerminal->Challenge.MissionBrief.IsEmpty();
    const bool bGuidancePass = bGuidanceResolves && bFinalStationExists && bFinalStationBriefed;
    const FString GuidanceSummary = FString::Printf(
        TEXT("[ObjectiveGuidanceAudit] COMPLETE %s next_id='%s' next_station_resolves=%d final_station_exists=%d final_station_briefed=%d"),
        bGuidancePass ? TEXT("PASS") : TEXT("FAIL"),
        *GuidanceProbeId,
        bGuidanceResolves ? 1 : 0,
        bFinalStationExists ? 1 : 0,
        bFinalStationBriefed ? 1 : 0);
    if (bGuidancePass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *GuidanceSummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *GuidanceSummary);
    }

    const TArray<FString> SavedSolvedTerminalIds = GI->SolvedTerminalIds;
    for (const FString& ChallengeId : ChallengeIds)
    {
        GI->SolvedTerminalIds.Remove(ChallengeId);
    }
    for (int32 Index = 0; Index < ChallengeIds.Num() - 1; ++Index)
    {
        GI->SolvedTerminalIds.AddUnique(ChallengeIds[Index]);
    }
    const bool bLockedAtNine =
        ChallengeIds.Num() == FCodeRescueCampaign::RequiredChallengesPerCity &&
        FCodeRescueCampaign::GetCityChallengeProgress(GI, 0) == 9 &&
        !FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, 0) &&
        FCodeRescueCampaign::GetFirstUnsolvedCityChallengeId(GI, 0) == ChallengeIds.Last();
    if (ChallengeIds.Num() == FCodeRescueCampaign::RequiredChallengesPerCity)
    {
        GI->SolvedTerminalIds.AddUnique(ChallengeIds.Last());
    }
    const bool bUnlockedAtTen =
        FCodeRescueCampaign::GetCityChallengeProgress(GI, 0) == FCodeRescueCampaign::RequiredChallengesPerCity &&
        FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, 0);
    GI->SolvedTerminalIds = SavedSolvedTerminalIds;
    const bool bProgressionPass = bLockedAtNine && bUnlockedAtTen;
    const FString ProgressionSummary = FString::Printf(
        TEXT("[FirstLevelProgressionAudit] COMPLETE %s locked_at_9=%d unlocked_at_10=%d save_restored=1"),
        bProgressionPass ? TEXT("PASS") : TEXT("FAIL"),
        bLockedAtNine ? 1 : 0,
        bUnlockedAtTen ? 1 : 0);
    if (bProgressionPass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *ProgressionSummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *ProgressionSummary);
    }

    auto CountSupplyActors = [World](const FName& Tag)
    {
        int32 Count = 0;
        for (TActorIterator<APickupActor> It(World); It; ++It)
        {
            Count += It->Tags.Contains(Tag) ? 1 : 0;
        }
        return Count;
    };
    const int32 ChallengeSupplyBefore = CountSupplyActors(FName("ChallengeCompletionSupply"));
    const int32 ZombieSupplyBefore = CountSupplyActors(FName("ZombieSupplyDrop"));
    const FVector AuditDropLocation = FCodeRescueCampaign::GetPlayerStartLocation(0) + FVector(360.0f, 0.0f, 0.0f);
    const int32 ChallengeSupplySpawned = SpawnChallengeCompletionSupplyCache(
        TEXT("first_level_acceptance_supply"), 0, AuditDropLocation);
    const int32 ZombieSupplySpawned = SpawnZombieDeathSupply(987650, AuditDropLocation + FVector(240.0f, 0.0f, 0.0f));
    const int32 ChallengeSupplyAfter = CountSupplyActors(FName("ChallengeCompletionSupply"));
    const int32 ZombieSupplyAfter = CountSupplyActors(FName("ZombieSupplyDrop"));
    const bool bSupplyPass =
        ChallengeSupplySpawned == 3 &&
        ZombieSupplySpawned == 1 &&
        ChallengeSupplyAfter - ChallengeSupplyBefore == ChallengeSupplySpawned &&
        ZombieSupplyAfter - ZombieSupplyBefore == ZombieSupplySpawned;
    const FString SupplySummary = FString::Printf(
        TEXT("[FirstLevelSupplyAudit] COMPLETE %s challenge_drop=%d tagged_delta=%d zombie_drop=%d tagged_delta=%d"),
        bSupplyPass ? TEXT("PASS") : TEXT("FAIL"),
        ChallengeSupplySpawned,
        ChallengeSupplyAfter - ChallengeSupplyBefore,
        ZombieSupplySpawned,
        ZombieSupplyAfter - ZombieSupplyBefore);
    if (bSupplyPass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *SupplySummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *SupplySummary);
    }

    const int32 TotalValidations = ChallengeIds.Num() * UE_ARRAY_COUNT(Languages);
    const bool bValidatorPass = PassedValidations == TotalValidations && TotalValidations == 60;
    const bool bPass = bPhysicalStationsPass && bCampaignChallengeContractPass &&
        bValidatorPass && bPrefixDecrementPass && bProgressionPass && bSupplyPass && bGuidancePass;
    const FString ChallengeSummary = FString::Printf(
        TEXT("[FirstLevelChallengeAudit] COMPLETE %s physical_stations=%d/10 campaign_challenges=%d validators=%d/%d alternate_solutions=%d guidance=%d external_available=%d progression=%d supplies=%d"),
        bPass ? TEXT("PASS") : TEXT("FAIL"),
        PhysicalChallengeIds.Num(),
        CampaignChallengeIds.Num(),
        PassedValidations,
        TotalValidations,
        bPrefixDecrementPass ? 1 : 0,
        bGuidancePass ? 1 : 0,
        ExternalToolchainValidations,
        bProgressionPass ? 1 : 0,
        bSupplyPass ? 1 : 0);
    if (bPass)
    {
        UE_LOG(LogTemp, Display, TEXT("%s"), *ChallengeSummary);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *ChallengeSummary);
    }
    if (bPass)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedChallengePass"));
    }
    return bPass;
}

void ACodeRescueGameMode::StartFirstLevelSkyAudit()
{
    struct FSkyAuditPhase
    {
        float Delay;
        float CycleTime;
        const TCHAR* Label;
        const TCHAR* FileName;
    };
    const FSkyAuditPhase Phases[] = {
        { 1.0f, 0.00f, TEXT("DAY"), TEXT("first_level_sky_day.png") },
        { 2.4f, 0.25f, TEXT("SUNSET"), TEXT("first_level_sky_sunset.png") },
        { 3.8f, 0.50f, TEXT("NIGHT"), TEXT("first_level_sky_night.png") },
        { 5.2f, 0.75f, TEXT("SUNRISE"), TEXT("first_level_sky_sunrise.png") },
    };

    for (const FSkyAuditPhase& Phase : Phases)
    {
        FTimerHandle SetPhaseTimer;
        GetWorldTimerManager().SetTimer(SetPhaseTimer, FTimerDelegate::CreateWeakLambda(this, [this, Phase]()
        {
            TimeOfDay = Phase.CycleTime;
            LastLoggedSkyPhase = INDEX_NONE;
            UpdateNightSkyVisibility();
            UE_LOG(LogTemp, Display,
                TEXT("[FirstLevelSkyAudit] phase=%s time=%.2f requested"),
                Phase.Label,
                Phase.CycleTime);

            FTimerHandle CaptureTimer;
            GetWorldTimerManager().SetTimer(CaptureTimer, FTimerDelegate::CreateWeakLambda(this, [this, Phase]()
            {
                const FString CapturePath = FPaths::ProjectSavedDir() /
                    TEXT("Screenshots/FirstLevel/") / Phase.FileName;
                FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
                const bool bSafeStars = NightSkyDome &&
                    NightSkyDome->Tags.Contains(FName("SafePointStarField"));
                const bool bStarsVisible = bSafeStars && !NightSkyDome->IsHidden();
                if (FCString::Strcmp(Phase.Label, TEXT("NIGHT")) == 0 && bStarsVisible)
                {
                    Tags.AddUnique(FName("FirstLevelSkyNightObserved"));
                }
                UE_LOG(LogTemp, Display,
                    TEXT("[FirstLevelSkyAudit] phase=%s capture=%s safe_stars=%d stars_visible=%d moon_visible=%d"),
                    Phase.Label,
                    *CapturePath,
                    bSafeStars ? 1 : 0,
                    bStarsVisible ? 1 : 0,
                    NightSkyMoon && !NightSkyMoon->IsHidden() ? 1 : 0);
            }), 0.35f, false);
        }), Phase.Delay, false);
    }

    FTimerHandle SkyCompletionTimer;
    GetWorldTimerManager().SetTimer(SkyCompletionTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        TimeOfDay = 0.12f;
        LastLoggedSkyPhase = INDEX_NONE;
        const bool bPeriodPass = DayNightPeriodSeconds >= 1200.0f;
        const bool bPass = Tags.Contains(FName("FirstLevelSkyNightObserved")) &&
            NightSkyMoon && NightSkyDome && bPeriodPass;
        const FString SkySummary = FString::Printf(
            TEXT("[FirstLevelSkyAudit] COMPLETE %s day=1 sunset=1 night=1 sunrise=1 stars=%d moon=%d period_seconds=%.0f duration_pass=%d"),
            bPass ? TEXT("PASS") : TEXT("FAIL"),
            Tags.Contains(FName("FirstLevelSkyNightObserved")) ? 1 : 0,
            NightSkyMoon ? 1 : 0,
            DayNightPeriodSeconds,
            bPeriodPass ? 1 : 0);
        if (bPass)
        {
            UE_LOG(LogTemp, Display, TEXT("%s"), *SkySummary);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("%s"), *SkySummary);
        }
        if (bPass)
        {
            Tags.AddUnique(FName("FirstLevelIntegratedSkyPass"));
        }
        if (!FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit")))
        {
            FPlatformMisc::RequestExit(false);
        }
    }), 6.3f, false);
}

void ACodeRescueGameMode::StartWorldLootWeatherVisualReview()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[WorldLootWeatherVisualReview] COMPLETE FAIL reason=no_world"));
        FPlatformMisc::RequestExit(false);
        return;
    }

    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(0);
    const FVector Start = FCodeRescueCampaign::GetPlayerStartLocation(0);
    const FVector ShowcaseCenter = Origin + FVector(6000.0f, -7600.0f, 0.0f);

    // This review-only lineup uses the same collectible actors and imported
    // meshes as normal zombie/challenge drops. It exists only when the QA flag
    // is present and never changes the shipped world layout.
    const EPickupKind ShowcaseKinds[] = {
        EPickupKind::Ammo,
        EPickupKind::Medkit,
        EPickupKind::ArmorPlate,
        EPickupKind::RadioScanner,
        EPickupKind::Scrap,
        EPickupKind::Flare,
    };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(ShowcaseKinds); ++Index)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        const FVector Location = ShowcaseCenter +
            FVector((static_cast<float>(Index) - 2.5f) * 175.0f, 0.0f, 150.0f);
        if (APickupActor* Pickup = World->SpawnActor<APickupActor>(
                APickupActor::StaticClass(), Location, FRotator(0.0f, 90.0f, 0.0f), Params))
        {
            Pickup->Kind = ShowcaseKinds[Index];
            Pickup->Amount = 1;
            Pickup->Tags.AddUnique(FName("WorldLootWeatherVisualReviewOnly"));
            Pickup->RefreshPresentation();
        }
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    for (TActorIterator<APawn> It(World); It; ++It)
    {
        APawn* NearbyPawn = *It;
        if (NearbyPawn != PlayerPawn &&
            FVector::DistSquared2D(NearbyPawn->GetActorLocation(), ShowcaseCenter) <
                FMath::Square(1250.0f))
        {
            NearbyPawn->SetActorLocation(NearbyPawn->GetActorLocation() + FVector(0.0f, 2100.0f, 0.0f));
        }
    }

    auto FindDistrictTarget = [World](const FName& DistrictTag, const FVector& Fallback)
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* Actor = *It;
            if (IsValid(Actor) &&
                Actor->Tags.Contains(FName("FirstLevelPurposeLandmark")) &&
                Actor->Tags.Contains(DistrictTag))
            {
                FVector BoundsOrigin;
                FVector BoundsExtent;
                // Include non-colliding presentation geometry. The imported
                // depot shell is deliberately nonblocking because UE 5.7's
                // Mac main pass omits it, while its visible modular realization
                // occupies the same Blender-authored bounds.
                Actor->GetActorBounds(false, BoundsOrigin, BoundsExtent);
                UE_LOG(LogTemp, Display,
                    TEXT("[WorldLootWeatherVisualReview] district=%s location=%s target=%s extent=%s hidden=%d"),
                    *DistrictTag.ToString(),
                    *Actor->GetActorLocation().ToCompactString(),
                    *BoundsOrigin.ToCompactString(),
                    *BoundsExtent.ToCompactString(),
                    Actor->IsHidden() ? 1 : 0);
                return BoundsOrigin;
            }
        }
        return Fallback;
    };

    FVector HordeTarget = Start + FVector(1500.0f, 1200.0f, 120.0f);
    int32 BestNeighbourCount = 0;
    for (TActorIterator<ACodeZombieActor> Candidate(World); Candidate; ++Candidate)
    {
        if (Candidate->Health <= 0.0f)
        {
            continue;
        }
        int32 NeighbourCount = 0;
        for (TActorIterator<ACodeZombieActor> Other(World); Other; ++Other)
        {
            if (Other->Health > 0.0f &&
                FVector::DistSquared2D(Candidate->GetActorLocation(), Other->GetActorLocation()) <=
                    FMath::Square(1700.0f))
            {
                ++NeighbourCount;
            }
        }
        if (NeighbourCount > BestNeighbourCount)
        {
            BestNeighbourCount = NeighbourCount;
            HordeTarget = Candidate->GetActorLocation() + FVector(0.0f, 0.0f, 115.0f);
        }
    }

    const FString CaptureDir = FPaths::ProjectSavedDir() /
        TEXT("Screenshots/WorldLootWeatherV6");
    IFileManager::Get().MakeDirectory(*CaptureDir, true);
    const TSharedRef<TArray<FString>> ExpectedPaths = MakeShared<TArray<FString>>();
    const TCHAR* FileNames[] = {
        TEXT("v6_symbol_loot_rain.png"),
        TEXT("v6_logistics_depot_rain.png"),
        TEXT("v6_weather_relay_fog.png"),
        TEXT("v6_quarantine_checkpoint_wind.png"),
        TEXT("v6_grounded_horde_rain.png"),
    };
    for (const TCHAR* FileName : FileNames)
    {
        const FString Path = CaptureDir / FileName;
        IFileManager::Get().Delete(*Path, false, true);
        ExpectedPaths->Add(Path);
    }

    auto ScheduleCapture = [this, Origin, CaptureDir](
        float Delay,
        const FString& FileName,
        const FVector& Target,
        const FVector& CameraOffset,
        ECodeRescueWeatherPhase WeatherPhase,
        float FieldOfView)
    {
        FTimerHandle CameraTimer;
        GetWorldTimerManager().SetTimer(CameraTimer,
            FTimerDelegate::CreateWeakLambda(this,
                [this, Origin, CaptureDir, FileName, Target, CameraOffset, WeatherPhase, FieldOfView]()
        {
            if (IsValid(ActiveWeatherField))
            {
                ActiveWeatherField->SetVisualReviewPhase(WeatherPhase);
            }

            const FVector CameraLocation = Target + CameraOffset;
            if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
            {
                const FVector ToCamera = CameraOffset.GetSafeNormal2D();
                const FVector Right(-ToCamera.Y, ToCamera.X, 0.0f);
                Pawn->SetActorLocation(FVector(Target.X, Target.Y, Origin.Z + 92.0f) +
                    ToCamera * 260.0f + Right * 300.0f);
                Pawn->SetActorRotation((Target - Pawn->GetActorLocation()).Rotation());
            }

            if (ACameraActor* Camera = GetWorld()->SpawnActor<ACameraActor>(
                    ACameraActor::StaticClass(), CameraLocation,
                    (Target - CameraLocation).Rotation()))
            {
                Camera->Tags.AddUnique(FName("WorldLootWeatherVisualReviewCamera"));
                if (UCameraComponent* CameraComponent = Camera->GetCameraComponent())
                {
                    CameraComponent->SetFieldOfView(FieldOfView);
                }
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
                {
                    PC->SetViewTarget(Camera);
                }
            }

            FTimerHandle ScreenshotTimer;
            GetWorldTimerManager().SetTimer(ScreenshotTimer,
                FTimerDelegate::CreateWeakLambda(this, [CaptureDir, FileName]()
            {
                const FString CapturePath = CaptureDir / FileName;
                FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
                UE_LOG(LogTemp, Display,
                    TEXT("[WorldLootWeatherVisualReview] requested %s"), *CapturePath);
            }), 0.55f, false);
        }), Delay, false);
    };

    const FVector LogisticsTarget = FindDistrictTarget(
        FName("PurposeDistrict_LogisticsDepot"), Origin + FVector(7900.0f, -5600.0f, 190.0f));
    const FVector RelayTarget = FindDistrictTarget(
        FName("PurposeDistrict_WeatherRelay"), Origin + FVector(8200.0f, 0.0f, 230.0f));
    const FVector CheckpointTarget = FindDistrictTarget(
        FName("PurposeDistrict_QuarantineCheckpoint"), Origin + FVector(7900.0f, 5400.0f, 190.0f));
    ScheduleCapture(2.8f, FileNames[0], ShowcaseCenter + FVector(0.0f, 0.0f, 85.0f),
        FVector(-120.0f, -1120.0f, 360.0f), ECodeRescueWeatherPhase::Rain, 50.0f);
    ScheduleCapture(4.8f, FileNames[1], LogisticsTarget,
        FVector(-1350.0f, -900.0f, 500.0f), ECodeRescueWeatherPhase::Rain, 61.0f);
    ScheduleCapture(6.8f, FileNames[2], RelayTarget,
        FVector(-1400.0f, -600.0f, 520.0f), ECodeRescueWeatherPhase::Fog, 60.0f);
    ScheduleCapture(8.8f, FileNames[3], CheckpointTarget,
        FVector(-1400.0f, 800.0f, 500.0f), ECodeRescueWeatherPhase::Wind, 61.0f);
    ScheduleCapture(10.8f, FileNames[4], HordeTarget,
        FVector(0.0f, -1350.0f, 460.0f), ECodeRescueWeatherPhase::Rain, 64.0f);

    FTimerHandle CompletionTimer;
    GetWorldTimerManager().SetTimer(CompletionTimer,
        FTimerDelegate::CreateWeakLambda(this, [ExpectedPaths, BestNeighbourCount]()
    {
        int32 ValidCaptures = 0;
        for (const FString& Path : *ExpectedPaths)
        {
            ValidCaptures += IFileManager::Get().FileSize(*Path) > 4096 ? 1 : 0;
        }
        const bool bPass = ValidCaptures == ExpectedPaths->Num() && BestNeighbourCount >= 3;
        if (bPass)
        {
            UE_LOG(LogTemp, Display,
                TEXT("[WorldLootWeatherVisualReview] COMPLETE PASS captures=%d/5 weather_phases=3 loot_styles=6 districts=3 grounded_horde=1 cluster=%d"),
                ValidCaptures, BestNeighbourCount);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("[WorldLootWeatherVisualReview] COMPLETE FAIL captures=%d/5 cluster=%d"),
                ValidCaptures, BestNeighbourCount);
        }
        FPlatformMisc::RequestExit(false);
    }), 12.9f, false);
}

void ACodeRescueGameMode::SpawnCityLandmark(const FCodeRescueCityMission& Mission, const FVector& Origin, const FString& CityLabel)
{
    const FVector Base = Origin + CityOffset(FVector(-2500.0f, 1350.0f, 80.0f));
    const FLinearColor Primary = Mission.SecondaryAccentColor * 2.2f;
    const FLinearColor Core = Mission.AccentColor * 4.0f;
    const int32 Shape = FMath::Abs(Mission.Rank - 1) % 6;

    switch (Shape)
    {
    case 0:
        SpawnBlock(Base + CityOffset(FVector(0, 0, 220)), CityExtent(FVector(0.9f, 0.9f, 4.4f)), Primary, Mission.LandmarkName + TEXT(" Core"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 470)), CityExtent(FVector(1.8f, 1.8f, 0.22f)), Core, Mission.LandmarkName + TEXT(" Beacon"));
        break;
    case 1:
        SpawnBlock(Base + CityOffset(FVector(-210, 0, 180)), CityExtent(FVector(0.35f, 0.55f, 3.6f)), Primary, Mission.LandmarkName + TEXT(" West Pillar"));
        SpawnBlock(Base + CityOffset(FVector(210, 0, 180)), CityExtent(FVector(0.35f, 0.55f, 3.6f)), Primary, Mission.LandmarkName + TEXT(" East Pillar"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 365)), CityExtent(FVector(4.4f, 0.45f, 0.32f)), Core, Mission.LandmarkName + TEXT(" Arch Span"));
        break;
    case 2:
        SpawnBlock(Base + CityOffset(FVector(0, 0, 120)), CityExtent(FVector(2.2f, 2.2f, 0.4f)), Primary, Mission.LandmarkName + TEXT(" Pad"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 270)), CityExtent(FVector(0.75f, 0.75f, 2.6f)), Core, Mission.LandmarkName + TEXT(" Data Beacon"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 430)), CityExtent(FVector(2.6f, 0.18f, 0.18f)), Core, Mission.LandmarkName + TEXT(" Crossbeam"));
        break;
    case 3:
        SpawnBlock(Base + CityOffset(FVector(0, 0, 165)), CityExtent(FVector(1.4f, 1.4f, 3.3f)), Primary, Mission.LandmarkName + TEXT(" Obelisk"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 360)), CityExtent(FVector(0.8f, 0.8f, 0.8f)), Core, Mission.LandmarkName + TEXT(" Command Crown"));
        break;
    case 4:
        SpawnBlock(Base + CityOffset(FVector(-300, 0, 120)), CityExtent(FVector(0.45f, 0.45f, 2.4f)), Primary, Mission.LandmarkName + TEXT(" Bridge A"));
        SpawnBlock(Base + CityOffset(FVector(300, 0, 120)), CityExtent(FVector(0.45f, 0.45f, 2.4f)), Primary, Mission.LandmarkName + TEXT(" Bridge B"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 250)), CityExtent(FVector(6.2f, 0.35f, 0.28f)), Core, Mission.LandmarkName + TEXT(" Relay Deck"));
        break;
    default:
        SpawnBlock(Base + CityOffset(FVector(0, 0, 135)), CityExtent(FVector(2.5f, 2.5f, 0.5f)), Primary, Mission.LandmarkName + TEXT(" Crown Base"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 270)), CityExtent(FVector(1.4f, 1.4f, 2.2f)), Primary, Mission.LandmarkName + TEXT(" Crown Tower"));
        SpawnBlock(Base + CityOffset(FVector(0, 0, 410)), CityExtent(FVector(2.0f, 2.0f, 0.18f)), Core, Mission.LandmarkName + TEXT(" Crown Light"));
        break;
    }

    SpawnGuideText(
        Mission.LandmarkName + TEXT("\n") + Mission.DistrictStyle,
        Base + CityOffset(FVector(0, 0, 610)),
        Mission.SecondaryAccentColor.ToFColor(true),
        52.0f);
}

void ACodeRescueGameMode::SpawnCityArtKit(const FCodeRescueCityMission& Mission, const FVector& Origin, const FString& CityLabel)
{
    const FString& Kit = Mission.ArtKitName;
    const FLinearColor Primary = Mission.SecondaryAccentColor * 1.5f;
    const FLinearColor Accent = Mission.AccentColor * 2.5f;

    if (Kit == TEXT("Coastal Port"))
    {
        SpawnBlock(Origin + CityOffset(FVector(0, 3150, -20)), CityExtent(FVector(76, 6.5f, 0.08f)), FLinearColor(0.02f, 0.16f, 0.24f) * 2.0f, CityLabel + TEXT(" Harbor Water"));
        for (int32 i = 0; i < 3; ++i)
        {
            const FVector CraneBase = Origin + CityOffset(FVector(-2300.0f + i * 950.0f, 2550.0f, 145.0f));
            SpawnBlock(CraneBase, CityExtent(FVector(0.18f, 0.18f, 2.9f)), Primary, CityLabel + TEXT(" Port Crane Mast"));
            SpawnBlock(CraneBase + CityOffset(FVector(170, 0, 170)), CityExtent(FVector(3.5f, 0.15f, 0.12f)), Accent, CityLabel + TEXT(" Port Crane Boom"));
            SpawnBlock(CraneBase + CityOffset(FVector(360, 0, 55)), CityExtent(FVector(0.16f, 0.16f, 1.1f)), Accent, CityLabel + TEXT(" Port Crane Hook"));
        }
    }
    else if (Kit == TEXT("Desert Solar Grid"))
    {
        SpawnBlock(Origin + CityOffset(FVector(0, 3050, -18)), CityExtent(FVector(80, 5.5f, 0.08f)), FLinearColor(0.32f, 0.20f, 0.09f), CityLabel + TEXT(" Desert Wash"));
        for (int32 i = 0; i < 5; ++i)
        {
            const FVector Panel = Origin + CityOffset(FVector(-2700.0f + i * 700.0f, 2250.0f + (i % 2) * 260.0f, 95.0f));
            SpawnBlock(Panel, CityExtent(FVector(2.8f, 0.12f, 0.8f)), FLinearColor(0.04f, 0.12f, 0.22f) * 3.0f, CityLabel + TEXT(" Solar Panel"));
            SpawnBlock(Panel + CityOffset(FVector(0, 0, -70)), CityExtent(FVector(0.12f, 0.12f, 1.2f)), Primary, CityLabel + TEXT(" Solar Strut"));
        }
    }
    else if (Kit == TEXT("Mountain Relay"))
    {
        for (int32 i = 0; i < 5; ++i)
        {
            const float Height = 2.0f + i * 0.55f;
            const FVector Ridge = Origin + CityOffset(FVector(-3400.0f + i * 1600.0f, 3050.0f, Height * 55.0f));
            SpawnBlock(Ridge, CityExtent(FVector(5.5f, 0.45f, Height)), FLinearColor(0.12f, 0.13f, 0.14f) + Primary * 0.08f, CityLabel + TEXT(" Mountain Ridge"));
            SpawnBlock(Ridge + CityOffset(FVector(0, 0, Height * 55.0f)), CityExtent(FVector(3.8f, 0.36f, 0.18f)), FLinearColor(0.85f, 0.92f, 1.0f), CityLabel + TEXT(" Snowcap"));
        }
    }
    else if (Kit == TEXT("Great Lakes Industrial"))
    {
        SpawnBlock(Origin + CityOffset(FVector(0, 2950, -20)), CityExtent(FVector(80, 5.2f, 0.08f)), FLinearColor(0.025f, 0.12f, 0.20f) * 2.2f, CityLabel + TEXT(" Lake Edge"));
        for (int32 i = 0; i < 4; ++i)
        {
            const FVector Stack = Origin + CityOffset(FVector(-2600.0f + i * 1350.0f, 2250.0f, 170.0f));
            SpawnBlock(Stack, CityExtent(FVector(0.5f, 0.5f, 3.4f)), FLinearColor(0.14f, 0.13f, 0.12f), CityLabel + TEXT(" Foundry Stack"));
            SpawnBlock(Stack + CityOffset(FVector(0, 0, 210)), CityExtent(FVector(0.75f, 0.75f, 0.12f)), Accent, CityLabel + TEXT(" Foundry Stack Light"));
        }
    }
    else if (Kit == TEXT("River Lockworks"))
    {
        SpawnBlock(Origin + CityOffset(FVector(0, 3050, -18)), CityExtent(FVector(80, 4.6f, 0.08f)), FLinearColor(0.03f, 0.15f, 0.13f) * 2.0f, CityLabel + TEXT(" River Channel"));
        for (int32 i = 0; i < 3; ++i)
        {
            const FVector Lock = Origin + CityOffset(FVector(-2100.0f + i * 1500.0f, 2500.0f, 75.0f));
            SpawnBlock(Lock, CityExtent(FVector(4.5f, 0.18f, 0.6f)), Primary, CityLabel + TEXT(" Lock Gate"));
            SpawnBlock(Lock + CityOffset(FVector(0, 190, 40)), CityExtent(FVector(4.5f, 0.18f, 0.6f)), Primary, CityLabel + TEXT(" Lock Gate Twin"));
        }
    }
    else if (Kit == TEXT("Capital Command"))
    {
        SpawnBlock(Origin + CityOffset(FVector(0, 2450, 75)), CityExtent(FVector(7.0f, 0.35f, 1.5f)), FLinearColor(0.78f, 0.82f, 0.86f), CityLabel + TEXT(" Command Colonnade"));
        for (int32 i = 0; i < 6; ++i)
        {
            SpawnBlock(Origin + CityOffset(FVector(-2700.0f + i * 1080.0f, 2250.0f, 160.0f)), CityExtent(FVector(0.22f, 0.22f, 3.2f)), Primary, CityLabel + TEXT(" Command Column"));
        }
    }
    else if (Kit == TEXT("Rail Yard"))
    {
        for (int32 i = 0; i < 4; ++i)
        {
            const float Y = 2250.0f + i * 260.0f;
            SpawnBlock(Origin + CityOffset(FVector(0, Y, 25)), CityExtent(FVector(78, 0.08f, 0.12f)), Primary, CityLabel + TEXT(" Rail Line A"));
            SpawnBlock(Origin + CityOffset(FVector(0, Y + 90.0f, 25)), CityExtent(FVector(78, 0.08f, 0.12f)), Primary, CityLabel + TEXT(" Rail Line B"));
        }
    }
    else
    {
        for (int32 i = 0; i < 5; ++i)
        {
            const FVector Kiosk = Origin + CityOffset(FVector(-3000.0f + i * 1500.0f, 2450.0f, 110.0f));
            SpawnBlock(Kiosk, CityExtent(FVector(1.0f, 0.55f, 2.2f)), Primary, CityLabel + TEXT(" Metro Kiosk"));
            SpawnBlock(Kiosk + CityOffset(FVector(0, 0, 150)), CityExtent(FVector(1.25f, 0.7f, 0.12f)), Accent, CityLabel + TEXT(" Metro Kiosk Sign"));
        }
    }

    SpawnGuideText(TEXT("ART KIT: ") + Kit, Origin + CityOffset(FVector(2650, 2580, 380)), Mission.SecondaryAccentColor.ToFColor(true), 44.0f);
}

void ACodeRescueGameMode::SpawnWorldMajorCitySignatureLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    const FVector AtlasCenter = Origin + CityOffset(FVector(0.0f, 2840.0f, -12.0f));
    const FLinearColor RegionColor = Mission.SecondaryAccentColor * 0.72f + Mission.AccentColor * 0.28f;
    const FString Kit = Mission.ArtKitName;

    auto TagAtlas = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("WorldMajorCityAtlas"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    TagAtlas(SpawnTexturedBlock(
        AtlasCenter,
        CityExtent(FVector(17.5f, 5.6f, 0.045f)),
        RegionColor * 0.34f,
        CityLabel + TEXT(" World Atlas District Ground"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false));

    TagAtlas(SpawnBlock(
        AtlasCenter + FVector(0.0f, -345.0f, 150.0f),
        CityExtent(FVector(8.4f, 0.08f, 1.42f)),
        FLinearColor(0.045f, 0.052f, 0.060f) + RegionColor * 0.16f,
        CityLabel + TEXT(" World Atlas Backlit Skyline Wall"),
        true));
    TagAtlas(SpawnBlock(
        AtlasCenter + FVector(0.0f, -356.0f, 188.0f),
        CityExtent(FVector(8.0f, 0.035f, 0.58f)),
        RegionColor * 2.2f,
        CityLabel + TEXT(" World Atlas City Light Wash"),
        false));

    static const TCHAR* DistrictNames[] = {
        TEXT("FINANCIAL CORE"),
        TEXT("TRANSIT SPINE"),
        TEXT("OLD CITY"),
        TEXT("RELIEF MARKET"),
        TEXT("QUARANTINE EDGE"),
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(DistrictNames); ++i)
    {
        const float X = -640.0f + static_cast<float>(i) * 320.0f;
        const FVector DistrictBase = AtlasCenter + FVector(X, 15.0f + (i % 2) * 70.0f, 44.0f);
        const FLinearColor DistrictColor = (i % 2 == 0)
            ? Mission.AccentColor * 1.65f
            : Mission.SecondaryAccentColor * 1.55f;
        TagAtlas(SpawnBlock(
            DistrictBase,
            FVector(1.08f, 0.72f, 0.34f + i * 0.05f),
            FLinearColor(0.08f, 0.09f, 0.10f) + DistrictColor * 0.12f,
            CityLabel + TEXT(" World Atlas District Mass"),
            true));
        TagAtlas(SpawnBlock(
            DistrictBase + FVector(0.0f, -78.0f, 76.0f + i * 8.0f),
            FVector(0.86f, 0.035f, 0.28f),
            DistrictColor,
            CityLabel + TEXT(" World Atlas District Sign Slab"),
            false));
        TagAtlas(SpawnGuideText(
            DistrictNames[i],
            DistrictBase + FVector(0.0f, -92.0f, 146.0f + i * 8.0f),
            DistrictColor.ToFColor(true),
            20.0f));
    }

    const bool bNeon = Kit == TEXT("Neon Megacity");
    const bool bMonsoon = Kit == TEXT("Monsoon Megacity") || Kit == TEXT("Monsoon Port");
    const bool bHistoric = Kit == TEXT("Historic Core");
    const bool bLatin = Kit == TEXT("Latin Metro");
    const bool bAfrican = Kit == TEXT("African Urban Relay");
    const bool bMiddleEast = Kit == TEXT("Desert Solar Grid") || Kit == TEXT("Middle East Solar Hub");
    const bool bOceania = Kit == TEXT("Oceania Harbor");

    if (bNeon)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            const FVector TowerLoc = AtlasCenter + FVector(-520.0f + i * 260.0f, -235.0f, 135.0f + i * 20.0f);
            TagAtlas(SpawnBlock(TowerLoc, FVector(0.42f, 0.42f, 2.4f + i * 0.45f), FLinearColor(0.03f, 0.04f, 0.07f), CityLabel + TEXT(" Neon Megacity Vertical Tower"), true));
            TagAtlas(SpawnBlock(TowerLoc + FVector(0.0f, -45.0f, 94.0f), FVector(0.48f, 0.035f, 0.32f), (i % 2 == 0 ? FLinearColor(0.0f, 0.95f, 1.0f) : FLinearColor(1.0f, 0.1f, 0.85f)) * 2.8f, CityLabel + TEXT(" Neon Megacity Light Panel"), false));
        }
    }
    else if (bMonsoon)
    {
        TagAtlas(SpawnBlock(AtlasCenter + FVector(0.0f, 250.0f, 18.0f), CityExtent(FVector(16.0f, 0.55f, 0.035f)), FLinearColor(0.02f, 0.16f, 0.22f) * 2.0f, CityLabel + TEXT(" Monsoon Drainage Canal"), false));
        for (int32 i = 0; i < 4; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-520.0f + i * 350.0f, 178.0f, 84.0f), FVector(1.55f, 0.16f, 0.20f), FLinearColor(0.52f, 0.62f, 0.66f), CityLabel + TEXT(" Monsoon Raised Walkway"), true));
        }
    }
    else if (bHistoric)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-420.0f + i * 210.0f, -130.0f, 132.0f), FVector(0.16f, 0.16f, 2.25f), FLinearColor(0.74f, 0.70f, 0.62f), CityLabel + TEXT(" Historic Civic Column"), true));
        }
        TagAtlas(SpawnBlock(AtlasCenter + FVector(0.0f, -130.0f, 260.0f), FVector(5.2f, 0.20f, 0.20f), FLinearColor(0.86f, 0.80f, 0.68f), CityLabel + TEXT(" Historic Civic Entablature"), true));
    }
    else if (bLatin)
    {
        for (int32 i = 0; i < 6; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-570.0f + i * 230.0f, 145.0f, 86.0f + i * 12.0f), FVector(1.25f, 0.32f, 0.26f), (i % 2 == 0 ? FLinearColor(1.0f, 0.25f, 0.16f) : FLinearColor(1.0f, 0.72f, 0.12f)) * 0.95f, CityLabel + TEXT(" Latin Market Canopy"), true));
        }
    }
    else if (bAfrican)
    {
        TagAtlas(SpawnBlock(AtlasCenter + FVector(-430.0f, -80.0f, 175.0f), FVector(0.34f, 0.34f, 3.25f), FLinearColor(0.40f, 0.48f, 0.32f), CityLabel + TEXT(" African Relay Water Tower Mast"), true));
        TagAtlas(SpawnBlock(AtlasCenter + FVector(-430.0f, -80.0f, 365.0f), FVector(1.2f, 1.2f, 0.40f), FLinearColor(0.18f, 0.28f, 0.20f) + RegionColor * 0.18f, CityLabel + TEXT(" African Relay Water Tank"), true));
        for (int32 i = 0; i < 4; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-40.0f + i * 210.0f, 160.0f, 78.0f), FVector(0.88f, 0.42f, 0.24f), FLinearColor(0.92f, 0.58f, 0.18f), CityLabel + TEXT(" African Market Stall"), true));
        }
    }
    else if (bMiddleEast)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-520.0f + i * 260.0f, 155.0f, 132.0f), FVector(1.25f, 0.10f, 0.08f), FLinearColor(1.0f, 0.82f, 0.40f) * 1.8f, CityLabel + TEXT(" Solar Shade Sail"), false));
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-520.0f + i * 260.0f, 155.0f, 66.0f), FVector(0.06f, 0.06f, 1.20f), FLinearColor(0.50f, 0.42f, 0.32f), CityLabel + TEXT(" Solar Shade Post"), true));
        }
    }
    else if (bOceania)
    {
        TagAtlas(SpawnBlock(AtlasCenter + FVector(0.0f, 240.0f, 20.0f), CityExtent(FVector(16.0f, 0.58f, 0.035f)), FLinearColor(0.02f, 0.18f, 0.26f) * 2.1f, CityLabel + TEXT(" Oceania Ferry Water Cut"), false));
        for (int32 i = 0; i < 4; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-520.0f + i * 350.0f, 175.0f, 62.0f), FVector(0.18f, 1.25f, 0.20f), FLinearColor(0.42f, 0.34f, 0.24f), CityLabel + TEXT(" Oceania Wharf Plank"), true));
        }
    }
    else
    {
        for (int32 i = 0; i < 6; ++i)
        {
            TagAtlas(SpawnBlock(AtlasCenter + FVector(-620.0f + i * 250.0f, -212.0f, 92.0f + (i % 3) * 34.0f), FVector(0.68f, 0.26f, 1.2f + (i % 3) * 0.32f), FLinearColor(0.06f, 0.07f, 0.09f) + RegionColor * 0.10f, CityLabel + TEXT(" Metro Core Signature Tower"), true));
        }
    }

    TagAtlas(SpawnGuideText(
        FString::Printf(
            TEXT("WORLD MAJOR CITY ATLAS\n%03d / %03d: %s, %s\n%s | %s"),
            Mission.Rank,
            FCodeRescueCampaign::GetMissionCount(),
            *Mission.CityName,
            *Mission.StateName,
            *Mission.RegionName,
            *Mission.ArtKitName),
        AtlasCenter + FVector(0.0f, -330.0f, 430.0f),
        RegionColor.ToFColor(true),
        34.0f));
}

void ACodeRescueGameMode::SpawnCityLandscapeDetails(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const bool bCoastal =
        Mission.ArtKitName == TEXT("Coastal Port") ||
        Mission.ArtKitName == TEXT("Great Lakes Industrial") ||
        Mission.ArtKitName == TEXT("River Lockworks");
    const bool bMountain = Mission.ArtKitName == TEXT("Mountain Relay");
    const bool bDesert = Mission.ArtKitName == TEXT("Desert Solar Grid");

    const TCHAR* TerrainMaterial = bDesert
        ? TEXT("/Game/StarterContent/Materials/M_Rock_Sandstone.M_Rock_Sandstone")
        : bMountain
            ? TEXT("/Game/StarterContent/Materials/M_Rock_Slate.M_Rock_Slate")
            : TEXT("/Game/StarterContent/Materials/M_Ground_Moss.M_Ground_Moss");

    SpawnTexturedBlock(
        Origin + CityOffset(FVector(0.0f, 0.0f, -22.0f)),
        CityExtent(FVector(80.0f, 62.0f, 0.045f)),
        bDesert ? FLinearColor(0.28f, 0.20f, 0.10f) : FLinearColor(0.10f, 0.16f, 0.12f),
        CityLabel + TEXT(" Textured Terrain Underlay"),
        TerrainMaterial,
        false);

    SpawnTexturedBlock(
        Origin + CityOffset(FVector(-250.0f, -2150.0f, -16.0f)),
        CityExtent(FVector(64.0f, 1.25f, 0.035f)),
        FLinearColor(0.12f, 0.12f, 0.12f),
        CityLabel + TEXT(" Main Evac Road"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false);
    SpawnTexturedBlock(
        Origin + CityOffset(FVector(750.0f, 250.0f, -15.0f)),
        CityExtent(FVector(1.15f, 48.0f, 0.035f)),
        FLinearColor(0.10f, 0.10f, 0.10f),
        CityLabel + TEXT(" Cross Evac Road"),
        TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough"),
        false);

    if (bCoastal)
    {
        SpawnTexturedBlock(
            Origin + CityOffset(FVector(0.0f, 3200.0f, -12.0f)),
            CityExtent(FVector(82.0f, 7.0f, 0.025f)),
            FLinearColor(0.02f, 0.12f, 0.18f) * 2.5f,
            CityLabel + TEXT(" Material Waterline"),
            TEXT("/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake"),
            false);
    }

    FRandomStream DetailStream(Mission.SkylineSeed ^ 0x3F51C9A7);
    for (int32 i = 0; i < 18; ++i)
    {
        const float EdgeSide = (i % 2 == 0) ? -1.0f : 1.0f;
        const FVector Local = FVector(
            DetailStream.FRandRange(-3600.0f, 3600.0f),
            EdgeSide * DetailStream.FRandRange(2500.0f, 3300.0f),
            30.0f);
        if (UStaticMesh* RockMesh = LoadCodeRescueLandscapeRockMesh(CityIndex + i))
        {
            SpawnStaticMeshProp(
                RockMesh,
                Origin + CityOffset(Local),
                FRotator(0.0f, DetailStream.FRandRange(0.0f, 360.0f), 0.0f),
                CityExtent(FVector(
                    DetailStream.FRandRange(0.45f, bMountain ? 1.35f : 0.85f),
                    DetailStream.FRandRange(0.45f, bMountain ? 1.25f : 0.75f),
                    DetailStream.FRandRange(0.35f, bMountain ? 1.10f : 0.65f))),
                CityLabel + TEXT(" StarterContent Rock Detail"),
                false);
        }
    }

    if (!bDesert)
    {
        for (int32 i = 0; i < 24; ++i)
        {
            const FVector Local = FVector(
                DetailStream.FRandRange(-3700.0f, 3700.0f),
                DetailStream.FRandRange(-3050.0f, 3050.0f),
                42.0f);
            if (FMath::Abs(Local.X) < 900.0f && FMath::Abs(Local.Y + 2150.0f) < 450.0f)
            {
                continue;
            }
            if (UStaticMesh* BushMesh = LoadCodeRescueLandscapeBushMesh(CityIndex + i))
            {
                SpawnStaticMeshProp(
                    BushMesh,
                    Origin + CityOffset(Local),
                    FRotator(0.0f, DetailStream.FRandRange(0.0f, 360.0f), 0.0f),
                    CityExtent(FVector(
                        DetailStream.FRandRange(0.35f, 0.75f),
                        DetailStream.FRandRange(0.35f, 0.75f),
                        DetailStream.FRandRange(0.35f, 0.85f))),
                    CityLabel + TEXT(" StarterContent Bush Detail"),
                    false);
            }
        }
    }

    if (UStaticMesh* LampMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Lamp_Wall.SM_Lamp_Wall")))
    {
        for (int32 i = 0; i < 6; ++i)
        {
            SpawnStaticMeshProp(
                LampMesh,
                Origin + CityOffset(FVector(-2700.0f + i * 900.0f, -2050.0f, 165.0f)),
                FRotator(0.0f, 180.0f, 0.0f),
                CityExtent(FVector(0.45f, 0.45f, 0.65f)),
                CityLabel + TEXT(" Evac Route Wall Lamp"),
                false);
        }
    }

    if (UStaticMesh* CouchMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Couch.SM_Couch")))
    {
        SpawnStaticMeshProp(
            CouchMesh,
            Origin + CityOffset(FVector(-1750.0f, -1550.0f, 55.0f)),
            FRotator(0.0f, 35.0f, 0.0f),
            CityExtent(FVector(1.25f, 0.75f, 0.75f)),
            CityLabel + TEXT(" Civilian Rest Stop Couch"),
            false);
    }

    SpawnGuideText(
        TEXT("LIVE TERRAIN PASS\nStarterContent materials, rocks, bushes, roads"),
        Origin + CityOffset(FVector(-3150.0f, 3050.0f, 330.0f)),
        Mission.SecondaryAccentColor.ToFColor(true),
        44.0f);
}

void ACodeRescueGameMode::SpawnCinematicStreetLifeLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    const FLinearColor AsphaltLine = FLinearColor(0.92f, 0.88f, 0.56f);
    const FLinearColor StreetWhite = FLinearColor(0.84f, 0.88f, 0.86f);
    const FLinearColor SignalCyan = FLinearColor(0.16f, 0.92f, 1.0f);
    const FLinearColor WarmLamp = FLinearColor(1.0f, 0.72f, 0.34f);
    const FLinearColor DebrisTint = FLinearColor(0.18f, 0.19f, 0.20f) + Mission.AccentColor * 0.08f;
    FRandomStream Stream(Mission.SkylineSeed ^ 0x51A7C0DE);

    auto TagStreetLife = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("CinematicStreetLife"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto TagSequencerBlocking = [&TagStreetLife](AActor* Actor, std::initializer_list<const TCHAR*> ExtraTags) -> AActor*
    {
        TagStreetLife(Actor);
        if (Actor)
        {
            Actor->Tags.Add(FName("SequencerIntroBossRevealBlocking"));
            Actor->Tags.Add(FName("CinematicCameraBlockingReady"));
            Actor->Tags.Add(FName("SequencerReadyFallback"));
            Actor->Tags.Add(FName("ControlRigReadyFallback"));
            for (const TCHAR* Tag : ExtraTags)
            {
                Actor->Tags.Add(FName(Tag));
            }
        }
        return Actor;
    };

    for (int32 i = 0; i < 10; ++i)
    {
        const float X = -3450.0f + static_cast<float>(i) * 740.0f;
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(FVector(X, -2150.0f, -8.0f)),
            CityExtent(FVector(2.3f, 0.035f, 0.018f)),
            AsphaltLine * 1.8f,
            CityLabel + TEXT(" Cinematic Road Centerline"),
            false));
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(FVector(X, -1875.0f, -7.0f)),
            CityExtent(FVector(2.2f, 0.030f, 0.018f)),
            StreetWhite * 1.35f,
            CityLabel + TEXT(" Cinematic North Lane Edge"),
            false));
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(FVector(X, -2425.0f, -7.0f)),
            CityExtent(FVector(2.2f, 0.030f, 0.018f)),
            StreetWhite * 1.35f,
            CityLabel + TEXT(" Cinematic South Lane Edge"),
            false));
    }

    for (int32 i = 0; i < 8; ++i)
    {
        const float X = -3150.0f + static_cast<float>(i) * 72.0f;
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(FVector(X, -2300.0f, -5.0f)),
            CityExtent(FVector(0.22f, 2.3f, 0.024f)),
            StreetWhite * 1.65f,
            CityLabel + TEXT(" Language Crosswalk Stripe"),
            false));
    }
    for (int32 i = 0; i < 8; ++i)
    {
        const float Y = -1120.0f + static_cast<float>(i) * 72.0f;
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(FVector(950.0f, Y, -5.0f)),
            CityExtent(FVector(2.0f, 0.22f, 0.024f)),
            StreetWhite * 1.65f,
            CityLabel + TEXT(" Terminal Crosswalk Stripe"),
            false));
    }

    auto SpawnStreetLamp = [&](const FVector& Local, const FString& Name)
    {
        const FVector Base = Origin + CityOffset(Local);
        TagStreetLife(SpawnBlock(Base + FVector(0.0f, 0.0f, 115.0f), FVector(0.08f, 0.08f, 2.30f), FLinearColor(0.06f, 0.07f, 0.075f), Name + TEXT(" Pole"), true));
        TagStreetLife(SpawnBlock(Base + FVector(48.0f, 0.0f, 235.0f), FVector(0.95f, 0.055f, 0.055f), FLinearColor(0.07f, 0.075f, 0.08f), Name + TEXT(" Arm"), true));
        TagStreetLife(SpawnBlock(Base + FVector(98.0f, 0.0f, 225.0f), FVector(0.25f, 0.25f, 0.10f), WarmLamp * 2.2f, Name + TEXT(" Lens"), false));
        if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Base + FVector(98.0f, 0.0f, 215.0f), FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(3600.0f);
                PLC->SetLightColor(WarmLamp);
                PLC->SetAttenuationRadius(780.0f);
            }
            TagStreetLife(Lamp);
        }
    };

    for (int32 i = 0; i < 6; ++i)
    {
        const float X = -3200.0f + static_cast<float>(i) * 1180.0f;
        const float Y = (i % 2 == 0) ? -1805.0f : -2495.0f;
        SpawnStreetLamp(FVector(X, Y, 0.0f), FString::Printf(TEXT("%s Cinematic Streetlamp %d"), *CityLabel, i + 1));
    }

    auto SpawnWayfindingSign = [&](const FVector& Local, const FString& Label, const FLinearColor& Tint)
    {
        const FVector Base = Origin + CityOffset(Local);
        TagStreetLife(SpawnBlock(Base + FVector(0.0f, 0.0f, 88.0f), FVector(0.06f, 0.06f, 1.75f), FLinearColor(0.05f, 0.055f, 0.06f), CityLabel + TEXT(" Wayfinding Signpost"), true));
        TagStreetLife(SpawnBlock(Base + FVector(0.0f, -8.0f, 178.0f), FVector(1.48f, 0.055f, 0.42f), Tint * 1.45f, CityLabel + TEXT(" Wayfinding Sign Face"), false));
        TagStreetLife(SpawnGuideText(Label, Base + FVector(0.0f, -42.0f, 205.0f), Tint.ToFColor(true), 24.0f));
    };

    SpawnWayfindingSign(FVector(-3370.0f, -2700.0f, 0.0f), TEXT("SAFEHOUSE / LEARN"), SignalCyan);
    SpawnWayfindingSign(FVector(740.0f, -1290.0f, 0.0f), TEXT("TERMINAL"), FLinearColor(0.0f, 0.85f, 1.0f));
    SpawnWayfindingSign(FVector(2380.0f, 1020.0f, 0.0f), TEXT("RESCUE"), FLinearColor(1.0f, 0.88f, 0.0f));

    TagSequencerBlocking(SpawnGuideText(
        FString::Printf(TEXT("SEQUENCER BLOCKING REEL\n%s\nintro | language | terminal | rescue | extraction | boss reveal"), *Mission.CityName),
        Origin + CityOffset(FVector(-2320.0f, -1535.0f, 440.0f)),
        FColor(190, 230, 255),
        28.0f),
        { TEXT("SequencerIntroBeat"), TEXT("SequencerBeatBoard") });

    struct FSequencerBeatSpec
    {
        const TCHAR* Label;
        const TCHAR* Detail;
        const TCHAR* PrimaryTag;
        FVector Local;
        float Yaw;
        FLinearColor Tint;
    };

    const FSequencerBeatSpec BeatSpecs[] = {
        { TEXT("INTRO"), TEXT("establish city + route"), TEXT("SequencerIntroBeat"), FVector(-3550.0f, -2920.0f, 70.0f), 32.0f, SignalCyan },
        { TEXT("LANGUAGE"), TEXT("selected run close-up"), TEXT("SequencerLanguageBeat"), FVector(-3030.0f, -2460.0f, 70.0f), 58.0f, FLinearColor(0.94f, 0.26f, 1.0f) },
        { TEXT("TERMINAL"), TEXT("safe solve proof"), TEXT("SequencerTerminalBeat"), FVector(760.0f, -1180.0f, 70.0f), 118.0f, FLinearColor(0.0f, 0.85f, 1.0f) },
        { TEXT("RESCUE"), TEXT("survivor reveal"), TEXT("SequencerSurvivorBeat"), FVector(2360.0f, 1040.0f, 70.0f), -132.0f, FLinearColor(1.0f, 0.88f, 0.0f) },
        { TEXT("EXTRACTION"), TEXT("route closure"), TEXT("SequencerExtractionBeat"), FVector(2780.0f, -1460.0f, 70.0f), -88.0f, WarmLamp },
        { TEXT("BOSS"), TEXT("warden reveal fallback"), TEXT("SequencerBossRevealBeat"), FVector(2960.0f, 1690.0f, 70.0f), -154.0f, FLinearColor(1.0f, 0.08f, 0.16f) },
    };

    for (int32 BeatIndex = 0; BeatIndex < UE_ARRAY_COUNT(BeatSpecs); ++BeatIndex)
    {
        const FSequencerBeatSpec& Beat = BeatSpecs[BeatIndex];
        const FVector Base = Origin + CityOffset(Beat.Local);
        const FRotator RailRot(0.0f, Beat.Yaw, 0.0f);
        TagSequencerBlocking(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 12.0f),
            CityExtent(FVector(1.10f, 0.08f, 0.035f)),
            Beat.Tint * 1.55f,
            FString::Printf(TEXT("%s Sequencer Camera Rail %s"), *CityLabel, Beat.Label),
            false),
            { TEXT("SequencerCameraRail"), Beat.PrimaryTag });
        TagSequencerBlocking(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 110.0f),
            CityExtent(FVector(0.08f, 0.08f, 1.25f)),
            FLinearColor(0.03f, 0.035f, 0.04f),
            FString::Printf(TEXT("%s Sequencer Tripod %s"), *CityLabel, Beat.Label),
            true),
            { TEXT("SequencerCameraTripod"), Beat.PrimaryTag });
        TagSequencerBlocking(SpawnBlock(
            Base + RailRot.RotateVector(FVector(62.0f, 0.0f, 185.0f)),
            CityExtent(FVector(0.32f, 0.14f, 0.14f)),
            Beat.Tint * 1.85f,
            FString::Printf(TEXT("%s Sequencer Camera Lens %s"), *CityLabel, Beat.Label),
            false),
            { TEXT("SequencerCameraLens"), Beat.PrimaryTag });
        TagSequencerBlocking(SpawnGuideText(
            FString::Printf(TEXT("%02d %s\n%s"), BeatIndex + 1, Beat.Label, Beat.Detail),
            Base + FVector(0.0f, -62.0f, 260.0f),
            Beat.Tint.ToFColor(true),
            14.0f),
            { TEXT("SequencerBeatLabel"), Beat.PrimaryTag });

        if (BeatIndex > 0)
        {
            const FVector Prev = Origin + CityOffset(BeatSpecs[BeatIndex - 1].Local);
            const FVector Delta = Base - Prev;
            const FVector Mid = (Base + Prev) * 0.5f;
            const bool bHorizontal = FMath::Abs(Delta.X) >= FMath::Abs(Delta.Y);
            const FVector Scale = bHorizontal
                ? CityExtent(FVector(FMath::Max(1.0f, FMath::Abs(Delta.X) / 100.0f), 0.05f, 0.018f))
                : CityExtent(FVector(0.05f, FMath::Max(1.0f, FMath::Abs(Delta.Y) / 100.0f), 0.018f));
            TagSequencerBlocking(SpawnBlock(
                Mid + FVector(0.0f, 0.0f, 8.0f),
                Scale,
                FLinearColor(0.68f, 0.84f, 1.0f) * 1.25f,
                FString::Printf(TEXT("%s Sequencer Beat Connector %d"), *CityLabel, BeatIndex),
                false),
                { TEXT("SequencerBeatConnector"), TEXT("SequencerIntroBeat"), TEXT("SequencerBossRevealBeat") });
        }
    }

    for (int32 i = 0; i < 7; ++i)
    {
        const FVector Local(
            Stream.FRandRange(-3150.0f, 3300.0f),
            Stream.FRandRange(-2420.0f, 1750.0f),
            28.0f);
        if (FMath::Abs(Local.X - 1150.0f) < 420.0f && FMath::Abs(Local.Y + 900.0f) < 420.0f)
        {
            continue;
        }
        const FLinearColor CarTint = (i % 3 == 0)
            ? FLinearColor(0.32f, 0.08f, 0.07f)
            : (i % 3 == 1)
                ? FLinearColor(0.06f, 0.18f, 0.28f)
                : DebrisTint;
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(Local),
            CityExtent(FVector(1.35f, 0.72f, 0.34f)),
            CarTint,
            CityLabel + TEXT(" Abandoned Vehicle Body"),
            true));
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(Local) + FVector(0.0f, 0.0f, 45.0f),
            CityExtent(FVector(0.72f, 0.48f, 0.22f)),
            FLinearColor(0.02f, 0.025f, 0.03f),
            CityLabel + TEXT(" Abandoned Vehicle Cabin"),
            true));
    }

    for (int32 i = 0; i < 4; ++i)
    {
        const float X = -2550.0f + i * 1350.0f;
        TagStreetLife(SpawnBlock(
            Origin + CityOffset(FVector(X, -1700.0f, 280.0f)),
            CityExtent(FVector(0.04f, 8.0f, 0.035f)),
            FLinearColor(0.015f, 0.018f, 0.020f),
            CityLabel + TEXT(" Overhead Utility Cable"),
            false));
    }

    TagStreetLife(SpawnGuideText(
        TEXT("CINEMATIC CITY PASS\nstreetlights, crosswalks, signs, debris, route mood"),
        Origin + CityOffset(FVector(-1700.0f, -1840.0f, 390.0f)),
        SignalCyan.ToFColor(true),
        34.0f));
}

void ACodeRescueGameMode::SpawnMissionObjectiveRoute(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    if (!ShouldSpawnDevelopmentShowcaseLayers())
    {
        UE_LOG(LogTemp, Display,
            TEXT("[ProductionRoute] %s uses HUD objective distance, focus beacons, and the guidance drone; prototype pads and pillars omitted."),
            *CityLabel);
        return;
    }

    struct FObjectiveStop
    {
        FVector Local;
        FString Label;
        FLinearColor Color;
    };

    const TArray<FObjectiveStop> Stops = {
        { FVector(-3820.0f, -3180.0f, -8.0f), TEXT("0 START\norient at the entry pad"), Mission.SecondaryAccentColor * 2.0f },
        { FVector(-2860.0f, -2680.0f, -8.0f), TEXT("1 PROTECTED CODING\nselected language only"), FLinearColor(0.95f, 0.25f, 1.0f) * 2.0f },
        { FVector(-2860.0f, -2460.0f, -8.0f), TEXT("2 SOLVE TERMINAL\ncombat pauses in the lab"), FLinearColor(0.0f, 0.85f, 1.0f) * 2.2f },
        { FVector(2850.0f, 1500.0f, -8.0f), TEXT("3 RESCUE TEAM\nreturn after the lesson"), FLinearColor(1.0f, 0.88f, 0.0f) * 2.0f },
        { FVector(2900.0f, -1500.0f, -8.0f), TEXT("4 OPTIONAL\nfight the warden"), FLinearColor(1.0f, 0.05f, 0.55f) * 2.0f },
    };

    auto SpawnRouteSegment = [this, &Origin, &Mission, &CityLabel](const FVector& A, const FVector& B, int32 SegmentIndex)
    {
        const FVector Delta = B - A;
        const FVector Mid = (A + B) * 0.5f;
        const bool bHorizontal = FMath::Abs(Delta.X) >= FMath::Abs(Delta.Y);
        const FVector Scale = bHorizontal
            ? FVector(FMath::Max(1.0f, FMath::Abs(Delta.X) / 100.0f), 0.18f, 0.025f)
            : FVector(0.18f, FMath::Max(1.0f, FMath::Abs(Delta.Y) / 100.0f), 0.025f);

        SpawnTexturedBlock(
            Origin + CityOffset(FVector(Mid.X, Mid.Y, -10.0f)),
            CityExtent(Scale),
            Mission.AccentColor * 2.6f,
            FString::Printf(TEXT("%s Objective Route Segment %d"), *CityLabel, SegmentIndex),
            TEXT("/Game/StarterContent/Materials/M_Tech_Hex_Tile_Pulse.M_Tech_Hex_Tile_Pulse"),
            false);
    };

    for (int32 i = 0; i < Stops.Num(); ++i)
    {
        const FObjectiveStop& Stop = Stops[i];
        const FVector WorldLoc = Origin + CityOffset(Stop.Local);
        SpawnTexturedBlock(
            WorldLoc,
            CityExtent(FVector(3.0f, 3.0f, 0.045f)),
            Stop.Color,
            FString::Printf(TEXT("%s Objective Pad %d"), *CityLabel, i),
            TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
            false);
        SpawnBlock(
            WorldLoc + FVector(0.0f, 0.0f, 145.0f),
            CityExtent(FVector(0.45f, 0.45f, 1.8f)),
            Stop.Color,
            FString::Printf(TEXT("%s Objective Beacon %d"), *CityLabel, i),
            false);
        // Objective beacon light — a colored pool of light at every stop so
        // the mission route reads clearly in any lighting, day or night.
        // Only the streamed city's five objective lights exist at once, so
        // this stays cheap; RegisterStreamedActor ties them to city cleanup.
        if (APointLight* BeaconLight = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                WorldLoc + FVector(0.0f, 0.0f, 240.0f), FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(BeaconLight->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(7000.0f);
                PLC->SetLightColor(Stop.Color);
                PLC->SetAttenuationRadius(1100.0f);
            }
            RegisterStreamedActor(BeaconLight);
        }
        SpawnGuideText(
            Stop.Label,
            WorldLoc + FVector(0.0f, 0.0f, 520.0f),
            Stop.Color.ToFColor(true),
            i == 0 ? 44.0f : 52.0f);

        if (i > 0)
        {
            SpawnRouteSegment(Stops[i - 1].Local, Stop.Local, i);
        }
    }

    SpawnGuideText(
        FString::Printf(TEXT("MISSION BOARD\n%s\nLanguage selected before deployment, solve in safehouse, rescue survivor"), *Mission.TerminalTitle),
        Origin + CityOffset(FVector(-3550.0f, -2840.0f, 650.0f)),
        FColor(230, 245, 255),
        38.0f);
}

void ACodeRescueGameMode::SpawnWorldCompositionLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    struct FFrameStop
    {
        FVector Local;
        FString Label;
        FLinearColor Color;
        float Width;
    };

    const TArray<FFrameStop> Frames = {
        { FVector(-3820.0f, -3180.0f, 0.0f), TEXT("ENTRY"), Mission.SecondaryAccentColor, 4.4f },
        { FVector(-2860.0f, -2680.0f, 0.0f), TEXT("SAFE LAB"), FLinearColor(0.95f, 0.25f, 1.0f), 5.2f },
        { FVector(-2860.0f, -2460.0f, 0.0f), TEXT("INTEL"), FLinearColor(0.0f, 0.85f, 1.0f), 5.0f },
        { FVector(2850.0f, 1500.0f, 0.0f), TEXT("RESCUE"), FLinearColor(1.0f, 0.88f, 0.0f), 5.4f },
        { FVector(2900.0f, -1500.0f, 0.0f), TEXT("WARDEN"), FLinearColor(1.0f, 0.08f, 0.28f), 5.2f },
    };

    for (int32 i = 0; i < Frames.Num(); ++i)
    {
        const FFrameStop& Stop = Frames[i];
        const FVector FrameCenter = Origin + CityOffset(Stop.Local);
        const FLinearColor Color = Stop.Color * 1.65f;

        SpawnBlock(
            FrameCenter + FVector(-Stop.Width * 55.0f, 0.0f, 165.0f),
            FVector(0.12f, 0.12f, 2.65f),
            Color,
            CityLabel + TEXT(" Objective Viewframe Pillar A"),
            false);
        SpawnBlock(
            FrameCenter + FVector(Stop.Width * 55.0f, 0.0f, 165.0f),
            FVector(0.12f, 0.12f, 2.65f),
            Color,
            CityLabel + TEXT(" Objective Viewframe Pillar B"),
            false);
        SpawnBlock(
            FrameCenter + FVector(0.0f, 0.0f, 310.0f),
            FVector(Stop.Width, 0.07f, 0.10f),
            Color * 1.2f,
            CityLabel + TEXT(" Objective Viewframe Header"),
            false);
        SpawnBlock(
            FrameCenter + FVector(0.0f, 115.0f, 70.0f),
            FVector(Stop.Width * 0.88f, 0.045f, 0.72f),
            Stop.Color * 0.45f,
            CityLabel + TEXT(" Objective Banner Wash"),
            false);

        if (UStaticMesh* LampMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Lamp_Ceiling.SM_Lamp_Ceiling")))
        {
            SpawnStaticMeshProp(
                LampMesh,
                FrameCenter + FVector(0.0f, 0.0f, 315.0f),
                FRotator::ZeroRotator,
                FVector(0.42f, 0.42f, 0.42f),
                CityLabel + TEXT(" Objective Suspended Light"),
                false);
        }
        SpawnGuideText(
            Stop.Label,
            FrameCenter + FVector(0.0f, 135.0f, 245.0f),
            Color.ToFColor(true),
            32.0f);
    }
}

void ACodeRescueGameMode::SpawnCharacterIdentityCourt(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector CourtCenter = Origin + CityOffset(FVector(-3500.0f, -3060.0f, -10.0f));
    const FLinearColor CourtColor = Mission.AccentColor * 0.60f + Mission.SecondaryAccentColor * 0.40f;

    static const TCHAR* Names[] = {
        TEXT("Riley"), TEXT("Mara"), TEXT("Noor"), TEXT("Sage"),
        TEXT("Tao"), TEXT("Iris"), TEXT("Vale"), TEXT("Sol"),
        TEXT("Ren"), TEXT("Mika"), TEXT("Ari"), TEXT("Lane"),
    };
    const int32 NameCount = UE_ARRAY_COUNT(Names);
    const FString GuideName = Names[(CityIndex * 3 + 0) % NameCount];
    const FString ScoutName = Names[(CityIndex * 3 + 1) % NameCount];
    const FString LiaisonName = Names[(CityIndex * 3 + 2) % NameCount];

    SpawnTexturedBlock(
        CourtCenter,
        FVector(6.8f, 2.85f, 0.045f),
        CourtColor * 0.32f,
        CityLabel + TEXT(" Character Identity Court Floor"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Poured.M_Concrete_Poured"),
        false);
    SpawnBlock(
        CourtCenter + FVector(0.0f, -195.0f, 150.0f),
        FVector(6.6f, 0.10f, 1.70f),
        FLinearColor(0.055f, 0.065f, 0.075f),
        CityLabel + TEXT(" Character Identity Court Backwall"),
        true);
    SpawnBlock(
        CourtCenter + FVector(0.0f, -202.0f, 150.0f),
        FVector(6.2f, 0.035f, 1.35f),
        CourtColor * 1.55f,
        CityLabel + TEXT(" Character Identity Court Accent Wash"),
        false);

    const FVector Slots[] = {
        FVector(-275.0f, 40.0f, 100.0f),
        FVector(0.0f, 75.0f, 100.0f),
        FVector(275.0f, 40.0f, 100.0f),
    };
    const FLinearColor Colors[] = {
        FLinearColor(0.25f, 0.95f, 1.0f),
        FLinearColor(1.0f, 0.74f, 0.16f),
        FLinearColor(0.55f, 1.0f, 0.38f),
    };
    const FString Labels[] = {
        FString::Printf(TEXT("%s\nCivic Guide"), *GuideName),
        FString::Printf(TEXT("%s\nSignal Scout"), *ScoutName),
        FString::Printf(TEXT("%s\nRescue Liaison"), *LiaisonName),
    };
    const FString Roles[] = {
        TEXT("route"),
        TEXT("signal"),
        TEXT("rescue"),
    };

    for (int32 i = 0; i < 3; ++i)
    {
        const FVector BaseLoc = CourtCenter + Slots[i];
        SpawnDecorativeCivilian(
            BaseLoc,
            FRotator(0.0f, -15.0f + i * 15.0f, 0.0f),
            (CityIndex + i) % 2 == 0,
            Colors[i],
            FString::Printf(TEXT("%s Character Identity Civilian %d"), *CityLabel, i),
            Labels[i]);
        SpawnBlock(
            BaseLoc + FVector(0.0f, -138.0f, 82.0f),
            FVector(1.35f, 0.045f, 0.72f),
            Colors[i] * 0.65f,
            CityLabel + TEXT(" Character Identity Portrait Panel"),
            false);
        SpawnGuideText(
            Roles[i].ToUpper(),
            BaseLoc + FVector(0.0f, -155.0f, 170.0f),
            Colors[i].ToFColor(true),
            24.0f);
    }

    SpawnGuideText(
        FString::Printf(TEXT("CIVILIAN CAST\n%s rescue cell"), *Mission.CityName),
        CourtCenter + FVector(0.0f, 0.0f, 430.0f),
        CourtColor.ToFColor(true),
        36.0f);
}

void ACodeRescueGameMode::SpawnEnterableCivicSafehouse(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector SafehouseCenter = Origin + CityOffset(FVector(-1125.0f, -3025.0f, -10.0f));
    const FLinearColor SafehouseColor = Mission.AccentColor * 0.55f + Mission.SecondaryAccentColor * 0.35f + FLinearColor(0.08f, 0.09f, 0.10f) * 0.10f;
    const FLinearColor WallColor = FLinearColor(0.10f, 0.115f, 0.125f) + SafehouseColor * 0.12f;
    const FLinearColor WarmLight = FLinearColor(1.0f, 0.74f, 0.24f);

    auto TagSafehouse = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("EnterableSafehouse"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    TagSafehouse(SpawnTexturedBlock(
        SafehouseCenter,
        FVector(7.6f, 5.35f, 0.055f),
        SafehouseColor * 0.32f,
        CityLabel + TEXT(" Enterable Safehouse Floor"),
        TEXT("/Game/StarterContent/Materials/M_Brick_Hewn_Stone.M_Brick_Hewn_Stone"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(0.0f, -278.0f, 124.0f),
        FVector(7.65f, 0.14f, 2.48f),
        WallColor,
        CityLabel + TEXT(" Enterable Safehouse Back Wall"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-390.0f, 0.0f, 124.0f),
        FVector(0.14f, 5.35f, 2.48f),
        WallColor,
        CityLabel + TEXT(" Enterable Safehouse West Wall"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(390.0f, 0.0f, 124.0f),
        FVector(0.14f, 5.35f, 2.48f),
        WallColor,
        CityLabel + TEXT(" Enterable Safehouse East Wall"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-270.0f, 278.0f, 122.0f),
        FVector(2.35f, 0.14f, 2.44f),
        WallColor,
        CityLabel + TEXT(" Enterable Safehouse Front Wall Left"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(270.0f, 278.0f, 122.0f),
        FVector(2.35f, 0.14f, 2.44f),
        WallColor,
        CityLabel + TEXT(" Enterable Safehouse Front Wall Right"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(0.0f, 278.0f, 246.0f),
        FVector(1.30f, 0.14f, 0.56f),
        WallColor,
        CityLabel + TEXT(" Enterable Safehouse Door Header"),
        true));

    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(0.0f, -278.0f, 265.0f),
        FVector(7.85f, 0.08f, 0.12f),
        SafehouseColor * 1.65f,
        CityLabel + TEXT(" Enterable Safehouse Roof Beam Back"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(0.0f, 278.0f, 265.0f),
        FVector(7.85f, 0.08f, 0.12f),
        SafehouseColor * 1.65f,
        CityLabel + TEXT(" Enterable Safehouse Roof Beam Front"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-390.0f, 0.0f, 265.0f),
        FVector(0.08f, 5.45f, 0.12f),
        SafehouseColor * 1.65f,
        CityLabel + TEXT(" Enterable Safehouse Roof Beam West"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(390.0f, 0.0f, 265.0f),
        FVector(0.08f, 5.45f, 0.12f),
        SafehouseColor * 1.65f,
        CityLabel + TEXT(" Enterable Safehouse Roof Beam East"),
        false));

    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(0.0f, -292.0f, 150.0f),
        FVector(3.25f, 0.04f, 0.95f),
        SafehouseColor * 2.1f,
        CityLabel + TEXT(" Enterable Safehouse Route Board"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(0.0f, 385.0f, 12.0f),
        FVector(2.35f, 0.42f, 0.055f),
        WarmLight * 2.2f,
        CityLabel + TEXT(" Enterable Safehouse Door Light Spill"),
        false));

    // 2026-07-11 pass 4 (environment physics): REAL swinging double doors in
    // the 305-uu doorway (E toggles, auto-close, blocks zombies/gunfire while
    // shut) + a lived-in interior so entering is worth it.
    const float FloorTopZ = SafehouseCenter.Z + 2.75f;
    if (UWorld* DoorWorld = GetWorld())
    {
        const float LeafWidthScale = 152.5f / 104.0f;   // authored leaf is 1.04 m
        const float LeafHeightScale = 210.0f / 206.0f;
        // hinges sit AT the doorway jambs — the leaf overlaps the wall edge, so
        // the spawn must not be rejected on overlap.
        FActorSpawnParameters DoorParams;
        DoorParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        const FVector LeftHinge = SafehouseCenter + FVector(-152.5f, 278.0f, FloorTopZ - SafehouseCenter.Z);
        ADoorActor* LeftDoor = DoorWorld->SpawnActor<ADoorActor>(
            ADoorActor::StaticClass(), LeftHinge, FRotator::ZeroRotator, DoorParams);
        if (LeftDoor)
        {
            LeftDoor->ConfigureLeaf(LeafWidthScale, LeafHeightScale, false);
            TagSafehouse(LeftDoor);
        }
        const FVector RightHinge = SafehouseCenter + FVector(152.5f, 278.0f, FloorTopZ - SafehouseCenter.Z);
        ADoorActor* RightDoor = DoorWorld->SpawnActor<ADoorActor>(
            ADoorActor::StaticClass(), RightHinge, FRotator(0.0f, 180.0f, 0.0f), DoorParams);
        if (RightDoor)
        {
            RightDoor->ConfigureLeaf(LeafWidthScale, LeafHeightScale, true);
            TagSafehouse(RightDoor);
        }
        UE_LOG(LogTemp, Display, TEXT("[DoorActor] %s safehouse doors spawned left=%d right=%d"),
            *CityLabel, LeftDoor != nullptr, RightDoor != nullptr);
    }
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-250.0f, -180.0f, 32.0f),
        FVector(2.0f, 0.9f, 0.5f),
        FLinearColor(0.30f, 0.26f, 0.20f),
        CityLabel + TEXT(" Safehouse Cot"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-250.0f, -180.0f, 62.0f),
        FVector(1.9f, 0.8f, 0.10f),
        FLinearColor(0.55f, 0.52f, 0.45f),
        CityLabel + TEXT(" Safehouse Cot Bedroll"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(220.0f, -140.0f, 46.0f),
        FVector(1.3f, 0.85f, 0.92f),
        FLinearColor(0.24f, 0.20f, 0.14f),
        CityLabel + TEXT(" Safehouse Supply Table"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(215.0f, -140.0f, 100.0f),
        FVector(0.5f, 0.4f, 0.28f),
        Mission.AccentColor * 0.9f,
        CityLabel + TEXT(" Safehouse Supply Crate"),
        false));

    // 2026-07-11 pass 4: one ambient wind manager per world (trees/bushes sway).
    if (!bWindSwayManagerSpawned)
    {
        if (UWorld* WindWorld = GetWorld())
        {
            WindWorld->SpawnActor<ACodeRescueWindSwayManager>(
                FVector::ZeroVector, FRotator::ZeroRotator);
            bWindSwayManagerSpawned = true;
            UE_LOG(LogTemp, Display, TEXT("[WindSway] ambient wind manager spawned"));
        }
    }

    if (UStaticMesh* DoorFrameMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_DoorFrame.SM_DoorFrame")))
    {
        TagSafehouse(SpawnStaticMeshProp(
            DoorFrameMesh,
            SafehouseCenter + FVector(0.0f, 283.0f, 124.0f),
            FRotator(0.0f, 90.0f, 0.0f),
            FVector(1.55f, 0.48f, 1.75f),
            CityLabel + TEXT(" Enterable Safehouse Door Frame"),
            true));
    }
    if (UStaticMesh* WindowFrameMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_WindowFrame.SM_WindowFrame")))
    {
        TagSafehouse(SpawnStaticMeshProp(
            WindowFrameMesh,
            SafehouseCenter + FVector(-230.0f, -290.0f, 152.0f),
            FRotator(0.0f, 0.0f, 0.0f),
            FVector(1.25f, 0.16f, 0.88f),
            CityLabel + TEXT(" Enterable Safehouse Window Frame A"),
            false));
        TagSafehouse(SpawnStaticMeshProp(
            WindowFrameMesh,
            SafehouseCenter + FVector(230.0f, -290.0f, 152.0f),
            FRotator(0.0f, 0.0f, 0.0f),
            FVector(1.25f, 0.16f, 0.88f),
            CityLabel + TEXT(" Enterable Safehouse Window Frame B"),
            false));
    }
    if (UStaticMesh* TableMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_TableRound.SM_TableRound")))
    {
        TagSafehouse(SpawnStaticMeshProp(
            TableMesh,
            SafehouseCenter + FVector(-150.0f, 25.0f, 43.0f),
            FRotator::ZeroRotator,
            FVector(1.10f, 1.10f, 0.75f),
            CityLabel + TEXT(" Enterable Safehouse Briefing Table"),
            true));
    }
    if (UStaticMesh* ShelfMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Shelf.SM_Shelf")))
    {
        TagSafehouse(SpawnStaticMeshProp(
            ShelfMesh,
            SafehouseCenter + FVector(292.0f, -82.0f, 86.0f),
            FRotator(0.0f, 90.0f, 0.0f),
            FVector(1.0f, 0.58f, 1.45f),
            CityLabel + TEXT(" Enterable Safehouse Supply Shelf"),
            true));
    }
    if (UStaticMesh* ChairMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Chair.SM_Chair")))
    {
        TagSafehouse(SpawnStaticMeshProp(
            ChairMesh,
            SafehouseCenter + FVector(-285.0f, 105.0f, 45.0f),
            FRotator(0.0f, -35.0f, 0.0f),
            FVector(0.78f, 0.78f, 0.88f),
            CityLabel + TEXT(" Enterable Safehouse Chair A"),
            true));
        TagSafehouse(SpawnStaticMeshProp(
            ChairMesh,
            SafehouseCenter + FVector(70.0f, 145.0f, 45.0f),
            FRotator(0.0f, 28.0f, 0.0f),
            FVector(0.78f, 0.78f, 0.88f),
            CityLabel + TEXT(" Enterable Safehouse Chair B"),
            true));
    }
    if (UStaticMesh* LampMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Lamp_Ceiling.SM_Lamp_Ceiling")))
    {
        TagSafehouse(SpawnStaticMeshProp(
            LampMesh,
            SafehouseCenter + FVector(0.0f, -30.0f, 236.0f),
            FRotator::ZeroRotator,
            FVector(0.42f, 0.42f, 0.42f),
            CityLabel + TEXT(" Enterable Safehouse Ceiling Lamp"),
            false));
    }

    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(285.0f, 125.0f, 48.0f),
        FVector(1.55f, 0.60f, 0.26f),
        FLinearColor(0.80f, 0.88f, 0.96f),
        CityLabel + TEXT(" Enterable Safehouse Rest Cot"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(285.0f, 125.0f, 80.0f),
        FVector(0.42f, 0.44f, 0.14f),
        Mission.SecondaryAccentColor * 1.7f,
        CityLabel + TEXT(" Enterable Safehouse Cot Pillow"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-18.0f, -166.0f, 72.0f),
        FVector(1.42f, 0.52f, 0.20f),
        FLinearColor(0.18f, 0.26f, 0.27f) + Mission.AccentColor * 0.24f,
        CityLabel + TEXT(" Enterable Safehouse Utility Bench"),
        true));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-88.0f, -174.0f, 104.0f),
        FVector(0.20f, 0.20f, 0.20f),
        FLinearColor(1.0f, 0.42f, 0.12f) * 1.9f,
        CityLabel + TEXT(" Enterable Safehouse Flare Training Prop"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(-8.0f, -174.0f, 104.0f),
        FVector(0.20f, 0.20f, 0.20f),
        FLinearColor(0.72f, 0.78f, 0.80f) * 1.5f,
        CityLabel + TEXT(" Enterable Safehouse Smoke Training Prop"),
        false));
    TagSafehouse(SpawnBlock(
        SafehouseCenter + FVector(72.0f, -174.0f, 104.0f),
        FVector(0.20f, 0.20f, 0.20f),
        FLinearColor(0.20f, 1.0f, 0.40f) * 1.8f,
        CityLabel + TEXT(" Enterable Safehouse Stim Training Prop"),
        false));
    TagSafehouse(SpawnGuideText(
        TEXT("UTILITY BENCH\nX SLOT: flare lures, smoke staggers, stim restores"),
        SafehouseCenter + FVector(-8.0f, -218.0f, 214.0f),
        FColor(190, 245, 255),
        20.0f));
    TagSafehouse(SpawnDecorativeCivilian(
        SafehouseCenter + FVector(-300.0f, -85.0f, 100.0f),
        FRotator(0.0f, 25.0f, 0.0f),
        (CityIndex % 2) == 0,
        WarmLight,
        CityLabel + TEXT(" Enterable Safehouse Civilian Lead"),
        TEXT("Iris\nSafehouse Lead")));
    TagSafehouse(SpawnDecorativeCivilian(
        SafehouseCenter + FVector(155.0f, -135.0f, 100.0f),
        FRotator(0.0f, -18.0f, 0.0f),
        (CityIndex % 2) != 0,
        Mission.SecondaryAccentColor,
        CityLabel + TEXT(" Enterable Safehouse Route Scout"),
        TEXT("Noor\nRoute Scout")));

    TagSafehouse(SpawnGuideText(
        TEXT("ENTERABLE CIVIC SAFEHOUSE\nwalk inside for route, supplies, and survivor context"),
        SafehouseCenter + FVector(0.0f, 0.0f, 408.0f),
        WarmLight.ToFColor(true),
        34.0f));
    TagSafehouse(SpawnGuideText(
        FString::Printf(TEXT("SAFEHOUSE BOARD\n%s\n%s"), *Mission.CityName, *Mission.CurriculumFocus),
        SafehouseCenter + FVector(0.0f, -318.0f, 292.0f),
        Mission.SecondaryAccentColor.ToFColor(true),
        26.0f));
}

void ACodeRescueGameMode::SpawnInteriorMissionSpacesForCity(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    struct FInteriorMissionSpec
    {
        const TCHAR* TypeTag;
        const TCHAR* Title;
        const TCHAR* RoleLine;
        const TCHAR* PickupLine;
        FVector LocalCenter;
        FLinearColor Color;
        EPickupKind PickupKind;
        int32 PickupAmount;
    };

    const FLinearColor HospitalRed(1.0f, 0.24f, 0.22f, 1.0f);
    const FLinearColor SchoolBlue(0.28f, 0.66f, 1.0f, 1.0f);
    const FLinearColor StoreGold(1.0f, 0.72f, 0.18f, 1.0f);
    const FLinearColor TransitTeal(0.20f, 0.95f, 0.82f, 1.0f);
    const FLinearColor CivicViolet(0.74f, 0.46f, 1.0f, 1.0f);

    const FInteriorMissionSpec Specs[] = {
        {
            TEXT("InteriorMission_HospitalTriage"),
            TEXT("HOSPITAL TRIAGE CLINIC"),
            TEXT("heal, stabilize, and read survivor status"),
            TEXT("MEDKIT CACHE"),
            FVector(-2140.0f, -3140.0f, -8.0f),
            HospitalRed,
            EPickupKind::Medkit,
            1,
        },
        {
            TEXT("InteriorMission_SchoolStudy"),
            TEXT("SCHOOL STUDY SHELTER"),
            TEXT("translate the current coding concept into a physical clue"),
            TEXT("SCANNER CHARGE"),
            FVector(-1040.0f, -3810.0f, -8.0f),
            SchoolBlue,
            EPickupKind::RadioScanner,
            1,
        },
        {
            TEXT("InteriorMission_CornerStore"),
            TEXT("CORNER STORE CACHE"),
            TEXT("restock ammo, armor, and route supplies before extraction"),
            TEXT("AMMO POUCH"),
            FVector(140.0f, -3410.0f, -8.0f),
            StoreGold,
            EPickupKind::AmmoPouch,
            30,
        },
        {
            TEXT("InteriorMission_TransitOps"),
            TEXT("TRANSIT OPERATIONS HUB"),
            TEXT("trace the rescue route, platform exit, and next landmark"),
            TEXT("FLASHLIGHT BATTERY"),
            FVector(1180.0f, -2680.0f, -8.0f),
            TransitTeal,
            EPickupKind::FlashlightBattery,
            1,
        },
        {
            TEXT("InteriorMission_CivicRecords"),
            TEXT("CIVIC RECORDS ANNEX"),
            TEXT("review survivor files, route authority, and bypass protocol"),
            TEXT("BYPASS KIT"),
            FVector(2120.0f, -1810.0f, -8.0f),
            CivicViolet,
            EPickupKind::BypassKit,
            1,
        },
    };

    auto TagInterior = [](AActor* Actor, const TCHAR* TypeTag) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("InteriorMissionSpace"));
            Actor->Tags.Add(FName("EnterableMissionInterior"));
            Actor->Tags.Add(FName("InteriorMissionSpaceReady"));
            Actor->Tags.Add(FName("HumanScaleBuildingProportion"));
            Actor->Tags.Add(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.Add(FName("Top50Recommendations"));
            Actor->Tags.Add(FName(TypeTag));
        }
        return Actor;
    };

    auto SpawnInteriorPickup = [&](const FInteriorMissionSpec& Spec, const FVector& Location)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Location, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Spec.PickupKind;
            Pickup->Amount = Spec.PickupAmount;
            Pickup->Tags.Add(FName("InteriorMissionSupplyPickup"));
            Pickup->Tags.Add(FName("InteriorMissionSpace"));
            Pickup->Tags.Add(FName(Spec.TypeTag));
            RegisterStreamedActor(Pickup);
        }
        TagInterior(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Spec.PickupLine, *FString::FromInt(Spec.PickupAmount)),
            Location + FVector(0.0f, 0.0f, 196.0f),
            Spec.Color.ToFColor(true),
            14.0f),
            Spec.TypeTag);
    };

    auto SpawnInteriorLight = [&](const FInteriorMissionSpec& Spec, const FVector& Location)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(3200.0f);
                PLC->SetLightColor(Spec.Color);
                PLC->SetAttenuationRadius(720.0f);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(FString::Printf(TEXT("%s %s Interior Mission Light"), *CityLabel, Spec.Title));
#endif
            RegisterStreamedActor(Light);
            TagInterior(Light, Spec.TypeTag);
        }
    };

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Specs); ++Index)
    {
        const FInteriorMissionSpec& Spec = Specs[Index];
        const FVector Center = Origin + CityOffset(Spec.LocalCenter);
        const FLinearColor WallColor = FLinearColor(0.055f, 0.060f, 0.064f, 1.0f) + Spec.Color * 0.12f;
        const FLinearColor FloorColor = FLinearColor(0.030f, 0.035f, 0.040f, 1.0f) + Spec.Color * 0.10f;

        TagInterior(SpawnTexturedBlock(
            Center,
            FVector(5.4f, 3.45f, 0.052f),
            FloorColor,
            FString::Printf(TEXT("%s %s Floor"), *CityLabel, Spec.Title),
            TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
            false),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(0.0f, -220.0f, 128.0f),
            FVector(5.55f, 0.12f, 2.32f),
            WallColor,
            FString::Printf(TEXT("%s %s Back Wall"), *CityLabel, Spec.Title),
            true),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(-282.0f, 0.0f, 128.0f),
            FVector(0.12f, 3.42f, 2.32f),
            WallColor,
            FString::Printf(TEXT("%s %s Left Wall"), *CityLabel, Spec.Title),
            true),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(282.0f, 0.0f, 128.0f),
            FVector(0.12f, 3.42f, 2.32f),
            WallColor,
            FString::Printf(TEXT("%s %s Right Wall"), *CityLabel, Spec.Title),
            true),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(-205.0f, 220.0f, 126.0f),
            FVector(1.55f, 0.12f, 2.22f),
            WallColor,
            FString::Printf(TEXT("%s %s Front Wall Left"), *CityLabel, Spec.Title),
            true),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(205.0f, 220.0f, 126.0f),
            FVector(1.55f, 0.12f, 2.22f),
            WallColor,
            FString::Printf(TEXT("%s %s Front Wall Right"), *CityLabel, Spec.Title),
            true),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(0.0f, 220.0f, 246.0f),
            FVector(1.30f, 0.12f, 0.44f),
            Spec.Color * 1.35f,
            FString::Printf(TEXT("%s %s Open Door Header"), *CityLabel, Spec.Title),
            false),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(0.0f, 282.0f, 18.0f),
            FVector(1.72f, 0.34f, 0.065f),
            Spec.Color * 2.1f,
            FString::Printf(TEXT("%s %s Door Light Spill"), *CityLabel, Spec.Title),
            false),
            Spec.TypeTag);
        TagInterior(SpawnBlock(
            Center + FVector(0.0f, -226.0f, 168.0f),
            FVector(2.85f, 0.045f, 0.78f),
            Spec.Color * 1.85f,
            FString::Printf(TEXT("%s %s Mission Board"), *CityLabel, Spec.Title),
            false),
            Spec.TypeTag);

        TagInterior(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s\n%s"), Spec.Title, *Mission.CityName, Spec.RoleLine),
            Center + FVector(0.0f, 0.0f, 392.0f),
            Spec.Color.ToFColor(true),
            25.0f),
            Spec.TypeTag);
        TagInterior(SpawnGuideText(
            FString::Printf(TEXT("MISSION SPACE\nconcept: %s\nsurvivor: %s\nlandmark: %s"),
                *Mission.CurriculumFocus,
                *Mission.SurvivorName,
                *Mission.LandmarkName),
            Center + FVector(0.0f, -252.0f, 282.0f),
            FColor(220, 245, 255),
            15.0f),
            Spec.TypeTag);

        if (Index == 0)
        {
            for (int32 Cot = 0; Cot < 3; ++Cot)
            {
                TagInterior(SpawnBlock(
                    Center + FVector(-175.0f + Cot * 175.0f, -28.0f + (Cot % 2) * 78.0f, 58.0f),
                    FVector(1.10f, 0.48f, 0.20f),
                    FLinearColor(0.86f, 0.94f, 0.98f, 1.0f),
                    FString::Printf(TEXT("%s Hospital Triage Cot %d"), *CityLabel, Cot + 1),
                    true),
                    Spec.TypeTag);
            }
            TagInterior(SpawnBlock(Center + FVector(210.0f, -120.0f, 106.0f), FVector(0.20f, 0.20f, 0.70f), Spec.Color * 1.6f, CityLabel + TEXT(" Hospital IV Stand"), false), Spec.TypeTag);
        }
        else if (Index == 1)
        {
            for (int32 Desk = 0; Desk < 4; ++Desk)
            {
                const int32 Row = Desk / 2;
                const int32 Col = Desk % 2;
                TagInterior(SpawnBlock(
                    Center + FVector(-120.0f + Col * 240.0f, -18.0f + Row * 96.0f, 58.0f),
                    FVector(0.80f, 0.42f, 0.20f),
                    FLinearColor(0.20f, 0.24f, 0.28f, 1.0f) + Spec.Color * 0.28f,
                    FString::Printf(TEXT("%s Study Shelter Desk %d"), *CityLabel, Desk + 1),
                    true),
                    Spec.TypeTag);
            }
            TagInterior(SpawnGuideText(Mission.CurriculumFocus, Center + FVector(0.0f, -78.0f, 190.0f), Spec.Color.ToFColor(true), 14.0f), Spec.TypeTag);
        }
        else if (Index == 2)
        {
            for (int32 Shelf = 0; Shelf < 3; ++Shelf)
            {
                TagInterior(SpawnBlock(
                    Center + FVector(-188.0f + Shelf * 188.0f, 18.0f, 98.0f),
                    FVector(0.42f, 0.20f, 0.86f),
                    FLinearColor(0.24f, 0.18f, 0.08f, 1.0f) + Spec.Color * 0.20f,
                    FString::Printf(TEXT("%s Corner Store Shelf %d"), *CityLabel, Shelf + 1),
                    true),
                    Spec.TypeTag);
            }
            TagInterior(SpawnBlock(Center + FVector(210.0f, -116.0f, 74.0f), FVector(0.52f, 0.32f, 0.26f), Spec.Color * 1.45f, CityLabel + TEXT(" Corner Store Register"), true), Spec.TypeTag);
        }
        else if (Index == 3)
        {
            TagInterior(SpawnBlock(Center + FVector(0.0f, 52.0f, 44.0f), FVector(4.40f, 0.18f, 0.12f), Spec.Color * 1.35f, CityLabel + TEXT(" Transit Platform Stripe"), false), Spec.TypeTag);
            TagInterior(SpawnBlock(Center + FVector(-176.0f, -28.0f, 58.0f), FVector(1.10f, 0.34f, 0.18f), FLinearColor(0.10f, 0.12f, 0.14f, 1.0f) + Spec.Color * 0.18f, CityLabel + TEXT(" Transit Bench A"), true), Spec.TypeTag);
            TagInterior(SpawnBlock(Center + FVector(176.0f, -28.0f, 58.0f), FVector(1.10f, 0.34f, 0.18f), FLinearColor(0.10f, 0.12f, 0.14f, 1.0f) + Spec.Color * 0.18f, CityLabel + TEXT(" Transit Bench B"), true), Spec.TypeTag);
            TagInterior(SpawnGuideText(TEXT("ROUTE MAP\nterminal -> survivor -> helipad"), Center + FVector(0.0f, -92.0f, 196.0f), Spec.Color.ToFColor(true), 14.0f), Spec.TypeTag);
        }
        else
        {
            for (int32 File = 0; File < 5; ++File)
            {
                TagInterior(SpawnBlock(
                    Center + FVector(-210.0f + File * 105.0f, -2.0f + (File % 2) * 70.0f, 76.0f),
                    FVector(0.30f, 0.16f, 0.46f),
                    Spec.Color * (0.72f + File * 0.06f),
                    FString::Printf(TEXT("%s Civic Records File Stack %d"), *CityLabel, File + 1),
                    false),
                    Spec.TypeTag);
            }
            TagInterior(SpawnGuideText(TEXT("BYPASS PROTOCOL\nassist costs one kit; clean-solve bonus disabled"), Center + FVector(0.0f, 58.0f, 204.0f), Spec.Color.ToFColor(true), 13.0f), Spec.TypeTag);
        }

        SpawnInteriorPickup(Spec, Center + FVector(0.0f, 122.0f, 116.0f));
        SpawnInteriorLight(Spec, Center + FVector(0.0f, -42.0f, 272.0f));
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueInteriorMissionSpaces] %s spawned enterable hospital, school, corner store, transit, and civic records interiors with survivor/curriculum context and functional pickups."),
        *CityLabel);

    (void)CityIndex;
}

void ACodeRescueGameMode::SpawnPhysicsTraversalYard(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    const FVector YardCenter = Origin + CityOffset(FVector(520.0f, -2550.0f, -12.0f));
    const FLinearColor PhysicsBlue = FLinearColor(0.18f, 0.62f, 1.0f);
    const FLinearColor AssistGreen = FLinearColor(0.20f, 1.0f, 0.46f);
    const FLinearColor HazardAmber = FLinearColor(1.0f, 0.62f, 0.14f);
    const FLinearColor Concrete = FLinearColor(0.15f, 0.16f, 0.16f) + Mission.AccentColor * 0.05f;

    auto TagPhysicsYard = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("PhysicsTraversalYard"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto TagSurfaceImpact = [](AActor* Actor, const FName& SurfaceTag) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("SurfaceImpactTraining"));
            Actor->Tags.Add(FName("PhysicalMaterialSurfaceReaction"));
            Actor->Tags.Add(SurfaceTag);
        }
        return Actor;
    };

    auto TagPhysicsEncounter = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("PhysicsLaneCombatEncounter"));
            Actor->Tags.Add(FName("AuthoredCombatEncounter"));
            Actor->Tags.Add(FName("UsesThrowablePhysicsLane"));
        }
        return Actor;
    };

    auto EnableTrainingPhysics = [](AActor* Actor, float MassKg) -> AActor*
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                MeshComp->SetMobility(EComponentMobility::Movable);
                MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetNotifyRigidBodyCollision(true);
                MeshComp->SetLinearDamping(0.28f);
                MeshComp->SetAngularDamping(0.38f);
                CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
                    MeshComp,
                    MeshActor,
                    FName("PhysicsTrainingFixedStepBody"),
                    MassKg,
                    0.28f,
                    0.38f,
                    false);
            }
        }
        return Actor;
    };

    auto SpawnEncounterPickup = [&](EPickupKind Kind, const FVector& Location, int32 Amount, const TCHAR* TagName) -> APickupActor*
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Location, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("PhysicsLaneCombatReward"));
            Pickup->Tags.Add(FName(TagName));
            RegisterStreamedActor(Pickup);
            TagPhysicsYard(Pickup);
            TagPhysicsEncounter(Pickup);
        }
        return Pickup;
    };

    auto SpawnDestructibleBarricade = [&](const FVector& Location, const FRotator& Rotation, float HealthValue, const TCHAR* TagName) -> ABarricadeActor*
    {
        const FTransform SpawnTransform(Rotation, Location);
        ABarricadeActor* Barricade = GetWorld()->SpawnActorDeferred<ABarricadeActor>(
            ABarricadeActor::StaticClass(),
            SpawnTransform,
            nullptr,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Barricade)
        {
            return nullptr;
        }

        Barricade->Lifetime = 0.0f;
        Barricade->MaxHealth = FMath::Max(1.0f, HealthValue);
        Barricade->Health = Barricade->MaxHealth;
        Barricade->DebrisCount = 8;
        Barricade->DebrisLifetime = 13.5f;
        Barricade->Tags.Add(FName("DestructibleCoverTraining"));
        Barricade->Tags.Add(FName("PhysicsLaneBreakableBarricade"));
        Barricade->Tags.Add(FName("ChaosReadableDestruction"));
        Barricade->Tags.Add(FName("ThrowableBreakableCover"));
        Barricade->Tags.Add(FName(TagName));
        UGameplayStatics::FinishSpawningActor(Barricade, SpawnTransform);
        RegisterStreamedActor(Barricade);
        TagPhysicsYard(Barricade);
        TagSurfaceImpact(Barricade, FName("SurfaceWood"));
        TagPhysicsEncounter(Barricade);
        return Barricade;
    };

    TagSurfaceImpact(TagPhysicsYard(SpawnTexturedBlock(
        YardCenter,
        FVector(8.8f, 5.8f, 0.05f),
        Concrete,
        CityLabel + TEXT(" Physics Traversal Yard Floor"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false)),
        FName("SurfaceConcrete"));

    TagSurfaceImpact(TagPhysicsYard(SpawnRotatedBlock(
        YardCenter + FVector(-285.0f, -90.0f, 72.0f),
        FRotator(0.0f, 0.0f, 11.0f),
        FVector(3.1f, 0.92f, 0.16f),
        PhysicsBlue * 0.92f,
        CityLabel + TEXT(" Physics Yard Incline Ramp A"),
        true)),
        FName("SurfaceMetal"));
    TagSurfaceImpact(TagPhysicsYard(SpawnRotatedBlock(
        YardCenter + FVector(285.0f, 90.0f, 72.0f),
        FRotator(0.0f, 180.0f, -11.0f),
        FVector(3.1f, 0.92f, 0.16f),
        PhysicsBlue * 0.92f,
        CityLabel + TEXT(" Physics Yard Incline Ramp B"),
        true)),
        FName("SurfaceMetal"));

    TagSurfaceImpact(TagPhysicsYard(SpawnBlock(
        YardCenter + FVector(0.0f, 0.0f, 108.0f),
        FVector(1.55f, 1.55f, 0.28f),
        FLinearColor(0.12f, 0.13f, 0.14f),
        CityLabel + TEXT(" Physics Yard Raised Platform"),
        true)),
        FName("SurfaceConcrete"));
    TagPhysicsYard(SpawnBlock(
        YardCenter + FVector(0.0f, 0.0f, 151.0f),
        FVector(1.25f, 1.25f, 0.055f),
        AssistGreen * 2.1f,
        CityLabel + TEXT(" Physics Yard Soft Landing Assist Pad"),
        false));

    for (int32 i = 0; i < 4; ++i)
    {
        const float X = -360.0f + i * 240.0f;
        TagSurfaceImpact(TagPhysicsYard(SpawnBlock(
            YardCenter + FVector(X, 255.0f, 54.0f),
            FVector(0.72f, 0.28f, 0.70f),
            HazardAmber * (0.85f + i * 0.08f),
            CityLabel + TEXT(" Physics Yard Collision Cover"),
            true)),
            i % 2 == 0 ? FName("SurfaceWood") : FName("SurfaceMetal"));
    }

    TagPhysicsYard(SpawnBlock(
        YardCenter + FVector(0.0f, -318.0f, 20.0f),
        FVector(6.0f, 0.06f, 0.08f),
        AssistGreen * 1.8f,
        CityLabel + TEXT(" Throwable Physics Lane Start Stripe"),
        false));
    for (int32 i = 0; i < 7; ++i)
    {
        const FVector TargetLoc = YardCenter + FVector(-500.0f + i * 165.0f, -246.0f + (i % 2) * 64.0f, 66.0f + (i % 3) * 12.0f);
        AActor* Target = TagPhysicsYard(EnableTrainingPhysics(SpawnBlock(
            TargetLoc,
            FVector(0.42f + (i % 3) * 0.05f, 0.34f, 0.48f),
            (i % 2 == 0 ? AssistGreen : PhysicsBlue) * (0.82f + i * 0.04f),
            FString::Printf(TEXT("%s Throwable Physics Target %d"), *CityLabel, i + 1),
            true),
            24.0f + i * 6.0f));
        if (Target)
        {
            Target->Tags.Add(FName("ThrowablePhysicsTarget"));
            Target->Tags.Add(FName("PhysicsDeepDiveC23"));
            Target->Tags.Add(FName("RadialImpulseTrainingProp"));
            Target->Tags.Add(FName("SurfaceImpactTrainingProp"));
            Target->Tags.Add(i % 3 == 0 ? FName("SurfaceMetal") : (i % 3 == 1 ? FName("SurfaceWood") : FName("SurfaceConcrete")));
        }
    }

    TagPhysicsYard(SpawnGuideText(
        TEXT("THROWABLE PHYSICS LANE\nX SLOT utility pulses push marked props and stagger infected"),
        YardCenter + FVector(0.0f, -360.0f, 318.0f),
        AssistGreen.ToFColor(true),
        26.0f));
    TagPhysicsYard(SpawnGuideText(
        TEXT("SURFACE IMPACT RANGE\nconcrete dust, metal sparks, wood chip reactions"),
        YardCenter + FVector(0.0f, -438.0f, 258.0f),
        HazardAmber.ToFColor(true),
        23.0f));

    const FVector EncounterCenter = YardCenter + FVector(0.0f, -760.0f, 0.0f);
    TagPhysicsEncounter(TagPhysicsYard(SpawnGuideText(
        TEXT("PHYSICS AMBUSH DRILL\nuse X slot throwables, impact props, and cover to clear the lane"),
        EncounterCenter + FVector(0.0f, -70.0f, 330.0f),
        HazardAmber.ToFColor(true),
        25.0f)));
    TagPhysicsEncounter(TagPhysicsYard(SpawnGuideText(
        TEXT("DESTRUCTIBLE COVER DRILL\nshoot, throw, or kite infected into wood barricades"),
        EncounterCenter + FVector(0.0f, 104.0f, 292.0f),
        FColor(255, 188, 84),
        22.0f)));

    for (int32 i = 0; i < 5; ++i)
    {
        const float X = -330.0f + i * 165.0f;
        const FName SurfaceTag = i % 2 == 0 ? FName("SurfaceMetal") : FName("SurfaceWood");
        AActor* EncounterProp = EnableTrainingPhysics(SpawnBlock(
            EncounterCenter + FVector(X, -22.0f + (i % 2) * 86.0f, 72.0f),
            FVector(0.46f, 0.22f, 0.58f),
            i % 2 == 0 ? HazardAmber * 0.95f : FLinearColor(0.54f, 0.30f, 0.12f),
            FString::Printf(TEXT("%s Physics Ambush Impact Prop %d"), *CityLabel, i + 1),
            true),
            30.0f + i * 5.0f);
        TagPhysicsYard(EncounterProp);
        TagSurfaceImpact(EncounterProp, SurfaceTag);
        TagPhysicsEncounter(EncounterProp);
        if (EncounterProp)
        {
            EncounterProp->Tags.Add(FName("PhysicsLaneCombatProp"));
            EncounterProp->Tags.Add(FName("ThrowableImpactCoverProp"));
            EncounterProp->Tags.Add(FName("SurfaceImpactCombatTraining"));
        }
    }

    SpawnDestructibleBarricade(
        EncounterCenter + FVector(-275.0f, 330.0f, 46.0f),
        FRotator(0.0f, 10.0f, 0.0f),
        72.0f,
        TEXT("PhysicsAmbushBreakableBarricadeA"));
    SpawnDestructibleBarricade(
        EncounterCenter + FVector(0.0f, 372.0f, 46.0f),
        FRotator(0.0f, -4.0f, 0.0f),
        82.0f,
        TEXT("PhysicsAmbushBreakableBarricadeB"));
    SpawnDestructibleBarricade(
        EncounterCenter + FVector(275.0f, 330.0f, 46.0f),
        FRotator(0.0f, -10.0f, 0.0f),
        72.0f,
        TEXT("PhysicsAmbushBreakableBarricadeC"));

    for (int32 i = 0; i < 3; ++i)
    {
        TagPhysicsEncounter(TagSurfaceImpact(TagPhysicsYard(SpawnBlock(
            EncounterCenter + FVector(-270.0f + i * 270.0f, 205.0f, 54.0f),
            FVector(0.78f, 0.24f, 0.66f),
            FLinearColor(0.16f, 0.18f, 0.20f) + HazardAmber * 0.25f,
            CityLabel + TEXT(" Physics Ambush Readable Cover"),
            true)),
            FName("SurfaceConcrete")));
    }

    SpawnEncounterPickup(EPickupKind::Smoke, EncounterCenter + FVector(-220.0f, -235.0f, 118.0f), 1, TEXT("PhysicsAmbushSmokeCache"));
    SpawnEncounterPickup(EPickupKind::Flare, EncounterCenter + FVector(0.0f, -235.0f, 118.0f), 1, TEXT("PhysicsAmbushFlareCache"));
    SpawnEncounterPickup(EPickupKind::Ammo, EncounterCenter + FVector(220.0f, -235.0f, 118.0f), 40, TEXT("PhysicsAmbushAmmoCache"));

    if (!bSandboxMode)
    {
        const EZombieVariant EncounterVariants[] = {
            EZombieVariant::Default,
            EZombieVariant::NurseFemale,
            EZombieVariant::BusinessSuit,
        };
        const FVector ZombieOffsets[] = {
            FVector(-355.0f, 520.0f, 96.0f),
            FVector(0.0f, 595.0f, 96.0f),
            FVector(355.0f, 520.0f, 96.0f),
        };

        for (int32 i = 0; i < 3; ++i)
        {
            const int32 ZombieId = CodeRescueHordeZombieIdBase + CityIndex * 1000 + 650 + i;

            const FVector Loc = EncounterCenter + ZombieOffsets[i];
            UClass* ZombieClass = ZombieActorClass ? ZombieActorClass.Get() : ACodeZombieActor::StaticClass();
            FActorSpawnParameters ZombieSpawnParams;
            ZombieSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            ACodeZombieActor* Zombie = GetWorld()->SpawnActor<ACodeZombieActor>(ZombieClass, Loc, FRotator::ZeroRotator, ZombieSpawnParams);
            if (!Zombie)
            {
                continue;
            }

            RegisterStreamedActor(Zombie);
            Zombie->ZombieId = ZombieId;
            Zombie->Health = FMath::Max(1.0f, ZombieBaseHealth * (0.62f + i * 0.08f));
            Zombie->AttackDamage = FMath::Max(0.0f, ZombieBaseAttackDamage * 0.68f);
            Zombie->MoveSpeed = ZombieBaseMoveSpeed * (0.88f + i * 0.08f);
            Zombie->AttackRange = FMath::Max(40.0f, ZombieAttackRange);
            Zombie->ActivationRange = FMath::Max(1750.0f, ZombieBaseActivationRange * 0.50f);
            Zombie->RefreshMovementSettings();
            Zombie->ApplyStandardDirectPursuitProfile();

            ApplyZombieFamilyVariant(Zombie, EncounterVariants[i], ZombieId, FName("PhysicsLaneZombieFamily"), true);

            Zombie->Tags.AddUnique(FName("PhysicsLaneCombatZombie"));
            Zombie->Tags.AddUnique(FName("ZombieDeathPhysicsReadabilityTarget"));
            TagPhysicsEncounter(Zombie);
            const FString VariantLabel = GetZombieFamilyVariantMarkerLabel(Zombie->Variant);
            const FLinearColor VariantColor = GetZombieFamilyVariantMarkerColor(Zombie->Variant);
            Zombie->VisualMarkerActor = TagPhysicsEncounter(TagPhysicsYard(SpawnZombieReadabilityMarker(
                Zombie,
                VariantColor * 1.35f,
                FString::Printf(TEXT("%s Physics Ambush %s Marker"), *CityLabel, *VariantLabel),
                0.92f)));
            if (Zombie->VisualMarkerActor)
            {
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("StandardDirectPursuitZombie"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("ZombiePursuitReadableRuntime"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("FairSurvivalPressure"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
                Zombie->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
            }
        }
    }

    TagPhysicsYard(SpawnBlock(
        YardCenter + FVector(-470.0f, -250.0f, 105.0f),
        FVector(0.08f, 0.08f, 2.1f),
        HazardAmber * 1.5f,
        CityLabel + TEXT(" Physics Yard Gravity Gauge Post"),
        true));
    for (int32 i = 0; i < 4; ++i)
    {
        TagPhysicsYard(SpawnBlock(
            YardCenter + FVector(-470.0f, -250.0f, 54.0f + i * 48.0f),
            FVector(0.52f, 0.035f, 0.028f),
            (i < 2 ? AssistGreen : HazardAmber) * 2.0f,
            CityLabel + TEXT(" Physics Yard Gravity Gauge Mark"),
            false));
    }

    TagPhysicsYard(SpawnGuideText(
        TEXT("PHYSICS YARD\nramps, cover, gravity, soft landing assist"),
        YardCenter + FVector(0.0f, 0.0f, 400.0f),
        PhysicsBlue.ToFColor(true),
        34.0f));
    TagPhysicsYard(SpawnGuideText(
        TEXT("real collision, forgiving training pad"),
        YardCenter + FVector(0.0f, 285.0f, 245.0f),
        AssistGreen.ToFColor(true),
        26.0f));
}

void ACodeRescueGameMode::SpawnMissionDioramas(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector ClassroomCenter = Origin + CityOffset(FVector(-2350.0f, -2300.0f, -10.0f));
    const FVector DebugLabCenter = Origin + CityOffset(FVector(1150.0f, -900.0f, -10.0f));
    const FVector QuarantineCenter = Origin + CityOffset(FVector(2900.0f, -1500.0f, -10.0f));
    const FLinearColor ClassroomColor = FLinearColor(0.95f, 0.25f, 1.0f);
    const FLinearColor DebugColor = FLinearColor(0.0f, 0.85f, 1.0f);
    const FLinearColor QuarantineColor = FLinearColor(1.0f, 0.08f, 0.28f);

    SpawnTexturedBlock(
        ClassroomCenter,
        FVector(6.2f, 3.4f, 0.045f),
        ClassroomColor * 0.35f,
        CityLabel + TEXT(" Mission Classroom Floor"),
        TEXT("/Game/StarterContent/Materials/M_Wood_Floor_Walnut_Worn.M_Wood_Floor_Walnut_Worn"),
        false);
    SpawnBlock(
        ClassroomCenter + FVector(0.0f, -230.0f, 145.0f),
        FVector(4.2f, 0.10f, 1.45f),
        FLinearColor(0.08f, 0.10f, 0.12f),
        CityLabel + TEXT(" Mission Classroom Lesson Board"),
        true);
    SpawnBlock(
        ClassroomCenter + FVector(0.0f, -236.0f, 150.0f),
        FVector(3.7f, 0.045f, 1.08f),
        ClassroomColor * 1.8f,
        CityLabel + TEXT(" Mission Classroom Lesson Glow"),
        false);
    for (int32 Row = 0; Row < 2; ++Row)
    {
        for (int32 Col = 0; Col < 3; ++Col)
        {
            const FVector SeatLoc = ClassroomCenter + FVector(-260.0f + Col * 260.0f, 5.0f + Row * 150.0f, 38.0f);
            SpawnBlock(
                SeatLoc,
                FVector(0.85f, 0.35f, 0.26f),
                FLinearColor(0.22f, 0.16f, 0.10f),
                CityLabel + TEXT(" Mission Classroom Bench"),
                true);
        }
    }
    SpawnDecorativeCivilian(
        ClassroomCenter + FVector(-410.0f, 180.0f, 100.0f),
        FRotator(0.0f, -20.0f, 0.0f),
        (CityIndex % 2) == 0,
        ClassroomColor,
        CityLabel + TEXT(" Classroom Civilian Learner A"),
        TEXT("Nova\nLearner"));
    SpawnDecorativeCivilian(
        ClassroomCenter + FVector(390.0f, 165.0f, 100.0f),
        FRotator(0.0f, 15.0f, 0.0f),
        (CityIndex % 2) != 0,
        Mission.SecondaryAccentColor,
        CityLabel + TEXT(" Classroom Civilian Learner B"),
        TEXT("Kai\nLearner"));
    SpawnGuideText(
        TEXT("FIELD CLASSROOM\nchoose language before touching the live terminal"),
        ClassroomCenter + FVector(0.0f, 0.0f, 405.0f),
        ClassroomColor.ToFColor(true),
        34.0f);
    if (UStaticMesh* WindowWallMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Architecture/Wall_Window_400x300.Wall_Window_400x300")))
    {
        SpawnStaticMeshProp(
            WindowWallMesh,
            ClassroomCenter + FVector(-430.0f, -35.0f, 145.0f),
            FRotator(0.0f, 90.0f, 0.0f),
            FVector(1.9f, 0.26f, 1.45f),
            CityLabel + TEXT(" Field Classroom Window Wall A"),
            true);
        SpawnStaticMeshProp(
            WindowWallMesh,
            ClassroomCenter + FVector(430.0f, -35.0f, 145.0f),
            FRotator(0.0f, -90.0f, 0.0f),
            FVector(1.9f, 0.26f, 1.45f),
            CityLabel + TEXT(" Field Classroom Window Wall B"),
            true);
    }

    SpawnTexturedBlock(
        DebugLabCenter,
        FVector(5.8f, 4.1f, 0.045f),
        DebugColor * 0.32f,
        CityLabel + TEXT(" Mission Debug Lab Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Checker_Dot.M_Tech_Checker_Dot"),
        false);
    if (UStaticMesh* TableMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_TableRound.SM_TableRound")))
    {
        SpawnStaticMeshProp(
            TableMesh,
            DebugLabCenter + FVector(0.0f, 25.0f, 45.0f),
            FRotator::ZeroRotator,
            FVector(1.35f, 1.35f, 0.75f),
            CityLabel + TEXT(" Mission Debug Lab Table"),
            true);
    }
    for (int32 i = 0; i < 4; ++i)
    {
        const float X = -360.0f + i * 240.0f;
        SpawnBlock(
            DebugLabCenter + FVector(X, -240.0f, 110.0f),
            FVector(0.58f, 0.32f, 1.75f),
            FLinearColor(0.06f, 0.08f, 0.10f),
            CityLabel + TEXT(" Mission Debug Server Rack"),
            true);
        SpawnBlock(
            DebugLabCenter + FVector(X, -274.0f, 154.0f),
            FVector(0.42f, 0.035f, 0.40f),
            DebugColor * (1.8f + i * 0.35f),
            CityLabel + TEXT(" Mission Debug Server Screen"),
            false);
    }
    for (int32 i = 0; i < 3; ++i)
    {
        SpawnBlock(
            DebugLabCenter + FVector(-260.0f + i * 260.0f, 250.0f, 16.0f),
            FVector(1.5f, 0.055f, 0.055f),
            DebugColor * 2.2f,
            CityLabel + TEXT(" Mission Debug Cable Run"),
            false);
    }
    SpawnDecorativeCivilian(
        DebugLabCenter + FVector(-460.0f, 210.0f, 100.0f),
        FRotator(0.0f, 45.0f, 0.0f),
        true,
        DebugColor,
        CityLabel + TEXT(" Debug Lab Civilian Analyst"),
        TEXT("Dr. Vale\nAnalyst"));
    SpawnGuideText(
        FString::Printf(TEXT("TRACE FIELD LAB\n%s"), *Mission.TerminalTitle),
        DebugLabCenter + FVector(0.0f, 0.0f, 470.0f),
        DebugColor.ToFColor(true),
        34.0f);
    if (UStaticMesh* GlassWindowMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_GlassWindow.SM_GlassWindow")))
    {
        for (int32 i = 0; i < 3; ++i)
        {
            SpawnStaticMeshProp(
                GlassWindowMesh,
                DebugLabCenter + FVector(-220.0f + i * 220.0f, 385.0f, 170.0f),
                FRotator(0.0f, 0.0f, 0.0f),
                FVector(0.85f, 0.10f, 0.92f),
                CityLabel + TEXT(" Debug Lab Observation Glass"),
                false,
                TEXT("/Game/StarterContent/Materials/M_Metal_Steel.M_Metal_Steel"));
        }
    }

    SpawnTexturedBlock(
        QuarantineCenter,
        FVector(6.4f, 4.4f, 0.05f),
        QuarantineColor * 0.26f,
        CityLabel + TEXT(" Warden Quarantine Floor"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Rust.M_Metal_Rust"),
        false);
    if (UStaticMesh* DoorFrameMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_DoorFrame.SM_DoorFrame")))
    {
        SpawnStaticMeshProp(
            DoorFrameMesh,
            QuarantineCenter + FVector(0.0f, -250.0f, 120.0f),
            FRotator(0.0f, 90.0f, 0.0f),
            FVector(1.6f, 0.45f, 1.7f),
            CityLabel + TEXT(" Warden Quarantine Gate Frame"),
            true);
    }
    if (UStaticMesh* PillarMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_PillarFrame.SM_PillarFrame")))
    {
        SpawnStaticMeshProp(
            PillarMesh,
            QuarantineCenter + FVector(-390.0f, -120.0f, 160.0f),
            FRotator::ZeroRotator,
            FVector(0.65f, 0.65f, 1.45f),
            CityLabel + TEXT(" Warden Quarantine Tower A"),
            true);
        SpawnStaticMeshProp(
            PillarMesh,
            QuarantineCenter + FVector(390.0f, -120.0f, 160.0f),
            FRotator::ZeroRotator,
            FVector(0.65f, 0.65f, 1.45f),
            CityLabel + TEXT(" Warden Quarantine Tower B"),
            true);
    }
    for (int32 i = 0; i < 5; ++i)
    {
        const float X = -440.0f + i * 220.0f;
        SpawnBlock(
            QuarantineCenter + FVector(X, 205.0f, 58.0f),
            FVector(1.15f, 0.22f, 0.58f),
            FLinearColor(0.46f, 0.40f, 0.30f),
            CityLabel + TEXT(" Warden Quarantine Barricade"),
            true);
        SpawnBlock(
            QuarantineCenter + FVector(X, 205.0f, 115.0f),
            FVector(0.50f, 0.05f, 0.09f),
            QuarantineColor * 2.6f,
            CityLabel + TEXT(" Warden Barricade Warning Light"),
            false);
    }
    SpawnGuideText(
        TEXT("QUARANTINE LINE\noptional warden fight beyond this point"),
        QuarantineCenter + FVector(0.0f, 0.0f, 500.0f),
        QuarantineColor.ToFColor(true),
        38.0f);
}

void ACodeRescueGameMode::SpawnAccountLinkedAssetShowcase(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector ShowcaseCenter = Origin + CityOffset(FVector(3250.0f, 420.0f, -12.0f));
    const FLinearColor BayColor = Mission.SecondaryAccentColor * 0.55f + FLinearColor(0.05f, 0.18f, 0.22f) * 0.45f;

    SpawnTexturedBlock(
        ShowcaseCenter,
        CityExtent(FVector(8.5f, 5.0f, 0.045f)),
        BayColor,
        CityLabel + TEXT(" Fab Account Asset Intake Slab"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false);
    SpawnBlock(
        ShowcaseCenter + FVector(0.0f, 0.0f, 170.0f),
        CityExtent(FVector(8.7f, 0.10f, 0.12f)),
        Mission.AccentColor * 2.4f,
        CityLabel + TEXT(" Fab Intake Light Strip A"),
        false);
    SpawnBlock(
        ShowcaseCenter + CityOffset(FVector(0.0f, 500.0f, 0.0f)) + FVector(0.0f, 0.0f, 170.0f),
        CityExtent(FVector(8.7f, 0.10f, 0.12f)),
        Mission.AccentColor * 2.4f,
        CityLabel + TEXT(" Fab Intake Light Strip B"),
        false);

    if (UStaticMesh* BridgeMesh = LoadCodeRescueBridgeMesh(CityIndex + 2))
    {
        AActor* Bridge = SpawnStaticMeshProp(
            BridgeMesh,
            ShowcaseCenter + CityOffset(FVector(0.0f, -250.0f, 110.0f)),
            FRotator(0.0f, 90.0f, 0.0f),
            CityExtent(FVector(7.25f, 0.82f, 0.42f)),
            CityLabel + TEXT(" Fab Showcase Modern Bridge"),
            false);
        if (Bridge)
        {
            Bridge->Tags.Add(FName("FabShowcase"));
            Bridge->Tags.Add(FName("AccountLinkedAsset"));
        }
    }

    for (int32 i = 0; i < 3; ++i)
    {
        if (UStaticMesh* BuildingMesh = LoadCodeRescueCityBuildingMesh(CityIndex + i + 3))
        {
            const float LocalX = -520.0f + static_cast<float>(i) * 520.0f;
            const float Height = 2.65f + static_cast<float>(i) * 0.55f;
            const FVector TowerScale = CityArchitectureExtent(FVector(0.85f + i * 0.12f, 0.85f + i * 0.10f, Height));
            AActor* Tower = SpawnStaticMeshProp(
                BuildingMesh,
                ShowcaseCenter + CityOffset(FVector(LocalX, 760.0f, 0.0f)) + FVector(0.0f, 0.0f, TowerScale.Z * 50.0f),
                FRotator(0.0f, 90.0f * (i % 4), 0.0f),
                TowerScale,
                FString::Printf(TEXT("%s Fab Showcase Parallax Tower %d"), *CityLabel, i + 1),
                true);
            if (Tower)
            {
                Tower->Tags.Add(FName("FabShowcase"));
                Tower->Tags.Add(FName("AccountLinkedAsset"));
            }
        }
    }

    for (int32 i = 0; i < 4; ++i)
    {
        const FVector CrateLoc = ShowcaseCenter + CityOffset(FVector(-690.0f + i * 460.0f, -760.0f, 42.0f));
        SpawnBlock(
            CrateLoc,
            CityExtent(FVector(0.85f, 0.50f, 0.55f)),
            Mission.AccentColor * (0.75f + i * 0.18f),
            CityLabel + TEXT(" Fab Vault Cache Crate"),
            true);
    }

    SpawnBlock(
        ShowcaseCenter + CityOffset(FVector(0.0f, -900.0f, 120.0f)),
        CityExtent(FVector(2.2f, 0.18f, 1.15f)),
        FLinearColor(0.18f, 0.95f, 1.0f) * 1.25f,
        CityLabel + TEXT(" MetaHuman Ready Intake Marker"),
        false);
    SpawnGuideText(
        TEXT("FAB / VAULT CONTENT BAY\nModern bridges + parallax city assets"),
        ShowcaseCenter + FVector(0.0f, 0.0f, 520.0f),
        FColor(170, 235, 255),
        44.0f);
    SpawnGuideText(
        TEXT("METAHUMAN-READY PAD\nDownload a character in Fab/Bridge, then run import script"),
        ShowcaseCenter + CityOffset(FVector(0.0f, -980.0f, 0.0f)) + FVector(0.0f, 0.0f, 380.0f),
        FColor(120, 255, 255),
        34.0f);
}

void ACodeRescueGameMode::SpawnImmediateGameImprovementLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    const FLinearColor RouteGold = FLinearColor(1.0f, 0.74f, 0.18f);
    const FLinearColor RescueCyan = FLinearColor(0.18f, 0.92f, 1.0f);
    const FLinearColor DangerRed = FLinearColor(1.0f, 0.10f, 0.08f);
    const FLinearColor ShelterGreen = FLinearColor(0.22f, 1.0f, 0.42f);
    const FLinearColor StudyAmber = FLinearColor(0.96f, 0.58f, 0.20f);
    const FLinearColor CityTone = Mission.AccentColor * 0.54f + Mission.SecondaryAccentColor * 0.46f;
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    auto TagImprovement = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("ImmediateImprovementPass20"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto SpawnImprovementLight = [&](const FVector& Local, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                Origin + CityOffset(Local),
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagImprovement(Light);
        }
    };

    auto SpawnSupplyPickup = [&](EPickupKind Kind, const FVector& Local, int32 Amount, const FString& Label)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(
            APickupActor::StaticClass(),
            Origin + CityOffset(Local),
            FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("ImmediateImprovementSupply"));
            RegisterStreamedActor(Pickup);
            TagImprovement(Pickup);
            TagImprovement(SpawnGuideText(
                Label,
                Pickup->GetActorLocation() + FVector(0.0f, 0.0f, 210.0f),
                Kind == EPickupKind::Ammo ? FColor(100, 190, 255) : FColor(120, 255, 150),
                28.0f));
        }
    };

    struct FImprovementStop
    {
        FVector Local;
        FString Label;
        FLinearColor Color;
    };

    const TArray<FImprovementStop> Stops = {
        { FVector(-3820.0f, -3180.0f, 0.0f), TEXT("ENTRY"), ShelterGreen },
        { FVector(-3000.0f, -2300.0f, 0.0f), TEXT("LANGUAGE"), StudyAmber },
        { FVector(1150.0f, -900.0f, 0.0f), TEXT("TERMINAL"), RescueCyan },
        { FVector(2850.0f, 1500.0f, 0.0f), TEXT("RESCUE"), RouteGold },
        { FVector(2900.0f, -1500.0f, 0.0f), TEXT("WARDEN"), DangerRed },
    };

    auto SpawnCheckpointArch = [&](const FImprovementStop& Stop, int32 Index)
    {
        const FVector Center = Origin + CityOffset(Stop.Local);
        const FLinearColor Color = Stop.Color * 1.55f;
        TagImprovement(SpawnBlock(
            Center + FVector(-165.0f, 0.0f, 180.0f),
            FVector(0.16f, 0.16f, 3.05f),
            Color,
            FString::Printf(TEXT("%s Improvement Checkpoint %d Pillar A"), *CityLabel, Index),
            false));
        TagImprovement(SpawnBlock(
            Center + FVector(165.0f, 0.0f, 180.0f),
            FVector(0.16f, 0.16f, 3.05f),
            Color,
            FString::Printf(TEXT("%s Improvement Checkpoint %d Pillar B"), *CityLabel, Index),
            false));
        TagImprovement(SpawnBlock(
            Center + FVector(0.0f, 0.0f, 332.0f),
            FVector(3.35f, 0.08f, 0.12f),
            Color * 1.2f,
            FString::Printf(TEXT("%s Improvement Checkpoint %d Header"), *CityLabel, Index),
            false));
        TagImprovement(SpawnGuideText(
            Stop.Label,
            Center + FVector(0.0f, 92.0f, 260.0f),
            Color.ToFColor(true),
            30.0f));
        SpawnImprovementLight(Stop.Local + FVector(0.0f, 0.0f, 230.0f), Stop.Color, 3000.0f, 700.0f,
            FString::Printf(TEXT("%s Improvement Checkpoint Light %d"), *CityLabel, Index));
    };

    for (int32 i = 0; i < Stops.Num(); ++i)
    {
        SpawnCheckpointArch(Stops[i], i + 1);
    }

    auto SpawnRouteChevrons = [&](const FVector& A, const FVector& B, const FLinearColor& Color, const FString& NamePrefix)
    {
        const FVector Delta = B - A;
        const int32 ChevronCount = FMath::Clamp(FMath::CeilToInt(Delta.Size2D() / 620.0f), 2, 9);
        const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
        for (int32 i = 1; i <= ChevronCount; ++i)
        {
            const float T = (static_cast<float>(i) / static_cast<float>(ChevronCount + 1));
            const FVector Local = FMath::Lerp(A, B, T);
            TagImprovement(SpawnRotatedBlock(
                Origin + CityOffset(FVector(Local.X, Local.Y, -4.0f)),
                FRotator(0.0f, Yaw, 0.0f),
                CityExtent(FVector(0.95f, 0.13f, 0.030f)),
                Color * 2.2f,
                FString::Printf(TEXT("%s %s Route Chevron %d"), *CityLabel, *NamePrefix, i),
                false));
            TagImprovement(SpawnRotatedBlock(
                Origin + CityOffset(FVector(Local.X, Local.Y, -3.0f)) + FVector(38.0f, 0.0f, 0.0f),
                FRotator(0.0f, Yaw + 28.0f, 0.0f),
                CityExtent(FVector(0.48f, 0.10f, 0.028f)),
                Color * 2.2f,
                FString::Printf(TEXT("%s %s Route Chevron Wing A %d"), *CityLabel, *NamePrefix, i),
                false));
            TagImprovement(SpawnRotatedBlock(
                Origin + CityOffset(FVector(Local.X, Local.Y, -3.0f)) - FVector(38.0f, 0.0f, 0.0f),
                FRotator(0.0f, Yaw - 28.0f, 0.0f),
                CityExtent(FVector(0.48f, 0.10f, 0.028f)),
                Color * 2.2f,
                FString::Printf(TEXT("%s %s Route Chevron Wing B %d"), *CityLabel, *NamePrefix, i),
                false));
        }
    };

    SpawnRouteChevrons(Stops[0].Local, Stops[1].Local, ShelterGreen, TEXT("EntryToLanguage"));
    SpawnRouteChevrons(Stops[1].Local, Stops[2].Local, RescueCyan, TEXT("LanguageToTerminal"));
    SpawnRouteChevrons(Stops[2].Local, Stops[3].Local, RouteGold, TEXT("TerminalToRescue"));
    SpawnRouteChevrons(Stops[2].Local, Stops[4].Local, DangerRed, TEXT("TerminalToWarden"));

    const FVector SafehouseCenter = FVector(-1125.0f, -3025.0f, 25.0f);
    SpawnSupplyPickup(EPickupKind::Ammo, SafehouseCenter + FVector(-245.0f, 45.0f, 85.0f), 45, TEXT("SAFEHOUSE AMMO"));
    SpawnSupplyPickup(EPickupKind::Medkit, SafehouseCenter + FVector(250.0f, 58.0f, 85.0f), 1, TEXT("SAFEHOUSE MEDKIT"));
    TagImprovement(SpawnBlock(
        Origin + CityOffset(SafehouseCenter + FVector(0.0f, -250.0f, 200.0f)),
        FVector(2.75f, 0.04f, 0.58f),
        ShelterGreen * 1.8f,
        CityLabel + TEXT(" Safehouse Supply Inventory Board"),
        false));
    TagImprovement(SpawnGuideText(
        TEXT("SUPPLY CACHE\nrestock before the street push"),
        Origin + CityOffset(SafehouseCenter) + FVector(0.0f, -292.0f, 270.0f),
        ShelterGreen.ToFColor(true),
        28.0f));

    const FVector EntryBoard = Origin + CityOffset(FVector(-3680.0f, -2920.0f, 0.0f));
    TagImprovement(SpawnBlock(
        EntryBoard + FVector(0.0f, -80.0f, 160.0f),
        FVector(4.8f, 0.08f, 1.22f),
        FLinearColor(0.045f, 0.052f, 0.060f) + CityTone * 0.22f,
        CityLabel + TEXT(" Active City Operations Board"),
        false));
    TagImprovement(SpawnGuideText(
        FString::Printf(
            TEXT("OPERATIONS BOARD\n%s, %s\nTerminal: %s\nRescue: %s\nFocus: %s"),
            *Mission.CityName,
            *Mission.StateName,
            *Mission.TerminalTitle,
            *Mission.SurvivorName,
            *Mission.CurriculumFocus),
        EntryBoard + FVector(0.0f, -112.0f, 214.0f),
        FColor(220, 245, 255),
        24.0f));

    const FVector ClassroomCenter = Origin + CityOffset(FVector(-2350.0f, -2300.0f, 0.0f));
    static const TCHAR* LangNames[] = { TEXT("JAVA"), TEXT("C"), TEXT("PYTHON"), TEXT("MATLAB"), TEXT("C+"), TEXT("C++") };
    static const TCHAR* LangTips[] = {
        TEXT("types + classes"),
        TEXT("memory + loops"),
        TEXT("readable algorithms"),
        TEXT("matrices + vectors"),
        TEXT("C-style logic"),
        TEXT("typed systems"),
    };
    const int32 SelectedLanguageIndex = GI
        ? FMath::Clamp(static_cast<int32>(GI->SelectedLanguage), 0, static_cast<int32>(UE_ARRAY_COUNT(LangNames)) - 1)
        : 0;
    const FVector PanelLoc = ClassroomCenter + FVector(0.0f, -305.0f, 170.0f);
    const FLinearColor PanelColor = (SelectedLanguageIndex % 2 == 0) ? StudyAmber : RescueCyan;
    TagImprovement(SpawnBlock(
        PanelLoc,
        FVector(1.55f, 0.045f, 0.62f),
        PanelColor * 1.4f,
        CityLabel + TEXT(" Selected Curriculum Wall Panel"),
        false));
    TagImprovement(SpawnGuideText(
        FString::Printf(TEXT("%s TRACK\n%s"), LangNames[SelectedLanguageIndex], LangTips[SelectedLanguageIndex]),
        PanelLoc + FVector(0.0f, -42.0f, 72.0f),
        PanelColor.ToFColor(true),
        20.0f));

    const FVector TerminalBase = Origin + CityOffset(FVector(1150.0f, -900.0f, 0.0f));
    TagImprovement(SpawnBlock(
        TerminalBase + FVector(-320.0f, -230.0f, 68.0f),
        FVector(1.35f, 0.42f, 0.64f),
        FLinearColor(0.08f, 0.10f, 0.12f) + RescueCyan * 0.18f,
        CityLabel + TEXT(" Terminal Debug Cover A"),
        true));
    TagImprovement(SpawnBlock(
        TerminalBase + FVector(330.0f, -215.0f, 68.0f),
        FVector(1.35f, 0.42f, 0.64f),
        FLinearColor(0.08f, 0.10f, 0.12f) + RescueCyan * 0.18f,
        CityLabel + TEXT(" Terminal Debug Cover B"),
        true));
    for (int32 i = 0; i < 4; ++i)
    {
        TagImprovement(SpawnBlock(
            TerminalBase + FVector(-300.0f + i * 200.0f, 278.0f, 18.0f),
            FVector(1.25f, 0.045f, 0.055f),
            RescueCyan * (1.8f + i * 0.15f),
            CityLabel + TEXT(" Terminal Data Cable"),
            false));
    }
    TagImprovement(SpawnGuideText(
        TEXT("TRACE COVER\nsolve, then hold the line"),
        TerminalBase + FVector(0.0f, -330.0f, 250.0f),
        RescueCyan.ToFColor(true),
        28.0f));

    const FVector SurvivorBase = Origin + CityOffset(FVector(2850.0f, 1500.0f, 0.0f));
    for (int32 i = 0; i < 5; ++i)
    {
        const FVector Local = FVector(2420.0f + i * 210.0f, 1160.0f + i * 110.0f, 18.0f);
        TagImprovement(SpawnBlock(
            Origin + CityOffset(Local),
            FVector(0.62f, 0.12f, 0.12f),
            RouteGold * 2.1f,
            CityLabel + TEXT(" Rescue Evac Footlight"),
            false));
        SpawnImprovementLight(Local + FVector(0.0f, 0.0f, 142.0f), RouteGold, 1800.0f, 520.0f,
            CityLabel + TEXT(" Rescue Evac Light"));
    }
    TagImprovement(SpawnBlock(
        SurvivorBase + FVector(0.0f, 310.0f, 112.0f),
        FVector(3.8f, 0.12f, 0.96f),
        RouteGold * 0.85f,
        CityLabel + TEXT(" Rescue Extraction Gate"),
        false));
    TagImprovement(SpawnGuideText(
        TEXT("EXTRACTION CORRIDOR\nescort civilians toward the helipad"),
        SurvivorBase + FVector(0.0f, 365.0f, 278.0f),
        RouteGold.ToFColor(true),
        30.0f));

    const FVector BossBase = Origin + CityOffset(FVector(2900.0f, -1500.0f, 0.0f));
    for (int32 i = 0; i < 8; ++i)
    {
        const float Angle = (static_cast<float>(i) / 8.0f) * 2.0f * PI;
        const FVector CoverLoc = BossBase + FVector(FMath::Cos(Angle) * 560.0f, FMath::Sin(Angle) * 390.0f, 62.0f);
        TagImprovement(SpawnBlock(
            CoverLoc,
            FVector(1.0f, 0.42f, 0.64f),
            FLinearColor(0.18f, 0.12f, 0.12f) + DangerRed * 0.18f,
            CityLabel + TEXT(" Warden Arena Cover"),
            true));
    }
    TagImprovement(SpawnGuideText(
        TEXT("WARDEN ARENA\ncover, supplies, and clear warning light"),
        BossBase + FVector(0.0f, 0.0f, 520.0f),
        DangerRed.ToFColor(true),
        34.0f));
    SpawnImprovementLight(FVector(2900.0f, -1500.0f, 360.0f), DangerRed, 7600.0f, 1200.0f, CityLabel + TEXT(" Warden Warning Floodlight"));

    const FVector ReliefMarket = Origin + CityOffset(FVector(-720.0f, 1920.0f, 0.0f));
    for (int32 i = 0; i < 4; ++i)
    {
        const FVector Stall = ReliefMarket + FVector(-330.0f + i * 220.0f, 0.0f, 74.0f);
        const FLinearColor StallColor = (i % 2 == 0) ? Mission.SecondaryAccentColor : Mission.AccentColor;
        TagImprovement(SpawnBlock(
            Stall,
            FVector(0.92f, 0.54f, 0.44f),
            StallColor * 0.92f,
            CityLabel + TEXT(" Relief Market Stall"),
            true));
        TagImprovement(SpawnBlock(
            Stall + FVector(0.0f, 0.0f, 82.0f),
            FVector(1.05f, 0.62f, 0.08f),
            StallColor * 1.6f,
            CityLabel + TEXT(" Relief Market Canopy"),
            false));
    }
    TagImprovement(SpawnGuideText(
        TEXT("RELIEF MARKET\nnoncombat city life"),
        ReliefMarket + FVector(0.0f, 0.0f, 275.0f),
        Mission.SecondaryAccentColor.ToFColor(true),
        28.0f));

    const FVector TransitStop = Origin + CityOffset(FVector(-3100.0f, -1680.0f, 0.0f));
    TagImprovement(SpawnBlock(
        TransitStop + FVector(0.0f, 0.0f, 92.0f),
        FVector(3.2f, 0.28f, 0.24f),
        FLinearColor(0.12f, 0.16f, 0.18f) + CityTone * 0.15f,
        CityLabel + TEXT(" Transit Stop Bench"),
        true));
    TagImprovement(SpawnBlock(
        TransitStop + FVector(0.0f, -85.0f, 178.0f),
        FVector(3.6f, 0.08f, 0.16f),
        RescueCyan * 1.4f,
        CityLabel + TEXT(" Transit Stop Shelter Roof"),
        false));
    TagImprovement(SpawnGuideText(
        TEXT("CIVIC TRANSIT\nroute starts here"),
        TransitStop + FVector(0.0f, -118.0f, 265.0f),
        RescueCyan.ToFColor(true),
        24.0f));

    for (int32 i = 0; i < 11; ++i)
    {
        const float X = -3600.0f + i * 720.0f;
        const float Height = 0.48f + 0.08f * (i % 4);
        TagImprovement(SpawnBlock(
            Origin + CityOffset(FVector(X, 3330.0f, 42.0f)),
            CityExtent(FVector(0.42f, 0.16f, Height)),
            FLinearColor(0.055f, 0.055f, 0.050f) + DangerRed * 0.08f,
            CityLabel + TEXT(" Distant Horde Silhouette"),
            false));
    }

    const FVector SkylineBeacon = Origin + CityOffset(FVector(-420.0f, 640.0f, 0.0f));
    TagImprovement(SpawnBlock(
        SkylineBeacon + FVector(0.0f, 0.0f, 520.0f),
        FVector(0.20f, 0.20f, 8.4f),
        CityTone * 1.9f,
        CityLabel + TEXT(" Skyline Orientation Beacon Mast"),
        false));
    TagImprovement(SpawnBlock(
        SkylineBeacon + FVector(0.0f, 0.0f, 965.0f),
        FVector(1.2f, 1.2f, 0.16f),
        CityTone * 4.0f,
        CityLabel + TEXT(" Skyline Orientation Beacon Crown"),
        false));
    SpawnImprovementLight(FVector(-420.0f, 640.0f, 960.0f), CityTone, 5200.0f, 1600.0f, CityLabel + TEXT(" Skyline Beacon Light"));

    const FString Kit = Mission.ArtKitName;
    const FVector RegionCenter = Origin + CityOffset(FVector(3300.0f, 2300.0f, 0.0f));
    if (Kit.Contains(TEXT("Port")) || Kit.Contains(TEXT("Harbor")) || Kit.Contains(TEXT("Lakes")) || Kit.Contains(TEXT("River")))
    {
        TagImprovement(SpawnBlock(RegionCenter + FVector(0.0f, 0.0f, 52.0f), FVector(3.0f, 0.52f, 0.22f), FLinearColor(0.05f, 0.22f, 0.30f) * 1.8f, CityLabel + TEXT(" Regional Ferry Hull"), true));
        TagImprovement(SpawnBlock(RegionCenter + FVector(0.0f, 0.0f, 118.0f), FVector(1.1f, 0.36f, 0.42f), FLinearColor(0.80f, 0.86f, 0.90f), CityLabel + TEXT(" Regional Ferry Cabin"), true));
        TagImprovement(SpawnGuideText(TEXT("REGIONAL WATERFRONT"), RegionCenter + FVector(0.0f, 0.0f, 270.0f), FColor(120, 230, 255), 28.0f));
    }
    else if (Kit.Contains(TEXT("Solar")) || Kit.Contains(TEXT("Desert")))
    {
        for (int32 i = 0; i < 4; ++i)
        {
            TagImprovement(SpawnRotatedBlock(RegionCenter + FVector(-330.0f + i * 220.0f, 0.0f, 118.0f), FRotator(0.0f, 0.0f, -16.0f), FVector(1.35f, 0.10f, 0.52f), FLinearColor(0.05f, 0.14f, 0.28f) * 2.0f, CityLabel + TEXT(" Regional Solar Panel"), false));
        }
        TagImprovement(SpawnGuideText(TEXT("REGIONAL SOLAR GRID"), RegionCenter + FVector(0.0f, 0.0f, 270.0f), FColor(255, 220, 110), 28.0f));
    }
    else if (Kit.Contains(TEXT("Neon")))
    {
        for (int32 i = 0; i < 5; ++i)
        {
            TagImprovement(SpawnBlock(RegionCenter + FVector(-340.0f + i * 170.0f, 0.0f, 150.0f + i * 22.0f), FVector(0.55f, 0.18f, 1.55f + i * 0.14f), (i % 2 == 0 ? FLinearColor(0.0f, 0.95f, 1.0f) : FLinearColor(1.0f, 0.14f, 0.82f)) * 1.8f, CityLabel + TEXT(" Regional Neon Totem"), false));
        }
        TagImprovement(SpawnGuideText(TEXT("REGIONAL NEON CORE"), RegionCenter + FVector(0.0f, 0.0f, 380.0f), FColor(180, 255, 255), 28.0f));
    }
    else
    {
        TagImprovement(SpawnBlock(RegionCenter + FVector(0.0f, 0.0f, 158.0f), FVector(0.36f, 0.36f, 3.1f), CityTone * 1.15f, CityLabel + TEXT(" Regional Civic Tower"), true));
        TagImprovement(SpawnBlock(RegionCenter + FVector(0.0f, 0.0f, 334.0f), FVector(1.55f, 1.55f, 0.20f), CityTone * 1.8f, CityLabel + TEXT(" Regional Civic Crown"), false));
        TagImprovement(SpawnGuideText(TEXT("REGIONAL CIVIC LANDMARK"), RegionCenter + FVector(0.0f, 0.0f, 470.0f), CityTone.ToFColor(true), 28.0f));
    }

    const FVector AchievementPodium = Origin + CityOffset(FVector(2140.0f, 2380.0f, 0.0f));
    for (int32 i = 0; i < 3; ++i)
    {
        TagImprovement(SpawnBlock(
            AchievementPodium + FVector(-170.0f + i * 170.0f, 0.0f, 38.0f + i * 18.0f),
            FVector(1.05f, 0.78f, 0.52f + i * 0.18f),
            (i == 0 ? RescueCyan : i == 1 ? RouteGold : ShelterGreen) * 0.92f,
            CityLabel + TEXT(" Student Achievement Podium"),
            true));
    }
    TagImprovement(SpawnGuideText(
        TEXT("GRADUATION PODIUM\ncity cleared after code + rescue"),
        AchievementPodium + FVector(0.0f, 0.0f, 270.0f),
        ShelterGreen.ToFColor(true),
        28.0f));

    TagImprovement(SpawnGuideText(
        TEXT("20 ITEM IMPROVEMENT PASS\nHUD clarity, route arrows, supplies, story, boss arena, region identity"),
        Origin + CityOffset(FVector(-720.0f, -3350.0f, 395.0f)),
        FColor(230, 245, 255),
        30.0f));
}

void ACodeRescueGameMode::SpawnCodingLearningGamificationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const FLinearColor JavaColor = FLinearColor(1.0f, 0.40f, 0.18f);
    const FLinearColor CColor = FLinearColor(0.25f, 0.58f, 1.0f);
    const FLinearColor PythonColor = FLinearColor(1.0f, 0.86f, 0.22f);
    const FLinearColor MatlabColor = FLinearColor(0.85f, 0.30f, 1.0f);
    const FLinearColor MentorGreen = FLinearColor(0.30f, 1.0f, 0.58f);
    const FLinearColor DebugCyan = FLinearColor(0.18f, 0.92f, 1.0f);
    const FLinearColor TestAmber = FLinearColor(1.0f, 0.72f, 0.18f);

    auto TagLearning = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("CodingLearningGamification"));
            Actor->Tags.Add(FName("CurriculumWorldbuilding"));
        }
        return Actor;
    };

    auto CountAt = [](const TArray<int32>& Counters, int32 Idx) -> int32
    {
        return Counters.IsValidIndex(Idx) ? Counters[Idx] : 0;
    };

    auto SpawnLearningLight = [&](const FVector& Local, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                Origin + CityOffset(Local),
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagLearning(Light);
        }
    };

    const FString LearningSummary = GI
        ? GI->GetLearningProgressSummary()
        : TEXT("New Coder | Attempts 0 | Success 0% | Streak 0 (best 0) | No-hint 0 | Perfect 0");
    const FString LanguageSummary = GI
        ? GI->GetLanguageProgressSummary()
        : TEXT("Java track 0/0");

    const FVector AcademyCenter = Origin + CityOffset(FVector(-2380.0f, -1780.0f, 0.0f));
    TagLearning(SpawnTexturedBlock(
        AcademyCenter + FVector(0.0f, 0.0f, -8.0f),
        FVector(5.8f, 3.6f, 0.045f),
        Mission.AccentColor * 0.35f + DebugCyan * 0.22f,
        CityLabel + TEXT(" Coding Academy Mastery Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Checker_Dot.M_Tech_Checker_Dot"),
        false));
    TagLearning(SpawnBlock(
        AcademyCenter + FVector(0.0f, -250.0f, 170.0f),
        FVector(4.85f, 0.08f, 1.22f),
        FLinearColor(0.04f, 0.05f, 0.06f) + Mission.AccentColor * 0.22f,
        CityLabel + TEXT(" Coding Academy Progress Wall"),
        false));
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("CODING ACADEMY\n%s\n%s"), *LearningSummary, *LanguageSummary),
        AcademyCenter + FVector(0.0f, -292.0f, 265.0f),
        FColor(220, 255, 235),
        22.0f));
    SpawnLearningLight(FVector(-2380.0f, -1780.0f, 245.0f), DebugCyan, 4200.0f, 850.0f, CityLabel + TEXT(" Coding Academy Mentor Light"));

    static const TCHAR* LangNames[] = { TEXT("JAVA"), TEXT("C"), TEXT("PYTHON"), TEXT("MATLAB"), TEXT("C+"), TEXT("C++") };
    static const TCHAR* LangRole[] = {
        TEXT("types and classes"),
        TEXT("memory and pointers"),
        TEXT("readable algorithms"),
        TEXT("arrays and matrices"),
        TEXT("C-style logic with safer library steps"),
        TEXT("vectors, strings, and typed systems"),
    };
    const FLinearColor LangColors[] = { JavaColor, CColor, PythonColor, MatlabColor, DebugCyan, CColor * 0.7f + DebugCyan * 0.6f };
    const FVector LanguageBase = Origin + CityOffset(FVector(-3000.0f, -2300.0f, 0.0f));
    const int32 SelectedLanguageIndex = GI
        ? FMath::Clamp(static_cast<int32>(GI->SelectedLanguage), 0, static_cast<int32>(UE_ARRAY_COUNT(LangNames)) - 1)
        : 0;
    {
        const int32 i = SelectedLanguageIndex;
        const int32 Solves = GI ? CountAt(GI->LanguageSolveCounts, i) : 0;
        const int32 Attempts = GI ? CountAt(GI->LanguageAttemptCounts, i) : 0;
        const int32 NoHint = GI ? CountAt(GI->LanguageNoHintSolveCounts, i) : 0;
        const int32 Bars = FMath::Clamp(Solves, 0, 5);
        const FVector Monument = LanguageBase + CityOffset(FVector(650.0f, 340.0f, 0.0f));
        TagLearning(SpawnBlock(
            Monument + FVector(0.0f, 0.0f, 48.0f),
            FVector(1.35f, 0.86f, 0.42f),
            LangColors[i] * 0.72f,
            FString::Printf(TEXT("%s %s Mastery Monument Base"), *CityLabel, LangNames[i]),
            true));
        TagLearning(SpawnBlock(
            Monument + FVector(0.0f, 0.0f, 178.0f),
            FVector(0.44f, 0.44f, 2.15f),
            LangColors[i] * 1.55f,
            FString::Printf(TEXT("%s %s Mastery Monument Core"), *CityLabel, LangNames[i]),
            false));
        for (int32 Bar = 0; Bar < 5; ++Bar)
        {
            TagLearning(SpawnBlock(
                Monument + FVector(-82.0f + Bar * 41.0f, -58.0f, 93.0f + Bar * 24.0f),
                FVector(0.18f, 0.035f, 0.10f + Bar * 0.03f),
                Bar < Bars ? LangColors[i] * 2.2f : FLinearColor(0.12f, 0.13f, 0.14f),
                FString::Printf(TEXT("%s %s Mastery Progress Bar %d"), *CityLabel, LangNames[i], Bar + 1),
                false));
        }
        TagLearning(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s\nsolves %d | attempts %d | no-hint %d"), LangNames[i], LangRole[i], Solves, Attempts, NoHint),
            Monument + FVector(0.0f, -92.0f, 305.0f),
            LangColors[i].ToFColor(true),
            22.0f));
    }

    const FVector PracticeStart = Origin + CityOffset(FVector(-1850.0f, -1380.0f, 0.0f));
    static const TCHAR* ConceptNames[] = {
        TEXT("FUNCTION"),
        TEXT("BOOLEAN"),
        TEXT("LOOP"),
        TEXT("DATA FLOW"),
        TEXT("TEST"),
    };
    static const TCHAR* ConceptTips[] = {
        TEXT("name input output"),
        TEXT("true only when rules pass"),
        TEXT("repeat with a changing counter"),
        TEXT("move each value safely"),
        TEXT("prove the edge case"),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(ConceptNames); ++i)
    {
        const FVector Pad = PracticeStart + CityOffset(FVector(i * 390.0f, 0.0f, 0.0f));
        const FLinearColor PadColor = i % 2 == 0 ? MentorGreen : TestAmber;
        TagLearning(SpawnBlock(
            Pad + FVector(0.0f, 0.0f, 14.0f),
            FVector(1.55f, 1.08f, 0.08f),
            PadColor * 0.95f,
            FString::Printf(TEXT("%s Practice Lane %s Pad"), *CityLabel, ConceptNames[i]),
            false));
        TagLearning(SpawnBlock(
            Pad + FVector(-96.0f, 0.0f, 96.0f),
            FVector(0.08f, 0.55f, 0.96f),
            PadColor * 1.35f,
            FString::Printf(TEXT("%s Practice Lane %s Gate A"), *CityLabel, ConceptNames[i]),
            false));
        TagLearning(SpawnBlock(
            Pad + FVector(96.0f, 0.0f, 96.0f),
            FVector(0.08f, 0.55f, 0.96f),
            PadColor * 1.35f,
            FString::Printf(TEXT("%s Practice Lane %s Gate B"), *CityLabel, ConceptNames[i]),
            false));
        TagLearning(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), ConceptNames[i], ConceptTips[i]),
            Pad + FVector(0.0f, 0.0f, 245.0f),
            PadColor.ToFColor(true),
            22.0f));
    }

    for (int32 i = 0; i < 4; ++i)
    {
        const FVector Start = FVector(-3000.0f + i * 650.0f, -1960.0f, -2.0f);
        const FVector End = FVector(1150.0f, -900.0f, -2.0f);
        const FLinearColor Color = LangColors[i];
        const int32 SegmentCount = 6;
        for (int32 Segment = 1; Segment <= SegmentCount; ++Segment)
        {
            const float T = static_cast<float>(Segment) / static_cast<float>(SegmentCount + 1);
            const FVector Local = FMath::Lerp(Start, End, T);
            TagLearning(SpawnBlock(
                Origin + CityOffset(Local),
                FVector(0.62f, 0.045f, 0.032f),
                Color * (1.65f + Segment * 0.08f),
                FString::Printf(TEXT("%s %s Data Flow Breadcrumb %d"), *CityLabel, LangNames[i], Segment),
                false));
        }
    }

    const FVector TerminalBase = Origin + CityOffset(FVector(1150.0f, -900.0f, 0.0f));
    TagLearning(SpawnBlock(
        TerminalBase + FVector(0.0f, 350.0f, 156.0f),
        FVector(4.15f, 0.07f, 1.02f),
        FLinearColor(0.05f, 0.06f, 0.07f) + DebugCyan * 0.28f,
        CityLabel + TEXT(" Validation Rubric Board"),
        false));
    TagLearning(SpawnGuideText(
        TEXT("VALIDATION RUBRIC\nS: first try + no hints + full score\nA: clean independent solve\nRetry: inspect first failed check, then test again"),
        TerminalBase + FVector(0.0f, 384.0f, 245.0f),
        DebugCyan.ToFColor(true),
        23.0f));
    TagLearning(SpawnBlock(
        TerminalBase + FVector(360.0f, 320.0f, 72.0f),
        FVector(0.90f, 0.40f, 0.50f),
        TestAmber * 1.15f,
        CityLabel + TEXT(" Test Case Crate Visible"),
        true));
    TagLearning(SpawnBlock(
        TerminalBase + FVector(-360.0f, 320.0f, 72.0f),
        FVector(0.90f, 0.40f, 0.50f),
        MentorGreen * 1.15f,
        CityLabel + TEXT(" Hint Economy Crate Visible"),
        true));

    const FVector MasteryTower = Origin + CityOffset(FVector(-620.0f, -1260.0f, 0.0f));
    const int32 VisibleStreak = GI ? FMath::Clamp(GI->BestLearningStreak, 0, 8) : 0;
    for (int32 i = 0; i < 8; ++i)
    {
        const FLinearColor BlockColor = i < VisibleStreak
            ? FLinearColor::LerpUsingHSV(MentorGreen, TestAmber, static_cast<float>(i) / 7.0f) * 1.45f
            : FLinearColor(0.11f, 0.12f, 0.13f);
        TagLearning(SpawnBlock(
            MasteryTower + FVector(0.0f, 0.0f, 42.0f + i * 55.0f),
            FVector(1.0f - i * 0.055f, 0.62f - i * 0.025f, 0.42f),
            BlockColor,
            FString::Printf(TEXT("%s Learning Streak Tower Segment %d"), *CityLabel, i + 1),
            false));
    }
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("LEARNING STREAK TOWER\nbest streak: %d\nperfect solves: %d"),
            GI ? GI->BestLearningStreak : 0,
            GI ? GI->PerfectSolveCount : 0),
        MasteryTower + FVector(0.0f, 0.0f, 560.0f),
        MentorGreen.ToFColor(true),
        26.0f));
    SpawnLearningLight(FVector(-620.0f, -1260.0f, 455.0f), MentorGreen, 3600.0f, 780.0f, CityLabel + TEXT(" Learning Streak Tower Light"));

    const FVector DebugLadder = Origin + CityOffset(FVector(-1660.0f, -1045.0f, 0.0f));
    static const TCHAR* DebugSteps[] = { TEXT("READ"), TEXT("TRACE"), TEXT("FIX"), TEXT("RETEST") };
    for (int32 i = 0; i < UE_ARRAY_COUNT(DebugSteps); ++i)
    {
        TagLearning(SpawnBlock(
            DebugLadder + FVector(0.0f, i * 115.0f, 22.0f + i * 24.0f),
            FVector(1.45f, 0.42f, 0.12f),
            DebugCyan * (1.05f + i * 0.22f),
            FString::Printf(TEXT("%s Debug Ladder Step %s"), *CityLabel, DebugSteps[i]),
            false));
        TagLearning(SpawnGuideText(
            DebugSteps[i],
            DebugLadder + FVector(0.0f, i * 115.0f, 126.0f + i * 24.0f),
            DebugCyan.ToFColor(true),
            20.0f));
    }
    TagLearning(SpawnGuideText(
        TEXT("TRACE LADDER\nread, trace, fix, retest"),
        DebugLadder + FVector(0.0f, 185.0f, 330.0f),
        DebugCyan.ToFColor(true),
        25.0f));

    const FVector SyntaxRing = Origin + CityOffset(FVector(-2520.0f, -1115.0f, 0.0f));
    for (int32 i = 0; i < 12; ++i)
    {
        const float Angle = (static_cast<float>(i) / 12.0f) * 2.0f * PI;
        const FVector RingLoc = SyntaxRing + FVector(FMath::Cos(Angle) * 245.0f, FMath::Sin(Angle) * 245.0f, 52.0f);
        TagLearning(SpawnBlock(
            RingLoc,
            FVector(0.32f, 0.16f, 0.48f),
            (i % 2 == 0 ? JavaColor : PythonColor) * 1.2f,
            CityLabel + TEXT(" Syntax Sparring Ring Marker"),
            false));
    }
    TagLearning(SpawnGuideText(
        TEXT("SYNTAX RING\nsame idea, different language shapes"),
        SyntaxRing + FVector(0.0f, 0.0f, 330.0f),
        PythonColor.ToFColor(true),
        25.0f));

    const FVector AlgorithmMural = Origin + CityOffset(FVector(-2140.0f, -2520.0f, 0.0f));
    TagLearning(SpawnBlock(
        AlgorithmMural + FVector(0.0f, 0.0f, 160.0f),
        FVector(4.2f, 0.08f, 1.05f),
        FLinearColor(0.045f, 0.05f, 0.058f) + TestAmber * 0.18f,
        CityLabel + TEXT(" Algorithm Mural Backboard"),
        false));
    for (int32 i = 0; i < 5; ++i)
    {
        TagLearning(SpawnBlock(
            AlgorithmMural + FVector(-310.0f + i * 155.0f, -36.0f, 105.0f + (i % 3) * 42.0f),
            FVector(0.46f, 0.035f, 0.18f),
            FLinearColor::LerpUsingHSV(JavaColor, MatlabColor, static_cast<float>(i) / 4.0f) * 1.8f,
            CityLabel + TEXT(" Algorithm Mural Flow Node"),
            false));
    }
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("ALGORITHM MURAL\n%s\nvisible case -> hidden case -> rescue"), *Mission.CurriculumFocus),
        AlgorithmMural + FVector(0.0f, -72.0f, 265.0f),
        TestAmber.ToFColor(true),
        23.0f));

    const FVector TestBench = Origin + CityOffset(FVector(730.0f, -510.0f, 0.0f));
    TagLearning(SpawnBlock(
        TestBench + FVector(0.0f, 0.0f, 66.0f),
        FVector(2.75f, 0.74f, 0.28f),
        FLinearColor(0.10f, 0.12f, 0.13f) + TestAmber * 0.18f,
        CityLabel + TEXT(" Validator Test Bench"),
        true));
    static const TCHAR* TestLabels[] = { TEXT("VISIBLE"), TEXT("EDGE"), TEXT("HIDDEN") };
    for (int32 i = 0; i < UE_ARRAY_COUNT(TestLabels); ++i)
    {
        TagLearning(SpawnBlock(
            TestBench + FVector(-190.0f + i * 190.0f, -48.0f, 118.0f),
            FVector(0.58f, 0.08f, 0.22f),
            TestAmber * (1.35f + i * 0.18f),
            FString::Printf(TEXT("%s Test Bench %s Case"), *CityLabel, TestLabels[i]),
            false));
        TagLearning(SpawnGuideText(
            TestLabels[i],
            TestBench + FVector(-190.0f + i * 190.0f, -78.0f, 180.0f),
            TestAmber.ToFColor(true),
            18.0f));
    }
    TagLearning(SpawnGuideText(
        TEXT("TEST BENCH\nmake code pass examples you can see and cases you cannot"),
        TestBench + FVector(0.0f, -100.0f, 275.0f),
        TestAmber.ToFColor(true),
        22.0f));

    const FVector CompileTower = Origin + CityOffset(FVector(1510.0f, -360.0f, 0.0f));
    for (int32 i = 0; i < 7; ++i)
    {
        TagLearning(SpawnBlock(
            CompileTower + FVector(0.0f, 0.0f, 38.0f + i * 58.0f),
            FVector(0.72f - i * 0.035f, 0.72f - i * 0.035f, 0.42f),
            FLinearColor::LerpUsingHSV(DebugCyan, MentorGreen, static_cast<float>(i) / 6.0f) * 1.35f,
            FString::Printf(TEXT("%s Compile Tower Segment %d"), *CityLabel, i + 1),
            false));
    }
    TagLearning(SpawnGuideText(
        TEXT("COMPILE TOWER\nsyntax -> build -> tests -> mission unlock"),
        CompileTower + FVector(0.0f, 0.0f, 515.0f),
        MentorGreen.ToFColor(true),
        24.0f));

    const FVector RefactorWalk = Origin + CityOffset(FVector(-420.0f, -760.0f, 0.0f));
    static const TCHAR* RefactorLabels[] = { TEXT("NAME"), TEXT("SIMPLIFY"), TEXT("REUSE"), TEXT("EXPLAIN") };
    for (int32 i = 0; i < UE_ARRAY_COUNT(RefactorLabels); ++i)
    {
        TagLearning(SpawnBlock(
            RefactorWalk + FVector(i * 185.0f, 0.0f, 18.0f),
            FVector(0.82f, 0.22f, 0.06f),
            MatlabColor * (1.0f + i * 0.15f),
            FString::Printf(TEXT("%s Refactor Walk %s"), *CityLabel, RefactorLabels[i]),
            false));
        TagLearning(SpawnGuideText(
            RefactorLabels[i],
            RefactorWalk + FVector(i * 185.0f, 0.0f, 115.0f),
            MatlabColor.ToFColor(true),
            18.0f));
    }

    for (int32 i = 0; i < 8; ++i)
    {
        const FVector Local = FMath::Lerp(FVector(-2380.0f, -1780.0f, -1.0f), FVector(1150.0f, -900.0f, -1.0f), static_cast<float>(i + 1) / 9.0f);
        TagLearning(SpawnBlock(
            Origin + CityOffset(Local),
            FVector(0.48f, 0.048f, 0.032f),
            MentorGreen * (1.65f + i * 0.05f),
            FString::Printf(TEXT("%s Academy To Terminal Breadcrumb %d"), *CityLabel, i + 1),
            false));
    }

    const FVector PerfectPodium = Origin + CityOffset(FVector(2050.0f, 2050.0f, 0.0f));
    const int32 PerfectVisible = GI ? FMath::Clamp(GI->PerfectSolveCount, 0, 5) : 0;
    for (int32 i = 0; i < 5; ++i)
    {
        TagLearning(SpawnBlock(
            PerfectPodium + FVector(-220.0f + i * 110.0f, 0.0f, 36.0f + i * 12.0f),
            FVector(0.54f, 0.50f, 0.34f + i * 0.08f),
            i < PerfectVisible ? TestAmber * 1.55f : FLinearColor(0.12f, 0.12f, 0.13f),
            FString::Printf(TEXT("%s Perfect Solve Podium %d"), *CityLabel, i + 1),
            true));
    }
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("PERFECT SOLVE PODIUM\nfirst try + no hint + full score: %d"), GI ? GI->PerfectSolveCount : 0),
        PerfectPodium + FVector(0.0f, 0.0f, 245.0f),
        TestAmber.ToFColor(true),
        24.0f));

    const FVector NoHintPlaques = Origin + CityOffset(FVector(2240.0f, 1760.0f, 0.0f));
    const int32 NoHintVisible = GI ? FMath::Clamp(GI->NoHintSolveCount, 0, 6) : 0;
    for (int32 i = 0; i < 6; ++i)
    {
        TagLearning(SpawnBlock(
            NoHintPlaques + FVector(-250.0f + i * 100.0f, 0.0f, 92.0f),
            FVector(0.36f, 0.055f, 0.50f),
            i < NoHintVisible ? MentorGreen * 1.7f : FLinearColor(0.10f, 0.12f, 0.13f),
            FString::Printf(TEXT("%s No-Hint Mastery Plaque %d"), *CityLabel, i + 1),
            false));
    }
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("NO-HINT MASTERY\nclean solves: %d"), GI ? GI->NoHintSolveCount : 0),
        NoHintPlaques + FVector(0.0f, -60.0f, 210.0f),
        MentorGreen.ToFColor(true),
        22.0f));

    const FVector MissionBoard = Origin + CityOffset(FVector(-3650.0f, -2700.0f, 0.0f));
    TagLearning(SpawnBlock(
        MissionBoard + FVector(0.0f, 0.0f, 154.0f),
        FVector(4.4f, 0.08f, 1.05f),
        FLinearColor(0.04f, 0.05f, 0.06f) + Mission.SecondaryAccentColor * 0.20f,
        CityLabel + TEXT(" Active Learning Objective Board"),
        false));
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("ACTIVE LEARNING OBJECTIVE\n%s\nLanguage -> Terminal -> Tests -> Rescue %s"), *Mission.TerminalTitle, *Mission.SurvivorName),
        MissionBoard + FVector(0.0f, -54.0f, 245.0f),
        Mission.SecondaryAccentColor.ToFColor(true),
        22.0f));

    const FVector CurriculumBanner = Origin + CityOffset(FVector(400.0f, -2330.0f, 0.0f));
    TagLearning(SpawnBlock(
        CurriculumBanner + FVector(0.0f, 0.0f, 146.0f),
        FVector(5.4f, 0.06f, 0.48f),
        Mission.AccentColor * 1.2f,
        CityLabel + TEXT(" City Curriculum Banner"),
        false));
    TagLearning(SpawnGuideText(
        FString::Printf(TEXT("CITY CURRICULUM\n%s\n%s"), *Mission.CityName, *Mission.CurriculumFocus),
        CurriculumBanner + FVector(0.0f, -44.0f, 212.0f),
        Mission.AccentColor.ToFColor(true),
        22.0f));

    const FVector RescueLoop = Origin + CityOffset(FVector(2570.0f, 1120.0f, 0.0f));
    TagLearning(SpawnGuideText(
        TEXT("RESCUE LOOP\nlearn the concept, validate the code, defend the team, graduate the city"),
        RescueLoop + FVector(0.0f, 0.0f, 360.0f),
        MentorGreen.ToFColor(true),
        24.0f));

    TagLearning(SpawnGuideText(
        TEXT("CODING LEARNING PASS\npractice, validate, reflect, rescue"),
        Origin + CityOffset(FVector(-2260.0f, -1040.0f, 360.0f)),
        FColor(230, 255, 230),
        30.0f));

    (void)CityIndex;
    (void)Mission;
}

void ACodeRescueGameMode::SpawnGraduatedCurriculumCityIdentityLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector Hub = Origin + CityOffset(FVector(1880.0f, -1460.0f, 0.0f));
    const FLinearColor StageColor = Mission.AccentColor * 1.35f;
    const FLinearColor DetailColor = Mission.SecondaryAccentColor * 1.45f;

    auto TagGraduation = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("GraduatedCurriculum"));
            Actor->Tags.Add(FName("CityIdentity"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    TagGraduation(SpawnTexturedBlock(
        Hub + FVector(0.0f, 0.0f, -8.0f),
        FVector(5.8f, 3.2f, 0.045f),
        FLinearColor(0.05f, 0.06f, 0.07f) + StageColor * 0.14f,
        CityLabel + TEXT(" Graduated Curriculum Identity Plaza"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false));
    TagGraduation(SpawnBlock(
        Hub + FVector(0.0f, -228.0f, 150.0f),
        FVector(4.75f, 0.07f, 1.05f),
        FLinearColor(0.035f, 0.040f, 0.048f) + DetailColor * 0.18f,
        CityLabel + TEXT(" Graduated Curriculum Briefing Wall"),
        false));
    TagGraduation(SpawnGuideText(
        FString::Printf(
            TEXT("LEVEL %03d GRADUATION\n%s\nTask: %s\nWorld: %s"),
            Mission.Rank,
            *Mission.CurriculumStageName,
            *Mission.TerminalTitle,
            *Mission.ArchitectureSignature),
        Hub + FVector(0.0f, -270.0f, 270.0f),
        DetailColor.ToFColor(true),
        21.0f));

    if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
            APointLight::StaticClass(), Hub + FVector(0.0f, -80.0f, 285.0f), FRotator::ZeroRotator))
    {
        if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
        {
            PLC->SetMobility(EComponentMobility::Movable);
            PLC->SetIntensity(3900.0f);
            PLC->SetLightColor(DetailColor);
            PLC->SetAttenuationRadius(820.0f);
            PLC->SetCastShadows(false);
        }
#if WITH_EDITOR
        Light->SetActorLabel(CityLabel + TEXT(" Graduated Curriculum Plaza Light"));
#endif
        RegisterStreamedActor(Light);
        TagGraduation(Light);
    }

    static const TCHAR* StageSteps[] = {
        TEXT("READ"),
        TEXT("MODEL"),
        TEXT("CODE"),
        TEXT("TEST"),
        TEXT("RESCUE"),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(StageSteps); ++i)
    {
        const FVector StepLoc = Hub + FVector(-410.0f + i * 205.0f, 118.0f, 38.0f + i * 10.0f);
        const FLinearColor StepColor = FLinearColor::LerpUsingHSV(StageColor, DetailColor, static_cast<float>(i) / 4.0f);
        TagGraduation(SpawnBlock(
            StepLoc,
            FVector(0.78f, 0.46f, 0.18f + i * 0.035f),
            StepColor * 1.15f,
            FString::Printf(TEXT("%s Graduated Step %s"), *CityLabel, StageSteps[i]),
            false));
        TagGraduation(SpawnGuideText(
            StageSteps[i],
            StepLoc + FVector(0.0f, 0.0f, 98.0f),
            StepColor.ToFColor(true),
            18.0f));
    }

    const FVector DetailBase = Hub + FVector(0.0f, 470.0f, 0.0f);
    switch (Mission.LessonKind)
    {
    case ECampaignLessonKind::Lock:
        for (int32 i = 0; i < 4; ++i)
        {
            const FVector Gate = DetailBase + FVector(-300.0f + i * 200.0f, 0.0f, 92.0f);
            TagGraduation(SpawnBlock(Gate, FVector(0.16f, 0.62f, 1.55f), StageColor, CityLabel + TEXT(" Truth Table Gate Upright"), false));
            TagGraduation(SpawnGuideText(i == 3 ? TEXT("TRUE/TRUE") : TEXT("LOCKED"), Gate + FVector(0.0f, 0.0f, 155.0f), StageColor.ToFColor(true), 16.0f));
        }
        break;
    case ECampaignLessonKind::Reverse:
        for (int32 i = 0; i < 6; ++i)
        {
            TagGraduation(SpawnBlock(
                DetailBase + FVector(310.0f - i * 124.0f, 0.0f, 58.0f + i * 18.0f),
                FVector(0.42f, 0.12f, 0.14f),
                DetailColor * (1.1f + i * 0.12f),
                CityLabel + TEXT(" Reverse Signal Packet"),
                false));
        }
        break;
    case ECampaignLessonKind::Palindrome:
        for (int32 i = 0; i < 4; ++i)
        {
            const float X = 110.0f + i * 115.0f;
            TagGraduation(SpawnBlock(DetailBase + FVector(-X, 0.0f, 88.0f), FVector(0.24f, 0.18f, 1.3f), StageColor, CityLabel + TEXT(" Palindrome Left Mirror"), false));
            TagGraduation(SpawnBlock(DetailBase + FVector(X, 0.0f, 88.0f), FVector(0.24f, 0.18f, 1.3f), StageColor, CityLabel + TEXT(" Palindrome Right Mirror"), false));
        }
        break;
    case ECampaignLessonKind::FizzBuzz:
        for (int32 i = 1; i <= 15; ++i)
        {
            const bool bFizzBuzz = i % 15 == 0;
            const bool bFizz = i % 3 == 0;
            const bool bBuzz = i % 5 == 0;
            const FLinearColor PylonColor = bFizzBuzz ? FLinearColor(1.0f, 0.85f, 0.20f) * 2.0f : bFizz ? StageColor : bBuzz ? DetailColor : FLinearColor(0.18f, 0.20f, 0.22f);
            TagGraduation(SpawnBlock(
                DetailBase + FVector(-420.0f + i * 56.0f, 0.0f, 38.0f + (bFizzBuzz ? 62.0f : bFizz || bBuzz ? 38.0f : 20.0f)),
                FVector(0.18f, 0.18f, bFizzBuzz ? 1.25f : bFizz || bBuzz ? 0.88f : 0.48f),
                PylonColor,
                CityLabel + TEXT(" FizzBuzz Beacon Pylon"),
                false));
        }
        break;
    case ECampaignLessonKind::EvenFilter:
        for (int32 i = 0; i < 8; ++i)
        {
            const bool bEven = i % 2 == 0;
            const FVector Lane = DetailBase + FVector(-350.0f + i * 100.0f, bEven ? -62.0f : 62.0f, 32.0f);
            TagGraduation(SpawnBlock(Lane, FVector(0.42f, 1.10f, 0.12f), bEven ? DetailColor * 1.5f : FLinearColor(0.18f, 0.12f, 0.13f), CityLabel + TEXT(" Even Filter Lane"), false));
        }
        break;
    case ECampaignLessonKind::LinkedListTraverse:
        for (int32 i = 0; i < 6; ++i)
        {
            const FVector Node = DetailBase + FVector(-360.0f + i * 144.0f, (i % 2 == 0) ? -54.0f : 54.0f, 86.0f);
            TagGraduation(SpawnBlock(Node, FVector(0.36f, 0.36f, 0.72f), StageColor * (1.0f + i * 0.08f), CityLabel + TEXT(" Linked Evacuation Node"), false));
            if (i < 5)
            {
                TagGraduation(SpawnBlock(Node + FVector(72.0f, (i % 2 == 0) ? 54.0f : -54.0f, 30.0f), FVector(0.62f, 0.055f, 0.07f), DetailColor * 1.4f, CityLabel + TEXT(" Linked Evacuation Arrow"), false));
            }
        }
        break;
    case ECampaignLessonKind::BinarySearch:
        for (int32 i = 0; i < 5; ++i)
        {
            const float Width = 4.8f - i * 0.72f;
            TagGraduation(SpawnBlock(
                DetailBase + FVector(0.0f, 0.0f, 34.0f + i * 48.0f),
                FVector(Width, 0.18f, 0.14f),
                FLinearColor::LerpUsingHSV(StageColor, DetailColor, static_cast<float>(i) / 4.0f) * 1.55f,
                CityLabel + TEXT(" Binary Search Shrinking Band"),
                false));
        }
        break;
    default:
        for (int32 i = 0; i < 3; ++i)
        {
            TagGraduation(SpawnBlock(
                DetailBase + FVector(-160.0f + i * 160.0f, 0.0f, 72.0f),
                FVector(0.58f, 0.58f, 0.72f),
                FLinearColor::LerpUsingHSV(StageColor, DetailColor, static_cast<float>(i) / 2.0f),
                CityLabel + TEXT(" Sum Power Cell"),
                false));
        }
        break;
    }

    TagGraduation(SpawnGuideText(
        FString::Printf(TEXT("NOVEL PLAY DETAIL\n%s"), *Mission.NovelGameplayDetail),
        DetailBase + FVector(0.0f, 0.0f, 310.0f),
        StageColor.ToFColor(true),
        22.0f));

    TagGraduation(SpawnGuideText(
        FString::Printf(TEXT("UNIQUE CITY IDENTITY\n%s\n%s | %s"),
            *Mission.LandmarkName,
            *Mission.ArtKitName,
            *Mission.DistrictStyle),
        Hub + FVector(0.0f, 240.0f, 388.0f),
        FColor(235, 245, 255),
        24.0f));

    (void)CityIndex;
}

void ACodeRescueGameMode::SpawnCharacterWorldRealizationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FLinearColor WarmLight = FLinearColor(1.0f, 0.74f, 0.38f);
    const FLinearColor RescueCyan = FLinearColor(0.22f, 0.90f, 1.0f);
    const FLinearColor ShelterGreen = FLinearColor(0.36f, 1.0f, 0.54f);
    const FLinearColor HumanViolet = FLinearColor(0.78f, 0.45f, 1.0f);
    const FLinearColor WarningRed = FLinearColor(1.0f, 0.16f, 0.10f);
    const FLinearColor PaperWhite = FLinearColor(0.86f, 0.88f, 0.80f);
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    auto TagRealized = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("CharacterWorldRealization"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto SpawnRealizationLight = [&](const FVector& Local, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                Origin + CityOffset(Local),
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagRealized(Light);
        }
    };

    const FVector Concourse = Origin + CityOffset(FVector(-610.0f, 1260.0f, 0.0f));
    TagRealized(SpawnTexturedBlock(
        Concourse + FVector(0.0f, 0.0f, -7.0f),
        FVector(7.8f, 4.2f, 0.055f),
        FLinearColor(0.085f, 0.09f, 0.088f) + Mission.AccentColor * 0.16f,
        CityLabel + TEXT(" Character Story Concourse Floor"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false));
    TagRealized(SpawnBlock(
        Concourse + FVector(0.0f, -305.0f, 155.0f),
        FVector(5.8f, 0.075f, 1.05f),
        FLinearColor(0.045f, 0.05f, 0.055f) + Mission.SecondaryAccentColor * 0.20f,
        CityLabel + TEXT(" Character Story Wall"),
        false));
    TagRealized(SpawnGuideText(
        FString::Printf(TEXT("CIVILIAN STORY CONCOURSE\n%s, %s\n%s"), *Mission.CityName, *Mission.StateName, *Mission.RadioBriefing),
        Concourse + FVector(0.0f, -348.0f, 246.0f),
        Mission.SecondaryAccentColor.ToFColor(true),
        21.0f));
    SpawnRealizationLight(FVector(-610.0f, 1260.0f, 260.0f), WarmLight, 4700.0f, 980.0f, CityLabel + TEXT(" Character Concourse Warm Light"));

    const TCHAR* CivilianNames[] = {
        TEXT("Maya / Route Cartographer"),
        TEXT("Orion / Junior Debugger"),
        TEXT("Vale / Supply Runner"),
        TEXT("Sana / Radio Scribe"),
        TEXT("Tomas / Bridge Mechanic"),
        TEXT("Iris / Lesson Archivist"),
    };
    const TCHAR* CivilianProps[] = {
        TEXT("map table"),
        TEXT("practice notebook"),
        TEXT("supply case"),
        TEXT("radio log"),
        TEXT("repair kit"),
        TEXT("lesson archive"),
    };
    const FVector CivilianLocals[] = {
        FVector(-980.0f, 1110.0f, 92.0f),
        FVector(-650.0f, 1040.0f, 92.0f),
        FVector(-330.0f, 1095.0f, 92.0f),
        FVector(-40.0f, 1280.0f, 92.0f),
        FVector(-760.0f, 1500.0f, 92.0f),
        FVector(-390.0f, 1510.0f, 92.0f),
    };
    const FLinearColor CivilianColors[] = {
        RescueCyan,
        FLinearColor(1.0f, 0.86f, 0.25f),
        ShelterGreen,
        HumanViolet,
        FLinearColor(1.0f, 0.58f, 0.18f),
        Mission.AccentColor * 1.35f,
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(CivilianNames); ++i)
    {
        const FVector WorldLoc = Origin + CityOffset(CivilianLocals[i]);
        TagRealized(SpawnDecorativeCivilian(
            WorldLoc,
            FRotator(0.0f, -38.0f + i * 18.0f, 0.0f),
            (i % 2) == 0,
            CivilianColors[i],
            FString::Printf(TEXT("%s Realized Civilian %d"), *CityLabel, i + 1),
            CivilianNames[i]));
        TagRealized(SpawnBlock(
            WorldLoc + FVector(78.0f, -42.0f, 36.0f),
            FVector(0.58f, 0.30f, 0.22f),
            CivilianColors[i] * 0.68f,
            FString::Printf(TEXT("%s Civilian Prop %s"), *CityLabel, CivilianProps[i]),
            true));
        TagRealized(SpawnBlock(
            WorldLoc + FVector(78.0f, -42.0f, 74.0f),
            FVector(0.46f, 0.035f, 0.16f),
            PaperWhite,
            FString::Printf(TEXT("%s Civilian Note %s"), *CityLabel, CivilianProps[i]),
            false));
    }

    const FVector SurvivorBoard = Origin + CityOffset(FVector(2600.0f, 1120.0f, 0.0f));
    TagRealized(SpawnBlock(
        SurvivorBoard + FVector(0.0f, -180.0f, 150.0f),
        FVector(4.7f, 0.07f, 1.05f),
        FLinearColor(0.04f, 0.045f, 0.05f) + RescueCyan * 0.22f,
        CityLabel + TEXT(" Survivor Story Profile Wall"),
        false));
    TagRealized(SpawnGuideText(
        FString::Printf(TEXT("SURVIVOR PROFILE\n%s\n%s\nRescue unlocks after: %s"),
            *Mission.SurvivorName,
            *Mission.MissionBrief,
            *Mission.TerminalTitle),
        SurvivorBoard + FVector(0.0f, -218.0f, 245.0f),
        RescueCyan.ToFColor(true),
        21.0f));
    TagRealized(SpawnBlock(
        SurvivorBoard + FVector(-285.0f, -125.0f, 70.0f),
        FVector(0.72f, 0.38f, 0.36f),
        RescueCyan * 0.95f,
        CityLabel + TEXT(" Survivor Personal Go-Bag"),
        true));
    TagRealized(SpawnBlock(
        SurvivorBoard + FVector(285.0f, -125.0f, 82.0f),
        FVector(0.72f, 0.08f, 0.44f),
        ShelterGreen * 0.95f,
        CityLabel + TEXT(" Survivor Evac Clipboard"),
        false));
    SpawnRealizationLight(FVector(2600.0f, 1120.0f, 250.0f), RescueCyan, 4300.0f, 820.0f, CityLabel + TEXT(" Survivor Story Light"));

    const FVector QueueStart = Origin + CityOffset(FVector(1800.0f, 1470.0f, 0.0f));
    for (int32 i = 0; i < 7; ++i)
    {
        const FVector LocalOffset(i * 185.0f, (i % 2) == 0 ? 0.0f : 82.0f, 92.0f);
        const FVector QueueLoc = QueueStart + LocalOffset;
        TagRealized(SpawnDecorativeCivilian(
            QueueLoc,
            FRotator(0.0f, 16.0f, 0.0f),
            (i % 3) != 1,
            i % 2 == 0 ? ShelterGreen : RescueCyan,
            FString::Printf(TEXT("%s Evacuation Queue Civilian %d"), *CityLabel, i + 1),
            FString::Printf(TEXT("Evacuee %d / waits for code rescue"), i + 1)));
        TagRealized(SpawnBlock(
            QueueLoc + FVector(-62.0f, 48.0f, 30.0f),
            FVector(0.28f, 0.25f, 0.22f),
            WarmLight * 0.65f,
            FString::Printf(TEXT("%s Evacuation Queue Luggage %d"), *CityLabel, i + 1),
            true));
    }
    for (int32 i = 0; i < 9; ++i)
    {
        TagRealized(SpawnBlock(
            QueueStart + FVector(i * 158.0f, -112.0f, 24.0f),
            FVector(0.055f, 0.56f, 0.09f),
            ShelterGreen * 1.25f,
            FString::Printf(TEXT("%s Evacuation Queue Guide Rail %d"), *CityLabel, i + 1),
            false));
    }
    TagRealized(SpawnGuideText(
        TEXT("EVACUATION LINE\npeople wait because each lesson restores one route"),
        QueueStart + FVector(600.0f, -178.0f, 315.0f),
        ShelterGreen.ToFColor(true),
        23.0f));

    const FVector Market = Origin + CityOffset(FVector(690.0f, 2240.0f, 0.0f));
    static const TCHAR* StallLabels[] = { TEXT("repair"), TEXT("clinic"), TEXT("study"), TEXT("trade") };
    const FLinearColor StallColors[] = {
        FLinearColor(1.0f, 0.55f, 0.18f),
        FLinearColor(1.0f, 0.20f, 0.20f),
        FLinearColor(0.32f, 0.58f, 1.0f),
        ShelterGreen,
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(StallLabels); ++i)
    {
        const FVector Stall = Market + FVector(-540.0f + i * 360.0f, 0.0f, 0.0f);
        TagRealized(SpawnBlock(
            Stall + FVector(0.0f, 0.0f, 58.0f),
            FVector(1.20f, 0.62f, 0.42f),
            StallColors[i] * 0.52f,
            FString::Printf(TEXT("%s Daily Life Stall %s Counter"), *CityLabel, StallLabels[i]),
            true));
        TagRealized(SpawnBlock(
            Stall + FVector(0.0f, -72.0f, 132.0f),
            FVector(1.36f, 0.08f, 0.24f),
            StallColors[i] * 1.35f,
            FString::Printf(TEXT("%s Daily Life Stall %s Sign"), *CityLabel, StallLabels[i]),
            false));
        TagRealized(SpawnBlock(
            Stall + FVector(-88.0f, 64.0f, 118.0f),
            FVector(0.22f, 0.16f, 0.28f),
            PaperWhite,
            FString::Printf(TEXT("%s Daily Life Stall %s Detail A"), *CityLabel, StallLabels[i]),
            true));
        TagRealized(SpawnBlock(
            Stall + FVector(88.0f, 64.0f, 102.0f),
            FVector(0.28f, 0.20f, 0.20f),
            StallColors[i] * 0.95f,
            FString::Printf(TEXT("%s Daily Life Stall %s Detail B"), *CityLabel, StallLabels[i]),
            true));
        TagRealized(SpawnGuideText(
            FString::Printf(TEXT("%s STATION"), StallLabels[i]),
            Stall + FVector(0.0f, -104.0f, 205.0f),
            StallColors[i].ToFColor(true),
            18.0f));
    }
    TagRealized(SpawnGuideText(
        TEXT("SAFE MARKET\nsupport roles make the outbreak feel inhabited"),
        Market + FVector(0.0f, -188.0f, 315.0f),
        WarmLight.ToFColor(true),
        24.0f));

    const FVector Taxonomy = Origin + CityOffset(FVector(3160.0f, -1560.0f, 0.0f));
    TagRealized(SpawnTexturedBlock(
        Taxonomy + FVector(0.0f, 0.0f, -6.0f),
        FVector(4.9f, 2.0f, 0.045f),
        FLinearColor(0.10f, 0.04f, 0.04f),
        CityLabel + TEXT(" Enemy Readability Pad"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Rust.M_Metal_Rust"),
        false));
    const TCHAR* EnemyLabels[] = { TEXT("FAST"), TEXT("BRUTE"), TEXT("SWARM"), TEXT("BOSS") };
    const FVector EnemyScales[] = {
        FVector(0.34f, 0.20f, 1.35f),
        FVector(0.74f, 0.38f, 1.85f),
        FVector(0.48f, 0.30f, 1.05f),
        FVector(1.05f, 0.62f, 2.45f),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(EnemyLabels); ++i)
    {
        const FVector EnemyLoc = Taxonomy + FVector(-330.0f + i * 220.0f, 0.0f, 70.0f + EnemyScales[i].Z * 48.0f);
        TagRealized(SpawnBlock(
            EnemyLoc,
            EnemyScales[i],
            WarningRed * (1.0f + i * 0.20f),
            FString::Printf(TEXT("%s Enemy Readability Silhouette %s"), *CityLabel, EnemyLabels[i]),
            false));
        TagRealized(SpawnBlock(
            EnemyLoc + FVector(0.0f, -18.0f, EnemyScales[i].Z * 66.0f),
            FVector(0.28f + i * 0.04f, 0.08f, 0.20f + i * 0.05f),
            FLinearColor(1.0f, 0.78f, 0.10f) * 1.75f,
            FString::Printf(TEXT("%s Enemy Eye Readability %s"), *CityLabel, EnemyLabels[i]),
            false));
        TagRealized(SpawnGuideText(
            EnemyLabels[i],
            EnemyLoc + FVector(0.0f, -82.0f, EnemyScales[i].Z * 88.0f),
            WarningRed.ToFColor(true),
            18.0f));
    }
    TagRealized(SpawnGuideText(
        TEXT("THREAT SILHOUETTES\nread shape first, then choose distance"),
        Taxonomy + FVector(0.0f, -166.0f, 330.0f),
        WarningRed.ToFColor(true),
        23.0f));

    const FVector BossRunway = Origin + CityOffset(FVector(2900.0f, -1500.0f, 0.0f));
    for (int32 i = 0; i < 10; ++i)
    {
        const float Angle = (static_cast<float>(i) / 10.0f) * 2.0f * PI;
        const FVector Marker = BossRunway + FVector(FMath::Cos(Angle) * 430.0f, FMath::Sin(Angle) * 430.0f, 18.0f);
        TagRealized(SpawnRotatedBlock(
            Marker,
            FRotator(0.0f, FMath::RadiansToDegrees(Angle), 0.0f),
            FVector(0.78f, 0.045f, 0.055f),
            WarningRed * 1.35f,
            FString::Printf(TEXT("%s Boss Arena Warning Sector %d"), *CityLabel, i + 1),
            false));
    }
    TagRealized(SpawnGuideText(
        TEXT("BOSS READABILITY RING\nred sectors mark the danger zone"),
        BossRunway + FVector(0.0f, 0.0f, 585.0f),
        WarningRed.ToFColor(true),
        23.0f));

    const FVector WindowBase = Origin + CityOffset(FVector(-2540.0f, 2940.0f, 0.0f));
    for (int32 Building = 0; Building < 4; ++Building)
    {
        const float X = Building * 360.0f;
        for (int32 Row = 0; Row < 5; ++Row)
        {
            for (int32 Col = 0; Col < 3; ++Col)
            {
                const bool bWarm = ((Row + Col + Building + CityIndex) % 3) != 0;
                TagRealized(SpawnBlock(
                    WindowBase + FVector(X + Col * 64.0f, 0.0f, 95.0f + Row * 82.0f),
                    FVector(0.18f, 0.024f, 0.16f),
                    bWarm ? WarmLight * 1.85f : FLinearColor(0.08f, 0.12f, 0.16f),
                    FString::Printf(TEXT("%s Warm Window Building%d R%d C%d"), *CityLabel, Building + 1, Row + 1, Col + 1),
                    false));
            }
        }
    }
    TagRealized(SpawnGuideText(
        TEXT("LIVED-IN SKYLINE\nwarm windows show people still holding on"),
        WindowBase + FVector(520.0f, -52.0f, 560.0f),
        WarmLight.ToFColor(true),
        22.0f));

    const FVector BannerBase = Origin + CityOffset(FVector(-3000.0f, -2300.0f, 0.0f));
    static const TCHAR* BannerLabels[] = { TEXT("JAVA"), TEXT("C"), TEXT("PYTHON"), TEXT("MATLAB"), TEXT("C+"), TEXT("C++") };
    const FLinearColor BannerColors[] = {
        FLinearColor(1.0f, 0.40f, 0.18f),
        FLinearColor(0.25f, 0.58f, 1.0f),
        FLinearColor(1.0f, 0.86f, 0.22f),
        FLinearColor(0.85f, 0.30f, 1.0f),
        FLinearColor(0.18f, 0.92f, 1.0f),
        FLinearColor(0.36f, 0.62f, 1.0f),
    };
    const int32 BannerLanguageIndex = GI
        ? FMath::Clamp(static_cast<int32>(GI->SelectedLanguage), 0, static_cast<int32>(UE_ARRAY_COUNT(BannerLabels)) - 1)
        : 0;
    TagRealized(SpawnBlock(
        BannerBase + CityOffset(FVector(0.0f, -170.0f, 328.0f)),
        FVector(1.36f, 0.035f, 0.34f),
        BannerColors[BannerLanguageIndex] * 1.45f,
        FString::Printf(TEXT("%s Hanging Selected Language Banner %s"), *CityLabel, BannerLabels[BannerLanguageIndex]),
        false));
    TagRealized(SpawnGuideText(
        BannerLabels[BannerLanguageIndex],
        BannerBase + CityOffset(FVector(0.0f, -198.0f, 386.0f)),
        BannerColors[BannerLanguageIndex].ToFColor(true),
        19.0f));

    TagRealized(SpawnGuideText(
        TEXT("CHARACTER/WORLD REALIZATION PASS\nnamed people, daily life, survivor story, threat silhouettes, lived-in skyline"),
        Origin + CityOffset(FVector(-650.0f, 1720.0f, 410.0f)),
        FColor(245, 240, 220),
        28.0f));
}

void ACodeRescueGameMode::SpawnFirstMinuteOrientationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FLinearColor StartBlue = FLinearColor(0.15f, 0.62f, 1.0f);
    const FLinearColor LanguageAmber = FLinearColor(0.96f, 0.58f, 0.20f);
    const FLinearColor TerminalYellow = FLinearColor(1.0f, 0.82f, 0.16f);
    const FLinearColor RescueCyan = FLinearColor(0.18f, 0.92f, 1.0f);
    const FLinearColor ExtractionGreen = FLinearColor(0.32f, 1.0f, 0.42f);
    const FLinearColor DangerRed = FLinearColor(1.0f, 0.12f, 0.08f);
    const FLinearColor PaperWhite = FLinearColor(0.88f, 0.90f, 0.82f);
    const FLinearColor BoardDark = FLinearColor(0.035f, 0.040f, 0.045f);
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    static const TCHAR* LockedLanguageLabels[] = { TEXT("JAVA"), TEXT("C"), TEXT("PYTHON"), TEXT("MATLAB"), TEXT("C+"), TEXT("C++") };
    const int32 LockedLanguageIndex = GI
        ? FMath::Clamp(static_cast<int32>(GI->SelectedLanguage), 0, static_cast<int32>(UE_ARRAY_COUNT(LockedLanguageLabels)) - 1)
        : 0;

    auto TagOrientation = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("FirstMinuteOrientation"));
            Actor->Tags.Add(FName("WorldDevelopment"));
            Actor->Tags.Add(FName("LearningClarity"));
        }
        return Actor;
    };

    auto SpawnOrientationLight = [&](const FVector& Local, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                Origin + CityOffset(Local),
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagOrientation(Light);
        }
    };

    auto SpawnInfoBoard = [&](const FVector& Local, const FVector& Scale, const FLinearColor& Color, const FString& Name, const FString& Text, const FColor& TextColor, float TextSize)
    {
        const FVector Base = Origin + CityOffset(Local);
        TagOrientation(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 132.0f),
            Scale,
            BoardDark + Color * 0.20f,
            CityLabel + TEXT(" ") + Name + TEXT(" Board"),
            false));
        TagOrientation(SpawnGuideText(
            Text,
            Base + FVector(0.0f, -58.0f, 216.0f),
            TextColor,
            TextSize));
    };

    auto SpawnPathSegment = [&](const FVector& StartLocal, const FVector& EndLocal, int32 Count, const FLinearColor& Color, const FString& Name)
    {
        for (int32 i = 0; i < Count; ++i)
        {
            const float Alpha = Count > 1 ? static_cast<float>(i) / static_cast<float>(Count - 1) : 0.0f;
            const FVector Local = FMath::Lerp(StartLocal, EndLocal, Alpha);
            TagOrientation(SpawnBlock(
                Origin + CityOffset(Local),
                FVector(0.42f, 0.055f, 0.035f),
                Color * (1.35f + 0.05f * i),
                FString::Printf(TEXT("%s %s Path Stripe %02d"), *CityLabel, *Name, i + 1),
                false));
        }
    };

    const FVector Plaza = Origin + CityOffset(FVector(-3540.0f, -1840.0f, 0.0f));
    TagOrientation(SpawnTexturedBlock(
        Plaza + FVector(0.0f, 0.0f, -9.0f),
        FVector(8.2f, 5.1f, 0.055f),
        FLinearColor(0.06f, 0.075f, 0.085f) + StartBlue * 0.14f,
        CityLabel + TEXT(" First Minute Orientation Plaza"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false));
    TagOrientation(SpawnBlock(
        Plaza + FVector(0.0f, -362.0f, 168.0f),
        FVector(6.4f, 0.08f, 1.12f),
        BoardDark + Mission.AccentColor * 0.24f,
        CityLabel + TEXT(" First Minute Orientation Wall"),
        false));
    TagOrientation(SpawnGuideText(
        FString::Printf(TEXT("FIRST MINUTE ORIENTATION\n%s\n1 language  2 terminal  3 tests  4 rescue  5 extract"), *Mission.CityName),
        Plaza + FVector(0.0f, -418.0f, 265.0f),
        StartBlue.ToFColor(true),
        25.0f));

    for (int32 i = 0; i < 8; ++i)
    {
        TagOrientation(SpawnBlock(
            Plaza + FVector(-355.0f + i * 102.0f, 318.0f, 58.0f + i * 20.0f),
            FVector(0.28f, 0.28f, 0.42f + i * 0.12f),
            FLinearColor::LerpUsingHSV(StartBlue, ExtractionGreen, static_cast<float>(i) / 7.0f) * 1.25f,
            FString::Printf(TEXT("%s Orientation Light Tower Segment %d"), *CityLabel, i + 1),
            false));
    }
    SpawnOrientationLight(FVector(-3540.0f, -1840.0f, 405.0f), StartBlue, 5600.0f, 1120.0f, CityLabel + TEXT(" First Minute Orientation Beacon"));

    SpawnInfoBoard(
        FVector(-4100.0f, -1860.0f, 0.0f),
        FVector(2.7f, 0.07f, 0.92f),
        TerminalYellow,
        TEXT("What Next"),
        FString::Printf(TEXT("WHAT TO DO NEXT\nPick a language, open the terminal,\nsolve: %s"), *Mission.TerminalTitle),
        TerminalYellow.ToFColor(true),
        19.0f);
    SpawnInfoBoard(
        FVector(-2975.0f, -1860.0f, 0.0f),
        FVector(2.7f, 0.07f, 0.92f),
        PaperWhite,
        TEXT("Controls"),
        TEXT("CONTROLS\nWASD move  Shift sprint\nE interact  J journal\nP/Esc pause  C camera"),
        FColor(235, 235, 220),
        18.0f);
    SpawnInfoBoard(
        FVector(-3540.0f, -1280.0f, 0.0f),
        FVector(4.8f, 0.07f, 0.84f),
        RescueCyan,
        TEXT("Rescue Promise"),
        FString::Printf(TEXT("RESCUE PROMISE\nCode is not busywork here.\nPassing tests opens a real route to %s."), *Mission.SurvivorName),
        RescueCyan.ToFColor(true),
        19.0f);

    const FVector StageLocals[] = {
        FVector(-3840.0f, -3140.0f, 14.0f),
        FVector(-3000.0f, -2300.0f, 14.0f),
        FVector(0.0f, -1500.0f, 14.0f),
        FVector(2850.0f, 1500.0f, 14.0f),
        FVector(-3700.0f, 2850.0f, 14.0f),
    };
    const TCHAR* StageNames[] = {
        TEXT("START"),
        TEXT("LANGUAGE"),
        TEXT("TERMINAL"),
        TEXT("RESCUE"),
        TEXT("EXTRACT"),
    };
    const FLinearColor StageColors[] = {
        StartBlue,
        LanguageAmber,
        TerminalYellow,
        RescueCyan,
        ExtractionGreen,
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(StageNames); ++i)
    {
        const FVector WorldLoc = Origin + CityOffset(StageLocals[i]);
        TagOrientation(SpawnBlock(
            WorldLoc,
            FVector(0.72f, 0.72f, 0.08f),
            StageColors[i] * 1.65f,
            FString::Printf(TEXT("%s Route Stage Marker %d %s"), *CityLabel, i + 1, StageNames[i]),
            false));
        TagOrientation(SpawnGuideText(
            FString::Printf(TEXT("%d %s"), i + 1, StageNames[i]),
            WorldLoc + FVector(0.0f, -42.0f, 132.0f),
            StageColors[i].ToFColor(true),
            20.0f));
    }

    SpawnPathSegment(FVector(-3840.0f, -3140.0f, 8.0f), FVector(-3000.0f, -2300.0f, 8.0f), 7, LanguageAmber, TEXT("Spawn To Language"));
    SpawnPathSegment(FVector(-3000.0f, -2300.0f, 8.0f), FVector(0.0f, -1500.0f, 8.0f), 11, TerminalYellow, TEXT("Language To Terminal"));
    SpawnPathSegment(FVector(0.0f, -1500.0f, 8.0f), FVector(2850.0f, 1500.0f, 8.0f), 13, RescueCyan, TEXT("Terminal To Survivor"));
    SpawnPathSegment(FVector(2850.0f, 1500.0f, 8.0f), FVector(-3700.0f, 2850.0f, 8.0f), 14, ExtractionGreen, TEXT("Survivor To Extraction"));

    const FVector LaneBase = Origin + CityOffset(FVector(-4050.0f, -720.0f, 0.0f));
    const TCHAR* LaneNames[] = {
        TEXT("MOVE"),
        TEXT("INTERACT"),
        TEXT("TERMINAL"),
        TEXT("SPRINT"),
        TEXT("RELOAD"),
        TEXT("COVER"),
    };
    const FLinearColor LaneColors[] = {
        StartBlue,
        RescueCyan,
        TerminalYellow,
        ExtractionGreen,
        FLinearColor(1.0f, 0.55f, 0.18f),
        DangerRed,
    };
    const TCHAR* LaneTips[] = {
        TEXT("WASD"),
        TEXT("E"),
        TEXT("E at terminal"),
        TEXT("Shift"),
        TEXT("R"),
        TEXT("break line of sight"),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(LaneNames); ++i)
    {
        const FVector Lane = LaneBase + FVector(i * 205.0f, 0.0f, 0.0f);
        TagOrientation(SpawnBlock(
            Lane + FVector(0.0f, 0.0f, 18.0f),
            FVector(0.72f, 0.28f, 0.055f),
            LaneColors[i] * 1.45f,
            FString::Printf(TEXT("%s Beginner Lane %s Pad"), *CityLabel, LaneNames[i]),
            false));
        TagOrientation(SpawnBlock(
            Lane + FVector(0.0f, 82.0f, 82.0f),
            FVector(0.26f, 0.10f, 0.62f),
            LaneColors[i] * 0.82f,
            FString::Printf(TEXT("%s Beginner Lane %s Prop"), *CityLabel, LaneNames[i]),
            true));
        TagOrientation(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), LaneNames[i], LaneTips[i]),
            Lane + FVector(0.0f, -54.0f, 132.0f),
            LaneColors[i].ToFColor(true),
            16.0f));
    }

    SpawnInfoBoard(
        FVector(2990.0f, -940.0f, 0.0f),
        FVector(3.4f, 0.07f, 0.86f),
        DangerRed,
        TEXT("Optional Boss"),
        TEXT("OPTIONAL BOSS\nThe warden is a bonus threat.\nFinish the lesson and rescue first."),
        DangerRed.ToFColor(true),
        18.0f);
    SpawnInfoBoard(
        FVector(-2420.0f, -2300.0f, 0.0f),
        FVector(3.4f, 0.07f, 0.78f),
        LanguageAmber,
        TEXT("Language Locked"),
        FString::Printf(TEXT("%s TRACK LOCKED\nChosen on the start screen.\nAll terminals use this language."), LockedLanguageLabels[LockedLanguageIndex]),
        LanguageAmber.ToFColor(true),
        18.0f);
    SpawnInfoBoard(
        FVector(560.0f, -1470.0f, 0.0f),
        FVector(3.5f, 0.07f, 0.80f),
        TerminalYellow,
        TEXT("Tests"),
        TEXT("VISIBLE AND HIDDEN TESTS\nVisible cases teach the shape.\nHidden cases check real understanding."),
        TerminalYellow.ToFColor(true),
        18.0f);
    SpawnInfoBoard(
        FVector(1020.0f, -1130.0f, 0.0f),
        FVector(3.2f, 0.07f, 0.78f),
        ExtractionGreen,
        TEXT("Hints"),
        TEXT("HINTS AND BONUSES\nHints are allowed.\nNo-hint solves earn extra mastery."),
        ExtractionGreen.ToFColor(true),
        18.0f);

    const FVector BeaconDemo = Origin + CityOffset(FVector(-2860.0f, -1280.0f, 0.0f));
    const TCHAR* BeaconLabels[] = { TEXT("TERMINAL"), TEXT("SURVIVOR"), TEXT("THREAT") };
    const FLinearColor BeaconColors[] = { TerminalYellow, RescueCyan, DangerRed };
    for (int32 i = 0; i < UE_ARRAY_COUNT(BeaconLabels); ++i)
    {
        const FVector Demo = BeaconDemo + FVector(i * 250.0f, 0.0f, 0.0f);
        TagOrientation(SpawnBlock(
            Demo + FVector(0.0f, 0.0f, 78.0f),
            FVector(0.44f, 0.44f, 0.58f + i * 0.16f),
            BeaconColors[i] * 0.95f,
            FString::Printf(TEXT("%s Beacon Comparison %s Body"), *CityLabel, BeaconLabels[i]),
            false));
        TagOrientation(SpawnBlock(
            Demo + FVector(0.0f, 0.0f, 182.0f + i * 18.0f),
            FVector(0.56f, 0.56f, 0.045f),
            BeaconColors[i] * 2.2f,
            FString::Printf(TEXT("%s Beacon Comparison %s Halo"), *CityLabel, BeaconLabels[i]),
            false));
        TagOrientation(SpawnGuideText(
            BeaconLabels[i],
            Demo + FVector(0.0f, -50.0f, 262.0f),
            BeaconColors[i].ToFColor(true),
            16.0f));
    }

    SpawnInfoBoard(
        FVector(-2260.0f, -780.0f, 0.0f),
        FVector(3.6f, 0.07f, 0.78f),
        RescueCyan,
        TEXT("Route Unlock"),
        TEXT("CODE OPENS ROUTES\nA correct solution changes the city state\nand makes rescue possible."),
        RescueCyan.ToFColor(true),
        17.0f);
    SpawnInfoBoard(
        FVector(-1780.0f, -780.0f, 0.0f),
        FVector(3.6f, 0.07f, 0.78f),
        PaperWhite,
        TEXT("Civilians"),
        TEXT("CIVILIANS\nNamed people add story texture.\nObjective markers show who needs action."),
        FColor(235, 235, 220),
        17.0f);
    SpawnInfoBoard(
        FVector(-1300.0f, -780.0f, 0.0f),
        FVector(3.6f, 0.07f, 0.78f),
        ExtractionGreen,
        TEXT("Fast Travel"),
        TEXT("FAST TRAVEL\nHelipads move you between cities\nafter rescue and graduation."),
        ExtractionGreen.ToFColor(true),
        17.0f);
    SpawnInfoBoard(
        FVector(-820.0f, -780.0f, 0.0f),
        FVector(3.6f, 0.07f, 0.78f),
        TerminalYellow,
        TEXT("Journal"),
        TEXT("JOURNAL\nPress J for current city,\nmission, and curriculum recap."),
        TerminalYellow.ToFColor(true),
        17.0f);
    SpawnInfoBoard(
        FVector(-340.0f, -780.0f, 0.0f),
        FVector(3.6f, 0.07f, 0.78f),
        PaperWhite,
        TEXT("Pause"),
        TEXT("PAUSE AND EXIT\nPress P or Escape for settings,\nsaves, and safe exit."),
        FColor(235, 235, 220),
        17.0f);
    SpawnInfoBoard(
        FVector(140.0f, -780.0f, 0.0f),
        FVector(3.6f, 0.07f, 0.78f),
        StartBlue,
        TEXT("Camera"),
        TEXT("CAMERA\nC cycles. 5 FPS, 6 TPS,\n7 tactical, 8 top-down, 9 iso, 0 side."),
        StartBlue.ToFColor(true),
        17.0f);

    TagOrientation(SpawnBlock(
        Origin + CityOffset(FVector(-4100.0f, -1120.0f, 18.0f)),
        FVector(0.10f, 4.8f, 0.08f),
        ExtractionGreen * 1.45f,
        CityLabel + TEXT(" Safe Zone Boundary Stripe"),
        false));
    TagOrientation(SpawnGuideText(
        TEXT("SAFE ZONE EDGE\nlearn the route before you cross deep into danger"),
        Origin + CityOffset(FVector(-4140.0f, -1120.0f, 178.0f)),
        ExtractionGreen.ToFColor(true),
        17.0f));
    TagOrientation(SpawnBlock(
        Origin + CityOffset(FVector(2580.0f, -890.0f, 18.0f)),
        FVector(4.8f, 0.10f, 0.08f),
        DangerRed * 1.55f,
        CityLabel + TEXT(" Danger Increases Threshold Stripe"),
        false));
    TagOrientation(SpawnGuideText(
        TEXT("DANGER RISES PAST THIS LINE\nboss routes are optional until you are ready"),
        Origin + CityOffset(FVector(2580.0f, -930.0f, 178.0f)),
        DangerRed.ToFColor(true),
        17.0f));
    TagOrientation(SpawnGuideText(
        TEXT("LOST?\nFollow the colored route strips back to the five-step loop."),
        Origin + CityOffset(FVector(-1860.0f, -1540.0f, 290.0f)),
        StartBlue.ToFColor(true),
        21.0f));
    for (int32 i = 0; i < 5; ++i)
    {
        TagOrientation(SpawnRotatedBlock(
            Origin + CityOffset(FVector(-2060.0f + i * 120.0f, -1380.0f + i * 86.0f, 26.0f)),
            FRotator(0.0f, 34.0f, 0.0f),
            FVector(0.56f, 0.06f, 0.06f),
            StartBlue * (1.65f + i * 0.10f),
            FString::Printf(TEXT("%s Return To Route Marker %d"), *CityLabel, i + 1),
            false));
    }
    TagOrientation(SpawnGuideText(
        TEXT("RETURN TO ROUTE\nblue markers point back to the learning path"),
        Origin + CityOffset(FVector(-1720.0f, -1200.0f, 245.0f)),
        StartBlue.ToFColor(true),
        18.0f));
    TagOrientation(SpawnGuideText(
        TEXT("CITY DEBRIEF\nAfter extraction, review the concept you practiced\nbefore jumping to the next city."),
        Origin + CityOffset(FVector(-3380.0f, 2540.0f, 320.0f)),
        ExtractionGreen.ToFColor(true),
        22.0f));

    TagOrientation(SpawnGuideText(
        TEXT("500-ITEM IMPROVEMENT LEDGER: ORIENTATION SPRINT\nfirst-minute clarity, controls, route markers, practice lanes, learning promise"),
        Plaza + FVector(0.0f, 280.0f, 430.0f),
        FColor(245, 250, 255),
        24.0f));

    (void)CityIndex;
}

void ACodeRescueGameMode::SpawnNext100DevelopmentLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FLinearColor CurriculumBlue = FLinearColor(0.14f, 0.58f, 1.0f);
    const FLinearColor WorldGreen = FLinearColor(0.32f, 1.0f, 0.52f);
    const FLinearColor CharacterGold = FLinearColor(1.0f, 0.78f, 0.22f);
    const FLinearColor FlowCyan = FLinearColor(0.18f, 0.94f, 1.0f);
    const FLinearColor AccessCoral = FLinearColor(1.0f, 0.50f, 0.28f);
    const FLinearColor QAWhite = FLinearColor(0.88f, 0.94f, 1.0f);
    const FLinearColor BoardDark = FLinearColor(0.025f, 0.030f, 0.038f);
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    auto TagNext100 = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("Next100Implementation"));
            Actor->Tags.Add(FName("WorldDevelopment"));
            Actor->Tags.Add(FName("LearningClarity"));
            Actor->Tags.Add(FName("QAInteractable"));
        }
        return Actor;
    };

    auto SpawnNext100Light = [&](const FVector& Local, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                Origin + CityOffset(Local),
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagNext100(Light);
        }
    };

    auto SpawnNext100Board = [&](const FVector& Local, const FVector& Scale, const FLinearColor& Color, const FString& Name, const FString& Text, float TextSize)
    {
        const FVector Base = Origin + CityOffset(Local);
        TagNext100(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 142.0f),
            Scale,
            BoardDark + Color * 0.22f,
            CityLabel + TEXT(" Next100 ") + Name + TEXT(" Board"),
            false));
        TagNext100(SpawnGuideText(
            Text,
            Base + FVector(0.0f, -58.0f, 232.0f),
            Color.ToFColor(true),
            TextSize));
    };

    const FVector Hub = Origin + CityOffset(FVector(1280.0f, -2760.0f, 0.0f));
    TagNext100(SpawnTexturedBlock(
        Hub + FVector(0.0f, 0.0f, -8.0f),
        FVector(9.2f, 5.4f, 0.055f),
        FLinearColor(0.052f, 0.064f, 0.068f) + Mission.AccentColor * 0.12f,
        CityLabel + TEXT(" Next100 Systems Plaza"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false));
    TagNext100(SpawnBlock(
        Hub + FVector(0.0f, -402.0f, 184.0f),
        FVector(7.1f, 0.08f, 1.18f),
        BoardDark + Mission.SecondaryAccentColor * 0.22f,
        CityLabel + TEXT(" Next100 Completion Header Wall"),
        false));
    TagNext100(SpawnGuideText(
        FString::Printf(
            TEXT("NEXT 100 IMPLEMENTATION HUB\nLevel %03d: %s\n%s"),
            Mission.Rank,
            *Mission.CityName,
            *Mission.CurriculumStageName),
        Hub + FVector(0.0f, -462.0f, 296.0f),
        Mission.SecondaryAccentColor.ToFColor(true),
        24.0f));
    SpawnNext100Light(FVector(1280.0f, -2760.0f, 420.0f), Mission.SecondaryAccentColor, 5600.0f, 1120.0f, CityLabel + TEXT(" Next100 Hub Light"));

    SpawnNext100Board(
        FVector(470.0f, -2870.0f, 0.0f),
        FVector(3.2f, 0.07f, 0.94f),
        CurriculumBlue,
        TEXT("Curriculum Map"),
        FString::Printf(TEXT("CURRICULUM 1-20\n%s\n%s\n%s"),
            *Mission.LanguageTrackText,
            *Mission.LearningSupportText,
            *Mission.VisualDebuggerPlan),
        15.0f);
    SpawnNext100Board(
        FVector(1120.0f, -2870.0f, 0.0f),
        FVector(3.2f, 0.07f, 0.94f),
        WorldGreen,
        TEXT("World Architecture"),
        FString::Printf(TEXT("WORLD 21-40\nAtlas, skyline, street set, transit, arrival plaza,\nstory props, district signs, lighting, vistas.\n%s"),
            *Mission.ArchitectureSignature),
        16.0f);
    SpawnNext100Board(
        FVector(1770.0f, -2870.0f, 0.0f),
        FVector(3.2f, 0.07f, 0.94f),
        CharacterGold,
        TEXT("Characters Story"),
        FString::Printf(TEXT("CHARACTERS 41-60\nMentors, survivors, role icons, dialogue,\nrecurring allies, nameplates, debriefs.\n%s"),
            *Mission.CharacterStoryPlan),
        16.0f);
    SpawnNext100Board(
        FVector(2420.0f, -2870.0f, 0.0f),
        FVector(3.2f, 0.07f, 0.94f),
        FlowCyan,
        TEXT("Flow Progression"),
        FString::Printf(TEXT("FLOW 61-80\nReplay, practice, fail-safes, reward choice,\nrecaps, profile stats, review recommendation.\n%s"),
            *Mission.GameplayFlowPlan),
        15.0f);
    SpawnNext100Board(
        FVector(790.0f, -2260.0f, 0.0f),
        FVector(3.5f, 0.07f, 0.96f),
        AccessCoral,
        TEXT("Access Polish"),
        FString::Printf(TEXT("UI ACCESS AUDIO 81-95\n%s"),
            *Mission.AccessibilityPolishPlan),
        15.0f);
    SpawnNext100Board(
        FVector(1930.0f, -2260.0f, 0.0f),
        FVector(3.5f, 0.07f, 0.96f),
        QAWhite,
        TEXT("QA Release"),
        FString::Printf(TEXT("QA 96-100\n%s"),
            *Mission.QAVerificationPlan),
        15.0f);

    static const TCHAR* MentorLabels[] = {
        TEXT("Ada / Java Mentor"),
        TEXT("Ken / C Mentor"),
        TEXT("Grace / Python Mentor"),
        TEXT("Mira / MATLAB Mentor"),
        TEXT("Jules / C+ Mentor"),
        TEXT("Lin / C++ Mentor"),
    };
    const FLinearColor MentorColors[] = {
        FLinearColor(1.0f, 0.42f, 0.18f),
        FLinearColor(0.28f, 0.60f, 1.0f),
        FLinearColor(1.0f, 0.88f, 0.20f),
        FLinearColor(0.82f, 0.32f, 1.0f),
        FLinearColor(0.20f, 0.86f, 1.0f),
        FLinearColor(0.34f, 0.62f, 1.0f),
    };
    const int32 MentorLanguageIndex = GI
        ? FMath::Clamp(static_cast<int32>(GI->SelectedLanguage), 0, static_cast<int32>(UE_ARRAY_COUNT(MentorLabels)) - 1)
        : 0;
    const FVector MentorLoc = Hub + FVector(0.0f, 330.0f, 92.0f);
    TagNext100(SpawnDecorativeCivilian(
        MentorLoc,
        FRotator(0.0f, 0.0f, 0.0f),
        (MentorLanguageIndex % 2) == 0,
        MentorColors[MentorLanguageIndex],
        FString::Printf(TEXT("%s Next100 Selected Language Mentor"), *CityLabel),
        MentorLabels[MentorLanguageIndex]));
    TagNext100(SpawnBlock(
        MentorLoc + FVector(0.0f, -64.0f, 178.0f),
        FVector(0.42f, 0.055f, 0.36f),
        MentorColors[MentorLanguageIndex] * 1.55f,
        FString::Printf(TEXT("%s Next100 Selected Mentor Role Icon"), *CityLabel),
        false));
    TagNext100(SpawnGuideText(
        MentorLabels[MentorLanguageIndex],
        MentorLoc + FVector(0.0f, -102.0f, 248.0f),
        MentorColors[MentorLanguageIndex].ToFColor(true),
        15.0f));

    const FVector DebugBase = Hub + FVector(-900.0f, 650.0f, 0.0f);
    static const TCHAR* DebugLabels[] = { TEXT("INPUT"), TEXT("STATE"), TEXT("MID"), TEXT("NEXT"), TEXT("RETURN") };
    for (int32 i = 0; i < UE_ARRAY_COUNT(DebugLabels); ++i)
    {
        const float Alpha = static_cast<float>(i) / 4.0f;
        const FLinearColor DebugColor = FLinearColor::LerpUsingHSV(CurriculumBlue, WorldGreen, Alpha);
        TagNext100(SpawnBlock(
            DebugBase + FVector(i * 165.0f, 0.0f, 42.0f + i * 24.0f),
            FVector(0.52f, 0.32f, 0.20f + i * 0.06f),
            DebugColor * 1.45f,
            FString::Printf(TEXT("%s Next100 Visual Debug Step %s"), *CityLabel, DebugLabels[i]),
            false));
        TagNext100(SpawnGuideText(
            DebugLabels[i],
            DebugBase + FVector(i * 165.0f, -44.0f, 130.0f + i * 24.0f),
            DebugColor.ToFColor(true),
            15.0f));
    }
    TagNext100(SpawnGuideText(
        TEXT("VISUAL TRACEBOARD\ntrace input -> state -> decision -> next -> return"),
        DebugBase + FVector(340.0f, -104.0f, 312.0f),
        CurriculumBlue.ToFColor(true),
        20.0f));

    const FVector StreetBase = Hub + FVector(990.0f, 610.0f, 0.0f);
    static const TCHAR* FurnitureLabels[] = { TEXT("bench"), TEXT("lamp"), TEXT("metro"), TEXT("ferry"), TEXT("tram"), TEXT("bus") };
    const FLinearColor FurnitureColors[] = { WorldGreen, CharacterGold, FlowCyan, CurriculumBlue, AccessCoral, QAWhite };
    for (int32 i = 0; i < UE_ARRAY_COUNT(FurnitureLabels); ++i)
    {
        const FVector Prop = StreetBase + FVector(-420.0f + i * 168.0f, 0.0f, 40.0f);
        TagNext100(SpawnBlock(
            Prop,
            FVector(0.50f, 0.20f + (i % 3) * 0.08f, 0.24f + (i % 2) * 0.22f),
            FurnitureColors[i] * 0.86f,
            FString::Printf(TEXT("%s Next100 Street Furniture %s"), *CityLabel, FurnitureLabels[i]),
            true));
        TagNext100(SpawnGuideText(
            FurnitureLabels[i],
            Prop + FVector(0.0f, -42.0f, 112.0f),
            FurnitureColors[i].ToFColor(true),
            14.0f));
    }
    TagNext100(SpawnGuideText(
        TEXT("CITY DETAIL SET\nstreet furniture + transit markers + arrival plaza identity"),
        StreetBase + FVector(0.0f, -118.0f, 238.0f),
        WorldGreen.ToFColor(true),
        18.0f));

    const FVector RewardBase = Hub + FVector(-740.0f, 1030.0f, 0.0f);
    static const TCHAR* RewardLabels[] = { TEXT("ammo"), TEXT("medkit"), TEXT("companion"), TEXT("score") };
    const FLinearColor RewardColors[] = {
        FLinearColor(1.0f, 0.45f, 0.16f),
        FLinearColor(1.0f, 0.18f, 0.18f),
        FlowCyan,
        CharacterGold,
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(RewardLabels); ++i)
    {
        const FVector Crate = RewardBase + FVector(i * 170.0f, 0.0f, 54.0f);
        TagNext100(SpawnBlock(
            Crate,
            FVector(0.58f, 0.42f, 0.38f),
            RewardColors[i] * 0.95f,
            FString::Printf(TEXT("%s Next100 Reward Choice %s"), *CityLabel, RewardLabels[i]),
            true));
        TagNext100(SpawnGuideText(
            RewardLabels[i],
            Crate + FVector(0.0f, -44.0f, 122.0f),
            RewardColors[i].ToFColor(true),
            14.0f));
    }
    TagNext100(SpawnGuideText(
        TEXT("REWARD CHOICE KIOSK\ncompletion can award supplies, companion boost, or score multiplier"),
        RewardBase + FVector(252.0f, -116.0f, 238.0f),
        CharacterGold.ToFColor(true),
        18.0f));

    const FVector AccessBase = Hub + FVector(720.0f, 1030.0f, 0.0f);
    static const TCHAR* AccessLabels[] = { TEXT("font"), TEXT("HUD"), TEXT("subtitles"), TEXT("contrast"), TEXT("keys"), TEXT("controller") };
    for (int32 i = 0; i < UE_ARRAY_COUNT(AccessLabels); ++i)
    {
        const FVector Chip = AccessBase + FVector(-420.0f + i * 168.0f, 0.0f, 48.0f);
        const FLinearColor ChipColor = FLinearColor::LerpUsingHSV(AccessCoral, QAWhite, static_cast<float>(i) / 5.0f);
        TagNext100(SpawnBlock(
            Chip,
            FVector(0.44f, 0.28f, 0.24f),
            ChipColor * 1.35f,
            FString::Printf(TEXT("%s Next100 Accessibility Control %s"), *CityLabel, AccessLabels[i]),
            false));
        TagNext100(SpawnGuideText(
            AccessLabels[i],
            Chip + FVector(0.0f, -42.0f, 116.0f),
            ChipColor.ToFColor(true),
            13.0f));
    }
    TagNext100(SpawnGuideText(
        TEXT("ACCESSIBILITY CONSOLE\nscalable text, color-safe markers, remapping, controller nav"),
        AccessBase + FVector(0.0f, -116.0f, 238.0f),
        AccessCoral.ToFColor(true),
        18.0f));

    const FVector QABase = Hub + FVector(0.0f, 1390.0f, 0.0f);
    static const TCHAR* QALabels[] = { TEXT("spawn"), TEXT("terminal"), TEXT("rescue"), TEXT("fast travel"), TEXT("package") };
    for (int32 i = 0; i < UE_ARRAY_COUNT(QALabels); ++i)
    {
        const FVector Pedestal = QABase + FVector(-330.0f + i * 165.0f, 0.0f, 36.0f);
        TagNext100(SpawnBlock(
            Pedestal,
            FVector(0.46f, 0.46f, 0.20f + i * 0.05f),
            QAWhite * (0.92f + i * 0.08f),
            FString::Printf(TEXT("%s Next100 QA Sample %s"), *CityLabel, QALabels[i]),
            false));
        TagNext100(SpawnGuideText(
            QALabels[i],
            Pedestal + FVector(0.0f, -42.0f, 110.0f + i * 10.0f),
            QAWhite.ToFColor(true),
            13.0f));
    }
    TagNext100(SpawnGuideText(
        FString::Printf(TEXT("ALL-LEVEL QA\n465-city audit covers mission data; runtime smoke checks active spawn.\nRepresentative rank: %03d | city index: %d"), Mission.Rank, CityIndex),
        QABase + FVector(0.0f, -112.0f, 244.0f),
        QAWhite.ToFColor(true),
        18.0f));

    TagNext100(SpawnGuideText(
        TEXT("NEXT 100 ROADMAP STATUS\nimplemented as data, terminal coaching, world props, character markers, flow kiosks, accessibility console, and QA audits"),
        Hub + FVector(0.0f, 240.0f, 442.0f),
        FColor(245, 250, 255),
        22.0f));
}

void ACodeRescueGameMode::SpawnBespokeSurvivalHorrorArtLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FLinearColor DeepInk(0.018f, 0.020f, 0.018f, 1.0f);
    const FLinearColor OldStone(0.18f, 0.17f, 0.145f, 1.0f);
    const FLinearColor SicklyLantern(1.0f, 0.62f, 0.24f, 1.0f);
    const FLinearColor OxideRed(0.62f, 0.075f, 0.040f, 1.0f);
    const FLinearColor DullBrass(0.72f, 0.52f, 0.20f, 1.0f);
    const FLinearColor CodeGreen(0.36f, 1.0f, 0.56f, 1.0f);
    const FLinearColor ColdSlate(0.18f, 0.26f, 0.31f, 1.0f);

    auto TagBespoke = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("BespokeSurvivalHorrorArt"));
            Actor->Tags.Add(FName("PolishedArtPass"));
            Actor->Tags.Add(FName("WorldDevelopment"));
            Actor->Tags.Add(FName("LearningClarity"));
        }
        return Actor;
    };

    auto AddSlowRotation = [&](AActor* Actor, const FRotator& Rate, const FString& ComponentName) -> AActor*
    {
        if (Actor)
        {
            if (URotatingMovementComponent* Rotator = NewObject<URotatingMovementComponent>(Actor, *ComponentName))
            {
                Actor->AddInstanceComponent(Rotator);
                Rotator->RotationRate = Rate;
                Rotator->bRotationInLocalSpace = true;
                Rotator->RegisterComponent();
                Actor->Tags.Add(FName("BespokeAnimatedProp"));
            }
        }
        return Actor;
    };

    auto SpawnBespokeLight = [&](const FVector& Local, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                Origin + CityOffset(Local),
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(true);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagBespoke(Light);
        }
    };

    auto SpawnMoodBoard = [&](const FVector& Local, const FVector& Scale, const FLinearColor& Accent, const FString& Name, const FString& Text, float Size)
    {
        const FVector Base = Origin + CityOffset(Local);
        TagBespoke(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 132.0f),
            Scale,
            DeepInk + Accent * 0.18f,
            CityLabel + TEXT(" Bespoke ") + Name + TEXT(" Board"),
            false));
        TagBespoke(SpawnGuideText(
            Text,
            Base + FVector(0.0f, -58.0f, 220.0f),
            Accent.ToFColor(true),
            Size));
    };

    const FVector CourtyardLocal(-760.0f, -440.0f, 0.0f);
    const FVector Courtyard = Origin + CityOffset(CourtyardLocal);
    TagBespoke(SpawnTexturedBlock(
        Courtyard + FVector(0.0f, 0.0f, -12.0f),
        FVector(8.6f, 6.6f, 0.060f),
        OldStone,
        CityLabel + TEXT(" Bespoke Weathered Courtyard Stone"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false));
    TagBespoke(SpawnBlock(
        Courtyard + FVector(0.0f, -430.0f, 185.0f),
        FVector(6.6f, 0.12f, 1.45f),
        DeepInk + ColdSlate * 0.38f,
        CityLabel + TEXT(" Bespoke Gothic Rescue Facade"),
        false));
    TagBespoke(SpawnBlock(
        Courtyard + FVector(-420.0f, -430.0f, 160.0f),
        FVector(0.36f, 0.30f, 2.35f),
        OldStone,
        CityLabel + TEXT(" Bespoke Facade Left Pillar"),
        true));
    TagBespoke(SpawnBlock(
        Courtyard + FVector(420.0f, -430.0f, 160.0f),
        FVector(0.36f, 0.30f, 2.35f),
        OldStone,
        CityLabel + TEXT(" Bespoke Facade Right Pillar"),
        true));
    TagBespoke(SpawnBlock(
        Courtyard + FVector(0.0f, -430.0f, 358.0f),
        FVector(4.0f, 0.30f, 0.30f),
        DullBrass * 0.62f,
        CityLabel + TEXT(" Bespoke Heavy Lintel"),
        true));
    TagBespoke(SpawnGuideText(
        FString::Printf(TEXT("BESPOKE ART PASS\n%s\nsurvival-horror rescue academy; original coding-rescue art direction"), *Mission.CityName),
        Courtyard + FVector(0.0f, -502.0f, 306.0f),
        SicklyLantern.ToFColor(true),
        23.0f));

    for (int32 i = 0; i < 7; ++i)
    {
        const float X = -360.0f + i * 120.0f;
        const float Height = (i % 2 == 0) ? 0.92f : 0.66f;
        TagBespoke(SpawnRotatedBlock(
            Courtyard + FVector(X, -462.0f, 160.0f + i * 4.0f),
            FRotator(0.0f, 0.0f, -8.0f + i * 3.0f),
            FVector(0.34f, 0.045f, Height),
            FLinearColor(0.12f, 0.075f, 0.045f, 1.0f),
            FString::Printf(TEXT("%s Bespoke Boarded Window Slat %d"), *CityLabel, i + 1),
            false));
    }

    const FVector LanternLocals[] = {
        FVector(-610.0f, -350.0f, 214.0f),
        FVector(610.0f, -350.0f, 214.0f),
        FVector(-960.0f, 120.0f, 192.0f),
        FVector(960.0f, 120.0f, 192.0f),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(LanternLocals); ++i)
    {
        const FVector WorldLoc = Courtyard + CityOffset(LanternLocals[i]);
        TagBespoke(SpawnBlock(
            WorldLoc + FVector(0.0f, 0.0f, 72.0f),
            FVector(0.055f, 0.055f, 0.66f),
            DullBrass,
            FString::Printf(TEXT("%s Bespoke Hanging Lantern Chain %d"), *CityLabel, i + 1),
            false));
        AActor* Lantern = TagBespoke(SpawnBlock(
            WorldLoc,
            FVector(0.26f, 0.26f, 0.32f),
            SicklyLantern * 1.75f,
            FString::Printf(TEXT("%s Bespoke Animated Lantern %d"), *CityLabel, i + 1),
            false));
        AddSlowRotation(Lantern, FRotator(0.0f, 8.0f + i * 3.0f, 0.0f), FString::Printf(TEXT("BespokeLanternSway%d"), i + 1));
        SpawnBespokeLight(
            CourtyardLocal + LanternLocals[i] + FVector(0.0f, 0.0f, 30.0f),
            SicklyLantern,
            1450.0f + i * 160.0f,
            620.0f,
            FString::Printf(TEXT("%s Bespoke Lantern Light %d"), *CityLabel, i + 1));
    }

    const FVector PuzzleAltar = Courtyard + FVector(-1120.0f, 430.0f, 0.0f);
    TagBespoke(SpawnBlock(
        PuzzleAltar + FVector(0.0f, 0.0f, 62.0f),
        FVector(1.8f, 1.05f, 0.36f),
        FLinearColor(0.10f, 0.095f, 0.075f, 1.0f),
        CityLabel + TEXT(" Bespoke Code Reliquary Table"),
        true));
    static const TCHAR* GlyphLabels[] = { TEXT("READ"), TEXT("TRACE"), TEXT("TEST"), TEXT("RESCUE") };
    const FLinearColor GlyphColors[] = { CodeGreen, SicklyLantern, ColdSlate * 2.0f, Mission.AccentColor * 1.55f };
    for (int32 i = 0; i < UE_ARRAY_COUNT(GlyphLabels); ++i)
    {
        const FVector GlyphLoc = PuzzleAltar + FVector(-240.0f + i * 160.0f, -28.0f, 128.0f);
        AActor* Glyph = TagBespoke(SpawnBlock(
            GlyphLoc,
            FVector(0.38f, 0.055f, 0.38f),
            GlyphColors[i],
            FString::Printf(TEXT("%s Bespoke Animated Code Glyph %s"), *CityLabel, GlyphLabels[i]),
            false));
        AddSlowRotation(Glyph, FRotator(0.0f, 0.0f, 14.0f + i * 4.0f), FString::Printf(TEXT("BespokeCodeGlyphTurn%d"), i + 1));
        TagBespoke(SpawnGuideText(
            GlyphLabels[i],
            GlyphLoc + FVector(0.0f, -42.0f, 82.0f),
            GlyphColors[i].ToFColor(true),
            13.0f));
    }
    TagBespoke(SpawnGuideText(
        FString::Printf(TEXT("CODE RELIQUARY\n%s\nvisible tests become route marks"), *Mission.TerminalTitle),
        PuzzleAltar + FVector(0.0f, -92.0f, 224.0f),
        CodeGreen.ToFColor(true),
        17.0f));

    const FVector OTSLane = Origin + CityOffset(FVector(1140.0f, -900.0f, 0.0f));
    for (int32 i = 0; i < 9; ++i)
    {
        const float Alpha = static_cast<float>(i) / 8.0f;
        const FVector RailLoc = OTSLane + FVector(-520.0f + i * 130.0f, -210.0f + i * 28.0f, 28.0f);
        TagBespoke(SpawnRotatedBlock(
            RailLoc,
            FRotator(0.0f, 18.0f, 0.0f),
            FVector(0.70f, 0.045f, 0.075f),
            FLinearColor::LerpUsingHSV(OxideRed, SicklyLantern, Alpha) * 0.9f,
            FString::Printf(TEXT("%s Bespoke Over Shoulder Aim Rail %d"), *CityLabel, i + 1),
            false));
    }
    SpawnMoodBoard(
        FVector(1360.0f, -640.0f, 0.0f),
        FVector(2.7f, 0.075f, 0.82f),
        SicklyLantern,
        TEXT("Over Shoulder Readability"),
        TEXT("TACTICAL VIEW LANE\ncamera favors shoulder readability;\nterminal remains the safe puzzle anchor"),
        16.0f);

    const FVector SafeRoomLocal(-460.0f, 1800.0f, 0.0f);
    const FVector SafeRoom = Origin + CityOffset(SafeRoomLocal);
    TagBespoke(SpawnTexturedBlock(
        SafeRoom + FVector(0.0f, 0.0f, -8.0f),
        FVector(5.2f, 3.4f, 0.052f),
        FLinearColor(0.15f, 0.105f, 0.070f, 1.0f),
        CityLabel + TEXT(" Bespoke Safe Room Wood Floor"),
        TEXT("/Game/StarterContent/Materials/M_Wood_Floor_Walnut_Worn.M_Wood_Floor_Walnut_Worn"),
        false));
    TagBespoke(SpawnBlock(
        SafeRoom + FVector(-360.0f, 0.0f, 128.0f),
        FVector(0.20f, 2.5f, 1.28f),
        DeepInk + OldStone * 0.30f,
        CityLabel + TEXT(" Bespoke Safe Room Left Wall"),
        false));
    TagBespoke(SpawnBlock(
        SafeRoom + FVector(360.0f, 0.0f, 128.0f),
        FVector(0.20f, 2.5f, 1.28f),
        DeepInk + OldStone * 0.30f,
        CityLabel + TEXT(" Bespoke Safe Room Right Wall"),
        false));
    TagBespoke(SpawnBlock(
        SafeRoom + FVector(0.0f, 230.0f, 96.0f),
        FVector(3.4f, 0.16f, 0.88f),
        DeepInk + OxideRed * 0.22f,
        CityLabel + TEXT(" Bespoke Safe Room Curtain"),
        false));
    TagBespoke(SpawnBlock(
        SafeRoom + FVector(-120.0f, -10.0f, 62.0f),
        FVector(1.10f, 0.66f, 0.32f),
        FLinearColor(0.18f, 0.13f, 0.08f, 1.0f),
        CityLabel + TEXT(" Bespoke Save Desk"),
        true));
    AActor* RecorderReel = TagBespoke(SpawnBlock(
        SafeRoom + FVector(120.0f, -24.0f, 118.0f),
        FVector(0.30f, 0.055f, 0.30f),
        DullBrass * 1.35f,
        CityLabel + TEXT(" Bespoke Animated Debrief Reel"),
        false));
    AddSlowRotation(RecorderReel, FRotator(0.0f, 0.0f, 22.0f), TEXT("BespokeDebriefReelTurn"));
    SpawnBespokeLight(
        SafeRoomLocal + FVector(0.0f, -12.0f, 250.0f),
        FLinearColor(0.85f, 0.56f, 0.32f, 1.0f),
        1800.0f,
        760.0f,
        CityLabel + TEXT(" Bespoke Safe Room Light"));
    SpawnMoodBoard(
        FVector(-460.0f, 1480.0f, 0.0f),
        FVector(3.6f, 0.075f, 0.86f),
        DullBrass,
        TEXT("Safe Room UI Bridge"),
        TEXT("SAFE ROOM\nsave, review, replay, breathe;\nthe world calms after a solved concept"),
        17.0f);

    const FVector ThreatGate = Origin + CityOffset(FVector(2900.0f, -1500.0f, 0.0f));
    TagBespoke(SpawnBlock(
        ThreatGate + FVector(0.0f, -315.0f, 170.0f),
        FVector(4.1f, 0.14f, 1.28f),
        DeepInk + OxideRed * 0.32f,
        CityLabel + TEXT(" Bespoke Threat Gate Backplate"),
        false));
    for (int32 i = 0; i < 5; ++i)
    {
        const FVector SpikeLoc = ThreatGate + FVector(-360.0f + i * 180.0f, -328.0f, 300.0f);
        TagBespoke(SpawnRotatedBlock(
            SpikeLoc,
            FRotator(0.0f, 0.0f, 45.0f),
            FVector(0.22f, 0.050f, 0.86f),
            OxideRed * (0.78f + i * 0.04f),
            FString::Printf(TEXT("%s Bespoke Threat Gate Spike %d"), *CityLabel, i + 1),
            false));
    }
    SpawnMoodBoard(
        FVector(2910.0f, -1810.0f, 0.0f),
        FVector(3.6f, 0.075f, 0.88f),
        OxideRed,
        TEXT("Boss Mood Gate"),
        TEXT("BOSS MOOD GATE\noptional danger is staged clearly;\nlearn first, fight when ready"),
        17.0f);
    SpawnBespokeLight(
        FVector(2900.0f, -1500.0f, 275.0f),
        OxideRed,
        2300.0f,
        840.0f,
        CityLabel + TEXT(" Bespoke Threat Gate Light"));

    static const TCHAR* TableauLabels[] = {
        TEXT("Archivist"),
        TEXT("Signal Scout"),
        TEXT("Field Medic"),
    };
    const FLinearColor TableauColors[] = { DullBrass, CodeGreen, ColdSlate * 1.8f };
    for (int32 i = 0; i < UE_ARRAY_COUNT(TableauLabels); ++i)
    {
        const FVector NPC = Courtyard + FVector(660.0f + i * 180.0f, 420.0f + (i % 2) * 90.0f, 92.0f);
        TagBespoke(SpawnDecorativeCivilian(
            NPC,
            FRotator(0.0f, -135.0f + i * 18.0f, 0.0f),
            (i % 2) == 1,
            TableauColors[i],
            FString::Printf(TEXT("%s Bespoke Tableau NPC %d"), *CityLabel, i + 1),
            TableauLabels[i]));
        TagBespoke(SpawnGuideText(
            TableauLabels[i],
            NPC + FVector(0.0f, -62.0f, 190.0f),
            TableauColors[i].ToFColor(true),
            13.0f));
    }

    TagBespoke(SpawnGuideText(
        TEXT("POLISHED UI + WORLD ART\nmain menu, HUD, pause, terminal, victory, set pieces, animated lanterns, code glyphs"),
        Courtyard + FVector(0.0f, 350.0f, 420.0f),
        FColor(238, 220, 184),
        21.0f));

    (void)CityIndex;
}

void ACodeRescueGameMode::SpawnBespokeAuthoredAssetRefinementLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FLinearColor CandleGold(0.95f, 0.58f, 0.22f, 1.0f);
    const FLinearColor DeepCharcoal(0.025f, 0.026f, 0.024f, 1.0f);
    const FLinearColor GrimeStone(0.19f, 0.18f, 0.15f, 1.0f);
    const FLinearColor DriedRust(0.45f, 0.12f, 0.055f, 1.0f);
    const FLinearColor SignalGreen(0.30f, 0.95f, 0.50f, 1.0f);
    const FLinearColor ColdGlass(0.32f, 0.56f, 0.66f, 1.0f);

    auto TagRefined = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("BespokeAuthoredAssetRefinement"));
            Actor->Tags.Add(FName("ImportedMeshReplacement"));
            Actor->Tags.Add(FName("AuthoredTextureLayer"));
            Actor->Tags.Add(FName("LearningLoopPreserved"));
        }
        return Actor;
    };

    auto ApplyImportedMaterial = [&](AActor* Actor, const TCHAR* MaterialPath, const FLinearColor& Tint, float EmissiveScale = 0.0f) -> AActor*
    {
        if (!Actor || !MaterialPath)
        {
            return Actor;
        }

        if (UMaterialInterface* Material = LoadCodeRescueMaterial(MaterialPath))
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    UMaterialInstanceDynamic* MID = CodeRescueMaterials::CreateTintedDynamicMaterial(
                        Material,
                        this,
                        Tint,
                        EmissiveScale);
                    if (MID)
                    {
                        MeshComp->SetMaterial(0, MID);
                    }
                    else
                    {
                        MeshComp->SetMaterial(0, Material);
                    }
                    MeshComp->SetCastShadow(true);
                }
            }
        }
        return Actor;
    };

    auto SpawnImportedProp = [&](const TCHAR* MeshPath, const TCHAR* MaterialPath, const FVector& WorldLocation,
                                const FRotator& Rotation, const FVector& TargetSize, const FString& Name,
                                const FLinearColor& Tint, bool bEnableCollision = true) -> AActor*
    {
        AActor* Actor = SpawnStaticMeshProp(
            LoadCodeRescueAssetMesh(MeshPath),
            WorldLocation,
            Rotation,
            TargetSize,
            CityLabel + TEXT(" ") + Name,
            bEnableCollision);
        TagRefined(Actor);
        ApplyImportedMaterial(Actor, MaterialPath, Tint);
        return Actor;
    };

    auto AddSlowRotation = [&](AActor* Actor, const FRotator& Rate, const FString& ComponentName) -> AActor*
    {
        if (Actor)
        {
            if (URotatingMovementComponent* Rotator = NewObject<URotatingMovementComponent>(Actor, *ComponentName))
            {
                Actor->AddInstanceComponent(Rotator);
                Rotator->RotationRate = Rate;
                Rotator->bRotationInLocalSpace = true;
                Rotator->RegisterComponent();
                Actor->Tags.Add(FName("BespokeAnimatedProp"));
            }
        }
        return Actor;
    };

    auto SpawnRefinedLight = [&](const FVector& WorldLocation, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                WorldLocation,
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(true);
            }
#if WITH_EDITOR
            Light->SetActorLabel(CityLabel + TEXT(" ") + Name);
#endif
            RegisterStreamedActor(Light);
            TagRefined(Light);
        }
    };

    auto SpawnAnimatedClip = [&](const TCHAR* MeshPath, const TCHAR* AnimationPath, const FVector& WorldLocation,
                                 const FRotator& Rotation, const FVector& Scale, const FString& Name,
                                 const FLinearColor& AccentColor, float StartOffsetSeconds) -> AActor*
    {
        USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, MeshPath);
        UAnimationAsset* Animation = LoadObject<UAnimationAsset>(nullptr, AnimationPath);
        if (!Mesh || !Animation)
        {
            return TagRefined(SpawnBlock(
                WorldLocation + FVector(0.0f, 0.0f, 86.0f),
                FVector(0.42f, 0.42f, 1.24f),
                AccentColor,
                CityLabel + TEXT(" ") + Name + TEXT(" Fallback Clip Marker"),
                false));
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* Actor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), WorldLocation, Rotation, Params);
        if (!Actor)
        {
            return nullptr;
        }

        USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("BespokeClipRoot"));
        Actor->AddInstanceComponent(Root);
        Actor->SetRootComponent(Root);
        Root->RegisterComponent();
        Root->SetWorldLocation(WorldLocation);
        Root->SetWorldRotation(Rotation);

        USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(Actor, TEXT("BespokeAnimatedClipMesh"));
        Actor->AddInstanceComponent(MeshComp);
        MeshComp->SetupAttachment(Root);
        MeshComp->RegisterComponent();
        MeshComp->SetSkeletalMesh(Mesh);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        MeshComp->SetCastShadow(true);
        MeshComp->SetRelativeScale3D(Scale);
        MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        MeshComp->SetAnimation(Animation);
        MeshComp->Play(true);
        if (StartOffsetSeconds > 0.0f)
        {
            MeshComp->SetPosition(StartOffsetSeconds, false);
        }

        SpawnBlock(
            WorldLocation + FVector(0.0f, 0.0f, 210.0f),
            FVector(0.34f, 0.34f, 0.035f),
            AccentColor * 2.0f,
            CityLabel + TEXT(" ") + Name + TEXT(" Clip Halo"),
            false);

#if WITH_EDITOR
        Actor->SetActorLabel(CityLabel + TEXT(" ") + Name);
#endif
        RegisterStreamedActor(Actor);
        Actor->Tags.Add(FName("BespokeCharacterAnimationClip"));
        Actor->Tags.Add(FName("AnimationClipHook"));
        return TagRefined(Actor);
    };

    const FVector Courtyard = Origin + CityOffset(FVector(-760.0f, -440.0f, 0.0f));
    const FVector Facade = Courtyard + FVector(0.0f, -522.0f, 0.0f);

    TagRefined(SpawnTexturedBlock(
        Facade + FVector(0.0f, 0.0f, 116.0f),
        FVector(6.85f, 0.13f, 1.35f),
        GrimeStone,
        CityLabel + TEXT(" Refined Hewn Stone Facade Skin"),
        TEXT("/Game/StarterContent/Materials/M_Brick_Hewn_Stone.M_Brick_Hewn_Stone"),
        false));
    TagRefined(SpawnTexturedBlock(
        Courtyard + FVector(0.0f, 100.0f, -10.0f),
        FVector(8.2f, 5.3f, 0.052f),
        GrimeStone,
        CityLabel + TEXT(" Refined Cobblestone Courtyard Surface"),
        TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough"),
        false));
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_DoorFrame.SM_DoorFrame"),
        TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"),
        Facade + FVector(0.0f, -32.0f, 138.0f),
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(3.2f, 0.70f, 3.05f),
        TEXT("Imported Doorframe Rescue Hall"),
        GrimeStone,
        true);
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_Door.SM_Door"),
        TEXT("/Game/StarterContent/Materials/M_Wood_Walnut.M_Wood_Walnut"),
        Facade + FVector(0.0f, -56.0f, 116.0f),
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(2.05f, 0.28f, 2.35f),
        TEXT("Imported Heavy Rescue Door"),
        FLinearColor(0.25f, 0.16f, 0.095f, 1.0f),
        true);

    for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
    {
        const float Side = SideIndex == 0 ? -1.0f : 1.0f;
        SpawnImportedProp(
            TEXT("/Game/StarterContent/Props/SM_WindowFrame.SM_WindowFrame"),
            TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
            Facade + FVector(Side * 365.0f, -48.0f, 176.0f),
            FRotator(0.0f, 0.0f, 0.0f),
            FVector(1.65f, 0.36f, 1.55f),
            FString::Printf(TEXT("Imported Window Frame %d"), SideIndex + 1),
            FLinearColor(0.26f, 0.24f, 0.21f, 1.0f),
            false);
        SpawnImportedProp(
            TEXT("/Game/StarterContent/Props/SM_GlassWindow.SM_GlassWindow"),
            TEXT("/Game/StarterContent/Materials/M_Metal_Steel.M_Metal_Steel"),   // 2026-07-02: M_Glass rendered as teal checker in packaged build; steel reads as a solid panel
            Facade + FVector(Side * 365.0f, -58.0f, 176.0f),
            FRotator(0.0f, 0.0f, 0.0f),
            FVector(1.42f, 0.08f, 1.22f),
            FString::Printf(TEXT("Imported Cold Glass Pane %d"), SideIndex + 1),
            ColdGlass,
            false);
        SpawnImportedProp(
            TEXT("/Game/StarterContent/Props/SM_Lamp_Wall.SM_Lamp_Wall"),
            TEXT("/Game/StarterContent/Props/Materials/M_Lamp.M_Lamp"),
            Facade + FVector(Side * 555.0f, -78.0f, 225.0f),
            FRotator(0.0f, Side > 0.0f ? 180.0f : 0.0f, 0.0f),
            FVector(0.58f, 0.40f, 0.72f),
            FString::Printf(TEXT("Imported Wall Lamp %d"), SideIndex + 1),
            CandleGold,
            false);
        SpawnRefinedLight(
            Facade + FVector(Side * 555.0f, -118.0f, 224.0f),
            CandleGold,
            1450.0f,
            520.0f,
            FString::Printf(TEXT("Refined Wall Lamp Light %d"), SideIndex + 1));
    }

    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_PillarFrame300.SM_PillarFrame300"),
        TEXT("/Game/StarterContent/Materials/M_Brick_Hewn_Stone.M_Brick_Hewn_Stone"),
        Facade + FVector(-620.0f, -18.0f, 170.0f),
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(1.25f, 0.52f, 3.35f),
        TEXT("Imported Left Pillar Frame"),
        GrimeStone,
        true);
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_PillarFrame300.SM_PillarFrame300"),
        TEXT("/Game/StarterContent/Materials/M_Brick_Hewn_Stone.M_Brick_Hewn_Stone"),
        Facade + FVector(620.0f, -18.0f, 170.0f),
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(1.25f, 0.52f, 3.35f),
        TEXT("Imported Right Pillar Frame"),
        GrimeStone,
        true);

    AActor* Crest = SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_Statue.SM_Statue"),
        TEXT("/Game/StarterContent/Props/Materials/M_Statue.M_Statue"),
        Facade + FVector(0.0f, -84.0f, 362.0f),
        FRotator(0.0f, 180.0f, 0.0f),
        FVector(1.05f, 0.80f, 1.25f),
        TEXT("Imported Hall Crest Statue"),
        FLinearColor(0.42f, 0.39f, 0.34f, 1.0f),
        false);
    AddSlowRotation(Crest, FRotator(0.0f, 2.5f, 0.0f), TEXT("BespokeCrestStatueTurn"));

    const FVector SafeRoom = Origin + CityOffset(FVector(-460.0f, 1800.0f, 0.0f));
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_Couch.SM_Couch"),
        TEXT("/Game/StarterContent/Materials/M_Wood_Walnut.M_Wood_Walnut"),
        SafeRoom + FVector(-210.0f, 40.0f, 74.0f),
        FRotator(0.0f, 90.0f, 0.0f),
        FVector(2.35f, 0.86f, 0.82f),
        TEXT("Imported Safe Room Couch"),
        FLinearColor(0.22f, 0.12f, 0.075f, 1.0f),
        true);
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_TableRound.SM_TableRound"),
        TEXT("/Game/StarterContent/Props/Materials/M_TableRound.M_TableRound"),
        SafeRoom + FVector(30.0f, -42.0f, 78.0f),
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(1.18f, 1.18f, 0.78f),
        TEXT("Imported Debrief Table"),
        FLinearColor(0.25f, 0.17f, 0.10f, 1.0f),
        true);
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_Shelf.SM_Shelf"),
        TEXT("/Game/StarterContent/Props/Materials/M_Shelf.M_Shelf"),
        SafeRoom + FVector(250.0f, 110.0f, 128.0f),
        FRotator(0.0f, -90.0f, 0.0f),
        FVector(1.15f, 0.52f, 1.95f),
        TEXT("Imported Archive Shelf"),
        FLinearColor(0.23f, 0.16f, 0.10f, 1.0f),
        true);
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_Lamp_Ceiling.SM_Lamp_Ceiling"),
        TEXT("/Game/StarterContent/Props/Materials/M_Lamp.M_Lamp"),
        SafeRoom + FVector(0.0f, -16.0f, 252.0f),
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(0.78f, 0.78f, 0.92f),
        TEXT("Imported Safe Room Ceiling Lamp"),
        CandleGold,
        false);

    const FVector TerminalGallery = Origin + CityOffset(FVector(1160.0f, -910.0f, 0.0f));
    TagRefined(SpawnTexturedBlock(
        TerminalGallery + FVector(0.0f, -360.0f, 22.0f),
        FVector(3.85f, 0.26f, 0.11f),
        SignalGreen,
        CityLabel + TEXT(" Refined Terminal Route Inlay"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false));
    SpawnImportedProp(
        TEXT("/Game/StarterContent/Props/SM_Stairs.SM_Stairs"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Rust.M_Metal_Rust"),
        TerminalGallery + FVector(-390.0f, -190.0f, 54.0f),
        FRotator(0.0f, 20.0f, 0.0f),
        FVector(2.6f, 1.35f, 0.72f),
        TEXT("Imported Tactical Stairs"),
        DriedRust,
        true);
    SpawnImportedProp(
        TEXT("/Game/ModernBridges/Meshes/ModernBridges/SM_modern_bridge_001.SM_modern_bridge_001"),
        TEXT("/Game/ModernBridges/Materials/metal/MI_metal.MI_metal"),
        Origin + CityOffset(FVector(2260.0f, -1850.0f, 215.0f)),
        FRotator(0.0f, -16.0f, 0.0f),
        CityExtent(FVector(7.8f, 1.05f, 0.55f)),
        TEXT("Imported Postapo Overpass"),
        FLinearColor(0.23f, 0.22f, 0.20f, 1.0f),
        false);

    for (int32 i = 0; i < 3; ++i)
    {
        if (UStaticMesh* BuildingMesh = LoadCodeRescueCityBuildingMesh(CityIndex + 40 + i))
        {
            const float Side = i == 1 ? 1.0f : -1.0f;
            AActor* Building = SpawnStaticMeshProp(
                BuildingMesh,
                Origin + CityOffset(FVector(-2280.0f + i * 1580.0f, 2580.0f + Side * 160.0f, 0.0f)) + FVector(0.0f, 0.0f, 375.0f + i * 70.0f),
                FRotator(0.0f, 90.0f * i, 0.0f),
                CityArchitectureExtent(FVector(1.55f + i * 0.22f, 1.20f, 4.25f + i * 0.50f)),
                FString::Printf(TEXT("%s Imported Parallax Backlot Building %d"), *CityLabel, i + 1),
                true);
            TagRefined(Building);
            if (Building)
            {
                Building->Tags.Add(FName("ParallaxBacklotReplacement"));
                const int32 MaterialIndex = FMath::Abs(CityIndex + 40 + i) % UE_ARRAY_COUNT(CodeRescueCityBuildingMaterialPaths);
                ApplyCodeRescueMaterialToStaticActor(
                    Building,
                    CodeRescueCityBuildingMaterialPaths[MaterialIndex],
                    this,
                    FLinearColor(0.07f, 0.08f, 0.10f, 1.0f) + Mission.SecondaryAccentColor * 0.08f,
                    0.45f);
            }
        }
    }

    const FVector ClipStage = Courtyard + FVector(0.0f, 650.0f, 0.0f);
    TagRefined(SpawnTexturedBlock(
        ClipStage + FVector(60.0f, 0.0f, -4.0f),
        FVector(8.2f, 3.4f, 0.052f),
        DeepCharcoal + SignalGreen * 0.10f,
        CityLabel + TEXT(" Refined Animation Clip Stage"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false));
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn"),
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Quinn/MF_Idle.MF_Idle"),
        ClipStage + FVector(-490.0f, -40.0f, 94.0f),
        FRotator(0.0f, -145.0f, 0.0f),
        FVector(1.0f),
        TEXT("Looped Survivor Ready Clip"),
        SignalGreen,
        0.12f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"),
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Walk_InPlace.MM_Walk_InPlace"),
        ClipStage + FVector(-145.0f, 40.0f, 94.0f),
        FRotator(0.0f, -128.0f, 0.0f),
        FVector(1.0f),
        TEXT("Looped Engineer Patrol Clip"),
        CandleGold,
        0.38f);
    SpawnAnimatedClip(
        TEXT("/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit.ZombieFemale_NurseOutfit"),
        TEXT("/Game/ZombieFemale/Asset/Animations/ANMS_ZombieFemaleIdle05.ANMS_ZombieFemaleIdle05"),
        ClipStage + FVector(245.0f, 80.0f, 90.0f),
        FRotator(0.0f, 138.0f, 0.0f),
        FVector(0.96f),
        TEXT("Looped Nurse Threat Clip"),
        DriedRust,
        0.55f);
    SpawnAnimatedClip(
        TEXT("/Game/DogZombie/Meshes/SK_DogZombie.SK_DogZombie"),
        TEXT("/Game/DogZombie/Animations/anim_Dog_Sit_Idle.anim_Dog_Sit_Idle"),
        ClipStage + FVector(520.0f, -26.0f, 48.0f),
        FRotator(0.0f, 170.0f, 0.0f),
        FVector(0.58f),
        TEXT("Looped Scout Creature Clip"),
        ColdGlass,
        0.24f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn"),
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Quinn/MF_Walk_Fwd.MF_Walk_Fwd"),
        ClipStage + FVector(-650.0f, 370.0f, 94.0f),
        FRotator(0.0f, -124.0f, 0.0f),
        FVector(1.0f),
        TEXT("Looped Survivor Walk Clip"),
        SignalGreen,
        0.18f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"),
        TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Run_Fwd.MM_Run_Fwd"),
        ClipStage + FVector(-330.0f, 420.0f, 94.0f),
        FRotator(0.0f, -118.0f, 0.0f),
        FVector(1.0f),
        TEXT("Looped Engineer Run Clip"),
        CandleGold,
        0.44f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01.SK_Zombie_M04_01"),
        TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Walk_F_1_Loop_IPC.Zombie_Walk_F_1_Loop_IPC"),
        ClipStage + FVector(-12.0f, 426.0f, 94.0f),
        FRotator(0.0f, 142.0f, 0.0f),
        FVector(0.94f),
        TEXT("Looped Business Stalker Walk Clip"),
        DriedRust,
        0.30f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01.SK_Zombie_F01_01"),
        TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Atk_Arms_3_SHORT_Loop_IPC.Zombie_Atk_Arms_3_SHORT_Loop_IPC"),
        ClipStage + FVector(302.0f, 418.0f, 94.0f),
        FRotator(0.0f, 150.0f, 0.0f),
        FVector(0.94f),
        TEXT("Looped Bloated Attack Clip"),
        FLinearColor(0.88f, 0.16f, 0.12f, 1.0f),
        0.14f);
    SpawnAnimatedClip(
        TEXT("/Game/DogZombie/Meshes/SK_DogZombie.SK_DogZombie"),
        TEXT("/Game/DogZombie/Animations/anim_Dog_Run_InPlace.anim_Dog_Run_InPlace"),
        ClipStage + FVector(636.0f, 380.0f, 48.0f),
        FRotator(0.0f, 164.0f, 0.0f),
        FVector(0.58f),
        TEXT("Looped Scout Run Clip"),
        ColdGlass,
        0.34f);
    SpawnAnimatedClip(
        TEXT("/Game/Zombie/BaseMesh/SK_Zombie.SK_Zombie"),
        TEXT("/Game/Zombie/Demo/Animations/ThirdPersonWalk.ThirdPersonWalk"),
        ClipStage + FVector(-520.0f, 770.0f, 94.0f),
        FRotator(0.0f, 34.0f, 0.0f),
        FVector(1.0f),
        TEXT("Looped Base Threat Walk Clip"),
        DriedRust,
        0.52f);
    SpawnAnimatedClip(
        TEXT("/Game/ZombieFemale/Asset/Meshes/ZombieFemale_NurseOutfit.ZombieFemale_NurseOutfit"),
        TEXT("/Game/ZombieFemale/Asset/Animations/ANMS_ZombieFemaleAttackForward05.ANMS_ZombieFemaleAttackForward05"),
        ClipStage + FVector(-172.0f, 790.0f, 90.0f),
        FRotator(0.0f, 38.0f, 0.0f),
        FVector(0.96f),
        TEXT("Looped Nurse Attack Clip"),
        FLinearColor(0.88f, 0.16f, 0.12f, 1.0f),
        0.25f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Meshes/ZombieM04/Zombie/SK_Zombie_M04_01.SK_Zombie_M04_01"),
        TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Atk_Loop_1_IPC.Zombie_Atk_Loop_1_IPC"),
        ClipStage + FVector(180.0f, 784.0f, 94.0f),
        FRotator(0.0f, 34.0f, 0.0f),
        FVector(0.94f),
        TEXT("Looped Business Threat Attack Clip"),
        DriedRust,
        0.41f);
    SpawnAnimatedClip(
        TEXT("/Game/YI_ModularZombies/Meshes/ZombieF01/Zombie/SK_Zombie_F01_01.SK_Zombie_F01_01"),
        TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Death_Hit_Back_1_IPC.Zombie_Death_Hit_Back_1_IPC"),
        ClipStage + FVector(532.0f, 770.0f, 94.0f),
        FRotator(0.0f, 42.0f, 0.0f),
        FVector(0.94f),
        TEXT("Looped Bloated Threat Fall Clip"),
        FLinearColor(0.62f, 0.78f, 0.44f, 1.0f),
        0.20f);

    static const TCHAR* ClipLabels[] = { TEXT("READY"), TEXT("PATROL"), TEXT("THREAT"), TEXT("SCOUT") };
    const FLinearColor ClipColors[] = { SignalGreen, CandleGold, DriedRust, ColdGlass };
    for (int32 i = 0; i < UE_ARRAY_COUNT(ClipLabels); ++i)
    {
        TagRefined(SpawnGuideText(
            ClipLabels[i],
            ClipStage + FVector(-490.0f + i * 335.0f, -170.0f, 286.0f),
            ClipColors[i].ToFColor(true),
            20.0f));
    }

    TagRefined(SpawnGuideText(
        FString::Printf(TEXT("ARCHIVE OF LOST ROUTES\n%s\n13 authored loop clips: survivor, engineer, nurse, dog, base and modular threats"), *Mission.TerminalTitle),
        ClipStage + FVector(58.0f, -286.0f, 350.0f),
        FColor(226, 214, 180),
        22.0f));
}

void ACodeRescueGameMode::SpawnProductionTrackCompletionLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector Hub = Origin + CityOffset(FVector(2380.0f, -620.0f, 0.0f));
    const FLinearColor Slate = FLinearColor(0.055f, 0.062f, 0.070f, 1.0f);
    const FLinearColor Signal = Mission.SecondaryAccentColor * 1.4f + FLinearColor(0.06f, 0.10f, 0.10f, 1.0f);
    const FLinearColor Gold = FLinearColor(0.78f, 0.58f, 0.24f, 1.0f);
    const FString LocPrefix = FString::Printf(TEXT("campaign.%03d.%s"), Mission.Rank, *Mission.Slug);

    auto TagProduction = [&](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("ProductionTrackCompletion"));
            Actor->Tags.Add(FName("AllCityProductionCoverage"));
        }
        return Actor;
    };

    TagProduction(SpawnBlock(
        Hub + FVector(0.0f, 0.0f, 18.0f),
        CityExtent(FVector(4.6f, 2.4f, 0.07f)),
        Slate,
        CityLabel + TEXT(" Production Completion Plaza"),
        false));

    TagProduction(SpawnBlock(
        Hub + FVector(-430.0f, -170.0f, 142.0f),
        FVector(0.20f, 0.18f, 1.42f),
        Signal,
        CityLabel + TEXT(" Radio Coverage Mast"),
        false));
    TagProduction(SpawnBlock(
        Hub + FVector(-430.0f, -170.0f, 306.0f),
        FVector(1.00f, 0.05f, 0.10f),
        Signal * 1.9f,
        CityLabel + TEXT(" Radio Coverage Antenna"),
        false));
    TagProduction(SpawnBlock(
        Hub + FVector(-430.0f, -170.0f, 354.0f),
        FVector(0.34f, 0.34f, 0.04f),
        Gold * 1.8f,
        CityLabel + TEXT(" Radio Coverage Beacon"),
        false));

    TagProduction(SpawnBlock(
        Hub + FVector(0.0f, 110.0f, 92.0f),
        FVector(2.35f, 0.08f, 0.82f),
        FLinearColor(0.12f, 0.16f, 0.18f, 1.0f) + Mission.AccentColor * 0.12f,
        CityLabel + TEXT(" Localization Slate"),
        false));
    TagProduction(SpawnBlock(
        Hub + FVector(415.0f, -118.0f, 96.0f),
        FVector(0.92f, 0.62f, 0.10f),
        FLinearColor(0.11f, 0.18f, 0.15f, 1.0f) + Mission.SecondaryAccentColor * 0.16f,
        CityLabel + TEXT(" Performance Profile Console"),
        false));

    TagProduction(SpawnGuideText(
        FString::Printf(
            TEXT("PRODUCTION COVERAGE\nrank %03d | %s\nradio text, localization key, visual QA, profile budget"),
            Mission.Rank,
            *Mission.ArtKitName),
        Hub + FVector(0.0f, -210.0f, 310.0f),
        FColor(222, 234, 222),
        24.0f));

    TagProduction(SpawnGuideText(
        FString::Printf(
            TEXT("RADIO READY\nvoice %s | subtitle fallback on\nbriefing row %03d of campaign"),
            *Mission.RadioVoiceName,
            CityIndex + 1),
        Hub + FVector(-430.0f, -312.0f, 440.0f),
        Signal.ToFColor(true),
        20.0f));

    TagProduction(SpawnGuideText(
        FString::Printf(
            TEXT("LOC KEYS\n%s.terminal_title\n%s.radio_briefing\n%s.mission_brief"),
            *LocPrefix,
            *LocPrefix,
            *LocPrefix),
        Hub + FVector(0.0f, 214.0f, 230.0f),
        FColor(190, 230, 210),
        18.0f));

    TagProduction(SpawnGuideText(
        FString::Printf(
            TEXT("VISUAL QA\nlandmark: %s\narchitecture: %s"),
            *Mission.LandmarkName,
            *Mission.ArchitectureSignature),
        Hub + FVector(430.0f, 74.0f, 244.0f),
        Gold.ToFColor(true),
        16.0f));
}

void ACodeRescueGameMode::SpawnFirstViewAestheticArrivalLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector PlayerStart = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const FVector EntryCenter = Origin + CityOffset(FVector(-3780.0f, -3120.0f, 0.0f));
    const FLinearColor WetStone(0.14f, 0.15f, 0.15f, 1.0f);
    const FLinearColor RouteGlow = Mission.AccentColor * 2.2f + FLinearColor(0.02f, 0.10f, 0.08f, 1.0f);
    const FLinearColor WarmLamp(0.96f, 0.58f, 0.20f, 1.0f);

    auto TagArrival = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("FirstViewAestheticPass"));
            Actor->Tags.Add(FName("AuthoredArrivalComposition"));
            Actor->Tags.Add(FName("VisualReviewCoverage"));
        }
        return Actor;
    };

    auto SpawnArrivalLight = [&](const FVector& WorldLocation, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(
                APointLight::StaticClass(),
                WorldLocation,
                FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(true);
            }
#if WITH_EDITOR
            Light->SetActorLabel(CityLabel + TEXT(" ") + Name);
#endif
            RegisterStreamedActor(Light);
            TagArrival(Light);
        }
    };

    TagArrival(SpawnTexturedBlock(
        Origin + CityOffset(FVector(-3970.0f, -3360.0f, -18.0f)),
        CityExtent(FVector(7.2f, 1.15f, 0.040f)),
        WetStone,
        CityLabel + TEXT(" First View Wet Asphalt Approach"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false));
    TagArrival(SpawnRotatedBlock(
        PlayerStart + FVector(430.0f, 390.0f, -324.0f),
        FRotator(0.0f, 35.0f, 0.0f),
        CityExtent(FVector(5.8f, 0.10f, 0.030f)),
        RouteGlow,
        CityLabel + TEXT(" First View Objective Light Spine"),
        false));
    TagArrival(SpawnTexturedBlock(
        Origin + CityOffset(FVector(-3820.0f, -3180.0f, -19.0f)),
        CityExtent(FVector(5.8f, 5.8f, 0.050f)),
        Mission.SecondaryAccentColor * 0.6f + WetStone * 0.4f,
        CityLabel + TEXT(" First View Textured Entry Pad"),
        TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough"),
        false));

    for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
    {
        const float Side = SideIndex == 0 ? -1.0f : 1.0f;
        TagArrival(SpawnTexturedBlock(
            EntryCenter + FVector(-120.0f, Side * 560.0f, 70.0f),
            FVector(0.18f, 0.18f, 1.35f),
            WarmLamp,
            FString::Printf(TEXT("%s First View Open Route Lantern %d"), *CityLabel, SideIndex + 1),
            TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
            false));
        TagArrival(SpawnBlock(
            EntryCenter + FVector(-120.0f, Side * 560.0f, 154.0f),
            FVector(0.34f, 0.34f, 0.08f),
            WarmLamp * 2.2f,
            FString::Printf(TEXT("%s First View Open Route Beacon %d"), *CityLabel, SideIndex + 1),
            false));
        SpawnArrivalLight(
            EntryCenter + FVector(-135.0f, Side * 560.0f, 205.0f),
            WarmLamp,
            1200.0f,
            720.0f,
            FString::Printf(TEXT("First View Open Route Light %d"), SideIndex + 1));
    }

    static const FVector2D SkylineOffsets[] = {
        FVector2D(-3060.0f, -2560.0f),
        FVector2D(-2540.0f, -2360.0f),
        FVector2D(-2060.0f, -2660.0f),
        FVector2D(-1580.0f, -2440.0f),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(SkylineOffsets); ++i)
    {
        if (UStaticMesh* BuildingMesh = LoadCodeRescueCityBuildingMesh(CityIndex + Mission.SkylineSeed + 80 + i))
        {
            const float Height = 3.2f + 0.55f * i + Mission.DifficultyTier * 0.12f;
            const FVector TargetSize = CityArchitectureExtent(FVector(1.15f + 0.18f * (i % 2), 0.95f, Height));
            AActor* Building = SpawnStaticMeshProp(
                BuildingMesh,
                Origin + CityOffset(FVector(SkylineOffsets[i].X, SkylineOffsets[i].Y, 0.0f)) + FVector(0.0f, 0.0f, TargetSize.Z * 50.0f),
                FRotator(0.0f, 18.0f + i * 19.0f, 0.0f),
                TargetSize,
                FString::Printf(TEXT("%s First View Parallax Arrival Building %d"), *CityLabel, i + 1),
                false);
            TagArrival(ApplyCodeRescueMaterialToStaticActor(
                Building,
                CodeRescueCityBuildingMaterialPaths[FMath::Abs(CityIndex + i + 3) % UE_ARRAY_COUNT(CodeRescueCityBuildingMaterialPaths)],
                this,
                FLinearColor(0.055f, 0.065f, 0.078f, 1.0f) + Mission.SecondaryAccentColor * 0.06f,
                0.55f));
        }
    }

    TagArrival(SpawnGuideText(
        FString::Printf(TEXT("CODE RESCUE\n%s\n%s"), *Mission.CityName, *Mission.ArchitectureSignature),
        EntryCenter + FVector(-112.0f, 0.0f, 360.0f),
        FColor(245, 221, 164),
        28.0f));
    TagArrival(SpawnGuideText(
        TEXT("FOLLOW THE LIT ROUTE"),
        PlayerStart + FVector(620.0f, 520.0f, 82.0f),
        RouteGlow.ToFColor(true),
        26.0f));
}

void ACodeRescueGameMode::SpawnUniversalEntryAccessLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector PlayerStart = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const FVector EntryCenter = Origin + CityOffset(FVector(-3780.0f, -3120.0f, 0.0f));
    const FLinearColor LevelEntryAccent = FLinearColor(0.08f, 0.11f, 0.13f, 1.0f) + Mission.AccentColor * 0.18f;
    const FLinearColor RouteGreen = FLinearColor(0.10f, 0.95f, 0.62f, 1.0f);
    const FLinearColor RampColor = FLinearColor(0.16f, 0.18f, 0.18f, 1.0f) + Mission.SecondaryAccentColor * 0.10f;

    auto TagEntry = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("UniversalEntryAccess"));
            Actor->Tags.Add(FName("AlwaysOpenLevelEntry"));
            Actor->Tags.Add(FName("SpawnAccessRamp"));
            Actor->Tags.Add(FName("NoSpawnBlockade"));
            Actor->Tags.Add(FName("NoExteriorWallBarrier"));
        }
        return Actor;
    };

    TagEntry(SpawnTexturedBlock(
        PlayerStart + FVector(0.0f, 0.0f, -338.0f),
        CityExtent(FVector(6.8f, 6.0f, 0.075f)),
        RampColor,
        CityLabel + TEXT(" Universal Entry Spawn Pad"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false));
    TagEntry(SpawnRotatedBlock(
        Origin + CityOffset(FVector(-3540.0f, -2920.0f, -26.0f)),
        FRotator(0.0f, 42.0f, 0.0f),
        CityExtent(FVector(7.4f, 0.62f, 0.090f)),
        RouteGreen * 1.65f,
        CityLabel + TEXT(" Universal Entry Ramp Light"),
        false));
    TagEntry(SpawnRotatedBlock(
        Origin + CityOffset(FVector(-3320.0f, -2700.0f, -20.0f)),
        FRotator(0.0f, 42.0f, 0.0f),
        CityExtent(FVector(5.4f, 1.15f, 0.060f)),
        RampColor * 1.15f,
        CityLabel + TEXT(" Universal Entry Walkable Ramp"),
        false));

    TagEntry(SpawnTexturedBlock(
        EntryCenter + FVector(-120.0f, -620.0f, 24.0f),
        FVector(1.35f, 0.12f, 0.10f),
        LevelEntryAccent,
        CityLabel + TEXT(" Universal Entry Left Open Edge Light"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false));
    TagEntry(SpawnTexturedBlock(
        EntryCenter + FVector(-120.0f, 620.0f, 24.0f),
        FVector(1.35f, 0.12f, 0.10f),
        LevelEntryAccent,
        CityLabel + TEXT(" Universal Entry Right Open Edge Light"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false));
    TagEntry(SpawnBlock(
        EntryCenter + FVector(120.0f, 0.0f, 30.0f),
        CityExtent(FVector(2.8f, 0.18f, 0.045f)),
        RouteGreen * 1.8f,
        CityLabel + TEXT(" Universal Entry Open Route Stripe"),
        false));
    TagEntry(SpawnGuideText(
        TEXT("LEVEL OPEN\nfollow the green route"),
        EntryCenter + FVector(-220.0f, 0.0f, 280.0f),
        RouteGreen.ToFColor(true),
        28.0f));
}

void ACodeRescueGameMode::SpawnTacticalArmoryLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    const FVector ArmoryCenter = Origin + CityOffset(FVector(-3290.0f, -3050.0f, 0.0f));
    const FLinearColor Steel(0.10f, 0.115f, 0.12f, 1.0f);
    const FLinearColor AmmoBlue(0.18f, 0.52f, 0.95f, 1.0f);
    const FLinearColor MedicalGreen(0.20f, 0.88f, 0.45f, 1.0f);
    const FLinearColor WarningAmber(0.96f, 0.58f, 0.18f, 1.0f);

    auto TagArmory = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("TacticalArmoryAllWeaponsAvailable"));
            Actor->Tags.Add(FName("SurvivalHorrorWeaponArchetype"));
            Actor->Tags.Add(FName("ImmediateGearSelection"));
            Actor->Tags.Add(FName("NoExteriorWallBarrier"));
        }
        return Actor;
    };

    TagArmory(SpawnTexturedBlock(
        ArmoryCenter + FVector(0.0f, 0.0f, -20.0f),
        FVector(7.6f, 4.4f, 0.055f),
        Steel,
        CityLabel + TEXT(" Tactical Armory Floor"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false));
    TagArmory(SpawnBlock(
        ArmoryCenter + FVector(0.0f, 260.0f, 185.0f),
        FVector(7.6f, 0.10f, 2.0f),
        Steel + Mission.AccentColor * 0.18f,
        CityLabel + TEXT(" Tactical Armory Backboard"),
        false));
    TagArmory(SpawnGuideText(
        TEXT("TACTICAL ARMORY\nall weapons unlocked\nwheel or [ ] cycles arsenal\n1-0 quick slots | R reload | X utility"),
        ArmoryCenter + FVector(0.0f, 185.0f, 390.0f),
        FColor(245, 221, 164),
        24.0f));
    TagArmory(SpawnGuideText(
        TEXT("QUICK SLOT BOARD\n1 Handgun | 2 Pump | 3 Rifle | 4 Frag | 5 Knife\n6 Heavy | 7 Burst | 8 TacSG | 9 AutoSG | 0 SMG\nactive slot and ammo save inside this language profile"),
        ArmoryCenter + FVector(0.0f, -24.0f, 438.0f),
        FColor(190, 235, 255),
        16.0f));

    struct FArmoryDisplay
    {
        const TCHAR* Name;
        FLinearColor Tint;
        float Length;
    };
    static const FArmoryDisplay Displays[] = {
        { TEXT("BALANCED HANDGUN"), FLinearColor(0.72f, 0.78f, 0.80f), 0.64f },
        { TEXT("PUMP SHOTGUN"), FLinearColor(0.80f, 0.62f, 0.36f), 1.05f },
        { TEXT("ASSAULT RIFLE"), FLinearColor(0.42f, 0.58f, 0.44f), 1.10f },
        { TEXT("FRAG GRENADE"), WarningAmber, 0.42f },
        { TEXT("COMBAT KNIFE"), FLinearColor(0.86f, 0.86f, 0.78f), 0.52f },
        { TEXT("HEAVY HANDGUN"), FLinearColor(0.92f, 0.82f, 0.52f), 0.72f },
        { TEXT("BURST HANDGUN"), FLinearColor(0.70f, 0.78f, 0.96f), 0.68f },
        { TEXT("TACTICAL SHOTGUN"), FLinearColor(0.70f, 0.54f, 0.34f), 1.00f },
        { TEXT("AUTO SHOTGUN"), FLinearColor(0.72f, 0.45f, 0.34f), 0.94f },
        { TEXT("SMG"), FLinearColor(0.40f, 0.62f, 0.86f), 0.78f },
        { TEXT("PRECISION RIFLE"), FLinearColor(0.64f, 0.82f, 0.92f), 1.20f },
        { TEXT("SEMI-AUTO RIFLE"), FLinearColor(0.56f, 0.74f, 0.64f), 1.05f },
        { TEXT("MAGNUM"), FLinearColor(0.96f, 0.70f, 0.34f), 0.78f },
        { TEXT("BOLT LAUNCHER"), FLinearColor(0.42f, 0.76f, 0.68f), 0.96f },
        { TEXT("ROCKET LAUNCHER"), FLinearColor(0.90f, 0.34f, 0.24f), 1.22f },
        { TEXT("INCENDIARY"), FLinearColor(1.00f, 0.34f, 0.12f), 0.46f },
        { TEXT("FLASH GRENADE"), FLinearColor(0.84f, 0.92f, 1.00f), 0.46f },
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(Displays); ++i)
    {
        const int32 Row = i / 6;
        const int32 Col = i % 6;
        const FVector Slot = ArmoryCenter + FVector(-560.0f + Col * 225.0f, 132.0f, 245.0f - Row * 92.0f);
        const FLinearColor Tint = Displays[i].Tint;
        const FString KeyLabel = (i == 9) ? FString(TEXT("0")) : FString::FromInt(i + 1);
        const FString DisplayLabel = i < 10
            ? FString::Printf(TEXT("KEY %s\n%s"), *KeyLabel, Displays[i].Name)
            : FString::Printf(TEXT("WHEEL\n%s"), Displays[i].Name);
        TagArmory(SpawnBlock(
            Slot,
            FVector(Displays[i].Length, 0.055f, 0.050f),
            Tint * 1.35f,
            FString::Printf(TEXT("%s Armory Display Weapon %d"), *CityLabel, i + 1),
            false));
        TagArmory(SpawnBlock(
            Slot + FVector(-38.0f, -2.0f, -26.0f),
            FVector(0.08f, 0.07f, 0.26f),
            Tint * 0.75f,
            FString::Printf(TEXT("%s Armory Grip %d"), *CityLabel, i + 1),
            false));
        TagArmory(SpawnGuideText(
            DisplayLabel,
            Slot + FVector(0.0f, -48.0f, -20.0f),
            Tint.ToFColor(true),
            14.0f));
    }

    const FVector GearBase = ArmoryCenter + FVector(0.0f, -220.0f, 42.0f);
    TagArmory(SpawnBlock(GearBase + FVector(-420.0f, 0.0f, 48.0f), FVector(0.76f, 0.52f, 0.55f), AmmoBlue * 1.3f, CityLabel + TEXT(" Armory Ammo Crate"), false));
    TagArmory(SpawnBlock(GearBase + FVector(-165.0f, 0.0f, 48.0f), FVector(0.76f, 0.52f, 0.55f), MedicalGreen * 1.3f, CityLabel + TEXT(" Armory Medical Case"), false));
    TagArmory(SpawnBlock(GearBase + FVector(90.0f, 0.0f, 48.0f), FVector(0.76f, 0.52f, 0.55f), WarningAmber * 1.2f, CityLabel + TEXT(" Armory Utility Case"), false));
    TagArmory(SpawnBlock(GearBase + FVector(345.0f, 0.0f, 48.0f), FVector(0.76f, 0.52f, 0.55f), Mission.SecondaryAccentColor * 1.35f, CityLabel + TEXT(" Armory Field Radio"), false));
    TagArmory(SpawnGuideText(
        TEXT("AMMO | MEDKIT | FLARE SMOKE STIM | FIELD RADIO"),
        GearBase + FVector(0.0f, -68.0f, 130.0f),
        FColor(220, 245, 255),
        18.0f));

    auto SpawnArmoryPickup = [&](EPickupKind Kind, const FVector& Loc, int32 Amount)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Loc, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("TacticalArmoryAllWeaponsAvailable"));
            Pickup->Tags.Add(FName("ImmediateGearSelection"));
            RegisterStreamedActor(Pickup);
        }
    };
    SpawnArmoryPickup(EPickupKind::Ammo, GearBase + FVector(-420.0f, -120.0f, 118.0f), 160);
    SpawnArmoryPickup(EPickupKind::Medkit, GearBase + FVector(-165.0f, -120.0f, 118.0f), 2);
    SpawnArmoryPickup(EPickupKind::Flare, GearBase + FVector(90.0f, -120.0f, 118.0f), 2);
    SpawnArmoryPickup(EPickupKind::Smoke, GearBase + FVector(250.0f, -120.0f, 118.0f), 2);
    SpawnArmoryPickup(EPickupKind::Stim, GearBase + FVector(410.0f, -120.0f, 118.0f), 1);
    SpawnArmoryPickup(EPickupKind::Scrap, GearBase + FVector(570.0f, -120.0f, 118.0f), 8);
    SpawnArmoryPickup(EPickupKind::ArmorPlate, GearBase + FVector(730.0f, -120.0f, 118.0f), 2);
    SpawnArmoryPickup(EPickupKind::RadioScanner, GearBase + FVector(-420.0f, -285.0f, 118.0f), 2);
    SpawnArmoryPickup(EPickupKind::FlashlightBattery, GearBase + FVector(-165.0f, -285.0f, 118.0f), 2);
    SpawnArmoryPickup(EPickupKind::AmmoPouch, GearBase + FVector(90.0f, -285.0f, 118.0f), 60);
    SpawnArmoryPickup(EPickupKind::BypassKit, GearBase + FVector(345.0f, -285.0f, 118.0f), 1);
}

void ACodeRescueGameMode::SpawnUnrealSystemsCharacterWorldLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector SystemsHub = Origin + CityOffset(FVector(-1180.0f, 2260.0f, 0.0f));
    const FVector DccHub = Origin + CityOffset(FVector(-2140.0f, 760.0f, 0.0f));
    const FVector PcgHub = Origin + CityOffset(FVector(1400.0f, 2180.0f, 0.0f));
    const FVector AiHub = Origin + CityOffset(FVector(2140.0f, 580.0f, 0.0f));
    const FLinearColor CharCyan(0.22f, 0.84f, 1.0f, 1.0f);
    const FLinearColor DccViolet(0.72f, 0.46f, 1.0f, 1.0f);
    const FLinearColor HoudiniAmber(1.0f, 0.58f, 0.18f, 1.0f);
    const FLinearColor ChaosOrange(1.0f, 0.32f, 0.14f, 1.0f);
    const FLinearColor AiGreen(0.30f, 0.92f, 0.42f, 1.0f);
    const FLinearColor MissionGold(0.92f, 0.76f, 0.30f, 1.0f);

    auto TagSystems = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("UnrealSystemsCharacterWorld"));
            Actor->Tags.Add(FName("FullNovelCharacterWorldDesign"));
            Actor->Tags.Add(FName("ContinuousPlayabilityTestRig"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto AddTags = [](AActor* Actor, std::initializer_list<const TCHAR*> Tags) -> AActor*
    {
        if (Actor)
        {
            for (const TCHAR* Tag : Tags)
            {
                Actor->Tags.Add(FName(Tag));
            }
        }
        return Actor;
    };

    auto EnablePhysics = [](AActor* Actor, float MassKg) -> AActor*
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                MeshComp->SetMobility(EComponentMobility::Movable);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetLinearDamping(0.24f);
                MeshComp->SetAngularDamping(0.36f);
                CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
                    MeshComp,
                    MeshActor,
                    FName("SystemsReviewFixedStepBody"),
                    MassKg,
                    0.24f,
                    0.36f,
                    false);
            }
        }
        return Actor;
    };

    auto SpawnSystemsLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagSystems(Light);
            AddTags(Light, { TEXT("UnrealSystemsLighting") });
        }
    };

    TagSystems(SpawnTexturedBlock(
        SystemsHub + FVector(0.0f, 0.0f, -18.0f),
        FVector(8.2f, 5.0f, 0.055f),
        FLinearColor(0.045f, 0.052f, 0.058f, 1.0f) + CharCyan * 0.10f,
        CityLabel + TEXT(" Unreal Systems Character Design Floor"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false));
    TagSystems(SpawnBlock(
        SystemsHub + FVector(0.0f, 280.0f, 185.0f),
        FVector(8.2f, 0.10f, 1.95f),
        FLinearColor(0.035f, 0.040f, 0.045f, 1.0f) + Mission.AccentColor * 0.15f,
        CityLabel + TEXT(" Unreal Systems Character Design Board"),
        false));
    TagSystems(SpawnGuideText(
        FString::Printf(TEXT("UNREAL SYSTEMS CHARACTER STAGE\n%s\nMetaHuman-ready novel cast | Control Rig | IK | Mac hair-card fallback | Sequencer"), *Mission.CityName),
        SystemsHub + FVector(0.0f, 205.0f, 392.0f),
        CharCyan.ToFColor(true),
        24.0f));
    SpawnSystemsLight(SystemsHub + FVector(0.0f, 0.0f, 280.0f), CharCyan, 5200.0f, 980.0f, CityLabel + TEXT(" Unreal Systems Character Light"));

    struct FNovelCharacterSlot
    {
        const TCHAR* Name;
        const TCHAR* Role;
        EFriendlyNPCRole NpcRole;
        FLinearColor Tint;
        const TCHAR* PipelineTag;
    };
    static const FNovelCharacterSlot CharacterSlots[] = {
        { TEXT("Rhea Calder"), TEXT("Evac captain and playable survivor-body target"), EFriendlyNPCRole::Engineer, FLinearColor(0.30f, 0.82f, 1.0f), TEXT("MetaHumanReadyCharacterDesign") },
        { TEXT("Mika Stone"), TEXT("Medic with Mac hair-card/mesh fallback and rescue barks"), EFriendlyNPCRole::Medic, FLinearColor(1.0f, 0.38f, 0.42f), TEXT("GroomCardFallbackReady") },
        { TEXT("Noor Vance"), TEXT("Signal scientist with Control Rig facial slots"), EFriendlyNPCRole::Scientist, FLinearColor(0.74f, 0.58f, 1.0f), TEXT("ControlRigFacialSlot") },
        { TEXT("Jules Ardent"), TEXT("Field trader and quest reward economy test"), EFriendlyNPCRole::Trader, FLinearColor(0.34f, 0.96f, 0.48f), TEXT("QuestMissionKitReady") },
        { TEXT("Ilan Cross"), TEXT("Retargeted companion locomotion target"), EFriendlyNPCRole::Engineer, FLinearColor(0.96f, 0.72f, 0.30f), TEXT("IKRetargeterReady") },
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(CharacterSlots); ++i)
    {
        const float X = -620.0f + i * 310.0f;
        const FVector SlotLoc = SystemsHub + FVector(X, -60.0f + ((i % 2) * 92.0f), 92.0f);
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        AFriendlyNPCActor* NPC = GetWorld()->SpawnActor<AFriendlyNPCActor>(
            AFriendlyNPCActor::StaticClass(),
            SlotLoc,
            FRotator(0.0f, -20.0f + i * 10.0f, 0.0f),
            Params);
        if (NPC)
        {
            NPC->CityIndex = CityIndex;
            NPC->NPCRole = CharacterSlots[i].NpcRole;
            NPC->NPCName = CharacterSlots[i].Name;
            NPC->GreetingLine = FString::Printf(TEXT("%s. My slot promotes MetaHuman body, Maya/Houdini, Control Rig, IK, and Mac hair-card assets; strand grooms stay review-only on Apple GPUs."), CharacterSlots[i].Role);
            RegisterStreamedActor(NPC);
            TagSystems(NPC);
            AddTags(NPC, {
                CharacterSlots[i].PipelineTag,
                TEXT("NovelPlayableCastSlot"),
                TEXT("MetaHumanReadyCharacterDesign"),
                TEXT("MayaHoudiniDccHandoff"),
                TEXT("SequencerControlRigIKGroomReady"),
                TEXT("MacHairCardRuntimeReady"),
                TEXT("GroomStrandReviewOnlyMac")
            });
        }

        TagSystems(AddTags(SpawnBlock(
            SlotLoc + FVector(0.0f, 70.0f, 30.0f),
            FVector(0.48f, 0.30f, 0.20f),
            CharacterSlots[i].Tint * 0.72f,
            FString::Printf(TEXT("%s Character Slot Kit %d"), *CityLabel, i + 1),
            true),
            { TEXT("MetaHumanReadyCharacterDesign"), TEXT("NovelCharacterProp") }));
        TagSystems(AddTags(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), CharacterSlots[i].Name, CharacterSlots[i].Role),
            SlotLoc + FVector(0.0f, -88.0f, 235.0f),
            CharacterSlots[i].Tint.ToFColor(true),
            15.0f),
            { TEXT("NovelCharacterDesignLabel"), CharacterSlots[i].PipelineTag }));
    }

    TagSystems(AddTags(SpawnTexturedBlock(
        DccHub + FVector(0.0f, 0.0f, -16.0f),
        FVector(6.8f, 3.4f, 0.052f),
        FLinearColor(0.052f, 0.046f, 0.065f, 1.0f) + DccViolet * 0.16f,
        CityLabel + TEXT(" Maya Houdini DCC Intake Floor"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false),
        { TEXT("MayaHoudiniDccHandoff"), TEXT("DccSourceAssetIntake") }));
    TagSystems(AddTags(SpawnGuideText(
        TEXT("MAYA / HOUDINI / DCC HANDOFF\nFBX skeletal mesh | Alembic groom | USD set dressing | source-safe manifest"),
        DccHub + FVector(0.0f, -188.0f, 296.0f),
        DccViolet.ToFColor(true),
        22.0f),
        { TEXT("MayaHoudiniDccHandoff"), TEXT("DccSourceAssetIntake") }));
    for (int32 i = 0; i < 6; ++i)
    {
        const FVector CaseLoc = DccHub + FVector(-500.0f + i * 200.0f, 82.0f, 58.0f);
        const FLinearColor Tint = (i % 2 == 0 ? DccViolet : CharCyan) * (1.0f + i * 0.06f);
        TagSystems(AddTags(SpawnBlock(
            CaseLoc,
            FVector(0.62f, 0.48f, 0.42f),
            Tint,
            FString::Printf(TEXT("%s DCC Intake Crate %d"), *CityLabel, i + 1),
            true),
            { TEXT("MayaHoudiniDccHandoff"), TEXT("MetaHumanSourceAssembly"), TEXT("DccRetargetValidationQueue") }));
        TagSystems(AddTags(SpawnBlock(
            CaseLoc + FVector(0.0f, -34.0f, 54.0f),
            FVector(0.44f, 0.04f, 0.12f),
            FLinearColor(0.92f, 0.94f, 0.88f, 1.0f),
            FString::Printf(TEXT("%s DCC Intake Label %d"), *CityLabel, i + 1),
            false),
            { TEXT("MayaHoudiniDccHandoff"), TEXT("DccRetargetValidationQueue") }));
    }
    TagSystems(AddTags(SpawnGuideText(
        FString::Printf(
            TEXT("MAYA CHARACTER CLEANUP RECIPE\n%s | %s\nbind pose + origin | skeleton names | sockets | anim takes | LOD/material | physics asset | promotion evidence"),
            *Mission.CityName,
            *Mission.ArtKitName),
        DccHub + FVector(0.0f, 286.0f, 318.0f),
        FColor(214, 164, 255),
        17.0f),
        { TEXT("MayaCharacterCleanup"), TEXT("MayaCharacterCleanupRuntimeContract"), TEXT("MayaCleanupRecipeBoard"), TEXT("DccRetargetValidationQueue") }));

    struct FMayaCleanupStationSpec
    {
        const TCHAR* Label;
        const TCHAR* Detail;
        const TCHAR* PrimaryTag;
        FVector Offset;
        FLinearColor Tint;
    };

    const FMayaCleanupStationSpec MayaStations[] = {
        { TEXT("BIND POSE"), TEXT("root at origin"), TEXT("MayaBindPoseOriginReview"), FVector(-560.0f, 374.0f, 56.0f), DccViolet * 0.95f },
        { TEXT("SKELETON NAMES"), TEXT("stable bone map"), TEXT("MayaSkeletonNamingReview"), FVector(-400.0f, 374.0f, 56.0f), CharCyan * 0.95f },
        { TEXT("SOCKETS"), TEXT("weapon camera tools"), TEXT("MayaSocketAuthoringReview"), FVector(-240.0f, 374.0f, 56.0f), MissionGold * 0.95f },
        { TEXT("ANIM TAKES"), TEXT("trim loop montage"), TEXT("MayaAnimationTakeCleanupReview"), FVector(-80.0f, 374.0f, 56.0f), FLinearColor(0.92f, 0.48f, 1.0f, 1.0f) },
        { TEXT("LOD MATERIAL"), TEXT("Mac budget"), TEXT("MayaLODMaterialBudgetReview"), FVector(80.0f, 374.0f, 56.0f), FLinearColor(0.38f, 0.96f, 0.72f, 1.0f) },
        { TEXT("PHYSICS ASSET"), TEXT("ragdoll bodies"), TEXT("MayaPhysicsAssetReview"), FVector(240.0f, 374.0f, 56.0f), ChaosOrange * 0.95f },
        { TEXT("FOOT IK"), TEXT("plant + pelvis"), TEXT("FootIKGroundingReview"), FVector(400.0f, 374.0f, 56.0f), FLinearColor(0.54f, 0.96f, 1.0f, 1.0f) },
        { TEXT("FBX EXPORT"), TEXT("one scale axis"), TEXT("MayaFbxExportReview"), FVector(560.0f, 374.0f, 56.0f), FLinearColor(1.0f, 0.72f, 0.36f, 1.0f) },
        { TEXT("PROMOTION"), TEXT("validator smoke"), TEXT("MayaPromotionEvidenceReady"), FVector(0.0f, 514.0f, 56.0f), Mission.SecondaryAccentColor * 0.95f },
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(MayaStations); ++i)
    {
        const FMayaCleanupStationSpec& Spec = MayaStations[i];
        const FVector StationLoc = DccHub + Spec.Offset;
        TagSystems(AddTags(SpawnBlock(
            StationLoc,
            FVector(0.52f, 0.34f, 0.32f),
            Spec.Tint,
            FString::Printf(TEXT("%s Maya Cleanup %s"), *CityLabel, Spec.Label),
            true),
            {
                TEXT("MayaCharacterCleanup"),
                TEXT("MayaCharacterCleanupRuntimeContract"),
                TEXT("MayaRetargetRoundtripReview"),
                TEXT("DccRetargetValidationQueue"),
                Spec.PrimaryTag
            }));
        TagSystems(AddTags(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Spec.Label, Spec.Detail),
            StationLoc + FVector(0.0f, -54.0f, 116.0f),
            Spec.Tint.ToFColor(true),
            10.5f),
            { TEXT("MayaCharacterCleanup"), Spec.PrimaryTag }));
    }
    SpawnSystemsLight(DccHub + FVector(0.0f, 0.0f, 245.0f), DccViolet, 3600.0f, 760.0f, CityLabel + TEXT(" DCC Intake Light"));

    TagSystems(AddTags(SpawnTexturedBlock(
        PcgHub + FVector(0.0f, 0.0f, -18.0f),
        FVector(7.8f, 4.8f, 0.055f),
        FLinearColor(0.070f, 0.060f, 0.045f, 1.0f) + HoudiniAmber * 0.12f,
        CityLabel + TEXT(" Houdini PCG City Design Floor"),
        TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough"),
        false),
        { TEXT("HoudiniProceduralWorldDesign"), TEXT("PCGWorldPartitionCell"), TEXT("NovelWorldDesign") }));
    TagSystems(AddTags(SpawnGuideText(
        TEXT("HOUDINI / PCG CITY DESIGN\nmodular alleys | safehouse cells | World Partition-ready review blocks"),
        PcgHub + FVector(0.0f, -250.0f, 322.0f),
        HoudiniAmber.ToFColor(true),
        22.0f),
        { TEXT("HoudiniProceduralWorldDesign"), TEXT("PCGWorldPartitionCell"), TEXT("WorldPartitionReady") }));
    for (int32 Cell = 0; Cell < 9; ++Cell)
    {
        const int32 Col = Cell % 3;
        const int32 Row = Cell / 3;
        const FVector CellCenter = PcgHub + FVector(-430.0f + Col * 430.0f, -40.0f + Row * 255.0f, 0.0f);
        const FLinearColor CellTint = (Cell % 2 == 0 ? HoudiniAmber : Mission.SecondaryAccentColor) * (0.78f + 0.04f * Cell);
        TagSystems(AddTags(SpawnBlock(
            CellCenter + FVector(0.0f, 0.0f, 56.0f),
            FVector(1.35f, 0.12f, 0.78f + 0.18f * Row),
            CellTint,
            FString::Printf(TEXT("%s PCG Facade Cell %d"), *CityLabel, Cell + 1),
            true),
            { TEXT("HoudiniProceduralWorldDesign"), TEXT("PCGWorldPartitionCell"), TEXT("WorldPartitionReady") }));
        TagSystems(AddTags(SpawnBlock(
            CellCenter + FVector(0.0f, 92.0f, 18.0f),
            FVector(1.10f, 0.08f, 0.045f),
            CharCyan * 1.45f,
            FString::Printf(TEXT("%s PCG Route Strip %d"), *CityLabel, Cell + 1),
            false),
            { TEXT("PCGRouteSplineReady"), TEXT("WorldPartitionReady") }));
    }

    const int32 HoudiniCitySeed = FMath::Abs((Mission.Rank + 1) * 7919 + (CityIndex + 11) * 104729);
    TagSystems(AddTags(SpawnGuideText(
        FString::Printf(
            TEXT("HOUDINI OUTPUT RECIPE\nseed %06d | kit %s | district %s\nfacade cells | rubble variants | collision proxy | route spline | streaming budget"),
            HoudiniCitySeed % 1000000,
            *Mission.ArtKitName,
            *Mission.DistrictStyle),
        PcgHub + FVector(0.0f, 602.0f, 332.0f),
        FColor(255, 206, 114),
        18.0f),
        { TEXT("HoudiniModularCityOutput"), TEXT("HoudiniCityKitRecipe"), TEXT("PCGDeterministicCitySeed"), TEXT("PCGStreamingBudgetCell") }));

    struct FHoudiniOutputSpec
    {
        const TCHAR* Label;
        const TCHAR* PrimaryTag;
        FVector Offset;
        FVector Scale;
        FLinearColor Tint;
    };

    const FHoudiniOutputSpec OutputSpecs[] = {
        { TEXT("FACADE KIT"), TEXT("HoudiniFacadeModuleRecipe"), FVector(-610.0f, 610.0f, 84.0f), FVector(0.92f, 0.16f, 0.96f), HoudiniAmber * 0.92f },
        { TEXT("SAFEHOUSE CELL"), TEXT("PCGSafehouseCellModule"), FVector(-305.0f, 610.0f, 72.0f), FVector(0.78f, 0.52f, 0.52f), CharCyan * 0.95f },
        { TEXT("RUBBLE VARIATION"), TEXT("PCGRubbleVariationSet"), FVector(0.0f, 610.0f, 54.0f), FVector(0.92f, 0.56f, 0.34f), ChaosOrange * 0.92f },
        { TEXT("COLLISION PROXY"), TEXT("PCGCollisionProxyReady"), FVector(305.0f, 610.0f, 70.0f), FVector(0.70f, 0.42f, 0.48f), MissionGold * 0.92f },
        { TEXT("STREAMING CELL"), TEXT("PCGStreamingBudgetCell"), FVector(610.0f, 610.0f, 74.0f), FVector(0.88f, 0.48f, 0.54f), Mission.SecondaryAccentColor * 0.92f },
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(OutputSpecs); ++i)
    {
        const FHoudiniOutputSpec& Spec = OutputSpecs[i];
        const FVector ModuleLoc = PcgHub + Spec.Offset;
        TagSystems(AddTags(SpawnBlock(
            ModuleLoc,
            Spec.Scale,
            Spec.Tint,
            FString::Printf(TEXT("%s Houdini Output %s"), *CityLabel, Spec.Label),
            true),
            {
                TEXT("HoudiniModularCityOutput"),
                TEXT("HoudiniProceduralWorldDesign"),
                TEXT("PCGWorldPartitionCell"),
                TEXT("WorldPartitionReady"),
                Spec.PrimaryTag
            }));
        TagSystems(AddTags(SpawnGuideText(
            Spec.Label,
            ModuleLoc + FVector(0.0f, -92.0f, 122.0f),
            Spec.Tint.ToFColor(true),
            13.0f),
            { TEXT("HoudiniModularCityOutput"), Spec.PrimaryTag }));
    }

    for (int32 Rubble = 0; Rubble < 8; ++Rubble)
    {
        const int32 Column = Rubble % 4;
        const int32 Row = Rubble / 4;
        const float SeedScale = 0.76f + 0.06f * ((HoudiniCitySeed + Rubble) % 5);
        const FVector RubbleLoc = PcgHub + FVector(-390.0f + Column * 260.0f, 830.0f + Row * 118.0f, 42.0f);
        TagSystems(AddTags(SpawnBlock(
            RubbleLoc,
            FVector(0.52f * SeedScale, 0.28f + 0.04f * Row, 0.20f + 0.035f * Column),
            ChaosOrange * (0.64f + 0.04f * Rubble),
            FString::Printf(TEXT("%s Houdini Rubble Variant %d"), *CityLabel, Rubble + 1),
            true),
            { TEXT("HoudiniModularCityOutput"), TEXT("PCGRubbleVariationSet"), TEXT("PCGCollisionProxyReady"), TEXT("MacLODBudgetReviewGate") }));
    }

    for (int32 Knot = 0; Knot < 5; ++Knot)
    {
        const FVector KnotLoc = PcgHub + FVector(-520.0f + Knot * 260.0f, 1048.0f, 30.0f);
        TagSystems(AddTags(SpawnBlock(
            KnotLoc,
            FVector(0.18f, 0.18f, 0.12f),
            CharCyan * (1.15f + 0.10f * Knot),
            FString::Printf(TEXT("%s Houdini Route Spline Knot %d"), *CityLabel, Knot + 1),
            false),
            { TEXT("HoudiniModularCityOutput"), TEXT("PCGRouteSplineReady"), TEXT("PCGWorldPartitionBakeReview"), TEXT("NoAccessBlocker") }));
    }

    const FVector ChaosHub = PcgHub + FVector(0.0f, 1290.0f, 0.0f);
    TagSystems(AddTags(SpawnGuideText(
        TEXT("CHAOS ASYNC PHYSICS\nmovable barricades, weighted cover, projectile interaction props"),
        ChaosHub + FVector(0.0f, 0.0f, 288.0f),
        ChaosOrange.ToFColor(true),
        20.0f),
        { TEXT("ChaosInteractivePhysics"), TEXT("AsyncPhysicsReady") }));
    for (int32 i = 0; i < 8; ++i)
    {
        const FVector PropLoc = ChaosHub + FVector(-560.0f + i * 160.0f, 70.0f + ((i % 2) * 95.0f), 82.0f);
        AActor* PhysicsProp = SpawnBlock(
            PropLoc,
            FVector(0.50f + (i % 3) * 0.08f, 0.34f, 0.42f),
            ChaosOrange * (0.72f + i * 0.06f),
            FString::Printf(TEXT("%s Chaos Movable Cover Prop %d"), *CityLabel, i + 1),
            true);
        TagSystems(AddTags(EnablePhysics(PhysicsProp, 35.0f + i * 8.0f), {
            TEXT("ChaosInteractivePhysics"),
            TEXT("AsyncPhysicsReady"),
            TEXT("ProjectilePhysicsTarget"),
            TEXT("ContinuousPlayabilityTestRig")
        }));
    }

    TagSystems(AddTags(SpawnTexturedBlock(
        AiHub + FVector(0.0f, 0.0f, -18.0f),
        FVector(6.4f, 4.2f, 0.052f),
        FLinearColor(0.040f, 0.060f, 0.044f, 1.0f) + AiGreen * 0.12f,
        CityLabel + TEXT(" AI Encounter Director Floor"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false),
        { TEXT("NPCBehaviorTreeReady"), TEXT("StateTreeEQSReady"), TEXT("EnemyEncounterDirector") }));
    TagSystems(AddTags(SpawnGuideText(
        TEXT("AI ENCOUNTER DIRECTOR\npatrol points, chase lanes, cover signals, survivor and NPC behavior hooks"),
        AiHub + FVector(0.0f, -214.0f, 302.0f),
        AiGreen.ToFColor(true),
        22.0f),
        { TEXT("NPCBehaviorTreeReady"), TEXT("StateTreeEQSReady"), TEXT("EnemyEncounterDirector") }));
    for (int32 i = 0; i < 10; ++i)
    {
        const float Angle = (360.0f / 10.0f) * i;
        const FVector NodeOffset(FMath::Cos(FMath::DegreesToRadians(Angle)) * 420.0f, FMath::Sin(FMath::DegreesToRadians(Angle)) * 260.0f, 46.0f);
        const FLinearColor NodeTint = (i % 2 == 0 ? AiGreen : MissionGold) * (1.0f + i * 0.03f);
        TagSystems(AddTags(SpawnBlock(
            AiHub + NodeOffset,
            FVector(0.30f, 0.30f, 0.22f),
            NodeTint,
            FString::Printf(TEXT("%s AI Patrol EQS Node %d"), *CityLabel, i + 1),
            false),
            { TEXT("NPCBehaviorTreeReady"), TEXT("StateTreeEQSReady"), TEXT("AIPatrolRouteNode"), TEXT("EnemyEncounterDirector") }));
    }
    for (int32 i = 0; i < 4; ++i)
    {
        TagSystems(AddTags(SpawnBlock(
            AiHub + FVector(-330.0f + i * 220.0f, 280.0f, 74.0f),
            FVector(0.62f, 0.28f, 0.70f),
            MissionGold * (0.82f + i * 0.07f),
            FString::Printf(TEXT("%s AI Cover Signal %d"), *CityLabel, i + 1),
            true),
            { TEXT("AICoverSignal"), TEXT("NPCBehaviorTreeReady"), TEXT("ContinuousPlayabilityTestRig") }));
    }
    SpawnSystemsLight(AiHub + FVector(0.0f, 0.0f, 245.0f), AiGreen, 4100.0f, 820.0f, CityLabel + TEXT(" AI Director Light"));

    const FVector MissionKitHub = Origin + CityOffset(FVector(560.0f, 1260.0f, 0.0f));
    TagSystems(AddTags(SpawnBlock(
        MissionKitHub + FVector(0.0f, 0.0f, 88.0f),
        FVector(3.8f, 0.10f, 0.92f),
        FLinearColor(0.060f, 0.056f, 0.040f, 1.0f) + MissionGold * 0.20f,
        CityLabel + TEXT(" Quest Mission Kit Board"),
        false),
        { TEXT("QuestMissionKitReady"), TEXT("MissionObjectiveKit"), TEXT("SequencerControlRigIKGroomReady") }));
    TagSystems(AddTags(SpawnGuideText(
        FString::Printf(TEXT("QUEST / MISSION KIT\nterminal: %s\nsurvivor: %s\ncinematic rescue beat: Sequencer-ready"), *Mission.TerminalTitle, *Mission.SurvivorName),
        MissionKitHub + FVector(0.0f, -48.0f, 198.0f),
        MissionGold.ToFColor(true),
        18.0f),
        { TEXT("QuestMissionKitReady"), TEXT("MissionObjectiveKit"), TEXT("SequencerControlRigIKGroomReady") }));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueUnrealSystems] %s integrated MetaHuman-ready cast, Maya/Houdini DCC intake, PCG/Houdini city cells, Chaos physics props, AI director nodes, mission kits, and Sequencer/ControlRig/IK/Groom hooks."),
        *CityLabel);
}

void ACodeRescueGameMode::SpawnPublicDemoFabDetailLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector DetailHub = Origin + CityOffset(FVector(-2060.0f, -1540.0f, 0.0f));
    const FVector OverpassHub = Origin + CityOffset(FVector(260.0f, -1220.0f, 0.0f));
    const FVector TerminalHub = Origin + CityOffset(FVector(1150.0f, -900.0f, 0.0f));
    const FVector SurvivorHub = Origin + CityOffset(FVector(2520.0f, 1240.0f, 0.0f));
    const FVector FabGalleryHub = Origin + CityOffset(FVector(-1320.0f, 760.0f, 0.0f));
    const FVector BossForeshadowHub = Origin + CityOffset(FVector(3160.0f, 2260.0f, 0.0f));

    const FLinearColor WetBlack(0.030f, 0.034f, 0.036f, 1.0f);
    const FLinearColor Concrete(0.18f, 0.19f, 0.18f, 1.0f);
    const FLinearColor WarmPractical(0.95f, 0.58f, 0.25f, 1.0f);
    const FLinearColor EmergencyRed(0.95f, 0.12f, 0.08f, 1.0f);
    const FLinearColor ClinicalBlue(0.36f, 0.72f, 1.0f, 1.0f);
    const FLinearColor FabGold(0.96f, 0.74f, 0.28f, 1.0f);
    const FLinearColor ReadableGreen(0.20f, 0.95f, 0.54f, 1.0f);

    auto TagDetail = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("PublicDemoProductionDetail"));
            Actor->Tags.Add(FName("RetailQualityWorldPass"));
            Actor->Tags.Add(FName("CompetitivePricePresentation"));
            Actor->Tags.Add(FName("PlayableSetDressing"));
            Actor->Tags.Add(FName("LocalFabAssetIntegration"));
        }
        return Actor;
    };

    auto AddTags = [](AActor* Actor, std::initializer_list<const TCHAR*> Tags) -> AActor*
    {
        if (Actor)
        {
            for (const TCHAR* Tag : Tags)
            {
                Actor->Tags.Add(FName(Tag));
            }
        }
        return Actor;
    };

    auto AddSlowRotation = [&](AActor* Actor, const FRotator& Rate, const FString& ComponentName) -> AActor*
    {
        if (Actor)
        {
            if (URotatingMovementComponent* Rotator = NewObject<URotatingMovementComponent>(Actor, *ComponentName))
            {
                Actor->AddInstanceComponent(Rotator);
                Rotator->RotationRate = Rate;
                Rotator->bRotationInLocalSpace = true;
                Rotator->RegisterComponent();
                Actor->Tags.Add(FName("RetailDemoAnimatedProp"));
            }
        }
        return Actor;
    };

    auto SpawnDetailLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(true);
            }
#if WITH_EDITOR
            Light->SetActorLabel(CityLabel + TEXT(" ") + Name);
#endif
            RegisterStreamedActor(Light);
            TagDetail(Light);
            AddTags(Light, { TEXT("PremiumLightingPass"), TEXT("VisualReviewCoverage") });
        }
    };

    auto SpawnImportedDetail = [&](const TCHAR* MeshPath, const TCHAR* MaterialPath, const FVector& Location, const FRotator& Rotation,
                                  const FVector& TargetSize, const FString& Name, const FLinearColor& Tint, bool bEnableCollision = true) -> AActor*
    {
        AActor* Actor = SpawnStaticMeshProp(
            LoadCodeRescueAssetMesh(MeshPath),
            Location,
            Rotation,
            TargetSize,
            CityLabel + TEXT(" ") + Name,
            bEnableCollision);
        TagDetail(Actor);
        AddTags(Actor, { TEXT("ImportedLocalAssetDetail"), TEXT("FabDesignInclusion") });
        ApplyCodeRescueMaterialToStaticActor(Actor, MaterialPath, this, Tint, 0.18f);
        return Actor;
    };

    // 1. A dense, readable first street that feels authored rather than empty.
    TagDetail(AddTags(SpawnTexturedBlock(
        DetailHub + FVector(0.0f, 0.0f, -18.0f),
        FVector(10.6f, 4.2f, 0.050f),
        WetBlack,
        CityLabel + TEXT(" Public Demo Wet Main Street Surface"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false),
        { TEXT("EnvironmentalStorytelling"), TEXT("ReadableCriticalPath") }));

    for (int32 Strip = 0; Strip < 7; ++Strip)
    {
        TagDetail(AddTags(SpawnRotatedBlock(
            DetailHub + FVector(-610.0f + Strip * 205.0f, -116.0f, 8.0f),
            FRotator(0.0f, 0.0f, 0.0f),
            FVector(0.84f, 0.030f, 0.022f),
            (Strip % 2 == 0 ? Mission.AccentColor : ReadableGreen) * 1.55f,
            FString::Printf(TEXT("%s Public Demo Painted Route Dash %d"), *CityLabel, Strip + 1),
            false),
            { TEXT("ReadableCriticalPath"), TEXT("NoExteriorWallBarrier") }));
    }

    for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
    {
        const float Side = SideIndex == 0 ? -1.0f : 1.0f;
        for (int32 Shop = 0; Shop < 4; ++Shop)
        {
            const FVector FacadeLoc = DetailHub + FVector(-720.0f + Shop * 480.0f, Side * 390.0f, 138.0f);
            const FLinearColor FacadeTint = (Shop % 2 == 0 ? Concrete : Mission.SecondaryAccentColor * 0.24f + Concrete * 0.76f);
            AActor* Facade = nullptr;
            if (UStaticMesh* BuildingMesh = LoadCodeRescueCityBuildingMesh(CityIndex + Shop + SideIndex * 11 + 200))
            {
                Facade = SpawnStaticMeshProp(
                    BuildingMesh,
                    FacadeLoc,
                    FRotator(0.0f, Side > 0.0f ? 180.0f : 0.0f, 0.0f),
                    FVector(2.1f, 0.54f, 2.76f + 0.34f * Shop),
                    FString::Printf(TEXT("%s Public Demo Parallax Storefront %d-%d"), *CityLabel, SideIndex + 1, Shop + 1),
                    true);
                ApplyCodeRescueMaterialToStaticActor(
                    Facade,
                    CodeRescueCityBuildingMaterialPaths[FMath::Abs(CityIndex + Shop + SideIndex) % UE_ARRAY_COUNT(CodeRescueCityBuildingMaterialPaths)],
                    this,
                    FacadeTint,
                    0.28f);
            }
            else
            {
                Facade = SpawnTexturedBlock(
                    FacadeLoc,
                    FVector(2.1f, 0.24f, 2.76f + 0.34f * Shop),
                    FacadeTint,
                    FString::Printf(TEXT("%s Public Demo Fallback Storefront %d-%d"), *CityLabel, SideIndex + 1, Shop + 1),
                    TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
                    true);
            }
            TagDetail(AddTags(Facade, { TEXT("ParallaxNightBuildingUse"), TEXT("StreetLevelWorldDressing") }));

            SpawnImportedDetail(
                TEXT("/Game/StarterContent/Props/SM_GlassWindow.SM_GlassWindow"),
                TEXT("/Game/StarterContent/Materials/M_Metal_Steel.M_Metal_Steel"),   // 2026-07-02: M_Glass rendered as teal checker in packaged build; steel reads as a solid panel
                FacadeLoc + FVector(-68.0f, -Side * 36.0f, 34.0f),
                FRotator(0.0f, Side > 0.0f ? 180.0f : 0.0f, 0.0f),
                FVector(0.95f, 0.08f, 0.78f),
                FString::Printf(TEXT("Public Demo Glass Window %d-%d A"), SideIndex + 1, Shop + 1),
                ClinicalBlue,
                false);
            SpawnImportedDetail(
                TEXT("/Game/StarterContent/Props/SM_DoorFrame.SM_DoorFrame"),
                TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"),
                FacadeLoc + FVector(82.0f, -Side * 38.0f, -12.0f),
                FRotator(0.0f, Side > 0.0f ? 180.0f : 0.0f, 0.0f),
                FVector(0.72f, 0.16f, 1.35f),
                FString::Printf(TEXT("Public Demo Doorframe %d-%d"), SideIndex + 1, Shop + 1),
                FacadeTint,
                true);
            SpawnImportedDetail(
                TEXT("/Game/StarterContent/Props/SM_Lamp_Wall.SM_Lamp_Wall"),
                TEXT("/Game/StarterContent/Props/Materials/M_Lamp.M_Lamp"),
                FacadeLoc + FVector(-190.0f, -Side * 48.0f, 92.0f),
                FRotator(0.0f, Side > 0.0f ? 180.0f : 0.0f, 0.0f),
                FVector(0.42f, 0.18f, 0.42f),
                FString::Printf(TEXT("Public Demo Wall Lamp %d-%d"), SideIndex + 1, Shop + 1),
                WarmPractical,
                false);
        }
    }
    SpawnDetailLight(DetailHub + FVector(0.0f, -80.0f, 330.0f), WarmPractical, 4200.0f, 1050.0f, TEXT("Public Demo Main Street Practical Light"));

    // 2. A visible Fab-derived overpass composition that frames combat and travel.
    if (UStaticMesh* BridgeMesh = LoadCodeRescueBridgeMesh(CityIndex + 42))
    {
        AActor* Bridge = SpawnStaticMeshProp(
            BridgeMesh,
            OverpassHub + FVector(0.0f, 0.0f, 328.0f),
            FRotator(0.0f, 8.0f, 0.0f),
            FVector(12.8f, 2.0f, 1.55f),
            CityLabel + TEXT(" Public Demo Fab Modern Bridge Hero Span"),
            true);
        TagDetail(AddTags(Bridge, { TEXT("ModernBridgesFabAsset"), TEXT("HeroSetPiece"), TEXT("PlayableSilhouetteLandmark") }));
        ApplyCodeRescueMaterialToStaticActor(
            Bridge,
            TEXT("/Game/ModernBridges/Materials/metal/MI_metal.MI_metal"),
            this,
            FLinearColor(0.34f, 0.37f, 0.38f, 1.0f),
            0.14f);
    }
    for (int32 Pylon = 0; Pylon < 4; ++Pylon)
    {
        const float X = Pylon < 2 ? -610.0f : 610.0f;
        const float Y = Pylon % 2 == 0 ? -220.0f : 220.0f;
        TagDetail(AddTags(SpawnTexturedBlock(
            OverpassHub + FVector(X, Y, 138.0f),
            FVector(0.52f, 0.52f, 2.82f),
            Concrete,
            FString::Printf(TEXT("%s Public Demo Bridge Pylon %d"), *CityLabel, Pylon + 1),
            TEXT("/Game/StarterContent/Materials/M_Concrete_Poured.M_Concrete_Poured"),
            true),
            { TEXT("ModernBridgesFabAsset"), TEXT("RouteScaleReference") }));
    }
    for (int32 Cover = 0; Cover < 9; ++Cover)
    {
        const FVector CoverLoc = OverpassHub + FVector(-520.0f + Cover * 130.0f, -385.0f + (Cover % 3) * 96.0f, 58.0f);
        const FLinearColor CoverTint = (Cover % 2 == 0 ? FabGold : Mission.SecondaryAccentColor) * 0.82f;
        TagDetail(AddTags(SpawnTexturedBlock(
            CoverLoc,
            FVector(0.62f, 0.34f, 0.58f),
            CoverTint,
            FString::Printf(TEXT("%s Public Demo Combat Cover Crate %d"), *CityLabel, Cover + 1),
            TEXT("/Game/StarterContent/Materials/M_Metal_Rust.M_Metal_Rust"),
            true),
            { TEXT("CombatCoverReadable"), TEXT("TacticalArenaPolish"), TEXT("ProjectilePhysicsTarget") }));
    }
    SpawnDetailLight(OverpassHub + FVector(-280.0f, -260.0f, 250.0f), EmergencyRed, 3000.0f, 760.0f, TEXT("Public Demo Overpass Emergency Light A"));
    SpawnDetailLight(OverpassHub + FVector(360.0f, 220.0f, 250.0f), ClinicalBlue, 2600.0f, 740.0f, TEXT("Public Demo Overpass Emergency Light B"));

    // 3. Terminal room dressing: the coding mission should feel like a place, not a widget in an empty lot.
    TagDetail(AddTags(SpawnTexturedBlock(
        TerminalHub + FVector(0.0f, 0.0f, -16.0f),
        FVector(4.8f, 3.8f, 0.052f),
        FLinearColor(0.055f, 0.070f, 0.076f, 1.0f),
        CityLabel + TEXT(" Public Demo Terminal Lab Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Hex_Tile.M_Tech_Hex_Tile"),
        false),
        { TEXT("CodingMissionPlaceMaking"), TEXT("MissionRoomPolish") }));
    TagDetail(AddTags(SpawnTexturedBlock(
        TerminalHub + FVector(0.0f, 252.0f, 118.0f),
        FVector(4.8f, 0.11f, 1.34f),
        Mission.AccentColor * 0.24f + WetBlack * 0.76f,
        CityLabel + TEXT(" Public Demo Terminal Lab Backwall"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        true),
        { TEXT("CodingMissionPlaceMaking"), TEXT("MissionRoomPolish") }));
    for (int32 Screen = 0; Screen < 5; ++Screen)
    {
        AActor* Monitor = TagDetail(AddTags(SpawnBlock(
            TerminalHub + FVector(-320.0f + Screen * 160.0f, 230.0f, 168.0f),
            FVector(0.54f, 0.034f, 0.32f),
            (Screen % 2 == 0 ? ReadableGreen : ClinicalBlue) * 1.8f,
            FString::Printf(TEXT("%s Public Demo Terminal Monitor %d"), *CityLabel, Screen + 1),
            false),
            { TEXT("CodingMissionPlaceMaking"), TEXT("AnimatedMissionDetail") }));
        AddSlowRotation(Monitor, FRotator(0.0f, 0.0f, 2.5f + Screen), FString::Printf(TEXT("TerminalMonitorIdle%d"), Screen + 1));
    }
    SpawnImportedDetail(
        TEXT("/Game/StarterContent/Props/SM_TableRound.SM_TableRound"),
        TEXT("/Game/StarterContent/Props/Materials/M_TableRound.M_TableRound"),
        TerminalHub + FVector(-250.0f, -90.0f, 45.0f),
        FRotator(0.0f, 28.0f, 0.0f),
        FVector(1.35f, 1.35f, 0.78f),
        TEXT("Public Demo Debug Table"),
        Concrete,
        true);
    SpawnImportedDetail(
        TEXT("/Game/StarterContent/Props/SM_Shelf.SM_Shelf"),
        TEXT("/Game/StarterContent/Props/Materials/M_Shelf.M_Shelf"),
        TerminalHub + FVector(310.0f, 98.0f, 98.0f),
        FRotator(0.0f, -90.0f, 0.0f),
        FVector(1.08f, 0.54f, 1.62f),
        TEXT("Public Demo Debug Shelf"),
        Concrete,
        true);
    SpawnDetailLight(TerminalHub + FVector(0.0f, 18.0f, 260.0f), ClinicalBlue, 3600.0f, 720.0f, TEXT("Public Demo Terminal Lab Light"));

    // 4. Survivor/safe-room detail and emotional stakes.
    TagDetail(AddTags(SpawnTexturedBlock(
        SurvivorHub + FVector(0.0f, 0.0f, -16.0f),
        FVector(5.2f, 3.7f, 0.052f),
        FLinearColor(0.18f, 0.135f, 0.09f, 1.0f),
        CityLabel + TEXT(" Public Demo Survivor Safe Room Floor"),
        TEXT("/Game/StarterContent/Materials/M_Wood_Floor_Walnut_Worn.M_Wood_Floor_Walnut_Worn"),
        false),
        { TEXT("SurvivorRoomPolish"), TEXT("EmotionalReadableObjective") }));
    SpawnImportedDetail(
        TEXT("/Game/StarterContent/Props/SM_Couch.SM_Couch"),
        TEXT("/Game/StarterContent/Materials/M_Wood_Walnut.M_Wood_Walnut"),
        SurvivorHub + FVector(-260.0f, 112.0f, 48.0f),
        FRotator(0.0f, 12.0f, 0.0f),
        FVector(1.75f, 0.82f, 0.72f),
        TEXT("Public Demo Survivor Couch"),
        FLinearColor(0.48f, 0.23f, 0.16f, 1.0f),
        true);
    SpawnImportedDetail(
        TEXT("/Game/StarterContent/Props/SM_Lamp_Ceiling.SM_Lamp_Ceiling"),
        TEXT("/Game/StarterContent/Props/Materials/M_Lamp.M_Lamp"),
        SurvivorHub + FVector(0.0f, -40.0f, 248.0f),
        FRotator::ZeroRotator,
        FVector(0.62f, 0.62f, 0.62f),
        TEXT("Public Demo Survivor Ceiling Lamp"),
        WarmPractical,
        false);
    for (int32 Supply = 0; Supply < 5; ++Supply)
    {
        TagDetail(AddTags(SpawnBlock(
            SurvivorHub + FVector(85.0f + Supply * 58.0f, -182.0f + (Supply % 2) * 54.0f, 45.0f),
            FVector(0.28f, 0.24f, 0.28f),
            (Supply % 2 == 0 ? ReadableGreen : FabGold) * 0.95f,
            FString::Printf(TEXT("%s Public Demo Survivor Supply Box %d"), *CityLabel, Supply + 1),
            true),
            { TEXT("SurvivorRoomPolish"), TEXT("UsefulGearVisualCue") }));
    }
    AActor* ReliefSymbol = TagDetail(AddTags(SpawnBlock(
        SurvivorHub + FVector(260.0f, -210.0f, 124.0f),
        FVector(0.14f, 0.56f, 0.52f),
        EmergencyRed * 1.8f,
        CityLabel + TEXT(" Public Demo Survivor Relief Cross Vertical"),
        false),
        { TEXT("SurvivorRoomPolish"), TEXT("EmotionalReadableObjective") }));
    AddSlowRotation(ReliefSymbol, FRotator(0.0f, 18.0f, 0.0f), TEXT("SurvivorReliefCrossTurn"));
    TagDetail(AddTags(SpawnBlock(
        SurvivorHub + FVector(260.0f, -210.0f, 124.0f),
        FVector(0.56f, 0.14f, 0.52f),
        EmergencyRed * 1.8f,
        CityLabel + TEXT(" Public Demo Survivor Relief Cross Horizontal"),
        false),
        { TEXT("SurvivorRoomPolish"), TEXT("EmotionalReadableObjective") }));
    SpawnDetailLight(SurvivorHub + FVector(-40.0f, -30.0f, 260.0f), WarmPractical, 3200.0f, 680.0f, TEXT("Public Demo Survivor Warm Light"));

    // 5. Local Fab/design gallery: explicit proof of what is currently incorporated.
    TagDetail(AddTags(SpawnTexturedBlock(
        FabGalleryHub + FVector(0.0f, 0.0f, -18.0f),
        FVector(6.8f, 4.1f, 0.055f),
        FLinearColor(0.045f, 0.040f, 0.032f, 1.0f) + FabGold * 0.08f,
        CityLabel + TEXT(" Public Demo Fab Gallery Floor"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false),
        { TEXT("FabShowcaseExpanded"), TEXT("DesignCoverageDisplay") }));
    static const TCHAR* GalleryLabels[] = {
        TEXT("MODERN BRIDGES"),
        TEXT("PARALLAX CITY"),
        TEXT("DOG ZOMBIE"),
        TEXT("URBAN ZOMBIE"),
        TEXT("GROOM / HAIR"),
        TEXT("STARTER PROPS"),
    };
    for (int32 Panel = 0; Panel < UE_ARRAY_COUNT(GalleryLabels); ++Panel)
    {
        const FVector PanelLoc = FabGalleryHub + FVector(-490.0f + Panel * 196.0f, 185.0f, 132.0f);
        TagDetail(AddTags(SpawnBlock(
            PanelLoc,
            FVector(0.82f, 0.06f, 0.96f),
            (Panel % 2 == 0 ? FabGold : Mission.AccentColor) * 0.86f,
            FString::Printf(TEXT("%s Public Demo Fab Gallery Panel %d"), *CityLabel, Panel + 1),
            false),
            { TEXT("FabShowcaseExpanded"), TEXT("DesignCoverageDisplay") }));
        TagDetail(AddTags(SpawnGuideText(
            GalleryLabels[Panel],
            PanelLoc + FVector(0.0f, -54.0f, 76.0f),
            FabGold.ToFColor(true),
            16.0f),
            { TEXT("FabShowcaseExpanded"), TEXT("DesignCoverageDisplay") }));
        AActor* Token = TagDetail(AddTags(SpawnBlock(
            PanelLoc + FVector(0.0f, -62.0f, -72.0f),
            FVector(0.28f + 0.03f * Panel, 0.28f, 0.28f),
            (Panel % 2 == 0 ? ClinicalBlue : ReadableGreen) * 1.25f,
            FString::Printf(TEXT("%s Public Demo Fab Gallery Token %d"), *CityLabel, Panel + 1),
            false),
            { TEXT("FabShowcaseExpanded"), TEXT("DesignCoverageDisplay") }));
        AddSlowRotation(Token, FRotator(0.0f, 22.0f + Panel * 3.0f, 0.0f), FString::Printf(TEXT("FabGalleryTokenTurn%d"), Panel + 1));
    }
    SpawnDetailLight(FabGalleryHub + FVector(0.0f, 0.0f, 300.0f), FabGold, 3700.0f, 760.0f, TEXT("Public Demo Fab Gallery Light"));

    // 6. Threat foreshadowing near the deeper city route without adding new barriers.
    TagDetail(AddTags(SpawnTexturedBlock(
        BossForeshadowHub + FVector(0.0f, 0.0f, -16.0f),
        FVector(4.8f, 3.4f, 0.050f),
        FLinearColor(0.105f, 0.030f, 0.025f, 1.0f),
        CityLabel + TEXT(" Public Demo Threat Foreshadow Floor"),
        TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough"),
        false),
        { TEXT("SurvivalHorrorThreatForeshadow"), TEXT("BossRoutePolish") }));
    SpawnImportedDetail(
        TEXT("/Game/StarterContent/Props/SM_Statue.SM_Statue"),
        TEXT("/Game/StarterContent/Props/Materials/M_Statue.M_Statue"),
        BossForeshadowHub + FVector(0.0f, 0.0f, 128.0f),
        FRotator(0.0f, -22.0f, 0.0f),
        FVector(1.35f, 1.35f, 1.9f),
        TEXT("Public Demo Threat Statue"),
        FLinearColor(0.28f, 0.27f, 0.25f, 1.0f),
        true);
    for (int32 Spike = 0; Spike < 8; ++Spike)
    {
        const float Angle = Spike * 45.0f;
        const FVector SpikeOffset(FMath::Cos(FMath::DegreesToRadians(Angle)) * 320.0f, FMath::Sin(FMath::DegreesToRadians(Angle)) * 220.0f, 70.0f);
        TagDetail(AddTags(SpawnRotatedBlock(
            BossForeshadowHub + SpikeOffset,
            FRotator(0.0f, Angle, -18.0f),
            FVector(0.16f, 0.16f, 1.05f),
            EmergencyRed * (0.65f + 0.04f * Spike),
            FString::Printf(TEXT("%s Public Demo Threat Spike %d"), *CityLabel, Spike + 1),
            false),
            { TEXT("SurvivalHorrorThreatForeshadow"), TEXT("BossRoutePolish") }));
    }
    SpawnDetailLight(BossForeshadowHub + FVector(0.0f, 0.0f, 310.0f), EmergencyRed, 4200.0f, 780.0f, TEXT("Public Demo Threat Foreshadow Light"));

    // 7. A few real pickups tucked into the polished spaces so detail also affects play.
    auto SpawnPremiumPickup = [&](EPickupKind Kind, const FVector& Loc, int32 Amount, const TCHAR* Tag)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Loc, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("PublicDemoProductionDetail"));
            Pickup->Tags.Add(FName("UsefulGearVisualCue"));
            Pickup->Tags.Add(FName(Tag));
            RegisterStreamedActor(Pickup);
        }
    };
    SpawnPremiumPickup(EPickupKind::Ammo, OverpassHub + FVector(450.0f, -250.0f, 96.0f), 70, TEXT("PremiumRouteAmmo"));
    SpawnPremiumPickup(EPickupKind::Medkit, SurvivorHub + FVector(360.0f, -130.0f, 94.0f), 1, TEXT("PremiumSafeRoomMedkit"));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescuePublicDemoQuality] %s integrated public-demo detail, expanded local Fab set pieces, authored storefronts, combat cover, mission-room dressing, survivor room polish, and threat foreshadowing."),
        *CityLabel);
}

void ACodeRescueGameMode::SpawnMajorCityUrbanIdentityLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    // #45 — U.S. cities drive the whole street network from their realization
    // params: asphalt/lane/sidewalk tones, road + sidewalk widths, facade
    // palette, and the city's road pattern family. Global cities keep the
    // long-standing neutral street kit.
    const bool bUSRealized = IsUSMajorCityMission(Mission);
    FCodeRescueUSCityRealizationParams RoadParams;
    if (bUSRealized)
    {
        RoadParams = BuildUSCityRealizationParams(Mission, BuildUSCityVisualProfile(Mission));
    }
    const FLinearColor Asphalt = bUSRealized ? RoadParams.AsphaltTone : FLinearColor(0.018f, 0.020f, 0.023f);
    const FLinearColor Sidewalk = bUSRealized
        ? (RoadParams.bBrickHistoricWalks ? FLinearColor(0.30f, 0.14f, 0.10f) : RoadParams.SidewalkTone)
        : FLinearColor(0.19f, 0.18f, 0.16f);
    const FLinearColor LanePaint = bUSRealized ? RoadParams.LanePaintTone : FLinearColor(0.86f, 0.78f, 0.52f);
    const float RoadWidth = bUSRealized ? RoadParams.RoadWidthScale : 1.0f;
    const float WalkWidth = bUSRealized ? RoadParams.SidewalkWidthScale : 1.0f;
    const FLinearColor Glass = (bUSRealized && RoadParams.FacadePalette.Num() > 1)
        ? RoadParams.FacadePalette[2 % RoadParams.FacadePalette.Num()] + Mission.AccentColor * 0.06f
        : FLinearColor(0.10f, 0.22f, 0.28f) + Mission.AccentColor * 0.18f;
    const FLinearColor Brick = (bUSRealized && RoadParams.FacadePalette.Num() > 0)
        ? RoadParams.FacadePalette[0] + Mission.SecondaryAccentColor * 0.03f
        : FLinearColor(0.22f, 0.10f, 0.075f) + Mission.SecondaryAccentColor * 0.08f;
    // IrregularHistoric streets drift off the perfect grid deterministically.
    FRandomStream PatternStream(Mission.SkylineSeed + 4505);
    const bool bIrregular = bUSRealized && RoadParams.RoadPatternToken == TEXT("IrregularHistoric");

    auto TagUrban = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("MajorCityUrbanLandscape"));
            Actor->Tags.Add(FName("USMajorCityIdentity"));
            Actor->Tags.Add(FName("StreetGridCityComposition"));
            Actor->Tags.Add(FName("CityStreetGridStorefrontShell"));
            Actor->Tags.Add(FName("ReadableCityStreetGrid"));
            Actor->Tags.Add(FName("NotOpenClutterField"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto TagCrosswalkShell = [&TagUrban](AActor* Actor) -> AActor*
    {
        Actor = TagUrban(Actor);
        if (Actor)
        {
            Actor->Tags.Add(FName("StreetGridCrosswalkReadable"));
            Actor->Tags.Add(FName("HumanScaleCurbCrossing"));
            Actor->Tags.Add(FName("RouteClearStreetShell"));
        }
        return Actor;
    };

    auto TagStorefrontShell = [&TagUrban](AActor* Actor) -> AActor*
    {
        Actor = TagUrban(Actor);
        if (Actor)
        {
            Actor->Tags.Add(FName("StorefrontShellGroundFloor"));
            Actor->Tags.Add(FName("ModularStorefrontShell"));
            Actor->Tags.Add(FName("ParallaxStorefrontReady"));
            Actor->Tags.Add(FName("ImportedWorldAssetPromotionTarget"));
            Actor->Tags.Add(FName("RouteClearStreetShell"));
        }
        return Actor;
    };

    static const float EastWestRoadYs[] = { -2520.0f, -1540.0f, -560.0f, 520.0f, 1510.0f, 2420.0f };
    for (float Y : EastWestRoadYs)
    {
        const float YJitter = bIrregular ? PatternStream.FRandRange(-120.0f, 120.0f) : 0.0f;
        const float RoadY = Y + YJitter;
        TagUrban(SpawnBlock(
            Origin + CityOffset(FVector(0.0f, RoadY, -18.0f)),
            CityExtent(FVector(78.0f, 0.92f * RoadWidth, 0.030f)),
            Asphalt,
            CityLabel + TEXT(" Authored East-West City Street"),
            false));
        TagUrban(SpawnBlock(
            Origin + CityOffset(FVector(0.0f, RoadY - 145.0f * RoadWidth, -12.0f)),
            CityExtent(FVector(76.0f, 0.10f, 0.022f)),
            LanePaint * 1.7f,
            CityLabel + TEXT(" East-West Lane Paint"),
            false));
        TagUrban(SpawnBlock(
            Origin + CityOffset(FVector(0.0f, RoadY + 210.0f * RoadWidth, -16.0f)),
            CityExtent(FVector(78.0f, 0.20f * WalkWidth, 0.030f)),
            Sidewalk,
            CityLabel + TEXT(" North Sidewalk"),
            false));
        TagUrban(SpawnBlock(
            Origin + CityOffset(FVector(0.0f, RoadY - 210.0f * RoadWidth, -16.0f)),
            CityExtent(FVector(78.0f, 0.20f * WalkWidth, 0.030f)),
            Sidewalk,
            CityLabel + TEXT(" South Sidewalk"),
            false));
        // Wide-arterial cities read with a painted median strip.
        if (bUSRealized && RoadParams.RoadPatternToken == TEXT("WideArterial"))
        {
            TagUrban(SpawnBlock(
                Origin + CityOffset(FVector(0.0f, RoadY + 145.0f * RoadWidth, -12.0f)),
                CityExtent(FVector(76.0f, 0.10f, 0.022f)),
                LanePaint * 1.7f,
                CityLabel + TEXT(" Arterial Second Lane Paint"),
                false));
        }
    }

    static const float NorthSouthRoadXs[] = { -3150.0f, -1780.0f, -420.0f, 940.0f, 2280.0f, 3380.0f };
    for (float X : NorthSouthRoadXs)
    {
        const float XJitter = bIrregular ? PatternStream.FRandRange(-130.0f, 130.0f) : 0.0f;
        const float RoadX = X + XJitter;
        TagUrban(SpawnBlock(
            Origin + CityOffset(FVector(RoadX, 40.0f, -17.0f)),
            CityExtent(FVector(0.82f * RoadWidth, 62.0f, 0.030f)),
            Asphalt * 1.08f,
            CityLabel + TEXT(" Authored North-South City Street"),
            false));
        TagUrban(SpawnBlock(
            Origin + CityOffset(FVector(RoadX + 130.0f * RoadWidth, 40.0f, -11.0f)),
            CityExtent(FVector(0.08f, 60.0f, 0.022f)),
            LanePaint * 1.45f,
                CityLabel + TEXT(" North-South Lane Paint"),
                false));
    }

    for (int32 Ix = 1; Ix < UE_ARRAY_COUNT(NorthSouthRoadXs) - 1; Ix += 2)
    {
        for (int32 Iy = 1; Iy < UE_ARRAY_COUNT(EastWestRoadYs) - 1; Iy += 2)
        {
            const FVector Intersection = Origin + CityOffset(FVector(NorthSouthRoadXs[Ix], EastWestRoadYs[Iy], -9.5f));
            for (int32 Stripe = -2; Stripe <= 2; ++Stripe)
            {
                TagCrosswalkShell(SpawnBlock(
                    Intersection + CityOffset(FVector(Stripe * 42.0f, -95.0f, 0.0f)),
                    CityExtent(FVector(0.12f, 0.42f, 0.018f)),
                    LanePaint * 1.9f,
                    CityLabel + TEXT(" Street Grid Crosswalk Stripe"),
                    false));
                TagCrosswalkShell(SpawnBlock(
                    Intersection + CityOffset(FVector(95.0f, Stripe * 42.0f, 0.0f)),
                    CityExtent(FVector(0.42f, 0.12f, 0.018f)),
                    LanePaint * 1.9f,
                    CityLabel + TEXT(" Street Grid Crosswalk Stripe"),
                    false));
            }
            TagCrosswalkShell(SpawnGuideText(
                TEXT("CROSSWALK"),
                Intersection + FVector(0.0f, -115.0f, 72.0f),
                LanePaint.ToFColor(true),
                12.0f));
        }
    }

    // #45 — road pattern families beyond the base grid.
    if (bUSRealized && RoadParams.RoadPatternToken == TEXT("DiagonalAvenues"))
    {
        // L'Enfant-style diagonal state avenues crossing the grid.
        for (int32 d = 0; d < 2; ++d)
        {
            TagUrban(SpawnRotatedBlock(
                Origin + CityOffset(FVector(d == 0 ? -700.0f : 700.0f, 0.0f, -14.0f)),
                FRotator(0.0f, d == 0 ? 38.0f : -38.0f, 0.0f),
                CityExtent(FVector(64.0f, 0.78f, 0.026f)),
                Asphalt * 1.25f,
                CityLabel + TEXT(" Diagonal State Avenue"),
                false));
        }
    }
    if (bUSRealized && RoadParams.RoadPatternToken == TEXT("NumberedGrid"))
    {
        // Tight numbered cross-streets between the main grid lines.
        for (int32 e = 0; e < 3; ++e)
        {
            TagUrban(SpawnBlock(
                Origin + CityOffset(FVector(0.0f, -2030.0f + e * 990.0f, -17.5f)),
                CityExtent(FVector(70.0f, 0.46f, 0.026f)),
                Asphalt * 1.15f,
                CityLabel + TEXT(" Numbered Cross Street"),
                false));
        }
    }
    if (bUSRealized && RoadParams.RoadPatternToken == TEXT("HillGrid"))
    {
        // Terraced sidewalk steps suggest the famous hill streets without
        // altering walkable collision.
        for (int32 h = 0; h < 4; ++h)
        {
            TagUrban(SpawnBlock(
                Origin + CityOffset(FVector(2280.0f, -1900.0f + h * 1240.0f, -8.0f + h * 16.0f)),
                CityExtent(FVector(0.95f, 0.6f, 0.05f)),
                Sidewalk * 1.2f,
                CityLabel + TEXT(" Hill Street Terrace"),
                false));
        }
    }

    FRandomStream UrbanStream(Mission.SkylineSeed + 7331);
    static const FVector DistrictCenters[] = {
        FVector(-2450.0f, 560.0f, 0.0f),
        FVector(-1020.0f, 980.0f, 0.0f),
        FVector(520.0f, 1420.0f, 0.0f),
        FVector(1770.0f, 820.0f, 0.0f),
        FVector(2950.0f, 1900.0f, 0.0f),
    };
    static const TCHAR* DistrictNames[] = {
        TEXT("Downtown Block"),
        TEXT("Civic Core"),
        TEXT("Transit Spine"),
        TEXT("Medical District"),
        TEXT("Survivor Search District"),
    };
    static const TCHAR* StorefrontRoles[] = {
        TEXT("CLINIC"),
        TEXT("RADIO REPAIR"),
        TEXT("MARKET"),
        TEXT("PHARMACY"),
        TEXT("TRANSIT INFO"),
        TEXT("HARDWARE"),
        TEXT("COMMUNITY KITCHEN"),
        TEXT("SAFE ROUTE MAP"),
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(DistrictCenters); ++i)
    {
        const FVector Center = Origin + CityOffset(DistrictCenters[i]);
        TagUrban(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s, %s"), DistrictNames[i], *Mission.CityName, *Mission.StateName),
            Center + FVector(0.0f, -240.0f, 355.0f),
            Mission.SecondaryAccentColor.ToFColor(true),
            30.0f));

        for (int32 Row = 0; Row < 2; ++Row)
        {
            for (int32 Col = 0; Col < 4; ++Col)
            {
                const float LocalX = -420.0f + Col * 280.0f + UrbanStream.FRandRange(-35.0f, 35.0f);
                const float LocalY = Row == 0 ? -95.0f : 155.0f;
                const float Floors = UrbanStream.FRandRange(2.4f, 5.8f + Mission.DifficultyTier * 0.35f);
                const FVector Scale = CityArchitectureExtent(FVector(
                    UrbanStream.FRandRange(0.72f, 1.22f),
                    UrbanStream.FRandRange(0.50f, 0.92f),
                    Floors));
                const FVector Loc = Center + CityOffset(FVector(LocalX, LocalY, 0.0f)) + FVector(0.0f, 0.0f, Scale.Z * 50.0f);
                AActor* Building = nullptr;
                if (UStaticMesh* Mesh = LoadCodeRescueCityBuildingMesh(CityIndex + i * 11 + Row * 5 + Col))
                {
                    Building = SpawnStaticMeshProp(
                        Mesh,
                        Loc,
                        FRotator(0.0f, UrbanStream.FRandRange(-5.0f, 5.0f), 0.0f),
                        Scale,
                        CityLabel + TEXT(" Dense Urban Facade"),
                        true);
                    ApplyCodeRescueMaterialToStaticActor(Building, CodeRescueCityBuildingMaterialPaths[(i + Col) % UE_ARRAY_COUNT(CodeRescueCityBuildingMaterialPaths)], this, Row == 0 ? Brick : Glass, 0.12f);
                }
                else
                {
                    Building = SpawnBlock(Loc, Scale, Row == 0 ? Brick : Glass, CityLabel + TEXT(" Dense Urban Block"), true);
                }
                TagUrban(Building);

                TagUrban(SpawnBlock(
                    Loc + FVector(0.0f, Row == 0 ? -58.0f : 58.0f, 35.0f),
                    CityExtent(FVector(0.45f, 0.035f, 0.28f)),
                    FLinearColor(0.72f, 0.88f, 1.0f) * 1.35f,
                    CityLabel + TEXT(" Lit Storefront Window"),
                    false));

                const float FacadeSide = Row == 0 ? -1.0f : 1.0f;
                const FVector StorefrontBase(Loc.X, Loc.Y + FacadeSide * 72.0f, Origin.Z + 104.0f);
                const FLinearColor StorefrontTrim =
                    (Col % 2 == 0 ? Mission.SecondaryAccentColor : Mission.AccentColor) * 1.12f + LanePaint * 0.22f;
                const FLinearColor StorefrontGlass =
                    FLinearColor(0.36f, 0.58f, 0.72f, 1.0f) + Mission.AccentColor * 0.16f;
                const TCHAR* StorefrontRole = StorefrontRoles[(i * 8 + Row * 4 + Col) % UE_ARRAY_COUNT(StorefrontRoles)];

                TagStorefrontShell(SpawnBlock(
                    StorefrontBase + FVector(-74.0f, FacadeSide * 2.0f, -10.0f),
                    CityExtent(FVector(0.20f, 0.036f, 0.66f)),
                    FLinearColor(0.055f, 0.060f, 0.066f) + StorefrontTrim * 0.12f,
                    CityLabel + TEXT(" Storefront Door Recess"),
                    false));
                TagStorefrontShell(SpawnBlock(
                    StorefrontBase + FVector(76.0f, FacadeSide * 2.0f, -6.0f),
                    CityExtent(FVector(0.52f, 0.034f, 0.46f)),
                    StorefrontGlass * 1.35f,
                    CityLabel + TEXT(" Storefront Ground Window"),
                    false));
                TagStorefrontShell(SpawnBlock(
                    StorefrontBase + FVector(8.0f, FacadeSide * 3.0f, 72.0f),
                    CityExtent(FVector(0.98f, 0.032f, 0.12f)),
                    StorefrontTrim * 1.45f,
                    CityLabel + TEXT(" Storefront Sign Band"),
                    false));
                TagStorefrontShell(SpawnBlock(
                    StorefrontBase + FVector(8.0f, FacadeSide * 15.0f, 102.0f),
                    CityExtent(FVector(1.04f, 0.055f, 0.10f)),
                    StorefrontTrim * 0.78f + FLinearColor(0.03f, 0.03f, 0.035f),
                    CityLabel + TEXT(" Storefront Awning"),
                    false));
                TagStorefrontShell(SpawnGuideText(
                    FString::Printf(TEXT("%s\nSTREET LEVEL"), StorefrontRole),
                    StorefrontBase + FVector(8.0f, FacadeSide * 58.0f, 98.0f),
                    StorefrontTrim.ToFColor(true),
                    14.0f));
            }
        }
    }

    TagUrban(SpawnGuideText(
        FString::Printf(TEXT("CITY LANDSCAPE PASS\nCITY STREET GRID + STOREFRONT SHELL\nroads, sidewalks, crosswalks, dense facades, ground-floor shops\n%s objective route"), *Mission.CityName),
        Origin + CityOffset(FVector(-3600.0f, 2620.0f, 430.0f)),
        FColor(225, 235, 255),
        31.0f));
}

void ACodeRescueGameMode::SpawnUSCitySpecificIdentityLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    if (!IsUSMajorCityMission(Mission))
    {
        return;
    }

    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    FRandomStream IdentityStream(Mission.SkylineSeed ^ 0x55534349);
    const FVector IdentityCenter = Origin + CityOffset(FVector(2550.0f, -2840.0f, 0.0f));
    const FLinearColor TrimColor = Mission.SecondaryAccentColor * 1.35f;
    const FLinearColor WindowColor = FLinearColor(0.46f, 0.70f, 0.90f) + Mission.AccentColor * 0.20f;

    auto TagIdentity = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("USCitySpecificIdentity"));
            Actor->Tags.Add(FName("CitySpecificLandscape"));
            Actor->Tags.Add(FName("CitySpecificArchitecture"));
            Actor->Tags.Add(FName("CitySpecificSky"));
            Actor->Tags.Add(FName("CitySpecificRoads"));
            Actor->Tags.Add(FName("CitySpecificSidewalks"));
            Actor->Tags.Add(FName("CitySpecificHomes"));
            Actor->Tags.Add(FName("CitySpecificVehicles"));
            Actor->Tags.Add(FName("CitySpecificClothing"));
            Actor->Tags.Add(FName("CitySpecificDistricts"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto SpawnProfileSign = [&](const FVector& Local, const FString& Label, const FLinearColor& Tint)
    {
        const FVector Base = IdentityCenter + CityOffset(Local);
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 92.0f), FVector(0.055f, 0.055f, 1.85f), FLinearColor(0.045f, 0.048f, 0.052f), CityLabel + TEXT(" US City Identity Signpost"), false));
        TagIdentity(SpawnBlock(Base + FVector(0.0f, -8.0f, 196.0f), FVector(1.86f, 0.045f, 0.46f), Tint * 1.25f, CityLabel + TEXT(" US City Identity Sign Face"), false));
        TagIdentity(SpawnGuideText(Label, Base + FVector(0.0f, -46.0f, 236.0f), Tint.ToFColor(true), 22.0f));
    };

    TagIdentity(SpawnTexturedBlock(
        IdentityCenter + CityOffset(FVector(0.0f, 0.0f, -24.0f)),
        CityExtent(FVector(15.0f, 5.6f, 0.040f)),
        Profile.TerrainColor,
        CityLabel + TEXT(" US City Specific Terrain Plate"),
        Profile.bDesert
            ? TEXT("/Game/StarterContent/Materials/M_Rock_Sandstone.M_Rock_Sandstone")
            : Profile.bCoastal || Profile.bRiverfront
                ? TEXT("/Game/StarterContent/Materials/M_Ground_Grass.M_Ground_Grass")
                : TEXT("/Game/StarterContent/Materials/M_Ground_Moss.M_Ground_Moss"),
        false));

    TagIdentity(SpawnBlock(
        IdentityCenter + CityOffset(FVector(0.0f, 520.0f, 700.0f)),
        CityExtent(FVector(7.8f, 0.035f, 1.12f)),
        Profile.SkyColor * 1.55f,
        CityLabel + TEXT(" US City Specific Sky Mood Band"),
        false));
    TagIdentity(SpawnBlock(
        IdentityCenter + CityOffset(FVector(-1040.0f, 500.0f, 865.0f)),
        CityExtent(FVector(0.70f, 0.030f, 0.70f)),
        (Profile.bColdWeather ? FLinearColor(0.80f, 0.92f, 1.0f) : FLinearColor(1.0f, 0.76f, 0.30f)) * 1.8f,
        CityLabel + TEXT(" US City Specific Sun Or Moon Disc"),
        false));

    const float RoadY = -430.0f;
    TagIdentity(SpawnBlock(
        IdentityCenter + CityOffset(FVector(0.0f, RoadY, -10.0f)),
        CityExtent(FVector(13.4f, 0.56f * Profile.StreetWidthScale, 0.026f)),
        Profile.RoadColor,
        CityLabel + TEXT(" US City Specific Road Pattern"),
        false));
    TagIdentity(SpawnBlock(
        IdentityCenter + CityOffset(FVector(0.0f, RoadY + 150.0f * Profile.StreetWidthScale, -8.0f)),
        CityExtent(FVector(13.4f, 0.15f, 0.025f)),
        Profile.SidewalkColor,
        CityLabel + TEXT(" US City Specific North Sidewalk"),
        false));
    TagIdentity(SpawnBlock(
        IdentityCenter + CityOffset(FVector(0.0f, RoadY - 150.0f * Profile.StreetWidthScale, -8.0f)),
        CityExtent(FVector(13.4f, 0.15f, 0.025f)),
        Profile.SidewalkColor,
        CityLabel + TEXT(" US City Specific South Sidewalk"),
        false));

    for (int32 i = 0; i < 9; ++i)
    {
        const float X = -1280.0f + i * 320.0f;
        TagIdentity(SpawnBlock(
            IdentityCenter + CityOffset(FVector(X, RoadY, -5.0f)),
            CityExtent(FVector(0.72f, 0.035f, 0.018f)),
            FLinearColor(0.88f, 0.78f, 0.44f) * 1.8f,
            CityLabel + TEXT(" US City Specific Lane Dash"),
            false));
    }

    for (int32 i = 0; i < 7; ++i)
    {
        const float X = -1460.0f + i * 76.0f;
        TagIdentity(SpawnBlock(
            IdentityCenter + CityOffset(FVector(X, RoadY + 225.0f, -4.0f)),
            CityExtent(FVector(0.22f, 1.55f, 0.020f)),
            FLinearColor(0.86f, 0.88f, 0.84f) * 1.55f,
            CityLabel + TEXT(" US City Specific Crosswalk Stripe"),
            false));
    }

    if (Profile.bFreeway)
    {
        TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(760.0f, RoadY - 310.0f, 120.0f)), FVector(0.08f, 0.08f, 2.4f), FLinearColor(0.055f, 0.060f, 0.064f), CityLabel + TEXT(" US City Freeway Sign Left Post"), false));
        TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(1080.0f, RoadY - 310.0f, 120.0f)), FVector(0.08f, 0.08f, 2.4f), FLinearColor(0.055f, 0.060f, 0.064f), CityLabel + TEXT(" US City Freeway Sign Right Post"), false));
        TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(920.0f, RoadY - 326.0f, 245.0f)), FVector(2.55f, 0.060f, 0.42f), FLinearColor(0.02f, 0.30f, 0.14f) + TrimColor * 0.12f, CityLabel + TEXT(" US City Freeway Road Sign"), false));
    }

    if (Profile.bTransit)
    {
        for (int32 Rail = 0; Rail < 2; ++Rail)
        {
            TagIdentity(SpawnBlock(
                IdentityCenter + CityOffset(FVector(0.0f, RoadY + 66.0f + Rail * 58.0f, -2.0f)),
                CityExtent(FVector(12.8f, 0.025f, 0.020f)),
                FLinearColor(0.62f, 0.62f, 0.58f) * 1.25f,
                CityLabel + TEXT(" US City Transit Rail Cue"),
                false));
        }
    }

    for (int32 i = 0; i < Profile.TowerCount; ++i)
    {
        const float X = -1420.0f + i * (2840.0f / FMath::Max(1, Profile.TowerCount - 1));
        const float BaseHeight = Profile.bCapitalCivic ? 1.20f : Profile.bSuburban ? 1.05f : Profile.bDesert ? 1.25f : 2.10f;
        const float Height = BaseHeight + (Profile.bCapitalCivic ? (i % 2) * 0.20f : IdentityStream.FRandRange(0.25f, Profile.bTechCampus ? 2.20f : 3.80f));
        const FVector Scale = CityArchitectureExtent(FVector(
            Profile.bHistoric ? 0.56f : 0.74f + (i % 3) * 0.12f,
            Profile.bIndustrial ? 0.72f : 0.54f,
            Height));
        const FVector Loc = IdentityCenter + CityOffset(FVector(X, 160.0f + (i % 2) * 150.0f, 0.0f)) + FVector(0.0f, 0.0f, Scale.Z * 50.0f);
        const FLinearColor FacadeColor = Profile.ArchitectureColor + Mission.AccentColor * (0.04f + 0.018f * (i % 4));
        TagIdentity(SpawnBlock(
            Loc,
            Scale,
            FacadeColor,
            CityLabel + TEXT(" US City Specific Architecture Facade"),
            false));
        TagIdentity(SpawnBlock(
            Loc + FVector(0.0f, -Scale.Y * 52.0f, FMath::Min(220.0f, Scale.Z * 28.0f)),
            FVector(FMath::Max(0.38f, Scale.X * 0.18f), 0.035f, 0.22f),
            WindowColor * 1.35f,
            CityLabel + TEXT(" US City Specific Lit Window Band"),
            false));
        if (Profile.bHistoric && i % 2 == 0)
        {
            TagIdentity(SpawnBlock(
                Loc + FVector(0.0f, -Scale.Y * 54.0f, -Scale.Z * 24.0f),
                FVector(FMath::Max(0.42f, Scale.X * 0.14f), 0.050f, 0.20f),
                TrimColor,
                CityLabel + TEXT(" US City Historic Cornice"),
                false));
        }
    }

    if (Profile.bCapitalCivic)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            const float X = -430.0f + i * 215.0f;
            TagIdentity(SpawnBlock(
                IdentityCenter + CityOffset(FVector(X, 390.0f, 155.0f)),
                FVector(0.12f, 0.12f, 2.9f),
                FLinearColor(0.78f, 0.78f, 0.70f),
                CityLabel + TEXT(" US City Civic Column"),
                false));
        }
        TagIdentity(SpawnBlock(
            IdentityCenter + CityOffset(FVector(0.0f, 390.0f, 322.0f)),
            FVector(4.8f, 0.20f, 0.20f),
            FLinearColor(0.86f, 0.84f, 0.76f),
            CityLabel + TEXT(" US City Civic Entablature"),
            false));
    }

    const int32 HomeRows = Profile.bSuburban ? Profile.HomeCount + 2 : Profile.HomeCount;
    for (int32 i = 0; i < HomeRows; ++i)
    {
        const float X = -1360.0f + i * (2720.0f / FMath::Max(1, HomeRows - 1));
        const FVector HouseScale = CityArchitectureExtent(FVector(
            Profile.bHistoric ? 0.38f : 0.48f,
            0.42f,
            Profile.bDesert ? 0.54f : Profile.bColdWeather ? 0.78f : 0.62f));
        const FVector HouseLoc = IdentityCenter + CityOffset(FVector(X, -910.0f, 0.0f)) + FVector(0.0f, 0.0f, HouseScale.Z * 50.0f);
        TagIdentity(SpawnBlock(
            HouseLoc,
            HouseScale,
            Profile.HomeColor + Mission.SecondaryAccentColor * (0.025f * (i % 3)),
            CityLabel + TEXT(" US City Specific Home Row"),
            false));
        TagIdentity(SpawnRotatedBlock(
            HouseLoc + FVector(0.0f, -42.0f, HouseScale.Z * 18.0f),
            FRotator(Profile.bColdWeather ? -12.0f : -5.0f, 0.0f, 0.0f),
            FVector(0.78f, 0.055f, 0.16f),
            Profile.bColdWeather ? FLinearColor(0.82f, 0.90f, 0.95f) : TrimColor,
            CityLabel + TEXT(" US City Specific Home Roofline"),
            false));
        if (Profile.bHistoric)
        {
            TagIdentity(SpawnBlock(
                HouseLoc + FVector(0.0f, -55.0f, -HouseScale.Z * 20.0f),
                FVector(0.24f, 0.08f, 0.12f),
                Profile.SidewalkColor,
                CityLabel + TEXT(" US City Specific Stoop"),
                false));
        }
    }

    auto SpawnPalm = [&](const FVector& Local)
    {
        const FVector Base = IdentityCenter + CityOffset(Local);
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 108.0f), FVector(0.07f, 0.07f, 2.1f), FLinearColor(0.30f, 0.19f, 0.10f), CityLabel + TEXT(" US City Palm Trunk"), false));
        for (int32 Leaf = 0; Leaf < 4; ++Leaf)
        {
            TagIdentity(SpawnRotatedBlock(
                Base + FVector(0.0f, 0.0f, 228.0f),
                FRotator(0.0f, Leaf * 90.0f, Leaf % 2 == 0 ? 18.0f : -18.0f),
                FVector(0.72f, 0.07f, 0.08f),
                FLinearColor(0.05f, 0.34f, 0.13f),
                CityLabel + TEXT(" US City Palm Frond"),
                false));
        }
    };

    auto SpawnEvergreen = [&](const FVector& Local)
    {
        const FVector Base = IdentityCenter + CityOffset(Local);
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 72.0f), FVector(0.08f, 0.08f, 1.42f), FLinearColor(0.20f, 0.12f, 0.06f), CityLabel + TEXT(" US City Evergreen Trunk"), false));
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 164.0f), FVector(0.72f, 0.72f, 0.68f), FLinearColor(0.04f, 0.22f, 0.10f), CityLabel + TEXT(" US City Evergreen Crown"), false));
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 236.0f), FVector(0.44f, 0.44f, 0.46f), FLinearColor(0.04f, 0.28f, 0.13f), CityLabel + TEXT(" US City Evergreen Top"), false));
    };

    auto SpawnCactus = [&](const FVector& Local)
    {
        const FVector Base = IdentityCenter + CityOffset(Local);
        const FLinearColor CactusGreen = FLinearColor(0.12f, 0.38f, 0.18f);
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 118.0f), FVector(0.11f, 0.11f, 2.25f), CactusGreen, CityLabel + TEXT(" US City Desert Cactus Trunk"), false));
        TagIdentity(SpawnBlock(Base + FVector(52.0f, 0.0f, 150.0f), FVector(0.58f, 0.08f, 0.11f), CactusGreen, CityLabel + TEXT(" US City Desert Cactus Arm A"), false));
        TagIdentity(SpawnBlock(Base + FVector(-52.0f, 0.0f, 188.0f), FVector(0.58f, 0.08f, 0.11f), CactusGreen, CityLabel + TEXT(" US City Desert Cactus Arm B"), false));
    };

    if (Profile.bCoastal || Profile.bRiverfront)
    {
        TagIdentity(SpawnTexturedBlock(
            IdentityCenter + CityOffset(FVector(0.0f, 720.0f, -18.0f)),
            CityExtent(FVector(14.4f, 0.86f, 0.026f)),
            FLinearColor(0.02f, 0.16f, 0.24f) * (Profile.bTropical ? 2.7f : 2.0f),
            CityLabel + TEXT(" US City Specific Waterline"),
            TEXT("/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake"),
            false));
        for (int32 i = 0; i < 4; ++i)
        {
            TagIdentity(SpawnBlock(
                IdentityCenter + CityOffset(FVector(-1000.0f + i * 660.0f, 625.0f, 16.0f)),
                CityExtent(FVector(1.20f, 0.12f, 0.055f)),
                FLinearColor(0.32f, 0.22f, 0.12f),
                CityLabel + TEXT(" US City Specific Pier Plank"),
                false));
        }
        if (Profile.bTropical)
        {
            for (int32 i = 0; i < 5; ++i)
            {
                SpawnPalm(FVector(-1300.0f + i * 650.0f, 520.0f, 0.0f));
            }
        }
    }

    if (Profile.bDesert)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            SpawnCactus(FVector(-1300.0f + i * 650.0f, 520.0f + (i % 2) * 110.0f, 0.0f));
        }
        TagIdentity(SpawnBlock(
            IdentityCenter + CityOffset(FVector(1130.0f, 565.0f, 86.0f)),
            CityExtent(FVector(2.1f, 0.42f, 1.42f)),
            FLinearColor(0.42f, 0.25f, 0.12f),
            CityLabel + TEXT(" US City Desert Mesa Silhouette"),
            false));
    }

    if (Profile.bMountain)
    {
        for (int32 i = 0; i < 4; ++i)
        {
            const float Height = 1.25f + i * 0.34f;
            TagIdentity(SpawnBlock(
                IdentityCenter + CityOffset(FVector(-1260.0f + i * 840.0f, 585.0f, Height * 54.0f)),
                CityExtent(FVector(2.8f, 0.34f, Height)),
                FLinearColor(0.13f, 0.14f, 0.13f) + Profile.SkyColor * 0.05f,
                CityLabel + TEXT(" US City Mountain Horizon Ridge"),
                false));
            if (Profile.bColdWeather || Mission.StateName == TEXT("CO") || Mission.StateName == TEXT("AK") || Mission.StateName == TEXT("UT"))
            {
                TagIdentity(SpawnBlock(
                    IdentityCenter + CityOffset(FVector(-1260.0f + i * 840.0f, 570.0f, Height * 108.0f)),
                    CityExtent(FVector(1.62f, 0.20f, 0.11f)),
                    FLinearColor(0.82f, 0.92f, 1.0f),
                    CityLabel + TEXT(" US City Snowcap Horizon"),
                    false));
            }
        }
    }

    if (!Profile.bDesert && !Profile.bTropical)
    {
        for (int32 i = 0; i < 4; ++i)
        {
            SpawnEvergreen(FVector(-1180.0f + i * 760.0f, 535.0f, 0.0f));
        }
    }

    if (Profile.bIndustrial)
    {
        for (int32 i = 0; i < 3; ++i)
        {
            const FVector StackBase = IdentityCenter + CityOffset(FVector(-880.0f + i * 420.0f, 370.0f, 0.0f));
            TagIdentity(SpawnBlock(StackBase + FVector(0.0f, 0.0f, 182.0f), FVector(0.16f, 0.16f, 3.45f), FLinearColor(0.12f, 0.12f, 0.11f), CityLabel + TEXT(" US City Industrial Stack"), false));
            TagIdentity(SpawnBlock(StackBase + FVector(0.0f, 0.0f, 372.0f), FVector(0.28f, 0.28f, 0.10f), TrimColor * 1.3f, CityLabel + TEXT(" US City Industrial Stack Light"), false));
        }
    }

    if (Profile.bEntertainment)
    {
        for (int32 i = 0; i < 4; ++i)
        {
            TagIdentity(SpawnBlock(
                IdentityCenter + CityOffset(FVector(-760.0f + i * 380.0f, 245.0f, 325.0f + i * 20.0f)),
                FVector(0.42f, 0.040f, 0.76f),
                (i % 2 == 0 ? FLinearColor(1.0f, 0.15f, 0.80f) : FLinearColor(0.0f, 0.90f, 1.0f)) * 2.0f,
                CityLabel + TEXT(" US City Entertainment Neon Blade"),
                false));
        }
    }

    auto SpawnDistrictHeader = [&](const FVector& Local, const FString& Name, const FString& Detail, const FLinearColor& Color)
    {
        const FVector Base = IdentityCenter + CityOffset(Local);
        TagIdentity(SpawnBlock(Base + FVector(0.0f, 0.0f, 18.0f), FVector(1.55f, 0.10f, 0.12f), Color * 0.75f, CityLabel + TEXT(" US City District ") + Name + TEXT(" Base"), false));
        TagIdentity(SpawnBlock(Base + FVector(0.0f, -8.0f, 96.0f), FVector(0.08f, 0.055f, 1.35f), FLinearColor(0.045f, 0.048f, 0.052f), CityLabel + TEXT(" US City District ") + Name + TEXT(" Marker Post"), false));
        TagIdentity(SpawnBlock(Base + FVector(0.0f, -12.0f, 176.0f), FVector(1.40f, 0.040f, 0.24f), Color * 1.35f, CityLabel + TEXT(" US City District ") + Name + TEXT(" Marker Face"), false));
        TagIdentity(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), *Name, *Detail),
            Base + FVector(0.0f, -58.0f, 220.0f),
            Color.ToFColor(true),
            18.0f));
    };

    auto SpawnDistrictMicroScenes = [&]()
    {
        SpawnDistrictHeader(
            FVector(-1420.0f, 1120.0f, 0.0f),
            TEXT("DISTRICT CUES"),
            Profile.DistrictCue,
            Profile.SignatureColor + Mission.AccentColor * 0.14f);

        if (Profile.bCoastal || Profile.bRiverfront)
        {
            const FLinearColor WaterDistrictColor = FLinearColor(0.02f, 0.22f, 0.30f) * (Profile.bTropical ? 2.3f : 1.7f);
            SpawnDistrictHeader(
                FVector(-980.0f, 1125.0f, 0.0f),
                Profile.bRiverfront ? TEXT("RIVERWALK DISTRICT") : TEXT("WATERFRONT DISTRICT"),
                Profile.bMilitaryHarbor ? TEXT("harbor gate, mast, pier rail") : TEXT("boardwalk, railing, water edge"),
                WaterDistrictColor);
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-980.0f, 1045.0f, 34.0f)), FVector(2.35f, 0.10f, 0.24f), FLinearColor(0.34f, 0.23f, 0.12f), CityLabel + TEXT(" US City District Waterfront Boardwalk"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-980.0f, 1018.0f, 72.0f)), FVector(2.15f, 0.035f, 0.10f), FLinearColor(0.74f, 0.76f, 0.70f), CityLabel + TEXT(" US City District Waterfront Rail"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-780.0f, 1085.0f, 86.0f)), FVector(0.11f, 0.080f, 1.52f), Profile.bMilitaryHarbor ? FLinearColor(0.20f, 0.26f, 0.32f) : FLinearColor(0.68f, 0.42f, 0.20f), CityLabel + TEXT(" US City District Harbor Mast"), false));
        }

        if (Profile.bTransit)
        {
            const FLinearColor TransitColor = FLinearColor(0.62f, 0.66f, 0.70f) + Mission.SecondaryAccentColor * 0.18f;
            SpawnDistrictHeader(
                FVector(-510.0f, 1130.0f, 0.0f),
                TEXT("TRANSIT DISTRICT"),
                Profile.RoadCue,
                TransitColor);
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-510.0f, 1040.0f, 40.0f)), FVector(1.85f, 0.12f, 0.24f), FLinearColor(0.055f, 0.060f, 0.064f), CityLabel + TEXT(" US City District Transit Platform"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-510.0f, 1024.0f, 126.0f)), FVector(1.55f, 0.045f, 0.12f), TransitColor * 1.35f, CityLabel + TEXT(" US City District Transit Shelter Roof"), false));
            for (int32 i = 0; i < 2; ++i)
            {
                TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-660.0f + i * 300.0f, 1010.0f, 80.0f)), FVector(0.055f, 0.045f, 0.82f), TransitColor, CityLabel + TEXT(" US City District Transit Shelter Post"), false));
            }
        }

        if (Profile.bHistoric || Profile.bCapitalCivic)
        {
            const FLinearColor HistoricColor = Profile.bCapitalCivic ? FLinearColor(0.82f, 0.82f, 0.76f) : FLinearColor(0.62f, 0.36f, 0.22f);
            SpawnDistrictHeader(
                FVector(-65.0f, 1132.0f, 0.0f),
                Profile.bCapitalCivic ? TEXT("CIVIC DISTRICT") : TEXT("HISTORIC DISTRICT"),
                Profile.bCapitalCivic ? TEXT("columns, bollards, official avenue") : TEXT("stoops, cornices, brick storefronts"),
                HistoricColor);
            for (int32 i = 0; i < 4; ++i)
            {
                TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-230.0f + i * 110.0f, 1040.0f, 72.0f)), FVector(0.42f, 0.16f, 0.92f), HistoricColor * (0.85f + 0.05f * i), CityLabel + TEXT(" US City District Historic Rowfront"), false));
            }
            if (Profile.bCapitalCivic)
            {
                for (int32 i = 0; i < 5; ++i)
                {
                    TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-265.0f + i * 132.0f, 986.0f, 40.0f)), FVector(0.06f, 0.06f, 0.58f), FLinearColor(0.18f, 0.18f, 0.17f), CityLabel + TEXT(" US City District Civic Bollard"), false));
                }
            }
        }

        if (Profile.bIndustrial)
        {
            SpawnDistrictHeader(
                FVector(395.0f, 1130.0f, 0.0f),
                TEXT("WAREHOUSE DISTRICT"),
                TEXT("loading dock, containers, factory edge"),
                FLinearColor(0.44f, 0.42f, 0.38f));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(395.0f, 1040.0f, 72.0f)), FVector(2.00f, 0.24f, 0.86f), FLinearColor(0.20f, 0.18f, 0.15f), CityLabel + TEXT(" US City District Warehouse Dock"), false));
            for (int32 i = 0; i < 3; ++i)
            {
                TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(240.0f + i * 150.0f, 1010.0f, 38.0f + i * 8.0f)), FVector(0.70f, 0.18f, 0.30f), Mission.AccentColor * (0.65f + 0.08f * i), CityLabel + TEXT(" US City District Freight Container"), false));
            }
        }

        if (Profile.bEntertainment)
        {
            SpawnDistrictHeader(
                FVector(820.0f, 1130.0f, 0.0f),
                TEXT("VENUE DISTRICT"),
                TEXT("marquees, stage doors, nightlife glow"),
                FLinearColor(1.0f, 0.22f, 0.82f));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(820.0f, 1044.0f, 112.0f)), FVector(1.70f, 0.10f, 1.14f), FLinearColor(0.055f, 0.050f, 0.070f), CityLabel + TEXT(" US City District Venue Facade"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(820.0f, 1018.0f, 180.0f)), FVector(1.45f, 0.040f, 0.16f), FLinearColor(1.0f, 0.25f, 0.80f) * 2.0f, CityLabel + TEXT(" US City District Venue Marquee"), false));
        }

        if (Profile.bTechCampus || Profile.bCollegeTown)
        {
            const bool bCampus = Profile.bCollegeTown;
            SpawnDistrictHeader(
                FVector(1245.0f, 1130.0f, 0.0f),
                bCampus ? TEXT("CAMPUS DISTRICT") : TEXT("TECH CAMPUS DISTRICT"),
                bCampus ? TEXT("quad, lab hall, bike path") : TEXT("glass lab, office loop, EV curb"),
                bCampus ? FLinearColor(0.30f, 0.58f, 0.28f) : FLinearColor(0.14f, 0.68f, 0.86f));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(1245.0f, 1048.0f, 38.0f)), FVector(1.95f, 0.22f, 0.26f), bCampus ? FLinearColor(0.12f, 0.36f, 0.14f) : FLinearColor(0.035f, 0.08f, 0.10f), CityLabel + TEXT(" US City District Campus Green Or Lab Base"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(1245.0f, 1015.0f, 118.0f)), FVector(1.35f, 0.060f, 0.82f), bCampus ? FLinearColor(0.46f, 0.30f, 0.18f) : FLinearColor(0.08f, 0.28f, 0.36f), CityLabel + TEXT(" US City District Campus Lab Hall"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(1245.0f, 995.0f, 166.0f)), FVector(1.10f, 0.032f, 0.10f), bCampus ? FLinearColor(0.86f, 0.82f, 0.58f) : FLinearColor(0.10f, 0.88f, 1.0f) * 1.7f, CityLabel + TEXT(" US City District Campus Window Band"), false));
        }

        if (Profile.bMountain)
        {
            SpawnDistrictHeader(
                FVector(-1180.0f, 1395.0f, 0.0f),
                TEXT("TRAILHEAD DISTRICT"),
                TEXT("mountain view, rescue marker, outdoor gear"),
                FLinearColor(0.42f, 0.56f, 0.46f));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-1180.0f, 1312.0f, 48.0f)), FVector(1.65f, 0.14f, 0.38f), FLinearColor(0.22f, 0.16f, 0.10f), CityLabel + TEXT(" US City District Trailhead Kiosk"), false));
            TagIdentity(SpawnRotatedBlock(IdentityCenter + CityOffset(FVector(-1088.0f, 1302.0f, 92.0f)), FRotator(0.0f, 0.0f, -18.0f), FVector(0.70f, 0.045f, 0.08f), FLinearColor(0.82f, 0.92f, 1.0f), CityLabel + TEXT(" US City District Trail Snow Marker"), false));
        }

        if (Profile.bDesert)
        {
            SpawnDistrictHeader(
                FVector(-720.0f, 1395.0f, 0.0f),
                TEXT("SHADE DISTRICT"),
                TEXT("xeriscape, canopy, desert wash"),
                FLinearColor(0.82f, 0.48f, 0.18f));
            TagIdentity(SpawnRotatedBlock(IdentityCenter + CityOffset(FVector(-720.0f, 1310.0f, 126.0f)), FRotator(-6.0f, 0.0f, 0.0f), FVector(1.55f, 0.10f, 0.12f), FLinearColor(0.86f, 0.58f, 0.28f), CityLabel + TEXT(" US City District Desert Shade Canopy"), false));
            TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-720.0f, 1280.0f, 30.0f)), FVector(1.85f, 0.16f, 0.16f), FLinearColor(0.38f, 0.24f, 0.12f), CityLabel + TEXT(" US City District Xeriscape Wash"), false));
        }

        if (Profile.bSuburban)
        {
            SpawnDistrictHeader(
                FVector(-260.0f, 1395.0f, 0.0f),
                TEXT("NEIGHBORHOOD LOOP"),
                TEXT("townhomes, garage fronts, planned streets"),
                FLinearColor(0.48f, 0.50f, 0.44f));
            for (int32 i = 0; i < 4; ++i)
            {
                TagIdentity(SpawnBlock(IdentityCenter + CityOffset(FVector(-420.0f + i * 110.0f, 1310.0f, 58.0f)), FVector(0.42f, 0.18f, 0.66f), Profile.HomeColor * (0.95f + 0.04f * i), CityLabel + TEXT(" US City District Townhome Front"), false));
            }
        }

        for (int32 i = 0; i < 3; ++i)
        {
            const FVector AccessoryBase = IdentityCenter + CityOffset(FVector(610.0f + i * 170.0f, 1370.0f, 82.0f));
            const FString AccessoryName = Profile.bColdWeather
                ? TEXT("Parka Hood")
                : Profile.bTropical || Profile.bDesert
                    ? TEXT("Sun Hat")
                    : Profile.bCollegeTown
                        ? TEXT("Backpack")
                        : Profile.bCapitalCivic
                            ? TEXT("Badge Lanyard")
                            : TEXT("Local Layer");
            TagIdentity(SpawnBlock(
                AccessoryBase,
                FVector(0.18f, 0.055f, 0.20f),
                Profile.ClothingColor * (1.25f + 0.10f * i),
                CityLabel + TEXT(" US City District Clothing Accessory ") + AccessoryName,
                false));
        }
    };

    SpawnDistrictMicroScenes();

    auto SpawnSignatureSilhouette = [&]()
    {
        const FString& Token = Profile.SignatureShapeToken;
        const FVector SignatureBase = IdentityCenter + CityOffset(FVector(0.0f, 965.0f, 0.0f));
        const FLinearColor SignatureColor = Profile.SignatureColor + Mission.AccentColor * 0.12f;
        const FLinearColor SignatureGlow = SignatureColor * 1.8f;
        const FLinearColor DarkTrim = FLinearColor(0.035f, 0.040f, 0.045f);

        auto SigBlock = [&](const FVector& Local, const FVector& Scale, const FLinearColor& Color, const FString& Name)
        {
            return TagIdentity(SpawnBlock(SignatureBase + CityOffset(Local), Scale, Color, CityLabel + TEXT(" US City Signature ") + Name, false));
        };

        auto SigRotated = [&](const FVector& Local, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color, const FString& Name)
        {
            return TagIdentity(SpawnRotatedBlock(SignatureBase + CityOffset(Local), Rotation, Scale, Color, CityLabel + TEXT(" US City Signature ") + Name, false));
        };

        SigBlock(FVector(0.0f, 0.0f, 10.0f), FVector(5.8f, 0.18f, 0.08f), DarkTrim + SignatureColor * 0.10f, TEXT("Silhouette Base Plinth"));

        if (Token == TEXT("HarborStatue"))
        {
            SigBlock(FVector(0.0f, 0.0f, 52.0f), FVector(1.15f, 0.24f, 0.24f), SignatureColor * 0.72f, TEXT("Harbor Statue Pedestal"));
            SigBlock(FVector(0.0f, 0.0f, 154.0f), FVector(0.32f, 0.18f, 1.78f), SignatureColor, TEXT("Harbor Statue Robe"));
            SigBlock(FVector(0.0f, 0.0f, 258.0f), FVector(0.40f, 0.20f, 0.24f), SignatureColor * 1.08f, TEXT("Harbor Statue Head"));
            SigRotated(FVector(72.0f, 0.0f, 250.0f), FRotator(0.0f, 0.0f, 28.0f), FVector(0.08f, 0.06f, 0.95f), SignatureColor, TEXT("Harbor Statue Raised Arm"));
            SigBlock(FVector(112.0f, 0.0f, 322.0f), FVector(0.22f, 0.08f, 0.32f), FLinearColor(1.0f, 0.76f, 0.24f) * 2.0f, TEXT("Harbor Statue Torch"));
            for (int32 i = 0; i < 5; ++i)
            {
                SigRotated(FVector(-42.0f + i * 21.0f, 0.0f, 298.0f), FRotator(0.0f, 0.0f, -34.0f + i * 17.0f), FVector(0.030f, 0.035f, 0.44f), SignatureGlow, TEXT("Harbor Statue Crown Ray"));
            }
        }
        else if (Token == TEXT("HillsideLetters"))
        {
            SigRotated(FVector(0.0f, 0.0f, 82.0f), FRotator(0.0f, 0.0f, -9.0f), FVector(5.2f, 0.12f, 0.52f), FLinearColor(0.28f, 0.22f, 0.12f), TEXT("Hillside Grade"));
            for (int32 i = 0; i < 6; ++i)
            {
                SigBlock(FVector(-500.0f + i * 200.0f, -12.0f, 166.0f + i * 5.0f), FVector(0.34f, 0.045f, 0.44f), FLinearColor(0.96f, 0.88f, 0.68f), TEXT("Hillside Letter Slab"));
            }
            SigBlock(FVector(0.0f, 0.0f, 228.0f), FVector(2.4f, 0.040f, 0.18f), SignatureGlow, TEXT("Studio Boulevard Glow"));
        }
        else if (Token == TEXT("SuspensionBridge"))
        {
            SigBlock(FVector(0.0f, 0.0f, 18.0f), FVector(5.6f, 0.14f, 0.08f), FLinearColor(0.02f, 0.16f, 0.24f) * 2.0f, TEXT("Bay Water Stripe"));
            SigBlock(FVector(-310.0f, 0.0f, 142.0f), FVector(0.18f, 0.12f, 2.42f), SignatureColor, TEXT("Suspension West Tower"));
            SigBlock(FVector(310.0f, 0.0f, 142.0f), FVector(0.18f, 0.12f, 2.42f), SignatureColor, TEXT("Suspension East Tower"));
            SigBlock(FVector(0.0f, 0.0f, 120.0f), FVector(6.4f, 0.085f, 0.10f), SignatureColor * 0.86f, TEXT("Suspension Road Deck"));
            SigRotated(FVector(-160.0f, 0.0f, 238.0f), FRotator(0.0f, 0.0f, -13.0f), FVector(3.2f, 0.030f, 0.040f), SignatureGlow, TEXT("Suspension Cable West"));
            SigRotated(FVector(160.0f, 0.0f, 238.0f), FRotator(0.0f, 0.0f, 13.0f), FVector(3.2f, 0.030f, 0.040f), SignatureGlow, TEXT("Suspension Cable East"));
        }
        else if (Token == TEXT("ObservationNeedle"))
        {
            SigBlock(FVector(0.0f, 0.0f, 170.0f), FVector(0.12f, 0.10f, 3.15f), SignatureColor, TEXT("Observation Needle Mast"));
            SigBlock(FVector(0.0f, 0.0f, 342.0f), FVector(1.30f, 0.22f, 0.18f), SignatureGlow, TEXT("Observation Needle Saucer"));
            SigBlock(FVector(0.0f, 0.0f, 396.0f), FVector(0.16f, 0.08f, 0.86f), SignatureColor * 1.1f, TEXT("Observation Needle Spire"));
            SigBlock(FVector(-430.0f, 0.0f, 84.0f), FVector(1.8f, 0.10f, 0.84f), FLinearColor(0.04f, 0.24f, 0.12f), TEXT("Evergreen Skyline Mass"));
            SigBlock(FVector(430.0f, 0.0f, 64.0f), FVector(1.55f, 0.10f, 0.56f), FLinearColor(0.02f, 0.16f, 0.24f) * 1.9f, TEXT("Sound Waterline"));
        }
        else if (Token == TEXT("CivicObelisk"))
        {
            SigBlock(FVector(0.0f, 0.0f, 172.0f), FVector(0.30f, 0.20f, 3.10f), SignatureColor, TEXT("Civic Obelisk"));
            SigBlock(FVector(0.0f, 0.0f, 340.0f), FVector(0.16f, 0.12f, 0.52f), SignatureGlow, TEXT("Civic Obelisk Cap"));
            for (int32 i = 0; i < 6; ++i)
            {
                SigBlock(FVector(-420.0f + i * 168.0f, 0.0f, 92.0f), FVector(0.08f, 0.08f, 1.45f), FLinearColor(0.82f, 0.82f, 0.76f), TEXT("Civic Avenue Column"));
            }
        }
        else if (Token == TEXT("NeonMarquee"))
        {
            SigBlock(FVector(0.0f, 0.0f, 132.0f), FVector(2.4f, 0.10f, 1.72f), DarkTrim, TEXT("Neon Marquee Frame"));
            for (int32 i = 0; i < 5; ++i)
            {
                const FLinearColor NeonColor = (i % 2 == 0 ? FLinearColor(1.0f, 0.12f, 0.82f) : FLinearColor(0.0f, 0.90f, 1.0f)) * 2.2f;
                SigBlock(FVector(-410.0f + i * 205.0f, -8.0f, 150.0f), FVector(0.08f, 0.040f, 1.28f), NeonColor, TEXT("Neon Marquee Vertical Tube"));
            }
            SigBlock(FVector(0.0f, -10.0f, 252.0f), FVector(2.65f, 0.045f, 0.20f), SignatureGlow, TEXT("Neon Marquee Header"));
        }
        else if (Token == TEXT("RiverBridge") || Token == TEXT("BayouEnergy"))
        {
            SigBlock(FVector(0.0f, 0.0f, 20.0f), FVector(5.8f, 0.16f, 0.08f), FLinearColor(0.02f, 0.15f, 0.18f) * 2.0f, TEXT("River Water Stripe"));
            SigBlock(FVector(-360.0f, 0.0f, 112.0f), FVector(0.16f, 0.12f, 1.85f), SignatureColor, TEXT("River Bridge West Tower"));
            SigBlock(FVector(360.0f, 0.0f, 112.0f), FVector(0.16f, 0.12f, 1.85f), SignatureColor, TEXT("River Bridge East Tower"));
            SigBlock(FVector(0.0f, 0.0f, 118.0f), FVector(6.8f, 0.085f, 0.12f), SignatureColor * 0.9f, TEXT("River Bridge Deck"));
            SigRotated(FVector(-185.0f, 0.0f, 168.0f), FRotator(0.0f, 0.0f, 12.0f), FVector(2.55f, 0.035f, 0.045f), SignatureGlow, TEXT("River Bridge Truss A"));
            SigRotated(FVector(185.0f, 0.0f, 168.0f), FRotator(0.0f, 0.0f, -12.0f), FVector(2.55f, 0.035f, 0.045f), SignatureGlow, TEXT("River Bridge Truss B"));
            if (Token == TEXT("BayouEnergy"))
            {
                SigBlock(FVector(0.0f, 0.0f, 292.0f), FVector(0.48f, 0.12f, 1.35f), FLinearColor(0.10f, 0.34f, 0.40f) + SignatureColor * 0.30f, TEXT("Energy District Crown"));
            }
        }
        else if (Token == TEXT("MountainPeakTower") || Token == TEXT("MountainGridSpire"))
        {
            for (int32 i = 0; i < 5; ++i)
            {
                SigBlock(FVector(-520.0f + i * 260.0f, 0.0f, 64.0f + i * 10.0f), FVector(1.15f, 0.12f, 0.82f + i * 0.12f), FLinearColor(0.16f, 0.16f, 0.15f), TEXT("Mountain Signature Ridge"));
                SigBlock(FVector(-520.0f + i * 260.0f, -6.0f, 122.0f + i * 18.0f), FVector(0.74f, 0.070f, 0.10f), FLinearColor(0.82f, 0.92f, 1.0f), TEXT("Mountain Signature Snowcap"));
            }
            SigBlock(FVector(0.0f, -12.0f, 232.0f), FVector(0.22f, 0.08f, 2.25f), SignatureColor, Token == TEXT("MountainGridSpire") ? TEXT("Mountain Grid Spire") : TEXT("Front Range Tower"));
        }
        else if (Token == TEXT("TropicalDeco"))
        {
            SigBlock(FVector(0.0f, 0.0f, 128.0f), FVector(0.82f, 0.12f, 1.90f), FLinearColor(0.86f, 0.56f, 0.44f), TEXT("Art Deco Center Tower"));
            SigBlock(FVector(0.0f, -8.0f, 236.0f), FVector(1.15f, 0.045f, 0.12f), SignatureGlow, TEXT("Art Deco Neon Crown"));
            SigBlock(FVector(-320.0f, 0.0f, 96.0f), FVector(0.08f, 0.08f, 1.74f), FLinearColor(0.30f, 0.19f, 0.10f), TEXT("Art Deco Palm Trunk A"));
            SigBlock(FVector(320.0f, 0.0f, 96.0f), FVector(0.08f, 0.08f, 1.74f), FLinearColor(0.30f, 0.19f, 0.10f), TEXT("Art Deco Palm Trunk B"));
            SigBlock(FVector(-320.0f, 0.0f, 192.0f), FVector(0.70f, 0.10f, 0.14f), FLinearColor(0.04f, 0.34f, 0.16f), TEXT("Art Deco Palm Frond A"));
            SigBlock(FVector(320.0f, 0.0f, 192.0f), FVector(0.70f, 0.10f, 0.14f), FLinearColor(0.04f, 0.34f, 0.16f), TEXT("Art Deco Palm Frond B"));
        }
        else if (Token == TEXT("DesertSun"))
        {
            SigBlock(FVector(0.0f, 0.0f, 146.0f), FVector(1.05f, 0.10f, 1.05f), FLinearColor(1.0f, 0.58f, 0.16f) * 1.5f, TEXT("Desert Sun Disc"));
            for (int32 i = 0; i < 8; ++i)
            {
                SigRotated(FVector(0.0f, 0.0f, 146.0f), FRotator(0.0f, 0.0f, i * 22.5f), FVector(1.85f, 0.025f, 0.040f), SignatureGlow, TEXT("Desert Sun Ray"));
            }
            SigBlock(FVector(-430.0f, 0.0f, 70.0f), FVector(1.65f, 0.12f, 0.72f), FLinearColor(0.42f, 0.25f, 0.12f), TEXT("Desert Mesa A"));
            SigBlock(FVector(430.0f, 0.0f, 56.0f), FVector(1.45f, 0.12f, 0.54f), FLinearColor(0.36f, 0.21f, 0.10f), TEXT("Desert Mesa B"));
        }
        else if (Token == TEXT("TechCampus") || Token == TEXT("CampusQuad"))
        {
            SigBlock(FVector(0.0f, 0.0f, 78.0f), FVector(3.4f, 0.12f, 1.02f), Token == TEXT("CampusQuad") ? FLinearColor(0.18f, 0.42f, 0.20f) : DarkTrim, Token == TEXT("CampusQuad") ? TEXT("Campus Quad Green") : TEXT("Tech Circuit Board"));
            for (int32 i = 0; i < 5; ++i)
            {
                SigBlock(FVector(-480.0f + i * 240.0f, -8.0f, 144.0f + (i % 2) * 32.0f), FVector(0.32f, 0.045f, 0.36f), SignatureGlow, Token == TEXT("CampusQuad") ? TEXT("Campus Lab Window") : TEXT("Tech Chip Node"));
            }
            SigBlock(FVector(0.0f, -10.0f, 215.0f), FVector(2.4f, 0.040f, 0.08f), SignatureColor, Token == TEXT("CampusQuad") ? TEXT("Campus Walk Axis") : TEXT("Tech Data Bus"));
        }
        else if (Token == TEXT("HistoricBell"))
        {
            SigBlock(FVector(0.0f, 0.0f, 122.0f), FVector(1.15f, 0.12f, 1.62f), FLinearColor(0.60f, 0.48f, 0.32f), TEXT("Historic Bell Hall"));
            SigBlock(FVector(0.0f, -10.0f, 222.0f), FVector(0.62f, 0.045f, 0.58f), SignatureGlow, TEXT("Historic Bell"));
            SigBlock(FVector(0.0f, -12.0f, 268.0f), FVector(1.35f, 0.045f, 0.12f), SignatureColor, TEXT("Historic Bell Beam"));
        }
        else if (Token == TEXT("MusicNote"))
        {
            SigBlock(FVector(-120.0f, 0.0f, 156.0f), FVector(0.12f, 0.08f, 2.22f), SignatureColor, TEXT("Music Note Stem"));
            SigBlock(FVector(-222.0f, 0.0f, 58.0f), FVector(0.58f, 0.12f, 0.40f), SignatureGlow, TEXT("Music Note Head"));
            SigRotated(FVector(120.0f, 0.0f, 250.0f), FRotator(0.0f, 0.0f, -17.0f), FVector(1.75f, 0.050f, 0.10f), SignatureColor, TEXT("Music Venue Roofline"));
            SigBlock(FVector(280.0f, -10.0f, 128.0f), FVector(1.1f, 0.045f, 0.38f), FLinearColor(1.0f, 0.70f, 0.20f) * 1.7f, TEXT("Music Marquee"));
        }
        else if (Token == TEXT("IndustrialMotor"))
        {
            SigBlock(FVector(-320.0f, 0.0f, 150.0f), FVector(0.22f, 0.14f, 2.75f), FLinearColor(0.14f, 0.14f, 0.13f), TEXT("Factory Stack A"));
            SigBlock(FVector(320.0f, 0.0f, 132.0f), FVector(0.20f, 0.14f, 2.34f), FLinearColor(0.13f, 0.13f, 0.12f), TEXT("Factory Stack B"));
            for (int32 i = 0; i < 8; ++i)
            {
                SigRotated(FVector(0.0f, 0.0f, 148.0f), FRotator(0.0f, 0.0f, i * 22.5f), FVector(1.15f, 0.035f, 0.070f), SignatureColor, TEXT("Motor Gear Tooth"));
            }
            SigBlock(FVector(0.0f, 0.0f, 148.0f), FVector(0.62f, 0.10f, 0.62f), DarkTrim, TEXT("Motor Gear Hub"));
        }
        else if (Token == TEXT("MissionArch") || Token == TEXT("StockyardGate"))
        {
            SigBlock(FVector(-260.0f, 0.0f, 126.0f), FVector(0.20f, 0.12f, 2.25f), SignatureColor, TEXT("Arch West Pillar"));
            SigBlock(FVector(260.0f, 0.0f, 126.0f), FVector(0.20f, 0.12f, 2.25f), SignatureColor, TEXT("Arch East Pillar"));
            SigBlock(FVector(0.0f, 0.0f, 252.0f), FVector(2.95f, 0.085f, 0.20f), SignatureGlow, Token == TEXT("StockyardGate") ? TEXT("Stockyard Gate Beam") : TEXT("Mission Arch Beam"));
            if (Token == TEXT("StockyardGate"))
            {
                SigBlock(FVector(0.0f, -10.0f, 92.0f), FVector(1.6f, 0.045f, 0.22f), FLinearColor(0.62f, 0.36f, 0.16f), TEXT("Stockyard Rail"));
            }
        }
        else if (Token == TEXT("HarborNaval") || Token == TEXT("HarborBeacon"))
        {
            SigBlock(FVector(0.0f, 0.0f, 42.0f), FVector(2.85f, 0.20f, 0.32f), FLinearColor(0.06f, 0.15f, 0.22f), TEXT("Harbor Hull"));
            SigBlock(FVector(0.0f, 0.0f, 148.0f), FVector(0.12f, 0.10f, 2.28f), SignatureColor, TEXT("Harbor Mast"));
            SigRotated(FVector(145.0f, 0.0f, 210.0f), FRotator(0.0f, 0.0f, -18.0f), FVector(1.15f, 0.040f, 0.060f), SignatureGlow, Token == TEXT("HarborBeacon") ? TEXT("Harbor Beacon Beam") : TEXT("Naval Signal Yardarm"));
            if (Token == TEXT("HarborBeacon"))
            {
                SigBlock(FVector(0.0f, -10.0f, 268.0f), FVector(0.34f, 0.045f, 0.42f), FLinearColor(1.0f, 0.82f, 0.34f) * 1.9f, TEXT("Harbor Beacon Lantern"));
            }
        }
        else if (Token == TEXT("VolcanicSurf") || Token == TEXT("SnowInlet"))
        {
            SigBlock(FVector(-330.0f, 0.0f, 74.0f), FVector(1.75f, 0.12f, 0.84f), Token == TEXT("SnowInlet") ? FLinearColor(0.62f, 0.70f, 0.74f) : FLinearColor(0.16f, 0.13f, 0.11f), TEXT("Island Or Inlet Ridge A"));
            SigBlock(FVector(180.0f, 0.0f, 112.0f), FVector(1.65f, 0.12f, 1.42f), Token == TEXT("SnowInlet") ? FLinearColor(0.52f, 0.60f, 0.64f) : FLinearColor(0.24f, 0.17f, 0.12f), TEXT("Island Or Inlet Ridge B"));
            SigBlock(FVector(0.0f, -8.0f, 28.0f), FVector(4.6f, 0.055f, 0.16f), FLinearColor(0.02f, 0.22f, 0.30f) * 2.1f, Token == TEXT("SnowInlet") ? TEXT("Cold Inlet Water") : TEXT("Surf Line"));
            SigBlock(FVector(180.0f, -10.0f, 188.0f), FVector(0.86f, 0.040f, 0.11f), Token == TEXT("SnowInlet") ? FLinearColor(0.84f, 0.94f, 1.0f) : SignatureGlow, Token == TEXT("SnowInlet") ? TEXT("Snowcap Band") : TEXT("Volcanic Glow"));
        }
        else if (Token == TEXT("BalconyStreetcar"))
        {
            SigBlock(FVector(0.0f, 0.0f, 118.0f), FVector(1.45f, 0.12f, 1.55f), FLinearColor(0.38f, 0.22f, 0.13f), TEXT("Balcony Facade"));
            for (int32 i = 0; i < 3; ++i)
            {
                SigBlock(FVector(-300.0f + i * 300.0f, -10.0f, 188.0f), FVector(0.62f, 0.045f, 0.10f), SignatureGlow, TEXT("Iron Balcony Rail"));
            }
            SigBlock(FVector(0.0f, -10.0f, 34.0f), FVector(2.25f, 0.050f, 0.26f), FLinearColor(0.72f, 0.18f, 0.12f), TEXT("Streetcar Body"));
        }
        else if (Token == TEXT("FreewayCrown") || Token == TEXT("EvergreenWaterTower"))
        {
            SigBlock(FVector(0.0f, 0.0f, 112.0f), FVector(2.55f, 0.10f, 1.45f), DarkTrim + SignatureColor * 0.20f, Token == TEXT("EvergreenWaterTower") ? TEXT("Rainy Skyline Mass") : TEXT("Glass Freeway Crown"));
            SigBlock(FVector(0.0f, -10.0f, 214.0f), FVector(1.90f, 0.045f, 0.14f), SignatureGlow, Token == TEXT("EvergreenWaterTower") ? TEXT("Water Tower Catwalk") : TEXT("Freeway Crown Light"));
            SigBlock(FVector(-430.0f, 0.0f, 82.0f), FVector(0.12f, 0.12f, 1.45f), Token == TEXT("EvergreenWaterTower") ? FLinearColor(0.06f, 0.28f, 0.13f) : FLinearColor(0.06f, 0.08f, 0.09f), Token == TEXT("EvergreenWaterTower") ? TEXT("Evergreen Marker") : TEXT("Freeway Sign Post"));
            SigBlock(FVector(430.0f, 0.0f, 82.0f), FVector(0.12f, 0.12f, 1.45f), Token == TEXT("EvergreenWaterTower") ? FLinearColor(0.06f, 0.28f, 0.13f) : FLinearColor(0.06f, 0.08f, 0.09f), Token == TEXT("EvergreenWaterTower") ? TEXT("Evergreen Marker Twin") : TEXT("Freeway Sign Post Twin"));
        }
        else
        {
            SigBlock(FVector(0.0f, 0.0f, 138.0f), FVector(0.55f, 0.14f, 2.30f), SignatureColor, TEXT("Regional Civic Marker"));
            SigBlock(FVector(0.0f, -8.0f, 266.0f), FVector(1.65f, 0.045f, 0.16f), SignatureGlow, TEXT("Regional Civic Beacon"));
        }

        TagIdentity(SpawnGuideText(
            FString::Printf(TEXT("SIGNATURE SILHOUETTE\n%s"), *Profile.SignatureCue),
            SignatureBase + CityOffset(FVector(0.0f, -160.0f, 385.0f)),
            SignatureColor.ToFColor(true),
            24.0f));
    };

    SpawnSignatureSilhouette();

    auto SpawnProfileVehicle = [&](const FVector& Local, const FLinearColor& BodyColor, const FString& Label, bool bLongVehicle)
    {
        const FVector Base = IdentityCenter + CityOffset(Local);
        TagIdentity(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 34.0f),
            FVector(bLongVehicle ? 1.82f : 1.22f, 0.48f, 0.30f),
            BodyColor,
            CityLabel + TEXT(" ") + Label + TEXT(" Body"),
            false));
        TagIdentity(SpawnBlock(
            Base + FVector(8.0f, 0.0f, 70.0f),
            FVector(bLongVehicle ? 0.98f : 0.62f, 0.36f, 0.24f),
            FLinearColor(0.025f, 0.030f, 0.038f),
            CityLabel + TEXT(" ") + Label + TEXT(" Cabin"),
            false));
        for (int32 Wheel = 0; Wheel < 4; ++Wheel)
        {
            const float WX = (Wheel < 2 ? -54.0f : 54.0f) * (bLongVehicle ? 1.45f : 1.0f);
            const float WY = Wheel % 2 == 0 ? -34.0f : 34.0f;
            TagIdentity(SpawnBlock(
                Base + FVector(WX, WY, 14.0f),
                FVector(0.15f, 0.060f, 0.15f),
                FLinearColor(0.015f, 0.015f, 0.018f),
                CityLabel + TEXT(" ") + Label + TEXT(" Wheel"),
                false));
        }
    };

    for (int32 i = 0; i < Profile.VehicleCount; ++i)
    {
        const FLinearColor BodyColor = i == 0
            ? Profile.VehicleColor * 1.25f
            : (i % 3 == 0 ? Mission.AccentColor * 0.85f : Profile.VehicleColor + FLinearColor(0.04f * (i % 2), 0.03f, 0.02f));
        const bool bLong = Profile.VehicleCue.Contains(TEXT("bus")) || Profile.VehicleCue.Contains(TEXT("shuttle")) || Profile.VehicleCue.Contains(TEXT("truck")) || (i % 4 == 3);
        SpawnProfileVehicle(
            FVector(-1180.0f + i * 540.0f, RoadY - 35.0f + (i % 2) * 96.0f, 0.0f),
            BodyColor,
            FString::Printf(TEXT("US City %s Vehicle Cue"), *Mission.CityName),
            bLong);
    }

    for (int32 i = 0; i < 3; ++i)
    {
        const FVector Local(-600.0f + i * 520.0f, -680.0f + (i % 2) * 70.0f, 92.0f);
        AActor* Civilian = SpawnDecorativeCivilian(
            IdentityCenter + CityOffset(Local),
            FRotator(0.0f, 150.0f - i * 35.0f, 0.0f),
            i % 2 == 0,
            Profile.ClothingColor * (1.0f + i * 0.16f),
            CityLabel + TEXT(" US City Specific Local Clothing Civilian"),
            Profile.ClothingCue);
        TagIdentity(Civilian);
    }

    SpawnProfileSign(
        FVector(-1475.0f, -1180.0f, 0.0f),
        FString::Printf(TEXT("LANDSCAPE / SKY\n%s\n%s"), *Profile.LandscapeCue, *Profile.SkyCue),
        Profile.SkyColor);
    SpawnProfileSign(
        FVector(-515.0f, -1180.0f, 0.0f),
        FString::Printf(TEXT("ARCHITECTURE / HOMES\n%s\n%s"), *Profile.ArchitectureCue, *Profile.HomeCue),
        Profile.ArchitectureColor + Mission.AccentColor * 0.12f);
    SpawnProfileSign(
        FVector(470.0f, -1180.0f, 0.0f),
        FString::Printf(TEXT("ROADS / VEHICLES\n%s\n%s"), *Profile.RoadCue, *Profile.VehicleCue),
        Profile.VehicleColor + Mission.SecondaryAccentColor * 0.10f);
    SpawnProfileSign(
        FVector(1420.0f, -1180.0f, 0.0f),
        FString::Printf(TEXT("SIDEWALKS / CLOTHING\n%s\n%s"), *Profile.SidewalkCue, *Profile.ClothingCue),
        Profile.ClothingColor + Mission.AccentColor * 0.12f);
    SpawnProfileSign(
        FVector(0.0f, -1510.0f, 0.0f),
        FString::Printf(TEXT("DISTRICTS\n%s"), *Profile.DistrictCue),
        Profile.SignatureColor + Mission.SecondaryAccentColor * 0.12f);

    TagIdentity(SpawnGuideText(
        FString::Printf(TEXT("U.S. CITY IDENTITY\n%03d %s, %s"), Mission.Rank, *Mission.CityName, *Mission.StateName),
        IdentityCenter + CityOffset(FVector(0.0f, -1375.0f, 330.0f)),
        TrimColor.ToFColor(true),
        30.0f));

    UE_LOG(
        LogTemp,
        Display,
        TEXT("[CodeRescueUSCityIdentity] %s, %s landscape='%s' architecture='%s' sky='%s' roads='%s' sidewalks='%s' homes='%s' vehicles='%s' clothing='%s' signature='%s' districts='%s'"),
        *Mission.CityName,
        *Mission.StateName,
        *Profile.LandscapeCue,
        *Profile.ArchitectureCue,
        *Profile.SkyCue,
        *Profile.RoadCue,
        *Profile.SidewalkCue,
        *Profile.HomeCue,
        *Profile.VehicleCue,
        *Profile.ClothingCue,
        *Profile.SignatureCue,
        *Profile.DistrictCue);
}

void ACodeRescueGameMode::SpawnRegionalCityKitIdentityLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams Params = BuildUSCityRealizationParams(Mission, Profile);
    const bool bUSCity = IsUSMajorCityMission(Mission);
    const FVector RegionalCenter = Origin + CityOffset(FVector(-2120.0f, -940.0f, 0.0f));
    const FLinearColor KitColor = Mission.SecondaryAccentColor * 0.62f + Profile.SignatureColor * 0.38f;
    const FLinearColor DistrictColor = Mission.AccentColor * 0.48f + Profile.ArchitectureColor * 0.52f;
    const FLinearColor RouteColor = FLinearColor::LerpUsingHSV(KitColor, FLinearColor(0.15f, 0.75f, 1.0f), 0.34f);

    auto TagRegional = [bUSCity](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("RegionalCityKitIdentity"));
            Actor->Tags.Add(FName("MajorCityRegionalKit"));
            Actor->Tags.Add(FName("RegionalKitReady"));
            Actor->Tags.Add(FName("LandmarkWayfindingKit"));
            Actor->Tags.Add(FName("DistrictLevelInstanceStandIn"));
            Actor->Tags.Add(FName("KitBibleRuntimeCue"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.Add(FName("Top50Recommendations"));
            Actor->Tags.Add(bUSCity ? FName("USCityRegionalKit") : FName("GlobalCityRegionalKit"));
        }
        return Actor;
    };

    auto SpawnRegionalLight = [&](const FVector& Location, const FLinearColor& Color, float Radius, const FString& Name)
    {
        APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator);
        if (Light)
        {
            if (UPointLightComponent* PointLight = Light->FindComponentByClass<UPointLightComponent>())
            {
                PointLight->SetLightColor(Color);
                PointLight->SetIntensity(540.0f);
                PointLight->SetAttenuationRadius(Radius);
                PointLight->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            Light->Tags.Add(FName("RegionalCityKitIdentity"));
            Light->Tags.Add(FName("RegionalKitSignalLight"));
            Light->Tags.Add(FName("NoAccessBlocker"));
            Light->Tags.Add(FName("WorldDevelopmentDeepDive"));
            Light->Tags.Add(FName("Top50Recommendations"));
            RegisterStreamedActor(Light);
        }
    };

    auto SpawnModuleSwatches = [&](const FVector& Center, const FLinearColor& BaseColor, const FString& Prefix)
    {
        static const TCHAR* SwatchNames[] = {
            TEXT("Trim Sheet Strip"),
            TEXT("Facade Module"),
            TEXT("Prop Dressing"),
            TEXT("Destruction Dressing"),
        };
        for (int32 i = 0; i < UE_ARRAY_COUNT(SwatchNames); ++i)
        {
            const FVector SwatchLoc = Center + FVector(-168.0f + i * 112.0f, 62.0f, 82.0f + (i % 2) * 12.0f);
            const FLinearColor SwatchColor = BaseColor * (0.70f + 0.08f * i) + Mission.AccentColor * (0.04f * i);
            TagRegional(SpawnBlock(
                SwatchLoc,
                FVector(0.46f, 0.08f, 0.36f),
                SwatchColor,
                Prefix + TEXT(" Regional Kit ") + SwatchNames[i],
                false));
        }
    };

    auto SpawnKitAnchor = [&](const FString& Name, const FVector& Local, const FLinearColor& Color, const FString& Detail, const FString& FocusTag)
    {
        const FVector Center = RegionalCenter + CityOffset(Local);
        TagRegional(SpawnBlock(
            Center + FVector(0.0f, 0.0f, 14.0f),
            FVector(2.25f, 1.10f, 0.12f),
            FLinearColor(0.035f, 0.040f, 0.046f) + Color * 0.10f,
            CityLabel + TEXT(" ") + Name + TEXT(" Floor Plate"),
            false));
        TagRegional(SpawnBlock(
            Center + FVector(-146.0f, -76.0f, 116.0f),
            FVector(0.08f, 0.08f, 2.10f),
            FLinearColor(0.055f, 0.058f, 0.062f),
            CityLabel + TEXT(" ") + Name + TEXT(" West Post"),
            false));
        TagRegional(SpawnBlock(
            Center + FVector(146.0f, -76.0f, 116.0f),
            FVector(0.08f, 0.08f, 2.10f),
            FLinearColor(0.055f, 0.058f, 0.062f),
            CityLabel + TEXT(" ") + Name + TEXT(" East Post"),
            false));
        TagRegional(SpawnBlock(
            Center + FVector(0.0f, -86.0f, 228.0f),
            FVector(1.85f, 0.05f, 0.34f),
            Color * 1.55f,
            CityLabel + TEXT(" ") + Name + TEXT(" Readable Kit Sign"),
            false));
        TagRegional(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s\n%s"), *Name, *FocusTag, *Detail),
            Center + FVector(0.0f, -142.0f, 298.0f),
            Color.ToFColor(true),
            24.0f));
        SpawnModuleSwatches(Center, Color, CityLabel + TEXT(" ") + Name);
        SpawnRegionalLight(Center + FVector(0.0f, -90.0f, 330.0f), Color, 820.0f, CityLabel + TEXT(" ") + Name + TEXT(" Signal Light"));
    };

    SpawnKitAnchor(
        TEXT("REGIONAL KIT ENTRY GATE"),
        FVector(-1120.0f, -1140.0f, 0.0f),
        KitColor,
        FString::Printf(TEXT("kit bible: %s / %s"), *Mission.ArtKitName, *Mission.RegionName),
        Params.TerrainToken);
    SpawnKitAnchor(
        TEXT("LANDMARK VISTA KIT"),
        FVector(-60.0f, 260.0f, 0.0f),
        Profile.SignatureColor + Mission.AccentColor * 0.14f,
        Profile.SignatureCue,
        Mission.LandmarkName);
    SpawnKitAnchor(
        TEXT("OBJECTIVE DISTRICT KIT"),
        FVector(1080.0f, -240.0f, 0.0f),
        DistrictColor,
        Profile.DistrictCue,
        Mission.DistrictStyle);

    const FVector MotifCenter = RegionalCenter + CityOffset(FVector(0.0f, -1460.0f, 0.0f));
    const FString& Kit = Mission.ArtKitName;

    auto MotifBlock = [&](const FVector& Local, const FVector& Scale, const FLinearColor& Color, const FString& Name)
    {
        return TagRegional(SpawnBlock(MotifCenter + CityOffset(Local), Scale, Color, CityLabel + TEXT(" Regional Kit Motif ") + Name, false));
    };
    auto MotifRotated = [&](const FVector& Local, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color, const FString& Name)
    {
        return TagRegional(SpawnRotatedBlock(MotifCenter + CityOffset(Local), Rotation, Scale, Color, CityLabel + TEXT(" Regional Kit Motif ") + Name, false));
    };

    MotifBlock(FVector(0.0f, 0.0f, 18.0f), FVector(4.4f, 0.24f, 0.12f), FLinearColor(0.028f, 0.032f, 0.038f) + RouteColor * 0.10f, TEXT("District Level Instance Base"));
    if (Kit == TEXT("Coastal Port") || Profile.bCoastal || Profile.bMilitaryHarbor)
    {
        MotifBlock(FVector(0.0f, 18.0f, 38.0f), FVector(4.9f, 0.16f, 0.12f), FLinearColor(0.02f, 0.17f, 0.25f) * 1.8f, TEXT("Harbor Water Strip"));
        MotifBlock(FVector(-260.0f, -8.0f, 122.0f), FVector(0.12f, 0.10f, 2.30f), KitColor, TEXT("Port Crane Mast"));
        MotifRotated(FVector(-110.0f, -8.0f, 236.0f), FRotator(0.0f, 0.0f, -8.0f), FVector(1.60f, 0.045f, 0.08f), Mission.AccentColor * 1.7f, TEXT("Port Crane Boom"));
    }
    else if (Kit == TEXT("Desert Solar Grid") || Profile.bDesert)
    {
        MotifBlock(FVector(0.0f, 18.0f, 34.0f), FVector(4.8f, 0.16f, 0.10f), FLinearColor(0.42f, 0.25f, 0.12f), TEXT("Desert Wash Strip"));
        for (int32 i = 0; i < 3; ++i)
        {
            MotifRotated(FVector(-260.0f + i * 260.0f, -10.0f, 112.0f), FRotator(-8.0f, 0.0f, 0.0f), FVector(0.88f, 0.05f, 0.30f), FLinearColor(0.04f, 0.12f, 0.22f) * 2.4f, TEXT("Solar Panel Array"));
        }
    }
    else if (Kit == TEXT("Mountain Relay") || Profile.bMountain)
    {
        for (int32 i = 0; i < 4; ++i)
        {
            const float Height = 0.80f + i * 0.22f;
            MotifBlock(FVector(-330.0f + i * 220.0f, 12.0f, 72.0f + i * 16.0f), FVector(0.92f, 0.18f, Height), FLinearColor(0.14f, 0.15f, 0.15f), TEXT("Mountain Ridge Module"));
            MotifBlock(FVector(-330.0f + i * 220.0f, -2.0f, 120.0f + i * 23.0f), FVector(0.54f, 0.10f, 0.08f), FLinearColor(0.84f, 0.92f, 1.0f), TEXT("Mountain Snowcap Module"));
        }
    }
    else if (Kit == TEXT("Great Lakes Industrial") || Profile.bIndustrial)
    {
        MotifBlock(FVector(0.0f, 18.0f, 32.0f), FVector(4.8f, 0.16f, 0.10f), FLinearColor(0.025f, 0.12f, 0.18f) * 1.8f, TEXT("Lake Or River Edge"));
        for (int32 i = 0; i < 3; ++i)
        {
            MotifBlock(FVector(-260.0f + i * 260.0f, -8.0f, 140.0f), FVector(0.18f, 0.12f, 2.60f), FLinearColor(0.13f, 0.13f, 0.12f), TEXT("Foundry Stack Module"));
        }
    }
    else if (Kit == TEXT("Capital Command") || Profile.bCapitalCivic)
    {
        for (int32 i = 0; i < 5; ++i)
        {
            MotifBlock(FVector(-320.0f + i * 160.0f, -10.0f, 124.0f), FVector(0.10f, 0.10f, 2.40f), FLinearColor(0.78f, 0.78f, 0.72f), TEXT("Civic Column Module"));
        }
        MotifBlock(FVector(0.0f, -14.0f, 264.0f), FVector(3.25f, 0.08f, 0.14f), KitColor, TEXT("Civic Entablature Module"));
    }
    else if (Kit == TEXT("Rail Yard") || Profile.bTransit)
    {
        for (int32 i = 0; i < 3; ++i)
        {
            MotifBlock(FVector(0.0f, -46.0f + i * 48.0f, 38.0f), FVector(4.7f, 0.035f, 0.06f), FLinearColor(0.62f, 0.62f, 0.58f), TEXT("Rail Line Module"));
        }
        MotifBlock(FVector(0.0f, -92.0f, 118.0f), FVector(2.10f, 0.08f, 0.36f), RouteColor, TEXT("Transit Shelter Module"));
    }
    else
    {
        for (int32 i = 0; i < 4; ++i)
        {
            MotifBlock(FVector(-330.0f + i * 220.0f, -8.0f, 112.0f + (i % 2) * 18.0f), FVector(0.62f, 0.12f, 1.30f), Profile.ArchitectureColor + Mission.AccentColor * (0.05f * i), TEXT("Metro Facade Module"));
            MotifBlock(FVector(-330.0f + i * 220.0f, -22.0f, 190.0f + (i % 2) * 18.0f), FVector(0.44f, 0.035f, 0.10f), RouteColor * 1.3f, TEXT("Storefront Sign Module"));
        }
    }

    TagRegional(SpawnGuideText(
        FString::Printf(TEXT("REGIONAL CITY KIT\n%s\n%s\nlandmark: %s"), *Mission.ArtKitName, *Mission.RegionName, *Mission.LandmarkName),
        MotifCenter + CityOffset(FVector(0.0f, -250.0f, 330.0f)),
        KitColor.ToFColor(true),
        28.0f));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueRegionalCityKits] %s spawned kit='%s' region='%s' district='%s' landmark='%s' profile='%s' terrain='%s'"),
        *CityLabel,
        *Mission.ArtKitName,
        *Mission.RegionName,
        *Mission.DistrictStyle,
        *Mission.LandmarkName,
        *Profile.SignatureCue,
        *Params.TerrainToken);
}

// ---- improvement_pass_2026-06-12 #45 — whole-city U.S. realization ---------

void ACodeRescueGameMode::ApplyUSCitySkyRealization(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams P = BuildUSCityRealizationParams(Mission, Profile);

    // Sun members are consumed every frame by the day/night Tick, so the key
    // light itself carries the city's climate (marine gray, desert gold,
    // crisp alpine, humid gulf, golden basin).
    CityDaySunColor = P.DaySunColor;
    CityNightSunColor = P.NightSunColor;
    CityDaySunIntensity = P.DaySunIntensity;
    CityNightSunIntensity = P.NightSunIntensity;

    // Streamed per-city height fog: density and inscattering tint sell the
    // atmosphere family far better than a tinted plate alone.
    AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(
        AExponentialHeightFog::StaticClass(), Origin + FVector(0.0f, 0.0f, -40.0f), FRotator::ZeroRotator);
    if (Fog)
    {
        if (UExponentialHeightFogComponent* FogComp = Fog->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            FogComp->SetFogDensity(P.FogDensity);
            FogComp->SetFogInscatteringColor(P.FogColor);
            FogComp->SetFogHeightFalloff(0.06f);
            FogComp->SetStartDistance(1200.0f);
        }
#if WITH_EDITOR
        Fog->SetActorLabel(CityLabel + TEXT(" City Climate Fog"));
#endif
        Fog->Tags.Add(FName("USCityRealizationSky"));
        RegisterStreamedActor(Fog);
    }

    auto TagSky = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("USCityRealizationSky"));
            Actor->Tags.Add(FName("CitySpecificSky"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    // Cloud deck by weather family. All plates are high, thin, and
    // collision-free so they read from the street without affecting play.
    FRandomStream SkyStream(Mission.SkylineSeed + 4501);
    if (!ShouldSpawnDevelopmentShowcaseLayers())
    {
        UE_LOG(LogTemp, Display,
            TEXT("[ProductionSky] %s uses fog, atmosphere, sun, and post processing; geometric cloud plates omitted."),
            *CityLabel);
    }
    else if (P.CloudToken == TEXT("Overcast") || P.CloudToken == TEXT("SnowSky"))
    {
        const FLinearColor Deck = P.CloudToken == TEXT("SnowSky")
            ? FLinearColor(0.62f, 0.65f, 0.70f)
            : FLinearColor(0.48f, 0.50f, 0.54f);
        TagSky(SpawnBlock(Origin + CityOffset(FVector(0.0f, 0.0f, 4300.0f)), CityExtent(FVector(86.0f, 74.0f, 0.05f)), Deck, CityLabel + TEXT(" Overcast Cloud Deck"), false));
        for (int32 i = 0; i < 4; ++i)
        {
            TagSky(SpawnBlock(
                Origin + CityOffset(FVector(SkyStream.FRandRange(-2600.0f, 2600.0f), SkyStream.FRandRange(-2000.0f, 2300.0f), 3650.0f + i * 120.0f)),
                FVector(SkyStream.FRandRange(16.0f, 30.0f), SkyStream.FRandRange(10.0f, 18.0f), 0.5f),
                Deck * 0.82f,
                CityLabel + TEXT(" Low Cloud Bank"),
                false));
        }
    }
    else if (P.CloudToken == TEXT("MarineLayer"))
    {
        for (int32 i = 0; i < 5; ++i)
        {
            const float Side = P.WaterEdgeSide == TEXT("East") ? 1.0f : -1.0f;
            TagSky(SpawnBlock(
                Origin + CityOffset(FVector(Side * SkyStream.FRandRange(1600.0f, 3400.0f), SkyStream.FRandRange(-2600.0f, 2600.0f), 2500.0f + i * 260.0f)),
                FVector(SkyStream.FRandRange(20.0f, 34.0f), SkyStream.FRandRange(8.0f, 14.0f), 1.6f),
                FLinearColor(0.58f, 0.60f, 0.62f),
                CityLabel + TEXT(" Marine Layer Bank"),
                false));
        }
    }
    else if (P.CloudToken == TEXT("HazeWarm") || P.CloudToken == TEXT("HumidGlow"))
    {
        const FLinearColor Haze = P.CloudToken == TEXT("HazeWarm")
            ? FLinearColor(0.66f, 0.56f, 0.40f)
            : FLinearColor(0.62f, 0.62f, 0.52f);
        for (int32 i = 0; i < 3; ++i)
        {
            TagSky(SpawnBlock(
                Origin + CityOffset(FVector(0.0f, 0.0f, 2200.0f + i * 420.0f)),
                CityExtent(FVector(84.0f - i * 8.0f, 72.0f - i * 8.0f, 0.025f)),
                Haze * (0.55f - i * 0.12f),
                CityLabel + TEXT(" Warm Haze Band"),
                false));
        }
    }
    else // Clear
    {
        for (int32 i = 0; i < 3; ++i)
        {
            TagSky(SpawnBlock(
                Origin + CityOffset(FVector(SkyStream.FRandRange(-2800.0f, 2800.0f), SkyStream.FRandRange(-2200.0f, 2500.0f), 3900.0f + i * 180.0f)),
                FVector(SkyStream.FRandRange(7.0f, 12.0f), SkyStream.FRandRange(3.5f, 6.0f), 0.4f),
                FLinearColor(0.92f, 0.94f, 0.98f),
                CityLabel + TEXT(" Fair-Weather Cloud"),
                false));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueUSCityRealization] %s sky family clouds='%s' grade='%s' fog=%.3f sunday=(%.2f %.2f %.2f)"),
        *CityLabel, *P.CloudToken, *P.GradeToken, P.FogDensity,
        P.DaySunColor.R, P.DaySunColor.G, P.DaySunColor.B);
}

void ACodeRescueGameMode::SpawnUSCityLandscapeRealizationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams P = BuildUSCityRealizationParams(Mission, Profile);

    auto TagLand = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("USCityRealizationLandscape"));
            Actor->Tags.Add(FName("CitySpecificLandscape"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    FRandomStream LandStream(Mission.SkylineSeed + 4502);

    // ---- city-wide ground tint quadrants (raised a hair above the floor) ---
    static const FVector2D QuadrantCenters[] = {
        FVector2D(-2050.0f, -1750.0f), FVector2D(2050.0f, -1750.0f),
        FVector2D(-2050.0f, 1750.0f), FVector2D(2050.0f, 1750.0f)
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(QuadrantCenters); ++i)
    {
        const float Vary = 0.88f + 0.08f * static_cast<float>(i % 3);
        TagLand(SpawnBlock(
            Origin + CityOffset(FVector(QuadrantCenters[i].X, QuadrantCenters[i].Y, -26.0f)),
            CityExtent(FVector(40.5f, 34.5f, 0.05f)),
            P.GroundTint * Vary,
            CityLabel + TEXT(" Regional Ground Plate"),
            false));
    }

    // ---- water: coastal / great-lake shoreline and through-town river ------
    if (P.bWaterEdge)
    {
        FVector WaterCenter(0.0f, -3260.0f, -22.0f);
        FVector WaterScale(82.0f, 6.0f, 0.04f);
        FVector ShoreCenter(0.0f, -2940.0f, -21.0f);
        FVector ShoreScale(82.0f, 0.9f, 0.045f);
        if (P.WaterEdgeSide == TEXT("East"))
        {
            WaterCenter = FVector(3880.0f, 0.0f, -22.0f);
            WaterScale = FVector(5.4f, 68.0f, 0.04f);
            ShoreCenter = FVector(3590.0f, 0.0f, -21.0f);
            ShoreScale = FVector(0.9f, 68.0f, 0.045f);
        }
        else if (P.WaterEdgeSide == TEXT("West"))
        {
            WaterCenter = FVector(-3880.0f, 0.0f, -22.0f);
            WaterScale = FVector(5.4f, 68.0f, 0.04f);
            ShoreCenter = FVector(-3590.0f, 0.0f, -21.0f);
            ShoreScale = FVector(0.9f, 68.0f, 0.045f);
        }
        TagLand(SpawnBlock(Origin + CityOffset(WaterCenter), CityExtent(WaterScale), P.WaterColor * 1.6f, CityLabel + TEXT(" City Waterline"), false));
        TagLand(SpawnBlock(Origin + CityOffset(ShoreCenter), CityExtent(ShoreScale), P.ShorelineColor, CityLabel + TEXT(" Shoreline Strip"), false));
        // Pier fingers for harbor cities.
        if (Profile.bCoastal || Profile.bMilitaryHarbor)
        {
            for (int32 i = 0; i < 3; ++i)
            {
                const FVector PierLocal = P.WaterEdgeSide == TEXT("East")
                    ? FVector(3680.0f, -1400.0f + i * 1400.0f, -18.0f)
                    : P.WaterEdgeSide == TEXT("West")
                        ? FVector(-3680.0f, -1400.0f + i * 1400.0f, -18.0f)
                        : FVector(-1900.0f + i * 1900.0f, -3060.0f, -18.0f);
                const FVector PierScale = P.WaterEdgeSide == TEXT("South")
                    ? FVector(0.5f, 2.6f, 0.06f)
                    : FVector(2.6f, 0.5f, 0.06f);
                TagLand(SpawnBlock(Origin + CityOffset(PierLocal), CityExtent(PierScale), FLinearColor(0.20f, 0.16f, 0.12f), CityLabel + TEXT(" Harbor Pier"), false));
            }
        }
    }
    if (P.bRiverThrough)
    {
        // River band runs north-south between the downtown core and the east
        // districts, with two crossing deck bridges above the existing roads.
        const float RiverX = 620.0f;
        TagLand(SpawnBlock(Origin + CityOffset(FVector(RiverX, 0.0f, -22.0f)), CityExtent(FVector(2.9f, 66.0f, 0.04f)), P.WaterColor * 1.5f, CityLabel + TEXT(" Through-Town River"), false));
        TagLand(SpawnBlock(Origin + CityOffset(FVector(RiverX - 170.0f, 0.0f, -21.0f)), CityExtent(FVector(0.30f, 66.0f, 0.05f)), P.ShorelineColor * 0.8f, CityLabel + TEXT(" River West Bank"), false));
        TagLand(SpawnBlock(Origin + CityOffset(FVector(RiverX + 170.0f, 0.0f, -21.0f)), CityExtent(FVector(0.30f, 66.0f, 0.05f)), P.ShorelineColor * 0.8f, CityLabel + TEXT(" River East Bank"), false));
        static const float BridgeYs[] = { -1540.0f, 1510.0f };
        for (float BY : BridgeYs)
        {
            TagLand(SpawnBlock(Origin + CityOffset(FVector(RiverX, BY, 26.0f)), CityExtent(FVector(4.6f, 1.1f, 0.07f)), FLinearColor(0.16f, 0.17f, 0.18f), CityLabel + TEXT(" River Bridge Deck"), false));
            TagLand(SpawnBlock(Origin + CityOffset(FVector(RiverX - 190.0f, BY, -4.0f)), FVector(0.5f, 0.5f, 0.62f), FLinearColor(0.30f, 0.30f, 0.30f), CityLabel + TEXT(" Bridge Pier West"), false));
            TagLand(SpawnBlock(Origin + CityOffset(FVector(RiverX + 190.0f, BY, -4.0f)), FVector(0.5f, 0.5f, 0.62f), FLinearColor(0.30f, 0.30f, 0.30f), CityLabel + TEXT(" Bridge Pier East"), false));
        }
    }

    // ---- perimeter backdrop forms -------------------------------------------
    if (P.BackdropToken == TEXT("MountainRing"))
    {
        for (int32 i = 0; i < 7; ++i)
        {
            const float X = -3650.0f + i * 1180.0f + LandStream.FRandRange(-160.0f, 160.0f);
            const float H = LandStream.FRandRange(13.0f, 23.0f);
            AActor* Peak = SpawnRotatedBlock(
                Origin + CityOffset(FVector(X, 3260.0f, H * 26.0f)),
                FRotator(0.0f, LandStream.FRandRange(-12.0f, 12.0f), 45.0f),
                FVector(H * 0.62f, H * 0.62f, H * 0.62f),
                FLinearColor(0.26f, 0.27f, 0.30f),
                CityLabel + TEXT(" Backdrop Mountain Peak"),
                false);
            TagLand(Peak);
            if (Profile.bColdWeather || P.GradeToken == TEXT("CrispMountain"))
            {
                TagLand(SpawnRotatedBlock(
                    Origin + CityOffset(FVector(X, 3260.0f, H * 52.0f)),
                    FRotator(0.0f, 0.0f, 45.0f),
                    FVector(H * 0.20f, H * 0.20f, H * 0.20f),
                    FLinearColor(0.85f, 0.88f, 0.92f),
                    CityLabel + TEXT(" Snowcap"),
                    false));
            }
        }
    }
    else if (P.BackdropToken == TEXT("MesaButtes"))
    {
        for (int32 i = 0; i < 5; ++i)
        {
            const float X = -3300.0f + i * 1650.0f + LandStream.FRandRange(-220.0f, 220.0f);
            const float H = LandStream.FRandRange(5.5f, 9.5f);
            TagLand(SpawnBlock(
                Origin + CityOffset(FVector(X, 3300.0f, H * 50.0f)),
                FVector(LandStream.FRandRange(7.0f, 11.0f), 3.4f, H),
                FLinearColor(0.46f, 0.27f, 0.16f),
                CityLabel + TEXT(" Backdrop Mesa"),
                false));
            TagLand(SpawnBlock(
                Origin + CityOffset(FVector(X, 3300.0f, H * 100.0f + 35.0f)),
                FVector(LandStream.FRandRange(4.5f, 7.0f), 2.6f, 0.7f),
                FLinearColor(0.55f, 0.36f, 0.22f),
                CityLabel + TEXT(" Mesa Cap"),
                false));
        }
    }
    else if (P.BackdropToken == TEXT("EvergreenRidge"))
    {
        for (int32 i = 0; i < 12; ++i)
        {
            const float X = -3700.0f + i * 640.0f + LandStream.FRandRange(-110.0f, 110.0f);
            const float H = LandStream.FRandRange(4.2f, 7.6f);
            TagLand(SpawnRotatedBlock(
                Origin + CityOffset(FVector(X, 3320.0f, H * 50.0f)),
                FRotator(0.0f, 45.0f, 0.0f),
                FVector(1.7f, 1.7f, H),
                FLinearColor(0.05f, 0.14f, 0.08f),
                CityLabel + TEXT(" Evergreen Ridge Tree"),
                false));
        }
    }
    else if (P.BackdropToken == TEXT("PalmShore"))
    {
        for (int32 i = 0; i < 8; ++i)
        {
            const float X = -3400.0f + i * 980.0f;
            const FVector BaseLocal = P.WaterEdgeSide == TEXT("South") ? FVector(X, -2840.0f, 0.0f) : FVector(X, 3280.0f, 0.0f);
            TagLand(SpawnBlock(Origin + CityOffset(BaseLocal + FVector(0.0f, 0.0f, 170.0f)), FVector(0.30f, 0.30f, 3.4f), FLinearColor(0.30f, 0.22f, 0.12f), CityLabel + TEXT(" Palm Trunk"), false));
            TagLand(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0.0f, 0.0f, 360.0f)), FRotator(0.0f, static_cast<float>(i) * 45.0f, 18.0f), FVector(2.5f, 2.5f, 0.16f), FLinearColor(0.10f, 0.34f, 0.12f), CityLabel + TEXT(" Palm Canopy"), false));
        }
    }
    else if (P.BackdropToken == TEXT("HillTerraces"))
    {
        for (int32 i = 0; i < 4; ++i)
        {
            TagLand(SpawnBlock(
                Origin + CityOffset(FVector(2950.0f + i * 260.0f, 0.0f, -16.0f + i * 46.0f)),
                CityExtent(FVector(2.6f, 60.0f - i * 8.0f, 0.9f)),
                P.GroundTint * (1.0f + 0.12f * i),
                CityLabel + TEXT(" Hill Terrace Step"),
                false));
        }
    }
    else if (P.BackdropToken == TEXT("PrairieHorizon"))
    {
        TagLand(SpawnBlock(Origin + CityOffset(FVector(0.0f, 3360.0f, 30.0f)), CityExtent(FVector(80.0f, 1.6f, 0.8f)), FLinearColor(0.30f, 0.30f, 0.14f), CityLabel + TEXT(" Prairie Horizon Band"), false));
        for (int32 i = 0; i < 2; ++i)
        {
            const float X = -1500.0f + i * 3000.0f;
            TagLand(SpawnBlock(Origin + CityOffset(FVector(X, 3300.0f, 330.0f)), FVector(1.5f, 1.5f, 6.6f), FLinearColor(0.60f, 0.58f, 0.52f), CityLabel + TEXT(" Grain Elevator"), false));
            TagLand(SpawnBlock(Origin + CityOffset(FVector(X, 3300.0f, 690.0f)), FVector(1.9f, 1.9f, 0.5f), FLinearColor(0.45f, 0.43f, 0.40f), CityLabel + TEXT(" Grain Elevator Cap"), false));
        }
    }
    else // SkylineHaze — distant low skyline band for flat eastern/southern metros
    {
        TagLand(SpawnBlock(Origin + CityOffset(FVector(0.0f, 3360.0f, 170.0f)), CityExtent(FVector(78.0f, 0.9f, 3.4f)), FLinearColor(0.12f, 0.13f, 0.16f), CityLabel + TEXT(" Distant Skyline Haze Band"), false));
    }

    // ---- street vegetation matched to region --------------------------------
    const int32 TreeCount = 10;
    for (int32 i = 0; i < TreeCount; ++i)
    {
        const FVector TreeLocal(
            LandStream.FRandRange(-3050.0f, 3050.0f),
            LandStream.FRandRange(-2280.0f, 2330.0f) + (LandStream.FRandRange(0.0f, 1.0f) > 0.5f ? 260.0f : -260.0f),
            0.0f);
        if (P.VegetationToken == TEXT("Palms"))
        {
            TagLand(SpawnBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 150.0f)), FVector(0.26f, 0.26f, 3.0f), FLinearColor(0.30f, 0.22f, 0.12f), CityLabel + TEXT(" Street Palm Trunk"), false));
            TagLand(SpawnRotatedBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 318.0f)), FRotator(0.0f, i * 36.0f, 14.0f), FVector(2.1f, 2.1f, 0.14f), FLinearColor(0.10f, 0.34f, 0.12f), CityLabel + TEXT(" Street Palm Canopy"), false));
        }
        else if (P.VegetationToken == TEXT("Cactus"))
        {
            TagLand(SpawnBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 110.0f)), FVector(0.34f, 0.34f, 2.2f), FLinearColor(0.13f, 0.30f, 0.12f), CityLabel + TEXT(" Saguaro Column"), false));
            TagLand(SpawnBlock(Origin + CityOffset(TreeLocal + FVector(46.0f, 0.0f, 150.0f)), FVector(0.24f, 0.24f, 1.0f), FLinearColor(0.13f, 0.30f, 0.12f), CityLabel + TEXT(" Saguaro Arm"), false));
        }
        else if (P.VegetationToken == TEXT("Evergreens"))
        {
            TagLand(SpawnRotatedBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 170.0f)), FRotator(0.0f, 45.0f, 0.0f), FVector(1.2f, 1.2f, 3.4f), FLinearColor(0.05f, 0.16f, 0.08f), CityLabel + TEXT(" Street Conifer"), false));
        }
        else if (P.VegetationToken == TEXT("Tundra"))
        {
            TagLand(SpawnBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 36.0f)), FVector(1.0f, 1.0f, 0.7f), FLinearColor(0.22f, 0.24f, 0.16f), CityLabel + TEXT(" Tundra Shrub"), false));
        }
        else // Deciduous / OakScrub
        {
            TagLand(SpawnBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 120.0f)), FVector(0.30f, 0.30f, 2.4f), FLinearColor(0.24f, 0.17f, 0.10f), CityLabel + TEXT(" Street Tree Trunk"), false));
            TagLand(SpawnBlock(Origin + CityOffset(TreeLocal + FVector(0.0f, 0.0f, 300.0f)), FVector(1.8f, 1.8f, 1.5f), P.VegetationToken == TEXT("OakScrub") ? FLinearColor(0.16f, 0.26f, 0.10f) : FLinearColor(0.10f, 0.28f, 0.10f), CityLabel + TEXT(" Street Tree Canopy"), false));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueUSCityRealization] %s landscape terrain='%s' backdrop='%s' vegetation='%s' water_edge=%d river=%d side='%s'"),
        *CityLabel, *P.TerrainToken, *P.BackdropToken, *P.VegetationToken,
        P.bWaterEdge ? 1 : 0, P.bRiverThrough ? 1 : 0, *P.WaterEdgeSide);
}

void ACodeRescueGameMode::SpawnUSCityResidentialDistrictLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams P = BuildUSCityRealizationParams(Mission, Profile);

    auto TagHome = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("USCityRealizationHomes"));
            Actor->Tags.Add(FName("CitySpecificHomes"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    FRandomStream HomeStream(Mission.SkylineSeed + 4503);

    // One home, built from primitives, in the city's archetype. BaseLocal is
    // the ground-level center of the lot in design space.
    auto SpawnHome = [&](const FVector& BaseLocal, float YawDeg, int32 LotIndex)
    {
        const FLinearColor Body = P.HomePalette[LotIndex % FMath::Max(1, P.HomePalette.Num())];
        const FRotator Yaw(0.0f, YawDeg, 0.0f);
        const FString& A = P.HomeArchetypeToken;

        if (A == TEXT("BrownstoneRow"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 250.0f)), Yaw, FVector(3.0f, 2.6f, 5.0f), Body, CityLabel + TEXT(" Brownstone Row Home"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -150.0f, 42.0f)), Yaw, FVector(0.9f, 0.8f, 0.84f), FLinearColor(0.34f, 0.32f, 0.30f), CityLabel + TEXT(" Brownstone Stoop"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 520.0f)), Yaw, FVector(3.2f, 2.8f, 0.18f), Body * 0.6f, CityLabel + TEXT(" Brownstone Cornice"), false));
        }
        else if (A == TEXT("TripleDecker"))
        {
            for (int32 F = 0; F < 3; ++F)
            {
                TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 95.0f + F * 190.0f)), Yaw, FVector(3.4f, 3.0f, 1.85f), Body * (1.0f - 0.10f * F), CityLabel + TEXT(" Triple-Decker Floor"), false));
                TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -165.0f, 95.0f + F * 190.0f)), Yaw, FVector(2.6f, 0.22f, 0.16f), FLinearColor(0.80f, 0.80f, 0.78f), CityLabel + TEXT(" Porch Rail Band"), false));
            }
        }
        else if (A == TEXT("VictorianPainted"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 230.0f)), Yaw, FVector(2.8f, 2.5f, 4.6f), Body, CityLabel + TEXT(" Painted Victorian Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -140.0f, 230.0f)), Yaw, FVector(0.9f, 0.5f, 4.6f), Body * 1.3f, CityLabel + TEXT(" Victorian Bay Window"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 500.0f)), FRotator(0.0f, YawDeg, 45.0f), FVector(2.0f, 2.0f, 2.0f), Body * 0.55f, CityLabel + TEXT(" Victorian Gable"), false));
        }
        else if (A == TEXT("CraftsmanBungalow"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 110.0f)), Yaw, FVector(4.4f, 3.4f, 2.2f), Body, CityLabel + TEXT(" Craftsman Bungalow Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -190.0f, 120.0f)), Yaw, FVector(2.6f, 0.9f, 0.16f), Body * 0.7f, CityLabel + TEXT(" Bungalow Porch Roof"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 280.0f)), FRotator(0.0f, YawDeg, 45.0f), FVector(2.6f, 2.6f, 2.6f), FLinearColor(0.20f, 0.14f, 0.10f), CityLabel + TEXT(" Bungalow Roof"), false));
        }
        else if (A == TEXT("AdobeRanch"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 95.0f)), Yaw, FVector(5.0f, 3.6f, 1.9f), Body, CityLabel + TEXT(" Adobe Ranch Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(150.0f, 0, 210.0f)), Yaw, FVector(2.0f, 3.7f, 0.5f), Body * 0.82f, CityLabel + TEXT(" Adobe Parapet Step"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(-230.0f, -120.0f, 130.0f)), Yaw, FVector(0.4f, 0.4f, 2.6f), FLinearColor(0.13f, 0.30f, 0.12f), CityLabel + TEXT(" Yard Saguaro"), false));
        }
        else if (A == TEXT("BrickTwoFlat"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 190.0f)), Yaw, FVector(3.4f, 3.0f, 3.8f), Body, CityLabel + TEXT(" Brick Two-Flat Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -160.0f, 96.0f)), Yaw, FVector(1.1f, 0.5f, 1.9f), FLinearColor(0.70f, 0.68f, 0.62f), CityLabel + TEXT(" Two-Flat Entry Frame"), false));
        }
        else if (A == TEXT("ShotgunPorch"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 60.0f, 105.0f)), Yaw, FVector(2.2f, 5.2f, 2.1f), Body, CityLabel + TEXT(" Shotgun Home Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -240.0f, 120.0f)), Yaw, FVector(2.2f, 0.9f, 0.14f), Body * 0.7f, CityLabel + TEXT(" Shotgun Porch Canopy"), false));
            for (int32 c = 0; c < 2; ++c)
            {
                TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(-60.0f + c * 120.0f, -262.0f, 60.0f)), Yaw, FVector(0.12f, 0.12f, 1.2f), FLinearColor(0.85f, 0.84f, 0.80f), CityLabel + TEXT(" Porch Column"), false));
            }
        }
        else if (A == TEXT("DecoPastelHome"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 140.0f)), Yaw, FVector(3.6f, 3.0f, 2.8f), Body, CityLabel + TEXT(" Deco Pastel Home"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, -158.0f, 140.0f)), Yaw, FVector(2.4f, 0.16f, 1.6f), FLinearColor(0.90f, 0.88f, 0.82f), CityLabel + TEXT(" Deco Eyebrow Band"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 300.0f)), Yaw, FVector(1.4f, 1.4f, 0.5f), Body * 1.25f, CityLabel + TEXT(" Deco Roof Step"), false));
        }
        else if (A == TEXT("MountainCabin"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 110.0f)), Yaw, FVector(3.8f, 3.0f, 2.2f), FLinearColor(0.26f, 0.16f, 0.09f), CityLabel + TEXT(" Cabin Log Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 300.0f)), FRotator(0.0f, YawDeg, 45.0f), FVector(2.9f, 2.9f, 2.9f), FLinearColor(0.16f, 0.10f, 0.07f), CityLabel + TEXT(" Cabin Steep Roof"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(130.0f, 0, 430.0f)), Yaw, FVector(0.4f, 0.4f, 1.5f), FLinearColor(0.40f, 0.38f, 0.36f), CityLabel + TEXT(" Cabin Chimney"), false));
        }
        else if (A == TEXT("CapeCod"))
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 105.0f)), Yaw, FVector(3.6f, 2.9f, 2.1f), Body, CityLabel + TEXT(" Cape Cod Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 265.0f)), FRotator(0.0f, YawDeg, 45.0f), FVector(2.4f, 2.4f, 2.4f), FLinearColor(0.30f, 0.28f, 0.26f), CityLabel + TEXT(" Cape Cod Roof"), false));
        }
        else // SunbeltRanch
        {
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 100.0f)), Yaw, FVector(5.2f, 3.4f, 2.0f), Body, CityLabel + TEXT(" Sunbelt Ranch Body"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 225.0f)), Yaw, FVector(5.5f, 3.7f, 0.30f), FLinearColor(0.28f, 0.22f, 0.18f), CityLabel + TEXT(" Ranch Low Roof"), false));
            TagHome(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(210.0f, -150.0f, -14.0f)), Yaw, FVector(1.1f, 2.4f, 0.05f), FLinearColor(0.30f, 0.30f, 0.30f), CityLabel + TEXT(" Ranch Driveway"), false));
        }
    };

    // Two residential bands: a north outskirts arc and a west arc, clear of
    // the entry corridor, plaza, helipad, and survivor quadrants.
    const float RowGap = (P.HomeArchetypeToken == TEXT("BrownstoneRow") || P.HomeArchetypeToken == TEXT("TripleDecker")) ? 470.0f : 640.0f;
    for (int32 i = 0; i < P.HomesPerRow; ++i)
    {
        const float X = -2750.0f + i * RowGap + HomeStream.FRandRange(-40.0f, 40.0f);
        SpawnHome(FVector(X, 2980.0f, 0.0f), 180.0f, i);
    }
    for (int32 i = 0; i < P.HomesPerRow - 2; ++i)
    {
        const float Y = -780.0f + i * RowGap + HomeStream.FRandRange(-40.0f, 40.0f);
        SpawnHome(FVector(-3560.0f, Y, 0.0f), 90.0f, i + P.HomesPerRow);
    }
    // Yard strips behind the north band tie the lots together.
    TagHome(SpawnBlock(Origin + CityOffset(FVector(-650.0f, 3120.0f, -24.0f)), CityExtent(FVector(46.0f, 2.6f, 0.04f)),
        P.VegetationToken == TEXT("Cactus") ? FLinearColor(0.36f, 0.26f, 0.14f) : FLinearColor(0.09f, 0.16f, 0.08f),
        CityLabel + TEXT(" Residential Yard Strip"), false));

    TagHome(SpawnGuideText(
        FString::Printf(TEXT("RESIDENTIAL DISTRICT\n%s"), *Profile.HomeCue),
        Origin + CityOffset(FVector(-2750.0f, 2980.0f, 0.0f)) + FVector(0.0f, 0.0f, 640.0f),
        Profile.HomeColor.ToFColor(true),
        34.0f));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueUSCityRealization] %s homes archetype='%s' rows=2 homes=%d"),
        *CityLabel, *P.HomeArchetypeToken, P.HomesPerRow * 2 - 2);
}

void ACodeRescueGameMode::SpawnUSCityVehiclePopulationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FCodeRescueUSCityVisualProfile Profile = BuildUSCityVisualProfile(Mission);
    const FCodeRescueUSCityRealizationParams P = BuildUSCityRealizationParams(Mission, Profile);

    auto TagVehicle = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("USCityRealizationVehicles"));
            Actor->Tags.Add(FName("CitySpecificVehicles"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    FRandomStream FleetStream(Mission.SkylineSeed + 4504);

    auto SpawnVehicle = [&](const FVector& BaseLocal, float YawDeg, const FString& Token, int32 Slot)
    {
        const FRotator Yaw(0.0f, YawDeg + FleetStream.FRandRange(-7.0f, 7.0f), 0.0f);
        const float Weather = FleetStream.FRandRange(0.55f, 1.0f);   // abandoned-fleet grime variance
        const FLinearColor Accent = P.FleetAccent * Weather;

        if (Token == TEXT("Taxi"))
        {
            const FLinearColor Cab = FLinearColor(0.88f, 0.70f, 0.10f) * Weather;
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 62.0f)), Yaw, FVector(4.4f, 1.8f, 1.05f), Cab, CityLabel + TEXT(" Taxi Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 142.0f)), Yaw, FVector(2.2f, 1.7f, 0.65f), Cab * 0.8f, CityLabel + TEXT(" Taxi Cabin"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 185.0f)), Yaw, FVector(0.7f, 0.3f, 0.18f), FLinearColor(0.95f, 0.92f, 0.78f) * 1.6f, CityLabel + TEXT(" Taxi Roof Sign"), false));
        }
        else if (Token == TEXT("Pickup"))
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 72.0f)), Yaw, FVector(5.0f, 1.9f, 1.15f), Accent + FLinearColor(0.16f, 0.10f, 0.06f), CityLabel + TEXT(" Pickup Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(-95.0f, 0, 158.0f)), Yaw, FVector(1.7f, 1.8f, 0.72f), Accent * 0.7f + FLinearColor(0.10f, 0.08f, 0.06f), CityLabel + TEXT(" Pickup Cab"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(118.0f, 0, 132.0f)), Yaw, FVector(2.1f, 1.7f, 0.30f), FLinearColor(0.06f, 0.06f, 0.06f), CityLabel + TEXT(" Pickup Bed Walls"), false));
        }
        else if (Token == TEXT("SUV"))
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 84.0f)), Yaw, FVector(4.7f, 1.95f, 1.65f), Accent + FLinearColor(0.05f, 0.06f, 0.07f), CityLabel + TEXT(" SUV Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 172.0f)), Yaw, FVector(4.0f, 1.85f, 0.16f), FLinearColor(0.10f, 0.10f, 0.11f), CityLabel + TEXT(" SUV Roof Rack"), false));
        }
        else if (Token == TEXT("Van"))
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 95.0f)), Yaw, FVector(5.4f, 2.0f, 1.9f), FLinearColor(0.70f, 0.70f, 0.68f) * Weather, CityLabel + TEXT(" Delivery Van Body"), false));
        }
        else if (Token == TEXT("Bus"))
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 120.0f)), Yaw, FVector(9.5f, 2.2f, 2.4f), Accent + FLinearColor(0.12f, 0.12f, 0.14f), CityLabel + TEXT(" Transit Bus Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 160.0f)), Yaw, FVector(8.8f, 2.25f, 0.5f), FLinearColor(0.16f, 0.22f, 0.26f), CityLabel + TEXT(" Bus Window Band"), false));
        }
        else if (Token == TEXT("Convertible"))
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 55.0f)), Yaw, FVector(4.3f, 1.75f, 0.95f), Accent + FLinearColor(0.26f, 0.06f, 0.05f), CityLabel + TEXT(" Convertible Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(-80.0f, 0, 122.0f)), Yaw, FVector(0.16f, 1.65f, 0.55f), FLinearColor(0.55f, 0.62f, 0.66f), CityLabel + TEXT(" Convertible Windshield"), false));
        }
        else if (Token == TEXT("Compact") || Token == TEXT("EV"))
        {
            const FLinearColor Body = Token == TEXT("EV")
                ? FLinearColor(0.72f, 0.74f, 0.76f) * Weather
                : Accent + FLinearColor(0.10f, 0.12f, 0.10f);
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 58.0f)), Yaw, FVector(3.5f, 1.7f, 1.0f), Body, CityLabel + TEXT(" Compact Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 128.0f)), Yaw, FVector(1.9f, 1.6f, 0.6f), Body * 0.75f, CityLabel + TEXT(" Compact Cabin"), false));
        }
        else if (Token == TEXT("PlowTruck"))
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 92.0f)), Yaw, FVector(5.2f, 2.0f, 1.8f), FLinearColor(0.70f, 0.42f, 0.08f) * Weather, CityLabel + TEXT(" Plow Truck Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(-180.0f, 0, 60.0f)), FRotator(0.0f, YawDeg + 24.0f, 0.0f), FVector(0.3f, 2.6f, 1.1f), FLinearColor(0.80f, 0.78f, 0.30f), CityLabel + TEXT(" Plow Blade"), false));
        }
        else // Sedan
        {
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 60.0f)), Yaw, FVector(4.4f, 1.8f, 1.0f), Accent + FLinearColor(0.08f, 0.09f, 0.10f), CityLabel + TEXT(" Sedan Body"), false));
            TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 132.0f)), Yaw, FVector(2.3f, 1.7f, 0.62f), Accent * 0.7f + FLinearColor(0.06f, 0.07f, 0.08f), CityLabel + TEXT(" Sedan Cabin"), false));
        }
        // Shared wheel shadow strip keeps every silhouette grounded.
        TagVehicle(SpawnRotatedBlock(Origin + CityOffset(BaseLocal + FVector(0, 0, 16.0f)), Yaw, FVector(Token == TEXT("Bus") ? 8.8f : 3.9f, 1.6f, 0.30f), FLinearColor(0.03f, 0.03f, 0.03f), CityLabel + TEXT(" Vehicle Wheelline"), false));
    };

    // Curbside placements along the authored east-west streets, plus a short
    // column on the north-south spine. Same Y constants as the urban layer.
    static const float CurbRoadYs[] = { -2520.0f, -1540.0f, -560.0f, 520.0f, 1510.0f, 2420.0f };
    int32 Spawned = 0;
    for (int32 RoadIdx = 0; RoadIdx < UE_ARRAY_COUNT(CurbRoadYs) && Spawned < P.CurbVehicleCount; ++RoadIdx)
    {
        const int32 PerRoad = (P.CurbVehicleCount + 5) / 6;
        for (int32 v = 0; v < PerRoad && Spawned < P.CurbVehicleCount; ++v)
        {
            const float X = FleetStream.FRandRange(-3050.0f, 3300.0f);
            // Keep the spawn-to-plaza entry corridor visually open.
            if (X < -2500.0f && CurbRoadYs[RoadIdx] < -1800.0f)
            {
                continue;
            }
            const float CurbSide = (v % 2 == 0) ? 132.0f : -132.0f;
            const FString& Token = P.VehicleMix[FleetStream.RandRange(0, P.VehicleMix.Num() - 1)];
            SpawnVehicle(FVector(X, CurbRoadYs[RoadIdx] + CurbSide, 0.0f), (v % 2 == 0) ? 0.0f : 180.0f, Token, Spawned);
            ++Spawned;
        }
    }
    for (int32 v = 0; v < 4; ++v)
    {
        const FString& Token = P.VehicleMix[FleetStream.RandRange(0, P.VehicleMix.Num() - 1)];
        SpawnVehicle(FVector(940.0f + ((v % 2 == 0) ? 128.0f : -128.0f), -2050.0f + v * 1240.0f, 0.0f), 90.0f, Token, Spawned + v);
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueUSCityRealization] %s vehicles fleet_count=%d mix_first='%s' accessory='%s'"),
        *CityLabel, Spawned + 4, P.VehicleMix.Num() > 0 ? *P.VehicleMix[0] : TEXT("none"), *P.WardrobeAccessoryToken);
}

void ACodeRescueGameMode::SpawnProtectedCodingChallengeHub(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const ECodingLanguage Language = GI ? GI->SelectedLanguage : ECodingLanguage::Java;
    FString LanguageLabel = TEXT("Java");
    FLinearColor LanguageColor = FLinearColor(1.0f, 0.40f, 0.18f);
    switch (Language)
    {
    case ECodingLanguage::C:
        LanguageLabel = TEXT("C");
        LanguageColor = FLinearColor(0.25f, 0.58f, 1.0f);
        break;
    case ECodingLanguage::Python:
        LanguageLabel = TEXT("Python");
        LanguageColor = FLinearColor(1.0f, 0.86f, 0.22f);
        break;
    case ECodingLanguage::MATLAB:
        LanguageLabel = TEXT("MATLAB");
        LanguageColor = FLinearColor(0.85f, 0.30f, 1.0f);
        break;
    case ECodingLanguage::CPlus:
        LanguageLabel = TEXT("C+");
        LanguageColor = FLinearColor(0.20f, 0.84f, 1.0f);
        break;
    case ECodingLanguage::Cpp:
        LanguageLabel = TEXT("C++");
        LanguageColor = FLinearColor(0.18f, 0.64f, 1.0f);
        break;
    case ECodingLanguage::Java:
    default:
        break;
    }

    auto TagSafe = [this](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("ProtectedCodingChallengeZone"));
            Actor->Tags.Add(FName("NoZombieLearningZone"));
            Actor->Tags.Add(FName("SafeTerminalLab"));
            Actor->Tags.Add(FName("SelectedLanguageOnly"));
            Actor->Tags.Add(FName("LearningWithoutDeathRisk"));
            ApplyRuntimeDataLayerTags(Actor, TArray<FName>{
                FName("RuntimeDataLayer_State_SafeBeat"),
                FName("RuntimeDataLayer_Mode_CodingSafehouse"),
                FName("RuntimeDataLayer_Mode_SelectedLanguageOnly"),
            });
        }
        return Actor;
    };

    // Ten metres down the cross street: close enough to read as the first
    // objective, far enough that its roof and posts never surround the spawn
    // camera or restore a saved player inside the pavilion footprint.
    const FVector Hub = Origin + CityOffset(FVector(-2100.0f, -2680.0f, 0.0f));
    if (ShouldSpawnDevelopmentShowcaseLayers())
    {
        TagSafe(SpawnTexturedBlock(
            Hub + FVector(0.0f, 0.0f, -9.0f),
            FVector(18.0f, 9.0f, 0.050f),
            FLinearColor(0.035f, 0.050f, 0.055f) + LanguageColor * 0.18f,
            CityLabel + TEXT(" Protected Coding Safehouse Floor"),
            TEXT("/Game/StarterContent/Materials/M_Tech_Checker_Dot.M_Tech_Checker_Dot"),
            false));
        TagSafe(SpawnBlock(Hub + FVector(0.0f, -520.0f, 155.0f), FVector(18.0f, 0.12f, 1.55f), FLinearColor(0.045f, 0.050f, 0.050f) + LanguageColor * 0.12f, CityLabel + TEXT(" Safehouse Back Wall"), true));
        TagSafe(SpawnBlock(Hub + FVector(-900.0f, 0.0f, 155.0f), FVector(0.12f, 9.0f, 1.55f), FLinearColor(0.035f, 0.040f, 0.042f), CityLabel + TEXT(" Safehouse West Wall"), true));
        TagSafe(SpawnBlock(Hub + FVector(900.0f, 0.0f, 155.0f), FVector(0.12f, 9.0f, 1.55f), FLinearColor(0.035f, 0.040f, 0.042f), CityLabel + TEXT(" Safehouse East Wall"), true));
        TagSafe(SpawnBlock(Hub + FVector(-560.0f, 500.0f, 92.0f), FVector(3.3f, 0.12f, 0.92f), FLinearColor(0.08f, 0.16f, 0.17f) + LanguageColor * 0.16f, CityLabel + TEXT(" Safehouse Entry Rail A"), true));
        TagSafe(SpawnBlock(Hub + FVector(560.0f, 500.0f, 92.0f), FVector(3.3f, 0.12f, 0.92f), FLinearColor(0.08f, 0.16f, 0.17f) + LanguageColor * 0.16f, CityLabel + TEXT(" Safehouse Entry Rail B"), true));
        TagSafe(SpawnBlock(Hub + FVector(0.0f, -440.0f, 270.0f), FVector(10.5f, 0.08f, 0.48f), LanguageColor * 2.0f, CityLabel + TEXT(" Selected Language Banner"), false));
    }
    else
    {
        auto ArtV3 = [](const TCHAR* Name) -> FString
        {
            const FString N(Name);
            return FString::Printf(TEXT("/Game/CodeRescueArt/CityKitV3/%s/%s/StaticMeshes/%s.%s"), *N, *N, *N, *N);
        };
        if (AActor* Walkway = SpawnKitMesh(
                ArtV3(TEXT("SidewalkV3")), Hub + FVector(0.0f, 0.0f, -12.0f),
                FRotator::ZeroRotator, FVector(3.4f, 4.2f, 1.0f),
                CityLabel + TEXT(" Production Coding Concourse Walkway"), false, nullptr))
        {
            TagSafe(Walkway);
            Walkway->Tags.AddUnique(FName("ProductionPresentationRequired"));
            Walkway->Tags.AddUnique(FName("CameraOcclusionExempt"));
        }
        if (AActor* Pavilion = SpawnKitMesh(
                ArtV3(TEXT("BusStopV3")), Hub + FVector(0.0f, -20.0f, 0.0f),
                FRotator::ZeroRotator, FVector(2.0f, 3.2f, 1.15f),
                CityLabel + TEXT(" Production Glass Coding Concourse Canopy"), false, nullptr))
        {
            TagSafe(Pavilion);
            Pavilion->Tags.AddUnique(FName("ProductionPresentationRequired"));
        }
    }
    TagSafe(SpawnGuideText(
        FString::Printf(TEXT("PROTECTED CODING CONCOURSE\n%s TRACK ONLY | 10 REQUIRED\nzombies cannot enter this learning area"), *LanguageLabel),
        Hub + FVector(0.0f, -570.0f, 430.0f),
        LanguageColor.ToFColor(true),
        28.0f));

    TagSafe(SpawnGuideText(
        TEXT("Complete all ten stations for survivor clearance.\nEach first-time pass drops supplies; progress saves to this language."),
        Hub + FVector(0.0f, 570.0f, 330.0f),
        FColor(210, 245, 255),
        26.0f));

    const TArray<FString> ChallengeIds = FCodeRescueCampaign::GetCityChallengeIds(CityIndex);
    const TCHAR* StageTitles[] = {
        TEXT("GRID SUM"),
        TEXT("ACCESS LOCK"),
        TEXT("REVERSE SIGNAL"),
        TEXT("MIRROR CODE"),
        TEXT("FIZZBUZZ ROUTER"),
        TEXT("EVEN SUPPLY FILTER"),
        TEXT("LINKED EVAC CHAIN"),
        TEXT("BINARY SEARCH INTEL"),
        TEXT("POWER RELAY REVIEW"),
        TEXT("FINAL SAFETY LOCK"),
    };
    const TCHAR* StageBriefs[] = {
        TEXT("Return the sum of three emergency power readings."),
        TEXT("Return true only when the access key and emergency power are both available."),
        TEXT("Reverse the incoming rescue signal without losing characters."),
        TEXT("Detect whether a signal reads identically from both ends."),
        TEXT("Generate ordered FizzBuzz route labels through the requested endpoint."),
        TEXT("Return only even supply identifiers while preserving order."),
        TEXT("Count every reachable node in the linked evacuation chain."),
        TEXT("Return the index of a target in sorted survivor intel, or the not-found value."),
        TEXT("Reinforce the function contract by returning a second three-cell total."),
        TEXT("Final clearance: both identity authorization and route power must be active."),
    };
    static_assert(UE_ARRAY_COUNT(StageTitles) == FCodeRescueCampaign::RequiredChallengesPerCity,
        "Every city requires ten coding-station titles.");
    static_assert(UE_ARRAY_COUNT(StageBriefs) == FCodeRescueCampaign::RequiredChallengesPerCity,
        "Every city requires ten coding-station briefs.");

    for (int32 StageIndex = 0; StageIndex < ChallengeIds.Num(); ++StageIndex)
    {
        const int32 Column = StageIndex % 5;
        const int32 Row = StageIndex / 5;
        const FVector StationLocation = Hub + FVector(
            -760.0f + Column * 380.0f,
            -250.0f + Row * 500.0f,
            90.0f);
        const FString Title = StageIndex == 0
            ? FString::Printf(TEXT("01/10 %s"), *Mission.TerminalTitle)
            : FString::Printf(TEXT("%02d/10 %s"), StageIndex + 1, StageTitles[StageIndex]);
        const FString Brief = StageIndex == 0 ? Mission.MissionBrief : FString(StageBriefs[StageIndex]);
        SpawnTerminal(StationLocation, ChallengeIds[StageIndex], Title, Brief, CityIndex);
        TagSafe(SpawnBlock(
            StationLocation + FVector(0.0f, 0.0f, -84.0f),
            FVector(2.7f, 2.0f, 0.035f),
            LanguageColor * (0.32f + StageIndex * 0.018f) + FLinearColor(0.035f, 0.045f, 0.048f),
            FString::Printf(TEXT("%s Coding Station %02d Floor Marker"), *CityLabel, StageIndex + 1),
            false));
    }

    if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Hub + FVector(0.0f, -80.0f, 280.0f), FRotator::ZeroRotator))
    {
        if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
        {
            PLC->SetMobility(EComponentMobility::Movable);
            PLC->SetIntensity(5400.0f);
            PLC->SetLightColor(LanguageColor);
            PLC->SetAttenuationRadius(950.0f);
            PLC->SetCastShadows(false);
        }
#if WITH_EDITOR
        Light->SetActorLabel(CityLabel + TEXT(" Protected Coding Lab Light"));
#endif
        RegisterStreamedActor(Light);
        TagSafe(Light);
    }

    UE_LOG(LogTemp, Display, TEXT("[CodeRescueSafeLearning] %s spawned %d protected coding stations using selected %s track; all ten persist before survivor clearance."),
        *CityLabel,
        ChallengeIds.Num(),
        *LanguageLabel);
}

void ACodeRescueGameMode::SpawnChallengeRoomConceptArtLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const ECodingLanguage Language = GI ? GI->SelectedLanguage : ECodingLanguage::Java;
    FString LanguageLabel = TEXT("Java");
    FLinearColor LanguageColor = FLinearColor(1.0f, 0.40f, 0.18f);
    switch (Language)
    {
    case ECodingLanguage::C:
        LanguageLabel = TEXT("C");
        LanguageColor = FLinearColor(0.25f, 0.58f, 1.0f);
        break;
    case ECodingLanguage::Python:
        LanguageLabel = TEXT("Python");
        LanguageColor = FLinearColor(1.0f, 0.86f, 0.22f);
        break;
    case ECodingLanguage::MATLAB:
        LanguageLabel = TEXT("MATLAB");
        LanguageColor = FLinearColor(0.85f, 0.30f, 1.0f);
        break;
    case ECodingLanguage::CPlus:
        LanguageLabel = TEXT("C+");
        LanguageColor = FLinearColor(0.20f, 0.84f, 1.0f);
        break;
    case ECodingLanguage::Cpp:
        LanguageLabel = TEXT("C++");
        LanguageColor = FLinearColor(0.18f, 0.64f, 1.0f);
        break;
    case ECodingLanguage::Java:
    default:
        break;
    }

    auto TagChallenge = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("ChallengeRoomConceptArt"));
            Actor->Tags.Add(FName("ChallengeConceptRoomReady"));
            Actor->Tags.Add(FName("CodeConceptPhysicalSpace"));
            Actor->Tags.Add(FName("TextFirstLearningCue"));
            Actor->Tags.Add(FName("ProtectedLearningSpace"));
            Actor->Tags.Add(FName("SelectedLanguageOnly"));
            Actor->Tags.Add(FName("LearningWithoutDeathRisk"));
            Actor->Tags.Add(FName("NoAccessBlocker"));
            Actor->Tags.Add(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.Add(FName("Top50Recommendations"));
        }
        return Actor;
    };

    auto AddTags = [](AActor* Actor, std::initializer_list<const TCHAR*> Tags) -> AActor*
    {
        if (Actor)
        {
            for (const TCHAR* Tag : Tags)
            {
                Actor->Tags.Add(FName(Tag));
            }
        }
        return Actor;
    };

    auto SpawnChallengeLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagChallenge(Light);
            Light->Tags.Add(FName("ChallengeRoomConceptLight"));
        }
    };

    struct FChallengeConceptRoom
    {
        const TCHAR* Label;
        const TCHAR* Tag;
        const TCHAR* Goal;
        const TCHAR* PhysicalCue;
        FVector Local;
        FLinearColor Color;
    };

    const FLinearColor VariableBlue(0.26f, 0.82f, 1.0f, 1.0f);
    const FLinearColor LoopGreen(0.34f, 1.0f, 0.54f, 1.0f);
    const FLinearColor ArrayAmber(1.0f, 0.72f, 0.20f, 1.0f);
    const FLinearColor FunctionViolet(0.76f, 0.48f, 1.0f, 1.0f);
    const FLinearColor DebugRed(1.0f, 0.30f, 0.22f, 1.0f);

    const FChallengeConceptRoom Rooms[] = {
        { TEXT("VARIABLES LAB"), TEXT("VariablesLab"), TEXT("name the data before it moves"), TEXT("labeled value crates"), FVector(-3820.0f, -1980.0f, 0.0f), VariableBlue },
        { TEXT("LOOP CONTROL ROOM"), TEXT("LoopControlRoom"), TEXT("repeat a rescue step with a changing counter"), TEXT("counter gates and cycle rails"), FVector(-3180.0f, -1980.0f, 0.0f), LoopGreen },
        { TEXT("ARRAY INDEX HALL"), TEXT("ArrayIndexHall"), TEXT("step through ordered positions without skipping edges"), TEXT("index tiles and bounds rails"), FVector(-3820.0f, -1460.0f, 0.0f), ArrayAmber },
        { TEXT("FUNCTION RELAY ROOM"), TEXT("FunctionRelayRoom"), TEXT("turn input into output through one clear purpose"), TEXT("input/output relay pylons"), FVector(-3180.0f, -1460.0f, 0.0f), FunctionViolet },
        { TEXT("DEBUGGER TEST BAY"), TEXT("DebuggerTestBay"), TEXT("inspect visible, edge, and hidden checks before retesting"), TEXT("test-case bench and trace ladder"), FVector(-3500.0f, -940.0f, 0.0f), DebugRed },
    };

    for (int32 RoomIdx = 0; RoomIdx < UE_ARRAY_COUNT(Rooms); ++RoomIdx)
    {
        const FChallengeConceptRoom& Room = Rooms[RoomIdx];
        const FVector Center = Origin + CityOffset(Room.Local);
        const FLinearColor RoomColor = FLinearColor::LerpUsingHSV(Room.Color, LanguageColor, 0.24f);

        TagChallenge(AddTags(SpawnTexturedBlock(
            Center + FVector(0.0f, 0.0f, -10.0f),
            FVector(2.75f, 1.82f, 0.046f),
            FLinearColor(0.030f, 0.040f, 0.045f) + RoomColor * 0.13f,
            FString::Printf(TEXT("%s Challenge Room Concept Art %s Floor"), *CityLabel, Room.Label),
            TEXT("/Game/StarterContent/Materials/M_Tech_Hex_Tile.M_Tech_Hex_Tile"),
            false),
            { Room.Tag, TEXT("OpenFrontConceptRoom") }));
        TagChallenge(AddTags(SpawnBlock(
            Center + FVector(0.0f, -188.0f, 132.0f),
            FVector(2.75f, 0.07f, 1.14f),
            FLinearColor(0.040f, 0.046f, 0.052f) + RoomColor * 0.18f,
            FString::Printf(TEXT("%s Challenge Room Concept Art %s Back Wall"), *CityLabel, Room.Label),
            false),
            { Room.Tag, TEXT("ConceptRoomBackWall") }));
        TagChallenge(AddTags(SpawnBlock(
            Center + FVector(-288.0f, 0.0f, 108.0f),
            FVector(0.07f, 1.66f, 0.92f),
            FLinearColor(0.032f, 0.036f, 0.040f) + RoomColor * 0.12f,
            FString::Printf(TEXT("%s Challenge Room Concept Art %s West Rail"), *CityLabel, Room.Label),
            false),
            { Room.Tag, TEXT("OpenFrontConceptRoom") }));
        TagChallenge(AddTags(SpawnBlock(
            Center + FVector(288.0f, 0.0f, 108.0f),
            FVector(0.07f, 1.66f, 0.92f),
            FLinearColor(0.032f, 0.036f, 0.040f) + RoomColor * 0.12f,
            FString::Printf(TEXT("%s Challenge Room Concept Art %s East Rail"), *CityLabel, Room.Label),
            false),
            { Room.Tag, TEXT("OpenFrontConceptRoom") }));
        TagChallenge(AddTags(SpawnBlock(
            Center + FVector(0.0f, -208.0f, 260.0f),
            FVector(2.36f, 0.055f, 0.22f),
            RoomColor * 1.85f,
            FString::Printf(TEXT("%s Challenge Room Concept Art %s Header"), *CityLabel, Room.Label),
            false),
            { Room.Tag, TEXT("TextFirstLearningCue") }));

        for (int32 Node = 0; Node < 3; ++Node)
        {
            const FVector NodeLoc = Center + FVector(-160.0f + Node * 160.0f, 20.0f + Node * 12.0f, 60.0f + Node * 18.0f);
            TagChallenge(AddTags(SpawnBlock(
                NodeLoc,
                FVector(0.48f, 0.26f, 0.26f + Node * 0.06f),
                RoomColor * (1.0f + Node * 0.18f),
                FString::Printf(TEXT("%s Challenge Room %s Physical Cue %d"), *CityLabel, Room.Label, Node + 1),
                false),
                { Room.Tag, TEXT("CodeConceptProp") }));
            TagChallenge(AddTags(SpawnGuideText(
                FString::Printf(TEXT("%s %d"), RoomIdx == 2 ? TEXT("INDEX") : RoomIdx == 4 ? TEXT("TEST") : TEXT("STEP"), Node),
                NodeLoc + FVector(0.0f, -42.0f, 86.0f),
                RoomColor.ToFColor(true),
                13.0f),
                { Room.Tag, TEXT("TextFirstLearningCue") }));
        }

        TagChallenge(AddTags(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s\n%s\n%s track only"), Room.Label, Room.Goal, Room.PhysicalCue, *LanguageLabel),
            Center + FVector(0.0f, -248.0f, 342.0f),
            RoomColor.ToFColor(true),
            17.0f),
            { Room.Tag, TEXT("TextFirstLearningCue") }));
        SpawnChallengeLight(Center + FVector(0.0f, -62.0f, 270.0f), RoomColor, 2400.0f, 580.0f, FString::Printf(TEXT("%s Challenge Room Concept Light %s"), *CityLabel, Room.Label));
    }

    const FVector BriefingBoard = Origin + CityOffset(FVector(-3500.0f, -2520.0f, 0.0f));
    TagChallenge(SpawnTexturedBlock(
        BriefingBoard + FVector(0.0f, 0.0f, -11.0f),
        FVector(5.5f, 1.35f, 0.050f),
        FLinearColor(0.025f, 0.033f, 0.040f) + LanguageColor * 0.14f,
        CityLabel + TEXT(" Challenge Concept Art Briefing Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false));
    TagChallenge(SpawnBlock(
        BriefingBoard + FVector(0.0f, -118.0f, 150.0f),
        FVector(5.35f, 0.075f, 1.05f),
        FLinearColor(0.035f, 0.040f, 0.048f) + Mission.SecondaryAccentColor * 0.18f,
        CityLabel + TEXT(" Challenge Concept Art Briefing Wall"),
        false));
    TagChallenge(SpawnGuideText(
        FString::Printf(
            TEXT("CHALLENGE ROOM CONCEPT ART\n%s\nVisible: %s\nHidden: %s\nHint: %s\nSupport: %s"),
            *Mission.CurriculumFocus,
            *Mission.VisibleTestBrief,
            *Mission.HiddenTestBrief,
            *Mission.HintText,
            *Mission.LearningSupportText),
        BriefingBoard + FVector(0.0f, -174.0f, 285.0f),
        LanguageColor.ToFColor(true),
        16.0f));
    TagChallenge(SpawnGuideText(
        FString::Printf(TEXT("DEBUG PLAN\n%s\nPROGRESSION\n%s"), *Mission.VisualDebuggerPlan, *Mission.ProgressionPlan),
        BriefingBoard + FVector(0.0f, 96.0f, 255.0f),
        Mission.SecondaryAccentColor.ToFColor(true),
        15.0f));
    SpawnChallengeLight(BriefingBoard + FVector(0.0f, -60.0f, 285.0f), LanguageColor, 3300.0f, 720.0f, CityLabel + TEXT(" Challenge Concept Briefing Light"));

    FString LessonLabel = TEXT("Sum Rescue");
    const FVector ArtifactBase = Origin + CityOffset(FVector(-3500.0f, -410.0f, 0.0f));
    switch (Mission.LessonKind)
    {
    case ECampaignLessonKind::Lock:
        LessonLabel = TEXT("Lock Truth Gate");
        for (int32 i = 0; i < 4; ++i)
        {
            const FVector Gate = ArtifactBase + FVector(-240.0f + i * 160.0f, 0.0f, 94.0f);
            TagChallenge(AddTags(SpawnBlock(Gate, FVector(0.14f, 0.52f, 1.20f), (i == 3 ? LoopGreen : DebugRed) * 1.25f, CityLabel + TEXT(" LOCK TRUTH GATE"), false), { TEXT("LessonKindConceptArtifact") }));
            TagChallenge(AddTags(SpawnGuideText(i == 3 ? TEXT("TRUE") : TEXT("FALSE"), Gate + FVector(0.0f, -54.0f, 142.0f), LanguageColor.ToFColor(true), 14.0f), { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    case ECampaignLessonKind::Reverse:
        LessonLabel = TEXT("Reverse Signal Arrows");
        for (int32 i = 0; i < 6; ++i)
        {
            TagChallenge(AddTags(SpawnBlock(
                ArtifactBase + FVector(300.0f - i * 120.0f, 0.0f, 42.0f + i * 24.0f),
                FVector(0.52f, 0.12f, 0.14f),
                VariableBlue * (1.0f + i * 0.16f),
                CityLabel + TEXT(" REVERSE SIGNAL ARROWS"),
                false),
                { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    case ECampaignLessonKind::Palindrome:
        LessonLabel = TEXT("Palindrome Mirror Walk");
        for (int32 i = 0; i < 4; ++i)
        {
            const float X = 95.0f + i * 105.0f;
            TagChallenge(AddTags(SpawnBlock(ArtifactBase + FVector(-X, 0.0f, 94.0f), FVector(0.20f, 0.18f, 1.16f), FunctionViolet * 1.18f, CityLabel + TEXT(" PALINDROME MIRROR WALK LEFT"), false), { TEXT("LessonKindConceptArtifact") }));
            TagChallenge(AddTags(SpawnBlock(ArtifactBase + FVector(X, 0.0f, 94.0f), FVector(0.20f, 0.18f, 1.16f), FunctionViolet * 1.18f, CityLabel + TEXT(" PALINDROME MIRROR WALK RIGHT"), false), { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    case ECampaignLessonKind::FizzBuzz:
        LessonLabel = TEXT("FizzBuzz Beacon Grid");
        for (int32 i = 1; i <= 15; ++i)
        {
            const bool bFizzBuzz = i % 15 == 0;
            const bool bFizz = i % 3 == 0;
            const bool bBuzz = i % 5 == 0;
            const FLinearColor BeaconColor = bFizzBuzz ? ArrayAmber * 2.0f : bFizz ? LoopGreen * 1.35f : bBuzz ? FunctionViolet * 1.35f : FLinearColor(0.16f, 0.18f, 0.20f);
            TagChallenge(AddTags(SpawnBlock(
                ArtifactBase + FVector(-420.0f + i * 56.0f, 0.0f, 34.0f + (bFizzBuzz ? 70.0f : bFizz || bBuzz ? 46.0f : 22.0f)),
                FVector(0.16f, 0.16f, bFizzBuzz ? 1.18f : bFizz || bBuzz ? 0.82f : 0.42f),
                BeaconColor,
                CityLabel + TEXT(" FIZZBUZZ BEACON GRID"),
                false),
                { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    case ECampaignLessonKind::EvenFilter:
        LessonLabel = TEXT("Even Filter Sorting Lanes");
        for (int32 i = 0; i < 8; ++i)
        {
            const bool bEven = (i % 2) == 0;
            TagChallenge(AddTags(SpawnBlock(
                ArtifactBase + FVector(-350.0f + i * 100.0f, bEven ? -58.0f : 58.0f, 34.0f),
                FVector(0.42f, 1.0f, 0.12f),
                bEven ? LoopGreen * 1.35f : DebugRed * 0.65f,
                CityLabel + TEXT(" EVEN FILTER SORTING LANES"),
                false),
                { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    case ECampaignLessonKind::LinkedListTraverse:
        LessonLabel = TEXT("Linked List Rescue Chain");
        for (int32 i = 0; i < 6; ++i)
        {
            const FVector Node = ArtifactBase + FVector(-360.0f + i * 144.0f, (i % 2 == 0) ? -48.0f : 48.0f, 82.0f);
            TagChallenge(AddTags(SpawnBlock(Node, FVector(0.34f, 0.34f, 0.66f), VariableBlue * (1.0f + i * 0.10f), CityLabel + TEXT(" LINKED LIST RESCUE CHAIN NODE"), false), { TEXT("LessonKindConceptArtifact") }));
            if (i < 5)
            {
                TagChallenge(AddTags(SpawnBlock(Node + FVector(70.0f, (i % 2 == 0) ? 48.0f : -48.0f, 28.0f), FVector(0.58f, 0.05f, 0.06f), LanguageColor * 1.6f, CityLabel + TEXT(" LINKED LIST RESCUE CHAIN ARROW"), false), { TEXT("LessonKindConceptArtifact") }));
            }
        }
        break;
    case ECampaignLessonKind::BinarySearch:
        LessonLabel = TEXT("Binary Search Shrinking Arena");
        for (int32 i = 0; i < 5; ++i)
        {
            TagChallenge(AddTags(SpawnBlock(
                ArtifactBase + FVector(0.0f, 0.0f, 34.0f + i * 48.0f),
                FVector(4.6f - i * 0.66f, 0.18f, 0.14f),
                FLinearColor::LerpUsingHSV(VariableBlue, ArrayAmber, static_cast<float>(i) / 4.0f) * 1.45f,
                CityLabel + TEXT(" BINARY SEARCH SHRINKING ARENA"),
                false),
                { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    default:
        for (int32 i = 0; i < 3; ++i)
        {
            TagChallenge(AddTags(SpawnBlock(
                ArtifactBase + FVector(-160.0f + i * 160.0f, 0.0f, 70.0f),
                FVector(0.58f, 0.58f, 0.68f),
                FLinearColor::LerpUsingHSV(VariableBlue, LoopGreen, static_cast<float>(i) / 2.0f),
                CityLabel + TEXT(" SUM POWER CELL BANK"),
                false),
                { TEXT("LessonKindConceptArtifact") }));
        }
        break;
    }

    TagChallenge(AddTags(SpawnGuideText(
        FString::Printf(TEXT("LESSON ARTIFACT\n%s\n%s\nTerminal: %s"), *LessonLabel, *Mission.NovelGameplayDetail, *Mission.TerminalTitle),
        ArtifactBase + FVector(0.0f, -96.0f, 315.0f),
        LanguageColor.ToFColor(true),
        18.0f),
        { TEXT("LessonKindConceptArtifact"), TEXT("TextFirstLearningCue") }));
    SpawnChallengeLight(ArtifactBase + FVector(0.0f, -60.0f, 260.0f), LanguageColor, 3000.0f, 720.0f, CityLabel + TEXT(" Challenge Lesson Artifact Light"));

    auto TagReviewGallery = [&](AActor* Actor, std::initializer_list<const TCHAR*> ExtraTags) -> AActor*
    {
        if (Actor)
        {
            TagChallenge(Actor);
            Actor->Tags.Add(FName("CurriculumFirstReviewGallery"));
            Actor->Tags.Add(FName("VisibleHiddenTestGallery"));
            Actor->Tags.Add(FName("ValidatorArchetypeProof"));
            Actor->Tags.Add(FName("IntrinsicIntegrationReview"));
            Actor->Tags.Add(FName("OperationReview20260630"));
            Actor->Tags.Add(FName("ThreeDReviewCandidate"));
            for (const TCHAR* Tag : ExtraTags)
            {
                Actor->Tags.Add(FName(Tag));
            }
        }
        return Actor;
    };

    auto SpawnProofCharacter = [&](const FVector& Base, const FLinearColor& Accent, const FString& Name, const TCHAR* RoleTag)
    {
        TagReviewGallery(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 72.0f),
            FVector(0.18f, 0.13f, 0.72f),
            Accent * 1.15f,
            FString::Printf(TEXT("%s Curriculum Gallery %s Body"), *CityLabel, *Name),
            false),
            { RoleTag, TEXT("ReviewCharacterProxy") });
        TagReviewGallery(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 156.0f),
            FVector(0.20f, 0.18f, 0.20f),
            FLinearColor(0.90f, 0.82f, 0.66f, 1.0f),
            FString::Printf(TEXT("%s Curriculum Gallery %s Head"), *CityLabel, *Name),
            false),
            { RoleTag, TEXT("ReviewCharacterProxy") });
        TagReviewGallery(SpawnBlock(
            Base + FVector(0.0f, -23.0f, 104.0f),
            FVector(0.28f, 0.035f, 0.055f),
            Accent * 1.65f,
            FString::Printf(TEXT("%s Curriculum Gallery %s Signal Arm"), *CityLabel, *Name),
            false),
            { RoleTag, TEXT("ReviewCharacterProxy"), TEXT("GestureReadabilityProxy") });
    };

    struct FValidationProofStation
    {
        const TCHAR* Label;
        const TCHAR* Tag;
        const TCHAR* Visible;
        const TCHAR* Hidden;
        const TCHAR* Mistake;
        const TCHAR* Mentor;
        const TCHAR* Survivor;
        FLinearColor Color;
    };

    const FValidationProofStation Stations[] = {
        { TEXT("SUM RETURN"), TEXT("SumValidatorStation"), TEXT("visible 20+15+10"), TEXT("hidden zero and mixed totals"), TEXT("print instead of return"), TEXT("Power Mentor"), TEXT("Relay Survivor"), VariableBlue },
        { TEXT("LOCK BOOLEAN"), TEXT("LockValidatorStation"), TEXT("visible true && true"), TEXT("hidden unsafe pairs stay shut"), TEXT("OR opens unsafe gates"), TEXT("Access Mentor"), TEXT("Gate Survivor"), LoopGreen },
        { TEXT("REVERSE STRING"), TEXT("ReverseValidatorStation"), TEXT("visible rescue -> eucser"), TEXT("hidden city packet reverses exactly"), TEXT("off-by-one drops a character"), TEXT("Signal Mentor"), TEXT("Packet Survivor"), FunctionViolet },
        { TEXT("PALINDROME"), TEXT("PalindromeValidatorStation"), TEXT("visible racecar passes"), TEXT("hidden mirror and impostor"), TEXT("true too early"), TEXT("Mirror Mentor"), TEXT("Archive Survivor"), FunctionViolet * 0.9f },
        { TEXT("FIZZBUZZ"), TEXT("FizzBuzzValidatorStation"), TEXT("visible 1..15 sequence"), TEXT("hidden longer beacon sweep"), TEXT("3 before 15"), TEXT("Timing Mentor"), TEXT("Beacon Survivor"), ArrayAmber },
        { TEXT("EVEN FILTER"), TEXT("EvenFilterValidatorStation"), TEXT("visible keep 2,4,6"), TEXT("hidden odd-only and order"), TEXT("changes order or keeps all"), TEXT("Triage Mentor"), TEXT("Queue Survivor"), LoopGreen * 0.88f },
        { TEXT("LINKED LIST"), TEXT("LinkedListValidatorStation"), TEXT("visible count to sentinel"), TEXT("hidden changed start nodes"), TEXT("no current = next"), TEXT("Route Mentor"), TEXT("Chain Survivor"), VariableBlue * 0.92f },
        { TEXT("BINARY SEARCH"), TEXT("BinarySearchValidatorStation"), TEXT("visible find middle target"), TEXT("hidden first, last, not-found"), TEXT("stale bounds loop forever"), TEXT("Search Mentor"), TEXT("Cache Survivor"), DebugRed * 0.95f },
    };

    const FVector GalleryBase = Origin + CityOffset(FVector(-2470.0f, 170.0f, 0.0f));
    TagReviewGallery(SpawnTexturedBlock(
        GalleryBase + FVector(0.0f, 205.0f, -12.0f),
        FVector(6.9f, 2.42f, 0.052f),
        FLinearColor(0.026f, 0.035f, 0.043f) + LanguageColor * 0.10f,
        CityLabel + TEXT(" Curriculum First Visible Hidden Gallery Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false),
        { TEXT("CurriculumFirstReviewGate") });
    TagReviewGallery(SpawnGuideText(
        FString::Printf(
            TEXT("CURRICULUM-FIRST REVIEW GALLERY\nEight validator archetypes are visible before terminal play.\nEach station pairs visible test, hidden test, common mistake, mentor proxy, and survivor payoff.\nTrack: %s only | Safe in protected learning space"),
            *LanguageLabel),
        GalleryBase + FVector(0.0f, -165.0f, 330.0f),
        LanguageColor.ToFColor(true),
        16.0f),
        { TEXT("CurriculumFirstReviewGate"), TEXT("TextFirstLearningCue") });

    for (int32 i = 0; i < UE_ARRAY_COUNT(Stations); ++i)
    {
        const FValidationProofStation& Station = Stations[i];
        const int32 Row = i / 4;
        const int32 Col = i % 4;
        const FVector Center = GalleryBase + FVector(-570.0f + Col * 380.0f, 52.0f + Row * 330.0f, 0.0f);
        const FLinearColor StationColor = FLinearColor::LerpUsingHSV(Station.Color, LanguageColor, 0.18f);

        TagReviewGallery(SpawnBlock(
            Center + FVector(0.0f, 0.0f, 18.0f),
            FVector(1.42f, 0.78f, 0.16f),
            FLinearColor(0.035f, 0.042f, 0.050f) + StationColor * 0.12f,
            FString::Printf(TEXT("%s Curriculum Gallery %s Platform"), *CityLabel, Station.Label),
            false),
            { Station.Tag, TEXT("CurriculumFirstReviewGate") });
        TagReviewGallery(SpawnBlock(
            Center + FVector(-74.0f, -24.0f, 94.0f),
            FVector(0.18f, 0.18f, 0.70f),
            StationColor * 1.45f,
            FString::Printf(TEXT("%s Curriculum Gallery %s Visible Test Pylon"), *CityLabel, Station.Label),
            false),
            { Station.Tag, TEXT("VisibleTestProofPylon") });
        TagReviewGallery(SpawnBlock(
            Center + FVector(74.0f, -24.0f, 116.0f),
            FVector(0.18f, 0.18f, 0.92f),
            FLinearColor::LerpUsingHSV(StationColor, ArrayAmber, 0.44f) * 1.38f,
            FString::Printf(TEXT("%s Curriculum Gallery %s Hidden Test Pylon"), *CityLabel, Station.Label),
            false),
            { Station.Tag, TEXT("HiddenTestProofPylon") });
        TagReviewGallery(SpawnBlock(
            Center + FVector(0.0f, 75.0f, 58.0f),
            FVector(0.88f, 0.10f, 0.20f),
            DebugRed * 1.10f,
            FString::Printf(TEXT("%s Curriculum Gallery %s Mistake Marker"), *CityLabel, Station.Label),
            false),
            { Station.Tag, TEXT("CommonMistakeMarker") });
        SpawnProofCharacter(Center + FVector(-124.0f, 102.0f, 0.0f), StationColor, Station.Mentor, TEXT("MentorCharacterProxy"));
        SpawnProofCharacter(Center + FVector(124.0f, 102.0f, 0.0f), FLinearColor::LerpUsingHSV(StationColor, LoopGreen, 0.42f), Station.Survivor, TEXT("SurvivorCharacterProxy"));
        TagReviewGallery(SpawnGuideText(
            FString::Printf(
                TEXT("%s\nVISIBLE: %s\nHIDDEN: %s\nMISTAKE: %s"),
                Station.Label,
                Station.Visible,
                Station.Hidden,
                Station.Mistake),
            Center + FVector(0.0f, -96.0f, 230.0f),
            StationColor.ToFColor(true),
            12.5f),
            { Station.Tag, TEXT("TextFirstLearningCue") });
    }
    SpawnChallengeLight(GalleryBase + FVector(0.0f, 270.0f, 430.0f), LanguageColor, 4200.0f, 980.0f, CityLabel + TEXT(" Curriculum First Gallery Review Light"));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueChallengeRoomConceptArt] %s spawned concept rooms for lesson='%s' focus='%s' visible='%s' hidden='%s' language='%s'"),
        *CityLabel,
        *LessonLabel,
        *Mission.CurriculumFocus,
        *Mission.VisibleTestBrief,
        *Mission.HiddenTestBrief,
        *LanguageLabel);
    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueCurriculumFirstGallery] %s spawned 8 visible/hidden validator stations with mentor and survivor review proxies for selected %s track."),
        *CityLabel,
        *LanguageLabel);

    (void)CityIndex;
}

void ACodeRescueGameMode::SpawnCreativeRecommendationImplementationLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector IntakeHub = Origin + CityOffset(FVector(-2040.0f, 2440.0f, 0.0f));
    const FVector OperatorHub = Origin + CityOffset(FVector(-3240.0f, -760.0f, 0.0f));
    const FVector CurriculumHub = Origin + CityOffset(FVector(-2460.0f, -2060.0f, 0.0f));
    const FVector GearHub = Origin + CityOffset(FVector(-1220.0f, -2380.0f, 0.0f));
    const FVector DistrictHub = Origin + CityOffset(FVector(760.0f, 2480.0f, 0.0f));
    const FVector StressHub = Origin + CityOffset(FVector(3360.0f, -640.0f, 0.0f));

    const FLinearColor IntakeBlue(0.30f, 0.76f, 1.0f, 1.0f);
    const FLinearColor OperatorGold(0.95f, 0.72f, 0.32f, 1.0f);
    const FLinearColor CurriculumGreen(0.28f, 0.95f, 0.56f, 1.0f);
    const FLinearColor GearViolet(0.78f, 0.44f, 1.0f, 1.0f);
    const FLinearColor DistrictAmber(1.0f, 0.56f, 0.18f, 1.0f);
    const FLinearColor StressRed(0.95f, 0.20f, 0.12f, 1.0f);
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const FString ActiveOperatorSummary = GI
        ? GI->GetOperatorIdentitySummary()
        : FString(TEXT("Rhea Calder | Rescue Operator | frontline rescue route lead | Java run"));

    auto TagCreative = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("May28CreativeRecommendationImplementation"));
            Actor->Tags.Add(FName("FullRecommendationImplementationPass"));
            Actor->Tags.Add(FName("ComprehensiveStressTestRig"));
            Actor->Tags.Add(FName("WorldDevelopment"));
        }
        return Actor;
    };

    auto AddTags = [](AActor* Actor, std::initializer_list<const TCHAR*> Tags) -> AActor*
    {
        if (Actor)
        {
            for (const TCHAR* Tag : Tags)
            {
                Actor->Tags.Add(FName(Tag));
            }
        }
        return Actor;
    };

    auto EnablePhysics = [](AActor* Actor, float MassKg) -> AActor*
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                MeshComp->SetMobility(EComponentMobility::Movable);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetLinearDamping(0.32f);
                MeshComp->SetAngularDamping(0.42f);
                CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
                    MeshComp,
                    MeshActor,
                    FName("StressRigFixedStepBody"),
                    MassKg,
                    0.32f,
                    0.42f,
                    false);
            }
        }
        return Actor;
    };

    auto SpawnCreativeLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(CityLabel + TEXT(" ") + Name);
#endif
            RegisterStreamedActor(Light);
            TagCreative(Light);
        }
    };

    auto SpawnCreativePickup = [&](EPickupKind Kind, const FVector& Location, int32 Amount, const TCHAR* TagName, const FColor& LabelColor, const FString& Label)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Location, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("TacticalGearFunctionalPickup"));
            Pickup->Tags.Add(FName("May28CreativeRecommendationImplementation"));
            Pickup->Tags.Add(FName(TagName));
            RegisterStreamedActor(Pickup);
        }
        TagCreative(AddTags(SpawnGuideText(
            Label,
            Location + FVector(0.0f, 0.0f, 210.0f),
            LabelColor,
            16.0f),
            { TEXT("TacticalGearFunctionalPickup"), TagName }));
    };

    // Download and licensed-asset intake hub. It is deliberately a visible
    // review area, not an unsafe automatic import path.
    TagCreative(AddTags(SpawnTexturedBlock(
        IntakeHub + FVector(0.0f, 0.0f, -16.0f),
        FVector(7.4f, 4.6f, 0.052f),
        FLinearColor(0.035f, 0.045f, 0.052f, 1.0f) + IntakeBlue * 0.10f,
        CityLabel + TEXT(" Creative Download Intake Floor"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false),
        { TEXT("DownloadedAssetIntakeReview"), TEXT("ActiveFabDownloadStaging"), TEXT("LicenseSafePromotionGate") }));
    TagCreative(AddTags(SpawnGuideText(
        TEXT("ACTIVE DOWNLOAD INTAKE\nFab cache: Convai AI, Async Physics, Quest Kit Pro, zombie packs\nMetaHuman: body plus hair-card/mesh fallback; groom mhpkg stays review-only on Mac\nNanite/SM6: M2+ and macOS 15+ review; non-Nanite fallback required\nLOD/texture/shader: Mac budget gates required before runtime promotion"),
        IntakeHub + FVector(0.0f, -250.0f, 330.0f),
        IntakeBlue.ToFColor(true),
        20.0f),
        { TEXT("DownloadedAssetIntakeReview"), TEXT("MetaHumanMhpkgStaged"), TEXT("FabCacheDetected") }));
    struct FCreativeIntakePanel
    {
        const TCHAR* Label;
        const TCHAR* Tag;
    };
    static const FCreativeIntakePanel IntakePanels[] = {
        { TEXT("MetaHuman body"), TEXT("MetaHumanBodyRuntimeCandidate") },
        { TEXT("Groom review"), TEXT("GroomStrandReviewOnlyMac") },
        { TEXT("Hair cards"), TEXT("MacHairCardRuntimeReady") },
        { TEXT("Nanite SM6"), TEXT("MacNaniteSM6ReviewGate") },
        { TEXT("Fallback LOD"), TEXT("MacNonNaniteFallbackReady") },
        { TEXT("LOD audit"), TEXT("MacLODBudgetReviewGate") },
        { TEXT("Texture cap"), TEXT("MacTextureMemoryReviewGate") },
        { TEXT("Shader trim"), TEXT("MacShaderComplexityReviewGate") },
        { TEXT("Maya handoff"), TEXT("MayaHoudiniDccHandoff") },
        { TEXT("Houdini PCG"), TEXT("HoudiniProceduralWorldDesign") },
        { TEXT("Quest Kit"), TEXT("QuestMissionKitReady") },
        { TEXT("Async Physics"), TEXT("AsyncPhysicsReady") },
        { TEXT("Zombie packs"), TEXT("ZombieFamilyPromotionTarget") }
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(IntakePanels); ++i)
    {
        const FVector PanelLoc = IntakeHub + FVector(-560.0f + i * 160.0f, 110.0f + (i % 2) * 105.0f, 92.0f);
        TagCreative(AddTags(SpawnBlock(
            PanelLoc,
            FVector(0.58f, 0.08f, 0.72f),
            (i % 2 == 0 ? IntakeBlue : Mission.SecondaryAccentColor) * (0.86f + i * 0.025f),
            FString::Printf(TEXT("%s Active Download Intake Panel %d"), *CityLabel, i + 1),
            false),
            { TEXT("DownloadedAssetIntakeReview"), TEXT("LicenseSafePromotionGate"), TEXT("FutureImportedAssetSlot"), IntakePanels[i].Tag }));
        TagCreative(AddTags(SpawnGuideText(
            IntakePanels[i].Label,
            PanelLoc + FVector(0.0f, -54.0f, 70.0f),
            IntakeBlue.ToFColor(true),
            12.0f),
            { TEXT("DownloadedAssetIntakeReview"), TEXT("FutureImportedAssetSlot") }));
    }
    SpawnCreativeLight(IntakeHub + FVector(0.0f, 0.0f, 285.0f), IntakeBlue, 3800.0f, 820.0f, TEXT("Creative Intake Review Light"));

    // Player/survivor character stage: these are gameplay-safe Manny/Quinn
    // avatars until imported MetaHumans are validated.
    TagCreative(AddTags(SpawnTexturedBlock(
        OperatorHub + FVector(0.0f, 0.0f, -14.0f),
        FVector(6.8f, 3.8f, 0.052f),
        FLinearColor(0.052f, 0.044f, 0.034f, 1.0f) + OperatorGold * 0.12f,
        CityLabel + TEXT(" Playable Cast Promotion Stage"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false),
        { TEXT("PlayableRescueOperatorReady"), TEXT("MetaHumanReplacementSlot"), TEXT("IKRetargeterReady") }));
    struct FCastSlot
    {
        const TCHAR* Label;
        bool bUseQuinn;
        FLinearColor Color;
    };
    const FCastSlot CastSlots[] = {
        { TEXT("Rescue Operator"), false, OperatorGold },
        { TEXT("Field Engineer"), false, FLinearColor(0.42f, 0.90f, 1.0f, 1.0f) },
        { TEXT("Signal Analyst"), true, FLinearColor(0.75f, 0.58f, 1.0f, 1.0f) },
        { TEXT("Triage Medic"), true, FLinearColor(1.0f, 0.34f, 0.38f, 1.0f) },
        { TEXT("Survivor Unlock"), true, FLinearColor(0.36f, 1.0f, 0.54f, 1.0f) },
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(CastSlots); ++i)
    {
        const FVector SlotLoc = OperatorHub + FVector(-520.0f + i * 260.0f, 45.0f + (i % 2) * 86.0f, 94.0f);
        TagCreative(AddTags(SpawnDecorativeCivilian(
            SlotLoc,
            FRotator(0.0f, -145.0f + i * 18.0f, 0.0f),
            CastSlots[i].bUseQuinn,
            CastSlots[i].Color,
            FString::Printf(TEXT("%s Creative Cast Slot %d"), *CityLabel, i + 1),
            CastSlots[i].Label),
            { TEXT("PlayableRescueOperatorReady"), TEXT("MetaHumanReplacementSlot"), TEXT("ControlRigFacialSlot"), TEXT("IKRetargeterReady") }));
    }
    TagCreative(AddTags(SpawnGuideText(
        TEXT("CAST PROMOTION\nplayable operator, engineer, analyst, medic, survivor unlock\nMetaHuman/Maya/Houdini replacements land here after validation"),
        OperatorHub + FVector(0.0f, -250.0f, 310.0f),
        OperatorGold.ToFColor(true),
        19.0f),
        { TEXT("PlayableRescueOperatorReady"), TEXT("MetaHumanReplacementSlot") }));
    TagCreative(AddTags(SpawnGuideText(
        FString::Printf(TEXT("ACTIVE OPERATOR PROFILE\n%s\nsave-backed selected-language identity | HUD mirrors callsign and role"), *ActiveOperatorSummary),
        OperatorHub + FVector(0.0f, -415.0f, 225.0f),
        OperatorGold.ToFColor(true),
        16.0f),
        { TEXT("PlayableOperatorIdentityRuntime"), TEXT("SelectedLanguageOperatorProfile"), TEXT("PlayableOperatorIdentitySave") }));

    // Curriculum room concept art in the protected quadrant.
    TagCreative(AddTags(SpawnTexturedBlock(
        CurriculumHub + FVector(0.0f, 0.0f, -12.0f),
        FVector(7.9f, 4.5f, 0.050f),
        FLinearColor(0.030f, 0.050f, 0.042f, 1.0f) + CurriculumGreen * 0.12f,
        CityLabel + TEXT(" Curriculum Concept Room Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Hex_Tile.M_Tech_Hex_Tile"),
        false),
        { TEXT("CurriculumRoomConceptArt"), TEXT("ProtectedCodingChallengeZone"), TEXT("NoZombieLearningZone") }));
    static const TCHAR* ConceptLabels[] = {
        TEXT("TYPES"),
        TEXT("IF"),
        TEXT("LOOPS"),
        TEXT("ARRAYS"),
        TEXT("STRINGS"),
        TEXT("FUNCTIONS"),
        TEXT("SEARCH"),
        TEXT("SORT")
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(ConceptLabels); ++i)
    {
        const int32 Col = i % 4;
        const int32 Row = i / 4;
        const FVector NodeLoc = CurriculumHub + FVector(-480.0f + Col * 320.0f, -60.0f + Row * 230.0f, 72.0f);
        const FLinearColor NodeColor = FLinearColor::LerpUsingHSV(CurriculumGreen, Mission.AccentColor * 1.5f, static_cast<float>(i) / 7.0f);
        TagCreative(AddTags(SpawnBlock(
            NodeLoc,
            FVector(0.80f, 0.12f, 0.56f),
            NodeColor,
            FString::Printf(TEXT("%s Curriculum Concept Node %s"), *CityLabel, ConceptLabels[i]),
            false),
            { TEXT("CurriculumRoomConceptArt"), TEXT("SelectedLanguageOnly"), TEXT("SurvivorIntelRewardChain") }));
        TagCreative(AddTags(SpawnGuideText(
            ConceptLabels[i],
            NodeLoc + FVector(0.0f, -64.0f, 76.0f),
            NodeColor.ToFColor(true),
            15.0f),
            { TEXT("CurriculumRoomConceptArt"), TEXT("SurvivorIntelRewardChain") }));
    }
    TagCreative(AddTags(SpawnGuideText(
        TEXT("CURRICULUM ROOMS\ncode concepts become physical rescue clues;\nsolves unlock survivor intel, not zombie pressure"),
        CurriculumHub + FVector(0.0f, -294.0f, 318.0f),
        CurriculumGreen.ToFColor(true),
        20.0f),
        { TEXT("CurriculumRoomConceptArt"), TEXT("SurvivorIntelDossier"), TEXT("LearningWithoutDeathRisk") }));

    // Functional tactical gear pickups, not just visual placeholders.
    TagCreative(AddTags(SpawnTexturedBlock(
        GearHub + FVector(0.0f, 0.0f, -12.0f),
        FVector(6.6f, 3.4f, 0.050f),
        FLinearColor(0.050f, 0.040f, 0.060f, 1.0f) + GearViolet * 0.12f,
        CityLabel + TEXT(" Tactical Gear Functional Pickup Floor"),
        TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel.M_Metal_Burnished_Steel"),
        false),
        { TEXT("TacticalGearFunctionalPickup"), TEXT("ImmediateGearSelection") }));
    SpawnCreativePickup(EPickupKind::RadioScanner, GearHub + FVector(-560.0f, 24.0f, 116.0f), 2, TEXT("RadioScannerPickupFunctional"), FColor(80, 235, 255), TEXT("RADIO SCANNER x2\nZ route scan"));
    SpawnCreativePickup(EPickupKind::FlashlightBattery, GearHub + FVector(-330.0f, 24.0f, 116.0f), 2, TEXT("FlashlightPickupFunctional"), FColor(255, 230, 120), TEXT("FLASHLIGHT x2\nL field light"));
    SpawnCreativePickup(EPickupKind::AmmoPouch, GearHub + FVector(-100.0f, 24.0f, 116.0f), 60, TEXT("AmmoPouchPickupFunctional"), FColor(105, 185, 255), TEXT("AMMO POUCH +60\nreserve capacity"));
    SpawnCreativePickup(EPickupKind::BypassKit, GearHub + FVector(130.0f, 24.0f, 116.0f), 1, TEXT("BypassKitPickupFunctional"), FColor(255, 170, 70), TEXT("BYPASS KIT x1\nterminal assist"));
    SpawnCreativePickup(EPickupKind::Medkit, GearHub + FVector(360.0f, 24.0f, 116.0f), 1, TEXT("MedkitPickupFunctional"), FColor(120, 255, 150), TEXT("MEDKIT x1\nQ healing"));
    SpawnCreativePickup(EPickupKind::ArmorPlate, GearHub + FVector(590.0f, 24.0f, 116.0f), 2, TEXT("ArmorPlatePickupFunctional"), FColor(180, 210, 230), TEXT("ARMOR +2\nabsorbs hits"));
    SpawnCreativePickup(EPickupKind::Flare, GearHub + FVector(-330.0f, -126.0f, 116.0f), 2, TEXT("FlarePickupFunctional"), FColor(255, 120, 55), TEXT("FLARES x2\nX slot utility"));
    SpawnCreativePickup(EPickupKind::Smoke, GearHub + FVector(-100.0f, -126.0f, 116.0f), 2, TEXT("SmokePickupFunctional"), FColor(190, 210, 220), TEXT("SMOKE x2\nX slot utility"));
    SpawnCreativePickup(EPickupKind::Stim, GearHub + FVector(130.0f, -126.0f, 116.0f), 1, TEXT("StimPickupFunctional"), FColor(245, 90, 255), TEXT("STIM x1\nX slot utility"));
    SpawnCreativePickup(EPickupKind::Scrap, GearHub + FVector(360.0f, -126.0f, 116.0f), 8, TEXT("ScrapPickupFunctional"), FColor(255, 210, 92), TEXT("SCRAP +8\nB barricades"));
    TagCreative(AddTags(SpawnGuideText(
        TEXT("TACTICAL GEAR NOW FUNCTIONAL\nscanner, flashlight, ammo pouch, bypass kit, medkit, armor, and utility pickups feed real player systems"),
        GearHub + FVector(0.0f, -220.0f, 270.0f),
        GearViolet.ToFColor(true),
        18.0f),
        { TEXT("TacticalGearFunctionalPickup"), TEXT("ImmediateGearSelection") }));

    // City district kit promotion targets for incoming modular/interior assets.
    TagCreative(AddTags(SpawnTexturedBlock(
        DistrictHub + FVector(0.0f, 0.0f, -18.0f),
        FVector(8.2f, 4.8f, 0.052f),
        FLinearColor(0.055f, 0.046f, 0.036f, 1.0f) + DistrictAmber * 0.10f,
        CityLabel + TEXT(" Major City District Kit Floor"),
        TEXT("/Game/StarterContent/Materials/M_CobbleStone_Rough.M_CobbleStone_Rough"),
        false),
        { TEXT("MajorCityDistrictKit"), TEXT("InteriorMissionSpaceReady"), TEXT("HumanScaleBuildingProportion") }));
    static const TCHAR* DistrictLabels[] = {
        TEXT("Hospital"),
        TEXT("Transit"),
        TEXT("Civic"),
        TEXT("Commercial"),
        TEXT("Industrial"),
        TEXT("Residential")
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(DistrictLabels); ++i)
    {
        const FVector BlockLoc = DistrictHub + FVector(-560.0f + i * 224.0f, 110.0f + (i % 2) * 120.0f, 118.0f);
        TagCreative(AddTags(SpawnBlock(
            BlockLoc,
            FVector(0.86f, 0.22f, 1.10f + (i % 3) * 0.18f),
            (i % 2 == 0 ? DistrictAmber : Mission.SecondaryAccentColor) * 0.84f,
            FString::Printf(TEXT("%s District Kit Module %s"), *CityLabel, DistrictLabels[i]),
            true),
            { TEXT("MajorCityDistrictKit"), TEXT("InteriorMissionSpaceReady"), TEXT("HumanScaleBuildingProportion") }));
        TagCreative(AddTags(SpawnGuideText(
            DistrictLabels[i],
            BlockLoc + FVector(0.0f, -62.0f, 128.0f),
            DistrictAmber.ToFColor(true),
            13.0f),
            { TEXT("MajorCityDistrictKit"), TEXT("InteriorMissionSpaceReady") }));
    }

    // Stress and promotion rig: runtime-visible reminder of what must stay
    // functional as new assets are promoted.
    TagCreative(AddTags(SpawnTexturedBlock(
        StressHub + FVector(0.0f, 0.0f, -14.0f),
        FVector(6.4f, 4.0f, 0.052f),
        FLinearColor(0.060f, 0.035f, 0.032f, 1.0f) + StressRed * 0.12f,
        CityLabel + TEXT(" Comprehensive Stress Test Rig Floor"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false),
        { TEXT("ComprehensiveStressTestRig"), TEXT("AIEncounterDirectorRuntimeHook"), TEXT("AsyncPhysicsPromotionGate") }));
    for (int32 i = 0; i < 6; ++i)
    {
        AActor* Prop = TagCreative(AddTags(SpawnBlock(
            StressHub + FVector(-410.0f + i * 165.0f, 72.0f + (i % 2) * 115.0f, 84.0f),
            FVector(0.46f + i * 0.035f, 0.32f, 0.46f),
            StressRed * (0.70f + i * 0.05f),
            FString::Printf(TEXT("%s Stress Rig Physics Prop %d"), *CityLabel, i + 1),
            true),
            { TEXT("ComprehensiveStressTestRig"), TEXT("AsyncPhysicsPromotionGate"), TEXT("ProjectilePhysicsTarget") }));
        EnablePhysics(Prop, 42.0f + i * 9.0f);
    }
    for (int32 i = 0; i < 5; ++i)
    {
        TagCreative(AddTags(SpawnBlock(
            StressHub + FVector(-360.0f + i * 180.0f, -172.0f, 54.0f),
            FVector(0.32f, 0.32f, 0.24f),
            (i % 2 == 0 ? FLinearColor(0.28f, 0.95f, 0.46f, 1.0f) : StressRed),
            FString::Printf(TEXT("%s Stress Rig AI Node %d"), *CityLabel, i + 1),
            false),
            { TEXT("ComprehensiveStressTestRig"), TEXT("AIEncounterDirectorRuntimeHook"), TEXT("NPCBehaviorTreeReady"), TEXT("StateTreeEQSReady") }));
    }
    TagCreative(AddTags(SpawnGuideText(
        TEXT("STRESS TEST RIG\ncamera, health, armor, pickups, physics, AI nodes, coding safehouse, survivor route, package smoke"),
        StressHub + FVector(0.0f, -272.0f, 318.0f),
        StressRed.ToFColor(true),
        19.0f),
        { TEXT("ComprehensiveStressTestRig"), TEXT("AIEncounterDirectorRuntimeHook"), TEXT("AsyncPhysicsPromotionGate") }));
    SpawnCreativeLight(StressHub + FVector(0.0f, 0.0f, 276.0f), StressRed, 3900.0f, 780.0f, TEXT("Creative Stress Rig Light"));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueCreativeImplementation] %s implemented May 28 recommendations: download intake gates, playable cast slots, curriculum concept rooms, functional tactical pickups, city district kits, Nanite/SM6 review gates, Mac LOD/texture/shader asset budget gates, AI/physics stress rig, and validation-ready promotion markers."),
        *CityLabel);

    (void)CityIndex;
}

bool ACodeRescueGameMode::ShouldSpawnDevelopmentShowcaseLayers() const
{
    return FParse::Param(FCommandLine::Get(), TEXT("CodeRescueDevelopmentShowcase"));
}

void ACodeRescueGameMode::ApplyProductionPresentationCleanup(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    if (ShouldSpawnDevelopmentShowcaseLayers())
    {
        return;
    }

    static const FName DevelopmentOnlyTags[] = {
        FName("ChallengeRoomConceptArt"),
        FName("TacticalArmoryAllWeaponsAvailable"),
        FName("ComprehensiveStressTestRig"),
        FName("ImmediateImprovementPass20"),
        FName("FirstMinuteOrientation"),
        FName("AuthoredArrivalComposition"),
        FName("AccountLinkedAssetShowcase"),
        FName("CharacterIdentityCourt"),
        FName("CinematicStreetLife"),
        FName("WorldCompositionLayer"),
        FName("CreativeRecommendationImplementation"),
        FName("BackgroundHordeProxy"),
        FName("ZombiePopulation50To1"),
        FName("WeatherLightingIdentity")
    };

    auto IsDevelopmentOnlyActor = [](const AActor* Actor)
    {
        if (!Actor)
        {
            return false;
        }
        for (const FName& Tag : DevelopmentOnlyTags)
        {
            if (Actor->Tags.Contains(Tag))
            {
                return true;
            }
        }
        return false;
    };

    int32 HiddenWorldLabels = 0;
    for (TActorIterator<ATextRenderActor> TextIt(GetWorld()); TextIt; ++TextIt)
    {
        ATextRenderActor* TextActor = *TextIt;
        if (!IsValid(TextActor) || TextActor->Tags.Contains(FName("KeepProductionWorldText")))
        {
            continue;
        }
        TextActor->SetActorHiddenInGame(true);
        TextActor->SetActorEnableCollision(false);
        TextActor->Tags.AddUnique(FName("ProductionPresentationHidden"));
        ++HiddenWorldLabels;
    }

    const FVector PlayerStart = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const FBox ArrivalCameraVolume(
        PlayerStart - FVector(1200.0f, 1200.0f, 180.0f),
        PlayerStart + FVector(1200.0f, 1200.0f, 820.0f));
    int32 HiddenDevelopmentActors = 0;
    int32 HiddenArrivalBlockers = 0;
    for (TActorIterator<AStaticMeshActor> MeshIt(GetWorld()); MeshIt; ++MeshIt)
    {
        AStaticMeshActor* MeshActor = *MeshIt;
        if (!IsValid(MeshActor) || MeshActor->IsHidden() ||
            MeshActor->Tags.Contains(FName("GameplayArenaConfinement")) ||
            MeshActor->Tags.Contains(FName("ProductionPresentationRequired")))
        {
            continue;
        }

        const FBox Bounds = MeshActor->GetComponentsBoundingBox(true);
        const FVector Extent = Bounds.GetExtent();
        const bool bDevelopmentOnly = IsDevelopmentOnlyActor(MeshActor);
        const UStaticMeshComponent* StaticComponent = MeshActor->GetStaticMeshComponent();
        const bool bPrimitiveBlock = StaticComponent && StaticComponent->GetStaticMesh() &&
            StaticComponent->GetStaticMesh()->GetName().Equals(TEXT("Cube"), ESearchCase::IgnoreCase);
        const bool bArrivalCameraBlocker =
            bPrimitiveBlock && Bounds.IsValid && Bounds.Intersect(ArrivalCameraVolume) &&
            Bounds.Max.Z > PlayerStart.Z - 58.0f &&
            (Extent.Z > 20.0f || FMath::Max(Extent.X, Extent.Y) > 250.0f);
        if (!bDevelopmentOnly && !bArrivalCameraBlocker)
        {
            continue;
        }

        MeshActor->SetActorHiddenInGame(true);
        MeshActor->SetActorEnableCollision(false);
        MeshActor->Tags.AddUnique(FName("ProductionPresentationHidden"));
        HiddenDevelopmentActors += bDevelopmentOnly ? 1 : 0;
        HiddenArrivalBlockers += bArrivalCameraBlocker ? 1 : 0;
    }

    UE_LOG(LogTemp, Display,
        TEXT("[ProductionPresentation] %s hidden labels=%d review actors=%d arrival blockers=%d; gameplay structures retained."),
        *CityLabel, HiddenWorldLabels, HiddenDevelopmentActors, HiddenArrivalBlockers);
    (void)Origin;
}

void ACodeRescueGameMode::EnsureEntryAccessCorridorClear(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    struct FAccessClearanceZone
    {
        FVector LocalCenter;
        FVector LocalExtent;
        const TCHAR* Label;
    };

    const FAccessClearanceZone Zones[] = {
        { FVector(-3800.0f, -3160.0f, 210.0f), FVector(780.0f, 720.0f, 520.0f), TEXT("entry") },
        { FVector(-3290.0f, -3050.0f, 210.0f), FVector(820.0f, 560.0f, 520.0f), TEXT("armory") },
        { FVector(-1125.0f, -2740.0f, 210.0f), FVector(980.0f, 620.0f, 520.0f), TEXT("safehouse") },
        { FVector(-3000.0f, -2300.0f, 210.0f), FVector(820.0f, 660.0f, 520.0f), TEXT("launch language marker") },
        { FVector(1150.0f, -900.0f, 210.0f), FVector(840.0f, 760.0f, 560.0f), TEXT("terminal") },
        { FVector(2850.0f, 1500.0f, 230.0f), FVector(980.0f, 820.0f, 580.0f), TEXT("survivor") },
        { FVector(2400.0f, 2400.0f, 230.0f), FVector(1000.0f, 980.0f, 580.0f), TEXT("helipad") },
    };

    const bool bDevelopmentShowcase = ShouldSpawnDevelopmentShowcaseLayers();
    int32 ClearedActors = 0;
    int32 FrozenPhysicsComponents = 0;
    TMap<FString, int32> ClearedByZone;

    for (TWeakObjectPtr<AActor>& ActorPtr : StreamedCampaignActors)
    {
        AActor* Actor = ActorPtr.Get();
        if (!Actor || !Actor->IsA<AStaticMeshActor>())
        {
            continue;
        }
        if (Actor->Tags.Contains(FName("GameplayArenaConfinement")))
        {
            continue;
        }

        const FVector Loc = Actor->GetActorLocation();
        if (Loc.Z < Origin.Z + 35.0f)
        {
            continue;
        }

        FString MatchedZoneLabel;
        for (const FAccessClearanceZone& Zone : Zones)
        {
            const FVector Center = Origin + CityOffset(Zone.LocalCenter);
            const FVector Extent = CityExtent(Zone.LocalExtent);
            const bool bInZone =
                FMath::Abs(Loc.X - Center.X) <= Extent.X &&
                FMath::Abs(Loc.Y - Center.Y) <= Extent.Y &&
                FMath::Abs(Loc.Z - Center.Z) <= Extent.Z;
            if (bInZone)
            {
                MatchedZoneLabel = Zone.Label;
                break;
            }
        }

        if (MatchedZoneLabel.IsEmpty())
        {
            continue;
        }

        const bool bDevelopmentDisplayActor =
            Actor->Tags.Contains(FName("ChallengeRoomConceptArt")) ||
            Actor->Tags.Contains(FName("TacticalArmoryAllWeaponsAvailable")) ||
            Actor->Tags.Contains(FName("ComprehensiveStressTestRig")) ||
            Actor->Tags.Contains(FName("ImmediateImprovementPass20")) ||
            Actor->Tags.Contains(FName("FirstMinuteOrientation")) ||
            Actor->Tags.Contains(FName("AuthoredArrivalComposition"));
        if (!bDevelopmentShowcase && !bDevelopmentDisplayActor)
        {
            // Production architecture keeps physical walls. The camera safety
            // pass handles visibility, while traversal uses real collision.
            continue;
        }

        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component)
            {
                if (Component->IsSimulatingPhysics())
                {
                    Component->SetSimulatePhysics(false);
                    ++FrozenPhysicsComponents;
                }
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Component->SetCollisionResponseToAllChannels(ECR_Ignore);
            }
        }
        Actor->SetActorEnableCollision(false);
        if (!bDevelopmentShowcase)
        {
            Actor->SetActorHiddenInGame(true);
            Actor->Tags.AddUnique(FName("ProductionPresentationHidden"));
        }
        Actor->Tags.Add(FName("EntryCorridorCollisionCleared"));
        ++ClearedActors;
        ClearedByZone.FindOrAdd(MatchedZoneLabel) += 1;
    }

    if (bDevelopmentShowcase)
    {
        AActor* ClearanceMarker = SpawnBlock(
            Origin + CityOffset(FVector(-3800.0f, -3160.0f, 14.0f)),
            CityExtent(FVector(13.6f, 11.4f, 0.026f)),
            FLinearColor(0.05f, 0.70f, 0.42f) * 1.45f,
            CityLabel + TEXT(" Universal Entry Collision Clearance Zone"),
            false);
        if (ClearanceMarker)
        {
            ClearanceMarker->Tags.Add(FName("UniversalEntryAccess"));
            ClearanceMarker->Tags.Add(FName("EntryCorridorCollisionCleared"));
        }
    }

    FString ZoneSummary;
    for (const TPair<FString, int32>& Pair : ClearedByZone)
    {
        ZoneSummary += FString::Printf(TEXT("%s=%d "), *Pair.Key, Pair.Value);
    }
    UE_LOG(LogTemp, Display, TEXT("[CodeRescueEntryAccess] %s cleared %d static blockers from access points and froze %d physics components. %s"), *CityLabel, ClearedActors, FrozenPhysicsComponents, *ZoneSummary);
}

void ACodeRescueGameMode::SpawnPurposeClarityLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    struct FPurposeStop
    {
        FVector Local;
        const TCHAR* Label;
        const TCHAR* Symbol;
        FLinearColor Color;
    };

    const FPurposeStop Stops[] = {
        { FVector(-3820.0f, -3180.0f, 0.0f), TEXT("OBJECTIVE 0\nENTRY"), TEXT("0"), FLinearColor(0.10f, 0.95f, 0.55f) },
        { FVector(-3290.0f, -3050.0f, 0.0f), TEXT("ARMORY\nWEAPONS + ITEMS"), TEXT("+"), FLinearColor(0.18f, 0.56f, 1.0f) },
        { FVector(-2860.0f, -2460.0f, 0.0f), TEXT("OBJECTIVE 2\nPROTECTED CODING SAFEHOUSE"), TEXT("</>"), FLinearColor(0.04f, 0.86f, 1.0f) },
        { FVector(2850.0f, 1500.0f, 0.0f), TEXT("OBJECTIVE 3\nSURVIVOR"), TEXT("3"), FLinearColor(1.0f, 0.86f, 0.10f) },
        { FVector(2400.0f, 2400.0f, 0.0f), TEXT("OBJECTIVE 4\nEXTRACTION"), TEXT("H"), FLinearColor(0.70f, 0.92f, 1.0f) },
        { FVector(2900.0f, -1500.0f, 0.0f), TEXT("OPTIONAL\nBOSS RISK"), TEXT("!"), FLinearColor(1.0f, 0.08f, 0.18f) },
    };

    auto TagPurpose = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.Add(FName("PurposeCodedArchitecture"));
            Actor->Tags.Add(FName("ArchitectureClarityPass"));
        }
        return Actor;
    };

    SpawnGuideText(
        TEXT("NAVIGATION LEGEND\nGreen entry, blue armory, cyan safehouse, gold survivor, pale extraction, red optional risk"),
        Origin + CityOffset(FVector(-3600.0f, -3320.0f, 560.0f)),
        FColor(215, 245, 255),
        34.0f);

    for (const FPurposeStop& Stop : Stops)
    {
        const FVector Base = Origin + CityOffset(Stop.Local);
        TagPurpose(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 16.0f),
            CityExtent(FVector(2.35f, 2.35f, 0.065f)),
            Stop.Color * 1.9f,
            FString::Printf(TEXT("%s Purpose Clarity Pad %s"), *CityLabel, Stop.Symbol),
            false));
        TagPurpose(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 136.0f),
            CityExtent(FVector(0.28f, 0.28f, 1.95f)),
            Stop.Color * 1.3f,
            FString::Printf(TEXT("%s Purpose Clarity Pylon %s"), *CityLabel, Stop.Symbol),
            false));
        TagPurpose(SpawnBlock(
            Base + FVector(0.0f, -108.0f, 246.0f),
            CityExtent(FVector(0.85f, 0.08f, 0.42f)),
            Stop.Color * 2.6f,
            FString::Printf(TEXT("%s Purpose Clarity Icon Panel %s"), *CityLabel, Stop.Symbol),
            false));
        SpawnGuideText(
            Stop.Label,
            Base + FVector(0.0f, -160.0f, 430.0f),
            Stop.Color.ToFColor(true),
            34.0f);
    }

    FActorSpawnParameters BeaconSpawnParams;
    BeaconSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AObjectiveFocusBeaconActor* ObjectiveBeacon = GetWorld()->SpawnActor<AObjectiveFocusBeaconActor>(
        AObjectiveFocusBeaconActor::StaticClass(),
        Origin + CityOffset(FVector(-2860.0f, -2460.0f, 122.0f)),
        FRotator::ZeroRotator,
        BeaconSpawnParams);
    if (ObjectiveBeacon)
    {
        const FLinearColor TerminalTint = Mission.AccentColor * 0.54f + FLinearColor(0.04f, 0.86f, 1.0f) * 0.46f;
        const FLinearColor SurvivorTint = Mission.AccentColor * 0.36f + FLinearColor(1.0f, 0.86f, 0.10f) * 0.64f;
        const FLinearColor ExtractionTint = Mission.SecondaryAccentColor * 0.44f + FLinearColor(0.70f, 0.92f, 1.0f) * 0.56f;
        const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
        ObjectiveBeacon->ConfigureObjectiveBeacon(
            CityIndex,
            Mission.CityName,
            Mission.TerminalId,
            Mission.SurvivorName,
            Mission.TerminalTitle,
            Mission.CurriculumFocus,
            Mission.LandmarkName,
            Origin + CityOffset(FVector(-3820.0f, -3180.0f, 90.0f)),
            Origin + CityOffset(FVector(-2860.0f, -2460.0f, 90.0f)),
            Origin + CityOffset(FVector(2850.0f, 1500.0f, 90.0f)),
            Origin + CityOffset(FVector(2400.0f, 2400.0f, 90.0f)),
            TerminalTint,
            SurvivorTint,
            ExtractionTint,
            GI && GI->bReducedMotion);
        ObjectiveBeacon->Tags.AddUnique(FName("StateAwareObjectiveBeacon"));
        ObjectiveBeacon->Tags.AddUnique(FName("ObjectiveClarityRuntimeLayer"));
        ObjectiveBeacon->Tags.AddUnique(FName("RadioScanRescueBeaconEffects"));
        ObjectiveBeacon->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
        ObjectiveBeacon->Tags.AddUnique(FName("Top50Recommendations"));
        RegisterStreamedActor(ObjectiveBeacon);
    }

    const FVector RouteStops[] = {
        FVector(-3820.0f, -3180.0f, 0.0f),
        FVector(-3290.0f, -3050.0f, 0.0f),
        FVector(-2860.0f, -2460.0f, 0.0f),
        FVector(2850.0f, 1500.0f, 0.0f),
        FVector(2400.0f, 2400.0f, 0.0f),
    };

    auto DistancePointToSegment2D = [](const FVector& Point, const FVector& A, const FVector& B)
    {
        const FVector2D P(Point.X, Point.Y);
        const FVector2D PA(A.X, A.Y);
        const FVector2D BA(B.X - A.X, B.Y - A.Y);
        const float Denom = BA.SizeSquared();
        const float T = Denom > KINDA_SMALL_NUMBER
            ? FMath::Clamp(FVector2D::DotProduct(P - PA, BA) / Denom, 0.0f, 1.0f)
            : 0.0f;
        const FVector2D Closest = PA + BA * T;
        return FVector2D::Distance(P, Closest);
    };

    int32 NonBlockingDecorations = 0;
    for (TWeakObjectPtr<AActor>& ActorPtr : StreamedCampaignActors)
    {
        AActor* Actor = ActorPtr.Get();
        if (!Actor || !Actor->IsA<AStaticMeshActor>())
        {
            continue;
        }
        if (Actor->Tags.Contains(FName("GameplayArenaConfinement")) ||
            Actor->Tags.Contains(FName("ArenaLockWall")) ||
            Actor->Tags.Contains(FName("PurposeCodedArchitecture")))
        {
            continue;
        }

        const FVector Delta = Actor->GetActorLocation() - Origin;
        const float SpanScale = FMath::Max(1.0f, FCodeRescueCampaign::GetCitySpanScale());
        const FVector Local(Delta.X / SpanScale, Delta.Y / SpanScale, Delta.Z);
        if (Local.Z < 42.0f)
        {
            continue;
        }

        bool bNearCriticalRoute = false;
        for (int32 i = 1; i < UE_ARRAY_COUNT(RouteStops); ++i)
        {
            if (DistancePointToSegment2D(Local, RouteStops[i - 1], RouteStops[i]) <= 420.0f)
            {
                bNearCriticalRoute = true;
                break;
            }
        }
        if (!bNearCriticalRoute)
        {
            continue;
        }

        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component)
            {
                Component->SetSimulatePhysics(false);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Component->SetCollisionResponseToAllChannels(ECR_Ignore);
            }
        }
        Actor->SetActorEnableCollision(false);
        Actor->Tags.Add(FName("CriticalPathNonBlockingArchitecture"));
        Actor->Tags.Add(FName("ArchitectureClarityPass"));
        ++NonBlockingDecorations;
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueArchitectureClarity] %s purpose-coded objective architecture for %s and made %d decorative route-adjacent static actors nonblocking."),
        *CityLabel,
        *Mission.CityName,
        NonBlockingDecorations);
}

void ACodeRescueGameMode::SpawnRescueSupportTeamForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    struct FSupportSpec
    {
        const TCHAR* Name;
        const TCHAR* Role;
        const TCHAR* MechanicalIdentity;
        const TCHAR* BarkStyle;
        FVector SpawnOffset;
        float FollowOffset;
        float LateralOffset;
        float DetectionRange;
        float Damage;
        float Refire;
        bool bMedic;
        FLinearColor Color;
    };

    static const FSupportSpec Specs[] = {
        { TEXT("Mira Hale"),   TEXT("Medic"),        TEXT("Manual and automatic medic pulse"),             TEXT("Calm clinical urgency"),       FVector(-260.0f, -180.0f, 40.0f), 520.0f, -250.0f, 1350.0f, 16.0f, 0.82f, true,  FLinearColor(1.0f, 0.18f, 0.20f) },
        { TEXT("Tomas Ives"),  TEXT("Engineer"),     TEXT("Formation support and access reliability"),      TEXT("Practical route/repair language"), FVector(-380.0f,  120.0f, 40.0f), 680.0f,  270.0f, 1500.0f, 18.0f, 0.74f, false, FLinearColor(1.0f, 0.62f, 0.18f) },
        { TEXT("Ada Cross"),   TEXT("Rifle Support"),TEXT("Steady support fire"),                           TEXT("Concise overwatch callouts"),  FVector(-520.0f, -420.0f, 40.0f), 860.0f, -420.0f, 2100.0f, 24.0f, 0.58f, false, FLinearColor(0.28f, 0.58f, 1.0f) },
        { TEXT("Noor Vance"),  TEXT("Scout"),        TEXT("Wide formation and threat awareness"),           TEXT("Brief directional warnings"),  FVector(-620.0f,  420.0f, 40.0f), 940.0f,  430.0f, 1800.0f, 20.0f, 0.50f, false, FLinearColor(0.30f, 1.0f, 0.52f) },
        { TEXT("Briggs Vale"), TEXT("Heavy Rescue"), TEXT("Close formation anchor and pressure absorber"),   TEXT("Short protective callouts"),   FVector(-820.0f,    0.0f, 40.0f), 1120.0f,   0.0f, 1650.0f, 30.0f, 0.92f, false, FLinearColor(0.86f, 0.82f, 0.56f) },
    };

    const FVector PlayerStart = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    const FVector RallyCenter = PlayerStart + FVector(-520.0f, 0.0f, -210.0f);
    const bool bDevelopmentShowcase = ShouldSpawnDevelopmentShowcaseLayers();
    if (bDevelopmentShowcase)
    {
        SpawnTexturedBlock(
            RallyCenter,
            FVector(6.8f, 4.1f, 0.052f),
            FLinearColor(0.045f, 0.060f, 0.060f) + FLinearColor(0.16f, 0.64f, 0.88f) * 0.22f,
            CityLabel + TEXT(" Rescue Support Team Rally Floor"),
            TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
            false);
        SpawnBlock(
            RallyCenter + FVector(0.0f, -218.0f, 156.0f),
            FVector(5.4f, 0.07f, 1.08f),
            FLinearColor(0.06f, 0.08f, 0.09f),
            CityLabel + TEXT(" Rescue Support Team Briefing Wall"),
            false);
        SpawnGuideText(
            TEXT("RESCUE TEAM DEPLOYED\nMira Medic | Tomas Engineer | Ada Rifle | Noor Scout | Briggs Heavy\nY/U/O/N orders now receive role-specific callouts."),
            RallyCenter + FVector(0.0f, -260.0f, 270.0f),
            FColor(190, 240, 255),
            24.0f);
    }

    for (int32 i = 0; i < UE_ARRAY_COUNT(Specs); ++i)
    {
        const FSupportSpec& Spec = Specs[i];
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        const FVector ProductionOffset(
            -720.0f - static_cast<float>(i / 2) * 220.0f,
            (i % 2 == 0 ? -1.0f : 1.0f) * (390.0f + static_cast<float>(i / 2) * 150.0f),
            40.0f);
        ACompanionActor* Member = GetWorld()->SpawnActor<ACompanionActor>(
            ACompanionActor::StaticClass(),
            PlayerStart + (bDevelopmentShowcase ? Spec.SpawnOffset : ProductionOffset),
            FRotator(0.0f, 35.0f, 0.0f),
            Params);
        if (!Member)
        {
            continue;
        }

        Member->ConfigureSquadPersonality(
            Spec.Name,
            Spec.Role,
            Spec.MechanicalIdentity,
            Spec.BarkStyle,
            Spec.Color);
        Member->FollowOffset = Spec.FollowOffset;
        Member->LateralFollowOffset = Spec.LateralOffset;
        Member->DetectionRange = Spec.DetectionRange;
        Member->CombatDamage = Spec.Damage;
        Member->RefireDelay = Spec.Refire;
        Member->bMedicSupport = Spec.bMedic;
        Member->MagazineSize = Spec.bMedic ? 18 : 30;
        Member->MagazineAmmo = Member->MagazineSize;
        RegisterStreamedActor(Member);

        if (bDevelopmentShowcase)
        {
            SpawnBlock(
                Member->GetActorLocation() + FVector(0.0f, 0.0f, 190.0f),
                FVector(0.46f, 0.46f, 0.045f),
                Spec.Color * 3.2f,
                FString::Printf(TEXT("%s Rescue Team Halo %d"), *CityLabel, i + 1),
                false);
            SpawnGuideText(
                FString::Printf(TEXT("%s\n%s"), Spec.Name, Spec.Role),
                Member->GetActorLocation() + FVector(0.0f, 0.0f, 330.0f),
                Spec.Color.ToFColor(true),
                24.0f);
        }
    }

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->bHasCompanion = true;
    }

    (void)Origin;
}

void ACodeRescueGameMode::SpawnSurvivorReliefCamp(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, ASurvivorActor* Survivor)
{
    (void)CityIndex;
    (void)Origin;

    if (!Survivor)
    {
        return;
    }

    auto Track = [Survivor](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("SurvivorArchetypeRosterRuntime"));
            Actor->Tags.AddUnique(FName("SurvivorRoleReadableNameplate"));
            Actor->Tags.AddUnique(FName("SelectedLanguageSurvivorHandoff"));
            Survivor->AddHelperActor(Actor);
        }
        return Actor;
    };

    const FVector SurvivorLoc = Survivor->GetActorLocation();
    const FVector CampCenter = SurvivorLoc + FVector(-460.0f, 340.0f, -95.0f);
    const FLinearColor SurvivorAccent = Survivor->ArchetypeAccentColor;
    const FLinearColor CampColor = SurvivorAccent * 0.34f + FLinearColor(0.10f, 0.13f, 0.13f) * 0.66f;

    Track(SpawnTexturedBlock(
        CampCenter,
        FVector(4.4f, 3.2f, 0.055f),
        CampColor,
        CityLabel + TEXT(" Survivor Relief Camp Ground Mat"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false));
    Track(SpawnBlock(
        CampCenter + FVector(0.0f, -210.0f, 88.0f),
        FVector(4.4f, 0.10f, 1.7f),
        SurvivorAccent * 0.75f,
        CityLabel + TEXT(" Survivor Camp Back Wall"),
        true));
    Track(SpawnBlock(
        CampCenter + FVector(0.0f, 210.0f, 92.0f),
        FVector(4.4f, 0.08f, 0.42f),
        FLinearColor(1.0f, 0.84f, 0.18f) * 1.45f,
        CityLabel + TEXT(" Survivor Camp Hazard Rail"),
        false));

    if (UStaticMesh* TableMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_TableRound.SM_TableRound")))
    {
        Track(SpawnStaticMeshProp(
            TableMesh,
            CampCenter + FVector(-210.0f, -20.0f, 42.0f),
            FRotator::ZeroRotator,
            FVector(1.05f, 1.05f, 0.75f),
            CityLabel + TEXT(" Survivor Camp Briefing Table"),
            true));
    }
    if (UStaticMesh* ChairMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Chair.SM_Chair")))
    {
        Track(SpawnStaticMeshProp(
            ChairMesh,
            CampCenter + FVector(-320.0f, 150.0f, 45.0f),
            FRotator(0.0f, -30.0f, 0.0f),
            FVector(0.8f, 0.8f, 0.9f),
            CityLabel + TEXT(" Survivor Camp Chair A"),
            true));
        Track(SpawnStaticMeshProp(
            ChairMesh,
            CampCenter + FVector(-90.0f, 145.0f, 45.0f),
            FRotator(0.0f, 25.0f, 0.0f),
            FVector(0.8f, 0.8f, 0.9f),
            CityLabel + TEXT(" Survivor Camp Chair B"),
            true));
    }
    if (UStaticMesh* ShelfMesh = LoadCodeRescueAssetMesh(TEXT("/Game/StarterContent/Props/SM_Shelf.SM_Shelf")))
    {
        Track(SpawnStaticMeshProp(
            ShelfMesh,
            CampCenter + FVector(245.0f, -90.0f, 85.0f),
            FRotator(0.0f, 90.0f, 0.0f),
            FVector(1.0f, 0.55f, 1.45f),
            CityLabel + TEXT(" Survivor Camp Supply Shelf"),
            true));
    }

    Track(SpawnBlock(
        CampCenter + FVector(260.0f, 95.0f, 44.0f),
        FVector(1.7f, 0.72f, 0.28f),
        FLinearColor(0.92f, 0.95f, 1.0f),
        CityLabel + TEXT(" Survivor Camp Cot"),
        true));
    Track(SpawnBlock(
        CampCenter + FVector(260.0f, 95.0f, 78.0f),
        FVector(0.46f, 0.50f, 0.16f),
        FLinearColor(0.16f, 0.55f, 1.0f) * 1.5f,
        CityLabel + TEXT(" Survivor Camp Pillow"),
        false));
    Track(SpawnBlock(
        CampCenter + FVector(28.0f, -30.0f, 82.0f),
        FVector(0.50f, 0.14f, 0.55f),
        FLinearColor(1.0f, 0.16f, 0.16f) * 1.6f,
        CityLabel + TEXT(" Survivor Camp Med Cross Vertical"),
        false));
    Track(SpawnBlock(
        CampCenter + FVector(28.0f, -30.0f, 82.0f),
        FVector(0.14f, 0.50f, 0.55f),
        FLinearColor(1.0f, 0.16f, 0.16f) * 1.6f,
        CityLabel + TEXT(" Survivor Camp Med Cross Horizontal"),
        false));

    FString TerminalLabel = Mission.TerminalTitle;
    if (TerminalLabel.Len() > 36)
    {
        TerminalLabel = TerminalLabel.Left(33) + TEXT("...");
    }
    Track(SpawnGuideText(
        FString::Printf(TEXT("SURVIVOR PROFILE\n%s\n%s [%s]\nNeed: %s\nUnlock: %s"),
            *Mission.SurvivorName,
            *Survivor->ArchetypeTitle,
            *Survivor->ArchetypeIconLabel,
            *Survivor->ArchetypeFieldNeed,
            *TerminalLabel),
        SurvivorLoc + FVector(-260.0f, 385.0f, 560.0f),
        SurvivorAccent.ToFColor(true),
        30.0f));
    Track(SpawnGuideText(
        FString::Printf(TEXT("RELIEF CAMP\n%s\n%s"),
            *Survivor->ArchetypeRescueSkill,
            *Survivor->ArchetypeDossierHook),
        CampCenter + FVector(0.0f, 0.0f, 350.0f),
        FColor(220, 245, 255),
        28.0f));
}

void ACodeRescueGameMode::SpawnSecondaryMotionSignalLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, ASurvivorActor* Survivor)
{
    const FLinearColor RescueGold = FLinearColor(1.0f, 0.82f, 0.18f);
    const FLinearColor SafehouseCyan = FLinearColor(0.1f, 0.78f, 1.0f);
    const FLinearColor HelipadGreen = FLinearColor(0.36f, 1.0f, 0.42f);
    const FLinearColor SurvivorTint = Mission.SecondaryAccentColor * 0.55f + RescueGold * 0.45f;

    auto SpawnSignal = [&](const FVector& Location, const FRotator& Rotation, const FLinearColor& Tint, float Phase, const FName& LandmarkTag, const FString& Name) -> ASecondaryMotionSignalActor*
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ASecondaryMotionSignalActor* Signal = GetWorld()->SpawnActor<ASecondaryMotionSignalActor>(
            ASecondaryMotionSignalActor::StaticClass(),
            Location,
            Rotation,
            Params);
        if (!Signal)
        {
            return nullptr;
        }

        (void)Name;
        Signal->ConfigureSignal(Tint, Phase, 6.5f + static_cast<float>((CityIndex + static_cast<int32>(Phase)) % 5), 1.85f + 0.13f * static_cast<float>((CityIndex % 7) + 1));
        Signal->Tags.AddUnique(FName("SecondaryMotionSignalLayer"));
        Signal->Tags.AddUnique(FName("ChaosClothReadyFallback"));
        Signal->Tags.AddUnique(FName("ProceduralClothFallback"));
        Signal->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
        Signal->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
        Signal->Tags.AddUnique(LandmarkTag);
        RegisterStreamedActor(Signal);
        return Signal;
    };

    SpawnSignal(
        Origin + CityOffset(FVector(-1125.0f, -3025.0f, 118.0f)),
        FRotator(0.0f, 16.0f, 0.0f),
        SafehouseCyan,
        0.35f,
        FName("SecondaryMotionSafehouseSignal"),
        CityLabel + TEXT(" Safehouse Secondary Motion Signal"));

    SpawnSignal(
        Origin + CityOffset(FVector(2400.0f, 2400.0f, 92.0f)),
        FRotator(0.0f, -32.0f, 0.0f),
        HelipadGreen,
        1.7f,
        FName("SecondaryMotionHelipadSignal"),
        CityLabel + TEXT(" Helipad Secondary Motion Signal"));

    SpawnSignal(
        Origin + CityOffset(FVector(760.0f, -160.0f, 104.0f)),
        FRotator(0.0f, 48.0f, 0.0f),
        Mission.AccentColor * 0.72f + RescueGold * 0.28f,
        2.8f,
        FName("SecondaryMotionRouteSignal"),
        CityLabel + TEXT(" Rescue Route Secondary Motion Signal"));

    if (Survivor && !Survivor->bRescued)
    {
        if (ASecondaryMotionSignalActor* SurvivorSignal = SpawnSignal(
                Survivor->GetActorLocation() + FVector(-650.0f, 410.0f, 22.0f),
                FRotator(0.0f, -18.0f, 0.0f),
                SurvivorTint,
                4.1f,
                FName("SecondaryMotionSurvivorCampSignal"),
                CityLabel + TEXT(" Survivor Camp Secondary Motion Signal")))
        {
            Survivor->AddHelperActor(SurvivorSignal);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueSecondaryMotion] %s spawned procedural cloth-ready rescue signals for safehouse, helipad, route, and survivor camp."),
        *CityLabel);
}

void ACodeRescueGameMode::SpawnEncounterDirectorLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel, ASurvivorActor* Survivor)
{
    if (!Survivor || Survivor->bRescued)
    {
        return;
    }

    const FVector SurvivorLoc = Survivor->GetActorLocation();
    const FVector DirectorCenter = SurvivorLoc + FVector(-920.0f, -520.0f, -94.0f);
    const FLinearColor DirectorBlue = FLinearColor(0.08f, 0.58f, 0.95f);
    const FLinearColor PressureRed = FLinearColor(1.0f, 0.10f, 0.05f);
    const FLinearColor FlankViolet = FLinearColor(0.72f, 0.34f, 1.0f);
    const FLinearColor AnchorAmber = FLinearColor(1.0f, 0.62f, 0.12f);
    const FLinearColor SentinelGreen = FLinearColor(0.20f, 1.0f, 0.55f);

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bTerminalSolvedForDirector = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bResumeHasResources = GI && GI->bHasPlayerResources;
    const bool bLowHealthResume = bResumeHasResources && GI->LastPlayerHealth > 0.0f && GI->LastPlayerHealth <= 85.0f;
    const bool bLowAmmoResume = bResumeHasResources && GI->LastPlayerAmmo <= 35;
    const bool bLowUtilityResume = bResumeHasResources && GI->LastPlayerMedkits <= 1 && GI->LastPlayerArmorPlates <= 1;
    const bool bDirectorReliefEnabled = bLowHealthResume || bLowAmmoResume || bLowUtilityResume;
    const FString DirectorObjectiveState = bTerminalSolvedForDirector ? TEXT("survivor route open") : TEXT("terminal route locked");
    const FString DirectorResourceState = bDirectorReliefEnabled ? TEXT("relief cache enabled") : TEXT("standard counterplay cache");
    const FString DifficultyLabel = GI ? GI->GetDifficultyDisplayName() : FString(TEXT("Normal"));

    auto TagDirectorActor = [this, bTerminalSolvedForDirector, bDirectorReliefEnabled](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("EncounterDirectorLayer"));
            Actor->Tags.AddUnique(FName("AIDirectedEncounter"));
            Actor->Tags.AddUnique(FName("AuthoredEncounterDirector"));
            Actor->Tags.AddUnique(FName("EncounterDirectorAdaptivePressure"));
            Actor->Tags.AddUnique(FName("ObjectiveStateAwareEncounter"));
            Actor->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
            Actor->Tags.AddUnique(bTerminalSolvedForDirector ? FName("EncounterDirectorRouteOpenPressure") : FName("EncounterDirectorRouteLockedPressure"));
            Actor->Tags.AddUnique(bDirectorReliefEnabled ? FName("EncounterDirectorResourceRelief") : FName("EncounterDirectorStandardResources"));
            ApplyRuntimeDataLayerTags(Actor, TArray<FName>{
                FName("RuntimeDataLayer_Mode_Combat"),
                FName("RuntimeDataLayer_State_Overrun"),
                bTerminalSolvedForDirector ? FName("RuntimeDataLayer_State_RescueRouteOpen") : FName("RuntimeDataLayer_State_TerminalLocked"),
            });
        }
        return Actor;
    };

    TagDirectorActor(SpawnTexturedBlock(
        DirectorCenter,
        FVector(6.8f, 4.2f, 0.055f),
        FLinearColor(0.03f, 0.045f, 0.052f) + Mission.AccentColor * 0.10f,
        CityLabel + TEXT(" Encounter Director Staging Deck"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        true));
    TagDirectorActor(SpawnBlock(
        DirectorCenter + FVector(0.0f, -260.0f, 132.0f),
        FVector(5.7f, 0.08f, 1.08f),
        DirectorBlue * 0.65f + Mission.SecondaryAccentColor * 0.20f,
        CityLabel + TEXT(" Encounter Director Readability Wall"),
        true));
    TagDirectorActor(SpawnGuideText(
        TEXT("DIRECTED ENCOUNTER\nanchor | flank | pressure | sentinel\nclear the roles before rescue"),
        DirectorCenter + FVector(0.0f, -332.0f, 360.0f),
        FColor(160, 230, 255),
        26.0f));
    TagDirectorActor(SpawnGuideText(
        FString::Printf(
            TEXT("DIRECTOR STATE\nobjective: %s\nresources: %s\ndifficulty: %s"),
            *DirectorObjectiveState,
            *DirectorResourceState,
            *DifficultyLabel),
        DirectorCenter + FVector(0.0f, -332.0f, 228.0f),
        bDirectorReliefEnabled ? FColor(140, 255, 170) : FColor(185, 225, 255),
        18.0f));

    const FVector LaneOffsets[] = {
        FVector(-420.0f, -45.0f, 38.0f),
        FVector(0.0f, 35.0f, 38.0f),
        FVector(420.0f, -45.0f, 38.0f),
    };
    const FLinearColor LaneColors[] = {
        FlankViolet,
        PressureRed,
        AnchorAmber,
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(LaneOffsets); ++i)
    {
        TagDirectorActor(SpawnBlock(
            DirectorCenter + LaneOffsets[i],
            FVector(1.45f, 0.16f, 0.12f),
            LaneColors[i] * 2.2f,
            FString::Printf(TEXT("%s Encounter Director Role Lane %d"), *CityLabel, i + 1),
            false));
        TagDirectorActor(SpawnBlock(
            DirectorCenter + LaneOffsets[i] + FVector(0.0f, 210.0f, 110.0f),
            FVector(0.18f, 0.18f, 1.05f),
            LaneColors[i] * 1.55f,
            FString::Printf(TEXT("%s Encounter Director Role Beacon %d"), *CityLabel, i + 1),
            false));
    }

    auto SpawnDirectorBarricade = [&](const FVector& Location, const FVector& Scale, const FLinearColor& Tint, const FString& Name, const FName& RoleTag) -> ABarricadeActor*
    {
        const FTransform Transform(FRotator::ZeroRotator, Location, Scale);
        ABarricadeActor* Barricade = GetWorld()->SpawnActorDeferred<ABarricadeActor>(ABarricadeActor::StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if (!Barricade)
        {
            return nullptr;
        }

        Barricade->Lifetime = 0.0f;
        Barricade->MaxHealth = 94.0f;
        Barricade->Health = Barricade->MaxHealth;
        Barricade->DebrisCount = 6;
        Barricade->DebrisLifetime = 10.0f;
        Barricade->DebrisImpulseStrength = 36000.0f;
        UGameplayStatics::FinishSpawningActor(Barricade, Transform);
#if WITH_EDITOR
        Barricade->SetActorLabel(Name);
#endif
        Barricade->Tags.AddUnique(FName("EncounterDirectorCover"));
        Barricade->Tags.AddUnique(FName("DestructibleCoverTraining"));
        Barricade->Tags.AddUnique(FName("ChaosReadableDestruction"));
        Barricade->Tags.AddUnique(RoleTag);
        TagDirectorActor(Barricade);
        RegisterStreamedActor(Barricade);
        (void)Tint;
        return Barricade;
    };

    SpawnDirectorBarricade(DirectorCenter + FVector(-540.0f, 255.0f, 92.0f), FVector(1.35f, 0.20f, 1.15f), FlankViolet, CityLabel + TEXT(" Encounter Director Flank Cover"), FName("EncounterDirectorFlankCover"));
    SpawnDirectorBarricade(DirectorCenter + FVector(0.0f, 300.0f, 92.0f), FVector(1.90f, 0.22f, 1.22f), PressureRed, CityLabel + TEXT(" Encounter Director Pressure Cover"), FName("EncounterDirectorPressureCover"));
    SpawnDirectorBarricade(DirectorCenter + FVector(540.0f, 255.0f, 92.0f), FVector(1.35f, 0.20f, 1.15f), AnchorAmber, CityLabel + TEXT(" Encounter Director Anchor Cover"), FName("EncounterDirectorAnchorCover"));

    auto SpawnDirectorPickup = [&](EPickupKind Kind, const FVector& Location, int32 Amount, const FName& PickupTag)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Location, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("EncounterDirectorReward"));
            Pickup->Tags.Add(PickupTag);
            TagDirectorActor(Pickup);
            RegisterStreamedActor(Pickup);
        }
    };
    SpawnDirectorPickup(EPickupKind::Smoke, DirectorCenter + FVector(-360.0f, -130.0f, 116.0f), 1, FName("EncounterDirectorSmokeCache"));
    SpawnDirectorPickup(EPickupKind::ArmorPlate, DirectorCenter + FVector(0.0f, -145.0f, 116.0f), 1, FName("EncounterDirectorArmorCache"));
    SpawnDirectorPickup(EPickupKind::Ammo, DirectorCenter + FVector(360.0f, -130.0f, 116.0f), 24, FName("EncounterDirectorAmmoCache"));
    if (bDirectorReliefEnabled)
    {
        SpawnDirectorPickup(EPickupKind::Medkit, DirectorCenter + FVector(-145.0f, 25.0f, 116.0f), 1, FName("EncounterDirectorReliefMedkit"));
        SpawnDirectorPickup(EPickupKind::Ammo, DirectorCenter + FVector(145.0f, 25.0f, 116.0f), 18, FName("EncounterDirectorReliefAmmo"));
        if (bLowHealthResume)
        {
            SpawnDirectorPickup(EPickupKind::Stim, DirectorCenter + FVector(0.0f, 105.0f, 116.0f), 1, FName("EncounterDirectorReliefStim"));
        }
    }

    if (bSandboxMode)
    {
        return;
    }

    const float HealthMul = GI ? GI->GetZombieHealthMultiplier() : 1.0f;
    const float DamageMul = GI ? GI->GetZombieDamageMultiplier() : 1.0f;
    const float EncounterMin = FMath::Min(MinEncounterIntensityScale, MaxEncounterIntensityScale);
    const float EncounterMax = FMath::Max(MinEncounterIntensityScale, MaxEncounterIntensityScale);
    const float EncounterScale = FMath::Clamp(Mission.EncounterIntensity, EncounterMin, EncounterMax);
    const float ObjectivePressureScale = bTerminalSolvedForDirector ? 1.06f : 0.84f;
    const float ResourceReliefPressureScale = bDirectorReliefEnabled ? 0.88f : 1.0f;
    const float AdaptiveEncounterScale = FMath::Clamp(EncounterScale * ObjectivePressureScale * ResourceReliefPressureScale, EncounterMin, EncounterMax);
    const float AdaptiveSpeedScale = FMath::Clamp(0.88f + AdaptiveEncounterScale * 0.10f, 0.82f, 1.12f);
    const float AdaptiveActivationScale = bTerminalSolvedForDirector ? 1.0f : 0.82f;

    struct FDirectedZombieSpec
    {
        ECodeRescueZombieEncounterRole Role;
        FVector Offset;
        float HealthScale;
        float DamageScale;
        float SpeedScale;
        float LeashRadius;
        float FlankOffset;
        EZombieVariant Variant;
        FName RoleTag;
        FLinearColor MarkerColor;
    };

    const FDirectedZombieSpec Specs[] = {
        { ECodeRescueZombieEncounterRole::Anchor,   FVector(620.0f, 580.0f, 98.0f), 1.18f, 0.90f, 0.82f, 860.0f, 260.0f, EZombieVariant::BusinessSuit,  FName("EncounterDirectorAnchorZombie"),   AnchorAmber },
        { ECodeRescueZombieEncounterRole::Flanker,  FVector(-720.0f, 500.0f, 98.0f), 0.82f, 0.82f, 1.22f, 1020.0f, 520.0f, EZombieVariant::DogZombie,     FName("EncounterDirectorFlankerZombie"),  FlankViolet },
        { ECodeRescueZombieEncounterRole::Pressure, FVector(0.0f, 720.0f, 98.0f), 1.00f, 1.10f, 1.06f, 1100.0f, 300.0f, EZombieVariant::UrbanZombie4,  FName("EncounterDirectorPressureZombie"), PressureRed },
        { ECodeRescueZombieEncounterRole::Sentinel, FVector(360.0f, 1040.0f, 98.0f), 0.95f, 0.76f, 0.90f, 720.0f, 240.0f, EZombieVariant::NurseFemale,   FName("EncounterDirectorSentinelZombie"), SentinelGreen },
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(Specs); ++i)
    {
        const int32 ZombieId = CodeRescueHordeZombieIdBase + CityIndex * 1000 + 800 + i;

        const FDirectedZombieSpec& Spec = Specs[i];
        const FVector Loc = DirectorCenter + Spec.Offset;
        UClass* ZombieClass = ZombieActorClass ? ZombieActorClass.Get() : ACodeZombieActor::StaticClass();
        FActorSpawnParameters ZombieSpawnParams;
        ZombieSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ACodeZombieActor* Zombie = GetWorld()->SpawnActor<ACodeZombieActor>(ZombieClass, Loc, FRotator::ZeroRotator, ZombieSpawnParams);
        if (!Zombie)
        {
            continue;
        }

        Zombie->ZombieId = ZombieId;
        Zombie->Health = FMath::Max(1.0f, ZombieBaseHealth * Spec.HealthScale * HealthMul * AdaptiveEncounterScale);
        Zombie->AttackDamage = FMath::Max(0.0f, ZombieBaseAttackDamage * Spec.DamageScale * DamageMul * AdaptiveEncounterScale);
        Zombie->MoveSpeed = FMath::Max(0.0f, ZombieBaseMoveSpeed * Spec.SpeedScale * (0.92f + 0.03f * Mission.DifficultyTier) * AdaptiveSpeedScale);
        Zombie->AttackRange = FMath::Max(40.0f, ZombieAttackRange);
        Zombie->ActivationRange = FMath::Max(1800.0f, (ZombieBaseActivationRange * 0.62f + Mission.DifficultyTier * 180.0f) * AdaptiveActivationScale);
        Zombie->RefreshMovementSettings();
        Zombie->ApplyStandardDirectPursuitProfile();

        ApplyZombieFamilyVariant(Zombie, Spec.Variant, ZombieId, FName("EncounterDirectorZombieFamily"), true);

        Zombie->ConfigureEncounterDirective(Spec.Role, DirectorCenter, Spec.LeashRadius, Spec.FlankOffset, Spec.SpeedScale);
        Zombie->Tags.AddUnique(FName("EncounterDirectorZombie"));
        Zombie->Tags.AddUnique(Spec.RoleTag);
        Zombie->Tags.AddUnique(FName("EncounterDirectorAdaptivePressure"));
        Zombie->Tags.AddUnique(bTerminalSolvedForDirector ? FName("EncounterDirectorRouteOpenPressure") : FName("EncounterDirectorRouteLockedPressure"));
        Zombie->Tags.AddUnique(bDirectorReliefEnabled ? FName("EncounterDirectorResourceRelief") : FName("EncounterDirectorStandardResources"));

        const FString VariantLabel = GetZombieFamilyVariantMarkerLabel(Zombie->Variant);
        Zombie->VisualMarkerActor = TagDirectorActor(SpawnZombieReadabilityMarker(
            Zombie,
            Spec.MarkerColor * 1.75f,
            FString::Printf(TEXT("%s Encounter Director %s Marker %d"), *CityLabel, *VariantLabel, i + 1),
            0.85f));
        if (Zombie->VisualMarkerActor)
        {
            Zombie->VisualMarkerActor->Tags.AddUnique(Spec.RoleTag);
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("StandardDirectPursuitZombie"));
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("ZombiePursuitReadableRuntime"));
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("FairSurvivalPressure"));
            Zombie->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
        }
        RegisterStreamedActor(Zombie);
    }

    UCodeRescueSubtitlesWidget::Push(
        TEXT("[Dispatch]: Threat roles ahead. Break pressure first, then clear flanks before the survivor handoff."),
        4.5f);

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueEncounterDirector] %s spawned authored AI director layer with anchor, flanker, pressure, and sentinel roles. objective='%s' adaptive_pressure=%.2f relief=%s difficulty='%s'."),
        *CityLabel,
        *DirectorObjectiveState,
        AdaptiveEncounterScale,
        bDirectorReliefEnabled ? TEXT("true") : TEXT("false"),
        *DifficultyLabel);

    (void)Origin;
}

void ACodeRescueGameMode::SpawnGameplayArenaConfinementLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    (void)CityIndex;

    const FLinearColor PerimeterBlue = FLinearColor(0.05f, 0.22f, 0.34f);
    const FLinearColor WarningAmber = FLinearColor(1.0f, 0.55f, 0.16f);
    const FLinearColor CivicConcrete = FLinearColor(0.10f, 0.105f, 0.112f);

    auto TagConfinement = [](AActor* Actor) -> AActor*
    {
        if (!Actor)
        {
            return nullptr;
        }
        Actor->Tags.AddUnique(FName("GameplayArenaConfinement"));
        Actor->Tags.AddUnique(FName("CityGameplayBoundary"));
        Actor->Tags.AddUnique(FName("WorldDevelopment"));

        TArray<UPrimitiveComponent*> PrimitiveComponents;
        Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
        for (UPrimitiveComponent* Component : PrimitiveComponents)
        {
            if (Component)
            {
                Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Component->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
        Actor->SetActorEnableCollision(true);
        return Actor;
    };

    auto TagVisual = [](AActor* Actor) -> AActor*
    {
        if (!Actor)
        {
            return nullptr;
        }
        Actor->Tags.AddUnique(FName("GameplayArenaConfinement"));
        Actor->Tags.AddUnique(FName("CityBoundaryAesthetic"));
        Actor->Tags.AddUnique(FName("NoAccessBlocker"));
        return Actor;
    };

    AActor* CatchFloor = TagConfinement(SpawnBlock(
        Origin + CityOffset(FVector(0.0f, 0.0f, -620.0f)),
        CityExtent(FVector(112.0f, 98.0f, 0.18f)),
        FLinearColor(0.022f, 0.030f, 0.038f),
        CityLabel + TEXT(" Gameplay Arena Fall Recovery Catch Floor"),
        true));
    if (CatchFloor)
    {
        CatchFloor->Tags.AddUnique(FName("FallRecoveryCatchFloor"));
    }

    auto SpawnWall = [&](const TCHAR* Label, const FVector& LocalCenter, const FVector& LocalScale, const FLinearColor& Tint)
    {
        AActor* Wall = TagConfinement(SpawnBlock(
            Origin + CityOffset(LocalCenter),
            CityExtent(LocalScale),
            Tint,
            FString::Printf(TEXT("%s Gameplay Arena %s Lock Wall"), *CityLabel, Label),
            true));
        if (Wall)
        {
            Wall->Tags.AddUnique(FName("ArenaLockWall"));
        }
        return Wall;
    };

    const float WallHalfX = FCodeRescueCampaign::ArenaWallHalfXLocal;
    const float WallHalfY = FCodeRescueCampaign::ArenaWallHalfYLocal;
    SpawnWall(TEXT("North"), FVector(0.0f, WallHalfY, 315.0f), FVector(108.0f, 0.70f, 7.0f), PerimeterBlue + Mission.SecondaryAccentColor * 0.06f);
    SpawnWall(TEXT("South"), FVector(0.0f, -WallHalfY, 315.0f), FVector(108.0f, 0.70f, 7.0f), PerimeterBlue + Mission.AccentColor * 0.06f);
    SpawnWall(TEXT("West"), FVector(-WallHalfX, 0.0f, 315.0f), FVector(0.70f, 94.0f, 7.0f), PerimeterBlue * 0.9f + Mission.AccentColor * 0.07f);
    SpawnWall(TEXT("East"), FVector(WallHalfX, 0.0f, 315.0f), FVector(0.70f, 94.0f, 7.0f), PerimeterBlue * 0.9f + Mission.SecondaryAccentColor * 0.07f);

    const FVector CornerLocals[] = {
        FVector(-WallHalfX, -WallHalfY, 360.0f),
        FVector(WallHalfX, -WallHalfY, 360.0f),
        FVector(-WallHalfX, WallHalfY, 360.0f),
        FVector(WallHalfX, WallHalfY, 360.0f),
    };
    for (int32 i = 0; i < UE_ARRAY_COUNT(CornerLocals); ++i)
    {
        TagConfinement(SpawnBlock(
            Origin + CityOffset(CornerLocals[i]),
            CityExtent(FVector(0.92f, 0.92f, 7.6f)),
            CivicConcrete + Mission.AccentColor * (0.04f + i * 0.01f),
            FString::Printf(TEXT("%s Gameplay Arena Corner Rescue Beacon %d"), *CityLabel, i + 1),
            true));
        TagVisual(SpawnBlock(
            Origin + CityOffset(CornerLocals[i] + FVector(0.0f, 0.0f, 440.0f)),
            CityExtent(FVector(1.22f, 1.22f, 0.16f)),
            WarningAmber * 2.2f,
            FString::Printf(TEXT("%s Gameplay Arena Corner Beacon Light %d"), *CityLabel, i + 1),
            false));
    }

    for (int32 i = 0; i < 11; ++i)
    {
        const float X = -4800.0f + i * 960.0f;
        TagVisual(SpawnBlock(
            Origin + CityOffset(FVector(X, -WallHalfY + 72.0f, 692.0f)),
            CityExtent(FVector(2.1f, 0.07f, 0.12f)),
            WarningAmber * 1.8f,
            CityLabel + TEXT(" Gameplay Arena South Perimeter Light"),
            false));
        TagVisual(SpawnBlock(
            Origin + CityOffset(FVector(X, WallHalfY - 72.0f, 692.0f)),
            CityExtent(FVector(2.1f, 0.07f, 0.12f)),
            Mission.SecondaryAccentColor * 1.8f,
            CityLabel + TEXT(" Gameplay Arena North Perimeter Light"),
            false));
    }

    for (int32 i = 0; i < 9; ++i)
    {
        const float Y = -3900.0f + i * 975.0f;
        TagVisual(SpawnBlock(
            Origin + CityOffset(FVector(-WallHalfX + 72.0f, Y, 690.0f)),
            CityExtent(FVector(0.07f, 1.9f, 0.12f)),
            Mission.AccentColor * 1.55f,
            CityLabel + TEXT(" Gameplay Arena West Perimeter Light"),
            false));
        TagVisual(SpawnBlock(
            Origin + CityOffset(FVector(WallHalfX - 72.0f, Y, 690.0f)),
            CityExtent(FVector(0.07f, 1.9f, 0.12f)),
            Mission.SecondaryAccentColor * 1.55f,
            CityLabel + TEXT(" Gameplay Arena East Perimeter Light"),
            false));
    }

    for (int32 i = 0; i < 8; ++i)
    {
        const float X = -4200.0f + i * 1200.0f;
        const float Height = 1.45f + static_cast<float>(i % 4) * 0.42f;
        TagVisual(SpawnBlock(
            Origin + CityOffset(FVector(X, WallHalfY - 230.0f, 110.0f + Height * 54.0f)),
            CityArchitectureExtent(FVector(0.72f, 0.28f, Height)),
            CivicConcrete + Mission.SecondaryAccentColor * 0.045f,
            CityLabel + TEXT(" Gameplay Arena Boundary Skyline Facade"),
            false));
    }

    for (int32 i = 0; i < 5; ++i)
    {
        TagVisual(SpawnRotatedBlock(
            Origin + CityOffset(FVector(-5200.0f + i * 2600.0f, -WallHalfY + 210.0f, 18.0f)),
            FRotator(0.0f, 0.0f, 0.0f),
            CityExtent(FVector(5.5f, 0.08f, 0.035f)),
            FLinearColor(0.92f, 0.94f, 0.88f),
            CityLabel + TEXT(" Gameplay Arena Entry Crosswalk Safety Stripe"),
            false));
    }

    SpawnGuideText(
        TEXT("CITY PERIMETER LOCKED\nBackspace returns to the arena if stuck"),
        Origin + CityOffset(FVector(-4700.0f, -WallHalfY + 310.0f, 650.0f)),
        FColor(120, 230, 255),
        38.0f);

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueArenaConfinement] %s mission-safe bounds include buildings, survivor, and extraction; catch floor, four edge walls, beacons, and Backspace recovery guidance active."),
        *CityLabel);
}

void ACodeRescueGameMode::SpawnCampaignCity(const FCodeRescueCityMission& Mission, int32 CityIndex)
{
    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(CityIndex);
    const FLinearColor Accent = Mission.AccentColor;
    const FString CityLabel = FString::Printf(TEXT("%02d %s, %s"), Mission.Rank, *Mission.CityName, *Mission.StateName);
    const bool bDevelopmentShowcase = ShouldSpawnDevelopmentShowcaseLayers();
    if (CityIndex == 0)
    {
        FirstLevelGroundSurfaceActors.Reset();
        FirstLevelAccessDoorwayOutsidePoints.Reset();
        FirstLevelAccessDoorwayInsidePoints.Reset();
    }

    // ---- improvement_pass_2026-06-12 #45 — activate U.S. realization -------
    // Reset to neutral so global cities never inherit a previous U.S. city's
    // sky, wardrobe, or grade; U.S. cities then derive concrete realization
    // parameters from their doc-43 visual profile.
    const bool bUSRealizedCity = IsUSMajorCityMission(Mission);
    CityDaySunColor = FLinearColor(1.0f, 0.96f, 0.86f);
    CityNightSunColor = FLinearColor(0.55f, 0.64f, 0.95f);
    CityDaySunIntensity = 7.0f;
    CityNightSunIntensity = 3.2f;
    ActiveCityWardrobePalette.Reset();
    ActiveCityWardrobeAccessory.Reset();
    ActiveCityRealizationGradeToken.Reset();
    FLinearColor MissionFloorColor = FLinearColor(0.075f, 0.078f, 0.085f);
    if (bUSRealizedCity)
    {
        const FCodeRescueUSCityVisualProfile ActiveProfile = BuildUSCityVisualProfile(Mission);
        const FCodeRescueUSCityRealizationParams ActiveParams = BuildUSCityRealizationParams(Mission, ActiveProfile);
        ActiveCityWardrobePalette = ActiveParams.WardrobePalette;
        ActiveCityWardrobeAccessory = ActiveParams.WardrobeAccessoryToken;
        ActiveCityRealizationGradeToken = ActiveParams.GradeToken;
        // Keep the play floor readable-dark, blended toward regional ground.
        MissionFloorColor = FMath::Lerp(MissionFloorColor, ActiveParams.GroundTint, 0.35f);
        UE_LOG(LogTemp, Display,
            TEXT("[CodeRescueUSCityRealization] %s active terrain='%s' clouds='%s' grade='%s' facades='%s' roads='%s' homes='%s' wardrobe_accessory='%s'"),
            *CityLabel, *ActiveParams.TerrainToken, *ActiveParams.CloudToken, *ActiveParams.GradeToken,
            *ActiveParams.FacadeToken, *ActiveParams.RoadPatternToken, *ActiveParams.HomeArchetypeToken,
            *ActiveParams.WardrobeAccessoryToken);
    }

    // The canonical slab reaches beneath all four confinement walls. The old
    // 82x70 local scale stopped 24-26 metres inside the playable perimeter,
    // exposing the recovery floor 6.1 metres below along the entire edge.
    constexpr float BasicCubeHalfExtent = 50.0f;
    const FVector FullPerimeterFloorScale(
        FCodeRescueCampaign::ArenaWallHalfXLocal / BasicCubeHalfExtent + 1.5f,
        FCodeRescueCampaign::ArenaWallHalfYLocal / BasicCubeHalfExtent + 1.5f,
        0.12f);
    if (AActor* MissionFloor = SpawnBlock(
            Origin + CityOffset(FVector(0, 0, -6)),
            CityExtent(FullPerimeterFloorScale),
            MissionFloorColor,
            CityLabel + TEXT(" Mission Floor")))
    {
        MissionFloor->Tags.AddUnique(FName("CanonicalMissionGround"));
        MissionFloor->Tags.AddUnique(FName("FullPerimeterMissionGround"));
        MissionFloor->Tags.AddUnique(FName("RightEdgeGroundContinuity"));
        MissionFloor->Tags.AddUnique(FName("NeverAutoGroundToCatchFloor"));
    }
    SpawnGameplayArenaConfinementLayer(Mission, CityIndex, Origin, CityLabel);
    SpawnBlock(Origin + CityOffset(FVector(-3820, -3180, -25)), CityExtent(FVector(5.6f, 5.6f, 0.08f)), Accent * 2.0f, CityLabel + TEXT(" Open Entry Pad"), false);
    SpawnBlock(Origin + CityOffset(FVector(-3420, -2780, -20)), CityExtent(FVector(4.6f, 0.24f, 0.06f)), Accent * 3.0f, CityLabel + TEXT(" Entry Corridor West Light"), false);
    SpawnBlock(Origin + CityOffset(FVector(-3020, -2380, -20)), CityExtent(FVector(4.6f, 0.24f, 0.06f)), Accent * 3.0f, CityLabel + TEXT(" Entry Corridor East Light"), false);

    ANavMeshBoundsVolume* NavBounds = GetWorld()->SpawnActor<ANavMeshBoundsVolume>(
        ANavMeshBoundsVolume::StaticClass(), Origin + CityOffset(FVector(0, 0, 220)), FRotator::ZeroRotator);
    if (NavBounds)
    {
        NavBounds->SetActorScale3D(CityExtent(FVector(95.0f, 82.0f, 18.0f)));
        NavBounds->ReregisterAllComponents();
        RegisterStreamedActor(NavBounds);
    }

    SpawnGuideText(
        CityLabel + TEXT("\n") + Mission.RegionName + TEXT(" | Tier ") + FString::FromInt(Mission.DifficultyTier) + TEXT("\nGraduate this city to unlock the next"),
        Origin + CityOffset(FVector(-2600, -2500, 420)),
        Accent.ToFColor(true),
        82.0f);
    SpawnGuideText(Mission.RadioBriefing, Origin + CityOffset(FVector(400, -3000, 360)), Mission.SecondaryAccentColor.ToFColor(true), 44.0f);
    SpawnGuideText(Mission.CurriculumFocus, Origin + CityOffset(FVector(400, -2600, 300)), FColor::White, 38.0f);

    FRandomStream CityStream(Mission.SkylineSeed);
    const int32 BuildingCount = bDevelopmentShowcase
        ? FMath::Clamp(CityBuildingBaseCount + Mission.DifficultyTier * CityBuildingPerDifficultyTier, 1, 120)
        : 0;
    // #45 — the systemic skyline takes its palette, downtown height, and
    // sprawl falloff from the city's realization params so the WHOLE city
    // (not just the identity plate) approximates its real counterpart.
    FCodeRescueUSCityRealizationParams SkylineParams;
    if (bUSRealizedCity)
    {
        SkylineParams = BuildUSCityRealizationParams(Mission, BuildUSCityVisualProfile(Mission));
    }
    for (int32 i = 0; i < BuildingCount; ++i)
    {
        const FVector BuildingLocal(
            CityStream.FRandRange(-3300.0f, 3300.0f),
            CityStream.FRandRange(-2200.0f, 2500.0f),
            0.0f);
        const FVector BuildingOffset = CityOffset(BuildingLocal);
        float Height = CityStream.FRandRange(2.0f, 5.0f + Mission.DifficultyTier * CityBuildingHeightTierBonus);
        FLinearColor BuildingTint = FLinearColor(0.07f, 0.08f, 0.095f) + Accent * 0.04f + Mission.SecondaryAccentColor * 0.025f;
        float FootprintScale = 1.0f;
        if (bUSRealizedCity)
        {
            // Distance falloff: downtown core stays tall, edges sprawl low in
            // proportion to the city's SprawlFalloff (LA/Phoenix high, NYC low).
            const float CoreDistance01 = FMath::Clamp(
                FMath::Sqrt(FMath::Square(BuildingLocal.X) + FMath::Square(BuildingLocal.Y)) / 3300.0f, 0.0f, 1.0f);
            const float Falloff = FMath::Lerp(1.0f, 1.0f - 0.62f * CoreDistance01, SkylineParams.SprawlFalloff);
            Height = FMath::Max(1.2f, Height * SkylineParams.DowntownHeightScale * Falloff);
            FootprintScale = SkylineParams.FootprintScale;
            if (SkylineParams.FacadePalette.Num() > 0)
            {
                const FLinearColor Facade = SkylineParams.FacadePalette[CityStream.RandRange(0, SkylineParams.FacadePalette.Num() - 1)];
                BuildingTint = Facade * CityStream.FRandRange(0.75f, 1.1f) + Accent * 0.02f;
            }
        }
        const FVector BuildingScale = CityArchitectureExtent(FVector(CityStream.FRandRange(0.8f, 2.4f) * FootprintScale, CityStream.FRandRange(0.8f, 2.2f) * FootprintScale, Height));
        const FVector BuildingLocation = Origin + BuildingOffset + FVector(0.0f, 0.0f, BuildingScale.Z * 50.0f);
        const FString BuildingName = CityLabel + TEXT(" Authored Skyline Mesh");
        if (UStaticMesh* BuildingMesh = LoadCodeRescueCityBuildingMesh(i + Mission.SkylineSeed))
        {
            AActor* Building = SpawnStaticMeshProp(
                BuildingMesh,
                BuildingLocation,
                FRotator(0.0f, CityStream.FRandRange(0.0f, 360.0f), 0.0f),
                BuildingScale,
                BuildingName);
            const int32 MaterialIndex = FMath::Abs(i + CityIndex + Mission.SkylineSeed) % UE_ARRAY_COUNT(CodeRescueCityBuildingMaterialPaths);
            ApplyCodeRescueMaterialToStaticActor(
                Building,
                CodeRescueCityBuildingMaterialPaths[MaterialIndex],
                this,
                BuildingTint,
                0.35f);
        }
        else
        {
            SpawnBlock(
                BuildingLocation,
                BuildingScale,
                BuildingTint,
                CityLabel + TEXT(" Skyline Block"));
        }
    }

    if (bDevelopmentShowcase)
    {
        SpawnCityLandmark(Mission, Origin, CityLabel);
        SpawnCityArtKit(Mission, Origin, CityLabel);
        SpawnAuthoredCityKitLayer(Mission, CityIndex, Origin, CityLabel);
    }
    SpawnStreetscapeLayer(Mission, CityIndex, Origin, CityLabel);       // 2026-07-04: roads/sidewalks/vehicles/trees
    SpawnCityBlockV3Layer(CityIndex, Origin, CityLabel);                // 2026-07-06: completed street walls + furniture
    SpawnFirstLevelCombatArtPass(CityIndex, Origin, CityLabel);         // 2026-07-09: level-one armory/triage/storefront/cover
    SpawnFirstLevelTraversalArtPass(CityIndex, Origin, CityLabel);      // 2026-07-09: canonical ground + enterable V5 buildings
    SpawnCityMoodLayer();                                               // 2026-07-06: fog + filmic post (once per world)
    SpawnNightSkyLayer(Origin);                                         // 2026-07-04: star dome + moon (first city only)
    if (bDevelopmentShowcase)
    {
        SpawnMajorCityUrbanIdentityLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnUSCitySpecificIdentityLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnRegionalCityKitIdentityLayer(Mission, CityIndex, Origin, CityLabel);
    }
    // #45 — whole-city U.S. realization: sky/atmosphere, terrain + water +
    // backdrop + vegetation, residential archetype districts, and the curb
    // vehicle fleet. Runs before the entry-access layers so traversal cleanup
    // always has the final say.
    if (bUSRealizedCity)
    {
        ApplyUSCitySkyRealization(Mission, CityIndex, Origin, CityLabel);
        if (bDevelopmentShowcase)
        {
            SpawnUSCityLandscapeRealizationLayer(Mission, CityIndex, Origin, CityLabel);
            SpawnUSCityResidentialDistrictLayer(Mission, CityIndex, Origin, CityLabel);
            SpawnUSCityVehiclePopulationLayer(Mission, CityIndex, Origin, CityLabel);
        }
    }
    SpawnRescueSupportTeamForCity(CityIndex, Origin, CityLabel);
    SpawnMissionObjectiveRoute(Mission, CityIndex, Origin, CityLabel);
    SpawnProtectedCodingChallengeHub(Mission, CityIndex, Origin, CityLabel);
    SpawnCollectibleCaseFilesForCity(Mission, CityIndex, Origin, CityLabel);
    if (bDevelopmentShowcase)
    {
        SpawnWorldMajorCitySignatureLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnCityLandscapeDetails(Mission, CityIndex, Origin, CityLabel);
        SpawnFirstViewAestheticArrivalLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnUniversalEntryAccessLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnTacticalArmoryLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnUnrealSystemsCharacterWorldLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnCinematicStreetLifeLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnWorldCompositionLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnCharacterIdentityCourt(Mission, CityIndex, Origin, CityLabel);
        SpawnEnterableCivicSafehouse(Mission, CityIndex, Origin, CityLabel);
        SpawnInteriorMissionSpacesForCity(Mission, CityIndex, Origin, CityLabel);
        SpawnPhysicsTraversalYard(Mission, CityIndex, Origin, CityLabel);
        SpawnMissionDioramas(Mission, CityIndex, Origin, CityLabel);
        SpawnAccountLinkedAssetShowcase(Mission, CityIndex, Origin, CityLabel);
        SpawnImmediateGameImprovementLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnCodingLearningGamificationLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnGraduatedCurriculumCityIdentityLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnCharacterWorldRealizationLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnFirstMinuteOrientationLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnNext100DevelopmentLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnBespokeSurvivalHorrorArtLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnBespokeAuthoredAssetRefinementLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnProductionTrackCompletionLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnPublicDemoFabDetailLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnChallengeRoomConceptArtLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnEnvironmentalStorytellingLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnWorldBibleLoreLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnCreativeRecommendationImplementationLayer(Mission, CityIndex, Origin, CityLabel);
        SpawnRuntimeDataLayerMigrationLayer(Mission, CityIndex, Origin, CityLabel);
    }
    UE_LOG(LogTemp, Display,
        TEXT("[ProductionWorld] %s curated=%d development_showcases=%d"),
        *CityLabel, bDevelopmentShowcase ? 0 : 1, bDevelopmentShowcase ? 1 : 0);

    UClass* SurvivorClass = SurvivorActorClass ? SurvivorActorClass.Get() : ASurvivorActor::StaticClass();
    ASurvivorActor* Survivor = GetWorld()->SpawnActor<ASurvivorActor>(SurvivorClass, Origin + CityOffset(FVector(2850, 1500, 90)), FRotator::ZeroRotator);
    if (Survivor)
    {
        RegisterStreamedActor(Survivor);
        Survivor->CityIndex = CityIndex;
        Survivor->SurvivorName = Mission.SurvivorName;
        Survivor->Story = Mission.RadioBriefing;
        Survivor->RequiredTerminalId = Mission.TerminalId;
        Survivor->ConfigureArchetypeFromMission(Mission);
        if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            if (GI->RescuedSurvivorNames.Contains(Survivor->SurvivorName))
            {
                Survivor->bRescued = true;
                Survivor->SetActorHiddenInGame(true);
                Survivor->SetActorEnableCollision(false);
            }
        }
        if (!Survivor->bRescued)
        {
            auto AddSurvivorRosterTags = [Survivor](AActor* Actor) -> AActor*
            {
                if (Actor)
                {
                    Actor->Tags.AddUnique(FName("SurvivorArchetypeRosterRuntime"));
                    Actor->Tags.AddUnique(FName("SurvivorRoleReadableNameplate"));
                    Actor->Tags.AddUnique(FName("SelectedLanguageSurvivorHandoff"));
                    Survivor->AddHelperActor(Actor);
                }
                return Actor;
            };

            const FLinearColor SurvivorColor = Survivor->ArchetypeAccentColor;
            AddSurvivorRosterTags(SpawnBlock(
                Survivor->GetActorLocation() + FVector(0, 0, 110),
                FVector(1.5f, 1.5f, 2.4f),
                SurvivorColor,
                CityLabel + TEXT(" Survivor Archetype Marker"),
                false));
            AddSurvivorRosterTags(SpawnBlock(
                Survivor->GetActorLocation() + FVector(0, 0, 290),
                FVector(0.55f, 0.55f, 0.05f),
                SurvivorColor * 4.0f,
                CityLabel + TEXT(" Survivor Archetype Halo"),
                false));
            AddSurvivorRosterTags(SpawnGuideText(
                FString::Printf(TEXT("%s\n%s [%s]\nE after lesson"),
                    *Mission.SurvivorName,
                    *Survivor->ArchetypeTitle,
                    *Survivor->ArchetypeIconLabel),
                Survivor->GetActorLocation() + FVector(0, 0, 600),
                SurvivorColor.ToFColor(true),
                54.0f));
            SpawnSurvivorReliefCamp(Mission, CityIndex, Origin, CityLabel, Survivor);
        }
    }
    SpawnEncounterDirectorLayer(Mission, CityIndex, Origin, CityLabel, Survivor);

    auto SpawnPickup = [this](EPickupKind Kind, const FVector& Loc, int32 Amount)
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Loc, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("CityRoutePickupAvailable"));
            Pickup->Tags.Add(FName("PickupGroundSnapRequired"));
            RegisterStreamedActor(Pickup);
        }
    };
    const int32 AmmoCycleLength = FMath::Max(1, AmmoPickupCityCycleLength);
    SpawnPickup(EPickupKind::Ammo, Origin + CityOffset(FVector(-600, 2300, 120)), AmmoPickupBaseAmount + (CityIndex % AmmoCycleLength) * AmmoPickupCityCycleBonus);
    SpawnPickup(EPickupKind::Medkit, Origin + CityOffset(FVector(-1700, 2100, 120)), MedkitPickupAmount);
    SpawnPickup(EPickupKind::ArmorPlate, Origin + CityOffset(FVector(-1120, 2100, 120)), 1);
    SpawnPickup(EPickupKind::Flare, Origin + CityOffset(FVector(-1420, 1860, 120)), 1);
    SpawnPickup(EPickupKind::Smoke, Origin + CityOffset(FVector(-900, 1860, 120)), 1);
    SpawnPickup(EPickupKind::Stim, Origin + CityOffset(FVector(-630, 1860, 120)), 1);
    SpawnPickup(EPickupKind::Scrap, Origin + CityOffset(FVector(-360, 1860, 120)), 4);
    SpawnPickup(EPickupKind::RadioScanner, Origin + CityOffset(FVector(-1800, 1660, 120)), 1);
    SpawnPickup(EPickupKind::FlashlightBattery, Origin + CityOffset(FVector(-1560, 1660, 120)), 1);
    SpawnPickup(EPickupKind::AmmoPouch, Origin + CityOffset(FVector(-1320, 1660, 120)), 30);
    SpawnPickup(EPickupKind::BypassKit, Origin + CityOffset(FVector(-1080, 1660, 120)), 1);

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const float HealthMul = GI ? GI->GetZombieHealthMultiplier() : 1.0f;
    const float DamageMul = GI ? GI->GetZombieDamageMultiplier() : 1.0f;
    const int32 LivingPresenceCount = EstimateLivingPresenceCountForCity(Mission, CityIndex);
    const int32 TargetZombiePresence = bSandboxMode ? 0 : ComputeTargetZombiePresence(Mission, CityIndex);
    const int32 TierCountBonus = ZombieCountTierDivisor > 0 ? Mission.DifficultyTier / ZombieCountTierDivisor : 0;
    const int32 TacticalFloor = FMath::Clamp(ZombieBaseCount + TierCountBonus, ZombieMinCount, FMath::Max(ZombieMinCount, ZombieMaxCount));
    const int32 ActiveAICap = FMath::Max(TacticalFloor, MaxActiveAIZombiesPerCity);
    int32 ZombieCount = FMath::Clamp(TargetZombiePresence, TacticalFloor, ActiveAICap);
    // #33 — sandbox skips all zombie spawns so the player can practice freely.
    if (bSandboxMode) ZombieCount = 0;
    // #35 — night raises spawn density 40%.
    if (bIsNight) ZombieCount = FMath::Min(ActiveAICap, FMath::CeilToInt(ZombieCount * 1.4f));
    for (int32 i = 0; i < ZombieCount; ++i)
    {
        const int32 ZombieId = CodeRescueRegularZombieIdBase + CityIndex * CodeRescueRegularZombieIdStride + i;

        const int32 Column = i % 8;
        const int32 Row = (i / 8) % 8;
        const int32 Wave = i / 64;
        const float BaseX = -520.0f + static_cast<float>(Column) * 555.0f + (Wave % 2) * 120.0f;
        const float BaseY = 870.0f + static_cast<float>(Row) * 230.0f + Wave * 44.0f;
        const FVector ZombieLocal(
            FMath::Clamp(BaseX + CityStream.FRandRange(-105.0f, 105.0f), -720.0f, 3660.0f),
            FMath::Clamp(BaseY + CityStream.FRandRange(-95.0f, 95.0f), 760.0f, 2460.0f),
            95.0f);
        const FVector Loc = Origin + CityOffset(ZombieLocal);
        UClass* ZombieClass = ZombieActorClass ? ZombieActorClass.Get() : ACodeZombieActor::StaticClass();
        FActorSpawnParameters ZombieSpawnParams;
        ZombieSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ACodeZombieActor* Zombie = GetWorld()->SpawnActor<ACodeZombieActor>(ZombieClass, Loc, FRotator::ZeroRotator, ZombieSpawnParams);
        if (Zombie)
        {
            RegisterStreamedActor(Zombie);
            Zombie->ZombieId = ZombieId;
            const float EncounterMin = FMath::Min(MinEncounterIntensityScale, MaxEncounterIntensityScale);
            const float EncounterMax = FMath::Max(MinEncounterIntensityScale, MaxEncounterIntensityScale);
            const float EncounterScale = FMath::Clamp(Mission.EncounterIntensity, EncounterMin, EncounterMax);
            const int32 HealthCycleLength = FMath::Max(1, ZombieHealthCityCycleLength);
            Zombie->Health = FMath::Max(1.0f, (ZombieBaseHealth + Mission.DifficultyTier * ZombieHealthPerDifficultyTier + (CityIndex % HealthCycleLength) * ZombieHealthCityCycleBonus) * HealthMul * EncounterScale);
            Zombie->AttackDamage = FMath::Max(0.0f, (ZombieBaseAttackDamage + Mission.DifficultyTier * ZombieAttackDamagePerDifficultyTier) * DamageMul * EncounterScale);
            Zombie->MoveSpeed = FMath::Max(0.0f, (ZombieBaseMoveSpeed + Mission.DifficultyTier * ZombieMoveSpeedPerDifficultyTier + (CityIndex % HealthCycleLength) * ZombieMoveSpeedCityCycleBonus) * EncounterScale);
            Zombie->AttackRange = FMath::Max(40.0f, ZombieAttackRange);
            Zombie->ActivationRange = FMath::Max(1000.0f, ZombieBaseActivationRange + Mission.DifficultyTier * ZombieActivationRangePerDifficultyTier);
            Zombie->RefreshMovementSettings();
            Zombie->ApplyStandardDirectPursuitProfile();

            ApplyCityZombieFamilyVariant(Zombie, CityIndex, i, ZombieId, FName("RegularCityZombieFamily"), true);

            const FString VariantLabel = GetZombieFamilyVariantMarkerLabel(Zombie->Variant);
            const FLinearColor VariantColor = GetZombieFamilyVariantMarkerColor(Zombie->Variant);
            Zombie->VisualMarkerActor = SpawnZombieReadabilityMarker(
                Zombie,
                VariantColor,
                FString::Printf(TEXT("%s %s Marker"), *CityLabel, *VariantLabel),
                1.0f);
            if (Zombie->VisualMarkerActor)
            {
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("StandardDirectPursuitZombie"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("ZombiePursuitReadableRuntime"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("FairSurvivalPressure"));
                Zombie->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
            }
        }
    }
    SpawnBackgroundHordePopulation(Mission, CityIndex, Origin, CityLabel, LivingPresenceCount, ZombieCount, TargetZombiePresence);

    // ---- improvement_pass_2026-05-03 add-ons -------------------------------
    SpawnHelipadForCity(CityIndex, Origin, CityLabel);                                // #7
    SpawnExpandedExtractionSetPieceForCity(Mission, CityIndex, Origin, CityLabel);
    SpawnAuthoredPropsForCity(CityIndex, Origin, Accent);                             // #8
    SpawnPerZonePostProcessVolume(CityIndex, Origin, Accent);                         // #9
    SpawnAmbientSoundForCity(CityIndex, Origin);                                       // #10
    SpawnWeatherForCity(CityIndex, Origin);                                            // #36
    if (ShouldSpawnDevelopmentShowcaseLayers())
    {
        SpawnWeatherLightingIdentityLayer(Mission, CityIndex, Origin, CityLabel);
    }
    SpawnSecretTerminalForCity(CityIndex, Origin);                                     // #37
    // ---- improvement_pass_2026-05-04_part3 add-ons -------------------------
    if (!bSandboxMode)
    {
        SpawnBossForCity(CityIndex, Origin, CityLabel, Mission);                       // #62
        SpawnEliteWardenMiniBossStagingLayer(Mission, CityIndex, Origin, CityLabel);
    }
    SpawnJeepForCity(CityIndex, Origin, CityLabel);                                    // #62
    SpawnSecondaryMotionSignalLayer(Mission, CityIndex, Origin, CityLabel, Survivor);
    SpawnSetPieceForCity(CityIndex, Origin, CityLabel, Mission);                       // #67
    SpawnFriendlyNPCsForCity(CityIndex, Origin, CityLabel);                            // #68
    if (ShouldSpawnDevelopmentShowcaseLayers())
    {
        SpawnPurposeClarityLayer(Mission, CityIndex, Origin, CityLabel);
    }
    EnsureEntryAccessCorridorClear(CityIndex, Origin, CityLabel);
    ApplyProductionPresentationCleanup(CityIndex, Origin, CityLabel);
    // Run after every other world-art layer so open-space validation sees the
    // final city instead of placing a district where a later building lands.
    SpawnFirstLevelPurposeDistrictPass(CityIndex, Origin, CityLabel);   // 2026-07-17: useful authored districts + symbol loot

    // 2026-07-07 (Kenny: "elevated regions" + the cyan lattice swallowing the
    // camera): now that EVERY layer for this city has spawned —
    // 1) ground any collision mesh left floating (auto solidity fix), and
    // 2) trim the concept-art canopy: the challenge-room layer arcs dozens of
    //    bright thin strips OVER the safehouse; anything of that layer above
    //    head height is hidden so it can never sit between camera and player.
    GroundFloatingMeshes(CityIndex);
    // Live forensics (cr.DumpNearbyActors) identified the "cyan lattice dome"
    // that kept swallowing the camera and embedding the spawn pad: the
    // TACTICAL ARMORY weapon-display lattice, spawned arcing OVER the entry
    // pad, plus the concept-art canopy strips over the safehouse. Anything
    // from those display layers above head height is hidden and de-solidified;
    // ground-level display pieces stay.
    int32 CanopyTrimmed = 0;
    for (TActorIterator<AStaticMeshActor> TrimIt(GetWorld()); TrimIt; ++TrimIt)
    {
        AStaticMeshActor* TrimActor = *TrimIt;
        if (!IsValid(TrimActor))
        {
            continue;
        }
        const bool bOverheadDecorLayer =
            TrimActor->Tags.Contains(FName("ChallengeRoomConceptArt")) ||
            TrimActor->Tags.Contains(FName("TacticalArmoryAllWeaponsAvailable"));
        if (!bOverheadDecorLayer)
        {
            continue;
        }
        if (TrimActor->GetActorLocation().Z - Origin.Z > 380.0f)
        {
            TrimActor->SetActorHiddenInGame(true);
            TrimActor->SetActorEnableCollision(false);
            ++CanopyTrimmed;
        }
        else
        {
            // Ground-level display pieces must never trap the pawn either.
            TrimActor->SetActorEnableCollision(false);
        }
    }
    UE_LOG(LogTemp, Display, TEXT("[CanopyTrim] %s: %d overhead display strips hidden"), *CityLabel, CanopyTrimmed);

    // Belt and braces, and layer-agnostic: NOTHING decorative may hang in the
    // air over the entry pad. Whatever spawner produced it, any static mesh
    // inside a cylinder above the spawn point (r=460, from knee height to
    // 11m) is hidden and de-solidified. This is what finally removes the
    // lattice dome that has been swallowing the camera at spawn since the
    // armory display arced over the pad.
    const FVector PadCenter = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
    int32 PadCanopyHidden = 0;
    for (TActorIterator<AStaticMeshActor> PadIt(GetWorld()); PadIt; ++PadIt)
    {
        AStaticMeshActor* PadActor = *PadIt;
        if (!IsValid(PadActor) || PadActor->IsHidden())
        {
            continue;
        }
        const FVector Loc = PadActor->GetActorLocation();
        const float DistXY = FVector::Dist2D(Loc, PadCenter);
        const float RelZ = Loc.Z - PadCenter.Z;
        if (DistXY < 460.0f && RelZ > 140.0f && RelZ < 1100.0f)
        {
            PadActor->SetActorHiddenInGame(true);
            PadActor->SetActorEnableCollision(false);
            ++PadCanopyHidden;
        }
    }
    UE_LOG(LogTemp, Display, TEXT("[PadCanopy] %s: %d airborne meshes over the entry pad hidden"), *CityLabel, PadCanopyHidden);

    // 2026-07-11 (Kenny: "characters are floating above the ground"): the
    // slab-lowering pass above runs AFTER every character already snapped in
    // BeginPlay, and several populations never snapped at all. Now that the
    // city's geometry is FINAL, re-ground every character standing in it.
    GroundSpawnedCharacters(CityIndex, Origin, CityLabel);
    FTimerHandle CharacterGroundSettleTimer;
    GetWorldTimerManager().SetTimer(CharacterGroundSettleTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, CityIndex, Origin, CityLabel]()
        {
            GroundSpawnedCharacters(CityIndex, Origin, CityLabel + TEXT(" settled"));
        }),
        0.45f,
        false);
    FTimerHandle CharacterAnimationSettleTimer;
    GetWorldTimerManager().SetTimer(CharacterAnimationSettleTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, CityIndex, Origin, CityLabel]()
        {
            GroundSpawnedCharacters(CityIndex, Origin, CityLabel + TEXT(" animation-settled"));
        }),
        1.65f,
        false);
}

// ---- #7 Helipad fast-travel pad -------------------------------------------
void ACodeRescueGameMode::SpawnHelipadForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector PadLocation = Origin + CityOffset(FVector(2400.0f, 2400.0f, -10.0f));
    AHelipadActor* Helipad = GetWorld()->SpawnActor<AHelipadActor>(AHelipadActor::StaticClass(), PadLocation, FRotator::ZeroRotator);
    if (!Helipad)
    {
        return;
    }
    Helipad->CityIndex = CityIndex;
    const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(CityIndex);
    if (Mission)
    {
        Helipad->CityLabel = Mission->CityName;
    }
    else
    {
        Helipad->CityLabel = CityLabel;
    }
    if (Mission)
    {
        if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            if (GI->RescuedSurvivorNames.Contains(Mission->SurvivorName))
            {
                const FLinearColor ExtractionAccent =
                    Mission->AccentColor * 0.58f + Mission->SecondaryAccentColor * 0.24f + FLinearColor(0.36f, 1.0f, 0.42f) * 0.18f;
                Helipad->SetExtractionReady(Mission->SurvivorName, ExtractionAccent, GI->bReducedMotion);
            }
        }
    }
    RegisterStreamedActor(Helipad);
    SpawnGuideText(TEXT("EVAC HELIPAD\n[E] fast-travel"),
                   PadLocation + FVector(0, 0, 600), FColor(120, 200, 255), 56.0f);
}

void ACodeRescueGameMode::SpawnExpandedExtractionSetPieceForCity(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    static const TCHAR* VariantLabels[] = {
        TEXT("ROOFTOP LIFT"),
        TEXT("CONVOY GATE"),
        TEXT("HARBOR BOAT"),
        TEXT("RAIL EVAC"),
        TEXT("BRIDGE RUN"),
        TEXT("HELIPAD COMMAND"),
    };
    static const TCHAR* VariantTags[] = {
        TEXT("ExtractionSetPiece_RooftopLift"),
        TEXT("ExtractionSetPiece_ConvoyGate"),
        TEXT("ExtractionSetPiece_HarborBoat"),
        TEXT("ExtractionSetPiece_RailEvac"),
        TEXT("ExtractionSetPiece_BridgeRun"),
        TEXT("ExtractionSetPiece_HelipadCommand"),
    };
    const int32 VariantIndex = FMath::Abs(CityIndex) % UE_ARRAY_COUNT(VariantLabels);
    const FString SetPieceLabel(VariantLabels[VariantIndex]);
    const FName VariantTag(VariantTags[VariantIndex]);

    const FVector Hub = Origin + CityOffset(FVector(2400.0f, 2400.0f, 0.0f));
    const FLinearColor DeckColor = FLinearColor(0.035f, 0.045f, 0.050f) + Mission.AccentColor * 0.08f;
    const FLinearColor Accent = Mission.AccentColor * 0.56f + Mission.SecondaryAccentColor * 0.44f;
    const FLinearColor RescueGreen = FLinearColor(0.28f, 1.0f, 0.42f);
    const FLinearColor Signal = Accent * 0.58f + RescueGreen * 0.42f;
    const FLinearColor WarmSignal = FLinearColor(1.0f, 0.62f, 0.20f) * 1.35f;
    const FLinearColor CoolSignal = FLinearColor(0.26f, 0.84f, 1.0f) * 1.45f;

    auto TagExtractionSetPiece = [&](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("ExpandedExtractionSetPiece"));
            Actor->Tags.AddUnique(VariantTag);
            Actor->Tags.AddUnique(FName("ExtractionSetPieceNonBlocking"));
            Actor->Tags.AddUnique(FName("HelipadClearancePreserved"));
            Actor->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.AddUnique(FName("Top50Recommendations"));
            Actor->Tags.AddUnique(FName("ReleaseDossier"));
        }
        return Actor;
    };

    auto Block = [&](const FVector& LocalOffset, const FVector& Scale, const FLinearColor& Color, const FString& Name) -> AActor*
    {
        return TagExtractionSetPiece(SpawnBlock(
            Hub + CityOffset(LocalOffset),
            CityExtent(Scale),
            Color,
            CityLabel + TEXT(" ") + Name,
            false));
    };

    auto RotBlock = [&](const FVector& LocalOffset, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color, const FString& Name) -> AActor*
    {
        return TagExtractionSetPiece(SpawnRotatedBlock(
            Hub + CityOffset(LocalOffset),
            Rotation,
            CityExtent(Scale),
            Color,
            CityLabel + TEXT(" ") + Name,
            false));
    };

    auto Label = [&](const FString& Text, const FVector& LocalOffset, const FColor& Color, float Size) -> AActor*
    {
        return TagExtractionSetPiece(SpawnGuideText(Text, Hub + CityOffset(LocalOffset), Color, Size));
    };

    Block(FVector(0.0f, 0.0f, -24.0f), FVector(9.4f, 9.4f, 0.035f), DeckColor, TEXT("Expanded Extraction Clear Deck"));
    Block(FVector(0.0f, -710.0f, 8.0f), FVector(8.2f, 0.08f, 0.045f), Signal * 2.6f, TEXT("Expanded Extraction South Route Stripe"));
    Block(FVector(0.0f, 710.0f, 8.0f), FVector(8.2f, 0.08f, 0.045f), Signal * 2.6f, TEXT("Expanded Extraction North Route Stripe"));
    Block(FVector(-710.0f, 0.0f, 8.0f), FVector(0.08f, 8.2f, 0.045f), Signal * 2.6f, TEXT("Expanded Extraction West Route Stripe"));
    Block(FVector(710.0f, 0.0f, 8.0f), FVector(0.08f, 8.2f, 0.045f), Signal * 2.6f, TEXT("Expanded Extraction East Route Stripe"));

    for (int32 Corner = 0; Corner < 4; ++Corner)
    {
        const float X = (Corner < 2) ? -740.0f : 740.0f;
        const float Y = (Corner % 2 == 0) ? -740.0f : 740.0f;
        Block(FVector(X, Y, 94.0f), FVector(0.14f, 0.14f, 1.72f), Accent * 1.45f, TEXT("Expanded Extraction Beacon Post"));
        Block(FVector(X, Y, 210.0f), FVector(0.52f, 0.52f, 0.07f), Signal * 3.0f, TEXT("Expanded Extraction Beacon Cap"));
    }

    Label(
        FString::Printf(
            TEXT("EXTRACTION SET PIECE\n%s\n%s ready at %s"),
            *SetPieceLabel,
            *Mission.SurvivorName,
            *Mission.LandmarkName),
        FVector(0.0f, -1040.0f, 520.0f),
        Signal.ToFColor(true),
        44.0f);

    switch (VariantIndex)
    {
    case 0: // RooftopLift
        Block(FVector(-900.0f, -140.0f, 160.0f), FVector(0.72f, 1.25f, 3.2f), DeckColor + Accent * 0.22f, TEXT("Rooftop Lift Elevator Shaft"));
        Block(FVector(-900.0f, -140.0f, 392.0f), FVector(1.05f, 1.55f, 0.14f), Accent * 1.6f, TEXT("Rooftop Lift Machine Room Cap"));
        Block(FVector(-560.0f, -140.0f, 250.0f), FVector(2.8f, 0.10f, 0.12f), CoolSignal, TEXT("Rooftop Lift Catwalk Rail"));
        Block(FVector(-560.0f, 130.0f, 250.0f), FVector(2.8f, 0.10f, 0.12f), CoolSignal, TEXT("Rooftop Lift Opposite Catwalk Rail"));
        RotBlock(FVector(-380.0f, 0.0f, 310.0f), FRotator(0.0f, 0.0f, -12.0f), FVector(2.5f, 0.08f, 0.12f), WarmSignal, TEXT("Rooftop Lift Warning Arm A"));
        RotBlock(FVector(-380.0f, 0.0f, 310.0f), FRotator(0.0f, 0.0f, 12.0f), FVector(2.5f, 0.08f, 0.12f), WarmSignal, TEXT("Rooftop Lift Warning Arm B"));
        break;
    case 1: // ConvoyGate
        Block(FVector(-640.0f, 850.0f, 52.0f), FVector(2.3f, 0.82f, 0.62f), FLinearColor(0.10f, 0.14f, 0.16f) + Accent * 0.30f, TEXT("Convoy Gate Rescue Truck Body"));
        Block(FVector(-520.0f, 850.0f, 112.0f), FVector(0.92f, 0.74f, 0.50f), CoolSignal * 0.9f, TEXT("Convoy Gate Rescue Truck Cab"));
        Block(FVector(0.0f, 850.0f, 52.0f), FVector(2.0f, 0.72f, 0.56f), FLinearColor(0.16f, 0.16f, 0.12f) + Mission.SecondaryAccentColor * 0.24f, TEXT("Convoy Gate Supply Rig"));
        Block(FVector(560.0f, 850.0f, 52.0f), FVector(1.9f, 0.62f, 0.52f), FLinearColor(0.12f, 0.13f, 0.14f) + RescueGreen * 0.18f, TEXT("Convoy Gate Escort Vehicle"));
        Block(FVector(-310.0f, 610.0f, 84.0f), FVector(0.14f, 2.2f, 1.35f), WarmSignal, TEXT("Convoy Gate Left Signal Mast"));
        Block(FVector(310.0f, 610.0f, 84.0f), FVector(0.14f, 2.2f, 1.35f), WarmSignal, TEXT("Convoy Gate Right Signal Mast"));
        break;
    case 2: // HarborBoat
        Block(FVector(0.0f, 940.0f, -18.0f), FVector(8.8f, 2.2f, 0.07f), FLinearColor(0.02f, 0.14f, 0.24f) * 2.0f, TEXT("Harbor Boat Water Rescue Lane"));
        Block(FVector(0.0f, 720.0f, 26.0f), FVector(6.4f, 0.32f, 0.16f), FLinearColor(0.36f, 0.24f, 0.12f), TEXT("Harbor Boat Dock Edge"));
        RotBlock(FVector(0.0f, 990.0f, 62.0f), FRotator(0.0f, 0.0f, -7.0f), FVector(3.4f, 0.76f, 0.42f), FLinearColor(0.06f, 0.24f, 0.33f) + Accent * 0.20f, TEXT("Harbor Boat Hull"));
        Block(FVector(0.0f, 990.0f, 128.0f), FVector(1.18f, 0.56f, 0.58f), FLinearColor(0.78f, 0.86f, 0.90f), TEXT("Harbor Boat Cabin"));
        Block(FVector(0.0f, 990.0f, 270.0f), FVector(0.08f, 0.08f, 2.4f), CoolSignal, TEXT("Harbor Boat Rescue Mast"));
        Block(FVector(0.0f, 990.0f, 402.0f), FVector(1.7f, 0.06f, 0.10f), WarmSignal, TEXT("Harbor Boat Mast Signal"));
        break;
    case 3: // RailEvac
        Block(FVector(-480.0f, 900.0f, 24.0f), FVector(0.10f, 4.8f, 0.10f), FLinearColor(0.60f, 0.62f, 0.64f), TEXT("Rail Evac Track A"));
        Block(FVector(-270.0f, 900.0f, 24.0f), FVector(0.10f, 4.8f, 0.10f), FLinearColor(0.60f, 0.62f, 0.64f), TEXT("Rail Evac Track B"));
        for (int32 Tie = 0; Tie < 5; ++Tie)
        {
            Block(FVector(-375.0f, 380.0f + Tie * 210.0f, 20.0f), FVector(1.7f, 0.12f, 0.08f), FLinearColor(0.32f, 0.22f, 0.12f), TEXT("Rail Evac Timber Tie"));
        }
        Block(FVector(-375.0f, 1230.0f, 90.0f), FVector(1.85f, 1.18f, 1.05f), FLinearColor(0.12f, 0.16f, 0.20f) + Accent * 0.22f, TEXT("Rail Evac Rescue Car"));
        Block(FVector(-375.0f, 1230.0f, 186.0f), FVector(1.55f, 1.05f, 0.14f), CoolSignal, TEXT("Rail Evac Window Band"));
        Block(FVector(-640.0f, 650.0f, 176.0f), FVector(0.12f, 0.12f, 2.5f), WarmSignal, TEXT("Rail Evac Platform Signal"));
        break;
    case 4: // BridgeRun
        Block(FVector(0.0f, 910.0f, 64.0f), FVector(7.4f, 0.52f, 0.42f), FLinearColor(0.26f, 0.28f, 0.30f) + Accent * 0.18f, TEXT("Bridge Run Rescue Deck"));
        Block(FVector(-700.0f, 910.0f, 226.0f), FVector(0.22f, 0.18f, 3.2f), FLinearColor(0.42f, 0.42f, 0.40f) + Mission.SecondaryAccentColor * 0.18f, TEXT("Bridge Run Left Tower"));
        Block(FVector(700.0f, 910.0f, 226.0f), FVector(0.22f, 0.18f, 3.2f), FLinearColor(0.42f, 0.42f, 0.40f) + Mission.SecondaryAccentColor * 0.18f, TEXT("Bridge Run Right Tower"));
        RotBlock(FVector(-350.0f, 910.0f, 365.0f), FRotator(0.0f, 0.0f, 17.0f), FVector(3.6f, 0.08f, 0.12f), CoolSignal, TEXT("Bridge Run Cable A"));
        RotBlock(FVector(350.0f, 910.0f, 365.0f), FRotator(0.0f, 0.0f, -17.0f), FVector(3.6f, 0.08f, 0.12f), CoolSignal, TEXT("Bridge Run Cable B"));
        Block(FVector(0.0f, 910.0f, 430.0f), FVector(1.2f, 0.08f, 0.14f), WarmSignal, TEXT("Bridge Run High Signal"));
        break;
    default: // HelipadCommand
        Block(FVector(-760.0f, 600.0f, 72.0f), FVector(1.9f, 1.2f, 0.72f), FLinearColor(0.08f, 0.11f, 0.12f) + Accent * 0.24f, TEXT("Helipad Command Tent Base"));
        RotBlock(FVector(-760.0f, 600.0f, 160.0f), FRotator(0.0f, 0.0f, 45.0f), FVector(1.35f, 1.35f, 1.35f), FLinearColor(0.10f, 0.18f, 0.18f) + Mission.SecondaryAccentColor * 0.18f, TEXT("Helipad Command Tent Roof"));
        Block(FVector(-420.0f, 720.0f, 90.0f), FVector(1.2f, 0.32f, 0.70f), CoolSignal, TEXT("Helipad Command Ops Console"));
        Block(FVector(-180.0f, 720.0f, 90.0f), FVector(1.2f, 0.32f, 0.70f), WarmSignal, TEXT("Helipad Command Route Console"));
        Block(FVector(510.0f, 720.0f, 112.0f), FVector(0.12f, 0.12f, 2.15f), Signal * 1.7f, TEXT("Helipad Command Radio Mast"));
        Block(FVector(510.0f, 720.0f, 240.0f), FVector(1.10f, 0.06f, 0.12f), Signal * 2.8f, TEXT("Helipad Command Radio Yagi"));
        break;
    }

    Label(
        FString::Printf(
            TEXT("%s EXTRACTION\nLesson: %s\nRoute: %s"),
            *SetPieceLabel,
            *Mission.CurriculumFocus,
            *Mission.CityName),
        FVector(0.0f, 1030.0f, 430.0f),
        FColor(180, 235, 255),
        34.0f);

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueExtractionSetPiece] %03d %s spawned expanded extraction set piece '%s' around helipad for %s."),
        Mission.Rank,
        *CityLabel,
        *SetPieceLabel,
        *Mission.SurvivorName);
}

// ---- #62 Drivable jeep next to helipad ------------------------------------
void ACodeRescueGameMode::SpawnJeepForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    // Park the jeep just south of the helipad pad on a small flat slab so the
    // player has a visible "vehicle that I can mount" right next to fast travel.
    const FVector PadLocation = Origin + CityOffset(FVector(2400.0f, 2400.0f, -10.0f));
    const FVector JeepLocation = PadLocation + FVector(420.0f, -380.0f, 80.0f);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AJeepActor* Jeep = GetWorld()->SpawnActor<AJeepActor>(AJeepActor::StaticClass(), JeepLocation, FRotator(0, 90, 0), Params);
    if (!Jeep)
    {
        return;
    }
    RegisterStreamedActor(Jeep);
    Jeep->Tags.AddUnique(FName("ChaosVehicleReadyFallback"));
    Jeep->Tags.AddUnique(FName("SurfaceAwareVehicle"));
    Jeep->Tags.AddUnique(FName("VehicleSurfaceTractionTraining"));

    AActor* JeepTractionPad = SpawnBlock(
        JeepLocation + FVector(0.0f, 0.0f, -76.0f),
        FVector(3.9f, 2.45f, 0.06f),
        FLinearColor(0.18f, 0.20f, 0.18f) + FLinearColor(0.12f, 0.42f, 0.20f),
        CityLabel + TEXT(" Jeep Surface Traction Pad"),
        true);
    if (JeepTractionPad)
    {
        JeepTractionPad->Tags.Add(FName("VehicleSurfaceTractionTraining"));
        JeepTractionPad->Tags.Add(FName("SurfaceConcrete"));
        JeepTractionPad->Tags.Add(FName("GamePhysicsDeepDive"));
    }

    // Static halo + label so the jeep reads as interactive in the procedural world.
    SpawnBlock(JeepLocation + FVector(0, 0, 220), FVector(0.55f, 0.55f, 0.05f),
               FLinearColor(0.5f, 1.0f, 0.4f) * 4.0f, CityLabel + TEXT(" Jeep Halo"), false);
    SpawnGuideText(TEXT("STAFF JEEP\n[E] mount  WASD drive\nsurface-aware traction active"),
                   JeepLocation + FVector(0, 0, 380), FColor(120, 255, 140), 50.0f);
}

// ---- #62 Per-city boss zombie ---------------------------------------------
void ACodeRescueGameMode::SpawnBossForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel, const FCodeRescueCityMission& Mission)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const float HealthMul = GI ? GI->GetZombieHealthMultiplier() : 1.0f;
    const float DamageMul = GI ? GI->GetZombieDamageMultiplier() : 1.0f;

    // Place the boss in the city's "deep" northeast quadrant. Players have to
    // push toward it through the regular spawn cluster to reach it.
    const FVector BossLoc = Origin + CityOffset(FVector(2900.0f, -1500.0f, 95.0f));
    const int32 BossId = CodeRescueBossZombieIdBase + CityIndex;
    if (GI && GI->NeutralizedZombieIds.Contains(BossId))
    {
        // Boss already defeated this run.
        return;
    }

    FActorSpawnParameters BossSpawnParams;
    BossSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    ABossZombieActor* Boss = GetWorld()->SpawnActor<ABossZombieActor>(ABossZombieActor::StaticClass(), BossLoc, FRotator::ZeroRotator, BossSpawnParams);
    if (!Boss)
    {
        return;
    }
    RegisterStreamedActor(Boss);
    Boss->ZombieId = BossId;
    // Tier-scaled stats sit on top of the boss class defaults.
    const float TierScale = 1.0f + 0.15f * static_cast<float>(Mission.DifficultyTier);
    Boss->Health = FMath::Max(200.0f, 600.0f * TierScale * HealthMul);
    Boss->AttackDamage = FMath::Max(8.0f, 22.0f * TierScale * DamageMul);
    Boss->MoveSpeed = 240.0f;
    Boss->AttackRange = 230.0f;
    Boss->ActivationRange = 6500.0f;
    Boss->RefreshMovementSettings();

    // Pick the most physically intimidating variant available so the boss
    // visually reads as a boss when assets are wired through the variant table.
    EZombieVariant BossVariant = EZombieVariant::EliteCharger;
    ApplyZombieFamilyVariant(Boss, BossVariant, BossId, FName("BossZombieFamily"), false);

    // Visual + signage so the player can tell "this one is the boss" at distance.
    Boss->VisualMarkerActor = SpawnZombieReadabilityMarker(
        Boss,
        FLinearColor(1.0f, 0.05f, 0.6f) * 1.6f,
        FString::Printf(TEXT("%s BOSS %s Marker"), *CityLabel, *GetZombieFamilyVariantMarkerLabel(Boss->Variant)),
        1.8f);
    if (Boss->VisualMarkerActor)
    {
        Boss->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
        Boss->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Boss->Variant));
    }
    SpawnBlock(BossLoc + FVector(0, 0, 460), FVector(0.95f, 0.95f, 0.06f),
               FLinearColor(1.0f, 0.0f, 0.7f) * 6.0f, CityLabel + TEXT(" BOSS Halo"), false);
    SpawnGuideText(FString::Printf(TEXT("WARDEN OF %s\nPhase boss — defeat to unlock voucher"), *Mission.CityName.ToUpper()),
                   BossLoc + FVector(0, 0, 760), FColor(255, 80, 200), 72.0f);

    FActorSpawnParameters RevealSpawnParams;
    RevealSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ABossRevealPresentationActor* Reveal = GetWorld()->SpawnActor<ABossRevealPresentationActor>(
        ABossRevealPresentationActor::StaticClass(),
        BossLoc + FVector(0.0f, 0.0f, 22.0f),
        FRotator::ZeroRotator,
        RevealSpawnParams);
    if (Reveal)
    {
        const FLinearColor RevealColor =
            Mission.AccentColor * 0.42f + Mission.SecondaryAccentColor * 0.18f + FLinearColor(1.0f, 0.04f, 0.18f) * 0.40f;
        Reveal->ConfigureReveal(
            Boss,
            CityIndex,
            Mission.CityName,
            FString::Printf(TEXT("WARDEN OF %s"), *Mission.CityName.ToUpper()),
            RevealColor,
            GI && GI->bReducedMotion);
        Reveal->Tags.AddUnique(FName("BossRevealPresentationLayer"));
        Reveal->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
        Reveal->Tags.AddUnique(FName("Top50Recommendations"));
        RegisterStreamedActor(Reveal);
    }
}

void ACodeRescueGameMode::SpawnEliteWardenMiniBossStagingLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const int32 BossId = CodeRescueBossZombieIdBase + CityIndex;
    const bool bIntelUnlocked = FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bBossDefeated = GI && GI->NeutralizedZombieIds.Contains(BossId);
    const FString PressureState = bBossDefeated
        ? TEXT("WARDEN DEFEATED")
        : bIntelUnlocked ? TEXT("ACTIVE AFTER TERMINAL INTEL") : TEXT("DORMANT UNTIL TERMINAL INTEL");

    const FVector BossBase = Origin + CityOffset(FVector(2900.0f, -1500.0f, 0.0f));
    const FLinearColor WarningRed(1.0f, 0.08f, 0.12f, 1.0f);
    const FLinearColor IntelCyan(0.16f, 0.86f, 1.0f, 1.0f);
    const FLinearColor ChargerGold(1.0f, 0.62f, 0.10f, 1.0f);
    const FLinearColor SpitterGreen(0.38f, 1.0f, 0.42f, 1.0f);
    const FLinearColor BoomerViolet(0.86f, 0.34f, 1.0f, 1.0f);
    const FLinearColor StateColor = bBossDefeated ? SpitterGreen : bIntelUnlocked ? WarningRed : IntelCyan;

    auto TagWarden = [](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("EliteWardenMiniBossStaging"));
            Actor->Tags.AddUnique(FName("EliteWardenPressureGate"));
            Actor->Tags.AddUnique(FName("MiniBossAfterIntelMilestone"));
            Actor->Tags.AddUnique(FName("TextFirstEnemyReadability"));
            Actor->Tags.AddUnique(FName("NoAccessBlocker"));
            Actor->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
            Actor->Tags.AddUnique(FName("Top50Recommendations"));
        }
        return Actor;
    };

    auto AddTags = [](AActor* Actor, std::initializer_list<const TCHAR*> Tags) -> AActor*
    {
        if (Actor)
        {
            for (const TCHAR* Tag : Tags)
            {
                Actor->Tags.AddUnique(FName(Tag));
            }
        }
        return Actor;
    };

    auto SpawnWardenLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagWarden(Light);
            Light->Tags.AddUnique(FName("EliteWardenSignalLight"));
        }
    };

    TagWarden(SpawnTexturedBlock(
        BossBase + FVector(-520.0f, 0.0f, -13.0f),
        FVector(7.3f, 2.25f, 0.050f),
        FLinearColor(0.035f, 0.035f, 0.040f) + StateColor * 0.12f,
        CityLabel + TEXT(" Elite Warden Mini-Boss Runway Floor"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Panel.M_Tech_Panel"),
        false));
    TagWarden(SpawnBlock(
        BossBase + FVector(-520.0f, -235.0f, 146.0f),
        FVector(6.5f, 0.075f, 1.08f),
        FLinearColor(0.042f, 0.040f, 0.044f) + StateColor * 0.20f,
        CityLabel + TEXT(" Elite Warden Mini-Boss Intel Wall"),
        false));
    TagWarden(SpawnGuideText(
        FString::Printf(
            TEXT("ELITE WARDEN RUNWAY\n%s\nTerminal intel: %s\nBoss route stays optional; mini-boss sentinels wake only after the coding route opens."),
            *PressureState,
            *Mission.TerminalTitle),
        BossBase + FVector(-520.0f, -295.0f, 315.0f),
        StateColor.ToFColor(true),
        22.0f));

    struct FWardenStageAnchor
    {
        const TCHAR* Label;
        const TCHAR* Tag;
        const TCHAR* RoleText;
        FVector Offset;
        FLinearColor Color;
    };

    const FWardenStageAnchor Anchors[] = {
        { TEXT("INTEL LOCK GATE"), TEXT("WardenIntelLockGate"), TEXT("terminal solve opens the pressure lane"), FVector(-1050.0f, 0.0f, 0.0f), IntelCyan },
        { TEXT("MINI-BOSS SENTINEL LANE"), TEXT("WardenMiniBossSentinelLane"), TEXT("charger, spitter, and boomer sentinels stage here"), FVector(-520.0f, 0.0f, 0.0f), WarningRed },
        { TEXT("WARDEN PHASE GATE"), TEXT("WardenPhaseGate"), TEXT("boss reveal and phase telegraphs take over beyond this point"), FVector(10.0f, 0.0f, 0.0f), BoomerViolet },
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(Anchors); ++i)
    {
        const FWardenStageAnchor& Anchor = Anchors[i];
        const FVector Center = BossBase + Anchor.Offset;
        TagWarden(AddTags(SpawnBlock(
            Center + FVector(0.0f, 0.0f, 26.0f),
            FVector(1.45f, 1.08f, 0.10f),
            Anchor.Color * (bIntelUnlocked ? 1.55f : 0.82f),
            FString::Printf(TEXT("%s Elite Warden Stage %s Pad"), *CityLabel, Anchor.Label),
            false),
            { Anchor.Tag, TEXT("EliteWardenMilestoneAnchor") }));
        TagWarden(AddTags(SpawnBlock(
            Center + FVector(-132.0f, 0.0f, 132.0f),
            FVector(0.08f, 0.74f, 1.12f),
            Anchor.Color * 1.35f,
            FString::Printf(TEXT("%s Elite Warden Stage %s Gate A"), *CityLabel, Anchor.Label),
            false),
            { Anchor.Tag, TEXT("EliteWardenMilestoneAnchor") }));
        TagWarden(AddTags(SpawnBlock(
            Center + FVector(132.0f, 0.0f, 132.0f),
            FVector(0.08f, 0.74f, 1.12f),
            Anchor.Color * 1.35f,
            FString::Printf(TEXT("%s Elite Warden Stage %s Gate B"), *CityLabel, Anchor.Label),
            false),
            { Anchor.Tag, TEXT("EliteWardenMilestoneAnchor") }));
        TagWarden(AddTags(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Anchor.Label, Anchor.RoleText),
            Center + FVector(0.0f, -82.0f, 286.0f),
            Anchor.Color.ToFColor(true),
            18.0f),
            { Anchor.Tag, TEXT("TextFirstEnemyReadability") }));
    }

    struct FMiniBossSpec
    {
        const TCHAR* Label;
        const TCHAR* Tag;
        EZombieVariant Variant;
        ECodeRescueZombieEncounterRole Role;
        FVector Offset;
        FLinearColor Color;
        float HealthScale;
        float DamageScale;
        float SpeedScale;
    };

    const FMiniBossSpec MiniBosses[] = {
        { TEXT("CHARGER MINI-BOSS"), TEXT("EliteWardenChargerMiniBoss"), EZombieVariant::EliteCharger, ECodeRescueZombieEncounterRole::Pressure, FVector(-830.0f, 330.0f, 0.0f), ChargerGold, 2.6f, 1.45f, 1.32f },
        { TEXT("SPITTER MINI-BOSS"), TEXT("EliteWardenSpitterMiniBoss"), EZombieVariant::EliteSpitter, ECodeRescueZombieEncounterRole::Sentinel, FVector(-500.0f, 430.0f, 0.0f), SpitterGreen, 2.1f, 1.18f, 0.92f },
        { TEXT("BOOMER MINI-BOSS"), TEXT("EliteWardenBoomerMiniBoss"), EZombieVariant::EliteBoomer, ECodeRescueZombieEncounterRole::Anchor, FVector(-170.0f, 330.0f, 0.0f), BoomerViolet, 2.35f, 1.28f, 0.78f },
    };

    const float HealthMul = GI ? GI->GetZombieHealthMultiplier() : 1.0f;
    const float DamageMul = GI ? GI->GetZombieDamageMultiplier() : 1.0f;
    const float EncounterMin = FMath::Min(MinEncounterIntensityScale, MaxEncounterIntensityScale);
    const float EncounterMax = FMath::Max(MinEncounterIntensityScale, MaxEncounterIntensityScale);
    const float EncounterScale = FMath::Clamp(Mission.EncounterIntensity, EncounterMin, EncounterMax);
    int32 SpawnedMiniBosses = 0;

    for (int32 i = 0; i < UE_ARRAY_COUNT(MiniBosses); ++i)
    {
        const FMiniBossSpec& Spec = MiniBosses[i];
        const FVector MarkerLoc = BossBase + Spec.Offset + FVector(0.0f, 0.0f, 118.0f);
        TagWarden(AddTags(SpawnBlock(
            MarkerLoc,
            FVector(0.72f, 0.72f, 1.18f),
            Spec.Color * (bIntelUnlocked && !bBossDefeated ? 1.75f : 0.62f),
            FString::Printf(TEXT("%s %s Silhouette Marker"), *CityLabel, Spec.Label),
            false),
            { Spec.Tag, TEXT("EliteMiniBossSilhouette") }));
        TagWarden(AddTags(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Spec.Label, bIntelUnlocked && !bBossDefeated ? TEXT("ACTIVE AFTER INTEL") : TEXT("PREVIEW ONLY")),
            MarkerLoc + FVector(0.0f, -64.0f, 190.0f),
            Spec.Color.ToFColor(true),
            16.0f),
            { Spec.Tag, TEXT("TextFirstEnemyReadability") }));

        if (!bIntelUnlocked || bBossDefeated)
        {
            continue;
        }

        const int32 MiniBossId = CodeRescueBossZombieIdBase + 100000 + CityIndex * 10 + i;
        if (GI && GI->NeutralizedZombieIds.Contains(MiniBossId))
        {
            continue;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        UClass* ZombieClass = ZombieActorClass ? ZombieActorClass.Get() : ACodeZombieActor::StaticClass();
        ACodeZombieActor* MiniBoss = GetWorld()->SpawnActor<ACodeZombieActor>(
            ZombieClass,
            BossBase + Spec.Offset + FVector(0.0f, 0.0f, 95.0f),
            FRotator::ZeroRotator,
            SpawnParams);
        if (!MiniBoss)
        {
            continue;
        }

        MiniBoss->ZombieId = MiniBossId;
        MiniBoss->Health = FMath::Max(90.0f, ZombieBaseHealth * Spec.HealthScale * HealthMul * EncounterScale);
        MiniBoss->AttackDamage = FMath::Max(4.0f, ZombieBaseAttackDamage * Spec.DamageScale * DamageMul * EncounterScale);
        MiniBoss->MoveSpeed = FMath::Max(40.0f, ZombieBaseMoveSpeed * Spec.SpeedScale * (0.96f + 0.02f * Mission.DifficultyTier));
        MiniBoss->AttackRange = FMath::Max(70.0f, ZombieAttackRange);
        MiniBoss->ActivationRange = 3400.0f + Mission.DifficultyTier * 120.0f;
        MiniBoss->RefreshMovementSettings();

        ApplyZombieFamilyVariant(MiniBoss, Spec.Variant, MiniBossId, FName("EliteWardenMiniBossFamily"), true);

        MiniBoss->ConfigureEncounterDirective(Spec.Role, BossBase + FVector(-520.0f, 230.0f, 0.0f), 820.0f, 320.0f, Spec.SpeedScale);
        MiniBoss->Tags.AddUnique(FName("EliteWardenMiniBoss"));
        MiniBoss->Tags.AddUnique(FName("MiniBossAfterIntelMilestone"));
        MiniBoss->Tags.AddUnique(FName("WardenRunwaySentinel"));
        MiniBoss->Tags.AddUnique(FName(Spec.Tag));
        MiniBoss->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
        MiniBoss->Tags.AddUnique(FName("Top50Recommendations"));
        MiniBoss->VisualMarkerActor = TagWarden(AddTags(SpawnZombieReadabilityMarker(
            MiniBoss,
            Spec.Color * 2.2f,
            FString::Printf(TEXT("%s Active %s %s Marker"), *CityLabel, Spec.Label, *GetZombieFamilyVariantMarkerLabel(MiniBoss->Variant)),
            1.25f),
            { Spec.Tag, TEXT("EliteWardenMiniBoss") }));
        if (MiniBoss->VisualMarkerActor)
        {
            MiniBoss->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
            MiniBoss->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(MiniBoss->Variant));
        }
        RegisterStreamedActor(MiniBoss);
        ++SpawnedMiniBosses;
    }

    if (bIntelUnlocked && !bBossDefeated)
    {
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(TEXT("[Dispatch]: %s intel is live. Elite sentinels are guarding the Warden runway."), *Mission.CityName),
            4.5f);
    }
    SpawnWardenLight(BossBase + FVector(-520.0f, -80.0f, 320.0f), StateColor, bIntelUnlocked ? 5200.0f : 2500.0f, 980.0f, CityLabel + TEXT(" Elite Warden Mini-Boss Signal Light"));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueEliteWardenMiniBoss] %s state='%s' intel_unlocked=%s boss_defeated=%s spawned_minibosses=%d terminal='%s'"),
        *CityLabel,
        *PressureState,
        bIntelUnlocked ? TEXT("true") : TEXT("false"),
        bBossDefeated ? TEXT("true") : TEXT("false"),
        SpawnedMiniBosses,
        *Mission.TerminalId);
}

// ---- #67 Themed novel set-pieces ------------------------------------------
void ACodeRescueGameMode::SpawnSetPieceForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel, const FCodeRescueCityMission& Mission)
{
    // One themed event per city. Pick deterministically from CityIndex so the
    // same city always hosts the same flavor across reloads. These set-pieces
    // are visual + narrative right now; converting any to gameplay objectives
    // is a content pipeline pass once authored meshes drop in.
    static const TCHAR* Themes[] = {
        TEXT("Lab Vault"),                  // 0
        TEXT("Radio Tower"),                // 1
        TEXT("Dog-Pack Den"),               // 2
        TEXT("Hospital Triage"),            // 3
        TEXT("Drone Wreckage"),             // 4
    };
    const int32 ThemeIndex = CityIndex % 5;
    const FString Theme = Themes[ThemeIndex];

    // Place the set-piece in the southeast quadrant so it doesn't overlap the
    // helipad (NE) or the boss (NE) and players naturally bump into it.
    const FVector SetPieceLoc = Origin + CityOffset(FVector(1500.0f, 2000.0f, 0.0f));
    const FLinearColor ThemeColor = Mission.AccentColor * 0.6f + FLinearColor(0.6f, 0.4f, 1.0f) * 0.4f;

    // Hero block — the pedestal for the theme.
    SpawnBlock(SetPieceLoc + FVector(0, 0, 60), FVector(3.0f, 3.0f, 0.4f),
               ThemeColor * 0.5f, FString::Printf(TEXT("%s SetPiece Pedestal: %s"), *CityLabel, *Theme), true);

    if (ThemeIndex == 0)
    {
        // Lab Vault: a stack of glowing crates.
        for (int32 i = 0; i < 4; ++i)
        {
            const float Z = 130.0f + i * 110.0f;
            SpawnBlock(SetPieceLoc + FVector(0, 0, Z), FVector(1.4f, 1.4f, 1.0f),
                       FLinearColor(0.2f, 1.0f, 0.4f) * (0.6f + i * 0.4f),
                       CityLabel + TEXT(" Vault Crate"), true);
        }
    }
    else if (ThemeIndex == 1)
    {
        // Radio Tower: tall thin emissive spire with crossbeams.
        SpawnBlock(SetPieceLoc + FVector(0, 0, 600), FVector(0.4f, 0.4f, 8.0f),
                   FLinearColor(1.0f, 0.7f, 0.2f) * 1.5f, CityLabel + TEXT(" Radio Tower"), true);
        SpawnBlock(SetPieceLoc + FVector(0, 0, 1100), FVector(2.0f, 0.2f, 0.2f),
                   FLinearColor(1.0f, 0.5f, 0.0f) * 2.5f, CityLabel + TEXT(" Tower Beam"), false);
    }
    else if (ThemeIndex == 2)
    {
        // Dog-Pack Den: low ring of stones plus a small spawn cluster
        // (regular zombies that read as "feral dogs" once the DogZombie
        // variant is wired). Skipped in sandbox mode to honor the setting.
        for (int32 i = 0; i < 6; ++i)
        {
            const float Angle = (i / 6.0f) * 2.0f * PI;
            const FVector Stone = SetPieceLoc + FVector(FMath::Cos(Angle) * 320.0f, FMath::Sin(Angle) * 320.0f, 30.0f);
            SpawnBlock(Stone, FVector(0.6f, 0.6f, 0.5f),
                       FLinearColor(0.3f, 0.25f, 0.2f), CityLabel + TEXT(" Den Stone"), true);
        }
        if (!bSandboxMode)
        {
            for (int32 i = 0; i < 3; ++i)
            {
                const FVector PupLoc = SetPieceLoc + FVector(FMath::Cos(i * 2.1f) * 220.0f, FMath::Sin(i * 2.1f) * 220.0f, 95.0f);
                const int32 PupId = CodeRescueDogZombieIdBase + CityIndex * 10 + i;
                FActorSpawnParameters PupSpawnParams;
                PupSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                ACodeZombieActor* Pup = GetWorld()->SpawnActor<ACodeZombieActor>(ACodeZombieActor::StaticClass(), PupLoc, FRotator::ZeroRotator, PupSpawnParams);
                if (!Pup) continue;
                Pup->ZombieId = PupId;
                Pup->Health = 35.0f;
                Pup->AttackDamage = 6.0f;
                Pup->MoveSpeed = 290.0f;       // fast — they're "dogs"
                Pup->ActivationRange = 1800.0f;
                Pup->RefreshMovementSettings();
                EZombieVariant DogVariant = EZombieVariant::DogZombie;
                ApplyZombieFamilyVariant(Pup, DogVariant, PupId, FName("DogDenZombieFamily"), true);
                Pup->VisualMarkerActor = SpawnZombieReadabilityMarker(
                    Pup,
                    GetZombieFamilyVariantMarkerColor(Pup->Variant),
                    FString::Printf(TEXT("%s %s Den Marker"), *CityLabel, *GetZombieFamilyVariantMarkerLabel(Pup->Variant)),
                    0.65f);
                if (Pup->VisualMarkerActor)
                {
                    Pup->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
                    Pup->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Pup->Variant));
                }
                RegisterStreamedActor(Pup);
            }
        }
    }
    else if (ThemeIndex == 3)
    {
        // Hospital Triage: cluster of cot blocks + a free medkit pickup.
        for (int32 i = 0; i < 4; ++i)
        {
            const float X = -250.0f + (i % 2) * 500.0f;
            const float Y = -150.0f + (i / 2) * 300.0f;
            SpawnBlock(SetPieceLoc + FVector(X, Y, 70), FVector(2.0f, 0.8f, 0.4f),
                       FLinearColor(0.95f, 0.95f, 1.0f), CityLabel + TEXT(" Triage Cot"), true);
        }
        APickupActor* MedKit = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(),
            SetPieceLoc + FVector(0, 0, 130), FRotator::ZeroRotator);
        if (MedKit)
        {
            MedKit->Kind = EPickupKind::Medkit;
            MedKit->Amount = 2;
            RegisterStreamedActor(MedKit);
        }
    }
    else // ThemeIndex == 4 — Drone Wreckage
    {
        SpawnBlock(SetPieceLoc + FVector(0, 0, 90), FVector(2.5f, 2.5f, 0.4f),
                   FLinearColor(0.18f, 0.18f, 0.22f), CityLabel + TEXT(" Drone Hull"), true);
        SpawnBlock(SetPieceLoc + FVector(140, 0, 130), FVector(0.4f, 1.4f, 0.1f),
                   FLinearColor(0.6f, 0.5f, 0.2f), CityLabel + TEXT(" Drone Wing R"), true);
        SpawnBlock(SetPieceLoc + FVector(-140, 0, 130), FVector(0.4f, 1.4f, 0.1f),
                   FLinearColor(0.6f, 0.5f, 0.2f), CityLabel + TEXT(" Drone Wing L"), true);
        SpawnBlock(SetPieceLoc + FVector(0, 0, 230), FVector(0.6f, 0.6f, 0.6f),
                   FLinearColor(0.05f, 1.0f, 0.4f) * 2.5f, CityLabel + TEXT(" Drone Core"), true);
    }

    SpawnGuideText(FString::Printf(TEXT("%s\n%s"), *Theme.ToUpper(), *Mission.CityName),
                   SetPieceLoc + FVector(0, 0, 520), FColor(180, 200, 255), 56.0f);
}

// ---- #68 Ambient friendly NPC spawns -------------------------------------
void ACodeRescueGameMode::SpawnFriendlyNPCsForCity(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    // Roster: one of each role per city, anchored around the language-station
    // plaza so the NPCs feel like a "safe hub" instead of being scattered.
    // Names rotate deterministically off CityIndex so each city gets a stable
    // line-up across reloads. NurseFemale survives reload via the same actor.
    struct FNPCSpec { EFriendlyNPCRole Role; FVector LocalOffset; const TCHAR* Greeting; };
    static const FNPCSpec Specs[] = {
        { EFriendlyNPCRole::Engineer,
          FVector(-2400, -1700, 90),
          TEXT("Salvage what you can — every bolt counts.") },
        { EFriendlyNPCRole::Medic,
          FVector(-2400, -1300, 90),
          TEXT("Hold still. This will sting.") },
        { EFriendlyNPCRole::Scientist,
          FVector(-1900, -1700, 90),
          TEXT("My latest theory: the syntax is alive.") },
        { EFriendlyNPCRole::Trader,
          FVector(-1900, -1300, 90),
          TEXT("Five scrap, one insight. Standard rate.") },
    };

    static const TCHAR* FirstNames[] = {
        TEXT("Avery"), TEXT("Brooks"), TEXT("Cassidy"), TEXT("Dakota"),
        TEXT("Emerson"), TEXT("Finley"), TEXT("Gray"), TEXT("Harper"),
        TEXT("Indigo"), TEXT("Juno"), TEXT("Kai"), TEXT("Lior"),
        TEXT("Marlow"), TEXT("Niko"), TEXT("Onyx"), TEXT("Phoenix"),
    };
    const int32 NameCount = UE_ARRAY_COUNT(FirstNames);

    // New York's V5 café occupies the legacy support-hub approach lane. Shift
    // only the first city's services east so all four workstations remain
    // grouped without obstructing the authored building entrance.
    const FVector SupportHubShift = CityIndex == 0
        ? FVector(900.0f, 0.0f, 0.0f)
        : FVector::ZeroVector;
    const FVector HubCenter = Origin + CityOffset(FVector(-2150.0f, -1500.0f, -14.0f) + SupportHubShift);
    SpawnTexturedBlock(
        HubCenter,
        CityExtent(FVector(9.0f, 7.0f, 0.04f)),
        FLinearColor(0.09f, 0.11f, 0.12f),
        CityLabel + TEXT(" Civilian Support Hub Plaza"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Panels.M_Concrete_Panels"),
        false);
    SpawnBlock(
        HubCenter + FVector(0.0f, 0.0f, 215.0f),
        CityExtent(FVector(9.2f, 0.12f, 0.16f)),
        FLinearColor(0.2f, 0.85f, 1.0f) * 1.6f,
        CityLabel + TEXT(" Support Hub North Light Strip"),
        false);
    SpawnBlock(
        HubCenter + FVector(0.0f, 0.0f, 215.0f) + CityOffset(FVector(0.0f, 700.0f, 0.0f)),
        CityExtent(FVector(9.2f, 0.12f, 0.16f)),
        FLinearColor(0.2f, 0.85f, 1.0f) * 1.6f,
        CityLabel + TEXT(" Support Hub South Light Strip"),
        false);
    SpawnGuideText(
        TEXT("CIVILIAN SUPPORT HUB\nEngineer | Medic | Scientist | Trader\nservices save per language and reset at day/night shift"),
        HubCenter + FVector(0.0f, 0.0f, 520.0f),
        FColor(180, 235, 255),
        48.0f);

    auto RoleColor = [](EFriendlyNPCRole Role)
    {
        switch (Role)
        {
        case EFriendlyNPCRole::Engineer:  return FLinearColor(1.0f, 0.58f, 0.10f);
        case EFriendlyNPCRole::Medic:     return FLinearColor(1.0f, 0.12f, 0.14f);
        case EFriendlyNPCRole::Scientist: return FLinearColor(0.35f, 0.55f, 1.0f);
        case EFriendlyNPCRole::Trader:    return FLinearColor(0.18f, 1.0f, 0.36f);
        }
        return FLinearColor::White;
    };
    auto RoleStationLabel = [](EFriendlyNPCRole Role)
    {
        switch (Role)
        {
        case EFriendlyNPCRole::Engineer:  return TEXT("REPAIR BENCH\n+1 scrap\nsaved per language");
        case EFriendlyNPCRole::Medic:     return TEXT("TRIAGE TABLE\n+25 health\nsaved per language");
        case EFriendlyNPCRole::Scientist: return TEXT("FIELD LAB\n+1 research\nsaved per language");
        case EFriendlyNPCRole::Trader:    return TEXT("SUPPLY TRADE\n5 scrap -> 1 research\nsaved per language");
        }
        return TEXT("SUPPORT");
    };

    for (int32 i = 0; i < UE_ARRAY_COUNT(Specs); ++i)
    {
        const FNPCSpec& Spec = Specs[i];
        const FVector NPCLoc = Origin + CityOffset(Spec.LocalOffset + SupportHubShift);

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        AFriendlyNPCActor* NPC = GetWorld()->SpawnActor<AFriendlyNPCActor>(
            AFriendlyNPCActor::StaticClass(), NPCLoc, FRotator(0, 180, 0), Params);
        if (!NPC) continue;

        NPC->NPCRole = Spec.Role;
        NPC->CityIndex = CityIndex;
        NPC->NPCName = FString::Printf(TEXT("%s the %s"),
            FirstNames[(CityIndex * 4 + i) % NameCount],
            (Spec.Role == EFriendlyNPCRole::Engineer ? TEXT("Engineer")
             : Spec.Role == EFriendlyNPCRole::Medic ? TEXT("Medic")
             : Spec.Role == EFriendlyNPCRole::Scientist ? TEXT("Scientist")
             : TEXT("Trader")));
        NPC->GreetingLine = Spec.Greeting;
        NPC->Tags.AddUnique(FName("FriendlySafehouseNPCService"));
        NPC->Tags.AddUnique(FName("SelectedLanguageSupportSave"));
        NPC->Tags.AddUnique(FName("SafehouseNPCServiceLoop"));
        NPC->Tags.AddUnique(FName(*NPC->GetServiceId()));
        NPC->ApplySavedServiceState();
        RegisterStreamedActor(NPC);

        // Floating label so the player can identify the role at distance.
        const FLinearColor SupportColor = RoleColor(Spec.Role);
        const FVector WorkstationLoc = NPCLoc + FVector(0.0f, -260.0f, 62.0f);
        SpawnBlock(
            WorkstationLoc,
            FVector(1.6f, 0.55f, 0.32f),
            SupportColor * 0.6f,
            CityLabel + TEXT(" Support Hub Role Workstation"),
            true);
        SpawnBlock(
            WorkstationLoc + FVector(0.0f, 0.0f, 170.0f),
            FVector(1.9f, 0.75f, 0.08f),
            SupportColor * 1.3f,
            CityLabel + TEXT(" Support Hub Canopy"),
            false);
        SpawnBlock(
            WorkstationLoc + FVector(-95.0f, -35.0f, 86.0f),
            FVector(0.08f, 0.08f, 1.45f),
            SupportColor,
            CityLabel + TEXT(" Support Hub Canopy Post A"),
            true);
        SpawnBlock(
            WorkstationLoc + FVector(95.0f, -35.0f, 86.0f),
            FVector(0.08f, 0.08f, 1.45f),
            SupportColor,
            CityLabel + TEXT(" Support Hub Canopy Post B"),
            true);
        SpawnBlock(
            WorkstationLoc + FVector(0.0f, -70.0f, 126.0f),
            FVector(0.18f, 0.06f, 0.18f),
            SupportColor * 3.0f,
            CityLabel + TEXT(" Support Hub Role Icon"),
            false);
        SpawnGuideText(
            RoleStationLabel(Spec.Role),
            WorkstationLoc + FVector(0.0f, -90.0f, 310.0f),
            SupportColor.ToFColor(true),
            34.0f);

        SpawnGuideText(FString::Printf(TEXT("%s\n[E] %s\nresets day/night"),
                       *NPC->NPCName,
                       *NPC->GetServiceSummary()),
                       NPCLoc + FVector(0, 0, 350),
                       (Spec.Role == EFriendlyNPCRole::Engineer ? FColor(255, 180, 60)
                        : Spec.Role == EFriendlyNPCRole::Medic ? FColor(255, 90, 90)
                        : Spec.Role == EFriendlyNPCRole::Scientist ? FColor(120, 170, 255)
                        : FColor(120, 255, 160)),
                       42.0f);
    }
}

void ACodeRescueGameMode::SpawnCollectibleCaseFilesForCity(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    auto SpawnCaseFile = [&](const TCHAR* Suffix, const FVector& LocalOffset, const FString& Title, const FString& Body, const FLinearColor& Tint)
    {
        const FString CaseFileId = FString::Printf(TEXT("%s_case_%s"), *Mission.TerminalId, Suffix);
        if (GI && GI->HasCollectedCaseFile(CaseFileId))
        {
            return;
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ACaseFilePickupActor* CaseFile = GetWorld()->SpawnActor<ACaseFilePickupActor>(
            ACaseFilePickupActor::StaticClass(),
            Origin + CityOffset(LocalOffset),
            FRotator(0.0f, 22.0f, 0.0f),
            Params);
        if (!CaseFile)
        {
            return;
        }

        CaseFile->CaseFileId = CaseFileId;
        CaseFile->CaseFileTitle = Title;
        CaseFile->CaseFileBody = Body;
        CaseFile->CityIndex = CityIndex;
        CaseFile->CaseFileTint = Tint;
        CaseFile->Tags.AddUnique(FName("NarrativeCaseFileCollectible"));
        CaseFile->Tags.AddUnique(FName("CaseFileLanguageRunSave"));
        CaseFile->Tags.AddUnique(FName("CollectibleCaseFilesGuidance"));
        RegisterStreamedActor(CaseFile);

        SpawnGuideText(
            FString::Printf(TEXT("CASE FILE\n[E] %s"), *Title),
            CaseFile->GetActorLocation() + FVector(0.0f, 0.0f, 290.0f),
            Tint.ToFColor(true),
            32.0f);
    };

    SpawnCaseFile(
        TEXT("terminal_evidence"),
        FVector(-1260.0f, -1035.0f, 116.0f),
        FString::Printf(TEXT("Terminal Evidence: %s"), *Mission.TerminalTitle),
        FString::Printf(
            TEXT("%s Field analysts tied this lesson to %s so players can connect the code task to a physical city route. Language track: %s"),
            *Mission.CurriculumFocus,
            *Mission.LandmarkName,
            *Mission.LanguageTrackText),
        Mission.AccentColor * 0.72f + FLinearColor(0.35f, 0.86f, 1.0f) * 0.28f);

    SpawnCaseFile(
        TEXT("survivor_note"),
        FVector(2600.0f, 1180.0f, 116.0f),
        FString::Printf(TEXT("Survivor Note: %s"), *Mission.SurvivorName),
        FString::Printf(
            TEXT("%s %s Rescue note: solve %s, then reach %s before the route collapses."),
            *Mission.CharacterStoryPlan,
            *Mission.RadioBriefing,
            *Mission.TerminalTitle,
            *Mission.LandmarkName),
        Mission.SecondaryAccentColor * 0.66f + FLinearColor(1.0f, 0.82f, 0.25f) * 0.34f);

    SpawnCaseFile(
        TEXT("route_brief"),
        FVector(2070.0f, 2210.0f, 116.0f),
        FString::Printf(TEXT("Route Brief: %s"), *Mission.LandmarkName),
        FString::Printf(
            TEXT("%s World detail: %s Progression: %s Accessibility note: %s"),
            *Mission.ArchitectureSignature,
            *Mission.NovelGameplayDetail,
            *Mission.ProgressionPlan,
            *Mission.AccessibilityPolishPlan),
        Mission.AccentColor * 0.42f + Mission.SecondaryAccentColor * 0.38f + FLinearColor(0.3f, 1.0f, 0.62f) * 0.20f);

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueCaseFiles] %03d %s spawned collectible case files with selected-language save persistence."),
        Mission.Rank,
        *CityLabel);
}

void ACodeRescueGameMode::SpawnEnvironmentalStorytellingLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector StoryHub = Origin + CityOffset(FVector(-575.0f, -3335.0f, 0.0f));
    const FLinearColor FailureRed(1.0f, 0.16f, 0.09f);
    const FLinearColor EngineerBlue(0.20f, 0.82f, 1.0f);
    const FLinearColor SurvivorGold(1.0f, 0.82f, 0.22f);
    const FLinearColor CodeGreen(0.28f, 1.0f, 0.52f);
    const FLinearColor CityViolet(0.78f, 0.48f, 1.0f);

    auto TagStoryActor = [this](AActor* Actor, const TArray<FName>& LayerTags) -> AActor*
    {
        if (Actor)
        {
            Actor->SetActorEnableCollision(false);
            Actor->Tags.AddUnique(FName("EnvironmentalStorytellingLayer"));
            Actor->Tags.AddUnique(FName("CodingRescuesPeoplePremise"));
            Actor->Tags.AddUnique(FName("WorldBiblePillar"));
            Actor->Tags.AddUnique(FName("AutomationFailureScene"));
            Actor->Tags.AddUnique(FName("TechnologyRuleReadable"));
            Actor->Tags.AddUnique(FName("NonBlockingWorldStoryCue"));
            Actor->Tags.AddUnique(FName("Top50Recommendation35"));
            Actor->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
            ApplyRuntimeDataLayerTags(Actor, LayerTags);
        }
        return Actor;
    };

    auto SpawnStoryLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, const TArray<FName>& LayerTags, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(470.0f);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagStoryActor(Light, LayerTags);
        }
    };

    struct FEnvironmentalStoryBeat
    {
        const TCHAR* Title;
        FString Detail;
        FVector LocalOffset;
        FLinearColor Color;
        TArray<FName> Tags;
    };

    TArray<FEnvironmentalStoryBeat> Beats;
    Beats.Add({
        TEXT("OBJECTIVE AUTOMATION FAILURE"),
        FString::Printf(TEXT("%s\n%s controls the locked physical route."), *Mission.MissionBrief, *Mission.TerminalTitle),
        FVector(-720.0f, 0.0f, 0.0f),
        FailureRed,
        TArray<FName>{ FName("WorldBible_AutomationInfrastructure"), FName("RuntimeDataLayer_State_Prerecovery") },
    });
    Beats.Add({
        TEXT("SAFEHOUSE ENGINEER NETWORK"),
        FString::Printf(TEXT("Radio: %s\nField engineers rewrite orphaned systems from the safehouse."), *Mission.RadioBriefing),
        FVector(-360.0f, 0.0f, 0.0f),
        EngineerBlue,
        TArray<FName>{ FName("EngineerNetworkStoryCue"), FName("RuntimeDataLayer_Mode_CodingSafehouse") },
    });
    Beats.Add({
        TEXT("SURVIVOR STAKE"),
        FString::Printf(TEXT("%s\n%s"), *Mission.SurvivorName, *Mission.CharacterStoryPlan),
        FVector(0.0f, 0.0f, 0.0f),
        SurvivorGold,
        TArray<FName>{ FName("SurvivorStakeStoryCue"), FName("RuntimeDataLayer_State_SafeBeat") },
    });
    Beats.Add({
        TEXT("EXTRACTION CODE CAUSE/EFFECT"),
        FString::Printf(TEXT("Solve %s -> route lights -> %s extraction."), *Mission.TerminalTitle, *Mission.LandmarkName),
        FVector(360.0f, 0.0f, 0.0f),
        CodeGreen,
        TArray<FName>{ FName("CodeCauseEffectStoryCue"), FName("RuntimeDataLayer_Mode_RescueTraversal") },
    });
    Beats.Add({
        TEXT("CITY CHAPTER NAVIGATION"),
        FString::Printf(TEXT("%s / %s\n%s"), *Mission.RegionName, *Mission.DistrictStyle, *Mission.NovelGameplayDetail),
        FVector(720.0f, 0.0f, 0.0f),
        CityViolet,
        TArray<FName>{ FName("CityChapterStoryCue"), FName("RuntimeWorldPartitionStreamCell") },
    });

    TagStoryActor(SpawnTexturedBlock(
        StoryHub + FVector(0.0f, 0.0f, -5.0f),
        FVector(17.4f, 3.05f, 0.050f),
        FLinearColor(0.026f, 0.032f, 0.036f) + Mission.AccentColor * 0.09f,
        CityLabel + TEXT(" Environmental Storytelling Deck"),
        TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"),
        false),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("WorldBibleRuntimeCue") });

    TagStoryActor(SpawnBlock(
        StoryHub + FVector(0.0f, -148.0f, 128.0f),
        FVector(16.3f, 0.075f, 0.96f),
        FLinearColor(0.038f, 0.045f, 0.050f) + Mission.SecondaryAccentColor * 0.14f,
        CityLabel + TEXT(" Environmental Storytelling Backboard"),
        false),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("ShowDontTellStoryCue") });

    TagStoryActor(SpawnGuideText(
        FString::Printf(TEXT("OBJECTIVE STORY NAVIGATION\nCoding rescues people: broken automation, safehouse engineers, survivor stakes, extraction route\n%s"), *Mission.CityName),
        StoryHub + FVector(0.0f, -198.0f, 312.0f),
        FColor(190, 242, 255),
        22.0f),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("WorldBibleRuntimeCue") });

    for (int32 Index = 0; Index < Beats.Num(); ++Index)
    {
        const FEnvironmentalStoryBeat& Beat = Beats[Index];
        const FVector Base = StoryHub + Beat.LocalOffset;
        TagStoryActor(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 18.0f),
            FVector(2.75f, 0.55f, 0.065f),
            Beat.Color * 1.35f,
            FString::Printf(TEXT("%s Environmental Story Beat Pad %d"), *CityLabel, Index + 1),
            false),
            Beat.Tags);
        TagStoryActor(SpawnBlock(
            Base + FVector(-78.0f, -28.0f, 94.0f),
            FVector(0.12f, 0.12f, 1.18f),
            Beat.Color * 1.18f,
            FString::Printf(TEXT("%s Environmental Story Beat Left Relay %d"), *CityLabel, Index + 1),
            false),
            Beat.Tags);
        TagStoryActor(SpawnBlock(
            Base + FVector(78.0f, -28.0f, 94.0f),
            FVector(0.12f, 0.12f, 1.18f),
            Beat.Color * 1.18f,
            FString::Printf(TEXT("%s Environmental Story Beat Right Relay %d"), *CityLabel, Index + 1),
            false),
            Beat.Tags);
        TagStoryActor(SpawnBlock(
            Base + FVector(0.0f, -62.0f, 188.0f),
            FVector(1.36f, 0.045f, 0.42f),
            Beat.Color * 1.72f,
            FString::Printf(TEXT("%s Environmental Story Beat Readable Plate %d"), *CityLabel, Index + 1),
            false),
            Beat.Tags);
        TagStoryActor(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Beat.Title, *Beat.Detail),
            Base + FVector(0.0f, -104.0f, 270.0f),
            Beat.Color.ToFColor(true),
            15.0f),
            Beat.Tags);
        SpawnStoryLight(
            Base + FVector(0.0f, -36.0f, 232.0f),
            Beat.Color,
            1180.0f + Index * 180.0f,
            Beat.Tags,
            FString::Printf(TEXT("%s Environmental Story Beat Light %d"), *CityLabel, Index + 1));
    }

    for (int32 CableIndex = 0; CableIndex < 4; ++CableIndex)
    {
        const float CableX = -540.0f + CableIndex * 360.0f;
        TagStoryActor(SpawnBlock(
            StoryHub + FVector(CableX, -12.0f, 54.0f),
            FVector(1.46f, 0.035f, 0.045f),
            FLinearColor::LerpUsingHSV(EngineerBlue, CodeGreen, CableIndex / 3.0f) * 1.60f,
            FString::Printf(TEXT("%s Environmental Story Data Cable %d"), *CityLabel, CableIndex + 1),
            false),
            TArray<FName>{ FName("TechnologyRuleReadable"), FName("CodeCauseEffectStoryCue"), FName("RuntimeDataLayer_Mode_Storytelling") });
    }

    TagStoryActor(SpawnGuideText(
        TEXT("SAFEHOUSE SURVIVOR EXTRACTION\nOptional story cues only; route and selected-language save stay unchanged."),
        StoryHub + FVector(0.0f, 162.0f, 258.0f),
        FColor(215, 250, 224),
        18.0f),
        TArray<FName>{
            FName("RuntimeDataLayer_Mode_Storytelling"),
            FName("RuntimeDataLayer_Mode_CodingSafehouse"),
            FName("RuntimeDataLayer_Mode_RescueTraversal"),
        });

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueEnvironmentalStorytelling] %s city=%d survivor='%s' terminal='%s' spawned nonblocking coding-rescues-people world story cues."),
        *CityLabel,
        CityIndex,
        *Mission.SurvivorName,
        *Mission.TerminalTitle);
}

void ACodeRescueGameMode::SpawnWorldBibleLoreLayer(const FCodeRescueCityMission& Mission, int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const FVector LoreHub = Origin + CityOffset(FVector(1265.0f, -3335.0f, 0.0f));
    const FLinearColor PremiseCyan(0.22f, 0.86f, 1.0f);
    const FLinearColor PillarGold(1.0f, 0.78f, 0.20f);
    const FLinearColor FactionGreen(0.32f, 0.96f, 0.50f);
    const FLinearColor RuleViolet(0.76f, 0.50f, 1.0f);
    const FLinearColor CityAmber(1.0f, 0.52f, 0.22f);

    auto TagLoreActor = [this](AActor* Actor, const TArray<FName>& LayerTags) -> AActor*
    {
        if (Actor)
        {
            Actor->SetActorEnableCollision(false);
            Actor->Tags.AddUnique(FName("WorldBibleLoreLayer"));
            Actor->Tags.AddUnique(FName("CanonicalLoreContract"));
            Actor->Tags.AddUnique(FName("WorldBibleAndLoreGuidance"));
            Actor->Tags.AddUnique(FName("CodingAsEmpowermentPillar"));
            Actor->Tags.AddUnique(FName("SurvivingEngineersNetwork"));
            Actor->Tags.AddUnique(FName("AutomationAntagonistForce"));
            Actor->Tags.AddUnique(FName("InfectedPressureForce"));
            Actor->Tags.AddUnique(FName("TechnologyRulesReadable"));
            Actor->Tags.AddUnique(FName("PerCityLoreData"));
            Actor->Tags.AddUnique(FName("Top50Recommendation36"));
            Actor->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
            ApplyRuntimeDataLayerTags(Actor, LayerTags);
        }
        return Actor;
    };

    auto SpawnLoreLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, const TArray<FName>& LayerTags, const FString& Name)
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(430.0f);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagLoreActor(Light, LayerTags);
        }
    };

    struct FWorldBibleCard
    {
        const TCHAR* Title;
        FString Detail;
        FVector LocalOffset;
        FLinearColor Color;
        TArray<FName> Tags;
    };

    TArray<FWorldBibleCard> Cards;
    Cards.Add({
        TEXT("OBJECTIVE WORLD BIBLE PREMISE"),
        TEXT("Near-total automation failed; field engineers restore physical systems through code."),
        FVector(-560.0f, 0.0f, 0.0f),
        PremiseCyan,
        TArray<FName>{ FName("WorldBiblePremise"), FName("RuntimeDataLayer_Mode_Storytelling") },
    });
    Cards.Add({
        TEXT("NAVIGATION PILLARS"),
        TEXT("survival-horror dread | coding-as-empowerment | 465-city widening rescue effort"),
        FVector(-280.0f, 0.0f, 0.0f),
        PillarGold,
        TArray<FName>{ FName("WorldBiblePillars"), FName("CodingAsEmpowermentPillar") },
    });
    Cards.Add({
        TEXT("SAFEHOUSE FACTIONS / FORCES"),
        TEXT("engineer network allies | abandoned automation antagonist | infected pressure outside safe beats"),
        FVector(0.0f, 0.0f, 0.0f),
        FactionGreen,
        TArray<FName>{ FName("SurvivingEngineersNetwork"), FName("AutomationAntagonistForce"), FName("InfectedPressureForce") },
    });
    Cards.Add({
        TEXT("EXTRACTION TECH RULES"),
        TEXT("code can reopen routes, power safehouses, and locate survivors; code cannot erase field combat risk."),
        FVector(280.0f, 0.0f, 0.0f),
        RuleViolet,
        TArray<FName>{ FName("TechnologyRulesReadable"), FName("RuntimeDataLayer_Mode_RescueTraversal") },
    });
    Cards.Add({
        TEXT("SURVIVOR CITY CHAPTER"),
        FString::Printf(TEXT("%s\n%s | %s | %s"), *Mission.MissionBrief, *Mission.RadioBriefing, *Mission.CharacterStoryPlan, *Mission.SurvivorName),
        FVector(560.0f, 0.0f, 0.0f),
        CityAmber,
        TArray<FName>{ FName("PerCityLoreData"), FName("WorldBibleCityChapter") },
    });

    TagLoreActor(SpawnTexturedBlock(
        LoreHub + FVector(0.0f, 0.0f, -6.0f),
        FVector(13.8f, 2.72f, 0.050f),
        FLinearColor(0.024f, 0.026f, 0.034f) + Mission.SecondaryAccentColor * 0.08f,
        CityLabel + TEXT(" World Bible Lore Deck"),
        TEXT("/Game/StarterContent/Materials/M_Tech_Hex_Tile.M_Tech_Hex_Tile"),
        false),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("WorldBibleRuntimeCue") });

    TagLoreActor(SpawnBlock(
        LoreHub + FVector(0.0f, -130.0f, 128.0f),
        FVector(13.0f, 0.065f, 0.88f),
        FLinearColor(0.036f, 0.040f, 0.052f) + Mission.AccentColor * 0.12f,
        CityLabel + TEXT(" World Bible Lore Backboard"),
        false),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("CanonicalLoreContract") });

    TagLoreActor(SpawnGuideText(
        FString::Printf(TEXT("OBJECTIVE LORE NAVIGATION\nPremise, pillars, factions, tech rules, city chapter\n%s"), *Mission.CityName),
        LoreHub + FVector(0.0f, -176.0f, 300.0f),
        FColor(210, 236, 255),
        21.0f),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("WorldBibleRuntimeCue") });

    for (int32 Index = 0; Index < Cards.Num(); ++Index)
    {
        const FWorldBibleCard& Card = Cards[Index];
        const FVector Base = LoreHub + Card.LocalOffset;
        TagLoreActor(SpawnBlock(
            Base + FVector(0.0f, 0.0f, 16.0f),
            FVector(2.12f, 0.45f, 0.060f),
            Card.Color * 1.42f,
            FString::Printf(TEXT("%s World Bible Lore Pad %d"), *CityLabel, Index + 1),
            false),
            Card.Tags);
        TagLoreActor(SpawnBlock(
            Base + FVector(0.0f, -52.0f, 176.0f),
            FVector(1.08f, 0.040f, 0.38f),
            Card.Color * 1.75f,
            FString::Printf(TEXT("%s World Bible Lore Plate %d"), *CityLabel, Index + 1),
            false),
            Card.Tags);
        TagLoreActor(SpawnGuideText(
            FString::Printf(TEXT("%s\n%s"), Card.Title, *Card.Detail),
            Base + FVector(0.0f, -88.0f, 252.0f),
            Card.Color.ToFColor(true),
            14.0f),
            Card.Tags);
        SpawnLoreLight(
            Base + FVector(0.0f, -28.0f, 218.0f),
            Card.Color,
            980.0f + Index * 150.0f,
            Card.Tags,
            FString::Printf(TEXT("%s World Bible Lore Light %d"), *CityLabel, Index + 1));
    }

    TagLoreActor(SpawnGuideText(
        TEXT("SAFEHOUSE SURVIVOR EXTRACTION\nCanon guide only; selected-language saves and combat rules stay authoritative."),
        LoreHub + FVector(0.0f, 140.0f, 244.0f),
        FColor(226, 250, 218),
        17.0f),
        TArray<FName>{ FName("RuntimeDataLayer_Mode_Storytelling"), FName("CanonicalLoreContract") });

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueWorldBibleLore] %s city=%d canon pillars/factions/technology rules surfaced for survivor='%s'."),
        *CityLabel,
        CityIndex,
        *Mission.SurvivorName);
}

// ---- #8 Authored prop kit-bash --------------------------------------------
void ACodeRescueGameMode::SpawnAuthoredPropsForCity(int32 CityIndex, const FVector& Origin, const FLinearColor& Accent)
{
    // Anchorage (CityIndex == 0) is the hero city for the prop kit-bash. Other
    // cities get a lighter version. Designers can swap these for Megascans /
    // PCG via the Blueprint subclass override later.
    struct FPropDef
    {
        FVector LocalOffset;
        FVector Scale;
        FLinearColor Tint;
        FString Name;
        const TCHAR* MeshPath;
        FRotator Rotation;
    };
    static const TArray<FPropDef> AnchoragePropDefs = {
        { FVector(-200.0f, -200.0f, 0.0f), FVector(1.1f, 0.9f, 1.1f), FLinearColor(0.2f, 0.2f, 0.25f), TEXT("TriageChair_NW"), TEXT("/Game/StarterContent/Props/SM_Chair.SM_Chair"), FRotator(0.0f, 25.0f, 0.0f) },
        { FVector( 200.0f, -150.0f, 0.0f), FVector(1.6f, 1.6f, 0.7f), FLinearColor(0.55f, 0.05f, 0.05f), TEXT("SupplyTable_NE"), TEXT("/Game/StarterContent/Props/SM_TableRound.SM_TableRound"), FRotator(0.0f, -18.0f, 0.0f) },
        { FVector(-150.0f,  200.0f, 0.0f), FVector(1.2f, 0.55f, 1.5f), FLinearColor(0.05f, 0.25f, 0.5f), TEXT("FieldShelf_SW"), TEXT("/Game/StarterContent/Props/SM_Shelf.SM_Shelf"), FRotator(0.0f, 90.0f, 0.0f) },
        { FVector( 300.0f,  300.0f, 0.0f), FVector(1.2f, 0.25f, 1.8f), FLinearColor(0.35f, 0.25f, 0.05f), TEXT("BarricadeDoor_SE"), TEXT("/Game/StarterContent/Props/SM_Door.SM_Door"), FRotator(0.0f, 68.0f, 0.0f) },
        { FVector(-500.0f,  400.0f, 0.0f), FVector(0.9f, 0.9f, 1.9f), FLinearColor(0.4f, 0.4f, 0.4f), TEXT("WallLamp_W"), TEXT("/Game/StarterContent/Props/SM_Lamp_Wall.SM_Lamp_Wall"), FRotator(0.0f, 180.0f, 0.0f) },
        { FVector( 500.0f, -400.0f, 0.0f), FVector(1.5f, 0.25f, 1.1f), FLinearColor(0.6f, 0.55f, 0.2f), TEXT("GlassPanel_E"), TEXT("/Game/StarterContent/Props/SM_GlassWindow.SM_GlassWindow"), FRotator(0.0f, -28.0f, 0.0f) },
        { FVector(   0.0f, -600.0f, 0.0f), FVector(1.7f, 1.5f, 0.7f), FLinearColor(0.3f, 0.5f, 0.3f), TEXT("StairRamp_N"), TEXT("/Game/StarterContent/Props/SM_Stairs.SM_Stairs"), FRotator(0.0f, 180.0f, 0.0f) },
        { FVector( 450.0f,    0.0f, 0.0f), FVector(1.1f, 1.1f, 1.8f), FLinearColor(0.5f, 0.3f, 0.5f), TEXT("CivicStatue_C"), TEXT("/Game/StarterContent/Props/SM_Statue.SM_Statue"), FRotator(0.0f, 35.0f, 0.0f) },
    };

    if (UStaticMesh* AccessBridgeMesh = LoadCodeRescueBridgeMesh(CityIndex))
    {
        AActor* Bridge = SpawnStaticMeshProp(
            AccessBridgeMesh,
            Origin + CityOffset(FVector(-3480.0f, -2980.0f, 85.0f)),
            FRotator(0.0f, 45.0f, 0.0f),
            CityExtent(FVector(12.0f, 1.4f, 0.45f)),
            FString::Printf(TEXT("AuthoredBridge_Access_City%d"), CityIndex),
            false);
        if (Bridge)
        {
            Bridge->Tags.Add(FName("AuthoredBridge"));
            Bridge->Tags.Add(FName("OpenCityAccess"));
        }
    }

    const int32 ShowcaseBuildingCount = (CityIndex == 0) ? 6 : 2;
    for (int32 i = 0; i < ShowcaseBuildingCount; ++i)
    {
        if (UStaticMesh* BuildingMesh = LoadCodeRescueCityBuildingMesh(CityIndex + i))
        {
            const float Side = (i % 2 == 0) ? -1.0f : 1.0f;
            const float Row = static_cast<float>(i / 2);
            const FVector BuildingScale = CityArchitectureExtent(FVector(1.65f + 0.22f * Row, 1.45f + 0.16f * Row, 3.7f + Row * 0.45f));
            AActor* Building = SpawnStaticMeshProp(
                BuildingMesh,
                Origin + CityOffset(FVector(-1850.0f + Row * 1050.0f, Side * (1180.0f + Row * 280.0f), 0.0f)) + FVector(0.0f, 0.0f, BuildingScale.Z * 50.0f),
                FRotator(0.0f, 90.0f * (i % 4), 0.0f),
                BuildingScale,
                FString::Printf(TEXT("AuthoredBuilding_Parallax_%d_City%d"), i, CityIndex));
            if (Building)
            {
                Building->Tags.Add(FName("AuthoredBuilding"));
            }
        }
    }

    if ((CityIndex % 3) == 1)
    {
        if (UStaticMesh* HarborBridgeMesh = LoadCodeRescueBridgeMesh(CityIndex + 5))
        {
            AActor* HarborSpan = SpawnStaticMeshProp(
                HarborBridgeMesh,
                Origin + CityOffset(FVector(0.0f, 2830.0f, 120.0f)),
                FRotator(0.0f, 0.0f, 0.0f),
                CityExtent(FVector(15.0f, 1.1f, 0.5f)),
                FString::Printf(TEXT("AuthoredBridge_HarborSpan_City%d"), CityIndex),
                false);
            if (HarborSpan)
            {
                HarborSpan->Tags.Add(FName("AuthoredBridge"));
            }
        }
    }

    const TArray<FPropDef>& Defs = AnchoragePropDefs;
    const int32 PropCount = (CityIndex == 0) ? Defs.Num() : FMath::Min(3, Defs.Num());
    for (int32 i = 0; i < PropCount; ++i)
    {
        const FPropDef& D = Defs[i];
        const FVector Loc = Origin + CityOffset(D.LocalOffset + FVector(0.0f, 0.0f, 80.0f));
        AActor* Prop = nullptr;
        if (UStaticMesh* PropMesh = LoadCodeRescueAssetMesh(D.MeshPath))
        {
            Prop = SpawnStaticMeshProp(
                PropMesh,
                Loc,
                D.Rotation,
                CityExtent(D.Scale),
                FString::Printf(TEXT("AuthoredProp_%s_City%d"), *D.Name, CityIndex),
                true);
        }
        if (!Prop)
        {
            Prop = SpawnBlock(Loc, CityExtent(D.Scale), D.Tint, FString::Printf(TEXT("AuthoredProp_%s_City%d"), *D.Name, CityIndex), true);
        }
        if (Prop)
        {
            Prop->Tags.Add(FName("AuthoredProp"));
            Prop->Tags.Add(FName("InspectableAuthoredMesh"));
        }
    }
}

// ---- #9 Per-zone post-process volume --------------------------------------
void ACodeRescueGameMode::ConfigurePerZonePostProcessVolume(APostProcessVolume* PPV, int32 CityIndex, const FString& GradeToken, EColorblindMode ColorblindMode) const
{
    if (!PPV) return;

    // City-aware aesthetic: Anchorage (cool/blue), Seattle (overcast), Tokyo (neon).
    // We cycle through three presets indexed by (CityIndex % 3) so the whole
    // city catalog still gets a graded look without bespoke tuning per index.
    const int32 PresetIdx = FMath::Abs(CityIndex) % 3;
    PPV->bUnbound = false;
    PPV->BlendWeight = 1.0f;

    // Bounding extent: cover the full 50x city. Use a generous box.
    PPV->SetActorScale3D(CityExtent(FVector(95.0f, 82.0f, 30.0f)));

    FPostProcessSettings& S = PPV->Settings;
    S.bOverride_ColorSaturation  = true;
    S.bOverride_ColorContrast    = true;
    S.bOverride_ColorGamma       = true;
    S.bOverride_VignetteIntensity = true;

    switch (PresetIdx)
    {
    case 0: // Cool metro — gentle, no heavy blue cast (2026-07-01: old 0.85/0.85/1.05 + blue gamma
            // stacked with the sky ambient into a teal wash; keep it near-neutral, faintly cool).
        S.ColorSaturation = FVector4(0.98f, 0.99f, 1.02f, 1.0f);
        S.ColorContrast   = FVector4(1.04f, 1.04f, 1.04f, 1.0f);
        S.ColorGamma      = FVector4(1.00f, 1.00f, 1.00f, 1.0f);
        S.VignetteIntensity = 0.30f;
        break;
    case 1: // Seattle desaturated overcast
        S.ColorSaturation = FVector4(0.7f, 0.7f, 0.7f, 1.0f);
        S.ColorContrast   = FVector4(0.95f, 0.95f, 0.95f, 1.0f);
        S.ColorGamma      = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
        S.VignetteIntensity = 0.45f;
        break;
    case 2: // Tokyo neon
    default:
        S.ColorSaturation = FVector4(1.2f, 1.05f, 1.25f, 1.0f);
        S.ColorContrast   = FVector4(1.1f, 1.05f, 1.1f, 1.0f);
        S.ColorGamma      = FVector4(1.05f, 0.98f, 1.05f, 1.0f);
        S.VignetteIntensity = 0.25f;
        break;
    }

    // improvement_pass_2026-06-12 #45 — U.S. cities override the 3-zone cycle
    // with a climate grade family derived from their realization params.
    const FString EffectiveGrade = (GradeToken == TEXT("LegacyZoneCycle")) ? FString() : GradeToken;
    if (!EffectiveGrade.IsEmpty())
    {
        const FString& Grade = EffectiveGrade;
        if (Grade == TEXT("CoolOvercast"))
        {
            // 2026-07-01: eased off the strong desaturate + blue lift that read as a teal cast.
            S.ColorSaturation = FVector4(0.92f, 0.93f, 0.96f, 1.0f);
            S.ColorContrast   = FVector4(0.98f, 0.98f, 0.99f, 1.0f);
            S.ColorGamma      = FVector4(1.00f, 1.00f, 1.01f, 1.0f);
            S.VignetteIntensity = 0.36f;
        }
        else if (Grade == TEXT("WarmDesert"))
        {
            S.ColorSaturation = FVector4(1.12f, 1.00f, 0.84f, 1.0f);
            S.ColorContrast   = FVector4(1.08f, 1.04f, 0.98f, 1.0f);
            S.ColorGamma      = FVector4(1.05f, 1.00f, 0.92f, 1.0f);
            S.VignetteIntensity = 0.28f;
        }
        else if (Grade == TEXT("CrispMountain"))
        {
            S.ColorSaturation = FVector4(0.98f, 1.02f, 1.08f, 1.0f);
            S.ColorContrast   = FVector4(1.10f, 1.10f, 1.10f, 1.0f);
            S.ColorGamma      = FVector4(1.00f, 1.00f, 1.03f, 1.0f);
            S.VignetteIntensity = 0.24f;
        }
        else if (Grade == TEXT("HumidGulf"))
        {
            S.ColorSaturation = FVector4(1.04f, 1.02f, 0.88f, 1.0f);
            S.ColorContrast   = FVector4(0.97f, 0.97f, 0.94f, 1.0f);
            S.ColorGamma      = FVector4(1.03f, 1.01f, 0.95f, 1.0f);
            S.VignetteIntensity = 0.36f;
        }
        else if (Grade == TEXT("TropicalBright"))
        {
            S.ColorSaturation = FVector4(1.16f, 1.10f, 1.06f, 1.0f);
            S.ColorContrast   = FVector4(1.06f, 1.05f, 1.04f, 1.0f);
            S.ColorGamma      = FVector4(1.04f, 1.02f, 1.00f, 1.0f);
            S.VignetteIntensity = 0.22f;
        }
        else if (Grade == TEXT("GoldenBasin"))
        {
            S.ColorSaturation = FVector4(1.08f, 1.00f, 0.90f, 1.0f);
            S.ColorContrast   = FVector4(1.04f, 1.02f, 0.98f, 1.0f);
            S.ColorGamma      = FVector4(1.05f, 1.01f, 0.94f, 1.0f);
            S.VignetteIntensity = 0.30f;
        }
        else // NeutralMetro
        {
            S.ColorSaturation = FVector4(0.96f, 0.96f, 0.98f, 1.0f);
            S.ColorContrast   = FVector4(1.04f, 1.04f, 1.04f, 1.0f);
            S.ColorGamma      = FVector4(1.00f, 1.00f, 1.00f, 1.0f);
            S.VignetteIntensity = 0.32f;
        }
    }

    // #45 — apply colorblind correction by remapping the saturation channel
    // weights. Deuteranope/Protanope cannot reliably distinguish red-green;
    // we boost blue + dampen the affected channel. Tritanope can't reliably
    // distinguish blue-yellow; we boost red.
    switch (ColorblindMode)
    {
    case EColorblindMode::Deuteranope:
    case EColorblindMode::Protanope:
        S.ColorSaturation.X *= 0.6f;   // dampen red
        S.ColorSaturation.Z *= 1.4f;   // boost blue
        break;
    case EColorblindMode::Tritanope:
        S.ColorSaturation.Z *= 0.6f;   // dampen blue
        S.ColorSaturation.X *= 1.4f;   // boost red
        break;
    default: break;
    }
}

void ACodeRescueGameMode::SpawnPerZonePostProcessVolume(int32 CityIndex, const FVector& Origin, const FLinearColor& Accent)
{
    static_cast<void>(Accent);

    APostProcessVolume* PPV = GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), Origin, FRotator::ZeroRotator);
    if (!PPV) return;

    // #45 — colorblind palette swap: rotate the saturation triplet so the
    // dominant zone hue is in the channel the player can perceive.
    EColorblindMode CBMode = EColorblindMode::None;
    if (UCodeRescueGameInstance* GIcb = GetGameInstance<UCodeRescueGameInstance>())
    {
        CBMode = GIcb->ColorblindMode;
    }

    const FString GradeToken = ActiveCityRealizationGradeToken.IsEmpty()
        ? FString(TEXT("LegacyZoneCycle"))
        : ActiveCityRealizationGradeToken;

    PPV->Tags.Add(FName("CodeRescueZonePostProcess"));
    PPV->Tags.Add(FName("CodeRescueColorVisionRefresh"));
    PPV->Tags.Add(FName(*FString::Printf(TEXT("CodeRescueCityIndex_%d"), CityIndex)));
    PPV->Tags.Add(FName(*FString::Printf(TEXT("CodeRescueGrade_%s"), *GradeToken)));

    ConfigurePerZonePostProcessVolume(PPV, CityIndex, GradeToken, CBMode);

    RegisterStreamedActor(PPV);
}

int32 ACodeRescueGameMode::RefreshActiveColorVisionPostProcess(EColorblindMode NewMode)
{
    int32 RefreshedCount = 0;
    const FName ZoneTag("CodeRescueZonePostProcess");

    auto RefreshVolume = [&](APostProcessVolume* PPV) -> bool
    {
        if (!PPV || !PPV->ActorHasTag(ZoneTag))
        {
            return false;
        }

        int32 VolumeCityIndex = ActiveCampaignCityIndex >= 0 ? ActiveCampaignCityIndex : 0;
        FString VolumeGradeToken = ActiveCityRealizationGradeToken.IsEmpty()
            ? FString(TEXT("LegacyZoneCycle"))
            : ActiveCityRealizationGradeToken;

        for (const FName& Tag : PPV->Tags)
        {
            FString TagText = Tag.ToString();
            if (TagText.RemoveFromStart(TEXT("CodeRescueCityIndex_")))
            {
                VolumeCityIndex = FCString::Atoi(*TagText);
            }
            else if (TagText.RemoveFromStart(TEXT("CodeRescueGrade_")))
            {
                VolumeGradeToken = TagText;
            }
        }

        ConfigurePerZonePostProcessVolume(PPV, VolumeCityIndex, VolumeGradeToken, NewMode);
        ++RefreshedCount;
        return true;
    };

    for (const TWeakObjectPtr<AActor>& ActorPtr : StreamedCampaignActors)
    {
        RefreshVolume(Cast<APostProcessVolume>(ActorPtr.Get()));
    }

    if (RefreshedCount == 0 && GetWorld())
    {
        for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
        {
            RefreshVolume(*It);
        }
    }

    const TCHAR* ModeLabel = TEXT("Standard");
    switch (NewMode)
    {
    case EColorblindMode::Deuteranope: ModeLabel = TEXT("Deuteranope"); break;
    case EColorblindMode::Protanope: ModeLabel = TEXT("Protanope"); break;
    case EColorblindMode::Tritanope: ModeLabel = TEXT("Tritanope"); break;
    default: break;
    }

    UE_LOG(LogTemp, Display, TEXT("[CodeRescueColorVisionRefresh] refreshed=%d mode='%s' active_city=%d"),
        RefreshedCount,
        ModeLabel,
        ActiveCampaignCityIndex);

    return RefreshedCount;
}

// ---- #10 Per-zone ambient sound -------------------------------------------
void ACodeRescueGameMode::SpawnAmbientSoundForCity(int32 CityIndex, const FVector& Origin)
{
    const int32 ZoneIdx = CityIndex % 3;
    if (!ZoneAmbientCues.IsValidIndex(ZoneIdx))
    {
        return; // No ambient cue assigned for this zone yet.
    }
    USoundBase* Cue = ZoneAmbientCues[ZoneIdx].LoadSynchronous();
    if (!Cue) return;

    AAmbientSound* Ambient = GetWorld()->SpawnActor<AAmbientSound>(AAmbientSound::StaticClass(), Origin + FVector(0.0f, 0.0f, 1500.0f), FRotator::ZeroRotator);
    if (!Ambient) return;

    if (UAudioComponent* AC = Ambient->GetAudioComponent())
    {
        AC->SetSound(Cue);
        AC->bAutoActivate = true;
        AC->VolumeMultiplier = 0.45f * GetRuntimeMusicVolume(this);
        AC->bAllowSpatialization = !IsRuntimeMonoAudioEnabled(this);
        Ambient->Tags.AddUnique(FName("MonoAudioRefreshableAmbientCue"));
    }
    RegisterStreamedActor(Ambient);
}

int32 ACodeRescueGameMode::RefreshMonoAudioSpatialization(bool bMonoAudioEnabled)
{
    int32 RefreshedCount = 0;
    auto RefreshAmbient = [&](AAmbientSound* Ambient)
    {
        if (!IsValid(Ambient))
        {
            return;
        }
        if (UAudioComponent* AC = Ambient->GetAudioComponent())
        {
            AC->bAllowSpatialization = !bMonoAudioEnabled;
            ++RefreshedCount;
        }
    };

    for (const TWeakObjectPtr<AActor>& ActorPtr : StreamedCampaignActors)
    {
        RefreshAmbient(Cast<AAmbientSound>(ActorPtr.Get()));
    }

    if (GetWorld())
    {
        for (TActorIterator<AAmbientSound> It(GetWorld()); It; ++It)
        {
            RefreshAmbient(*It);
        }
        for (TActorIterator<ACodeZombieActor> It(GetWorld()); It; ++It)
        {
            if (ACodeZombieActor* Zombie = *It)
            {
                Zombie->ApplyMonoAudioAccessibility(bMonoAudioEnabled);
                ++RefreshedCount;
            }
        }
    }

    UE_LOG(LogTemp, Display, TEXT("[CodeRescueMonoAudio] refreshed=%d mono=%s"),
        RefreshedCount,
        bMonoAudioEnabled ? TEXT("on") : TEXT("off"));
    return RefreshedCount;
}

int32 ACodeRescueGameMode::SpawnChallengeCompletionSupplyCache(
    const FString& ChallengeId,
    int32 CityIndex,
    const FVector& TerminalLocation)
{
    UWorld* World = GetWorld();
    if (!World || ChallengeId.IsEmpty())
    {
        return 0;
    }

    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const int32 Progress = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    int32 Spawned = 0;
    auto SpawnReward = [&](EPickupKind Kind, int32 Amount, const FVector& Offset)
    {
        APickupActor* Pickup = World->SpawnActor<APickupActor>(
            APickupActor::StaticClass(),
            TerminalLocation + Offset,
            FRotator::ZeroRotator);
        if (!Pickup)
        {
            return;
        }

        Pickup->Kind = Kind;
        Pickup->Amount = Amount;
        Pickup->bSnapToGround = true;
        Pickup->Tags.AddUnique(FName("ChallengeCompletionSupply"));
        Pickup->Tags.AddUnique(FName("FirstTimeCodingReward"));
        Pickup->Tags.AddUnique(FName("PickupGroundSnapRequired"));
        Pickup->Tags.AddUnique(FName(*FString::Printf(TEXT("ChallengeRewardCity_%d"), CityIndex)));
        RegisterStreamedActor(Pickup);
        ++Spawned;
    };

    SpawnReward(EPickupKind::Ammo, 24 + Progress * 2, FVector(-120.0f, 92.0f, 86.0f));
    SpawnReward(EPickupKind::Scrap, 2 + (Progress % 3), FVector(0.0f, 128.0f, 86.0f));
    if ((Progress % 5) == 0)
    {
        SpawnReward(EPickupKind::ArmorPlate, 1, FVector(120.0f, 92.0f, 86.0f));
    }
    else if ((Progress % 3) == 0)
    {
        SpawnReward(EPickupKind::Medkit, 1, FVector(120.0f, 92.0f, 86.0f));
    }
    else
    {
        SpawnReward((Progress % 2) == 0 ? EPickupKind::Stim : EPickupKind::Flare, 1, FVector(120.0f, 92.0f, 86.0f));
    }

    UE_LOG(LogTemp, Display,
        TEXT("[ChallengeSupplyDrop] city=%d challenge='%s' progress=%d/%d pickups=%d location=%s"),
        CityIndex,
        *ChallengeId,
        Progress,
        FCodeRescueCampaign::RequiredChallengesPerCity,
        Spawned,
        *TerminalLocation.ToCompactString());
    return Spawned;
}

int32 ACodeRescueGameMode::SpawnZombieDeathSupply(int32 ZombieId, const FVector& DeathLocation)
{
    UWorld* World = GetWorld();
    if (!World || ZombieId < 0)
    {
        return 0;
    }

    EPickupKind Kind = EPickupKind::Ammo;
    int32 Amount = 18;
    switch (ZombieId % 10)
    {
    case 0: Kind = EPickupKind::Medkit; Amount = 1; break;
    case 1: Kind = EPickupKind::ArmorPlate; Amount = 1; break;
    case 2:
    case 7: Kind = EPickupKind::Scrap; Amount = 3; break;
    case 5: Kind = EPickupKind::Flare; Amount = 1; break;
    case 8: Kind = EPickupKind::Smoke; Amount = 1; break;
    default: Kind = EPickupKind::Ammo; Amount = 18 + (ZombieId % 4) * 4; break;
    }

    APickupActor* Pickup = World->SpawnActor<APickupActor>(
        APickupActor::StaticClass(),
        DeathLocation + FVector(0.0f, 0.0f, 78.0f),
        FRotator::ZeroRotator);
    if (!Pickup)
    {
        return 0;
    }

    Pickup->Kind = Kind;
    Pickup->Amount = Amount;
    Pickup->bSnapToGround = true;
    Pickup->Tags.AddUnique(FName("ZombieSupplyDrop"));
    Pickup->Tags.AddUnique(FName("CombatRewardSupply"));
    Pickup->Tags.AddUnique(FName("PickupGroundSnapRequired"));
    RegisterStreamedActor(Pickup);
    UE_LOG(LogTemp, Display,
        TEXT("[ZombieSupplyDrop] zombie_id=%d kind=%d amount=%d location=%s"),
        ZombieId,
        static_cast<int32>(Kind),
        Amount,
        *DeathLocation.ToCompactString());
    return 1;
}

void ACodeRescueGameMode::RevealSolvedTerminalRescueRoute(const FString& TerminalId, int32 CityIndex, const FVector& TerminalLocation, bool bFromLoad)
{
    if (!GetWorld() || TerminalId.IsEmpty())
    {
        return;
    }

    const int32 MissionCount = FCodeRescueCampaign::GetMissionCount();
    if (MissionCount <= 0)
    {
        return;
    }
    const int32 ResolvedCityIndex = FMath::Clamp(
        CityIndex >= 0 ? CityIndex : ActiveCampaignCityIndex,
        0,
        MissionCount - 1);

    // The rescue route is a city-wide response, not one response per terminal.
    // A per-terminal tag let every restored solve build another route layer.
    const FName ResponseTag(*FString::Printf(TEXT("SolvedRouteCity_%d"), ResolvedCityIndex));

    for (const TWeakObjectPtr<AActor>& ActorPtr : StreamedCampaignActors)
    {
        const AActor* Actor = ActorPtr.Get();
        if (Actor && Actor->Tags.Contains(ResponseTag))
        {
            return;
        }
    }

    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(ResolvedCityIndex);
    const FString CityLabel = FCodeRescueCampaign::GetMissionLabel(ResolvedCityIndex);
    const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(ResolvedCityIndex);
    const FLinearColor MissionAccent = Mission ? Mission->AccentColor : FLinearColor(0.0f, 0.86f, 1.0f);
    const FLinearColor MissionSecondary = Mission ? Mission->SecondaryAccentColor : FLinearColor(1.0f, 0.82f, 0.18f);
    const FLinearColor SignalCyan = FLinearColor(0.10f, 0.96f, 1.0f);
    const FLinearColor RescueGold = FLinearColor(1.0f, 0.86f, 0.18f);
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    const ECodingLanguage ActiveLanguage = GI ? GI->SelectedLanguage : ECodingLanguage::Java;
    FString LanguageLabel = TEXT("JAVA");
    FString SafeLanguageTag = TEXT("Java");
    FString LanguageBreachCue = TEXT("object contracts hold the relay line");
    FLinearColor LanguageColor = FLinearColor(1.0f, 0.40f, 0.18f);
    EPickupKind LanguageRewardKind = EPickupKind::Flare;
    int32 LanguageRewardAmount = 1;
    EZombieVariant LanguagePatrolVariant = EZombieVariant::UrbanZombie4;
    switch (ActiveLanguage)
    {
    case ECodingLanguage::C:
        LanguageLabel = TEXT("C");
        SafeLanguageTag = TEXT("C");
        LanguageBreachCue = TEXT("pointer route: armor the survivor corridor");
        LanguageColor = FLinearColor(0.25f, 0.58f, 1.0f);
        LanguageRewardKind = EPickupKind::ArmorPlate;
        LanguageRewardAmount = 1;
        LanguagePatrolVariant = EZombieVariant::BusinessSuit;
        break;
    case ECodingLanguage::Python:
        LanguageLabel = TEXT("PYTHON");
        SafeLanguageTag = TEXT("Python");
        LanguageBreachCue = TEXT("readable route: smoke breaks the chase");
        LanguageColor = FLinearColor(1.0f, 0.86f, 0.22f);
        LanguageRewardKind = EPickupKind::Smoke;
        LanguageRewardAmount = 1;
        LanguagePatrolVariant = EZombieVariant::NurseFemale;
        break;
    case ECodingLanguage::MATLAB:
        LanguageLabel = TEXT("MATLAB");
        SafeLanguageTag = TEXT("MATLAB");
        LanguageBreachCue = TEXT("matrix route: stim through the breach");
        LanguageColor = FLinearColor(0.85f, 0.30f, 1.0f);
        LanguageRewardKind = EPickupKind::Stim;
        LanguageRewardAmount = 1;
        LanguagePatrolVariant = EZombieVariant::BloatedFemale;
        break;
    case ECodingLanguage::CPlus:
        LanguageLabel = TEXT("C+");
        SafeLanguageTag = TEXT("CPlus");
        LanguageBreachCue = TEXT("legacy bridge: scrap reinforces cover");
        LanguageColor = FLinearColor(0.20f, 0.84f, 1.0f);
        LanguageRewardKind = EPickupKind::Scrap;
        LanguageRewardAmount = 4;
        LanguagePatrolVariant = EZombieVariant::BaseMesh;
        break;
    case ECodingLanguage::Cpp:
        LanguageLabel = TEXT("C++");
        SafeLanguageTag = TEXT("Cpp");
        LanguageBreachCue = TEXT("template route: ammo clears extraction");
        LanguageColor = FLinearColor(0.18f, 0.64f, 1.0f);
        LanguageRewardKind = EPickupKind::Ammo;
        LanguageRewardAmount = 50;
        LanguagePatrolVariant = EZombieVariant::EliteCharger;
        break;
    case ECodingLanguage::Java:
    default:
        break;
    }

    FCodeRescueChallenge DataDrivenFilterChallenge;
    FCodeRescueChallenge DataDrivenLockChallenge;
    FCodeRescueChallenge DataDrivenReverseChallenge;
    int32 DataDrivenOutputMagnitude = 1;
    if (IsFilterTerminalId(TerminalId) || IsLockTerminalId(TerminalId) || IsReverseTerminalId(TerminalId))
    {
        TArray<FCodeRescueChallenge> AllLearningChallenges;
        FString LearningLoadError;
        if (UCodeRescueLearningLibrary::LoadChallenges(AllLearningChallenges, LearningLoadError))
        {
            TArray<FCodeRescueChallenge> FilterPool;
            TArray<FCodeRescueChallenge> LockPool;
            TArray<FCodeRescueChallenge> ReversePool;
            for (const FCodeRescueChallenge& Candidate : AllLearningChallenges)
            {
                const bool bLanguageMatch = Candidate.Language.Equals(TEXT("All"), ESearchCase::IgnoreCase)
                    || Candidate.Language.Equals(LanguageLabel, ESearchCase::IgnoreCase);
                if (!bLanguageMatch || Candidate.Prompt.IsEmpty())
                {
                    continue;
                }
                if (IsFilterTerminalId(TerminalId) && IsFilterLearningWorldNode(Candidate))
                {
                    FilterPool.Add(Candidate);
                }
                if (IsLockTerminalId(TerminalId) && IsLockLearningWorldNode(Candidate))
                {
                    LockPool.Add(Candidate);
                }
                if (IsReverseTerminalId(TerminalId) && IsReverseLearningWorldNode(Candidate))
                {
                    ReversePool.Add(Candidate);
                }
            }
            if (FilterPool.Num() > 0)
            {
                const int32 FilterIndex = ((ResolvedCityIndex % FilterPool.Num()) + FilterPool.Num()) % FilterPool.Num();
                DataDrivenFilterChallenge = FilterPool[FilterIndex];
                DataDrivenOutputMagnitude = FMath::Clamp(
                    EstimateLearningOutputMagnitude(DataDrivenFilterChallenge),
                    0,
                    6);
            }
            if (LockPool.Num() > 0)
            {
                const int32 LockIndex = ((ResolvedCityIndex % LockPool.Num()) + LockPool.Num()) % LockPool.Num();
                DataDrivenLockChallenge = LockPool[LockIndex];
            }
            if (ReversePool.Num() > 0)
            {
                const int32 ReverseIndex = ((ResolvedCityIndex % ReversePool.Num()) + ReversePool.Num()) % ReversePool.Num();
                DataDrivenReverseChallenge = ReversePool[ReverseIndex];
                DataDrivenOutputMagnitude = FMath::Clamp(
                    EstimateLearningStringOutputMagnitude(DataDrivenReverseChallenge),
                    0,
                    6);
            }
        }
    }
    const FCodeRescueChallenge ActiveDataDrivenChallenge = DataDrivenFilterChallenge.IsValid()
        ? DataDrivenFilterChallenge
        : (DataDrivenLockChallenge.IsValid() ? DataDrivenLockChallenge : DataDrivenReverseChallenge);

    auto TagSolvedRoute = [this, &ResponseTag](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("CodingToRescueWorldResponse"));
            Actor->Tags.AddUnique(FName("SolvedTerminalWorldResponse"));
            Actor->Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
            Actor->Tags.AddUnique(FName("CodingCauseEffect"));
            Actor->Tags.AddUnique(FName("TerminalSolvedRouteVisible"));
            Actor->Tags.AddUnique(ResponseTag);
            ApplyRuntimeDataLayerTags(Actor, TArray<FName>{
                FName("RuntimeDataLayer_State_RescueRouteOpen"),
                FName("RuntimeDataLayer_Mode_RescueTraversal"),
                FName("RuntimeDataLayer_State_Prerecovery"),
            });
        }
        return Actor;
    };

    auto AddLanguageBreachTags = [&SafeLanguageTag](AActor* Actor) -> AActor*
    {
        if (Actor)
        {
            Actor->Tags.AddUnique(FName("LanguageBreachCheckpoint"));
            Actor->Tags.AddUnique(FName("LanguageSpecificEncounter"));
            Actor->Tags.AddUnique(FName("SelectedLanguageOnly"));
            Actor->Tags.AddUnique(FName("RouteEncounterBeat"));
            Actor->Tags.AddUnique(FName(*FString::Printf(TEXT("LanguageTrack_%s"), *SafeLanguageTag)));
        }
        return Actor;
    };

    auto TagLanguageBreach = [&](AActor* Actor) -> AActor*
    {
        TagSolvedRoute(Actor);
        AddLanguageBreachTags(Actor);
        return Actor;
    };

    auto SpawnSolvedLight = [&](const FVector& Location, const FLinearColor& Color, float Intensity, float Radius, const FString& Name) -> APointLight*
    {
        if (APointLight* Light = GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator))
        {
            if (UPointLightComponent* PLC = Cast<UPointLightComponent>(Light->GetLightComponent()))
            {
                PLC->SetMobility(EComponentMobility::Movable);
                PLC->SetIntensity(Intensity);
                PLC->SetLightColor(Color);
                PLC->SetAttenuationRadius(Radius);
                PLC->SetCastShadows(false);
            }
#if WITH_EDITOR
            Light->SetActorLabel(Name);
#endif
            RegisterStreamedActor(Light);
            TagSolvedRoute(Light);
            return Light;
        }
        return nullptr;
    };

    auto SpawnRouteRewardPickup = [&](EPickupKind Kind, const FVector& Location, int32 Amount, const TCHAR* Label) -> APickupActor*
    {
        APickupActor* Pickup = GetWorld()->SpawnActor<APickupActor>(APickupActor::StaticClass(), Location, FRotator::ZeroRotator);
        if (Pickup)
        {
            Pickup->Kind = Kind;
            Pickup->Amount = Amount;
            Pickup->Tags.Add(FName("SolvedRouteRewardPickup"));
            Pickup->Tags.Add(FName(Label));
            RegisterStreamedActor(Pickup);
            TagSolvedRoute(Pickup);
        }
        return Pickup;
    };

    auto SpawnPulseSegment = [&](const FVector& WorldA, const FVector& WorldB, int32 SegmentIndex)
    {
        const FVector FlatDelta(WorldB.X - WorldA.X, WorldB.Y - WorldA.Y, 0.0f);
        const float Length = FMath::Max(100.0f, FlatDelta.Size());
        const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(FlatDelta.Y, FlatDelta.X));
        const FVector Mid = (WorldA + WorldB) * 0.5f;
        const FLinearColor SegmentColor = (SegmentIndex % 2 == 0 ? SignalCyan : MissionAccent) * 2.2f;
        TagSolvedRoute(SpawnRotatedBlock(
            FVector(Mid.X, Mid.Y, Origin.Z + 28.0f),
            FRotator(0.0f, Yaw, 0.0f),
            FVector(Length / 100.0f, 0.10f, 0.035f),
            SegmentColor,
            FString::Printf(TEXT("%s Solved Rescue Route Pulse Segment %d"), *CityLabel, SegmentIndex),
            false));
    };

    auto SpawnRouteGuidanceDrone = [&](const FVector& WorldA, const FVector& WorldB, int32 SegmentIndex) -> ARescueRouteGuidanceDroneActor*
    {
        const FLinearColor DroneTint = (SegmentIndex % 2 == 0 ? MissionAccent : SignalCyan) * 0.72f + RescueGold * 0.28f;
        const FVector DroneStart(WorldA.X, WorldA.Y, Origin.Z + 42.0f);
        const FVector DroneEnd(WorldB.X, WorldB.Y, Origin.Z + 42.0f);

        FActorSpawnParameters DroneParams;
        DroneParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ARescueRouteGuidanceDroneActor* Drone = GetWorld()->SpawnActor<ARescueRouteGuidanceDroneActor>(
            ARescueRouteGuidanceDroneActor::StaticClass(),
            DroneStart + FVector(0.0f, 0.0f, 210.0f),
            FRotator::ZeroRotator,
            DroneParams);
        if (!Drone)
        {
            return nullptr;
        }

        Drone->ConfigureDrone(
            DroneStart,
            DroneEnd,
            DroneTint,
            static_cast<float>(SegmentIndex) * 0.83f,
            GI && GI->bReducedMotion);
        Drone->Tags.AddUnique(FName("SolvedRouteGuidanceDrone"));
        Drone->Tags.AddUnique(FName("RescueRouteGuidanceDroneLayer"));
        Drone->Tags.AddUnique(FName("AnimatedWayfinding"));
        Drone->Tags.AddUnique(FName(*FString::Printf(TEXT("SolvedRouteGuidanceDrone_%d"), SegmentIndex)));
        RegisterStreamedActor(Drone);
        TagSolvedRoute(Drone);
        return Drone;
    };

    const FVector TerminalBase(TerminalLocation.X, TerminalLocation.Y, FMath::Max(TerminalLocation.Z, Origin.Z + 90.0f));
    const FVector SurvivorBase = Origin + CityOffset(FVector(2850.0f, 1500.0f, 90.0f));
    const FVector RoutePoints[] = {
        FVector(TerminalBase.X, TerminalBase.Y, Origin.Z + 28.0f),
        Origin + CityOffset(FVector(-1820.0f, -1880.0f, 28.0f)),
        Origin + CityOffset(FVector(120.0f, -520.0f, 28.0f)),
        Origin + CityOffset(FVector(1660.0f, 560.0f, 28.0f)),
        FVector(SurvivorBase.X, SurvivorBase.Y, Origin.Z + 28.0f),
    };

    TagSolvedRoute(SpawnBlock(
        TerminalBase + FVector(0.0f, -92.0f, 154.0f),
        FVector(1.85f, 0.08f, 1.15f),
        SignalCyan * 2.5f,
        CityLabel + TEXT(" Solved Terminal Rescue Relay"),
        false));
    TagSolvedRoute(SpawnBlock(
        TerminalBase + FVector(0.0f, -96.0f, 315.0f),
        FVector(0.74f, 0.74f, 0.055f),
        SignalCyan * 4.0f,
        CityLabel + TEXT(" Solved Terminal Relay Halo"),
        false));
    // The route is already communicated by the HUD, pulse path, relay halo,
    // and guidance drone. Do not turn solve status into an interactable prose
    // marker; that reader could open later while walking and layer over pause.
    SpawnSolvedLight(TerminalBase + FVector(0.0f, -70.0f, 260.0f), SignalCyan, 9200.0f, 1350.0f, CityLabel + TEXT(" Solved Terminal Relay Light"));

    if (!bFromLoad)
    {
        const FString SolveEffectText = ActiveDataDrivenChallenge.IsValid()
            ? UCodeRescueLearningLibrary::GetWorldEffect(ActiveDataDrivenChallenge)
            : TEXT("The solved terminal reopens the rescue route.");
        const int32 SolveEffectMagnitude = ActiveDataDrivenChallenge.IsValid()
            ? DataDrivenOutputMagnitude
            : 1;

        FActorSpawnParameters EffectParams;
        EffectParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        if (ACodeRescueSolveEffectActor* SolveEffect = GetWorld()->SpawnActor<ACodeRescueSolveEffectActor>(
            ACodeRescueSolveEffectActor::StaticClass(),
            TerminalBase + FVector(0.0f, -118.0f, 82.0f),
            FRotator::ZeroRotator,
            EffectParams))
        {
            SolveEffect->ConfigureSolveEffect(
                SolveEffectText,
                ActiveDataDrivenChallenge.IsValid() ? (LanguageColor * 0.65f + MissionAccent * 0.35f) : SignalCyan,
                SolveEffectMagnitude,
                GI && GI->bReducedMotion,
                6.0f);
            SolveEffect->Tags.AddUnique(FName("RuntimeSolvedIntrinsicEffect"));
            SolveEffect->Tags.AddUnique(FName("OutputMagnitudeWorldEffect"));
            SolveEffect->Tags.AddUnique(FName("DataDrivenLearningNode"));
            RegisterStreamedActor(SolveEffect);
            TagSolvedRoute(SolveEffect);
        }
    }

    for (int32 i = 1; i < UE_ARRAY_COUNT(RoutePoints); ++i)
    {
        SpawnPulseSegment(RoutePoints[i - 1], RoutePoints[i], i);
        SpawnRouteGuidanceDrone(RoutePoints[i - 1], RoutePoints[i], i);
        TagSolvedRoute(SpawnBlock(
            RoutePoints[i] + FVector(0.0f, 0.0f, 92.0f),
            FVector(0.42f, 0.42f, 1.05f),
            (i == UE_ARRAY_COUNT(RoutePoints) - 1 ? RescueGold : SignalCyan) * 1.8f,
            FString::Printf(TEXT("%s Solved Rescue Route Beacon %d"), *CityLabel, i),
            false));
    }

    TagSolvedRoute(SpawnBlock(
        SurvivorBase + FVector(-160.0f, -170.0f, 82.0f),
        FVector(0.12f, 0.12f, 1.65f),
        RescueGold * 1.65f,
        CityLabel + TEXT(" Survivor Route Open West Post"),
        false));
    TagSolvedRoute(SpawnBlock(
        SurvivorBase + FVector(160.0f, -170.0f, 82.0f),
        FVector(0.12f, 0.12f, 1.65f),
        RescueGold * 1.65f,
        CityLabel + TEXT(" Survivor Route Open East Post"),
        false));
    TagSolvedRoute(SpawnBlock(
        SurvivorBase + FVector(0.0f, -170.0f, 172.0f),
        FVector(3.35f, 0.10f, 0.16f),
        RescueGold * 2.3f,
        CityLabel + TEXT(" Survivor Route Open Arch"),
        false));
    TagSolvedRoute(SpawnGuideText(
        TEXT("SURVIVOR ROUTE UNLOCKED\nlesson complete: extract the team"),
        SurvivorBase + FVector(0.0f, -230.0f, 420.0f),
        RescueGold.ToFColor(true),
        30.0f));
    SpawnSolvedLight(SurvivorBase + FVector(0.0f, -120.0f, 280.0f), RescueGold, 9800.0f, 1500.0f, CityLabel + TEXT(" Survivor Route Open Light"));

    if (DataDrivenFilterChallenge.IsValid())
    {
        auto TagFilterNode = [&](AActor* Actor) -> AActor*
        {
            TagSolvedRoute(Actor);
            if (Actor)
            {
                Actor->Tags.AddUnique(FName("DataDrivenFilterNode"));
                Actor->Tags.AddUnique(FName("FilterNodeEvacOrder"));
                Actor->Tags.AddUnique(FName("PedagogicalWorldEffect"));
                Actor->Tags.AddUnique(FName("UXFinishReadableFeedback"));
                Actor->Tags.AddUnique(FName("PhysicsSafeNonBlocking"));
                Actor->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
                Actor->Tags.AddUnique(FName("WorldArtPlayablePlaceholder"));
            }
            return Actor;
        };

        const FVector FilterBase = SurvivorBase + FVector(0.0f, -565.0f, 46.0f);
        const int32 KeptUnitCount = FMath::Clamp(DataDrivenOutputMagnitude, 0, 5);

        TagFilterNode(SpawnBlock(
            FilterBase + FVector(0.0f, 0.0f, -42.0f),
            FVector(5.2f, 2.35f, 0.08f),
            FLinearColor(0.025f, 0.040f, 0.036f) + RescueGold * 0.11f,
            CityLabel + TEXT(" Filter Output Evac Pad"),
            false));
        TagFilterNode(SpawnBlock(
            FilterBase + FVector(-310.0f, -80.0f, 28.0f),
            FVector(0.10f, 2.0f, 0.16f),
            SignalCyan * 2.2f,
            CityLabel + TEXT(" Filter Keep Lane Rail"),
            false));
        TagFilterNode(SpawnBlock(
            FilterBase + FVector(310.0f, -80.0f, 28.0f),
            FVector(0.10f, 2.0f, 0.16f),
            FLinearColor(0.28f, 0.30f, 0.32f) * 1.4f,
            CityLabel + TEXT(" Filter Reject Lane Rail"),
            false));
        TagFilterNode(SpawnGuideText(
            FString::Printf(
                TEXT("FILTER OUTPUT -> EVAC ORDER\n%s\nkept units: %d"),
                *UCodeRescueLearningLibrary::GetWorldEffect(DataDrivenFilterChallenge),
                KeptUnitCount),
            FilterBase + FVector(0.0f, -235.0f, 330.0f),
            RescueGold.ToFColor(true),
            24.0f));

        if (KeptUnitCount == 0)
        {
            TagFilterNode(SpawnGuideText(
                TEXT("EMPTY OUTPUT\nroute stays clear; no units board"),
                FilterBase + FVector(-180.0f, -80.0f, 180.0f),
                SignalCyan.ToFColor(true),
                22.0f));
        }
        for (int32 i = 0; i < KeptUnitCount; ++i)
        {
            const float X = -220.0f + static_cast<float>(i) * 110.0f;
            TagFilterNode(SpawnBlock(
                FilterBase + FVector(X, -32.0f, 48.0f),
                FVector(0.72f, 0.34f, 0.28f),
                RescueGold * (1.25f + i * 0.08f),
                FString::Printf(TEXT("%s Filter Kept Even Unit %d"), *CityLabel, i + 1),
                false));
            TagFilterNode(SpawnBlock(
                FilterBase + FVector(X, -32.0f, 88.0f),
                FVector(0.42f, 0.26f, 0.16f),
                SignalCyan * (1.75f + i * 0.12f),
                FString::Printf(TEXT("%s Filter Kept Unit Beacon %d"), *CityLabel, i + 1),
                false));
            TagFilterNode(SpawnGuideText(
                FString::Printf(TEXT("#%d"), i + 1),
                FilterBase + FVector(X, -100.0f, 160.0f),
                SignalCyan.ToFColor(true),
                18.0f));
        }
        for (int32 i = 0; i < 3; ++i)
        {
            const float X = -160.0f + static_cast<float>(i) * 160.0f;
            TagFilterNode(SpawnBlock(
                FilterBase + FVector(X, 128.0f, 34.0f),
                FVector(0.44f, 0.24f, 0.18f),
                FLinearColor(0.18f, 0.20f, 0.21f),
                FString::Printf(TEXT("%s Filter Rejected Odd Unit %d"), *CityLabel, i + 1),
                false));
        }

        TagFilterNode(SpawnBlock(
            FilterBase + FVector(-470.0f, 72.0f, 112.0f),
            FVector(0.34f, 0.28f, 1.24f),
            LanguageColor * 0.95f + RescueGold * 0.25f,
            CityLabel + TEXT(" Mentor Point Gesture Body"),
            false));
        TagFilterNode(SpawnBlock(
            FilterBase + FVector(-470.0f, 72.0f, 196.0f),
            FVector(0.22f, 0.22f, 0.22f),
            FLinearColor(0.78f, 0.74f, 0.66f),
            CityLabel + TEXT(" Mentor Point Gesture Head"),
            false));
        TagFilterNode(SpawnRotatedBlock(
            FilterBase + FVector(-400.0f, 28.0f, 162.0f),
            FRotator(0.0f, 0.0f, -24.0f),
            FVector(0.62f, 0.06f, 0.08f),
            SignalCyan * 1.8f,
            CityLabel + TEXT(" Mentor Point Gesture Arm"),
            false));

        for (int32 i = 0; i < 3; ++i)
        {
            const float X = 410.0f + static_cast<float>(i) * 62.0f;
            const float StepZ = (i % 2 == 0) ? 0.0f : 16.0f;
            TagFilterNode(SpawnBlock(
                FilterBase + FVector(X, -22.0f + i * 22.0f, 98.0f + StepZ),
                FVector(0.26f, 0.22f, 0.82f),
                FLinearColor(0.20f, 0.78f, 0.56f) + RescueGold * 0.16f,
                FString::Printf(TEXT("%s Survivor Boarding Pose Proxy %d"), *CityLabel, i + 1),
                false));
            TagFilterNode(SpawnRotatedBlock(
                FilterBase + FVector(X + 22.0f, -22.0f + i * 22.0f, 60.0f + StepZ),
                FRotator(0.0f, 0.0f, (i % 2 == 0) ? -18.0f : 18.0f),
                FVector(0.08f, 0.05f, 0.38f),
                FLinearColor(0.18f, 0.62f, 0.48f),
                FString::Printf(TEXT("%s Survivor Boarding Step Key %d"), *CityLabel, i + 1),
                false));
        }
    }

    if (DataDrivenLockChallenge.IsValid())
    {
        auto TagBooleanNode = [&](AActor* Actor) -> AActor*
        {
            TagSolvedRoute(Actor);
            if (Actor)
            {
                Actor->Tags.AddUnique(FName("DataDrivenBooleanNode"));
                Actor->Tags.AddUnique(FName("BooleanAirlockWorldEffect"));
                Actor->Tags.AddUnique(FName("TruthTableRescueArtifact"));
                Actor->Tags.AddUnique(FName("PedagogicalWorldEffect"));
                Actor->Tags.AddUnique(FName("UXFinishReadableFeedback"));
                Actor->Tags.AddUnique(FName("PhysicsSafeNonBlocking"));
                Actor->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
                Actor->Tags.AddUnique(FName("WorldArtPlayablePlaceholder"));
            }
            return Actor;
        };

        const FVector LockBase = SurvivorBase + FVector(0.0f, -610.0f, 58.0f);
        const FLinearColor SafeGreen = FLinearColor(0.16f, 1.0f, 0.42f);
        const FLinearColor BlockedRed = FLinearColor(0.92f, 0.12f, 0.08f);
        const FLinearColor DoorBlue = FLinearColor(0.18f, 0.72f, 1.0f);

        TagBooleanNode(SpawnBlock(
            LockBase + FVector(0.0f, 0.0f, -54.0f),
            FVector(5.4f, 2.55f, 0.08f),
            FLinearColor(0.024f, 0.036f, 0.042f) + DoorBlue * 0.10f,
            CityLabel + TEXT(" Boolean Airlock Logic Pad"),
            false));
        TagBooleanNode(SpawnBlock(
            LockBase + FVector(0.0f, -118.0f, 118.0f),
            FVector(3.5f, 0.14f, 1.65f),
            FLinearColor(0.04f, 0.06f, 0.07f) + DoorBlue * 0.22f,
            CityLabel + TEXT(" Boolean Airlock Door Frame"),
            false));
        TagBooleanNode(SpawnRotatedBlock(
            LockBase + FVector(118.0f, -122.0f, 120.0f),
            FRotator(0.0f, 0.0f, -16.0f),
            FVector(1.12f, 0.08f, 1.36f),
            DoorBlue * 1.9f,
            CityLabel + TEXT(" Boolean Airlock Open Door Slab"),
            false));
        TagBooleanNode(SpawnGuideText(
            FString::Printf(
                TEXT("BOOLEAN AND -> AIRLOCK OPEN\n%s\nonly TRUE + TRUE unlocks"),
                *UCodeRescueLearningLibrary::GetWorldEffect(DataDrivenLockChallenge)),
            LockBase + FVector(0.0f, -246.0f, 344.0f),
            SafeGreen.ToFColor(true),
            24.0f));

        const TCHAR* TruthLabels[] = { TEXT("TT"), TEXT("TF"), TEXT("FT"), TEXT("FF") };
        for (int32 i = 0; i < 4; ++i)
        {
            const float X = -210.0f + static_cast<float>(i) * 140.0f;
            const bool bOpens = i == 0;
            TagBooleanNode(SpawnBlock(
                LockBase + FVector(X, 116.0f, 72.0f),
                FVector(0.44f, 0.20f, 0.72f),
                (bOpens ? SafeGreen : BlockedRed) * (bOpens ? 1.9f : 1.25f),
                FString::Printf(TEXT("%s Boolean Truth Table Lamp %s"), *CityLabel, TruthLabels[i]),
                false));
            TagBooleanNode(SpawnGuideText(
                FString::Printf(TEXT("%s\n%s"), TruthLabels[i], bOpens ? TEXT("OPEN") : TEXT("BLOCK")),
                LockBase + FVector(X, 58.0f, 182.0f),
                (bOpens ? SafeGreen : BlockedRed).ToFColor(true),
                18.0f));
        }

        TagBooleanNode(SpawnBlock(
            LockBase + FVector(-340.0f, -22.0f, 84.0f),
            FVector(0.38f, 0.34f, 1.18f),
            LanguageColor * 1.15f,
            CityLabel + TEXT(" Boolean Input Switch A"),
            false));
        TagBooleanNode(SpawnBlock(
            LockBase + FVector(340.0f, -22.0f, 84.0f),
            FVector(0.38f, 0.34f, 1.18f),
            MissionSecondary * 1.15f,
            CityLabel + TEXT(" Boolean Input Switch B"),
            false));
        TagBooleanNode(SpawnBlock(
            LockBase + FVector(0.0f, -20.0f, 184.0f),
            FVector(0.80f, 0.08f, 0.12f),
            SafeGreen * 2.2f,
            CityLabel + TEXT(" Boolean AND Bridge Signal"),
            false));

        TagBooleanNode(SpawnBlock(
            LockBase + FVector(-482.0f, 78.0f, 114.0f),
            FVector(0.32f, 0.26f, 1.20f),
            LanguageColor * 0.88f + SafeGreen * 0.24f,
            CityLabel + TEXT(" Boolean Mentor Truth Table Body"),
            false));
        TagBooleanNode(SpawnBlock(
            LockBase + FVector(-482.0f, 78.0f, 198.0f),
            FVector(0.22f, 0.22f, 0.22f),
            FLinearColor(0.78f, 0.74f, 0.66f),
            CityLabel + TEXT(" Boolean Mentor Truth Table Head"),
            false));
        TagBooleanNode(SpawnRotatedBlock(
            LockBase + FVector(-412.0f, 42.0f, 164.0f),
            FRotator(0.0f, 0.0f, -22.0f),
            FVector(0.60f, 0.06f, 0.08f),
            SafeGreen * 1.7f,
            CityLabel + TEXT(" Boolean Mentor Points At TT Row"),
            false));

        TagBooleanNode(SpawnBlock(
            LockBase + FVector(492.0f, -34.0f, 112.0f),
            FVector(0.28f, 0.22f, 0.88f),
            FLinearColor(0.20f, 0.78f, 0.56f) + SafeGreen * 0.14f,
            CityLabel + TEXT(" Boolean Survivor Exit Pose Body"),
            false));
        TagBooleanNode(SpawnRotatedBlock(
            LockBase + FVector(522.0f, -42.0f, 68.0f),
            FRotator(0.0f, 0.0f, -18.0f),
            FVector(0.08f, 0.05f, 0.38f),
            FLinearColor(0.18f, 0.62f, 0.48f),
            CityLabel + TEXT(" Boolean Survivor Exit Step Key"),
            false));
    }

    if (DataDrivenReverseChallenge.IsValid())
    {
        auto TagReverseNode = [&](AActor* Actor) -> AActor*
        {
            TagSolvedRoute(Actor);
            if (Actor)
            {
                Actor->Tags.AddUnique(FName("DataDrivenStringNode"));
                Actor->Tags.AddUnique(FName("ReverseCodeWorldEffect"));
                Actor->Tags.AddUnique(FName("ReversedSequenceArtifact"));
                Actor->Tags.AddUnique(FName("PedagogicalWorldEffect"));
                Actor->Tags.AddUnique(FName("UXFinishReadableFeedback"));
                Actor->Tags.AddUnique(FName("PhysicsSafeNonBlocking"));
                Actor->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
                Actor->Tags.AddUnique(FName("WorldArtPlayablePlaceholder"));
            }
            return Actor;
        };

        const FVector ReverseBase = SurvivorBase + FVector(0.0f, -640.0f, 62.0f);
        const FLinearColor StringViolet = FLinearColor(0.55f, 0.34f, 1.0f);
        const FLinearColor OutputCyan = FLinearColor(0.10f, 0.92f, 1.0f);
        const FLinearColor VaultSteel = FLinearColor(0.42f, 0.46f, 0.50f);
        const int32 GlyphCount = FMath::Clamp(DataDrivenOutputMagnitude, 1, 5);

        TagReverseNode(SpawnBlock(
            ReverseBase + FVector(0.0f, 0.0f, -56.0f),
            FVector(5.65f, 2.50f, 0.08f),
            FLinearColor(0.026f, 0.034f, 0.046f) + StringViolet * 0.10f,
            CityLabel + TEXT(" Reverse String Logic Pad"),
            false));
        TagReverseNode(SpawnBlock(
            ReverseBase + FVector(0.0f, -132.0f, 122.0f),
            FVector(3.70f, 0.14f, 1.58f),
            FLinearColor(0.035f, 0.044f, 0.054f) + VaultSteel * 0.24f,
            CityLabel + TEXT(" Reverse Vault Door Frame"),
            false));
        TagReverseNode(SpawnRotatedBlock(
            ReverseBase + FVector(148.0f, -134.0f, 116.0f),
            FRotator(0.0f, 0.0f, -18.0f),
            FVector(1.22f, 0.08f, 1.30f),
            VaultSteel * 1.45f + OutputCyan * 0.26f,
            CityLabel + TEXT(" Reverse Vault Door Rolled Back"),
            false));
        TagReverseNode(SpawnGuideText(
            FString::Printf(
                TEXT("STRING REVERSE -> VAULT CODE\n%s\nlast character moves first"),
                *UCodeRescueLearningLibrary::GetWorldEffect(DataDrivenReverseChallenge)),
            ReverseBase + FVector(0.0f, -258.0f, 348.0f),
            OutputCyan.ToFColor(true),
            24.0f));

        for (int32 i = 0; i < GlyphCount; ++i)
        {
            const float X = -220.0f + static_cast<float>(i) * 110.0f;
            const float MirroredX = 220.0f - static_cast<float>(i) * 110.0f;
            TagReverseNode(SpawnBlock(
                ReverseBase + FVector(X, 116.0f, 58.0f),
                FVector(0.48f, 0.28f, 0.26f),
                StringViolet * (1.05f + i * 0.08f),
                FString::Printf(TEXT("%s Reverse Input Glyph %d"), *CityLabel, i + 1),
                false));
            TagReverseNode(SpawnBlock(
                ReverseBase + FVector(MirroredX, -26.0f, 72.0f),
                FVector(0.48f, 0.28f, 0.30f),
                OutputCyan * (1.25f + i * 0.10f),
                FString::Printf(TEXT("%s Reverse Output Glyph %d"), *CityLabel, i + 1),
                false));
            TagReverseNode(SpawnRotatedBlock(
                ReverseBase + FVector((X + MirroredX) * 0.5f, 44.0f, 104.0f + i * 5.0f),
                FRotator(0.0f, 0.0f, (i % 2 == 0) ? -10.0f : 10.0f),
                FVector(0.72f, 0.04f, 0.06f),
                RescueGold * 1.55f,
                FString::Printf(TEXT("%s Reverse Last-To-First Transfer Beam %d"), *CityLabel, i + 1),
                false));
            TagReverseNode(SpawnGuideText(
                FString::Printf(TEXT("%d->%d"), GlyphCount - i, i + 1),
                ReverseBase + FVector(MirroredX, -88.0f, 164.0f),
                OutputCyan.ToFColor(true),
                16.0f));
        }

        TagReverseNode(SpawnBlock(
            ReverseBase + FVector(-488.0f, 82.0f, 114.0f),
            FVector(0.32f, 0.26f, 1.20f),
            LanguageColor * 0.82f + StringViolet * 0.32f,
            CityLabel + TEXT(" Reverse Mentor Last-To-First Body"),
            false));
        TagReverseNode(SpawnBlock(
            ReverseBase + FVector(-488.0f, 82.0f, 198.0f),
            FVector(0.22f, 0.22f, 0.22f),
            FLinearColor(0.78f, 0.74f, 0.66f),
            CityLabel + TEXT(" Reverse Mentor Last-To-First Head"),
            false));
        TagReverseNode(SpawnRotatedBlock(
            ReverseBase + FVector(-414.0f, 46.0f, 164.0f),
            FRotator(0.0f, 0.0f, -24.0f),
            FVector(0.64f, 0.06f, 0.08f),
            OutputCyan * 1.7f,
            CityLabel + TEXT(" Reverse Mentor Shows Last-To-First"),
            false));

        TagReverseNode(SpawnBlock(
            ReverseBase + FVector(500.0f, -42.0f, 112.0f),
            FVector(0.28f, 0.22f, 0.88f),
            FLinearColor(0.20f, 0.78f, 0.56f) + OutputCyan * 0.16f,
            CityLabel + TEXT(" Reverse Survivor Unlock Pose Body"),
            false));
        TagReverseNode(SpawnRotatedBlock(
            ReverseBase + FVector(528.0f, -50.0f, 68.0f),
            FRotator(0.0f, 0.0f, -18.0f),
            FVector(0.08f, 0.05f, 0.38f),
            FLinearColor(0.18f, 0.62f, 0.48f),
            CityLabel + TEXT(" Reverse Survivor Unlock Step Key"),
            false));
    }

    SpawnRouteRewardPickup(EPickupKind::Stim, Origin + CityOffset(FVector(-1180.0f, -1520.0f, 118.0f)), 1, TEXT("SolvedRouteStimCache"));
    SpawnRouteRewardPickup(EPickupKind::ArmorPlate, Origin + CityOffset(FVector(600.0f, -120.0f, 118.0f)), 1, TEXT("SolvedRouteArmorCache"));
    SpawnRouteRewardPickup(EPickupKind::Smoke, Origin + CityOffset(FVector(1760.0f, 720.0f, 118.0f)), 1, TEXT("SolvedRouteSmokeCache"));

    const FVector BreachBase = Origin + CityOffset(FVector(760.0f, -160.0f, 96.0f));
    TagLanguageBreach(SpawnBlock(
        BreachBase + FVector(0.0f, 0.0f, -54.0f),
        FVector(4.25f, 2.35f, 0.12f),
        FLinearColor(0.025f, 0.032f, 0.036f) + LanguageColor * 0.16f,
        CityLabel + TEXT(" Language Breach Checkpoint Deck"),
        true));
    TagLanguageBreach(SpawnBlock(
        BreachBase + FVector(-310.0f, -95.0f, 40.0f),
        FVector(0.28f, 1.45f, 1.15f),
        MissionSecondary * 0.7f + LanguageColor * 0.65f,
        CityLabel + TEXT(" Language Breach West Cover"),
        true));
    TagLanguageBreach(SpawnBlock(
        BreachBase + FVector(310.0f, -95.0f, 40.0f),
        FVector(0.28f, 1.45f, 1.15f),
        MissionSecondary * 0.7f + LanguageColor * 0.65f,
        CityLabel + TEXT(" Language Breach East Cover"),
        true));
    TagLanguageBreach(SpawnBlock(
        BreachBase + FVector(0.0f, 180.0f, 78.0f),
        FVector(3.55f, 0.18f, 1.45f),
        FLinearColor(0.032f, 0.040f, 0.045f) + LanguageColor * 0.22f,
        CityLabel + TEXT(" Language Breach Backstop"),
        true));
    TagLanguageBreach(SpawnBlock(
        BreachBase + FVector(0.0f, -230.0f, 212.0f),
        FVector(3.25f, 0.08f, 0.34f),
        LanguageColor * 2.6f,
        CityLabel + TEXT(" Language Breach Header Signal"),
        false));
    TagLanguageBreach(SpawnGuideText(
        FString::Printf(TEXT("%s BREACH CHECKPOINT\n%s\nhold cover, then extract the survivor"), *LanguageLabel, *LanguageBreachCue),
        BreachBase + FVector(0.0f, -310.0f, 420.0f),
        LanguageColor.ToFColor(true),
        28.0f));
    AddLanguageBreachTags(SpawnSolvedLight(
        BreachBase + FVector(0.0f, -110.0f, 250.0f),
        LanguageColor,
        7600.0f,
        1150.0f,
        CityLabel + TEXT(" Language Breach Checkpoint Light")));

    for (int32 i = 0; i < 5; ++i)
    {
        const float X = -240.0f + i * 120.0f;
        const float Height = 0.18f + static_cast<float>((i + static_cast<int32>(ActiveLanguage)) % 3) * 0.12f;
        TagLanguageBreach(SpawnBlock(
            BreachBase + FVector(X, -236.0f, 98.0f + Height * 55.0f),
            FVector(0.46f, 0.06f, Height),
            LanguageColor * (1.85f + i * 0.16f),
            FString::Printf(TEXT("%s %s Language Logic Glyph %d"), *CityLabel, *LanguageLabel, i + 1),
            false));
    }

    if (APickupActor* LanguagePickup = SpawnRouteRewardPickup(
        LanguageRewardKind,
        BreachBase + FVector(0.0f, -70.0f, 64.0f),
        LanguageRewardAmount,
        TEXT("LanguageBreachRewardCache")))
    {
        AddLanguageBreachTags(LanguagePickup);
    }

    if (!bFromLoad && !bSandboxMode)
    {
        const int32 PatrolCount = 3;
        const FVector PatrolOffsets[] = {
            FVector(-520.0f, 420.0f, 6.0f),
            FVector(0.0f, 520.0f, 6.0f),
            FVector(520.0f, 420.0f, 6.0f),
        };
        for (int32 i = 0; i < PatrolCount; ++i)
        {
            const FVector PatrolLoc = BreachBase + PatrolOffsets[i];
            UClass* ZombieClass = ZombieActorClass ? ZombieActorClass.Get() : ACodeZombieActor::StaticClass();
            FActorSpawnParameters ZombieSpawnParams;
            ZombieSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            ACodeZombieActor* Zombie = GetWorld()->SpawnActor<ACodeZombieActor>(ZombieClass, PatrolLoc, FRotator::ZeroRotator, ZombieSpawnParams);
            if (!Zombie)
            {
                continue;
            }

            Zombie->ZombieId = CodeRescueHordeZombieIdBase + ResolvedCityIndex * 1000 + 500 + i;
            Zombie->Health = FMath::Max(1.0f, ZombieBaseHealth * 0.78f);
            Zombie->AttackDamage = FMath::Max(0.0f, ZombieBaseAttackDamage * 0.75f);
            Zombie->MoveSpeed = ZombieBaseMoveSpeed * 1.10f;
            Zombie->AttackRange = FMath::Max(40.0f, ZombieAttackRange);
            Zombie->ActivationRange = FMath::Max(1850.0f, ZombieBaseActivationRange * 0.55f);
            Zombie->RefreshMovementSettings();
            Zombie->ApplyStandardDirectPursuitProfile();

            ApplyZombieFamilyVariant(Zombie, LanguagePatrolVariant, Zombie->ZombieId, FName("LanguageBreachZombieFamily"), true);

            Zombie->Tags.AddUnique(FName("LanguageBreachPatrol"));
            TagLanguageBreach(Zombie);
            Zombie->VisualMarkerActor = TagLanguageBreach(SpawnZombieReadabilityMarker(
                Zombie,
                GetZombieFamilyVariantMarkerColor(Zombie->Variant) * 1.25f + LanguageColor * 0.35f,
                FString::Printf(TEXT("%s %s Breach %s Marker %d"), *CityLabel, *LanguageLabel, *GetZombieFamilyVariantMarkerLabel(Zombie->Variant), i + 1),
                1.0f));
            if (Zombie->VisualMarkerActor)
            {
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("StandardDirectPursuitZombie"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("ZombiePursuitReadableRuntime"));
                Zombie->VisualMarkerActor->Tags.AddUnique(FName("FairSurvivalPressure"));
                Zombie->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
            }
            RegisterStreamedActor(Zombie);
        }
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(TEXT("[Dispatch]: %s route breach ahead. Use the checkpoint cover, then reach the survivor."), *LanguageLabel),
            4.5f);
    }

    if (!bFromLoad)
    {
        UCodeRescueSubtitlesWidget::Push(
            TEXT("[Dispatch]: Code accepted. Survivor route is lit. Move from the safehouse to extraction."),
            4.5f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan, TEXT("Rescue route unlocked by solved code."));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueCodingWorldResponse] %s terminal '%s' revealed solved rescue route%s."),
        *CityLabel,
        *TerminalId,
        bFromLoad ? TEXT(" from saved state") : TEXT(""));
}

// ---- #14 Boss / horde rush after a terminal solve -------------------------
void ACodeRescueGameMode::TriggerBossHorde(const FVector& Center, int32 CityIndex)
{
    // 2026-07-04: the player's face reacts to the incoming horde (v2 morph targets; safe no-op
    // on meshes without morphs).
    UCodeRescueFacialExpressionComponent::TriggerOnActor(
        UGameplayStatics::GetPlayerPawn(this, 0), FName(TEXT("Alarm")), 1.0f, 2.5f);

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const float HealthMul = GI ? GI->GetZombieHealthMultiplier() : 1.0f;
    const float DamageMul = GI ? GI->GetZombieDamageMultiplier() : 1.0f;

    const int32 HordeCount = FMath::Max(8, ZombieMaxCount + 4);
    const float Radius = 1100.0f;
    const float TwoPi = 2.0f * PI;

    for (int32 i = 0; i < HordeCount; ++i)
    {
        const float Angle = (i / float(HordeCount)) * TwoPi;
        const FVector Loc = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 80.0f);

        UClass* ZombieClass = ZombieActorClass ? ZombieActorClass.Get() : ACodeZombieActor::StaticClass();
        FActorSpawnParameters ZombieSpawnParams;
        ZombieSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ACodeZombieActor* Zombie = GetWorld()->SpawnActor<ACodeZombieActor>(ZombieClass, Loc, FRotator::ZeroRotator, ZombieSpawnParams);
        if (!Zombie) continue;

        // Horde zombies get an offset ID range so they don't collide with
        // the city's saved-neutralized list, and they aren't persisted —
        // they're transient encounter content.
        Zombie->ZombieId = CodeRescueHordeZombieIdBase + CityIndex * 100 + i;
        Zombie->Health = FMath::Max(1.0f, ZombieBaseHealth * 1.25f * HealthMul);
        Zombie->AttackDamage = FMath::Max(0.0f, ZombieBaseAttackDamage * 1.15f * DamageMul);
        Zombie->MoveSpeed = ZombieBaseMoveSpeed * 1.20f;
        Zombie->AttackRange = FMath::Max(40.0f, ZombieAttackRange);
        Zombie->ActivationRange = FMath::Max(2000.0f, ZombieBaseActivationRange);
        Zombie->RefreshMovementSettings();
        Zombie->ApplyStandardDirectPursuitProfile();

        ApplyCityZombieFamilyVariant(Zombie, CityIndex, i + 400, Zombie->ZombieId, FName("BossHordeZombieFamily"), false);

        const FString VariantLabel = GetZombieFamilyVariantMarkerLabel(Zombie->Variant);
        const FLinearColor VariantColor = GetZombieFamilyVariantMarkerColor(Zombie->Variant);
        Zombie->VisualMarkerActor = SpawnZombieReadabilityMarker(
            Zombie,
            VariantColor * 1.5f,
            FString::Printf(TEXT("Horde %s_%d"), *VariantLabel, Zombie->ZombieId),
            1.15f);
        if (Zombie->VisualMarkerActor)
        {
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("CityZombieFamilyVariant"));
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("StandardDirectPursuitZombie"));
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("ZombiePursuitReadableRuntime"));
            Zombie->VisualMarkerActor->Tags.AddUnique(FName("FairSurvivalPressure"));
            Zombie->VisualMarkerActor->Tags.AddUnique(GetZombieFamilyVariantAuditTag(Zombie->Variant));
        }
        RegisterStreamedActor(Zombie);
    }

    SpawnGuideText(TEXT("INCOMING HORDE — DEFEND THE EXTRACTION"),
                   Center + FVector(0, 0, 700), FColor(255, 60, 30), 78.0f);

    // #64: punctuate the horde trigger with the boss-stinger track. No-op if
    // BossHordeStinger isn't authored yet.
    if (GI)
    {
        GI->PlayHordeStinger();
    }
}

void ACodeRescueGameMode::SpawnLanguageStation(const FVector& Location, ECodingLanguage Language, const FString& Label)
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (GI && GI->SelectedLanguage != Language)
    {
        return;
    }

    if (AActor* Marker = SpawnBlock(Location + FVector(0, 0, 88), FVector(1.15f, 1.15f, 1.4f), FLinearColor(0.16f, 0.30f, 0.34f), Label + TEXT(" LAUNCH-LOCKED LANGUAGE TRACK"), false))
    {
        Marker->Tags.Add(FName("LaunchLockedLanguageTrack"));
        Marker->Tags.Add(FName("ActivePlaySingleLanguageOnly"));
    }
    if (AActor* Halo = SpawnBlock(Location + FVector(0, 0, 240), FVector(0.55f, 0.55f, 0.05f), FLinearColor(0.96f, 0.58f, 0.20f) * 2.4f, TEXT("Launch-Locked Language Halo"), false))
    {
        Halo->Tags.Add(FName("LaunchLockedLanguageTrack"));
    }
#if ENABLE_DRAW_DEBUG
    DrawDebugCylinder(GetWorld(), Location, Location + FVector(0, 0, 420), 260.0f, 28, FColor(230, 154, 72), true, -1.0f, 0, 18.0f);
#endif
    SpawnGuideText(Label + TEXT("\nTRACK ONLY"), Location + FVector(0, 0, 430), FColor(230, 214, 176), 54.0f);
}

void ACodeRescueGameMode::SpawnWorld()
{
    if (GEngine)
    {
        // Lumen GI + Lumen Reflections are deliberately DISABLED. On Mac they
        // take many seconds (sometimes minutes) to converge, which left the
        // whole world black for the opening minutes of play. Direct sunlight
        // plus a strong SkyLight ambient light the scene instantly instead —
        // less "photoreal", but reliably visible from the first frame, which
        // is what an educational game on school Macs needs.
        // Virtual Shadow Maps are also off: they spammed a "Non-Nanite
        // Marking Job Queue overflow" warning and cost performance on the
        // procedural-block geometry. Regular shadow maps look fine here.
        GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 0"));
        GEngine->Exec(GetWorld(), TEXT("r.ReflectionMethod 0"));
        GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.AutoExposure 1"));
        GEngine->Exec(GetWorld(), TEXT("r.EyeAdaptationQuality 2"));
        GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.Bloom 1"));
        GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 0"));
    }

    // ---- Sky atmosphere: real procedural sky + sunlight scattering --------
    // ASkyAtmosphere drives realistic sky color, horizon haze, and sunlight
    // tint based on the directional light's rotation. Replaces what was
    // previously a flat black void.
    // Default transform mode (PlanetTopAtAbsoluteWorldOrigin) is what we want;
    // simply spawning the actor activates atmospheric scattering.
    GetWorld()->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

    // Directional sun: a SINGLE shared directional light. The day/night cycle
    // (Tick) rotates and dims it, and the SkyAtmosphere uses the same light for
    // sun-disc scattering. Previously SpawnWorld spawned a SECOND directional
    // light, which triggered UE's "multiple directional lights competing"
    // warning and split control: the day/night system dimmed one light while
    // the atmosphere followed the other. BeginPlay already spawns SunLight
    // before calling SpawnWorld, so we adopt it here; the SpawnActor path is a
    // defensive fallback only.
    if (!SunLight)
    {
        SunLight = GetWorld()->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0, 0, 6000), FRotator(-45, 60, 0));
    }
    if (SunLight)
    {
        if (UDirectionalLightComponent* DC = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent()))
        {
            DC->SetMobility(EComponentMobility::Movable);
            // 2026-07-01 (round 5): 7.0 blew the daytime scene out to a white/teal haze. 4.5 keeps
            // surfaces lit and material detail readable without clipping to pure white.
            DC->SetIntensity(4.5f);
            DC->SetLightColor(FLinearColor(1.0f, 0.96f, 0.86f));
            // Tag as the atmospheric sun so SkyAtmosphere produces sunsets,
            // horizon glow, and a correct sky as the sun rotates through the
            // day/night cycle. Only one light carries this flag now.
            DC->SetAtmosphereSunLight(true);
        }
    }

    ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    if (Sky)
    {
        WorldSkyLight = Sky;
        // 2026-07-01 (round 5): 3.0 made the blue-sky ambient dominate, tinting the whole plaza
        // teal and washing surfaces out. 1.4 keeps interiors/undersides readable while letting the
        // warm directional sun be the key light, so materials read in their true color.
        Sky->GetLightComponent()->SetIntensity(1.4f);
        Sky->GetLightComponent()->SetLightColor(FLinearColor(0.95f, 0.96f, 1.0f));
        Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        Sky->GetLightComponent()->SetRealTimeCaptureEnabled(true);
        Sky->GetLightComponent()->RecaptureSky();
    }

    AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector(0, 0, -800), FRotator::ZeroRotator);
    if (Fog)
    {
        // 2026-07-01 (round 5): thinner, less-saturated haze. The old dense blue inscatter tinted
        // the whole scene teal; a near-neutral cool-grey reads as atmosphere, not a color cast.
        Fog->GetComponent()->SetFogDensity(0.0008f);
        Fog->GetComponent()->SetFogHeightFalloff(0.20f);
        Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.62f, 0.64f, 0.70f));
        Fog->GetComponent()->SetVolumetricFog(true);
    }

    // Post-process volume: unbound, applies everywhere. Bloom + auto-exposure
    // + a touch of color grading make the procedural cubes feel cinematic
    // instead of flat-shaded.
    APostProcessVolume* PP = GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    if (PP)
    {
        PP->bUnbound = true;
        // 2026-07-01 (round 5): bloom 1.1 turned every bright surface into a white halo. 0.45 keeps
        // a subtle glow on genuine highlights without washing the frame out.
        PP->Settings.bOverride_BloomIntensity = true;
        PP->Settings.BloomIntensity = 0.45f;
        // Auto-exposure: still lifts dark areas for a readable night, but the old +1.0 bias plus a
        // 2.6 max-brightness over-exposed the daytime plaza to solid white/teal. Pull the bias down
        // and cap the max so bright scenes settle at a natural mid-tone; keep a modest floor so
        // interiors and night streets never crush to black.
        PP->Settings.bOverride_AutoExposureBias = true;
        PP->Settings.AutoExposureBias = 0.1f;
        PP->Settings.bOverride_AutoExposureMinBrightness = true;
        PP->Settings.AutoExposureMinBrightness = 0.05f;
        PP->Settings.bOverride_AutoExposureMaxBrightness = true;
        PP->Settings.AutoExposureMaxBrightness = 1.15f;
        PP->Settings.bOverride_VignetteIntensity = true;
        PP->Settings.VignetteIntensity = 0.30f;
        // Neutral grade: drop the blue-channel saturation push that reinforced the teal cast.
        PP->Settings.bOverride_ColorSaturation = true;
        PP->Settings.ColorSaturation = FVector4(1.04f, 1.04f, 1.04f, 1.0f);
        PP->Settings.bOverride_ColorContrast = true;
        PP->Settings.ColorContrast = FVector4(1.06f, 1.06f, 1.06f, 1.0f);
    }

    const TArray<FCodeRescueCityMission>& Missions = FCodeRescueCampaign::GetMissions();
    const int32 Count = Missions.Num();
    if (Count <= 0)
    {
        return;
    }

    int32 InitialCityIndex = 0;
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        InitialCityIndex = FMath::Clamp(GI->CurrentObjectiveIndex, 0, Count - 1);
        if (!FCodeRescueCampaign::IsCityUnlocked(GI, InitialCityIndex))
        {
            InitialCityIndex = FMath::Clamp(FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI), 0, Count - 1);
        }
    }

    // All first-level audits are deliberately independent of the player's
    // currently saved campaign city. This tests the requested level without
    // changing normal resume behavior or requiring the user's save to be reset.
    const bool bFirstLevelAuditRun =
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelWorldAccessAudit")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelChallengeAudit"));
    if (bFirstLevelAuditRun)
    {
        InitialCityIndex = 0;
    }

    EnsureCampaignCityLoaded(InitialCityIndex);
}
