#include "SurvivorActor.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescueFacialExpressionComponent.h"
#include "CodeRescueGameMode.h"
#include "Animation/AnimSequence.h"
#include "CodeRescueAnimationBudget.h"
#include "CodeRescueRetargetRig.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CompanionActor.h"
#include "HelipadActor.h"
#include "RescueExtractionPresentationActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
float GetRuntimeSfxVolume(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI ? GI->GetSfxVolumeScalar() : 1.0f;
}

FVector GetMonoSafeSoundLocation(const UObject* Context, const FVector& RequestedLocation)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    if (!GI || !GI->bMonoAudio)
    {
        return RequestedLocation;
    }

    if (const AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        return PlayerActor->GetActorLocation();
    }
    return RequestedLocation;
}

FString GetMissionCityLabel(const FCodeRescueCityMission* Mission)
{
    if (!Mission)
    {
        return TEXT("this city");
    }
    if (Mission->StateName.IsEmpty())
    {
        return Mission->CityName.IsEmpty() ? FString(TEXT("this city")) : Mission->CityName;
    }
    return FString::Printf(TEXT("%s, %s"), *Mission->CityName, *Mission->StateName);
}

FString GetMissionConceptLabel(const FCodeRescueCityMission* Mission)
{
    if (!Mission)
    {
        return TEXT("active coding route");
    }
    if (!Mission->CurriculumStageName.IsEmpty())
    {
        return Mission->CurriculumStageName;
    }
    if (!Mission->TerminalTitle.IsEmpty())
    {
        return Mission->TerminalTitle;
    }
    return TEXT("active coding route");
}

FString GetMissionLandmarkLabel(const FCodeRescueCityMission* Mission)
{
    if (!Mission)
    {
        return TEXT("the survivor marker");
    }
    if (!Mission->LandmarkName.IsEmpty())
    {
        return Mission->LandmarkName;
    }
    if (!Mission->DistrictStyle.IsEmpty())
    {
        return Mission->DistrictStyle;
    }
    return TEXT("the survivor marker");
}

FString GetMissionTerminalLabel(const FCodeRescueCityMission* Mission, const FString& TerminalId)
{
    if (Mission && !Mission->TerminalTitle.IsEmpty())
    {
        return Mission->TerminalTitle;
    }
    return TerminalId.IsEmpty() ? FString(TEXT("the active terminal")) : TerminalId;
}

FString GetMissionPayoffLabel(const FCodeRescueCityMission* Mission)
{
    if (Mission && !Mission->NovelGameplayDetail.IsEmpty())
    {
        return Mission->NovelGameplayDetail;
    }
    return TEXT("the rescue beacon is back online");
}

FString GetSelectedLanguageLabel(const UCodeRescueGameInstance* GI)
{
    return GI ? GI->GetLanguageName() : FString(TEXT("selected language"));
}

FString MakeSafeSurvivorTagFragment(const FString& Source)
{
    FString Result;
    for (TCHAR Ch : Source)
    {
        Result.AppendChar(FChar::IsAlnum(Ch) ? Ch : TCHAR('_'));
    }
    while (Result.Contains(TEXT("__")))
    {
        Result.ReplaceInline(TEXT("__"), TEXT("_"));
    }
    Result.TrimStartAndEndInline();
    Result.RemoveFromStart(TEXT("_"));
    Result.RemoveFromEnd(TEXT("_"));
    return Result.IsEmpty() ? FString(TEXT("Unknown")) : Result;
}

FString BuildStoryTail(const FString& Story)
{
    return Story.IsEmpty() ? FString() : FString::Printf(TEXT(" %s"), *Story);
}

FString BuildSurvivorLockedRouteLine(
    const FString& SurvivorName,
    const FString& ArchetypeTitle,
    const FString& ArchetypeFieldNeed,
    const FString& Story,
    const FCodeRescueCityMission* Mission,
    const FString& TerminalId,
    int32 CompletedChallenges,
    const UCodeRescueGameInstance* GI)
{
    return FString::Printf(
        TEXT("[%s - %s]: Rescue clearance is %d/%d at %s. I need %s. Finish every %s station in %s, then I can move from %s.%s"),
        *SurvivorName,
        *ArchetypeTitle,
        CompletedChallenges,
        FCodeRescueCampaign::RequiredChallengesPerCity,
        *GetMissionTerminalLabel(Mission, TerminalId),
        *ArchetypeFieldNeed,
        *GetMissionConceptLabel(Mission),
        *GetSelectedLanguageLabel(GI),
        *GetMissionLandmarkLabel(Mission),
        *BuildStoryTail(Story));
}

FString BuildSurvivorRescueLine(
    const FString& SurvivorName,
    const FString& ArchetypeTitle,
    const FString& ArchetypeRescueSkill,
    const FString& ArchetypeDossierHook,
    const FString& Story,
    const FCodeRescueCityMission* Mission,
    const UCodeRescueGameInstance* GI)
{
    return FString::Printf(
        TEXT("[%s - %s]: Your %s fix held. %s opened %s; %s. %s %s%s"),
        *SurvivorName,
        *ArchetypeTitle,
        *GetSelectedLanguageLabel(GI),
        *GetMissionConceptLabel(Mission),
        *GetMissionLandmarkLabel(Mission),
        *GetMissionPayoffLabel(Mission),
        *ArchetypeRescueSkill,
        *ArchetypeDossierHook,
        *BuildStoryTail(Story));
}

FString BuildExtractionDispatchLine(
    const FString& SurvivorName,
    const FString& ArchetypeTitle,
    const FCodeRescueCityMission* Mission,
    const UCodeRescueGameInstance* GI)
{
    return FString::Printf(
        TEXT("[Dispatch]: %s (%s) rescued in %s. Helipad route is live, %s save updated, journal dossier marked RESCUED."),
        *SurvivorName,
        *ArchetypeTitle,
        *GetMissionCityLabel(Mission),
        *GetSelectedLanguageLabel(GI));
}

FString BuildCompanionHandoffLine(
    const FString& SurvivorName,
    const FString& ArchetypeTitle,
    const FCodeRescueCityMission* Mission)
{
    return FString::Printf(
        TEXT("[%s - %s]: I'll cover the %s route. Beacon first, then next city."),
        *SurvivorName,
        *ArchetypeTitle,
        *GetMissionConceptLabel(Mission));
}
}

ASurvivorActor::ASurvivorActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Body: torso. Slightly narrower + taller than before so head + body
    // proportions read as a humanoid silhouette.
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveBody"));
    RootComponent = Body;
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Body->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    Body->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));
    Body->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.40f));

    // Head: separate sphere on top of the body. Color is applied at BeginPlay
    // via a dynamic material so we get a warm skin tone vs. a flat gray.
    Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveHead"));
    Head->SetupAttachment(Body);
    // Body extends ~70 units above pivot (cube=100*1.40 scale, half=70). Place
    // head sphere just above that. Sphere mesh is 100 units; scale 0.55 = 55
    // units diameter. Offset Z so its bottom touches the top of the body.
    Head->SetRelativeLocation(FVector(0, 0, 95));
    Head->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.55f));
    Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SkeletalBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ProfessionalSkeletalBody"));
    SkeletalBody->SetupAttachment(Body);
    SkeletalBody->SetVisibility(false);
    SkeletalBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletalBody->SetRelativeLocation(FVector(0.0f, 0.0f, -64.0f));
    SkeletalBody->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    SkeletalBody->SetRelativeScale3D(FVector(1.0f / 0.55f, 1.0f / 0.55f, 1.0f / 1.40f));
    CodeRescueAnimationBudget::ApplySkeletalMeshBudget(
        SkeletalBody, ECodeRescueAnimationBudgetProfile::HeroNPC, this);
    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        SkeletalBody, ECodeRescueRetargetRigProfile::SurvivorHero, this);

    RescueLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RescueLight"));
    RescueLight->SetupAttachment(Body);
    RescueLight->SetRelativeLocation(FVector(0, 0, 130));
    RescueLight->SetLightColor(FLinearColor(0.1f, 0.8f, 1.0f));
    // Brighter, longer-reach rescue beacon so survivors are easy to spot
    // from across the city. Color stays cyan to match the HUD crosshair
    // and minimap survivor color-coding.
    RescueLight->SetIntensity(3000.0f);
    RescueLight->SetAttenuationRadius(750.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CubeMesh.Succeeded())  Body->SetStaticMesh(CubeMesh.Object);
    if (SphereMesh.Succeeded()) Head->SetStaticMesh(SphereMesh.Object);
}

void ASurvivorActor::ConfigureArchetypeFromMission(const FCodeRescueCityMission& Mission)
{
    const FCodeRescueSurvivorArchetypeProfile Profile = FCodeRescueCampaign::GetSurvivorArchetypeProfile(Mission);
    ArchetypeTitle = Profile.Title;
    ArchetypeIconLabel = Profile.IconLabel;
    ArchetypeFieldNeed = Profile.FieldNeed;
    ArchetypeRescueSkill = Profile.RescueSkill;
    ArchetypeDossierHook = Profile.DossierHook;
    ArchetypeAccentColor = Profile.AccentColor;

    Tags.AddUnique(FName("SurvivorArchetypeRosterRuntime"));
    Tags.AddUnique(FName("SurvivorRoleReadableNameplate"));
    Tags.AddUnique(FName("SelectedLanguageSurvivorHandoff"));
    Tags.AddUnique(FName(*FString::Printf(TEXT("SurvivorArchetype_%s"), *MakeSafeSurvivorTagFragment(ArchetypeTitle))));
    Tags.AddUnique(FName(*FString::Printf(TEXT("SurvivorRoleIcon_%s"), *MakeSafeSurvivorTagFragment(ArchetypeIconLabel))));
}

FString ASurvivorActor::GetSurvivorArchetypeSummary() const
{
    return FString::Printf(
        TEXT("%s | %s | needs %s | %s"),
        *SurvivorName,
        *ArchetypeTitle,
        *ArchetypeFieldNeed,
        *ArchetypeRescueSkill);
}

FString ASurvivorActor::GetInteractionPrompt() const
{
    if (bRescued)
    {
        return FString::Printf(TEXT("%s rescued - dossier saved"), *SurvivorName);
    }

    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    const bool bLocked = !FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    if (bLocked)
    {
        return FString::Printf(
            TEXT("[E] %s locked - coding clearance %d/%d"),
            *ArchetypeTitle,
            CompletedChallenges,
            FCodeRescueCampaign::RequiredChallengesPerCity);
    }
    return FString::Printf(TEXT("[E] rescue %s - %s"), *ArchetypeTitle, *ArchetypeIconLabel);
}

void ASurvivorActor::BeginPlay()
{
    Super::BeginPlay();

    // 2026-07-07 elevation fix / upgraded 2026-07-11: survivors must STAND ON
    // the layered street surfaces, not float above or sink into them. The
    // shared robust snap measures to the lowest visible mesh (feet), ignores
    // catch floors and other characters, and retries on ECC_WorldStatic.
    ACodeRescueGameMode::SnapCharacterBaseToGround(this);

    Tags.AddUnique(FName("SurvivorArchetypeRosterRuntime"));
    Tags.AddUnique(FName("SurvivorRoleReadableNameplate"));
    Tags.AddUnique(FName("SelectedLanguageSurvivorHandoff"));
    Tags.AddUnique(FName(*FString::Printf(TEXT("SurvivorArchetype_%s"), *MakeSafeSurvivorTagFragment(ArchetypeTitle))));
    Tags.AddUnique(FName(*FString::Printf(TEXT("SurvivorRoleIcon_%s"), *MakeSafeSurvivorTagFragment(ArchetypeIconLabel))));
    RescueLight->SetLightColor(ArchetypeAccentColor);
    RescueLight->SetIntensity(bRescued ? 0.0f : 3600.0f);

    // Use the complete Quinn locomotion rig in production. The procedural Maya
    // study remains opt-in for asset review rather than replacing a tested
    // skeletal hierarchy in the public game.
    UAnimSequence* V2SurvivorIdle = nullptr;
    bool bV2SurvivorBody = false;
    const bool bUsePrototypeCharacter =
        FParse::Param(FCommandLine::Get(), TEXT("CodeRescueUsePrototypeCharacters"));
    if (!ProfessionalSurvivorMesh && !bUsePrototypeCharacter)
    {
        // 2026-07-11 v3: the authored medic survivor (satchel, shoulder patch,
        // morph face, celebration Wave) is now the DEFAULT body; the grey
        // mannequin only remains as the asset-missing fallback below.
        ProfessionalSurvivorMesh = LoadObject<USkeletalMesh>(nullptr,
            TEXT("/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3.SurvivorMayaV3"));
        if (ProfessionalSurvivorMesh)
        {
            auto LoadMayaAnim = [](const TCHAR* ActionName) -> UAnimSequence*
            {
                const FString Candidates[3] = {
                    FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3_Anim_SurvivorMayaV3_%s.SurvivorMayaV3_Anim_SurvivorMayaV3_%s"), ActionName, ActionName),
                    FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3_%s.SurvivorMayaV3_%s"), ActionName, ActionName),
                    FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/SurvivorMayaV3/SurvivorMayaV3SurvivorMayaV3_%s.SurvivorMayaV3SurvivorMayaV3_%s"), ActionName, ActionName)};
                for (const FString& Path : Candidates)
                {
                    if (UAnimSequence* Anim = LoadObject<UAnimSequence>(nullptr, *Path))
                    {
                        return Anim;
                    }
                }
                return nullptr;
            };
            V2SurvivorIdle = LoadMayaAnim(TEXT("Idle"));
            V3IdleAnim = V2SurvivorIdle;
            V3WaveAnim = LoadMayaAnim(TEXT("Wave"));
            bV2SurvivorBody = true;
            UE_LOG(LogTemp, Display, TEXT("[SurvivorV3] %s wears SurvivorMayaV3 (idle=%d wave=%d)"),
                *GetName(), V3IdleAnim != nullptr, V3WaveAnim != nullptr);
        }
        else
        {
            ProfessionalSurvivorMesh = LoadObject<USkeletalMesh>(
                nullptr,
                TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn"));
        }
    }
    if (!ProfessionalSurvivorMesh && bUsePrototypeCharacter)
    {
        ProfessionalSurvivorMesh = LoadObject<USkeletalMesh>(
            nullptr, TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorMaya.SurvivorMaya"));
        if (ProfessionalSurvivorMesh)
        {
            V2SurvivorIdle = LoadObject<UAnimSequence>(nullptr,
                TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorMayaSurvivorMaya_Idle.SurvivorMayaSurvivorMaya_Idle"));
            bV2SurvivorBody = true;
        }
    }
    if (!ProfessionalSurvivorMesh)
    {
        ProfessionalSurvivorMesh = LoadObject<USkeletalMesh>(
            nullptr,
            TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn"));
    }
    if (!ProfessionalSurvivorAnimClass && !bV2SurvivorBody)
    {
        ProfessionalSurvivorAnimClass = LoadClass<UAnimInstance>(
            nullptr,
            TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C"));
    }

    if (ProfessionalSurvivorMesh)
    {
        SkeletalBody->SetSkeletalMesh(ProfessionalSurvivorMesh);
        SkeletalBody->SetVisibility(true);
        Body->SetVisibility(false);
        Head->SetVisibility(false);
        if (!bV2SurvivorBody)
        {
            // Mannequin path keeps the accent tint (authored v2 clothing already carries color).
            if (UMaterialInstanceDynamic* BodyMat = SkeletalBody->CreateAndSetMaterialInstanceDynamic(0))
            {
                BodyMat->SetVectorParameterValue(TEXT("Color"), ArchetypeAccentColor);
                BodyMat->SetVectorParameterValue(TEXT("BaseColor"), ArchetypeAccentColor);
                BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), ArchetypeAccentColor * 0.18f);
            }
        }

        if (bV2SurvivorBody)
        {
            SkeletalBody->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            if (V2SurvivorIdle)
            {
                SkeletalBody->PlayAnimation(V2SurvivorIdle, true);
            }
            if (UCodeRescueFacialExpressionComponent* Face =
                    NewObject<UCodeRescueFacialExpressionComponent>(this, TEXT("FacialExpression")))
            {
                Face->RegisterComponent();
            }
        }
        // Hook up the animation Blueprint class if Kenny assigned one in
        // editor (e.g. an AnimBP shipped with a zombie / character pack, or
        // the MetaHuman default). Without this the skeletal mesh stands in
        // T-pose. With it, the character idles / walks / etc.
        else if (ProfessionalSurvivorAnimClass)
        {
            SkeletalBody->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            SkeletalBody->SetAnimInstanceClass(ProfessionalSurvivorAnimClass);
        }
        UE_LOG(LogTemp, Display, TEXT("[CharacterPresentation] Survivor '%s' uses %s rig."),
            *SurvivorName, bV2SurvivorBody ? TEXT("prototype Maya") : TEXT("production Quinn"));
    }
    else
    {
        // Apply distinct materials to body (orange jumpsuit-ish) and head
        // (warm skin tone) so the figure reads as a person rather than a
        // single yellow block. Done at BeginPlay because dynamic materials
        // need a valid world.
        if (UMaterialInstanceDynamic* JumpMat = Body->CreateAndSetMaterialInstanceDynamic(0))
        {
            const FLinearColor Jumpsuit = ArchetypeAccentColor * 0.72f + FLinearColor(1.00f, 0.55f, 0.10f) * 0.28f;
            JumpMat->SetVectorParameterValue(TEXT("Color"),         Jumpsuit);
            JumpMat->SetVectorParameterValue(TEXT("BaseColor"),     Jumpsuit);
            JumpMat->SetVectorParameterValue(TEXT("EmissiveColor"), Jumpsuit * 0.4f);
        }
        if (UMaterialInstanceDynamic* HeadMat = Head->CreateAndSetMaterialInstanceDynamic(0))
        {
            const FLinearColor Skin = FLinearColor(0.92f, 0.74f, 0.58f);
            HeadMat->SetVectorParameterValue(TEXT("Color"),         Skin);
            HeadMat->SetVectorParameterValue(TEXT("BaseColor"),     Skin);
            HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), Skin * 0.15f);
        }
    }

    if (RescueBeaconVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(RescueBeaconVFX, RootComponent, NAME_None, FVector(0, 0, 100), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
    }

    CacheGestureBasePose(true);

    // #11 — start idle-bark loop (no-op if cue is unset).
    ScheduleNextIdleBark();
}

void ASurvivorActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateSurvivorGesture(DeltaSeconds);
}

void ASurvivorActor::CacheGestureBasePose(bool bForce)
{
    if (!bForce && bGestureBasePoseCached)
    {
        return;
    }

    if (SkeletalBody)
    {
        SkeletalGestureBaseLocation = SkeletalBody->GetRelativeLocation();
        SkeletalGestureBaseRotation = SkeletalBody->GetRelativeRotation();
        SkeletalGestureBaseScale = SkeletalBody->GetRelativeScale3D();
        SkeletalBody->ComponentTags.AddUnique(FName("SurvivorGestureReadabilityComponent"));
    }
    if (Head)
    {
        HeadGestureBaseLocation = Head->GetRelativeLocation();
        HeadGestureBaseRotation = Head->GetRelativeRotation();
        HeadGestureBaseScale = Head->GetRelativeScale3D();
        Head->ComponentTags.AddUnique(FName("SurvivorGestureReadabilityComponent"));
    }
    if (RescueLight)
    {
        LightGestureBaseLocation = RescueLight->GetRelativeLocation();
        LightGestureBaseIntensity = RescueLight->Intensity;
        RescueLight->ComponentTags.AddUnique(FName("SurvivorGestureReadabilityComponent"));
    }

    Tags.AddUnique(FName("SurvivorGestureReadabilityRuntime"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    bGestureBasePoseCached = true;
}

void ASurvivorActor::UpdateSurvivorGesture(float DeltaSeconds)
{
    if (!bEnableSurvivorGestureReadability)
    {
        return;
    }

    CacheGestureBasePose();

    LockedGestureTimer = FMath::Max(0.0f, LockedGestureTimer - DeltaSeconds);
    RescueGestureTimer = FMath::Max(0.0f, RescueGestureTimer - DeltaSeconds);

    const float IdleScale = bRescued ? 0.0f : FMath::Clamp(SurvivorIdleGestureScale, 0.0f, 2.0f);
    const float LockedAlpha = LockedGestureDuration > KINDA_SMALL_NUMBER
        ? FMath::Clamp(LockedGestureTimer / LockedGestureDuration, 0.0f, 1.0f)
        : 0.0f;
    const float RescueAlpha = RescueGestureDuration > KINDA_SMALL_NUMBER
        ? FMath::Clamp(RescueGestureTimer / RescueGestureDuration, 0.0f, 1.0f)
        : 0.0f;
    const float RescueCurve = FMath::Sin((1.0f - RescueAlpha) * PI);

    GesturePhase += DeltaSeconds * (1.25f + LockedAlpha * 5.0f + RescueAlpha * 4.0f);
    const float IdleBob = FMath::Sin(GesturePhase * 2.0f) * 2.5f * IdleScale;
    const float IdleTilt = FMath::Sin(GesturePhase) * 1.8f * IdleScale;
    const float LockedShake = FMath::Sin(GesturePhase * 9.0f) * 4.5f * LockedAlpha;

    const FVector SharedOffset(
        -7.0f * LockedAlpha + 7.0f * RescueCurve,
        LockedShake,
        IdleBob + 24.0f * RescueCurve);
    const FRotator SharedRotation(
        -3.0f * IdleScale - 8.0f * LockedAlpha + 10.0f * RescueCurve,
        0.0f,
        IdleTilt + LockedShake * 0.65f + 6.0f * RescueCurve);
    const float RescuePulse = 1.0f + 0.055f * RescueCurve - 0.018f * LockedAlpha;

    if (SkeletalBody && !SkeletalBody->IsSimulatingPhysics())
    {
        SkeletalBody->SetRelativeLocation(SkeletalGestureBaseLocation + SharedOffset);
        SkeletalBody->SetRelativeRotation(FRotator(
            SkeletalGestureBaseRotation.Pitch + SharedRotation.Pitch,
            SkeletalGestureBaseRotation.Yaw,
            SkeletalGestureBaseRotation.Roll + SharedRotation.Roll));
        SkeletalBody->SetRelativeScale3D(SkeletalGestureBaseScale * RescuePulse);
    }
    if (Head && !Head->IsSimulatingPhysics())
    {
        Head->SetRelativeLocation(HeadGestureBaseLocation + SharedOffset + FVector(0.0f, LockedShake * 0.35f, 4.0f * RescueCurve));
        Head->SetRelativeRotation(FRotator(
            HeadGestureBaseRotation.Pitch + SharedRotation.Pitch * 0.55f,
            HeadGestureBaseRotation.Yaw,
            HeadGestureBaseRotation.Roll + SharedRotation.Roll * 1.25f));
        Head->SetRelativeScale3D(HeadGestureBaseScale * (1.0f + 0.035f * RescueCurve));
    }
    if (RescueLight)
    {
        RescueLight->SetRelativeLocation(LightGestureBaseLocation + FVector(0.0f, LockedShake * 0.25f, IdleBob + 18.0f * RescueCurve));
        RescueLight->SetIntensity(LightGestureBaseIntensity + 240.0f * IdleScale + 1100.0f * LockedAlpha + 2200.0f * RescueCurve);
    }

    if (IdleScale > 0.0f)
    {
        Tags.AddUnique(FName("SurvivorIdleLifePose"));
    }
    if (LockedAlpha > 0.0f)
    {
        Tags.AddUnique(FName("SurvivorLockedGesturePose"));
    }
    if (RescueAlpha > 0.0f)
    {
        Tags.AddUnique(FName("SurvivorRescueGesturePose"));
    }
}

void ASurvivorActor::TriggerLockedGesture()
{
    if (!bEnableSurvivorGestureReadability)
    {
        return;
    }

    LockedGestureTimer = FMath::Max(LockedGestureTimer, FMath::Max(0.15f, LockedGestureDuration));
    Tags.AddUnique(FName("SurvivorLockedGesturePose"));
    Tags.AddUnique(FName("SurvivorGestureReadabilityRuntime"));
}

void ASurvivorActor::TriggerRescueGesture()
{
    if (!bEnableSurvivorGestureReadability)
    {
        return;
    }

    RescueGestureTimer = FMath::Max(RescueGestureTimer, FMath::Max(0.15f, RescueGestureDuration));
    Tags.AddUnique(FName("SurvivorRescueGesturePose"));
    Tags.AddUnique(FName("SurvivorGestureReadabilityRuntime"));
    Tags.AddUnique(FName("SelectedLanguageSurvivorHandoff"));

    // 2026-07-11 v3: authored celebration — raise-and-wave clip, then settle
    // back into the idle loop (single-node presentation, so montages don't apply).
    if (V3WaveAnim && SkeletalBody && SkeletalBody->GetSkinnedAsset())
    {
        SkeletalBody->PlayAnimation(V3WaveAnim, false);
        GetWorldTimerManager().ClearTimer(WaveResumeTimer);
        if (V3IdleAnim)
        {
            TWeakObjectPtr<ASurvivorActor> WeakThis(this);
            GetWorldTimerManager().SetTimer(WaveResumeTimer,
                FTimerDelegate::CreateLambda([WeakThis]()
                {
                    if (WeakThis.IsValid() && WeakThis->SkeletalBody && WeakThis->V3IdleAnim)
                    {
                        WeakThis->SkeletalBody->PlayAnimation(WeakThis->V3IdleAnim, true);
                    }
                }),
                FMath::Max(0.2f, V3WaveAnim->GetPlayLength()), false);
        }
        Tags.AddUnique(FName("SurvivorRescueWaveAuthored"));
    }
}

void ASurvivorActor::ScheduleRescueFadeOut()
{
    const float FadeDelay = bEnableSurvivorGestureReadability
        ? FMath::Max(0.15f, RescueGestureDuration)
        : 0.0f;

    if (FadeDelay <= KINDA_SMALL_NUMBER || !GetWorld())
    {
        SetActorHiddenInGame(true);
        SetActorTickEnabled(false);
        return;
    }

    GetWorldTimerManager().SetTimer(
        RescueFadeOutTimer,
        FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            SetActorHiddenInGame(true);
            SetActorTickEnabled(false);
        }),
        FadeDelay,
        false);
}

void ASurvivorActor::ScheduleNextIdleBark()
{
    if (bRescued || IdleBarkCue.IsNull())
    {
        return;
    }
    const float Delay = FMath::RandRange(20.0f, 30.0f);
    GetWorldTimerManager().SetTimer(IdleBarkTimer, this, &ASurvivorActor::PlayIdleBark, Delay, false);
}

void ASurvivorActor::PlayIdleBark()
{
    if (bRescued)
    {
        return;
    }
    if (USoundBase* Cue = IdleBarkCue.LoadSynchronous())
    {
        UGameplayStatics::PlaySoundAtLocation(this, Cue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
    }
    ScheduleNextIdleBark();
}

bool ASurvivorActor::Rescue()
{
    const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(CityIndex);

    if (bRescued)
    {
        ClearHelperActors();
        return false;
    }

    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    if (GI && GI->RescuedSurvivorNames.Contains(SurvivorName))
    {
        bRescued = true;
        ClearHelperActors();
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        return false;
    }

    // Soft-gate: every city requires the complete ten-station coding set.
    // Refuse early rescue and surface progress without opening a modal.
    // The hint is intentionally on-screen (not modal) so the player isn't
    // blocked from running away if zombies are closing in.
    //   [item 17 in roadmap; see Documentation/zombie_system/17_objective_gating.md]
    const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, CityIndex);
    if (!FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex))
    {
        TriggerLockedGesture();
        if (GEngine)
        {
            const FString Hint = FString::Printf(
                TEXT("%s needs coding clearance: %d/%d challenges complete"),
                *SurvivorName,
                CompletedChallenges,
                FCodeRescueCampaign::RequiredChallengesPerCity);
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, Hint);
        }
        UCodeRescueSubtitlesWidget::Push(
            BuildSurvivorLockedRouteLine(
                SurvivorName,
                ArchetypeTitle,
                ArchetypeFieldNeed,
                Story,
                Mission,
                RequiredTerminalId,
                CompletedChallenges,
                GI),
            5.0f);
        return false;
    }

    bRescued = true;
    ClearHelperActors();
    GetWorldTimerManager().ClearTimer(IdleBarkTimer);
    TriggerRescueGesture();

    // #11 — play rescue voice line
    if (USoundBase* VoCue = RescueVoCue.LoadSynchronous())
    {
        UGameplayStatics::PlaySoundAtLocation(this, VoCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
    }
    // #44 — push subtitle for accessibility
    UCodeRescueSubtitlesWidget::Push(
        BuildSurvivorRescueLine(
            SurvivorName,
            ArchetypeTitle,
            ArchetypeRescueSkill,
            ArchetypeDossierHook,
            Story,
            Mission,
            GI),
        5.0f);

    FLinearColor PresentationAccent = FLinearColor(1.0f, 0.82f, 0.18f);
    if (Mission)
    {
        PresentationAccent = Mission->AccentColor * 0.62f + Mission->SecondaryAccentColor * 0.24f + FLinearColor(1.0f, 0.82f, 0.18f) * 0.14f;
    }

    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters PresentationParams;
        PresentationParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ARescueExtractionPresentationActor* Presentation = World->SpawnActor<ARescueExtractionPresentationActor>(
            ARescueExtractionPresentationActor::StaticClass(),
            GetActorLocation() + FVector(0.0f, 0.0f, -86.0f),
            FRotator::ZeroRotator,
            PresentationParams);
        if (Presentation)
        {
            Presentation->ConfigurePresentation(SurvivorName, CityIndex, PresentationAccent, GI && GI->bReducedMotion);
        }

        TArray<AActor*> HelipadActors;
        UGameplayStatics::GetAllActorsOfClass(World, AHelipadActor::StaticClass(), HelipadActors);
        for (AActor* Actor : HelipadActors)
        {
            AHelipadActor* Helipad = Cast<AHelipadActor>(Actor);
            if (Helipad && Helipad->CityIndex == CityIndex)
            {
                Helipad->SetExtractionReady(SurvivorName, PresentationAccent, GI && GI->bReducedMotion);
            }
        }
    }

    if (GI)
    {
        // Persist *which* survivor was rescued, not just the count, so reload
        // restores the world state instead of resurrecting them.
        GI->MarkSurvivorRescued(SurvivorName);
        GI->SurvivorsRescued = GI->RescuedSurvivorNames.Num();
        GI->IncrementRescueCount();   // #15 scoreboard
        GI->SavePersistentRun();

        UCodeRescueSubtitlesWidget::Push(
            BuildExtractionDispatchLine(SurvivorName, ArchetypeTitle, Mission, GI),
            4.5f);

        // #63: spawn a follower companion the first time we rescue someone.
        // The flag survives across cities for the rest of the run; it's
        // reset on Restart-Fresh via ResetRun(). We pick a spawn offset
        // behind+left of the player so the companion appears in their FOV
        // when they look around.
        if (!GI->bHasCompanion)
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            const FVector SpawnAt = (PlayerPawn ? PlayerPawn->GetActorLocation() : GetActorLocation())
                                    + FVector(-220.0f, -180.0f, 0.0f);
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            ACompanionActor* Buddy = GetWorld()->SpawnActor<ACompanionActor>(ACompanionActor::StaticClass(), SpawnAt, FRotator::ZeroRotator, Params);
            if (Buddy)
            {
                Buddy->DisplayName = SurvivorName;
                GI->bHasCompanion = true;
                UCodeRescueSubtitlesWidget::Push(
                    BuildCompanionHandoffLine(SurvivorName, ArchetypeTitle, Mission),
                    4.0f);
            }
        }
    }

    SetActorEnableCollision(false);
    ScheduleRescueFadeOut();
    return true;
}

void ASurvivorActor::AddHelperActor(AActor* HelperActor)
{
    if (IsValid(HelperActor) && HelperActor != this)
    {
        HelperActors.Add(HelperActor);
    }
}

void ASurvivorActor::ClearHelperActors()
{
    for (TWeakObjectPtr<AActor>& HelperPtr : HelperActors)
    {
        if (AActor* Helper = HelperPtr.Get())
        {
            Helper->Destroy();
        }
    }
    HelperActors.Reset();
}
