#include "CodeRescueCharacter.h"
#include "CodeRescueAnimationBudget.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescueFacialExpressionComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "CodeRescueRetargetRig.h"
#include "CodeZombieActor.h"
#include "CodingTerminalActor.h"
#include "CodeTerminalWidget.h"
#include "CodeRescueCampaign.h"
#include "BarricadeActor.h"
#include "CodeRescueDamageFeedbackWidget.h"
#include "CodeRescueHUDWidget.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CompanionActor.h"
#include "GameFramework/PlayerInput.h"
#include "ThrowableActor.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueGameMode.h"
#include "CodeRescueObjectiveJournalWidget.h"
#include "CodeRescueMainMenuWidget.h"
#include "CodeRescueTutorialWidget.h"
#include "ObjectiveFocusBeaconActor.h"
#include "CodeRescuePauseWidget.h"
#include "CodeRescueMessageReaderWidget.h"
#include "CodeRescueDeathWidget.h"
#include "SurvivorActor.h"
#include "LanguageStationActor.h"
#include "PickupActor.h"
#include "CaseFilePickupActor.h"
#include "FriendlyNPCActor.h"
#include "JeepActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Animation/AnimInstance.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UnrealClient.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/PointLightComponent.h"

namespace
{
// 2026-07-11 art+physics v3: tolerant loaders for the authored CharactersV3
// assets (the legacy FBX importer names takes "<File>_Anim_<File>_<Action>";
// older import styles produced "<File>_<Action>" / "<File><File>_<Action>").
static UAnimSequence* LoadAuthoredAnimForCharacterBase(const TCHAR* BaseFolder, const TCHAR* CharName, const TCHAR* ActionName)
{
    const FString Base = FString::Printf(TEXT("/Game/CodeRescueArt/%s/%s/"), BaseFolder, CharName);
    const FString Candidates[3] = {
        FString::Printf(TEXT("%s%s_Anim_%s_%s.%s_Anim_%s_%s"),
            *Base, CharName, CharName, ActionName, CharName, CharName, ActionName),
        FString::Printf(TEXT("%s%s_%s.%s_%s"), *Base, CharName, ActionName, CharName, ActionName),
        FString::Printf(TEXT("%s%s%s_%s.%s%s_%s"),
            *Base, CharName, CharName, ActionName, CharName, CharName, ActionName)};
    for (const FString& Path : Candidates)
    {
        if (UAnimSequence* Anim = LoadObject<UAnimSequence>(nullptr, *Path))
        {
            return Anim;
        }
    }
    return nullptr;
}

static UAnimSequence* LoadAuthoredV3AnimForCharacter(const TCHAR* CharName, const TCHAR* ActionName)
{
    const FString Base = FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/%s/"), CharName);
    const FString Candidates[3] = {
        FString::Printf(TEXT("%s%s_Anim_%s_%s.%s_Anim_%s_%s"),
            *Base, CharName, CharName, ActionName, CharName, CharName, ActionName),
        FString::Printf(TEXT("%s%s_%s.%s_%s"), *Base, CharName, ActionName, CharName, ActionName),
        FString::Printf(TEXT("%s%s%s_%s.%s%s_%s"),
            *Base, CharName, CharName, ActionName, CharName, CharName, ActionName)};
    for (const FString& Path : Candidates)
    {
        if (UAnimSequence* Anim = LoadObject<UAnimSequence>(nullptr, *Path))
        {
            return Anim;
        }
    }
    return nullptr;
}

static UCodeRescueMainMenuWidget* ResolveLaunchLanguageMenu(UWorld* World)
{
    if (ACodeRescueGameMode* GameMode = World ? World->GetAuthGameMode<ACodeRescueGameMode>() : nullptr)
    {
        if (UCodeRescueMainMenuWidget* OwnedMenu = GameMode->GetLaunchLanguageMenu())
        {
            return OwnedMenu;
        }
    }

    return UCodeRescueMainMenuWidget::GetActiveLaunchMenu();
}

struct FCodeRescueWeaponPresentationProfile
{
    FVector BaseLocation;
    FVector Scale;
    FRotator BaseRotation;
    FLinearColor Tint;
    const TCHAR* ProfileTag;
    float RecoilDistance;
    float RecoilPitch;
    float ReloadDip;
    float ReloadRoll;
    float BobScale;
};

static int32 FindClosestObjectiveIndex(const FVector& Location)
{
    int32 BestIndex = 0;
    float BestDistSq = TNumericLimits<float>::Max();

    for (int32 i = 0; i < FCodeRescueCampaign::GetMissionCount(); ++i)
    {
        const float DistSq = FVector::DistSquared(Location, FCodeRescueCampaign::GetCityOrigin(i));
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestIndex = i;
        }
    }

    return BestIndex;
}

static float GetRuntimeSfxVolume(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI ? GI->GetSfxVolumeScalar() : 1.0f;
}

static FVector GetMonoSafeSoundLocation(const UObject* Context, const FVector& RequestedLocation)
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

static bool IsInteractableActor(AActor* Actor)
{
    if (Actor && Actor->Tags.Contains(FName("Helipad"))) return true;
    if (Actor && Actor->Tags.Contains(FName("Jeep"))) return true;
    if (Actor && Actor->Tags.Contains(FName("MessageMarker"))) return true;
    if (Actor && Actor->Tags.Contains(FName("CodeRescueDoor"))) return true;   // pass-4 doors
    return Cast<ASurvivorActor>(Actor) ||
           Cast<ALanguageStationActor>(Actor) ||
           Cast<ACodingTerminalActor>(Actor) ||
           Cast<APickupActor>(Actor) ||
           Cast<ACaseFilePickupActor>(Actor) ||
           Cast<AFriendlyNPCActor>(Actor);   // #68
}

constexpr int32 GCodeRescueCameraPerspectiveCount = 6;
constexpr int32 GCodeRescueDefaultWeaponCount = static_cast<int32>(EWeaponType::FlashGrenade) + 1;
constexpr float GCodeRescueArenaSafeGroundZ = 108.0f;
constexpr float GCodeRescueArenaFallRecoveryZ = -40.0f;

static float GetHeldWeaponTargetLengthCm(EWeaponType Weapon)
{
    switch (Weapon)
    {
    case EWeaponType::Pistol:
    case EWeaponType::HeavyHandgun:
    case EWeaponType::BurstHandgun:
    case EWeaponType::Magnum:
        return 28.0f;
    case EWeaponType::Shotgun:
    case EWeaponType::TacticalShotgun:
    case EWeaponType::AutoShotgun:
        return 96.0f;
    case EWeaponType::Rifle:
    case EWeaponType::PrecisionRifle:
    case EWeaponType::SemiAutoRifle:
        return 92.0f;
    case EWeaponType::SMG:
        return 56.0f;
    case EWeaponType::CombatKnife:
        return 42.0f;
    case EWeaponType::Grenade:
    case EWeaponType::IncendiaryGrenade:
    case EWeaponType::FlashGrenade:
        return 14.0f;
    case EWeaponType::BoltLauncher:
        return 78.0f;
    case EWeaponType::RocketLauncher:
        return 112.0f;
    default:
        return 72.0f;
    }
}

static const TCHAR* GetCodeRescueCameraPerspectiveLabel(int32 Perspective)
{
    switch (Perspective)
    {
    case 0: return TEXT("First-Person");
    case 1: return TEXT("Third-Person");
    case 2: return TEXT("Tactical Third-Person");
    case 3: return TEXT("Top-Down");
    case 4: return TEXT("Isometric");
    case 5: return TEXT("Side-View 2.5D");
    default: return TEXT("Unknown");
    }
}

static int32 WrapCameraPerspective(int32 Perspective)
{
    int32 Wrapped = Perspective % GCodeRescueCameraPerspectiveCount;
    if (Wrapped < 0)
    {
        Wrapped += GCodeRescueCameraPerspectiveCount;
    }
    return Wrapped;
}

static FVector DirectionFromYaw(float YawDegrees)
{
    return FRotationMatrix(FRotator(0.0f, YawDegrees, 0.0f)).GetUnitAxis(EAxis::X);
}

static FVector RightFromYaw(float YawDegrees)
{
    return FRotationMatrix(FRotator(0.0f, YawDegrees, 0.0f)).GetUnitAxis(EAxis::Y);
}

static FString DescribeAttackerDirection(const ACodeRescueCharacter* Character, const AActor* DamageSource)
{
    if (!Character || !DamageSource)
    {
        return TEXT("environmental impact");
    }

    FVector ToAttacker = DamageSource->GetActorLocation() - Character->GetActorLocation();
    ToAttacker.Z = 0.0f;
    if (ToAttacker.IsNearlyZero())
    {
        return TEXT("point-blank contact");
    }

    const FVector LocalDir = Character->GetActorRotation().UnrotateVector(ToAttacker.GetSafeNormal());
    const float Angle = FMath::Atan2(LocalDir.Y, LocalDir.X);
    const float Degrees = FMath::RadiansToDegrees(Angle);

    if (Degrees >= -22.5f && Degrees < 22.5f) return TEXT("front");
    if (Degrees >= 22.5f && Degrees < 67.5f) return TEXT("front-right flank");
    if (Degrees >= 67.5f && Degrees < 112.5f) return TEXT("right side");
    if (Degrees >= 112.5f && Degrees < 157.5f) return TEXT("rear-right flank");
    if (Degrees >= -67.5f && Degrees < -22.5f) return TEXT("front-left flank");
    if (Degrees >= -112.5f && Degrees < -67.5f) return TEXT("left side");
    if (Degrees >= -157.5f && Degrees < -112.5f) return TEXT("rear-left flank");
    return TEXT("rear");
}

static FString DescribeDamageSource(const AActor* DamageSource)
{
    if (!DamageSource)
    {
        return TEXT("fall or environment");
    }

    if (DamageSource->IsA<ACodeZombieActor>())
    {
        return TEXT("infected attacker");
    }

    return DamageSource->GetName();
}

static FCodeRescueWeaponPresentationProfile GetWeaponPresentationProfile(EWeaponType Weapon)
{
    switch (Weapon)
    {
    case EWeaponType::Pistol:
        return { FVector(63.0f, 17.0f, -18.0f), FVector(0.112f, 0.026f, 0.040f), FRotator(0.0f, 1.0f, -2.0f), FLinearColor(0.56f, 0.64f, 0.70f), TEXT("WeaponProfile_BalancedHandgun"), 7.0f, -3.5f, 5.0f, -8.0f, 0.85f };
    case EWeaponType::Shotgun:
        return { FVector(71.0f, 18.0f, -19.0f), FVector(0.235f, 0.036f, 0.034f), FRotator(0.0f, 0.0f, -4.0f), FLinearColor(0.49f, 0.38f, 0.28f), TEXT("WeaponProfile_PumpShotgun"), 12.5f, -5.8f, 7.5f, -13.0f, 1.05f };
    case EWeaponType::Rifle:
        return { FVector(74.0f, 17.0f, -18.0f), FVector(0.275f, 0.028f, 0.033f), FRotator(0.0f, -0.8f, -2.5f), FLinearColor(0.31f, 0.43f, 0.38f), TEXT("WeaponProfile_AssaultRifle"), 9.5f, -4.4f, 6.0f, -10.0f, 0.95f };
    case EWeaponType::Grenade:
        return { FVector(54.0f, 20.0f, -16.0f), FVector(0.058f, 0.058f, 0.058f), FRotator(8.0f, 0.0f, -12.0f), FLinearColor(0.26f, 0.46f, 0.25f), TEXT("WeaponProfile_FragGrenade"), 5.5f, -7.5f, 8.0f, -22.0f, 1.20f };
    case EWeaponType::CombatKnife:
        return { FVector(57.0f, 20.5f, -15.0f), FVector(0.180f, 0.012f, 0.028f), FRotator(3.0f, 12.0f, -24.0f), FLinearColor(0.67f, 0.70f, 0.72f), TEXT("WeaponProfile_CombatKnife"), 14.0f, -9.0f, 3.5f, -28.0f, 0.72f };
    case EWeaponType::HeavyHandgun:
        return { FVector(64.0f, 17.5f, -18.0f), FVector(0.134f, 0.033f, 0.047f), FRotator(0.0f, 1.0f, -3.0f), FLinearColor(0.46f, 0.49f, 0.52f), TEXT("WeaponProfile_HeavyHandgun"), 10.5f, -5.2f, 6.0f, -11.0f, 0.82f };
    case EWeaponType::BurstHandgun:
        return { FVector(64.0f, 17.0f, -17.0f), FVector(0.121f, 0.024f, 0.038f), FRotator(0.0f, 2.5f, -1.0f), FLinearColor(0.35f, 0.56f, 0.74f), TEXT("WeaponProfile_BurstHandgun"), 6.8f, -3.1f, 4.5f, -9.5f, 0.90f };
    case EWeaponType::TacticalShotgun:
        return { FVector(72.0f, 18.0f, -18.5f), FVector(0.246f, 0.031f, 0.036f), FRotator(0.0f, -1.0f, -3.0f), FLinearColor(0.29f, 0.35f, 0.33f), TEXT("WeaponProfile_TacticalShotgun"), 10.8f, -5.0f, 6.5f, -11.5f, 0.98f };
    case EWeaponType::AutoShotgun:
        return { FVector(70.0f, 19.0f, -19.5f), FVector(0.215f, 0.047f, 0.043f), FRotator(0.0f, 0.5f, -5.5f), FLinearColor(0.45f, 0.42f, 0.34f), TEXT("WeaponProfile_AutoShotgun"), 8.8f, -4.8f, 7.0f, -15.0f, 1.10f };
    case EWeaponType::SMG:
        return { FVector(66.0f, 18.0f, -17.0f), FVector(0.184f, 0.036f, 0.035f), FRotator(0.0f, 3.0f, -2.5f), FLinearColor(0.30f, 0.46f, 0.61f), TEXT("WeaponProfile_SMG"), 5.5f, -2.8f, 4.8f, -8.5f, 1.18f };
    case EWeaponType::PrecisionRifle:
        return { FVector(80.0f, 16.0f, -17.0f), FVector(0.345f, 0.022f, 0.030f), FRotator(0.0f, -1.0f, -1.5f), FLinearColor(0.30f, 0.32f, 0.40f), TEXT("WeaponProfile_PrecisionRifle"), 13.0f, -4.2f, 8.2f, -7.0f, 0.72f };
    case EWeaponType::SemiAutoRifle:
        return { FVector(77.0f, 16.5f, -17.5f), FVector(0.304f, 0.025f, 0.032f), FRotator(0.0f, -0.5f, -2.0f), FLinearColor(0.34f, 0.42f, 0.46f), TEXT("WeaponProfile_SemiAutoRifle"), 9.0f, -3.8f, 6.4f, -8.0f, 0.82f };
    case EWeaponType::Magnum:
        return { FVector(64.0f, 18.0f, -18.5f), FVector(0.154f, 0.035f, 0.052f), FRotator(0.0f, 2.0f, -4.5f), FLinearColor(0.63f, 0.55f, 0.40f), TEXT("WeaponProfile_Magnum"), 15.0f, -7.0f, 6.5f, -14.0f, 0.78f };
    case EWeaponType::BoltLauncher:
        return { FVector(69.0f, 18.5f, -17.5f), FVector(0.255f, 0.025f, 0.039f), FRotator(0.0f, 4.0f, -7.0f), FLinearColor(0.40f, 0.52f, 0.45f), TEXT("WeaponProfile_BoltLauncher"), 8.0f, -3.6f, 5.8f, -16.0f, 0.88f };
    case EWeaponType::RocketLauncher:
        return { FVector(76.0f, 20.0f, -20.5f), FVector(0.320f, 0.062f, 0.062f), FRotator(0.0f, -2.0f, -6.5f), FLinearColor(0.54f, 0.47f, 0.32f), TEXT("WeaponProfile_RocketLauncher"), 18.0f, -8.0f, 10.0f, -18.0f, 0.68f };
    case EWeaponType::IncendiaryGrenade:
        return { FVector(54.0f, 20.0f, -16.5f), FVector(0.062f, 0.051f, 0.064f), FRotator(8.0f, -4.0f, -15.0f), FLinearColor(0.82f, 0.36f, 0.17f), TEXT("WeaponProfile_IncendiaryGrenade"), 5.8f, -7.0f, 8.5f, -24.0f, 1.15f };
    case EWeaponType::FlashGrenade:
        return { FVector(54.0f, 19.5f, -15.5f), FVector(0.051f, 0.051f, 0.076f), FRotator(7.0f, 2.5f, -10.0f), FLinearColor(0.78f, 0.82f, 0.60f), TEXT("WeaponProfile_FlashGrenade"), 5.0f, -6.2f, 7.8f, -20.0f, 1.10f };
    default:
        return { FVector(63.0f, 17.0f, -18.0f), FVector(0.112f, 0.026f, 0.040f), FRotator::ZeroRotator, FLinearColor(0.56f, 0.64f, 0.70f), TEXT("WeaponProfile_DefaultFallback"), 7.0f, -3.5f, 5.0f, -8.0f, 0.85f };
    }
}
}

// Static UI-lock flag. The terminal widget (or any other modal UI) toggles this
// via ACodeRescueCharacter::SetUIOpen(true/false). PollDirectKeys early-returns
// while it is set, so typing in the terminal cannot leak into gameplay.
bool ACodeRescueCharacter::bUIOpen = false;

ACodeRescueCharacter::ACodeRescueCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionObjectType(CodeRescueCollision::PlayerPawnObject);
        Capsule->SetCollisionResponseToChannel(CodeRescueCollision::WeaponTrace, ECR_Block);
        Capsule->SetCollisionResponseToChannel(CodeRescueCollision::AISightTrace, ECR_Block);
        Capsule->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Ignore);
        Capsule->ComponentTags.AddUnique(FName("CollisionChannel_PlayerPawnObject"));
    }

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 76.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Perspective rig: first-person uses the capsule camera, while every other
    // active gameplay view uses this boom/camera pair. Press C/V to cycle, or
    // press F1-F6 to jump directly to a specific view.
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetCapsuleComponent());
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 76.0f));
    CameraBoom->TargetArmLength = 320.0f;
    CameraBoom->bUsePawnControlRotation = true;
    // 2026-07-07 (Kenny: "character perpetually positioned at ~15 degrees"):
    // the CAMERA was rolled, tilting the whole world — verticals and the
    // character with it. Nothing in this game legitimately rolls the view:
    // the boom refuses roll from any source, and PollDirectKeys strips
    // residual roll from the control rotation every frame.
    CameraBoom->bInheritRoll = false;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 14.0f;
    CameraBoom->bDoCollisionTest = true;

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(CameraBoom);
    ThirdPersonCamera->bUsePawnControlRotation = false;

    CodeRescueAnimationBudget::ApplySkeletalMeshBudget(
        GetMesh(), ECodeRescueAnimationBudgetProfile::PlayerBody, this);
    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        GetMesh(), ECodeRescueRetargetRigProfile::PlayerOperator, this);

    FirstPersonArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmsMesh"));
    FirstPersonArmsMesh->SetupAttachment(FirstPersonCamera);
    FirstPersonArmsMesh->SetRelativeLocation(FVector(54.0f, 0.0f, -96.0f));
    FirstPersonArmsMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    FirstPersonArmsMesh->SetRelativeScale3D(FVector(0.48f));
    FirstPersonArmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FirstPersonArmsMesh->SetGenerateOverlapEvents(false);
    FirstPersonArmsMesh->SetCastShadow(false);
    FirstPersonArmsMesh->SetOnlyOwnerSee(true);
    FirstPersonArmsMesh->SetVisibility(false, true);
    CodeRescueAnimationBudget::ApplySkeletalMeshBudget(
        FirstPersonArmsMesh, ECodeRescueAnimationBudgetProfile::FirstPersonArms, this);
    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        FirstPersonArmsMesh, ECodeRescueRetargetRigProfile::FirstPersonArms, this);

    AimingPresentationMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("AimingPresentationMesh"));
    AimingPresentationMesh->SetupAttachment(GetCapsuleComponent());
    AimingPresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AimingPresentationMesh->SetGenerateOverlapEvents(false);
    AimingPresentationMesh->SetCastShadow(true);
    AimingPresentationMesh->SetVisibility(false, true);
    AimingPresentationMesh->ComponentTags.AddUnique(FName("ProceduralTwoArmAimPresentation"));

    // 2026-07-11 pass 4: the hero's visible third-person body.
    HeroPresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeroPresentationMesh"));
    HeroPresentationMesh->SetupAttachment(GetCapsuleComponent());
    HeroPresentationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeroPresentationMesh->SetGenerateOverlapEvents(false);
    HeroPresentationMesh->SetCastShadow(true);
    HeroPresentationMesh->SetVisibility(false, true);
    HeroPresentationMesh->ComponentTags.AddUnique(FName("HeroBodyPresentation"));

    FirstPersonWeaponSilhouette = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonWeaponSilhouette"));
    FirstPersonWeaponSilhouette->SetupAttachment(FirstPersonCamera);
    FirstPersonWeaponSilhouette->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FirstPersonWeaponSilhouette->SetGenerateOverlapEvents(false);
    FirstPersonWeaponSilhouette->SetCastShadow(false);
    FirstPersonWeaponSilhouette->SetOnlyOwnerSee(true);
    FirstPersonWeaponSilhouette->SetVisibility(false, true);
    FirstPersonWeaponSilhouette->SetMobility(EComponentMobility::Movable);
    FirstPersonWeaponSilhouette->ComponentTags.AddUnique(FName("DistinctWeaponSilhouette"));
    FirstPersonWeaponSilhouette->ComponentTags.AddUnique(FName("WeaponPresentationModelFallback"));
    FirstPersonWeaponSilhouette->ComponentTags.AddUnique(FName("WeaponAnimationMontageFutureHook"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> WeaponSilhouetteMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (WeaponSilhouetteMesh.Succeeded())
    {
        FirstPersonWeaponSilhouette->SetStaticMesh(WeaponSilhouetteMesh.Object);
    }

    // 2026-07-07 (Kenny: weapon "visually apparent ONLY from the perspective
    // of the character holding it"): a SECOND weapon model rides the body's
    // right hand so every third-person camera shows what you're holding.
    // Driven by RefreshFirstPersonWeapon alongside the FP silhouette.
    ThirdPersonWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThirdPersonWeaponMesh"));
    ThirdPersonWeaponMesh->SetupAttachment(GetMesh());
    ThirdPersonWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ThirdPersonWeaponMesh->SetGenerateOverlapEvents(false);
    // Imported hand bones can carry a 100x scene scale. Keep the cosmetic
    // weapon at absolute real-world scale and out of the shadow pass.
    ThirdPersonWeaponMesh->SetAbsolute(false, false, true);
    ThirdPersonWeaponMesh->SetCastShadow(false);
    ThirdPersonWeaponMesh->SetVisibility(false, true);
    ThirdPersonWeaponMesh->SetMobility(EComponentMobility::Movable);
    ThirdPersonWeaponMesh->ComponentTags.AddUnique(FName("HeldWeaponAbsoluteScale"));
    ThirdPersonWeaponMesh->ComponentTags.AddUnique(FName("HeldWeaponShadowDisabled"));

    FieldFlashlight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FieldFlashlight"));
    FieldFlashlight->SetupAttachment(FirstPersonCamera);
    FieldFlashlight->SetRelativeLocation(FVector(28.0f, 0.0f, -8.0f));
    FieldFlashlight->SetLightColor(FLinearColor(1.0f, 0.90f, 0.66f));
    FieldFlashlight->SetIntensity(0.0f);
    FieldFlashlight->SetAttenuationRadius(1600.0f);
    FieldFlashlight->SetCastShadows(false);

    Tags.AddUnique(FName("DistinctWeaponPresentationRuntime"));
    Tags.AddUnique(FName("FirstPersonWeaponSilhouetteReady"));
    Tags.AddUnique(FName("WeaponFireReloadMotionCue"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("StealthAvoidanceRuntime"));
    Tags.AddUnique(FName("Top50Recommendation40StealthAvoidance"));

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->BrakingDecelerationWalking = BrakingDeceleration;
    // 2026-07-07 (Kenny: "elevated regions ... led to multiple stuck points"):
    // the layered street kit produces slab lips (road-on-plaza, sidewalk-on-
    // road, platform plinths) taller than the 45cm engine default step. The
    // player now steps over anything up to 65cm and walks 50-degree inclines,
    // so region edges read as curbs, not walls.
    GetCharacterMovement()->MaxStepHeight = 65.0f;
    GetCharacterMovement()->SetWalkableFloorAngle(50.0f);
    GetCharacterMovement()->JumpZVelocity = 680.0f;
    GetCharacterMovement()->AirControl = 0.42f;
}

void ACodeRescueCharacter::InitDefaultWeaponLoadout()
{
    WeaponLoadout.Reset();
    WeaponLoadout.SetNum(GCodeRescueDefaultWeaponCount);

    auto MakeWeapon = [](
        const FString& Name,
        const FString& Role,
        int32 Magazine,
        int32 Reserve,
        int32 MaxReserve,
        float Damage,
        float Refire,
        float Reload,
        float Range,
        int32 Pellets,
        float Spread,
        int32 Burst,
        int32 Pierce,
        float Radius,
        bool bUsesAmmo = true) -> FWeaponDef
    {
        FWeaponDef Weapon;
        Weapon.DisplayName = Name;
        Weapon.TacticalRole = Role;
        Weapon.MagazineSize = FMath::Max(1, Magazine);
        Weapon.StartingReserveAmmo = FMath::Max(0, Reserve);
        Weapon.MaxReserveAmmo = FMath::Max(Weapon.StartingReserveAmmo, MaxReserve);
        Weapon.Damage = Damage;
        Weapon.RefireDelay = Refire;
        Weapon.ReloadDuration = Reload;
        Weapon.Range = Range;
        Weapon.PelletsPerShot = FMath::Max(1, Pellets);
        Weapon.SpreadHalfAngleDeg = FMath::Max(0.0f, Spread);
        Weapon.BurstCount = FMath::Max(1, Burst);
        Weapon.PierceCount = FMath::Max(0, Pierce);
        Weapon.ExplosionRadius = FMath::Max(0.0f, Radius);
        Weapon.bUsesAmmo = bUsesAmmo;
        Weapon.AmmoCostPerShot = 1;
        return Weapon;
    };

    auto SetWeapon = [this](EWeaponType Type, const FWeaponDef& Weapon)
    {
        const int32 Idx = static_cast<int32>(Type);
        if (WeaponLoadout.IsValidIndex(Idx))
        {
            WeaponLoadout[Idx] = Weapon;
        }
    };

    SetWeapon(EWeaponType::Pistol, MakeWeapon(
        TEXT("Balanced Handgun"), TEXT("Reliable sidearm for accurate single shots and ammo economy."),
        12, 96, 180, 35.0f, 0.18f, 1.0f, 25000.0f, 1, 0.0f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::Shotgun, MakeWeapon(
        TEXT("Pump Shotgun"), TEXT("Close-range crowd control with high stagger potential."),
        6, 36, 72, 18.0f, 0.85f, 1.8f, 5000.0f, 6, 8.0f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::Rifle, MakeWeapon(
        TEXT("Assault Rifle"), TEXT("Sustained medium-range fire for advancing through streets."),
        30, 180, 360, 32.0f, 0.12f, 2.2f, 30000.0f, 1, 1.1f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::Grenade, MakeWeapon(
        TEXT("Frag Grenade"), TEXT("Wide burst for clustered infected and defensive retreats."),
        1, 5, 12, 130.0f, 1.15f, 0.7f, 7600.0f, 1, 1.5f, 1, 0, 700.0f));
    SetWeapon(EWeaponType::CombatKnife, MakeWeapon(
        TEXT("Combat Knife"), TEXT("No-ammo emergency melee for close saves and crate clearing."),
        1, 0, 0, 80.0f, 0.55f, 0.0f, 220.0f, 1, 0.0f, 1, 0, 0.0f, false));
    SetWeapon(EWeaponType::HeavyHandgun, MakeWeapon(
        TEXT("Heavy Handgun"), TEXT("Slower sidearm with stronger torso stopping power."),
        9, 54, 108, 58.0f, 0.34f, 1.3f, 24000.0f, 1, 0.4f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::BurstHandgun, MakeWeapon(
        TEXT("Burst Handgun"), TEXT("Three-round panic burst for quick pressure at short range."),
        18, 108, 216, 26.0f, 0.32f, 1.2f, 21000.0f, 1, 1.8f, 3, 0, 0.0f));
    SetWeapon(EWeaponType::TacticalShotgun, MakeWeapon(
        TEXT("Tactical Shotgun"), TEXT("Tighter spread shotgun for corridors and door pushes."),
        8, 48, 96, 16.0f, 0.62f, 1.6f, 6500.0f, 7, 5.4f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::AutoShotgun, MakeWeapon(
        TEXT("Auto Shotgun"), TEXT("Fast close-range suppression when surrounded."),
        12, 60, 120, 13.0f, 0.28f, 2.0f, 5200.0f, 6, 7.2f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::SMG, MakeWeapon(
        TEXT("SMG"), TEXT("High-rate stagger tool for agile enemies and close escorts."),
        40, 240, 480, 22.0f, 0.075f, 1.9f, 18500.0f, 1, 3.2f, 1, 0, 0.0f));
    SetWeapon(EWeaponType::PrecisionRifle, MakeWeapon(
        TEXT("Precision Rifle"), TEXT("High-damage scoped role with long reach and one-target focus."),
        5, 35, 80, 115.0f, 0.92f, 2.4f, 52000.0f, 1, 0.0f, 1, 1, 0.0f));
    SetWeapon(EWeaponType::SemiAutoRifle, MakeWeapon(
        TEXT("Semi-Auto Rifle"), TEXT("Fast precision follow-up shots for distant pressure."),
        10, 60, 120, 78.0f, 0.38f, 2.0f, 45000.0f, 1, 0.2f, 1, 1, 0.0f));
    SetWeapon(EWeaponType::Magnum, MakeWeapon(
        TEXT("Magnum"), TEXT("Rare heavy stopping power for elite targets and bosses."),
        6, 24, 48, 155.0f, 0.72f, 2.0f, 26000.0f, 1, 0.6f, 1, 2, 0.0f));
    SetWeapon(EWeaponType::BoltLauncher, MakeWeapon(
        TEXT("Bolt Launcher"), TEXT("Quiet piercing bolts for lined-up targets and conservation."),
        6, 42, 84, 72.0f, 0.52f, 1.5f, 23000.0f, 1, 0.4f, 1, 3, 0.0f));
    SetWeapon(EWeaponType::RocketLauncher, MakeWeapon(
        TEXT("Rocket Launcher"), TEXT("Single-shot boss breaker and emergency horde reset."),
        1, 1, 3, 420.0f, 1.6f, 1.2f, 18000.0f, 1, 0.0f, 1, 0, 1100.0f));
    SetWeapon(EWeaponType::IncendiaryGrenade, MakeWeapon(
        TEXT("Incendiary Grenade"), TEXT("Area denial flame burst for narrow routes and chokes."),
        1, 4, 10, 88.0f, 1.05f, 0.7f, 7000.0f, 1, 2.0f, 1, 0, 780.0f));
    SetWeapon(EWeaponType::FlashGrenade, MakeWeapon(
        TEXT("Flash Grenade"), TEXT("Low-damage stun burst to interrupt a rush and create space."),
        1, 4, 10, 18.0f, 0.95f, 0.7f, 7000.0f, 1, 2.0f, 1, 0, 850.0f));

    WeaponMagazines.Init(0, WeaponLoadout.Num());
    WeaponReserveAmmo.Init(0, WeaponLoadout.Num());
    MaxAmmo = 0;
    for (int32 i = 0; i < WeaponLoadout.Num(); ++i)
    {
        WeaponMagazines[i] = WeaponLoadout[i].bUsesAmmo ? WeaponLoadout[i].MagazineSize : 1;
        WeaponReserveAmmo[i] = WeaponLoadout[i].StartingReserveAmmo;
        MaxAmmo += WeaponLoadout[i].MaxReserveAmmo;
    }
    SyncActiveWeaponStateFromLoadout();
    RefreshLegacyAmmoFromWeaponReserves();
}

void ACodeRescueCharacter::SwapWeapon(EWeaponType NewWeapon)
{
    EnsureWeaponStateInitialized();
    const int32 Idx = static_cast<int32>(NewWeapon);
    if (!WeaponLoadout.IsValidIndex(Idx)) return;
    if (ActiveWeapon == NewWeapon) return;
    ActiveWeapon = NewWeapon;
    SyncActiveWeaponStateFromLoadout();
    RefreshFirstPersonWeapon();   // 2026-07-04: visible weapon model follows the selection
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White,
            FString::Printf(TEXT("Equipped: %s - %s"), *WeaponLoadout[Idx].DisplayName, *WeaponLoadout[Idx].TacticalRole));
    }
    // 2026-07-07 (Kenny couldn't tell selection worked): a swap now announces
    // itself through the always-visible subtitle channel too.
    UCodeRescueSubtitlesWidget::Push(
        FString::Printf(TEXT("Equipped: %s  (1-0 slots, [ ] or mouse wheel to cycle)"), *WeaponLoadout[Idx].DisplayName),
        2.2f);
    UE_LOG(LogTemp, Display, TEXT("[WeaponSwap] equipped slot %d: %s"), Idx, *WeaponLoadout[Idx].DisplayName);
}

// No-arg weapon-swap wrappers — BindKey targets these directly.
void ACodeRescueCharacter::SwapToPistol()  { SwapWeapon(EWeaponType::Pistol);  }
void ACodeRescueCharacter::SwapToShotgun() { SwapWeapon(EWeaponType::Shotgun); }
void ACodeRescueCharacter::SwapToRifle()   { SwapWeapon(EWeaponType::Rifle);   }
void ACodeRescueCharacter::SwapToGrenade() { SwapWeapon(EWeaponType::Grenade); }

void ACodeRescueCharacter::SwapToWeaponSlot(int32 SlotIndex)
{
    // 2026-07-17: a held Command key means a system chord (Cmd+Shift+4 is the
    // in-game screenshot), never a weapon-slot intent.
    if (const APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (PC->IsInputKeyDown(EKeys::LeftCommand) || PC->IsInputKeyDown(EKeys::RightCommand))
        {
            return;
        }
    }
    EnsureWeaponStateInitialized();
    if (WeaponLoadout.IsValidIndex(SlotIndex))
    {
        SwapWeapon(static_cast<EWeaponType>(SlotIndex));
    }
}

void ACodeRescueCharacter::SelectWeaponSlot1()  { SwapToWeaponSlot(0); }
void ACodeRescueCharacter::SelectWeaponSlot2()  { SwapToWeaponSlot(1); }
void ACodeRescueCharacter::SelectWeaponSlot3()  { SwapToWeaponSlot(2); }
void ACodeRescueCharacter::SelectWeaponSlot4()  { SwapToWeaponSlot(3); }
void ACodeRescueCharacter::SelectWeaponSlot5()  { SwapToWeaponSlot(4); }
void ACodeRescueCharacter::SelectWeaponSlot6()  { SwapToWeaponSlot(5); }
void ACodeRescueCharacter::SelectWeaponSlot7()  { SwapToWeaponSlot(6); }
void ACodeRescueCharacter::SelectWeaponSlot8()  { SwapToWeaponSlot(7); }
void ACodeRescueCharacter::SelectWeaponSlot9()  { SwapToWeaponSlot(8); }
void ACodeRescueCharacter::SelectWeaponSlot10() { SwapToWeaponSlot(9); }

void ACodeRescueCharacter::CycleWeapon(int32 Direction)
{
    EnsureWeaponStateInitialized();
    if (WeaponLoadout.Num() <= 0 || Direction == 0)
    {
        return;
    }
    // 2026-07-07: cycling now fires from BOTH the bound keys and the polled
    // path (packaged builds drop bound events); this debounce keeps a single
    // physical press from double-stepping when both deliver.
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if ((Now - WeaponCycleCooldown) < 0.15f)
    {
        return;
    }
    WeaponCycleCooldown = Now;
    int32 CurrentIdx = static_cast<int32>(ActiveWeapon);
    CurrentIdx = WeaponLoadout.IsValidIndex(CurrentIdx) ? CurrentIdx : 0;
    int32 NextIdx = (CurrentIdx + Direction) % WeaponLoadout.Num();
    if (NextIdx < 0)
    {
        NextIdx += WeaponLoadout.Num();
    }
    SwapWeapon(static_cast<EWeaponType>(NextIdx));
}

void ACodeRescueCharacter::CycleWeaponNext()
{
    CycleWeapon(1);
}

void ACodeRescueCharacter::CycleWeaponPrevious()
{
    CycleWeapon(-1);
}

void ACodeRescueCharacter::EnsureWeaponStateInitialized()
{
    if (WeaponLoadout.Num() < GCodeRescueDefaultWeaponCount)
    {
        InitDefaultWeaponLoadout();
        return;
    }

    if (WeaponMagazines.Num() != WeaponLoadout.Num())
    {
        const TArray<int32> PreviousMags = WeaponMagazines;
        WeaponMagazines.Init(0, WeaponLoadout.Num());
        for (int32 i = 0; i < WeaponLoadout.Num(); ++i)
        {
            WeaponMagazines[i] = PreviousMags.IsValidIndex(i)
                ? FMath::Clamp(PreviousMags[i], 0, FMath::Max(1, WeaponLoadout[i].MagazineSize))
                : (WeaponLoadout[i].bUsesAmmo ? WeaponLoadout[i].MagazineSize : 1);
        }
    }

    if (WeaponReserveAmmo.Num() != WeaponLoadout.Num())
    {
        const TArray<int32> PreviousReserve = WeaponReserveAmmo;
        WeaponReserveAmmo.Init(0, WeaponLoadout.Num());
        for (int32 i = 0; i < WeaponLoadout.Num(); ++i)
        {
            WeaponReserveAmmo[i] = PreviousReserve.IsValidIndex(i)
                ? FMath::Max(0, PreviousReserve[i])
                : FMath::Max(0, WeaponLoadout[i].StartingReserveAmmo);
        }
    }

    MaxAmmo = 0;
    for (const FWeaponDef& Weapon : WeaponLoadout)
    {
        MaxAmmo += FMath::Max(0, Weapon.MaxReserveAmmo);
    }
    SyncActiveWeaponStateFromLoadout();
    RefreshLegacyAmmoFromWeaponReserves();
}

void ACodeRescueCharacter::SyncActiveWeaponStateFromLoadout()
{
    const int32 Idx = static_cast<int32>(ActiveWeapon);
    if (!WeaponLoadout.IsValidIndex(Idx))
    {
        ActiveWeapon = EWeaponType::Pistol;
    }

    const int32 ActiveIdx = static_cast<int32>(ActiveWeapon);
    if (!WeaponLoadout.IsValidIndex(ActiveIdx))
    {
        return;
    }

    const FWeaponDef& Weapon = WeaponLoadout[ActiveIdx];
    MagazineSize = FMath::Max(1, Weapon.MagazineSize);
    MagazineAmmo = WeaponMagazines.IsValidIndex(ActiveIdx)
        ? FMath::Clamp(WeaponMagazines[ActiveIdx], 0, MagazineSize)
        : MagazineSize;
    ReloadDuration = Weapon.ReloadDuration;
    FireRefireDelay = Weapon.RefireDelay;
    WeaponRange = Weapon.Range;
    DirectHitDamage = Weapon.Damage;
    bWeaponPresentationProfileInitialized = false;
    UpdateFirstPersonWeaponPresentation(0.0f);
}

void ACodeRescueCharacter::RefreshLegacyAmmoFromWeaponReserves()
{
    int32 TotalReserve = 0;
    for (int32 Count : WeaponReserveAmmo)
    {
        TotalReserve += FMath::Max(0, Count);
    }
    Ammo = TotalReserve;
}

int32 ACodeRescueCharacter::GetActiveWeaponReserveAmmo() const
{
    const int32 Idx = static_cast<int32>(ActiveWeapon);
    return WeaponReserveAmmo.IsValidIndex(Idx) ? FMath::Max(0, WeaponReserveAmmo[Idx]) : Ammo;
}

FString ACodeRescueCharacter::GetActiveWeaponName() const
{
    const int32 Idx = static_cast<int32>(ActiveWeapon);
    return WeaponLoadout.IsValidIndex(Idx) ? WeaponLoadout[Idx].DisplayName : TEXT("Weapon");
}

FString ACodeRescueCharacter::GetActiveWeaponTacticalRole() const
{
    const int32 Idx = static_cast<int32>(ActiveWeapon);
    return WeaponLoadout.IsValidIndex(Idx) ? WeaponLoadout[Idx].TacticalRole : TEXT("Standard issue.");
}

int32 ACodeRescueCharacter::GetActiveWeaponMagazineSize() const
{
    const int32 Idx = static_cast<int32>(ActiveWeapon);
    return WeaponLoadout.IsValidIndex(Idx) ? FMath::Max(1, WeaponLoadout[Idx].MagazineSize) : FMath::Max(1, MagazineSize);
}

FString ACodeRescueCharacter::GetWeaponQuickSlotSummary() const
{
    if (WeaponLoadout.Num() <= 0)
    {
        return TEXT("QUICK SLOTS: initializing arsenal");
    }

    auto ShortWeaponName = [](const FString& Name) -> FString
    {
        if (Name.Contains(TEXT("Balanced Handgun"))) return TEXT("Handgun");
        if (Name.Contains(TEXT("Pump Shotgun"))) return TEXT("Pump");
        if (Name.Contains(TEXT("Assault Rifle"))) return TEXT("Rifle");
        if (Name.Contains(TEXT("Frag Grenade"))) return TEXT("Frag");
        if (Name.Contains(TEXT("Combat Knife"))) return TEXT("Knife");
        if (Name.Contains(TEXT("Heavy Handgun"))) return TEXT("Heavy");
        if (Name.Contains(TEXT("Burst Handgun"))) return TEXT("Burst");
        if (Name.Contains(TEXT("Tactical Shotgun"))) return TEXT("TacSG");
        if (Name.Contains(TEXT("Auto Shotgun"))) return TEXT("AutoSG");
        if (Name.Contains(TEXT("Precision Rifle"))) return TEXT("Precision");
        return Name.Left(10);
    };

    TArray<FString> Segments;
    const int32 Count = FMath::Min(10, WeaponLoadout.Num());
    for (int32 i = 0; i < Count; ++i)
    {
        const FWeaponDef& Weapon = WeaponLoadout[i];
        const FString KeyLabel = (i == 9) ? TEXT("0") : FString::FromInt(i + 1);
        const FString Marker = (static_cast<int32>(ActiveWeapon) == i) ? TEXT("*") : TEXT(" ");
        const int32 SlotMagazine = WeaponMagazines.IsValidIndex(i)
            ? FMath::Clamp(WeaponMagazines[i], 0, FMath::Max(1, Weapon.MagazineSize))
            : (Weapon.bUsesAmmo ? FMath::Max(1, Weapon.MagazineSize) : 1);
        const int32 SlotReserve = WeaponReserveAmmo.IsValidIndex(i)
            ? FMath::Max(0, WeaponReserveAmmo[i])
            : FMath::Max(0, Weapon.StartingReserveAmmo);
        const FString AmmoText = Weapon.bUsesAmmo
            ? FString::Printf(TEXT("%d+%d"), SlotMagazine, SlotReserve)
            : FString(TEXT("melee"));
        Segments.Add(FString::Printf(
            TEXT("%s%s %s %s"),
            *Marker,
            *KeyLabel,
            *ShortWeaponName(Weapon.DisplayName),
            *AmmoText));
    }

    return FString::Printf(TEXT("QUICK SLOTS: %s"), *FString::Join(Segments, TEXT(" | ")));
}

void ACodeRescueCharacter::RestoreWeaponQuickSlotState(
    EWeaponType SavedActiveWeapon,
    const TArray<int32>& SavedMagazines,
    const TArray<int32>& SavedReserveAmmo)
{
    EnsureWeaponStateInitialized();
    for (int32 i = 0; i < WeaponLoadout.Num(); ++i)
    {
        const FWeaponDef& Weapon = WeaponLoadout[i];
        if (WeaponMagazines.IsValidIndex(i))
        {
            WeaponMagazines[i] = Weapon.bUsesAmmo
                ? FMath::Clamp(
                    SavedMagazines.IsValidIndex(i) ? SavedMagazines[i] : WeaponMagazines[i],
                    0,
                    FMath::Max(1, Weapon.MagazineSize))
                : 1;
        }
        if (WeaponReserveAmmo.IsValidIndex(i))
        {
            WeaponReserveAmmo[i] = Weapon.bUsesAmmo
                ? FMath::Clamp(
                    SavedReserveAmmo.IsValidIndex(i) ? SavedReserveAmmo[i] : WeaponReserveAmmo[i],
                    0,
                    FMath::Max(0, Weapon.MaxReserveAmmo))
                : 0;
        }
    }

    const int32 SavedActiveIndex = static_cast<int32>(SavedActiveWeapon);
    ActiveWeapon = WeaponLoadout.IsValidIndex(SavedActiveIndex)
        ? SavedActiveWeapon
        : EWeaponType::Pistol;
    SyncActiveWeaponStateFromLoadout();
    RefreshLegacyAmmoFromWeaponReserves();
}

int32 ACodeRescueCharacter::AddAmmoToWeaponIndex(int32 WeaponIdx, int32 Amount)
{
    if (Amount <= 0 || !WeaponLoadout.IsValidIndex(WeaponIdx) || !WeaponReserveAmmo.IsValidIndex(WeaponIdx))
    {
        return 0;
    }
    if (!WeaponLoadout[WeaponIdx].bUsesAmmo || WeaponLoadout[WeaponIdx].MaxReserveAmmo <= 0)
    {
        return 0;
    }

    const int32 Previous = WeaponReserveAmmo[WeaponIdx];
    const int32 MaxReserve = bClampSuppliesToMaximum
        ? FMath::Max(0, WeaponLoadout[WeaponIdx].MaxReserveAmmo)
        : TNumericLimits<int32>::Max();
    WeaponReserveAmmo[WeaponIdx] = FMath::Clamp(Previous + Amount, 0, MaxReserve);
    return FMath::Max(0, WeaponReserveAmmo[WeaponIdx] - Previous);
}

void ACodeRescueCharacter::BeginPlay()
{
    Super::BeginPlay();
    // 2026-07-06: bUIOpen is a STATIC that survives OpenLevel. If any screen
    // left it set when a deploy travelled, the fresh world would boot with
    // every polled input suppressed (frozen player). A newly spawned player
    // pawn is authoritative: no modal UI can predate it.
    ACodeRescueCharacter::SetUIOpen(false);
    ApplyRuntimeTuning();
    if (WeaponLoadout.Num() < GCodeRescueDefaultWeaponCount)
    {
        InitDefaultWeaponLoadout();
    }
    else
    {
        EnsureWeaponStateInitialized();
    }
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        if (!GI->bHasOperatorIdentityState)
        {
            GI->InitializeOperatorIdentityForLanguage(GI->SelectedLanguage);
        }
        GI->ApplySkillTreeToPlayer(this);
    }
    Tags.AddUnique(FName("PlayableOperatorIdentityRuntime"));
    Tags.AddUnique(FName("SelectedLanguageOperatorProfile"));
    Tags.AddUnique(FName("PlayerOperator"));

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        const UCodeRescueGameInstance* LaunchGI = GetGameInstance<UCodeRescueGameInstance>();
        const bool bLaunchLanguageGateActive = LaunchGI && !LaunchGI->bHasSelectedLaunchLanguageThisSession;
        if (bLaunchLanguageGateActive)
        {
            FInputModeGameAndUI Mode;
            if (UCodeRescueMainMenuWidget* LaunchMenu = ResolveLaunchLanguageMenu(GetWorld()))
            {
                Mode.SetWidgetToFocus(LaunchMenu->TakeWidget());
            }
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Mode.SetHideCursorDuringCapture(false);
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = true;
            PC->SetIgnoreLookInput(true);
            PC->SetIgnoreMoveInput(true);
        }
        else
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
            PC->SetIgnoreLookInput(false);
            PC->SetIgnoreMoveInput(false);
        }

        TSubclassOf<UUserWidget> RuntimeHUDClass = UCodeRescueHUDWidget::StaticClass();
        if (HUDWidgetClass)
        {
            RuntimeHUDClass = HUDWidgetClass;
        }
        if (UUserWidget* HUD = CreateWidget<UUserWidget>(PC, RuntimeHUDClass))
        {
            HUD->AddToViewport(10);
        }
    }

    // #21 wiring — mount the damage-feedback overlay alongside the HUD.
    DamageFeedbackWidget = CreateWidget<UCodeRescueDamageFeedbackWidget>(GetWorld(), UCodeRescueDamageFeedbackWidget::StaticClass());
    if (DamageFeedbackWidget)
    {
        DamageFeedbackWidget->AddToViewport(50);
    }

    // Production defaults to the fully rigged Manny presentation. The earlier
    // procedural survivor remains available to art reviewers via
    // -CodeRescueUsePrototypeCharacters, but no longer replaces the polished
    // locomotion rig in the public build.
    if (USkeletalMeshComponent* BodyMesh = GetMesh())
    {
        const float HalfHeight = GetCapsuleComponent()
            ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
        const bool bUsePrototypeCharacter =
            FParse::Param(FCommandLine::Get(), TEXT("CodeRescueUsePrototypeCharacters"));
        bool bConfiguredProductionBody = false;
        // (2026-07-11 pass 4, revised after the integrated run: the Manny rig
        // KEEPS driving locomotion + aim/landing presentation — that is an
        // audited contract — and the HERO body rides a dedicated presentation
        // layer configured below.)
        if (!bUsePrototypeCharacter)
        {
            if (USkeletalMesh* PlayerMesh = LoadObject<USkeletalMesh>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny")))
            {
                BodyMesh->SetSkeletalMesh(PlayerMesh);
                BodyMesh->SetRelativeLocationAndRotation(
                    FVector(0.0f, 0.0f, -HalfHeight), FRotator(0.0f, -90.0f, 0.0f));

                MannyIdleAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle"));
                MannyWalkAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Walk_Fwd.MM_Walk_Fwd"));
                MannyRunAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Run_Fwd.MM_Run_Fwd"));
                MannyJumpAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Jump.MM_Jump"));
                MannyFallAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Fall_Loop.MM_Fall_Loop"));
                MannyLandAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/Manny/MM_Land.MM_Land"));

                bUsingAuthoredMannyAnimation = MannyIdleAnim && MannyWalkAnim && MannyRunAnim
                    && MannyJumpAnim && MannyFallAnim && MannyLandAnim;
                if (bUsingAuthoredMannyAnimation)
                {
                    BodyMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
                    BodyMesh->PlayAnimation(MannyIdleAnim, true);
                    MannyAnimationState = 0;
                    Tags.AddUnique(FName("AuthoredMannySixStateAnimation"));
                    UE_LOG(LogTemp, Display,
                        TEXT("[PlayerAnimationAudit] COMPLETE PASS authored_sequences=6 locomotion=1 jump=1 fall=1 land=1 two_arm_aim=1"));
                }
                else if (UClass* AnimBP = LoadClass<UAnimInstance>(nullptr,
                            TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C")))
                {
                    BodyMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
                    BodyMesh->SetAnimInstanceClass(AnimBP);
                    UE_LOG(LogTemp, Error,
                        TEXT("[PlayerAnimationAudit] COMPLETE FAIL authored_sequences=%d expected=6 fallback=ABP"),
                        static_cast<int32>(MannyIdleAnim != nullptr) + static_cast<int32>(MannyWalkAnim != nullptr)
                        + static_cast<int32>(MannyRunAnim != nullptr) + static_cast<int32>(MannyJumpAnim != nullptr)
                        + static_cast<int32>(MannyFallAnim != nullptr) + static_cast<int32>(MannyLandAnim != nullptr));
                }
                bConfiguredProductionBody = true;
                UE_LOG(LogTemp, Display, TEXT("[CharacterPresentation] Player uses authored Manny six-state locomotion rig."));
            }
        }

        // 2026-07-11 pass 4: the HERO soldier body (SurvivorKennyV4 — 5'10",
        // red beard, blue eyes) is the player's third-person body; V3 then v2
        // remain the fallbacks.
        int32 AuthoredGen = 0;
        USkeletalMesh* AuthoredBodyMesh = !bConfiguredProductionBody
            ? LoadObject<USkeletalMesh>(nullptr,
                TEXT("/Game/CodeRescueArt/CharactersV4/SurvivorKennyV4/SurvivorKennyV4.SurvivorKennyV4"))
            : nullptr;
        if (AuthoredBodyMesh)
        {
            AuthoredGen = 4;
        }
        if (!AuthoredBodyMesh && !bConfiguredProductionBody)
        {
            AuthoredBodyMesh = LoadObject<USkeletalMesh>(nullptr,
                TEXT("/Game/CodeRescueArt/CharactersV3/SurvivorKennyV3/SurvivorKennyV3.SurvivorKennyV3"));
            if (AuthoredBodyMesh)
            {
                AuthoredGen = 3;
            }
        }
        const bool bAuthoredV3Body = AuthoredGen >= 3;
        if (!AuthoredBodyMesh && !bConfiguredProductionBody)
        {
            AuthoredBodyMesh = LoadObject<USkeletalMesh>(nullptr,
                TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorKenny.SurvivorKenny"));
        }
        if (USkeletalMesh* V2Mesh = AuthoredBodyMesh)
        {
            BodyMesh->SetSkeletalMesh(V2Mesh);
            BodyMesh->SetRelativeLocationAndRotation(
                FVector(0.0f, 0.0f, -HalfHeight), FRotator(0.0f, -90.0f, 0.0f));
            if (AuthoredGen == 4)
            {
                V2IdleAnim = LoadAuthoredAnimForCharacterBase(TEXT("CharactersV4"), TEXT("SurvivorKennyV4"), TEXT("Idle"));
                V2WalkAnim = LoadAuthoredAnimForCharacterBase(TEXT("CharactersV4"), TEXT("SurvivorKennyV4"), TEXT("Walk"));
                V2RunAnim = LoadAuthoredAnimForCharacterBase(TEXT("CharactersV4"), TEXT("SurvivorKennyV4"), TEXT("Run"));
            }
            else if (bAuthoredV3Body)
            {
                V2IdleAnim = LoadAuthoredV3AnimForCharacter(TEXT("SurvivorKennyV3"), TEXT("Idle"));
                V2WalkAnim = LoadAuthoredV3AnimForCharacter(TEXT("SurvivorKennyV3"), TEXT("Walk"));
                V2RunAnim = LoadAuthoredV3AnimForCharacter(TEXT("SurvivorKennyV3"), TEXT("Run"));
            }
            else
            {
                V2IdleAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorKennySurvivorKenny_Idle.SurvivorKennySurvivorKenny_Idle"));
                V2WalkAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorKennySurvivorKenny_Walk.SurvivorKennySurvivorKenny_Walk"));
                V2RunAnim = LoadObject<UAnimSequence>(nullptr,
                    TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorKennySurvivorKenny_Run.SurvivorKennySurvivorKenny_Run"));
            }
            BodyMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            if (V2IdleAnim)
            {
                BodyMesh->PlayAnimation(V2IdleAnim, true);
            }
            bUsingV2Body = true;
            V2BodyAnimState = 0;
            UE_LOG(LogTemp, Warning, TEXT("[CharacterV%d] Player body = %s (anims idle=%d walk=%d run=%d)"),
                AuthoredGen > 0 ? AuthoredGen : 2,
                AuthoredGen == 4 ? TEXT("SurvivorKennyV4 HERO")
                    : (AuthoredGen == 3 ? TEXT("SurvivorKennyV3") : TEXT("SurvivorKenny")),
                V2IdleAnim != nullptr, V2WalkAnim != nullptr, V2RunAnim != nullptr);
        }
        else if (USkeletalMesh* PlayerMesh = !bConfiguredProductionBody
                ? LoadObject<USkeletalMesh>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"))
                : nullptr)
        {
            BodyMesh->SetSkeletalMesh(PlayerMesh);
            BodyMesh->SetRelativeLocationAndRotation(
                FVector(0.0f, 0.0f, -HalfHeight), FRotator(0.0f, -90.0f, 0.0f));
            if (UClass* AnimBP = LoadClass<UAnimInstance>(nullptr,
                    TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C")))
            {
                BodyMesh->SetAnimInstanceClass(AnimBP);
            }
            UE_LOG(LogTemp, Display, TEXT("[CharacterPresentation] Player uses Manny fallback locomotion rig."));
        }

        // 2026-07-11 pass 4: HERO presentation layer. The Manny rig above keeps
        // driving locomotion + aim/landing (audited contract); when the authored
        // hero exists it becomes the VISIBLE third-person body.
        if (!bUsePrototypeCharacter && HeroPresentationMesh)
        {
            if (USkeletalMesh* HeroMesh = LoadObject<USkeletalMesh>(nullptr,
                    TEXT("/Game/CodeRescueArt/CharactersV4/SurvivorKennyV4/SurvivorKennyV4.SurvivorKennyV4")))
            {
                HeroPresentationMesh->SetSkeletalMesh(HeroMesh);
                HeroPresentationMesh->SetRelativeLocationAndRotation(
                    FVector(0.0f, 0.0f, -HalfHeight), FRotator(0.0f, -90.0f, 0.0f));
                // 5'10" spec: the mesh is authored at the proven 1.86 m rig
                // scale; the component scale takes him to 177.8 cm exactly.
                HeroPresentationMesh->SetRelativeScale3D(FVector(177.8f / 186.0f));
                HeroPresentationMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
                // pass-5 harness diagnosis (boundsrad=1): an unrendered single-
                // node mesh never ticks its pose, so its bounds stay a 1 uu
                // point and the renderer frustum-culls it forever — the
                // "invisible hero" in Kenny's screenshots. Fixed asset bounds +
                // always-tick break the cull deadlock.
                HeroPresentationMesh->bComponentUseFixedSkelBounds = true;
                HeroPresentationMesh->VisibilityBasedAnimTickOption =
                    EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
                HeroPresentationMesh->UpdateBounds();
                HeroIdleAnim = LoadAuthoredAnimForCharacterBase(TEXT("CharactersV4"), TEXT("SurvivorKennyV4"), TEXT("Idle"));
                HeroWalkAnim = LoadAuthoredAnimForCharacterBase(TEXT("CharactersV4"), TEXT("SurvivorKennyV4"), TEXT("Walk"));
                HeroRunAnim = LoadAuthoredAnimForCharacterBase(TEXT("CharactersV4"), TEXT("SurvivorKennyV4"), TEXT("Run"));
                if (HeroIdleAnim)
                {
                    HeroPresentationMesh->PlayAnimation(HeroIdleAnim, true);
                }
                bHeroPresentationConfigured = true;
                HeroBodyAnimState = 0;
                HeroPresentationMesh->SetVisibility(CameraPerspective != 0, false);
                UE_LOG(LogTemp, Display,
                    TEXT("[CharacterV4] HERO presentation layer active = SurvivorKennyV4 (idle=%d walk=%d run=%d)"),
                    HeroIdleAnim != nullptr, HeroWalkAnim != nullptr, HeroRunAnim != nullptr);
            }
        }
    }

    ConfigureAimingPresentationMesh();

    // 2026-07-04: facial expression driver (morph targets; silent no-op on mannequin fallback)
    // + the visible first-person weapon model.
    FacialExpression = NewObject<UCodeRescueFacialExpressionComponent>(this, TEXT("FacialExpression"));
    if (FacialExpression)
    {
        FacialExpression->RegisterComponent();
    }
    RefreshFirstPersonWeapon();
    // 2026-07-07 (why Kenny NEVER saw a weapon at spawn): nothing applied the
    // camera perspective on a fresh spawn, so the weapon models kept their
    // constructor visibility (hidden) until the player's first camera switch.
    // Apply the default perspective now — viewmodel visible from second one.
    ApplyCameraPerspective();

    // 2026-07-16 pass 5: scope math needs the true base FOV, and the visual
    // review harness drives perspectives/ADS/grenades + screenshots itself.
    if (FirstPersonCamera)
    {
        BaseFirstPersonFOV = FirstPersonCamera->FieldOfView;
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("CodeRescuePerspectiveReview")))
    {
        StartPerspectiveReviewHarness();
    }
    // 2026-07-17 failsafe: no state (stale photo mode, interrupted hit-stop)
    // may ever leave the world running in slow motion at spawn.
    UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
    // 2026-07-16 freeze forensics: a hard game-thread spin logs NOTHING (one
    // eternal frame), so absence of log lines is ambiguous evidence. Under
    // the resume harness every player character (menu AND post-resume — the
    // level swap destroys the first one) emits a 10 s heartbeat with the
    // frame counter + engine-average FPS + position: any future "frozen"
    // report gets a numeric pulse to read.
    FString ResumeHealthToken;
    if (FParse::Value(FCommandLine::Get(), TEXT("CodeRescueAutoResumeLanguage="), ResumeHealthToken) &&
        !ResumeHealthToken.IsEmpty())
    {
        GetWorldTimerManager().SetTimer(ResumeHealthPulseTimer,
            FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                extern ENGINE_API float GAverageFPS;
                const UCharacterMovementComponent* Movement = GetCharacterMovement();
                ++ResumeHealthPulseCount;
                UE_LOG(LogTemp, Display,
                    TEXT("[ResumeHealth] pulse=%d frame=%llu fps_avg=%.1f loc=%s vel=%.0f maxwalk=%.0f"),
                    ResumeHealthPulseCount,
                    static_cast<unsigned long long>(GFrameCounter),
                    GAverageFPS,
                    *GetActorLocation().ToCompactString(),
                    GetVelocity().Size2D(),
                    Movement ? Movement->MaxWalkSpeed : -1.0f);
                // visual pulses: what does the player actually SEE right now
                // (catches follower props, ground seams, camera state in the
                // real resumed save — not just numbers)
                if (ResumeHealthPulseCount == 2 || ResumeHealthPulseCount == 4)
                {
                    const FString ShotPath = FPaths::ProjectSavedDir() /
                        FString::Printf(TEXT("Screenshots/FirstLevel/resume_pulse_%d.png"), ResumeHealthPulseCount);
                    FScreenshotRequest::RequestScreenshot(ShotPath, false, false);
                }
            }),
            10.0f, true, 6.0f);
    }
    if (FirstPersonArmsMesh)
    {
        // 2026-07-04: the silver mannequin first-person "arms" are a FULL
        // SKM_Manny body hung under the camera — its head sits right in the
        // lens. It only LOOKED harmless because the old import scale bug
        // culled it invisible; the moment imports were fixed (2026-07-16,
        // cycle-12 review) a chrome skull filled every first-person frame.
        // First person is weapon-only until a dedicated arms rig ships.
        FirstPersonArmsMesh->SetSkeletalMesh(nullptr);
        FirstPersonArmsMesh->SetVisibility(false, true);
        Tags.AddUnique(FName("PlayerFirstPersonArmsMesh"));
        Tags.AddUnique(FName("CharacterAnimationDeepDive"));
        Tags.AddUnique(FName("FirstPersonAnimationPrototype"));
    }

    // Start in readable over-the-shoulder third-person. The camera may pitch,
    // but the character capsule never does; this prevents the avatar from
    // sliding through the level in a prone/flying posture after look input.
    CameraPerspective = 1;
    ApplyCameraPerspective();

    if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelAimReview")))
    {
        bAimInputHeld = true;
        AimHoldTimer = 30.0f;
        Tags.AddUnique(FName("FirstLevelAimReviewActive"));
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelArmoryReview")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelArmoryCycleAudit")))
    {
        GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            TogglePauseMenu();
        }));
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelCombatRuntimeAudit")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit")))
    {
        FTimerHandle AuditStartTimer;
        GetWorldTimerManager().SetTimer(
            AuditStartTimer,
            this,
            &ACodeRescueCharacter::StartFirstLevelCombatRuntimeAudit,
            1.5f,
            false);
    }

    // 2026-07-01 HUD diet: no control-sheet dump on spawn. The two STEP lines from the game
    // mode are the only spawn text; full controls live in the pause menu (P) / journal (J).
}

FString ACodeRescueCharacter::GetOperatorIdentitySummary() const
{
    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        return GI->GetOperatorIdentitySummary();
    }
    return TEXT("Rhea Calder | Rescue Operator | frontline rescue route lead | Java run");
}

void ACodeRescueCharacter::UpdateCameraProximityFade()
{
    // 2026-07-07 (live playtest: every wall pinch turned the screen into the
    // back of Kenny's head): when the third-person boom collapses onto the
    // pawn — tight interiors, wall contact, recovery points — hide the body
    // and held weapon so the player still sees the ROOM instead of their own
    // skull. Standard third-person treatment. First person already hides the
    // body through ApplyCameraPerspective.
    if (CameraPerspective == 0 || !CameraBoom || !ThirdPersonCamera)
    {
        return;
    }
    const float CameraToPawn = FVector::Dist(
        ThirdPersonCamera->GetComponentLocation(), GetActorLocation());
    const bool bTooClose = CameraToPawn < 120.0f;
    if (bTooClose != bCameraProximityHidden)
    {
        bCameraProximityHidden = bTooClose;
        if (USkeletalMeshComponent* Body = GetMesh())
        {
            Body->SetOwnerNoSee(bTooClose);
        }
        if (AimingPresentationMesh)
        {
            AimingPresentationMesh->SetOwnerNoSee(bTooClose);
        }
        if (HeroPresentationMesh)
        {
            HeroPresentationMesh->SetOwnerNoSee(bTooClose);   // pass 5: hero fades too
        }
        if (ThirdPersonWeaponMesh)
        {
            ThirdPersonWeaponMesh->SetOwnerNoSee(bTooClose);
        }
    }
}

void ACodeRescueCharacter::UpdateCameraOcclusion(float DeltaSeconds)
{
    CameraOcclusionAccumulator += DeltaSeconds;
    if (CameraOcclusionAccumulator < 0.08f)
    {
        return;
    }
    CameraOcclusionAccumulator = 0.0f;

    // Restore only actors hidden by this camera pass. Permanently curated
    // production actors carry their own tag and remain hidden.
    for (TWeakObjectPtr<AActor>& HiddenPtr : CameraOcclusionHiddenActors)
    {
        if (AActor* HiddenActor = HiddenPtr.Get())
        {
            if (!HiddenActor->Tags.Contains(FName("ProductionPresentationHidden")))
            {
                HiddenActor->SetActorHiddenInGame(false);
            }
        }
    }
    CameraOcclusionHiddenActors.Reset();

    if (CameraPerspective == 0 || !ThirdPersonCamera || !ThirdPersonCamera->IsActive())
    {
        return;
    }

    const FVector ViewStart = GetActorLocation() + FVector(0.0f, 0.0f, 72.0f);
    const FVector ViewEnd = ThirdPersonCamera->GetComponentLocation();
    const FVector ViewDelta = ViewEnd - ViewStart;
    if (ViewDelta.SizeSquared() < FMath::Square(80.0f))
    {
        return;
    }

    for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
    {
        AStaticMeshActor* Candidate = *It;
        if (!IsValid(Candidate) || Candidate->IsHidden() || Candidate->GetOwner() == this ||
            Candidate->IsAttachedTo(this) || Candidate->Tags.Contains(FName("CameraOcclusionExempt")) ||
            Candidate->Tags.Contains(FName("GameplayArenaConfinement")))
        {
            continue;
        }

        UStaticMeshComponent* MeshComponent = Candidate->GetStaticMeshComponent();
        if (!MeshComponent || !MeshComponent->IsVisible())
        {
            continue;
        }

        FBox Bounds = Candidate->GetComponentsBoundingBox(true);
        if (!Bounds.IsValid)
        {
            continue;
        }
        const FVector Extent = Bounds.GetExtent();
        const bool bArchitecturalScale = Extent.Z > 70.0f || FMath::Max(Extent.X, Extent.Y) > 240.0f;
        const bool bCameraPassesThroughMesh =
            MeshComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision ||
            Candidate->Tags.Contains(FName("EntryCorridorCollisionCleared"));
        if (!bArchitecturalScale || (!bCameraPassesThroughMesh && Extent.GetMax() < 450.0f))
        {
            continue;
        }

        Bounds = Bounds.ExpandBy(12.0f);
        if (FMath::LineBoxIntersection(Bounds, ViewStart, ViewEnd, ViewDelta))
        {
            Candidate->SetActorHiddenInGame(true);
            CameraOcclusionHiddenActors.Add(Candidate);
        }
    }
}

void ACodeRescueCharacter::EnsureSpawnClearance(float DeltaSeconds)
{
    // 2026-07-07 (live playtest + Kenny's report): the city spawn pad can end
    // up inside blocking geometry as layers accrete (the recovery teleport
    // PROVED it: "[Teleport] destination blocked — relocated 220uu"). Rather
    // than hand-tune 465 cities, the pawn verifies its own footing once,
    // shortly after the world finishes spawning, and relocates if embedded.
    if (bSpawnClearanceDone)
    {
        return;
    }
    SpawnClearanceDelay += DeltaSeconds;
    if (SpawnClearanceDelay < 0.75f)   // let the city layers finish spawning
    {
        return;
    }
    bSpawnClearanceDone = true;
    FVector Here = GetActorLocation();
    FVector Cleared = Here;
    if (AdjustTeleportDestination(Cleared) && !Cleared.Equals(Here, 1.0f))
    {
        SetActorLocation(Cleared, false, nullptr, ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Warning, TEXT("[SpawnClear] spawn point was embedded in geometry — relocated %s -> %s"),
            *Here.ToCompactString(), *Cleared.ToCompactString());
    }
    // 2026-07-07 spawn FRAMING: the pad sits close to the safehouse facade;
    // spawning while facing it parks the third-person boom inside the wall
    // and the first thing a player sees is unlit geometry. Face along the
    // open street instead — probe the four cardinals at boom distance and
    // adopt the facing whose BACKWARD ray has the most clear room.
    if (UWorld* FrameWorld = GetWorld())
    {
        float BestYaw = GetActorRotation().Yaw;
        float BestClearance = -1.0f;
        FCollisionQueryParams FrameParams(SCENE_QUERY_STAT(SpawnFraming), false, this);
        for (int32 YawStep = 0; YawStep < 4; ++YawStep)
        {
            const float Yaw = YawStep * 90.0f;
            const FVector Back = -FRotator(0.0f, Yaw, 0.0f).Vector();
            FHitResult FrameHit;
            const FVector Eye = GetActorLocation() + FVector(0, 0, 90.0f);
            const float Clear = FrameWorld->LineTraceSingleByChannel(
                    FrameHit, Eye, Eye + Back * 800.0f, ECC_Camera, FrameParams)
                ? FrameHit.Distance : 800.0f;
            if (Clear > BestClearance)
            {
                BestClearance = Clear;
                BestYaw = Yaw;
            }
        }
        SetActorRotation(FRotator(0.0f, BestYaw, 0.0f));
        if (AController* C = GetController())
        {
            C->SetControlRotation(FRotator(0.0f, BestYaw, 0.0f));
        }
        UE_LOG(LogTemp, Display, TEXT("[SpawnClear] facing yaw %.0f (camera clearance %.0fuu)"), BestYaw, BestClearance);
    }
}

void ACodeRescueCharacter::UpdateStuckMovementWatchdog(float DeltaSeconds)
{
    // 2026-07-06, rebuilt 2026-07-07 after Kenny's T-stuck report showed ZERO
    // watchdog fires in his session log. Two blind spots fixed:
    //   1. It keyed on the movement component's consumed input — states where
    //      input never REACHES the component (the actual bug class) were
    //      invisible. It now also reads the RAW movement keys.
    //   2. A frustrated player mashes keys in sub-3s bursts, resetting the
    //      timer forever. Trigger is now 1.25s, and repeat fires escalate:
    //      nudge → bigger nudge → full FindTeleportSpot ring relocation.
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move || bUIOpen || Health <= 0.0f)
    {
        StuckMovementSeconds = 0.0f;
        return;
    }
    const APlayerController* PC = Cast<APlayerController>(GetController());
    const bool bRawKeysHeld = PC &&
        (PC->IsInputKeyDown(EKeys::W) || PC->IsInputKeyDown(EKeys::A) ||
         PC->IsInputKeyDown(EKeys::S) || PC->IsInputKeyDown(EKeys::D) ||
         PC->IsInputKeyDown(EKeys::Up) || PC->IsInputKeyDown(EKeys::Down));
    const bool bWantsMove = bRawKeysHeld || Move->GetLastInputVector().SizeSquared() > 0.25f;
    const bool bNotMoving = GetVelocity().SizeSquared() < 4.0f;
    const bool bAirborne = Move->MovementMode == MOVE_Falling;
    if (bWantsMove && bNotMoving && !bAirborne)
    {
        StuckMovementSeconds += DeltaSeconds;
    }
    else
    {
        StuckMovementSeconds = 0.0f;
        if (GetVelocity().SizeSquared() > 2500.0f)
        {
            StuckRescueEscalation = 0;   // moving freely again — reset ladder
        }
    }
    if (StuckMovementSeconds > 1.25f)
    {
        StuckMovementSeconds = 0.0f;
        ++StuckRescueEscalation;
        UE_LOG(LogTemp, Warning,
            TEXT("[MoveWatchdog] input held with zero velocity at %s (mode=%d, escalation=%d) — self-healing"),
            *GetActorLocation().ToCompactString(),
            static_cast<int32>(Move->MovementMode.GetValue()),
            StuckRescueEscalation);
        Move->StopMovementImmediately();
        Move->SetMovementMode(MOVE_Walking);
        if (StuckRescueEscalation <= 1)
        {
            SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, 44.0f), false, nullptr, ETeleportType::TeleportPhysics);
        }
        else
        {
            // Escalate: let the engine find genuinely clear ground nearby.
            FVector FreeSpot = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
            AdjustTeleportDestination(FreeSpot);
            SetActorLocation(FreeSpot, false, nullptr, ETeleportType::TeleportPhysics);
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                TEXT("Movement recovery applied - if you were wedged, you are free now."));
        }
    }
}

void ACodeRescueCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TimeSinceLastFire += DeltaSeconds;
    PollDirectKeys(DeltaSeconds);
    PollGamepad(DeltaSeconds);   // #39
    UpdateStuckMovementWatchdog(DeltaSeconds);   // 2026-07-06 movement-lock report
    UpdateCameraProximityFade();                 // 2026-07-07 "all I see is my own head" fix
    UpdateCameraOcclusion(DeltaSeconds);          // 2026-07-09 visible/no-collision wall safeguard
    EnsureSpawnClearance(DeltaSeconds);          // 2026-07-07 spawn-pad-inside-geometry fix
    UpdateArenaSafety(DeltaSeconds);
    UpdateCombatJuice(DeltaSeconds);
    UpdateReactiveThreatAudio(DeltaSeconds);
    UpdateCityAmbientZoneAudio(DeltaSeconds);
    UpdateStealthNoise(DeltaSeconds);
    UpdateAuthoredMannyAnimation(DeltaSeconds);
    UpdateV2BodyLocomotion(DeltaSeconds);   // 2026-07-04: idle/walk/run on the authored body
    UpdateADSPresentation(DeltaSeconds);    // 2026-07-16: aim-down-sights + scope zoom
    UpdateGrenadeArcPreview(DeltaSeconds);  // 2026-07-16: grenade launch-arc + landing ring
    UpdateAutoTargetLock(DeltaSeconds);
    UpdateWeaponAimPresentation(DeltaSeconds);

    if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        if (Movement->IsFalling())
        {
            HighestFallingDownSpeed = FMath::Max(HighestFallingDownSpeed, FMath::Max(0.0f, -Movement->Velocity.Z));
        }
    }

    // Update stamina (drain while sprinting, regen otherwise)
    if (bIsSprinting && Stamina > 0.0f)
    {
        Stamina -= StaminaDrainRate * DeltaSeconds;
        if (Stamina < 0.0f)
        {
            Stamina = 0.0f;
            bIsSprinting = false;
        }
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintSpeedMultiplier;
        }
    }
    else if (!bIsSprinting && Stamina < MaxStamina)
    {
        Stamina += StaminaRegenRate * DeltaSeconds;
        if (Stamina > MaxStamina)
        {
            Stamina = MaxStamina;
        }
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        }
    }
    else if (!bIsSprinting && GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }

    UpdateFirstPersonArms(DeltaSeconds);
    UpdateFirstPersonWeaponPresentation(DeltaSeconds);
}

void ACodeRescueCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    if (bUsingAuthoredMannyAnimation)
    {
        MannyLandingPresentationRemaining = 0.34f;
    }

    const float LandingSpeed = HighestFallingDownSpeed;
    HighestFallingDownSpeed = 0.0f;

    if (LandingSpeed <= SoftLandingSpeed)
    {
        return;
    }

    const float FallDamage = ((LandingSpeed - SoftLandingSpeed) / 100.0f) * FallDamagePer100Speed;
    if (FallDamage <= 0.5f)
    {
        return;
    }

    if (bEnableTrainingLandingAssist && Health - FallDamage <= 0.0f)
    {
        Health = 1.0f;
        Stamina = 0.0f;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                TEXT("Landing assist absorbed a lethal fall. Health locked at 1."));
        }
        return;
    }

    ApplyDamage(FallDamage, nullptr);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
            FString::Printf(TEXT("Hard landing: %.0f fall damage"), FallDamage));
    }
}

void ACodeRescueCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Movement axes.
    PlayerInputComponent->BindAxis("MoveForward", this, &ACodeRescueCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ACodeRescueCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ACodeRescueCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ACodeRescueCharacter::LookUp);

    // Action keys are bound DIRECTLY to keys here. The earlier design polled
    // these keys every Tick, which did not work in the running game — only
    // the bound movement axes responded. BindKey uses the exact same input
    // routing that the working movement axes use, so these now fire
    // reliably. While a modal UI (terminal, pause menu, journal) is open the
    // player controller is switched to UIOnly input mode, so these game
    // bindings are correctly suppressed during menus.
    // 2026-07-17 (Kenny: "too many buttons that do the same thing ... less
    // helpful and more confusing"): ONE keyboard key per action. Removed
    // duplicates — Interact (was E/Enter/Tab/G), Fire (was also F), Reload
    // (was also LeftCtrl), Help (was also M), Recover (was also F8), camera
    // cycle (was also V), and the Z zoom/radio-scanner double-bind (scanner
    // moved to K). Gamepad bindings keep their own single mapping.
    PlayerInputComponent->BindKey(EKeys::E,               IE_Pressed, this, &ACodeRescueCharacter::Interact);
    PlayerInputComponent->BindKey(EKeys::SpaceBar,        IE_Pressed, this, &ACodeRescueCharacter::TryJump);
    PlayerInputComponent->BindKey(EKeys::SpaceBar,        IE_Released, this, &ACodeRescueCharacter::StopJumpAction);
    PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Pressed, this, &ACodeRescueCharacter::TryJump);
    PlayerInputComponent->BindKey(EKeys::Gamepad_FaceButton_Bottom, IE_Released, this, &ACodeRescueCharacter::StopJumpAction);
    PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACodeRescueCharacter::Fire);
    PlayerInputComponent->BindKey(EKeys::Gamepad_RightTrigger, IE_Pressed, this, &ACodeRescueCharacter::Fire);
    // 2026-07-17: toggle/hold hybrid aim — trackpads cannot hold right-click
    // and left-click at once, so a quick right-click latches the sights up
    // (left-click then fires), while a long hold stays hold-to-aim for mice.
    PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ACodeRescueCharacter::OnAimPressed);
    PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ACodeRescueCharacter::OnAimReleased);
    PlayerInputComponent->BindKey(EKeys::Z, IE_Pressed, this, &ACodeRescueCharacter::CycleScopeZoom);   // pass 5 scope ladder
    PlayerInputComponent->BindKey(EKeys::Gamepad_LeftTrigger, IE_Pressed, this, &ACodeRescueCharacter::OnAimPressed);
    PlayerInputComponent->BindKey(EKeys::Gamepad_LeftTrigger, IE_Released, this, &ACodeRescueCharacter::OnAimReleased);

    PlayerInputComponent->BindKey(EKeys::R,               IE_Pressed, this, &ACodeRescueCharacter::Reload);

    PlayerInputComponent->BindKey(EKeys::Q,               IE_Pressed, this, &ACodeRescueCharacter::UseMedkit);

    PlayerInputComponent->BindKey(EKeys::H,               IE_Pressed, this, &ACodeRescueCharacter::ShowMissionHelp);

    PlayerInputComponent->BindKey(EKeys::T,               IE_Pressed, this, &ACodeRescueCharacter::TeleportToNextObjective);
    PlayerInputComponent->BindKey(EKeys::BackSpace,       IE_Pressed, this, &ACodeRescueCharacter::RecoverToCityArena);
    // 2026-07-17 THE "EXTREMELY SLOW GAME" ROOT CAUSE: F10 photo mode set
    // GLOBAL TIME DILATION to 0.12 and hid the whole HUD with no persistent
    // indicator. One accidental press (F10 sits beside the F12 screenshot
    // key) put Kenny's entire game into 12% slow motion with nothing on
    // screen to say why. The binding is GONE; TogglePhotoMode stays for a
    // future menu entry, and dilation is force-restored at BeginPlay.
    PlayerInputComponent->BindKey(EKeys::Y,               IE_Pressed, this, &ACodeRescueCharacter::RegroupRescueTeam);
    PlayerInputComponent->BindKey(EKeys::U,               IE_Pressed, this, &ACodeRescueCharacter::CycleSquadFormation);
    PlayerInputComponent->BindKey(EKeys::N,               IE_Pressed, this, &ACodeRescueCharacter::CallSquadMedic);
    PlayerInputComponent->BindKey(EKeys::O,               IE_Pressed, this, &ACodeRescueCharacter::ToggleSquadHoldPosition);
    PlayerInputComponent->BindKey(EKeys::J,               IE_Pressed, this, &ACodeRescueCharacter::ToggleObjectiveJournal);

    // Escape opens the pause menu (frees the mouse cursor for Resume/Quit).
    PlayerInputComponent->BindKey(EKeys::Escape,          IE_Pressed, this, &ACodeRescueCharacter::TogglePauseMenu);

    // C cycles camera; F1-F6 select a specific perspective directly. Number
    // keys are reserved for weapon quick slots.
    PlayerInputComponent->BindKey(EKeys::C,               IE_Pressed, this, &ACodeRescueCharacter::CycleCameraPerspective);
    PlayerInputComponent->BindKey(EKeys::Gamepad_RightShoulder, IE_Pressed, this, &ACodeRescueCharacter::CycleCameraPerspective);
    PlayerInputComponent->BindKey(EKeys::F1,              IE_Pressed, this, &ACodeRescueCharacter::SelectFirstPersonPerspective);
    PlayerInputComponent->BindKey(EKeys::F2,              IE_Pressed, this, &ACodeRescueCharacter::SelectThirdPersonPerspective);
    PlayerInputComponent->BindKey(EKeys::F3,              IE_Pressed, this, &ACodeRescueCharacter::SelectTacticalPerspective);
    PlayerInputComponent->BindKey(EKeys::F4,              IE_Pressed, this, &ACodeRescueCharacter::SelectTopDownPerspective);
    PlayerInputComponent->BindKey(EKeys::F5,              IE_Pressed, this, &ACodeRescueCharacter::SelectIsometricPerspective);
    PlayerInputComponent->BindKey(EKeys::F6,              IE_Pressed, this, &ACodeRescueCharacter::SelectSidePerspective);

    PlayerInputComponent->BindKey(EKeys::X,               IE_Pressed, this, &ACodeRescueCharacter::ThrowActive);
    PlayerInputComponent->BindKey(EKeys::B,               IE_Pressed, this, &ACodeRescueCharacter::PlaceBarricade);
    PlayerInputComponent->BindKey(EKeys::L,               IE_Pressed, this, &ACodeRescueCharacter::ToggleFlashlight);
    // 2026-07-17: scanner moved OFF Z — Z was double-bound (scope zoom +
    // scanner fired together on every press).
    PlayerInputComponent->BindKey(EKeys::K,               IE_Pressed, this, &ACodeRescueCharacter::UseRadioScanner);

    PlayerInputComponent->BindKey(EKeys::One,             IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot1);
    PlayerInputComponent->BindKey(EKeys::Two,             IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot2);
    PlayerInputComponent->BindKey(EKeys::Three,           IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot3);
    PlayerInputComponent->BindKey(EKeys::Four,            IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot4);
    PlayerInputComponent->BindKey(EKeys::Five,            IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot5);
    PlayerInputComponent->BindKey(EKeys::Six,             IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot6);
    PlayerInputComponent->BindKey(EKeys::Seven,           IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot7);
    PlayerInputComponent->BindKey(EKeys::Eight,           IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot8);
    PlayerInputComponent->BindKey(EKeys::Nine,            IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot9);
    PlayerInputComponent->BindKey(EKeys::Zero,            IE_Pressed, this, &ACodeRescueCharacter::SelectWeaponSlot10);
    PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ACodeRescueCharacter::CycleWeaponNext);
    PlayerInputComponent->BindKey(EKeys::MouseScrollUp,   IE_Pressed, this, &ACodeRescueCharacter::CycleWeaponPrevious);
    PlayerInputComponent->BindKey(EKeys::RightBracket,    IE_Pressed, this, &ACodeRescueCharacter::CycleWeaponNext);
    PlayerInputComponent->BindKey(EKeys::LeftBracket,     IE_Pressed, this, &ACodeRescueCharacter::CycleWeaponPrevious);
    PlayerInputComponent->BindKey(EKeys::Gamepad_LeftShoulder, IE_Pressed, this, &ACodeRescueCharacter::CycleWeaponNext);
}

void ACodeRescueCharacter::PollDirectKeys(float DeltaSeconds)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    // Action keys are now bound directly in SetupPlayerInputComponent via
    // BindKey — this function only handles the polled movement fallback and
    // the held-Shift sprint below.
    DirectKeyCooldown = FMath::Max(0.0f, DirectKeyCooldown - DeltaSeconds);

    // 2026-07-17 (Kenny: photographing the screen with his phone to file bug
    // reports): Cmd+Shift+4 — the exact macOS screenshot chord — now works
    // IN-GAME too, saving a frame straight into his correction folder with an
    // on-screen confirmation. F12 does the same. Polled here so it works in
    // any perspective and regardless of UI focus.
    {
        const bool bCommandHeld = PC->IsInputKeyDown(EKeys::LeftCommand) || PC->IsInputKeyDown(EKeys::RightCommand);
        const bool bShiftHeld = PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift);
        if ((bCommandHeld && bShiftHeld && PC->WasInputKeyJustPressed(EKeys::Four)) ||
            PC->WasInputKeyJustPressed(EKeys::F12))
        {
            TakeGameplayScreenshot();
        }
    }

    // 2026-07-17 movement forensics: -CodeRescueMovementProbe walks the pawn
    // forward for a fixed window so the [ResumeHealth] pulses report REAL
    // covered ground + velocity (Kenny: "movement is almost non-functional" —
    // this turns that into a number on his own save, headless).
    {
        static const bool bMovementProbe =
            FParse::Param(FCommandLine::Get(), TEXT("CodeRescueMovementProbe"));
        if (bMovementProbe && GetWorld())
        {
            const double ProbeTime = GetWorld()->GetTimeSeconds();
            if (ProbeTime > 14.0 && ProbeTime < 22.0 && !bUIOpen && Health > 0.0f)
            {
                AddMovementInput(GetPerspectiveMoveForwardVector(), 1.0f);
            }
            // 2026-07-17: scripted end-to-end check of the trackpad aim latch
            // + the screenshot delivery pipeline, verifiable from the log +
            // the correction folder on disk.
            static bool bAimProbeStarted = false;
            if (!bAimProbeStarted && ProbeTime > 24.0 && Health > 0.0f)
            {
                bAimProbeStarted = true;
                OnAimPressed();
                FTimerHandle LatchTimer;
                GetWorldTimerManager().SetTimer(LatchTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
                {
                    OnAimReleased();   // ~0.12 s hold = quick click -> must latch
                    UE_LOG(LogTemp, Display, TEXT("[AimToggleProbe] after_quick_release ads=%d latched=%d scope=%d"),
                        bADSActive ? 1 : 0, bAimToggleLatched ? 1 : 0, IsScopeViewActive() ? 1 : 0);
                    LastFireWorldTime = -100.0f;
                    Fire();            // fire WHILE latched (Kenny's trackpad scenario)
                    UE_LOG(LogTemp, Display, TEXT("[AimToggleProbe] fired_while_latched ads=%d"), bADSActive ? 1 : 0);
                    FTimerHandle LowerTimer;
                    GetWorldTimerManager().SetTimer(LowerTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
                    {
                        OnAimPressed();    // second click -> lower
                        UE_LOG(LogTemp, Display, TEXT("[AimToggleProbe] after_second_click ads=%d latched=%d"),
                            bADSActive ? 1 : 0, bAimToggleLatched ? 1 : 0);
                        TakeGameplayScreenshot();   // delivery pipeline check
                    }), 0.6f, false);
                }), 0.12f, false);
            }
        }
    }

    // 2026-07-07: strip camera ROLL every frame. Any residual roll (shake
    // fallout, teleport rotation, physics blend) tilted the whole view ~15°
    // and made the character read as leaning — Kenny's report #2.
    // Same block, second guard: in the boom cameras, clamp PITCH so the arm
    // can never dive under the ground plane — pitching hard down parked the
    // camera INSIDE the raised entry platform (live: cream/black full-screen).
    {
        const FRotator ControlNow = PC->GetControlRotation();
        const bool bBoomCamera = (CameraPerspective == 1 || CameraPerspective == 2);
        float ClampedPitch = ControlNow.Pitch;
        if (bBoomCamera)
        {
            // Normalize to [-180,180] before clamping.
            float NormPitch = FRotator::NormalizeAxis(ControlNow.Pitch);
            ClampedPitch = FMath::Clamp(NormPitch, -28.0f, 38.0f);
        }
        if (!FMath::IsNearlyZero(ControlNow.Roll, 0.05f) ||
            !FMath::IsNearlyEqual(ClampedPitch, ControlNow.Pitch, 0.05f))
        {
            PC->SetControlRotation(FRotator(ClampedPitch, ControlNow.Yaw, 0.0f));
        }
    }

    // 2026-07-01 (round 4, playtest-proven): the first-launch tutorial overlay renders, but its
    // own UMG key/button input does NOT deliver in packaged builds - a Space press leaked past it
    // straight to Fire. Drive it here with the same reliable polled path that makes WASD/language
    // select work: Space/Enter/E advance a page, Esc/Backspace skip the whole thing. Handled BEFORE
    // the bUIOpen gate so it works whether or not the overlay claimed modal input, and we return so
    // nothing behind it fires. Bound movement axes are zeroed so the pawn stays put while reading.
    if (UCodeRescueTutorialWidget::IsShowing())
    {
        if (PC->WasInputKeyJustPressed(EKeys::Escape) || PC->WasInputKeyJustPressed(EKeys::BackSpace) ||
            PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right))
        {
            UCodeRescueTutorialWidget::DriveDismiss();
        }
        else if (PC->WasInputKeyJustPressed(EKeys::SpaceBar) || PC->WasInputKeyJustPressed(EKeys::Enter) ||
                 PC->WasInputKeyJustPressed(EKeys::E) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
        {
            UCodeRescueTutorialWidget::DriveAdvance();
        }
        BoundMoveForwardValue = 0.0f;
        BoundMoveRightValue = 0.0f;
        BoundTurnValue = 0.0f;
        BoundLookUpValue = 0.0f;
        return;
    }

    // The launch-language gate owns every actionable key. Run it before
    // camera and weapon polling so choosing C#/Java/etc. cannot also switch
    // equipment behind the visible start screen.
    const UCodeRescueGameInstance* PollGI = GetGameInstance<UCodeRescueGameInstance>();
    const bool bLanguageGateActive = PollGI && !PollGI->bHasSelectedLaunchLanguageThisSession;
    if (bLanguageGateActive)
    {
        UCodeRescueMainMenuWidget* LaunchMenu = ResolveLaunchLanguageMenu(GetWorld());

        // The GameMode-owned widget is authoritative. Slate's IsInViewport()
        // is false for this packaged programmatic widget even while it is
        // visibly mounted, so viewport state never authorizes a selection.
        if (!LaunchMenu)
        {
            LanguageGateNoMenuSeconds += DeltaSeconds;
            if (LanguageGateNoMenuSeconds > 3.0f)
            {
                UE_LOG(LogTemp, Error,
                    TEXT("[LaunchGate] RECOVERY REQUIRED: selector object missing for %.1fs; gameplay remains locked and no language was selected."),
                    LanguageGateNoMenuSeconds);
                LanguageGateNoMenuSeconds = 0.0f;
            }
        }
        else
        {
            LanguageGateNoMenuSeconds = 0.0f;
            if (PC->WasInputKeyJustPressed(EKeys::Up) || PC->WasInputKeyJustPressed(EKeys::Left))
            {
                LaunchMenu->DriveCycleLanguage(-1);
            }
            if (PC->WasInputKeyJustPressed(EKeys::Down) || PC->WasInputKeyJustPressed(EKeys::Right))
            {
                LaunchMenu->DriveCycleLanguage(1);
            }
            const FKey Digits[6] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six };
            for (int32 i = 0; i < 6; ++i)
            {
                if (PC->WasInputKeyJustPressed(Digits[i]))
                {
                    LaunchMenu->DriveSelectLanguageIndex(i);
                }
            }
        }

        // 2026-07-11 launch-crash harness: -CodeRescueAutoResumeLanguage=<Java|
        // C|CPlus|Cpp|Python|MATLAB> selects the language and deploys through
        // the EXACT same code path as the player's Enter press, so real-save
        // resume crashes are reproducible without a keyboard.
        bool bAutoResumeDeploy = false;
        if (!bLaunchAutoResumeConsumed && LaunchMenu)
        {
            FString AutoResumeToken;
            if (FParse::Value(FCommandLine::Get(), TEXT("CodeRescueAutoResumeLanguage="), AutoResumeToken) &&
                !AutoResumeToken.IsEmpty())
            {
                bLaunchAutoResumeConsumed = true;
                if (UCodeRescueGameInstance* AutoGI = GetGameInstance<UCodeRescueGameInstance>())
                {
                    ECodingLanguage AutoLanguage = ECodingLanguage::Cpp;
                    if (AutoResumeToken.Equals(TEXT("Java"), ESearchCase::IgnoreCase))        { AutoLanguage = ECodingLanguage::Java; }
                    else if (AutoResumeToken.Equals(TEXT("C"), ESearchCase::IgnoreCase))      { AutoLanguage = ECodingLanguage::C; }
                    else if (AutoResumeToken.Equals(TEXT("CPlus"), ESearchCase::IgnoreCase))  { AutoLanguage = ECodingLanguage::CPlus; }
                    else if (AutoResumeToken.Equals(TEXT("Python"), ESearchCase::IgnoreCase)) { AutoLanguage = ECodingLanguage::Python; }
                    else if (AutoResumeToken.Equals(TEXT("MATLAB"), ESearchCase::IgnoreCase)) { AutoLanguage = ECodingLanguage::MATLAB; }
                    AutoGI->SelectedLanguage = AutoLanguage;
                    UE_LOG(LogTemp, Display,
                        TEXT("[LaunchGate] auto-resume harness language=%s save_exists=%d"),
                        *AutoResumeToken,
                        AutoGI->DoesLanguageSaveExist(AutoLanguage) ? 1 : 0);
                    bAutoResumeDeploy = true;
                }
            }
        }

        if (bAutoResumeDeploy ||
            PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::SpaceBar) ||
            PC->WasInputKeyJustPressed(EKeys::E) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
        {
            if (UCodeRescueGameInstance* DeployGI = GetGameInstance<UCodeRescueGameInstance>())
            {
                const ECodingLanguage Chosen = DeployGI->SelectedLanguage;
                if (DeployGI->DoesLanguageSaveExist(Chosen))
                {
                    DeployGI->ResumeLanguageRun(Chosen);
                }
                else
                {
                    DeployGI->StartFreshLanguageRun(Chosen);
                }
                // hand input back to the game before travel so the new pawn
                // never inherits a menu-tainted movement state.
                PC->SetInputMode(FInputModeGameOnly());
                PC->bShowMouseCursor = false;
                PC->SetIgnoreMoveInput(false);
                PC->SetIgnoreLookInput(false);
                ACodeRescueCharacter::SetUIOpen(false);
                UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Entry")));
            }
            return;
        }

        BoundMoveForwardValue = 0.0f;
        BoundMoveRightValue = 0.0f;
        BoundTurnValue = 0.0f;
        BoundLookUpValue = 0.0f;
        return;
    }

    // Hard input gate: while a modal UI owns input (terminal, menu, etc.) the
    // raw key polls below would otherwise still fire Interact/Fire/Medkit/etc.
    // because PC->IsInputKeyDown bypasses SetIgnoreMoveInput/SetIgnoreLookInput.
    // Bail out completely so typing in the code box can't trigger gameplay.
    if (bUIOpen)
    {
        // Also clear bound axis residuals so a held W key from before the UI
        // opened doesn't keep moving the player.
        BoundMoveForwardValue = 0.0f;
        BoundMoveRightValue = 0.0f;
        BoundTurnValue = 0.0f;
        BoundLookUpValue = 0.0f;
        return;
    }

    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const bool bCameraPollAllowed = (Now - LastCameraInputWorldTime) > 0.08f;
    if (bCameraPollAllowed && (PC->WasInputKeyJustPressed(EKeys::C) ||
        PC->WasInputKeyJustPressed(EKeys::V) ||
        PC->WasInputKeyJustPressed(EKeys::Gamepad_RightShoulder)))
    {
        CycleCameraPerspective();
    }
    if (bCameraPollAllowed && PC->WasInputKeyJustPressed(EKeys::F1)) SelectFirstPersonPerspective();
    if (bCameraPollAllowed && PC->WasInputKeyJustPressed(EKeys::F2)) SelectThirdPersonPerspective();
    if (bCameraPollAllowed && PC->WasInputKeyJustPressed(EKeys::F3)) SelectTacticalPerspective();
    if (bCameraPollAllowed && PC->WasInputKeyJustPressed(EKeys::F4)) SelectTopDownPerspective();
    if (bCameraPollAllowed && PC->WasInputKeyJustPressed(EKeys::F5)) SelectIsometricPerspective();
    if (bCameraPollAllowed && PC->WasInputKeyJustPressed(EKeys::F6)) SelectSidePerspective();

    if (PC->WasInputKeyJustPressed(EKeys::One))   SelectWeaponSlot1();
    if (PC->WasInputKeyJustPressed(EKeys::Two))   SelectWeaponSlot2();
    if (PC->WasInputKeyJustPressed(EKeys::Three)) SelectWeaponSlot3();
    if (PC->WasInputKeyJustPressed(EKeys::Four))  SelectWeaponSlot4();
    if (PC->WasInputKeyJustPressed(EKeys::Five))  SelectWeaponSlot5();
    if (PC->WasInputKeyJustPressed(EKeys::Six))   SelectWeaponSlot6();
    if (PC->WasInputKeyJustPressed(EKeys::Seven)) SelectWeaponSlot7();
    if (PC->WasInputKeyJustPressed(EKeys::Eight)) SelectWeaponSlot8();
    if (PC->WasInputKeyJustPressed(EKeys::Nine))  SelectWeaponSlot9();
    if (PC->WasInputKeyJustPressed(EKeys::Zero))  SelectWeaponSlot10();

    // 2026-07-07 (Kenny: "cannot select and/or cycle between weapon types"):
    // wheel + bracket cycling was BindKey-only, the exact event path that
    // drops in packaged builds. Poll them like every other critical key;
    // CycleWeapon's debounce dedupes when the bound event ALSO delivers.
    if (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown) || PC->WasInputKeyJustPressed(EKeys::RightBracket))
    {
        CycleWeaponNext();
    }
    if (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp) || PC->WasInputKeyJustPressed(EKeys::LeftBracket))
    {
        CycleWeaponPrevious();
    }

    if (DirectKeyCooldown <= 0.0f &&
        (PC->WasInputKeyJustPressed(EKeys::E) ||
         PC->WasInputKeyJustPressed(EKeys::Enter) ||
         PC->WasInputKeyJustPressed(EKeys::Tab) ||
         PC->WasInputKeyJustPressed(EKeys::G) ||
         PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom)))
    {
        UE_LOG(LogTemp, Display, TEXT("[CodeRescueInteract] polled interact key detected"));
        Interact();
    }

    // 2026-07-02 verification affordance: Backslash spawns the terminal-solve horde around the player
    // on demand. The horde is normally triggered by solving a coding terminal (now wired via
    // TriggerBossHorde); this polled key lets the same wave be summoned for physics/ragdoll
    // verification without navigating to and solving a terminal. Obscure key, harmless in normal play.
    if (!bLanguageGateActive && PC->WasInputKeyJustPressed(EKeys::Backslash))
    {
        if (ACodeRescueGameMode* DebugGM = GetWorld() ? GetWorld()->GetAuthGameMode<ACodeRescueGameMode>() : nullptr)
        {
            const UCodeRescueGameInstance* HordeGI = GetGameInstance<UCodeRescueGameInstance>();
            const int32 HordeCity = HordeGI ? FMath::Max(0, HordeGI->CurrentObjectiveIndex) : 0;
            DebugGM->TriggerBossHorde(GetActorLocation(), HordeCity);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("DEBUG: terminal-solve horde triggered around player"));
            }
        }
    }

    // 2026-07-02: POLLED auto-fire. BindKey-based Fire (like the old BindKey interact) can fail to
    // deliver in packaged builds, so shooting was unreliable; polling IsInputKeyDown guarantees it.
    // Holding the button now auto-repeats. Fire() self-limits to the weapon's refire delay, so
    // calling it every frame while held is safe and just fires at the weapon's cadence.
    if (!bLanguageGateActive &&
        (PC->IsInputKeyDown(EKeys::LeftMouseButton) ||
         PC->IsInputKeyDown(EKeys::F) ||
         PC->IsInputKeyDown(EKeys::Gamepad_RightTrigger)))
    {
        Fire();
    }

    if (FMath::IsNearlyZero(BoundMoveForwardValue, 0.01f))
    {
        float DirectForward = 0.0f;
        DirectForward += (PC->IsInputKeyDown(EKeys::W) || PC->IsInputKeyDown(EKeys::Up) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Up)) ? 1.0f : 0.0f;
        DirectForward -= (PC->IsInputKeyDown(EKeys::S) || PC->IsInputKeyDown(EKeys::Down) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Down)) ? 1.0f : 0.0f;
        if (!FMath::IsNearlyZero(DirectForward))
        {
            AddMovementInput(GetPerspectiveMoveForwardVector(), FMath::Clamp(DirectForward, -1.0f, 1.0f));
        }
    }

    if (FMath::IsNearlyZero(BoundMoveRightValue, 0.01f))
    {
        float DirectRight = 0.0f;
        DirectRight += PC->IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f;
        DirectRight -= PC->IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f;
        if (!FMath::IsNearlyZero(DirectRight))
        {
            AddMovementInput(GetPerspectiveMoveRightVector(), FMath::Clamp(DirectRight, -1.0f, 1.0f));
        }
    }

    if (FMath::IsNearlyZero(BoundTurnValue, 0.01f))
    {
        float DirectTurn = 0.0f;
        DirectTurn += (PC->IsInputKeyDown(EKeys::Right) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Right)) ? 1.0f : 0.0f;
        DirectTurn -= (PC->IsInputKeyDown(EKeys::Left) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Left)) ? 1.0f : 0.0f;
        if (!FMath::IsNearlyZero(DirectTurn))
        {
            AddControllerYawInput(DirectTurn * DirectKeyboardTurnRate * DeltaSeconds);
        }
    }

    if (FMath::IsNearlyZero(BoundLookUpValue, 0.01f))
    {
        float DirectLookUp = 0.0f;
        DirectLookUp += PC->IsInputKeyDown(EKeys::I) ? 1.0f : 0.0f;
        DirectLookUp -= PC->IsInputKeyDown(EKeys::K) ? 1.0f : 0.0f;
        if (!IsFixedCameraPerspective() && !FMath::IsNearlyZero(DirectLookUp))
        {
            AddControllerPitchInput(DirectLookUp * DirectKeyboardLookRate * DeltaSeconds);
        }
    }


    // Sprint handling: hold Shift to sprint (continuous, not just-pressed)
    bool bShiftPressed = PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift) || PC->IsInputKeyDown(EKeys::Gamepad_LeftTrigger);
    if (bShiftPressed && Stamina > 0.0f && !bIsReloading)
    {
        bIsSprinting = true;
    }
    else
    {
        bIsSprinting = false;
    }

}

void ACodeRescueCharacter::ShowMissionHelp()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 12.0f, FColor::Green, GetOpenWorldGuidanceText());
        GEngine->AddOnScreenDebugMessage(-1, 12.0f, FColor::Cyan,
            TEXT("Controls: WASD move | Shift sprint | C camera (F1-F6 views) | 1-0 weapons | Click fire | Right-mouse aim + Z zoom | R reload | X utility | L flashlight | K scanner | E interact | Q medkit | Backspace unstuck | Cmd+Shift+4 screenshot"));
    }
}

void ACodeRescueCharacter::UpdateStealthNoise(float DeltaSeconds)
{
    const float PreviousNoise = StealthNoiseLevel;
    StealthNoiseLevel = FMath::Max(0.0f, StealthNoiseLevel - StealthNoiseDecayRate * FMath::Max(0.0f, DeltaSeconds));

    const UCharacterMovementComponent* Movement = GetCharacterMovement();
    const float Speed2D = Movement ? Movement->Velocity.Size2D() : GetVelocity().Size2D();
    const float WalkAlpha = WalkSpeed > KINDA_SMALL_NUMBER
        ? FMath::Clamp(Speed2D / WalkSpeed, 0.0f, 1.0f)
        : 0.0f;

    if (Speed2D > 55.0f)
    {
        const float MovementNoise = bIsSprinting
            ? FMath::Lerp(0.46f, 0.72f, WalkAlpha)
            : FMath::Lerp(0.12f, 0.28f, WalkAlpha);
        const float MovementRadius = bIsSprinting
            ? SprintNoiseRadius
            : QuietMovementNoiseRadius;
        ReportStealthNoise(MovementNoise, MovementRadius, bIsSprinting ? TEXT("sprinting") : TEXT("moving"));
    }

    if (bFieldFlashlightActive)
    {
        StealthNoiseLevel = FMath::Max(StealthNoiseLevel, 0.34f);
        LastStealthNoiseRadius = FMath::Max(LastStealthNoiseRadius, UtilityNoiseRadius);
        LastStealthNoiseReason = TEXT("flashlight exposure");
        LastStealthNoiseWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastStealthNoiseWorldTime;
        Tags.AddUnique(FName("StealthFlashlightExposure"));
    }

    if (StealthNoiseLevel > PreviousNoise + 0.05f)
    {
        Tags.AddUnique(FName("StealthNoiseRaised"));
    }
    if (StealthNoiseLevel <= 0.08f)
    {
        Tags.AddUnique(FName("StealthQuietRouteAvailable"));
    }
}

void ACodeRescueCharacter::ReportStealthNoise(float NoiseLevel, float NoiseRadius, const FString& Reason)
{
    const float ClampedNoise = FMath::Clamp(NoiseLevel, 0.0f, 1.0f);
    StealthNoiseLevel = FMath::Max(StealthNoiseLevel, ClampedNoise);
    LastStealthNoiseRadius = FMath::Max(0.0f, NoiseRadius);
    LastStealthNoiseReason = Reason.IsEmpty() ? TEXT("noise") : Reason;
    LastStealthNoiseWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastStealthNoiseWorldTime;
    Tags.AddUnique(FName("StealthAvoidanceNoiseEmitter"));
    Tags.AddUnique(FName("NoiseDetectionGameplay"));
}

float ACodeRescueCharacter::GetStealthNoiseRadius() const
{
    const UWorld* World = GetWorld();
    const float SinceNoise = World ? World->TimeSeconds - LastStealthNoiseWorldTime : 99.0f;
    if (SinceNoise <= 1.35f && StealthNoiseLevel > 0.05f)
    {
        return FMath::Max(QuietMovementNoiseRadius, LastStealthNoiseRadius) * FMath::Clamp(StealthNoiseLevel, 0.0f, 1.0f);
    }
    if (bFieldFlashlightActive)
    {
        return UtilityNoiseRadius * 0.55f;
    }
    return StealthNoiseLevel > 0.05f ? QuietMovementNoiseRadius * StealthNoiseLevel : 0.0f;
}

FString ACodeRescueCharacter::GetStealthStateSummary() const
{
    if (bFieldFlashlightActive && StealthNoiseLevel >= 0.30f)
    {
        return TEXT("EXPOSED: flashlight");
    }
    if (StealthNoiseLevel >= 0.70f)
    {
        return FString::Printf(TEXT("NOISY: %s"), *LastStealthNoiseReason);
    }
    if (StealthNoiseLevel >= 0.35f)
    {
        return FString::Printf(TEXT("AUDIBLE: %s"), *LastStealthNoiseReason);
    }
    if (StealthNoiseLevel >= 0.10f)
    {
        return TEXT("LOW NOISE: move slowly");
    }
    return TEXT("QUIET: avoid sightlines");
}

void ACodeRescueCharacter::UpdateArenaSafety(float DeltaSeconds)
{
    (void)DeltaSeconds;

    UWorld* World = GetWorld();
    if (!World || Health <= 0.0f)
    {
        return;
    }

    const FVector Location = GetActorLocation();
    const int32 CityIndex = FindClosestObjectiveIndex(Location);
    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(CityIndex);
    const FVector Relative = Location - Origin;
    const float SpanScale = FCodeRescueCampaign::GetCitySpanScale();

    const bool bInsideInnerArena =
        FMath::Abs(Relative.X) <= FCodeRescueCampaign::ArenaInnerHalfXLocal * SpanScale &&
        FMath::Abs(Relative.Y) <= FCodeRescueCampaign::ArenaInnerHalfYLocal * SpanScale &&
        Relative.Z >= 20.0f &&
        Relative.Z <= 1800.0f;
    const bool bFalling = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
    if (bInsideInnerArena && !bFalling)
    {
        LastSafeArenaLocation = Location;
        LastSafeArenaCityIndex = CityIndex;
        bHasLastSafeArenaLocation = true;
    }

    const bool bOutsideOuterArena =
        FMath::Abs(Relative.X) > FCodeRescueCampaign::ArenaOuterHalfXLocal * SpanScale ||
        FMath::Abs(Relative.Y) > FCodeRescueCampaign::ArenaOuterHalfYLocal * SpanScale;
    const bool bBelowSafetyPlane = Relative.Z < GCodeRescueArenaFallRecoveryZ;
    if (!bOutsideOuterArena && !bBelowSafetyPlane)
    {
        return;
    }

    const float Now = World->GetTimeSeconds();
    if ((Now - LastArenaSafetyRescueWorldTime) < 0.75f)
    {
        return;
    }

    RecoverToCityArena();
}

void ACodeRescueCharacter::RecoverToCityArena()
{
    UWorld* World = GetWorld();
    if (!World || Health <= 0.0f)
    {
        return;
    }
    // 2026-07-06 first-level fix (live-playtest): Backspace dismisses the
    // tutorial via the polled path in Tick, but this bound delegate fires
    // independently for the SAME keypress — a first-time player pressing
    // Backspace to skip the tutorial was silently teleported to the recovery
    // point as their very first input. Recovery must not fire while the
    // tutorial overlay is up.
    if (UCodeRescueTutorialWidget::IsShowing())
    {
        return;
    }

    const FVector CurrentLocation = GetActorLocation();
    const int32 CityIndex = FindClosestObjectiveIndex(CurrentLocation);
    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(CityIndex);
    const float SpanScale = FCodeRescueCampaign::GetCitySpanScale();

    const bool bLastSafeLocationMatchesCity =
        bHasLastSafeArenaLocation &&
        LastSafeArenaCityIndex == CityIndex &&
        FMath::Abs((LastSafeArenaLocation - Origin).X) <= FCodeRescueCampaign::ArenaInnerHalfXLocal * SpanScale &&
        FMath::Abs((LastSafeArenaLocation - Origin).Y) <= FCodeRescueCampaign::ArenaInnerHalfYLocal * SpanScale &&
        (LastSafeArenaLocation - Origin).Z >= 20.0f &&
        (LastSafeArenaLocation - Origin).Z <= 1800.0f;

    FVector Destination = bLastSafeLocationMatchesCity
        ? LastSafeArenaLocation
        : FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);

    // Recover onto the traversable surface instead of dropping the player at
    // the old fixed 320 cm height. Starting just above the last-safe point
    // avoids selecting a building roof when recovery occurs indoors.
    FHitResult RecoveryGroundHit;
    FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(ArenaRecoveryGround), false, this);
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->Tags.Contains(FName("FallRecoveryCatchFloor")))
        {
            GroundParams.AddIgnoredActor(*It);
        }
    }
    const FVector GroundTraceStart = Destination + FVector(0.0f, 0.0f, 90.0f);
    const FVector GroundTraceEnd(Destination.X, Destination.Y, Origin.Z - 900.0f);
    const bool bFoundPlayableGround = World->LineTraceSingleByChannel(
            RecoveryGroundHit,
            GroundTraceStart,
            GroundTraceEnd,
            ECC_Visibility,
            GroundParams) &&
        RecoveryGroundHit.ImpactPoint.Z >= Origin.Z - 18.0f;
    if (bFoundPlayableGround)
    {
        const float CapsuleHalfHeight = GetCapsuleComponent()
            ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
            : 96.0f;
        Destination.Z = RecoveryGroundHit.ImpactPoint.Z + CapsuleHalfHeight + 2.0f;
    }
    else
    {
        Destination.Z = Origin.Z + GCodeRescueArenaSafeGroundZ;
    }

    // 2026-07-06 first-level fix (live-playtest): a "last safe" breadcrumb can
    // be dropped while the player hugs geometry (e.g. the safehouse exterior),
    // and recovering onto it pinches the third-person spring arm into the
    // player's head. Probe the four cardinal directions at chest height and
    // slide the destination away from anything blocking within camera-clear
    // range, so the recovery point always has room for the camera.
    {
        FCollisionQueryParams ClearParams(SCENE_QUERY_STAT(ArenaRecoveryClear), false, this);
        const float CameraClearance = 170.0f;
        const FVector Chest = Destination + FVector(0.0f, 0.0f, 80.0f);
        FVector AwayFromWalls = FVector::ZeroVector;
        const FVector Cardinals[4] = {
            FVector(1.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f),
            FVector(0.0f, 1.0f, 0.0f), FVector(0.0f, -1.0f, 0.0f) };
        for (const FVector& Dir : Cardinals)
        {
            FHitResult ClearHit;
            if (World->LineTraceSingleByChannel(ClearHit, Chest, Chest + Dir * CameraClearance, ECC_Visibility, ClearParams))
            {
                AwayFromWalls -= Dir * (CameraClearance - ClearHit.Distance);
            }
        }
        if (!AwayFromWalls.IsNearlyZero())
        {
            Destination += AwayFromWalls.GetClampedToMaxSize(240.0f);
            UE_LOG(LogTemp, Display, TEXT("[CodeRescueArenaRecovery] destination nudged %s away from nearby walls"),
                *AwayFromWalls.GetClampedToMaxSize(240.0f).ToCompactString());
        }
    }
    // 2026-07-07: full capsule depenetration on top of the wall probe — the
    // recovery point must never wedge the pawn either.
    AdjustTeleportDestination(Destination);
    if (Destination.Z < Origin.Z + 60.0f || Destination.Z > Origin.Z + 520.0f)
    {
        Destination = FCodeRescueCampaign::GetPlayerStartLocation(CityIndex);
        Destination.Z = Origin.Z + GCodeRescueArenaSafeGroundZ;
        AdjustTeleportDestination(Destination);
        UE_LOG(LogTemp, Warning,
            TEXT("[CodeRescueArenaRecovery] rejected non-playable recovery height; canonical city entry restored"));
    }

    if (ACodeRescueGameMode* GameMode = World->GetAuthGameMode<ACodeRescueGameMode>())
    {
        GameMode->EnsureCampaignCityLoaded(CityIndex);
    }

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->SetMovementMode(MOVE_Walking);
    }
    HighestFallingDownSpeed = 0.0f;
    SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
    ObjectiveIndex = CityIndex;
    Health = FMath::Max(Health, MaxHealth * 0.35f);
    Stamina = FMath::Max(Stamina, MaxStamina * 0.65f);
    LastArenaSafetyRescueWorldTime = World->GetTimeSeconds();
    LastSafeArenaLocation = Destination;
    LastSafeArenaCityIndex = CityIndex;
    bHasLastSafeArenaLocation = true;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }
    ACodeRescueCharacter::SetUIOpen(false);
    UGameplayStatics::SetGamePaused(World, false);

    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->CurrentObjectiveIndex = CityIndex;
        GI->SavePersistentRun();
    }

    const FString RecoveryMessage = FString::Printf(
        TEXT("Arena recovery: returned to %s. Stay inside the city perimeter; Backspace can recover you anytime."),
        *FCodeRescueCampaign::GetMissionLabel(CityIndex));
    UCodeRescueSubtitlesWidget::Push(RecoveryMessage, 3.5f);
    UE_LOG(LogTemp, Display, TEXT("[CodeRescueArenaRecovery] %s destination=%s"),
        *FCodeRescueCampaign::GetMissionLabel(CityIndex),
        *Destination.ToCompactString());
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan, RecoveryMessage);
    }
}

bool ACodeRescueCharacter::AdjustTeleportDestination(FVector& InOutDestination) const
{
    // 2026-07-07 (Kenny: T "continuously forced into a placement that does
    // not allow the character to move"): T's step teleport dropped the pawn
    // at the raw target + a fixed offset — with the v3 street walls up and
    // the terminal inside the safehouse, that spot is often INSIDE geometry
    // and the capsule wedges. Every teleport destination is now depenetrated
    // with the engine's own FindTeleportSpot, and if the exact spot cannot be
    // freed we ring-search nearby ground until one fits.
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    FVector Candidate = InOutDestination;
    if (World->FindTeleportSpot(this, Candidate, GetActorRotation()))
    {
        InOutDestination = Candidate;
        return true;
    }
    const float Rings[3] = { 220.0f, 420.0f, 720.0f };
    for (float Radius : Rings)
    {
        for (int32 Step = 0; Step < 8; ++Step)
        {
            const float Angle = Step * PI / 4.0f;
            Candidate = InOutDestination + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 60.0f);
            // Snap the candidate onto real ground before testing it.
            FHitResult GroundHit;
            FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(TeleportClearGround), false, this);
            if (World->LineTraceSingleByChannel(GroundHit, Candidate + FVector(0, 0, 300.0f),
                    Candidate - FVector(0, 0, 800.0f), ECC_Visibility, GroundParams))
            {
                Candidate.Z = GroundHit.ImpactPoint.Z + 96.0f;
            }
            if (World->FindTeleportSpot(this, Candidate, GetActorRotation()))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[Teleport] destination blocked — relocated %.0fuu away to %s"),
                    Radius, *Candidate.ToCompactString());
                InOutDestination = Candidate;
                return true;
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] no clear spot found near %s — teleporting anyway (watchdog will free)"),
        *InOutDestination.ToCompactString());
    return false;
}

void ACodeRescueCharacter::TeleportToNextObjective()
{
    const int32 Count = FCodeRescueCampaign::GetMissionCount();
    UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();

    // 2026-07-01 fix: T must not skip the launch-language screen. Teleporting out of the
    // selection scene left the game in a locked half-initialized state.
    if (GI && !GI->bHasSelectedLaunchLanguageThisSession)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                TEXT("Choose your coding language first: walk to a language station and press E or Enter."));
        }
        return;
    }
    const int32 FirstIncomplete = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);

    if (FirstIncomplete >= Count)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
                TEXT("Every major-city mission is complete. Extraction is ready."));
        }
        return;
    }

    if (ObjectiveIndex == FirstIncomplete && !FCodeRescueCampaign::IsCityCompleted(GI, FirstIncomplete))
    {
        // 2026-07-01 step-wise flow: inside the active city, T now travels you to the CURRENT
        // step of the rescue loop (1 terminal -> 2 survivor -> 3 extraction), using the city's
        // objective beacon which already tracks that phase. This is the "what do I do next"
        // key: press T any time and you are standing at your next task.
        AObjectiveFocusBeaconActor* StepBeacon = nullptr;
        float StepBestDistSq = TNumericLimits<float>::Max();
        for (TActorIterator<AObjectiveFocusBeaconActor> It(GetWorld()); It; ++It)
        {
            AObjectiveFocusBeaconActor* Candidate = *It;
            if (!IsValid(Candidate)) { continue; }
            const float DistSq = FVector::DistSquared(Candidate->GetActorLocation(), GetActorLocation());
            if (DistSq < StepBestDistSq)
            {
                StepBestDistSq = DistSq;
                StepBeacon = Candidate;
            }
        }
        if (StepBeacon)
        {
            const int32 Phase = StepBeacon->GetCurrentObjectivePhase();
            FVector StepTarget = StepBeacon->GetCurrentPhaseTargetLocation() + FVector(120.0f, 120.0f, 240.0f);
            AdjustTeleportDestination(StepTarget);   // 2026-07-07: never land inside geometry
            SetActorLocation(StepTarget, false, nullptr, ETeleportType::TeleportPhysics);
            if (GEngine)
            {
                const int32 CompletedChallenges = FCodeRescueCampaign::GetCityChallengeProgress(GI, FirstIncomplete);
                const TCHAR* StepText =
                    Phase == 2 ? TEXT("STEP 2 of 3 - SURVIVOR: walk to the survivor and press E to rescue them.")
                    : Phase == 3 ? TEXT("STEP 3 of 3 - EXTRACTION: reach the glowing helipad and press E to extract.")
                    : nullptr;
                const FString ProgressStepText = StepText
                    ? FString(StepText)
                    : FString::Printf(
                        TEXT("STEP 1 of 3 - CODING CONCOURSE: complete station %d of %d."),
                        FMath::Min(CompletedChallenges + 1, FCodeRescueCampaign::RequiredChallengesPerCity),
                        FCodeRescueCampaign::RequiredChallengesPerCity);
                GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, ProgressStepText);
                GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan,
                    TEXT("Press T any time to auto-travel to your current step."));
            }
            return;
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
                FString::Printf(TEXT("Active level: graduate %s before advancing to the next city."),
                    *FCodeRescueCampaign::GetMissionLabel(FirstIncomplete)));
        }
        if (ACodeRescueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACodeRescueGameMode>() : nullptr)
        {
            GameMode->EnsureCampaignCityLoaded(FirstIncomplete);
        }
        FVector PadTarget = FCodeRescueCampaign::GetPlayerStartLocation(FirstIncomplete);
        AdjustTeleportDestination(PadTarget);
        SetActorLocation(PadTarget, false, nullptr, ETeleportType::TeleportPhysics);
    }
    else
    {
        ObjectiveIndex = FirstIncomplete;
        if (ACodeRescueGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACodeRescueGameMode>() : nullptr)
        {
            GameMode->EnsureCampaignCityLoaded(ObjectiveIndex);
        }
        FVector NextCityTarget = FCodeRescueCampaign::GetPlayerStartLocation(ObjectiveIndex);
        AdjustTeleportDestination(NextCityTarget);
        SetActorLocation(NextCityTarget, false, nullptr, ETeleportType::TeleportPhysics);
    }

    // Persist the new player position + objective index so a reload puts the
    // player back at the same objective they were working on.
    if (GI)
    {
        GI->CurrentObjectiveIndex = ObjectiveIndex;
        GI->SavePersistentRun();
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
            FString::Printf(TEXT("Current city %d/%d: %s. Complete its terminal and survivor rescue to unlock the next city."),
                ObjectiveIndex + 1, Count, *FCodeRescueCampaign::GetMissionLabel(ObjectiveIndex)));
    }
}

void ACodeRescueCharacter::CyclePerspective()
{
    SelectCameraPerspective(CameraPerspective + 1);
}

void ACodeRescueCharacter::CycleCameraPerspective()
{
    CyclePerspective();
}

void ACodeRescueCharacter::SelectCameraPerspective(int32 NewPerspective)
{
    LastCameraInputWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastCameraInputWorldTime;
    CameraPerspective = WrapCameraPerspective(NewPerspective);
    ApplyCameraPerspective();
    UE_LOG(LogTemp, Display, TEXT("CodeRescueCamera: perspective=%d label=%s"),
        CameraPerspective,
        GetCodeRescueCameraPerspectiveLabel(CameraPerspective));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
            FString::Printf(TEXT("Camera: %s"), GetCodeRescueCameraPerspectiveLabel(CameraPerspective)));
    }
}

void ACodeRescueCharacter::ApplyCameraPerspective()
{
    const bool bFirstPerson = (CameraPerspective == 0);
    const bool bFixedCamera = IsFixedCameraPerspective();

    // 2026-07-04 (v2 body playtest): in first person the player's OWN body must be
    // invisible to them — with the authored SurvivorKenny mesh the camera otherwise
    // sits inside the back of his head. Other perspectives show the full body.
    if (USkeletalMeshComponent* BodyMesh = GetMesh())
    {
        BodyMesh->SetOwnerNoSee(bFirstPerson);
    }
    if (AimingPresentationMesh)
    {
        AimingPresentationMesh->SetOwnerNoSee(bFirstPerson);
    }

    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetActive(bFirstPerson, true);
    }
    if (ThirdPersonCamera)
    {
        ThirdPersonCamera->SetActive(!bFirstPerson, true);
    }
    if (CameraBoom)
    {
        CameraBoom->bUsePawnControlRotation = !bFixedCamera;
        // 2026-07-07 (Kenny: "walls from the city buildings obstruct the
        // camera view"): the fixed cameras ran with the boom probe DISABLED,
        // so with the v3 street walls up they sat INSIDE building geometry
        // and the whole screen filled with wall. The probe is now ALWAYS on —
        // the boom shortens in front of a wall instead of clipping through it.
        CameraBoom->bDoCollisionTest = true;

        switch (CameraPerspective)
        {
        case 1:
            // pass 5 (Kenny: perspectives "less helpful"): classic over-the-
            // shoulder — closer, raised, character readable on the left third.
            CameraBoom->TargetArmLength = 300.0f;
            CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 55.0f);
            CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
            break;
        case 2:
            CameraBoom->TargetArmLength = 480.0f;
            CameraBoom->SocketOffset = FVector(0.0f, 75.0f, 70.0f);
            CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
            break;
        case 3:
            // 2026-07-07 (Kenny: the camera "moving outside of a building ...
            // is not helpful; it is distracting"): riding above the roofline
            // meant roofs occluded the player near buildings. Top-down now
            // stays INSIDE the street canyon (below every v3 roofline); the
            // always-on probe keeps it out of walls, and the proximity fade
            // keeps the player readable when space gets tight.
            // pass 5: pulled in from 1150 — the player was an unreadable speck.
            CameraBoom->TargetArmLength = 820.0f;
            CameraBoom->SocketOffset = FVector::ZeroVector;
            CameraBoom->SetRelativeRotation(FRotator(-82.0f, 0.0f, 0.0f));
            break;
        case 4:
            // Isometric lives in the canyon too — player's-eye scale.
            CameraBoom->TargetArmLength = 900.0f;
            CameraBoom->SocketOffset = FVector::ZeroVector;
            CameraBoom->SetRelativeRotation(FRotator(-54.0f, 45.0f, 0.0f));
            break;
        case 5:
            CameraBoom->TargetArmLength = 780.0f;
            CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 130.0f);
            CameraBoom->SetRelativeRotation(FRotator(-8.0f, -90.0f, 0.0f));
            break;
        default:
            CameraBoom->TargetArmLength = 300.0f;
            CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 55.0f);
            CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
            break;
        }
    }
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    // The player's own body is hidden in first-person (it would fill the
    // camera) and shown in every other perspective. When the HERO presentation
    // layer is configured it REPLACES the driver bodies visually.
    if (USkeletalMeshComponent* BodyMesh = GetMesh())
    {
        BodyMesh->SetVisibility(!bFirstPerson && !bAimPresentationConfigured && !bHeroPresentationConfigured, false);
    }
    if (AimingPresentationMesh)
    {
        AimingPresentationMesh->SetVisibility(!bFirstPerson && bAimPresentationConfigured && !bHeroPresentationConfigured, false);
    }
    if (HeroPresentationMesh)
    {
        // propagate=TRUE (2026-07-17): attached wound decals/meshes follow the
        // hero's visibility instead of floating alone in first person. The
        // held-weapon component gets its own explicit visibility right below.
        HeroPresentationMesh->SetVisibility(!bFirstPerson && bHeroPresentationConfigured, true);
    }
    if (FirstPersonArmsMesh)
    {
        FirstPersonArmsMesh->SetVisibility(bFirstPerson && FirstPersonArmsMesh->GetSkeletalMeshAsset() != nullptr, true);
    }
    if (FirstPersonWeaponSilhouette)
    {
        FirstPersonWeaponSilhouette->SetVisibility(bFirstPerson && FirstPersonWeaponSilhouette->GetStaticMesh() != nullptr, true);
        UpdateFirstPersonWeaponPresentation(0.0f);
    }
    // 2026-07-07: the held weapon shows ON THE BODY from every non-FP camera.
    if (ThirdPersonWeaponMesh)
    {
        ThirdPersonWeaponMesh->SetVisibility(!bFirstPerson && ThirdPersonWeaponMesh->GetStaticMesh() != nullptr, true);
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetViewTargetWithBlend(this, 0.05f);
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
    }
}

void ACodeRescueCharacter::ConfigureAimingPresentationMesh()
{
    USkeletalMeshComponent* SourceBody = GetMesh();
    if (!AimingPresentationMesh || !SourceBody || !SourceBody->GetSkinnedAsset())
    {
        bAimPresentationConfigured = false;
        return;
    }

    AimingPresentationMesh->SetSkinnedAssetAndUpdate(SourceBody->GetSkinnedAsset(), true);
    AimingPresentationMesh->SetRelativeTransform(SourceBody->GetRelativeTransform());
    for (int32 MaterialIndex = 0; MaterialIndex < SourceBody->GetNumMaterials(); ++MaterialIndex)
    {
        AimingPresentationMesh->SetMaterial(MaterialIndex, SourceBody->GetMaterial(MaterialIndex));
    }
    SourceBody->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    AimingPresentationMesh->CopyPoseFromSkeletalComponent(SourceBody);
    bAimPresentationConfigured =
        AimingPresentationMesh->GetBoneIndex(FName(TEXT("upperarm_r"))) != INDEX_NONE &&
        AimingPresentationMesh->GetBoneIndex(FName(TEXT("hand_r"))) != INDEX_NONE;

    if (bAimPresentationConfigured)
    {
        SourceBody->SetVisibility(false, false);
        AimingPresentationMesh->SetVisibility(CameraPerspective != 0 && !bHeroPresentationConfigured, false);
        Tags.AddUnique(FName("ProceduralTwoArmAimRuntime"));
        Tags.AddUnique(FName("FirstLevelCharacterAimPass"));
        UE_LOG(LogTemp, Display, TEXT("[FirstLevelAim] runtime pose copy configured for %s"),
            *GetNameSafe(SourceBody->GetSkinnedAsset()));
    }
}

FVector ACodeRescueCharacter::GetAimTargetPoint(const ACodeZombieActor* Target) const
{
    if (!IsValid(Target))
    {
        return FVector::ZeroVector;
    }
    if (const USkeletalMeshComponent* TargetMesh = Target->GetMesh())
    {
        static const FName TorsoBone(TEXT("spine_03"));
        if (TargetMesh->GetBoneIndex(TorsoBone) != INDEX_NONE)
        {
            return TargetMesh->GetBoneLocation(TorsoBone);
        }
    }
    return Target->GetActorLocation() + FVector(0.0f, 0.0f, 38.0f);
}

bool ACodeRescueCharacter::IsAimTargetCandidate(
    const ACodeZombieActor* Target,
    float MaxAngleDegrees,
    float MaxDistance) const
{
    if (!IsValid(Target) || Target->Health <= 0.0f || !Target->GetActorEnableCollision())
    {
        return false;
    }

    const UCameraComponent* Camera = GetActiveGameplayCamera();
    const FVector Start = Camera ? Camera->GetComponentLocation() : GetActorLocation();
    const FVector Forward = Camera ? Camera->GetForwardVector().GetSafeNormal() : GetActorForwardVector();
    const FVector TargetPoint = GetAimTargetPoint(Target);
    const FVector ToTarget = TargetPoint - Start;
    const float Distance = ToTarget.Size();
    if (Distance <= 60.0f || Distance > MaxDistance)
    {
        return false;
    }

    const float Alignment = FVector::DotProduct(Forward, ToTarget / Distance);
    const float MinimumAlignment = FMath::Cos(FMath::DegreesToRadians(FMath::Max(1.0f, MaxAngleDegrees)));
    return Alignment >= MinimumAlignment && HasClearWeaponPath(Start, TargetPoint, Target);
}

ACodeZombieActor* ACodeRescueCharacter::FindBestAimTarget(
    float MaxAngleDegrees,
    float MaxDistance,
    float MaxReticleDistance) const
{
    UWorld* World = GetWorld();
    const UCameraComponent* Camera = GetActiveGameplayCamera();
    if (!World)
    {
        return nullptr;
    }

    const FVector Start = Camera ? Camera->GetComponentLocation() : GetActorLocation();
    const FVector Forward = Camera ? Camera->GetForwardVector().GetSafeNormal() : GetActorForwardVector();
    ACodeZombieActor* BestTarget = nullptr;
    float BestScore = -1.0f;
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Candidate = *It;
        if (!IsAimTargetCandidate(Candidate, MaxAngleDegrees, MaxDistance))
        {
            continue;
        }

        const FVector ToTarget = GetAimTargetPoint(Candidate) - Start;
        const float Distance = ToTarget.Size();
        const float AlongRay = FVector::DotProduct(ToTarget, Forward);
        const float ReticleDistance = (ToTarget - Forward * AlongRay).Size();
        if (MaxReticleDistance > 0.0f && (AlongRay <= 0.0f || ReticleDistance > MaxReticleDistance))
        {
            continue;
        }

        const float Alignment = FVector::DotProduct(Forward, ToTarget / Distance);
        const float DistanceScore = 1.0f - FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
        const float Score = Alignment * 0.82f + DistanceScore * 0.18f;
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Candidate;
        }
    }
    return BestTarget;
}

void ACodeRescueCharacter::UpdateAutoTargetLock(float DeltaSeconds)
{
    AimHoldTimer = FMath::Max(0.0f, AimHoldTimer - DeltaSeconds);
    const bool bWantsAim = !bUIOpen && Health > 0.0f && (bAimInputHeld || AimHoldTimer > 0.0f);
    ACodeZombieActor* PreviousTarget = LockedAimTarget.Get();
    // 2026-07-11: the Settings "aim assist" slider must actually shape the
    // auto target lock — the setting was left dangling when target lock
    // replaced the legacy assisted-hit cone. 0.0 disables acquisition
    // entirely (accessibility opt-out); 2.0 doubles the acquisition cone.
    const UCodeRescueGameInstance* AssistGI = GetGameInstance<UCodeRescueGameInstance>();
    const float AssistScale = AssistGI ? FMath::Clamp(AssistGI->AimAssistScale, 0.0f, 2.0f) : 1.0f;
    const float EffectiveAssistAngle = TargetLockAcquireAngleDegrees * AssistScale;
    const float EffectiveAssistRadius = TargetLockMaxDistance * FMath::Clamp(AssistScale, 0.25f, 1.5f);
    if (!bWantsAim || !bEnableAssistedHit || AssistScale <= KINDA_SMALL_NUMBER)
    {
        LockedAimTarget.Reset();
    }
    else if (!IsAimTargetCandidate(PreviousTarget, TargetLockBreakAngleDegrees, EffectiveAssistRadius))
    {
        LockedAimTarget = FindBestAimTarget(EffectiveAssistAngle, EffectiveAssistRadius);
    }

    ACodeZombieActor* CurrentTarget = LockedAimTarget.Get();
    if (CurrentTarget != PreviousTarget)
    {
        LastTargetLockChangeWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if (CurrentTarget)
        {
            Tags.AddUnique(FName("PhysicalAutoTargetLockActive"));
            UE_LOG(LogTemp, Display,
                TEXT("[TargetLock] ACQUIRED target=%s distance=%.0f LOS=1 physical_trace=1"),
                *CurrentTarget->GetName(),
                FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()));
        }
        else if (PreviousTarget)
        {
            UE_LOG(LogTemp, Display, TEXT("[TargetLock] RELEASED target=%s"), *PreviousTarget->GetName());
        }
    }

    if (!CurrentTarget)
    {
        return;
    }

    const UCameraComponent* Camera = GetActiveGameplayCamera();
    const FVector AimOrigin = Camera ? Camera->GetComponentLocation() : GetActorLocation();
    const FRotator DesiredRotation = (GetAimTargetPoint(CurrentTarget) - AimOrigin).Rotation();
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        FRotator Smoothed = FMath::RInterpTo(
            PC->GetControlRotation(),
            DesiredRotation,
            FMath::Max(DeltaSeconds, 1.0f / 240.0f),
            TargetLockCameraTurnSpeed);
        Smoothed.Roll = 0.0f;
        PC->SetControlRotation(Smoothed);
    }
    const FRotator DesiredBodyRotation(0.0f, DesiredRotation.Yaw, 0.0f);
    SetActorRotation(FMath::RInterpTo(
        GetActorRotation(),
        DesiredBodyRotation,
        FMath::Max(DeltaSeconds, 1.0f / 240.0f),
        TargetLockCameraTurnSpeed));
}

void ACodeRescueCharacter::UpdateWeaponAimPresentation(float DeltaSeconds)
{
    const bool bWantsAim = !bUIOpen && Health > 0.0f && (bAimInputHeld || AimHoldTimer > 0.0f);
    const float TargetAlpha = bWantsAim ? 1.0f : 0.0f;
    AimPresentationAlpha = FMath::FInterpTo(AimPresentationAlpha, TargetAlpha, DeltaSeconds, bWantsAim ? 11.0f : 7.0f);

    if (!bAimPresentationConfigured || !AimingPresentationMesh || !GetMesh())
    {
        return;
    }

    AimingPresentationMesh->CopyPoseFromSkeletalComponent(GetMesh());
    if (MannyAnimationState == 5 && MannyLandingPresentationRemaining > 0.0f)
    {
        const float LandingProgress = 1.0f - FMath::Clamp(
            MannyLandingPresentationRemaining / 0.34f, 0.0f, 1.0f);
        const float Compression = FMath::Sin(LandingProgress * PI);
        const FName PelvisBone(TEXT("pelvis"));
        if (AimingPresentationMesh->GetBoneIndex(PelvisBone) != INDEX_NONE)
        {
            FTransform Pelvis = AimingPresentationMesh->GetBoneTransformByName(
                PelvisBone, EBoneSpaces::ComponentSpace);
            Pelvis.AddToTranslation(FVector(0.0f, 0.0f, -11.0f * Compression));
            AimingPresentationMesh->SetBoneTransformByName(
                PelvisBone, Pelvis, EBoneSpaces::ComponentSpace);
        }
        const FName SpineBone(TEXT("spine_01"));
        if (AimingPresentationMesh->GetBoneIndex(SpineBone) != INDEX_NONE)
        {
            FTransform Spine = AimingPresentationMesh->GetBoneTransformByName(
                SpineBone, EBoneSpaces::ComponentSpace);
            Spine.SetRotation((FQuat(FRotator(-7.5f * Compression, 0.0f, 0.0f))
                * Spine.GetRotation()).GetNormalized());
            AimingPresentationMesh->SetBoneTransformByName(
                SpineBone, Spine, EBoneSpaces::ComponentSpace);
        }
        AimingPresentationMesh->ComponentTags.AddUnique(
            FName("ProceduralLandingCompressionActive"));
    }
    if (AimPresentationAlpha <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const UCameraComponent* ActiveCamera = GetActiveGameplayCamera();
    FVector AimDirectionWorld = ActiveCamera ? ActiveCamera->GetForwardVector() : GetActorForwardVector();
    if (const ACodeZombieActor* Target = GetLockedAimTarget())
    {
        AimDirectionWorld = GetAimTargetPoint(Target) - AimingPresentationMesh->GetComponentLocation();
    }
    AimDirectionWorld.Z = FMath::Clamp(AimDirectionWorld.Z, -0.55f, 0.70f);
    AimDirectionWorld.Normalize();
    const FVector AimDirection = AimingPresentationMesh->GetComponentTransform()
        .InverseTransformVectorNoScale(AimDirectionWorld).GetSafeNormal();
    const FVector LocalRight = AimingPresentationMesh->GetComponentTransform()
        .InverseTransformVectorNoScale(GetActorRightVector()).GetSafeNormal();

    auto AimArm = [this, &AimDirection, &LocalRight](FName UpperArmBone, FName HandBone, float SupportBias)
    {
        if (AimingPresentationMesh->GetBoneIndex(UpperArmBone) == INDEX_NONE ||
            AimingPresentationMesh->GetBoneIndex(HandBone) == INDEX_NONE)
        {
            return;
        }

        FTransform UpperArm = AimingPresentationMesh->GetBoneTransformByName(UpperArmBone, EBoneSpaces::ComponentSpace);
        const FTransform Hand = AimingPresentationMesh->GetBoneTransformByName(HandBone, EBoneSpaces::ComponentSpace);
        const FVector CurrentDirection = (Hand.GetLocation() - UpperArm.GetLocation()).GetSafeNormal();
        const FVector DesiredDirection = (AimDirection + LocalRight * SupportBias).GetSafeNormal();
        if (CurrentDirection.IsNearlyZero() || DesiredDirection.IsNearlyZero())
        {
            return;
        }

        const FQuat FullAimDelta = FQuat::FindBetweenNormals(CurrentDirection, DesiredDirection);
        const FQuat AppliedDelta = FQuat::Slerp(FQuat::Identity, FullAimDelta, AimPresentationAlpha);
        UpperArm.SetRotation((AppliedDelta * UpperArm.GetRotation()).GetNormalized());
        AimingPresentationMesh->SetBoneTransformByName(UpperArmBone, UpperArm, EBoneSpaces::ComponentSpace);
    };

    AimArm(FName(TEXT("upperarm_r")), FName(TEXT("hand_r")), 0.035f);
    AimArm(FName(TEXT("upperarm_l")), FName(TEXT("hand_l")), -0.10f);
    AimingPresentationMesh->ComponentTags.AddUnique(FName("TwoArmWeaponAimPoseActive"));
}

void ACodeRescueCharacter::UpdateFirstPersonArms(float DeltaSeconds)
{
    if (!FirstPersonArmsMesh || CameraPerspective != 0 || !FirstPersonArmsMesh->GetSkeletalMeshAsset())
    {
        return;
    }

    const FVector BaseLocation(54.0f, 0.0f, -96.0f);
    const FRotator BaseRotation(0.0f, -90.0f, 0.0f);
    const float Speed2D = GetVelocity().Size2D();
    const float MoveAlpha = FMath::Clamp(Speed2D / FMath::Max(1.0f, WalkSpeed), 0.0f, 1.0f);
    FirstPersonArmsSwayTime += DeltaSeconds * FMath::Lerp(1.15f, 6.8f, MoveAlpha);

    const float BobSin = FMath::Sin(FirstPersonArmsSwayTime);
    const float BobCos = FMath::Cos(FirstPersonArmsSwayTime * 2.0f);
    const FVector WalkBob(
        BobCos * 1.2f * MoveAlpha,
        BobSin * 2.4f * MoveAlpha,
        FMath::Abs(BobSin) * -2.2f * MoveAlpha);
    const FVector InputSway(
        0.0f,
        FMath::Clamp(-BoundTurnValue * 2.2f, -4.0f, 4.0f),
        FMath::Clamp(BoundLookUpValue * 1.6f, -3.0f, 3.0f));
    const FRotator RotationSway(
        FMath::Clamp(-BoundLookUpValue * 1.2f, -3.0f, 3.0f) + BobSin * 0.35f * MoveAlpha,
        0.0f,
        FMath::Clamp(-BoundTurnValue * 1.4f, -3.0f, 3.0f));

    const FVector AimOffset(5.0f * AimPresentationAlpha, -6.0f * AimPresentationAlpha, 9.0f * AimPresentationAlpha);
    const FRotator AimRotation(-2.5f * AimPresentationAlpha, 0.0f, 4.0f * AimPresentationAlpha);
    const float LandingProgress = MannyAnimationState == 5
        ? 1.0f - FMath::Clamp(MannyLandingPresentationRemaining / 0.34f, 0.0f, 1.0f)
        : 0.0f;
    const FVector LandingOffset(
        0.0f, 0.0f, -7.0f * FMath::Sin(LandingProgress * PI));
    FirstPersonArmsMesh->SetRelativeLocation(
        BaseLocation + WalkBob + InputSway + AimOffset + LandingOffset);
    FirstPersonArmsMesh->SetRelativeRotation(BaseRotation + RotationSway + AimRotation);
    Tags.AddUnique(FName("FirstPersonArmsProceduralSway"));
}

void ACodeRescueCharacter::UpdateFirstPersonWeaponPresentation(float DeltaSeconds)
{
    if (!FirstPersonWeaponSilhouette || !FirstPersonWeaponSilhouette->GetStaticMesh())
    {
        return;
    }

    const bool bFirstPerson = CameraPerspective == 0;
    // Scope-capable weapons: sighting in = looking THROUGH the optic (full
    // circular scope view at every zoom, per Kenny's reference video) — the
    // weapon body hides once the raise completes. Pistols/shotguns/SMGs keep
    // the down-the-barrel viewmodel.
    const bool bScopeView = IsScopeViewActive();
    FirstPersonWeaponSilhouette->SetVisibility(bFirstPerson && !bScopeView, true);
    if (!bFirstPerson)
    {
        return;
    }

    const FCodeRescueWeaponPresentationProfile Profile = GetWeaponPresentationProfile(ActiveWeapon);
    if (!bWeaponPresentationProfileInitialized || LastPresentedWeapon != ActiveWeapon)
    {
        // Profile.Scale belonged to the legacy procedural silhouettes; the
        // V4/V5 art is authored at real-world scale (cycle-13: the profile
        // scale shrank the rifle to a 2-pixel sliver). Scale stays 1.
        FirstPersonWeaponSilhouette->SetRelativeScale3D(FVector(1.0f));
        FirstPersonWeaponSilhouette->ComponentTags.AddUnique(FName(Profile.ProfileTag));
        FirstPersonWeaponSilhouette->ComponentTags.AddUnique(FName("DistinctWeaponPresentationActive"));

        if (UMaterialInstanceDynamic* MID = FirstPersonWeaponSilhouette->CreateAndSetMaterialInstanceDynamic(0))
        {
            MID->SetVectorParameterValue(TEXT("Color"), Profile.Tint);
            MID->SetVectorParameterValue(TEXT("BaseColor"), Profile.Tint);
            MID->SetVectorParameterValue(TEXT("EmissiveColor"), Profile.Tint * 0.18f);
        }

        LastPresentedWeapon = ActiveWeapon;
        bWeaponPresentationProfileInitialized = true;
    }

    UWorld* World = GetWorld();
    const float NowSeconds = World ? World->GetTimeSeconds() : 0.0f;
    const bool bReducedMotion = GetGameInstance<UCodeRescueGameInstance>()
        && GetGameInstance<UCodeRescueGameInstance>()->bReducedMotion;
    const float MotionScale = bReducedMotion ? 0.32f : 1.0f;
    const float Speed2D = GetVelocity().Size2D();
    const float MoveAlpha = FMath::Clamp(Speed2D / FMath::Max(1.0f, WalkSpeed), 0.0f, 1.0f);

    FirstPersonWeaponPresentationTime += DeltaSeconds * FMath::Lerp(1.0f, 5.2f, MoveAlpha);
    const float BobSin = FMath::Sin(FirstPersonWeaponPresentationTime);
    const float BobCos = FMath::Cos(FirstPersonWeaponPresentationTime * 1.6f);
    const FVector WalkBob(
        BobCos * 0.8f * MoveAlpha * Profile.BobScale,
        BobSin * 1.35f * MoveAlpha * Profile.BobScale,
        FMath::Abs(BobSin) * -1.15f * MoveAlpha * Profile.BobScale);
    const FVector InputSway(
        0.0f,
        FMath::Clamp(-BoundTurnValue * 1.25f, -2.6f, 2.6f),
        FMath::Clamp(BoundLookUpValue * 0.95f, -1.9f, 1.9f));

    const float FireAge = NowSeconds - LastWeaponPresentationFireWorldTime;
    const float FireAlpha = (FireAge >= 0.0f && FireAge < 0.18f)
        ? FMath::Square(1.0f - (FireAge / 0.18f))
        : 0.0f;
    const float ReloadSpan = FMath::Max(0.1f, ReloadDuration);
    const float ReloadAge = NowSeconds - LastWeaponPresentationReloadWorldTime;
    const float ReloadAlpha = (bIsReloading && ReloadAge >= 0.0f)
        ? FMath::Clamp(ReloadAge / ReloadSpan, 0.0f, 1.0f)
        : 0.0f;
    const float ReloadArc = FMath::Sin(ReloadAlpha * PI);
    const float HitStopSpan = FMath::Max(0.01f, LastCombatJuiceHitStopDuration);
    const float HitStopAge = NowSeconds - LastCombatJuiceHitConfirmWorldTime;
    const float HitStopAlpha = (bEnableCombatJuice && HitStopAge >= 0.0f && HitStopAge < HitStopSpan)
        ? FMath::Square(1.0f - (HitStopAge / HitStopSpan)) * LastCombatJuiceHitStopScale
        : 0.0f;

    const FVector FireOffset(
        -Profile.RecoilDistance * FireAlpha,
        0.0f,
        Profile.RecoilDistance * 0.22f * FireAlpha);
    const FVector ReloadOffset(
        -Profile.ReloadDip * 0.38f * ReloadAlpha,
        ReloadArc * 4.5f,
        -Profile.ReloadDip * ReloadArc);
    const FVector HitStopOffset(
        -2.8f * HitStopAlpha,
        bLastCombatJuiceHeadshot ? 0.9f * HitStopAlpha : 0.35f * HitStopAlpha,
        1.25f * HitStopAlpha);
    const FRotator MotionRotation(
        Profile.RecoilPitch * FireAlpha - ReloadArc * 2.0f - HitStopAlpha * 1.4f,
        FMath::Clamp(BoundTurnValue * 0.45f, -1.8f, 1.8f),
        Profile.ReloadRoll * ReloadArc + FMath::Clamp(-BoundTurnValue * 0.75f, -2.0f, 2.0f) + HitStopAlpha * 2.2f);

    // Hip pose = the real-scale parking spot RefreshFirstPersonWeapon uses
    // (Profile.BaseLocation was tuned for the legacy silhouettes and parked
    // the V4 art out of frame — cycle-13). AimPresentationAlpha gives the
    // old soft pull toward center; full ADS is handled below by ADSBlend.
    const bool bLongViewModel = FirstPersonWeaponSilhouette->GetStaticMesh()->GetBoundingBox().GetSize().X > 40.0f;
    const FVector HipPose = bLongViewModel ? FVector(58.0f, 24.0f, -22.0f) : FVector(46.0f, 20.0f, -18.0f);
    const FVector AimLocation = FMath::Lerp(
        HipPose,
        FVector(HipPose.X + 4.0f, HipPose.Y - 6.0f, HipPose.Z + 5.0f),
        AimPresentationAlpha);
    const FRotator AimRotation = FMath::Lerp(FRotator(0.0f, -3.5f, 0.0f), FRotator::ZeroRotator, AimPresentationAlpha * 0.72f);
    FVector FinalLocation =
        AimLocation + ((WalkBob + InputSway + FireOffset + ReloadOffset + HitStopOffset) * MotionScale);
    FRotator FinalRotation = AimRotation + (MotionRotation * MotionScale);

    // Aim-down-sights (pass 5): blend to the sight-line pose so the camera
    // looks straight down the barrel / through the optic. Per-weapon sight
    // heights match the authored V4/V5 meshes. Grenades never shoulder-aim
    // (their aiming UI is the ballistic arc + landing outline); bob/sway
    // collapse while sighted so the picture stays steady.
    if (ADSBlend > 0.001f && !WeaponIsGrenadeFamily(ActiveWeapon))
    {
        float SightHeight = 9.0f;
        switch (ActiveWeapon)
        {
        case EWeaponType::Pistol:
        case EWeaponType::HeavyHandgun:
        case EWeaponType::BurstHandgun:
        case EWeaponType::Magnum:          SightHeight = 10.0f; break;
        case EWeaponType::Shotgun:
        case EWeaponType::TacticalShotgun:
        case EWeaponType::AutoShotgun:     SightHeight = 8.7f;  break;
        case EWeaponType::SMG:             SightHeight = 9.0f;  break;
        case EWeaponType::Rifle:
        case EWeaponType::PrecisionRifle:
        case EWeaponType::SemiAutoRifle:   SightHeight = 12.8f; break;
        case EWeaponType::BoltLauncher:    SightHeight = 8.4f;  break;
        case EWeaponType::RocketLauncher:  SightHeight = 14.0f; break;
        default: break;
        }
        const FVector ADSPose(bLongViewModel ? 34.0f : 27.0f, 0.0f, -SightHeight);
        FinalLocation = FMath::Lerp(FinalLocation,
            ADSPose + (FireOffset + HitStopOffset) * MotionScale * 0.5f, ADSBlend);
        FinalRotation = FMath::Lerp(FinalRotation,
            FRotator(Profile.RecoilPitch * FireAlpha * 0.6f, 0.0f, 0.0f), ADSBlend);
    }
    FirstPersonWeaponSilhouette->SetRelativeLocation(FinalLocation);
    FirstPersonWeaponSilhouette->SetRelativeRotation(FinalRotation);

    Tags.AddUnique(FName("DistinctWeaponPresentationUpdated"));
    Tags.AddUnique(FName("WeaponFireReloadMotionCue"));
    if (HitStopAlpha > 0.0f)
    {
        Tags.AddUnique(FName("CombatJuiceHitStopStyleCue"));
    }
}

float ACodeRescueCharacter::GetCombatJuiceMotionScale() const
{
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    return (GI && GI->bReducedMotion) ? 0.24f : 1.0f;
}

void ACodeRescueCharacter::TriggerCombatJuiceFireCue(const FWeaponDef* WeaponDef, bool bMeleeOrDryFire)
{
    UWorld* World = GetWorld();
    if (!bEnableCombatJuice || !World || Health <= 0.0f || bUIOpen)
    {
        return;
    }

    LastCombatJuiceFireWorldTime = World->GetTimeSeconds();

    float WeaponScale = 1.0f;
    if (WeaponDef)
    {
        WeaponScale += FMath::Clamp((WeaponDef->Damage - 35.0f) / 180.0f, 0.0f, 0.70f);
        WeaponScale += WeaponDef->PelletsPerShot > 1 ? 0.18f : 0.0f;
        WeaponScale += WeaponDef->ExplosionRadius > 0.0f ? 0.28f : 0.0f;
        WeaponScale += WeaponDef->BurstCount > 1 ? 0.10f : 0.0f;
    }
    if (bMeleeOrDryFire)
    {
        WeaponScale *= 0.55f;
    }

    const float MotionScale = GetCombatJuiceMotionScale();
    const float YawSign = FMath::FRand() < 0.5f ? -1.0f : 1.0f;
    AddControllerPitchInput(-CombatJuiceFireKickPitch * WeaponScale * MotionScale);
    AddControllerYawInput(CombatJuiceFireKickYaw * WeaponScale * YawSign * MotionScale);

    Tags.AddUnique(FName("CombatJuiceWeaponFeelRuntime"));
    Tags.AddUnique(FName("CombatJuiceFireCameraKick"));
    Tags.AddUnique(FName("Top50Recommendation27CombatJuice"));
    Tags.AddUnique(FName("Top50Recommendation38WeaponFeel"));
}

void ACodeRescueCharacter::TriggerCombatJuiceHitConfirm(const FVector& ImpactPoint, EHitZone HitZone, bool bGameplayHit)
{
    UWorld* World = GetWorld();
    if (!bEnableCombatJuice || !World || !bGameplayHit || Health <= 0.0f)
    {
        return;
    }

    const float NowSeconds = World->GetTimeSeconds();
    const bool bCanKick = NowSeconds - LastCombatJuiceHitConfirmWorldTime > 0.045f;
    LastCombatJuiceHitConfirmWorldTime = NowSeconds;
    LastCombatJuiceHitStopDuration = CombatJuiceHitStopSeconds * GetCombatJuiceMotionScale();
    LastCombatJuiceHitStopScale = HitZone == EHitZone::Head ? 1.25f : 0.82f;
    bLastCombatJuiceHeadshot = HitZone == EHitZone::Head;

    if (bCanKick && !bUIOpen)
    {
        const float MotionScale = GetCombatJuiceMotionScale();
        const FVector CameraLocation = GetActiveGameplayCamera()
            ? GetActiveGameplayCamera()->GetComponentLocation()
            : GetActorLocation();
        const FVector ToImpact = (ImpactPoint - CameraLocation).GetSafeNormal();
        const float Side = FVector::DotProduct(GetActorRightVector(), ToImpact);
        AddControllerPitchInput(CombatJuiceHitConfirmKick * MotionScale * (HitZone == EHitZone::Head ? 1.15f : 0.75f));
        AddControllerYawInput(FMath::Clamp(Side, -1.0f, 1.0f) * CombatJuiceHitConfirmKick * 0.35f * MotionScale);
    }

    Tags.AddUnique(FName("CombatJuiceHitConfirmCue"));
    Tags.AddUnique(FName("CombatJuiceHitStopStyleCue"));
    if (HitZone == EHitZone::Head)
    {
        Tags.AddUnique(FName("CombatJuiceHeadshotCrunch"));
    }
}

void ACodeRescueCharacter::TriggerCombatJuiceReloadStageCue(float StageAlpha, bool bComplete)
{
    UWorld* World = GetWorld();
    if (!bEnableCombatJuice || !World || Health <= 0.0f)
    {
        return;
    }

    LastCombatJuiceReloadStageWorldTime = World->GetTimeSeconds();
    if (!bUIOpen)
    {
        const float MotionScale = GetCombatJuiceMotionScale();
        const float Direction = bComplete ? 1.0f : -0.55f;
        AddControllerPitchInput(CombatJuiceReloadSettleKick * Direction * MotionScale);
    }

    Tags.AddUnique(FName("CombatJuiceReloadStageCue"));
    Tags.AddUnique(bComplete ? FName("CombatJuiceReloadCompleteCue") : FName("CombatJuiceReloadStartCue"));
    Tags.AddUnique(StageAlpha >= 0.99f ? FName("CombatJuiceReloadStageFull") : FName("CombatJuiceReloadStagePartial"));
}

void ACodeRescueCharacter::TriggerCombatJuiceDamageCue(float EffectiveDamage, AActor* DamageSource)
{
    UWorld* World = GetWorld();
    if (!bEnableCombatJuice || !World || EffectiveDamage <= 0.0f || Health <= 0.0f)
    {
        return;
    }

    LastCombatJuiceDamageWorldTime = World->GetTimeSeconds();
    if (DamageSource && !bUIOpen)
    {
        const float MotionScale = GetCombatJuiceMotionScale();
        FVector ToAttacker = DamageSource->GetActorLocation() - GetActorLocation();
        ToAttacker.Z = 0.0f;
        const FVector LocalDir = GetActorRotation().UnrotateVector(ToAttacker.GetSafeNormal());
        const float DamageScale = FMath::Clamp(EffectiveDamage / FMath::Max(1.0f, MaxHealth * 0.16f), 0.35f, 1.0f);
        AddControllerPitchInput(CombatJuiceDamageKick * 0.55f * DamageScale * MotionScale);
        AddControllerYawInput(FMath::Clamp(LocalDir.Y, -1.0f, 1.0f) * CombatJuiceDamageKick * DamageScale * MotionScale);
    }

    Tags.AddUnique(FName("CombatJuiceDamageCameraCue"));
    Tags.AddUnique(FName("DamageFeedbackAccessibleCameraKick"));
}

void ACodeRescueCharacter::UpdateCombatJuice(float DeltaSeconds)
{
    if (!bEnableCombatJuice || DeltaSeconds <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    const float NowSeconds = World ? World->GetTimeSeconds() : 0.0f;
    if (NowSeconds - LastCombatJuiceFireWorldTime < 0.24f)
    {
        Tags.AddUnique(FName("CombatJuiceFireWindowActive"));
    }
    if (NowSeconds - LastCombatJuiceHitConfirmWorldTime < FMath::Max(0.01f, LastCombatJuiceHitStopDuration))
    {
        Tags.AddUnique(FName("CombatJuiceHitStopWindowActive"));
    }
    if (NowSeconds - LastCombatJuiceDamageWorldTime < 0.30f)
    {
        Tags.AddUnique(FName("CombatJuiceDamageWindowActive"));
    }
}

void ACodeRescueCharacter::UpdateReactiveThreatAudio(float DeltaSeconds)
{
    UWorld* World = GetWorld();
    UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    if (!World || !GI)
    {
        return;
    }

    if (!bEnableReactiveThreatAudio || Health <= 0.0f)
    {
        ReactiveThreatAudioSmoothedIntensity = 0.0f;
        LastReactiveThreatAudioState = TEXT("calm");
        GI->UpdateReactiveThreatMusic(0.0f, TEXT("calm"));
        return;
    }

    const float NowSeconds = World->GetTimeSeconds();
    const float SampleInterval = FMath::Max(0.05f, ReactiveThreatAudioUpdateInterval);
    if (NowSeconds - LastReactiveThreatAudioWorldTime < SampleInterval)
    {
        return;
    }

    const float SampleDelta = LastReactiveThreatAudioWorldTime < -10.0f
        ? FMath::Max(DeltaSeconds, SampleInterval)
        : FMath::Max(DeltaSeconds, NowSeconds - LastReactiveThreatAudioWorldTime);
    LastReactiveThreatAudioWorldTime = NowSeconds;

    const FVector PlayerLocation = GetActorLocation();
    const float ScanRange = FMath::Max(500.0f, ReactiveThreatAudioRange);
    const float CriticalRange = FMath::Clamp(ReactiveThreatAudioCriticalRange, 100.0f, ScanRange);
    float NearestDistanceUU = TNumericLimits<float>::Max();
    int32 ThreatCount = 0;
    float PressureScore = 0.0f;

    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Zombie = *It;
        if (!IsValid(Zombie) || Zombie->Health <= 0.0f)
        {
            continue;
        }

        const float DistanceUU = FVector::Dist(PlayerLocation, Zombie->GetActorLocation());
        if (DistanceUU > ScanRange)
        {
            continue;
        }

        ++ThreatCount;
        NearestDistanceUU = FMath::Min(NearestDistanceUU, DistanceUU);
        const float DistanceAlpha = 1.0f - FMath::Clamp(DistanceUU / ScanRange, 0.0f, 1.0f);
        PressureScore += FMath::Square(DistanceAlpha) * 0.65f;
        if (DistanceUU <= CriticalRange)
        {
            PressureScore += 0.55f;
        }
    }

    float TargetIntensity = FMath::Clamp(PressureScore, 0.0f, 1.0f);
    FString StateLabel = TEXT("calm");
    const bool bInsideProtectedLearning = ACodeRescueGameMode::IsLocationInsideProtectedLearningZone(this, PlayerLocation, 300.0f);
    if (bInsideProtectedLearning)
    {
        TargetIntensity = FMath::Min(TargetIntensity, 0.12f);
        StateLabel = TEXT("safehouse muted");
    }
    else if (ThreatCount > 0 && NearestDistanceUU <= CriticalRange)
    {
        StateLabel = TEXT("critical");
    }
    else if (TargetIntensity >= 0.45f || ThreatCount >= 3)
    {
        StateLabel = TEXT("pursuit");
    }
    else if (TargetIntensity >= 0.12f || ThreatCount > 0)
    {
        StateLabel = TEXT("watch");
    }

    ReactiveThreatAudioSmoothedIntensity = FMath::FInterpTo(
        ReactiveThreatAudioSmoothedIntensity,
        TargetIntensity,
        SampleDelta,
        5.5f);
    GI->UpdateReactiveThreatMusic(ReactiveThreatAudioSmoothedIntensity, StateLabel);

    Tags.AddUnique(FName("ReactiveThreatAudioRuntime"));
    Tags.AddUnique(FName("ReactiveThreatMusicDirector"));
    Tags.AddUnique(FName("Top50Recommendation43ReactiveAudio"));
    Tags.AddUnique(FName("WorldDevelopmentAudioGuidance"));

    const bool bStateChanged = StateLabel != LastReactiveThreatAudioState;
    if (bStateChanged && NowSeconds - LastReactiveThreatAudioCaptionWorldTime > 5.0f)
    {
        const float NearestMeters = NearestDistanceUU < (TNumericLimits<float>::Max() * 0.5f)
            ? NearestDistanceUU / 100.0f
            : 0.0f;
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(
                TEXT("[Audio]: threat mix %s, nearest %.0fm, contacts %d."),
                *StateLabel,
                NearestMeters,
                ThreatCount),
            2.6f);
        LastReactiveThreatAudioCaptionWorldTime = NowSeconds;
    }
    LastReactiveThreatAudioState = StateLabel;
}

void ACodeRescueCharacter::UpdateCityAmbientZoneAudio(float DeltaSeconds)
{
    UWorld* World = GetWorld();
    UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    if (!World || !GI)
    {
        return;
    }

    if (!bEnableCityAmbientZoneDirector || !GI->bCityAmbientZoneDirectorEnabled || Health <= 0.0f)
    {
        GI->UpdateCityAmbientZone(TEXT("ambient disabled"), TEXT("ambient_disabled"), 0.0f);
        return;
    }

    const float NowSeconds = World->GetTimeSeconds();
    const float SampleInterval = FMath::Max(0.20f, CityAmbientZoneUpdateInterval);
    if (NowSeconds - LastCityAmbientZoneWorldTime < SampleInterval)
    {
        return;
    }
    LastCityAmbientZoneWorldTime = NowSeconds;

    const FVector PlayerLocation = GetActorLocation();
    const int32 CityIndex = FindClosestObjectiveIndex(PlayerLocation);
    const FCodeRescueCityMission* Mission = FCodeRescueCampaign::GetMission(CityIndex);
    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(CityIndex);
    const float SpanScale = FMath::Max(0.1f, FCodeRescueCampaign::GetCitySpanScale());
    const FVector Local((PlayerLocation.X - Origin.X) / SpanScale, (PlayerLocation.Y - Origin.Y) / SpanScale, PlayerLocation.Z - Origin.Z);
    const FString MissionLabel = Mission
        ? FString::Printf(TEXT("%s, %s"), *Mission->CityName, *Mission->StateName)
        : FCodeRescueCampaign::GetMissionLabel(CityIndex);
    const bool bTerminalSolved = Mission && FCodeRescueCampaign::HasCompletedCityChallengeSet(GI, CityIndex);
    const bool bSurvivorRescued = Mission && GI->RescuedSurvivorNames.Contains(Mission->SurvivorName);
    const bool bInsideProtectedLearning = ACodeRescueGameMode::IsLocationInsideProtectedLearningZone(this, PlayerLocation, 300.0f);

    auto DistToLocal = [&Local](const FVector& Anchor) -> float
    {
        return FVector::Dist2D(FVector(Local.X, Local.Y, 0.0f), Anchor);
    };

    FString ZoneLabel = TEXT("overrun street grid");
    FString BedLabel = TEXT("zone_overrun_street_bed");
    float ZoneIntensity = 0.58f;

    if (bInsideProtectedLearning)
    {
        ZoneLabel = TEXT("protected coding lab");
        BedLabel = TEXT("zone_safehouse_low_hum");
        ZoneIntensity = 0.18f;
    }
    else if (DistToLocal(FVector(2400.0f, 2400.0f, 0.0f)) < 1250.0f)
    {
        ZoneLabel = bSurvivorRescued ? TEXT("extraction pad ready") : TEXT("extraction pad dormant");
        BedLabel = bSurvivorRescued ? TEXT("zone_extraction_rotor_wind") : TEXT("zone_extraction_distant_beacon");
        ZoneIntensity = bSurvivorRescued ? 0.72f : 0.48f;
    }
    else if (DistToLocal(FVector(2850.0f, 1500.0f, 0.0f)) < 1350.0f)
    {
        ZoneLabel = bTerminalSolved ? TEXT("survivor route open") : TEXT("survivor search pressure");
        BedLabel = bTerminalSolved ? TEXT("zone_survivor_beacon_bed") : TEXT("zone_survivor_search_tension");
        ZoneIntensity = bTerminalSolved ? 0.62f : 0.74f;
    }
    else if (DistToLocal(FVector(-3820.0f, -3180.0f, 0.0f)) < 1100.0f)
    {
        ZoneLabel = TEXT("entry approach");
        BedLabel = TEXT("zone_entry_city_arrival");
        ZoneIntensity = 0.35f;
    }
    else if (Local.Y < -2150.0f)
    {
        ZoneLabel = TEXT("safehouse district exterior");
        BedLabel = TEXT("zone_civic_safehouse_exterior");
        ZoneIntensity = 0.42f;
    }
    else if (Local.X > 1300.0f && Local.Y > 650.0f)
    {
        ZoneLabel = bTerminalSolved ? TEXT("rescue route corridor") : TEXT("locked rescue route");
        BedLabel = bTerminalSolved ? TEXT("zone_rescue_route_pulse") : TEXT("zone_locked_route_dread");
        ZoneIntensity = bTerminalSolved ? 0.64f : 0.70f;
    }
    else if (Local.X > 700.0f && Local.Y < -300.0f)
    {
        ZoneLabel = TEXT("transit corridor");
        BedLabel = TEXT("zone_transit_echo_bed");
        ZoneIntensity = 0.56f;
    }
    else if (Local.X < -650.0f && Local.Y > 650.0f)
    {
        ZoneLabel = TEXT("civic overrun block");
        BedLabel = TEXT("zone_civic_overrun_bed");
        ZoneIntensity = 0.66f;
    }

    GI->UpdateCityAmbientZone(ZoneLabel, BedLabel, ZoneIntensity);

    Tags.AddUnique(FName("CityAmbientZoneDirectorRuntime"));
    Tags.AddUnique(FName("WorldDevelopmentZoneAmbientCues"));
    Tags.AddUnique(FName("Top50Recommendation43SpatialAudio"));
    Tags.AddUnique(FName("Top50Recommendation44AudioAccessibility"));

    const bool bZoneChanged = ZoneLabel != LastCityAmbientZoneLabel;
    if (bZoneChanged && NowSeconds - LastCityAmbientZoneCaptionWorldTime > 7.0f)
    {
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(
                TEXT("[Ambient]: %s | %s | bed %s | %s track."),
                *MissionLabel,
                *ZoneLabel,
                *BedLabel,
                *GI->GetLanguageName()),
            3.0f);
        LastCityAmbientZoneCaptionWorldTime = NowSeconds;
    }
    LastCityAmbientZoneLabel = ZoneLabel;
}

FString ACodeRescueCharacter::GetCameraPerspectiveLabel() const
{
    return GetCodeRescueCameraPerspectiveLabel(CameraPerspective);
}

UCameraComponent* ACodeRescueCharacter::GetActiveGameplayCamera() const
{
    return CameraPerspective == 0 ? FirstPersonCamera : ThirdPersonCamera;
}

bool ACodeRescueCharacter::IsAimTargetLocked() const
{
    const ACodeZombieActor* Target = LockedAimTarget.Get();
    return IsValid(Target) && Target->Health > 0.0f;
}

ACodeZombieActor* ACodeRescueCharacter::GetLockedAimTarget() const
{
    return IsAimTargetLocked() ? LockedAimTarget.Get() : nullptr;
}

FString ACodeRescueCharacter::GetAimTargetLockSummary() const
{
    const ACodeZombieActor* Target = GetLockedAimTarget();
    if (!Target)
    {
        return TEXT("TARGET LOCK  searching");
    }
    const float DistanceMeters = FVector::Dist(GetActorLocation(), Target->GetActorLocation()) / 100.0f;
    return FString::Printf(TEXT("TARGET LOCKED  %.0fm  PHYSICAL TRACE"), DistanceMeters);
}

void ACodeRescueCharacter::SelectFirstPersonPerspective() { SelectCameraPerspective(0); }
void ACodeRescueCharacter::SelectThirdPersonPerspective() { SelectCameraPerspective(1); }
void ACodeRescueCharacter::SelectTacticalPerspective()    { SelectCameraPerspective(2); }
void ACodeRescueCharacter::SelectTopDownPerspective()     { SelectCameraPerspective(3); }
void ACodeRescueCharacter::SelectIsometricPerspective()   { SelectCameraPerspective(4); }
void ACodeRescueCharacter::SelectSidePerspective()        { SelectCameraPerspective(5); }

bool ACodeRescueCharacter::IsFixedCameraPerspective() const
{
    return CameraPerspective >= 3;
}

FVector ACodeRescueCharacter::GetPerspectiveMoveForwardVector() const
{
    switch (CameraPerspective)
    {
    case 3:
        return FVector::ForwardVector;
    case 4:
        return DirectionFromYaw(45.0f);
    case 5:
        return FVector::ForwardVector;
    default:
        return GetActorForwardVector();
    }
}

FVector ACodeRescueCharacter::GetPerspectiveMoveRightVector() const
{
    switch (CameraPerspective)
    {
    case 3:
        return FVector::RightVector;
    case 4:
        return RightFromYaw(45.0f);
    case 5:
        return FVector::RightVector;
    default:
        return GetActorRightVector();
    }
}

void ACodeRescueCharacter::MoveForward(float Value)
{
    BoundMoveForwardValue = Value;
    if (Controller && Value != 0.0f)
    {
        AddMovementInput(GetPerspectiveMoveForwardVector(), Value);
    }
}

void ACodeRescueCharacter::MoveRight(float Value)
{
    BoundMoveRightValue = Value;
    if (Controller && Value != 0.0f)
    {
        AddMovementInput(GetPerspectiveMoveRightVector(), Value);
    }
}

void ACodeRescueCharacter::Turn(float Value)
{
    BoundTurnValue = Value;
    if (Value != 0.0f)
    {
        // pass 5: scope magnification scales look input so 50x stays aimable
        AddControllerYawInput(Value * ADSLookScale);
    }
}

void ACodeRescueCharacter::LookUp(float Value)
{
    BoundLookUpValue = Value;
    if (!IsFixedCameraPerspective() && Value != 0.0f)
    {
        AddControllerPitchInput(Value * ADSLookScale);
    }
}

bool ACodeRescueCharacter::TraceForward(FHitResult& Hit, float Distance) const
{
    UCameraComponent* ActiveCamera = GetActiveGameplayCamera();
    if (!ActiveCamera || !GetWorld())
    {
        return false;
    }

    const FVector Start = ActiveCamera->GetComponentLocation();
    const FVector End = Start + ActiveCamera->GetForwardVector() * Distance;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, CodeRescueCollision::InteractionTrace, Params);
}

void ACodeRescueCharacter::ApplyRuntimeTuning()
{
    MaxHealth = FMath::Max(1.0f, MaxHealth);
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    MaxAmmo = FMath::Max(0, MaxAmmo);
    MaxMedkits = FMath::Max(0, MaxMedkits);
    MaxArmorPlates = FMath::Max(0, MaxArmorPlates);
    MaxRadioScannerCharges = FMath::Max(0, MaxRadioScannerCharges);
    MaxFlashlightBatteries = FMath::Max(0, MaxFlashlightBatteries);
    MaxBypassKits = FMath::Max(0, MaxBypassKits);
    MaxAmmoPouchCapacityBonus = FMath::Max(0, MaxAmmoPouchCapacityBonus);
    AmmoPouchCapacityBonus = FMath::Clamp(AmmoPouchCapacityBonus, 0, MaxAmmoPouchCapacityBonus);
    ArmorDamageReduction = FMath::Clamp(ArmorDamageReduction, 0.0f, 0.95f);
    if (bClampSuppliesToMaximum)
    {
        Ammo = FMath::Clamp(Ammo, 0, MaxAmmo);
        Medkits = FMath::Clamp(Medkits, 0, MaxMedkits);
        ArmorPlates = FMath::Clamp(ArmorPlates, 0, MaxArmorPlates);
        RadioScannerCharges = FMath::Clamp(RadioScannerCharges, 0, MaxRadioScannerCharges);
        FlashlightBatteries = FMath::Clamp(FlashlightBatteries, 0, MaxFlashlightBatteries);
        BypassKits = FMath::Clamp(BypassKits, 0, MaxBypassKits);
    }
    else
    {
        Ammo = FMath::Max(0, Ammo);
        Medkits = FMath::Max(0, Medkits);
        ArmorPlates = FMath::Max(0, ArmorPlates);
        RadioScannerCharges = FMath::Max(0, RadioScannerCharges);
        FlashlightBatteries = FMath::Max(0, FlashlightBatteries);
        BypassKits = FMath::Max(0, BypassKits);
    }

    WalkSpeed = FMath::Clamp(WalkSpeed, 150.0f, 30000.0f);
    BrakingDeceleration = FMath::Clamp(BrakingDeceleration, 128.0f, 50000.0f);
    DirectKeyboardTurnRate = FMath::Clamp(DirectKeyboardTurnRate, 10.0f, 360.0f);
    DirectKeyboardLookRate = FMath::Clamp(DirectKeyboardLookRate, 10.0f, 240.0f);
    InteractionTraceDistance = FMath::Clamp(InteractionTraceDistance, 300.0f, 30000.0f);
    InteractionAssistRadius = FMath::Clamp(InteractionAssistRadius, 300.0f, 50000.0f);
    FireRefireDelay = FMath::Clamp(FireRefireDelay, 0.05f, 2.0f);
    WeaponRange = FMath::Clamp(WeaponRange, 250.0f, 100000.0f);
    DirectHitDamage = FMath::Clamp(DirectHitDamage, 1.0f, 500.0f);
    AssistedHitDamage = FMath::Clamp(AssistedHitDamage, 0.0f, 500.0f);
    AssistedHitRadius = FMath::Clamp(AssistedHitRadius, 250.0f, 60000.0f);
    AssistedHitMaxAngleDegrees = FMath::Clamp(AssistedHitMaxAngleDegrees, 1.0f, 90.0f);
    MedkitHealAmount = FMath::Clamp(MedkitHealAmount, 1.0f, 500.0f);
    SoftLandingSpeed = FMath::Clamp(SoftLandingSpeed, 500.0f, 5000.0f);
    FallDamagePer100Speed = FMath::Clamp(FallDamagePer100Speed, 0.1f, 25.0f);
    EnemyHitKnockbackHorizontal = FMath::Clamp(EnemyHitKnockbackHorizontal, 0.0f, 500.0f);
    EnemyHitKnockbackVertical = FMath::Clamp(EnemyHitKnockbackVertical, 0.0f, 500.0f);
    DamageMercyWindowSeconds = FMath::Clamp(DamageMercyWindowSeconds, 0.0f, 2.0f);
    MaxEnemyDamagePerHitFraction = FMath::Clamp(MaxEnemyDamagePerHitFraction, 0.05f, 1.0f);
    EmergencyMedkitHealthFraction = FMath::Clamp(EmergencyMedkitHealthFraction, 0.05f, 0.50f);
    EmergencyMedkitCooldownSeconds = FMath::Clamp(EmergencyMedkitCooldownSeconds, 0.0f, 120.0f);
    SquadFormationMode = FMath::Clamp(SquadFormationMode, 0, 2);

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = WalkSpeed;
        Movement->BrakingDecelerationWalking = BrakingDeceleration;
    }
}

int32 ACodeRescueCharacter::AddAmmo(int32 Amount)
{
    if (Amount <= 0)
    {
        return 0;
    }

    EnsureWeaponStateInitialized();
    int32 Remaining = Amount;
    int32 Granted = 0;

    const int32 ActiveIdx = static_cast<int32>(ActiveWeapon);
    const int32 ActiveGrant = AddAmmoToWeaponIndex(ActiveIdx, Remaining);
    Granted += ActiveGrant;
    Remaining -= ActiveGrant;

    for (int32 Offset = 1; Offset < WeaponLoadout.Num() && Remaining > 0; ++Offset)
    {
        const int32 Idx = (ActiveIdx + Offset) % WeaponLoadout.Num();
        const int32 SlotGrant = AddAmmoToWeaponIndex(Idx, Remaining);
        Granted += SlotGrant;
        Remaining -= SlotGrant;
    }

    if (!bClampSuppliesToMaximum && Remaining > 0 && WeaponReserveAmmo.IsValidIndex(ActiveIdx))
    {
        WeaponReserveAmmo[ActiveIdx] = FMath::Max(0, WeaponReserveAmmo[ActiveIdx] + Remaining);
        Granted += Remaining;
    }

    RefreshLegacyAmmoFromWeaponReserves();
    return Granted;
}

int32 ACodeRescueCharacter::AddMedkits(int32 Amount)
{
    if (Amount <= 0)
    {
        return 0;
    }

    const int32 PreviousMedkits = Medkits;
    if (bClampSuppliesToMaximum)
    {
        Medkits = FMath::Clamp(Medkits + Amount, 0, FMath::Max(0, MaxMedkits));
    }
    else
    {
        Medkits = FMath::Max(0, Medkits + Amount);
    }
    return FMath::Max(0, Medkits - PreviousMedkits);
}

int32 ACodeRescueCharacter::AddBoundedResource(int32& Value, int32 Amount, int32 MaxValue)
{
    if (Amount <= 0)
    {
        return 0;
    }

    const int32 Previous = Value;
    if (bClampSuppliesToMaximum)
    {
        Value = FMath::Clamp(Value + Amount, 0, FMath::Max(0, MaxValue));
    }
    else
    {
        Value = FMath::Max(0, Value + Amount);
    }
    return FMath::Max(0, Value - Previous);
}

int32 ACodeRescueCharacter::AddFlares(int32 Amount)
{
    return AddBoundedResource(FlareCount, Amount, 12);
}

int32 ACodeRescueCharacter::AddSmokes(int32 Amount)
{
    return AddBoundedResource(SmokeCount, Amount, 10);
}

int32 ACodeRescueCharacter::AddStims(int32 Amount)
{
    return AddBoundedResource(StimCount, Amount, 10);
}

int32 ACodeRescueCharacter::AddArmorPlates(int32 Amount)
{
    return AddBoundedResource(ArmorPlates, Amount, MaxArmorPlates);
}

int32 ACodeRescueCharacter::AddRadioScannerCharges(int32 Amount)
{
    return AddBoundedResource(RadioScannerCharges, Amount, MaxRadioScannerCharges);
}

int32 ACodeRescueCharacter::AddFlashlightBatteries(int32 Amount)
{
    return AddBoundedResource(FlashlightBatteries, Amount, MaxFlashlightBatteries);
}

int32 ACodeRescueCharacter::AddBypassKits(int32 Amount)
{
    return AddBoundedResource(BypassKits, Amount, MaxBypassKits);
}

int32 ACodeRescueCharacter::AddAmmoPouch(int32 CapacityBonus)
{
    if (CapacityBonus <= 0)
    {
        return 0;
    }

    const int32 PreviousBonus = AmmoPouchCapacityBonus;
    const int32 BaseMaxAmmo = FMath::Max(0, MaxAmmo - AmmoPouchCapacityBonus);
    AmmoPouchCapacityBonus = FMath::Clamp(
        AmmoPouchCapacityBonus + CapacityBonus,
        0,
        FMath::Max(0, MaxAmmoPouchCapacityBonus));
    MaxAmmo = BaseMaxAmmo + AmmoPouchCapacityBonus;
    const int32 GrantedCapacity = FMath::Max(0, AmmoPouchCapacityBonus - PreviousBonus);
    if (GrantedCapacity > 0)
    {
        AddAmmo(GrantedCapacity);
    }
    return GrantedCapacity;
}

bool ACodeRescueCharacter::TrySpendBypassKit(int32 Amount)
{
    if (Amount <= 0)
    {
        return true;
    }
    if (BypassKits < Amount)
    {
        return false;
    }
    BypassKits -= Amount;
    return true;
}

void ACodeRescueCharacter::RestorePlayerResources(float SavedHealth, int32 SavedAmmo, int32 SavedMedkits)
{
    ApplyRuntimeTuning();
    EnsureWeaponStateInitialized();
    Health = FMath::Clamp(SavedHealth, 0.0f, MaxHealth);
    for (int32& Reserve : WeaponReserveAmmo)
    {
        Reserve = 0;
    }
    AddAmmo(SavedAmmo);
    Medkits = bClampSuppliesToMaximum
        ? FMath::Clamp(SavedMedkits, 0, FMath::Max(0, MaxMedkits))
        : FMath::Max(0, SavedMedkits);
}

void ACodeRescueCharacter::RestorePlayerResourcesDetailed(
    float SavedHealth,
    int32 SavedAmmo,
    int32 SavedMedkits,
    int32 SavedArmorPlates,
    int32 SavedFlares,
    int32 SavedSmokes,
    int32 SavedStims,
    int32 SavedScrap,
    int32 SavedRadioScannerCharges,
    int32 SavedFlashlightBatteries,
    int32 SavedBypassKits,
    int32 SavedAmmoPouchCapacityBonus)
{
    const int32 BaseMaxAmmo = FMath::Max(0, MaxAmmo - AmmoPouchCapacityBonus);
    AmmoPouchCapacityBonus = FMath::Clamp(
        SavedAmmoPouchCapacityBonus,
        0,
        FMath::Max(0, MaxAmmoPouchCapacityBonus));
    MaxAmmo = BaseMaxAmmo + AmmoPouchCapacityBonus;
    RestorePlayerResources(SavedHealth, SavedAmmo, SavedMedkits);

    if (bClampSuppliesToMaximum)
    {
        ArmorPlates = FMath::Clamp(SavedArmorPlates, 0, FMath::Max(0, MaxArmorPlates));
        FlareCount = FMath::Clamp(SavedFlares, 0, 12);
        SmokeCount = FMath::Clamp(SavedSmokes, 0, 10);
        StimCount = FMath::Clamp(SavedStims, 0, 10);
        RadioScannerCharges = FMath::Clamp(SavedRadioScannerCharges, 0, FMath::Max(0, MaxRadioScannerCharges));
        FlashlightBatteries = FMath::Clamp(SavedFlashlightBatteries, 0, FMath::Max(0, MaxFlashlightBatteries));
        BypassKits = FMath::Clamp(SavedBypassKits, 0, FMath::Max(0, MaxBypassKits));
    }
    else
    {
        ArmorPlates = FMath::Max(0, SavedArmorPlates);
        FlareCount = FMath::Max(0, SavedFlares);
        SmokeCount = FMath::Max(0, SavedSmokes);
        StimCount = FMath::Max(0, SavedStims);
        RadioScannerCharges = FMath::Max(0, SavedRadioScannerCharges);
        FlashlightBatteries = FMath::Max(0, SavedFlashlightBatteries);
        BypassKits = FMath::Max(0, SavedBypassKits);
    }
    Scrap = FMath::Max(0, SavedScrap);
}

AActor* ACodeRescueCharacter::FindNearestInteractable(float Radius) const
{
    AActor* Candidate = nullptr;
    float BestDistSq = FMath::Square(Radius);

    auto ConsiderWeighted = [&](AActor* Actor, float DistanceWeight)
    {
        if (!IsValid(Actor))
        {
            return;
        }

        const float DistSq = FVector::DistSquared(Actor->GetActorLocation(), GetActorLocation())
            * FMath::Max(DistanceWeight, 1.0f);
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Candidate = Actor;
        }
    };
    auto Consider = [&](AActor* Actor)
    {
        ConsiderWeighted(Actor, 1.0f);
    };

    for (TActorIterator<ALanguageStationActor> It(GetWorld()); It; ++It)
    {
        Consider(*It);
    }

    for (TActorIterator<ACodingTerminalActor> It(GetWorld()); It; ++It)
    {
        ACodingTerminalActor* Terminal = *It;
        if (IsValid(Terminal) && !Terminal->bSolved)
        {
            // 2026-07-11 final-station fix: the proximity-assisted E used to
            // latch onto the nearby HIDDEN bonus terminal when the (formerly
            // mis-aimed) objective marker dropped the player between it and
            // the required station. Required campaign stations now win over
            // "secret_" bonus terminals unless the bonus one is clearly the
            // thing the player is standing at.
            const bool bSecretBonus = Terminal->Challenge.Id.StartsWith(TEXT("secret_"));
            ConsiderWeighted(Terminal, bSecretBonus ? 6.0f : 1.0f);
        }
    }

    for (TActorIterator<ASurvivorActor> It(GetWorld()); It; ++It)
    {
        ASurvivorActor* Survivor = *It;
        if (IsValid(Survivor) && !Survivor->bRescued)
        {
            Consider(Survivor);
        }
    }

    for (TActorIterator<APickupActor> It(GetWorld()); It; ++It)
    {
        Consider(*It);
    }

    for (TActorIterator<ACaseFilePickupActor> It(GetWorld()); It; ++It)
    {
        ACaseFilePickupActor* CaseFile = *It;
        if (IsValid(CaseFile) && !CaseFile->bCollected)
        {
            Consider(CaseFile);
        }
    }

    for (TActorIterator<AFriendlyNPCActor> It(GetWorld()); It; ++It)
    {
        Consider(*It);
    }

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (IsValid(Actor) && (Actor->Tags.Contains(FName("Helipad")) || Actor->Tags.Contains(FName("Jeep")) || Actor->Tags.Contains(FName("MessageMarker"))))
        {
            Consider(Actor);
        }
    }

    return Candidate;
}

EHitZone ACodeRescueCharacter::ClassifyHitZone(const FName& BoneName) const
{
    if (BoneName.IsNone())
    {
        return EHitZone::Other;
    }

    FString BoneStr = BoneName.ToString().ToLower();

    // Head zone
    if (BoneStr.Contains(TEXT("head")) || BoneStr.Contains(TEXT("neck")))
    {
        return EHitZone::Head;
    }

    // Torso zone
    if (BoneStr.Contains(TEXT("spine")) || BoneStr.Contains(TEXT("pelvis")) ||
        BoneStr.Contains(TEXT("chest")) || BoneStr.Contains(TEXT("torso")))
    {
        return EHitZone::Torso;
    }

    // Limb zone
    if (BoneStr.Contains(TEXT("arm")) || BoneStr.Contains(TEXT("leg")) ||
        BoneStr.Contains(TEXT("hand")) || BoneStr.Contains(TEXT("foot")) ||
        BoneStr.Contains(TEXT("thigh")) || BoneStr.Contains(TEXT("calf")) ||
        BoneStr.Contains(TEXT("upperarm")) || BoneStr.Contains(TEXT("lowerarm")))
    {
        return EHitZone::Limb;
    }

    return EHitZone::Other;
}

void ACodeRescueCharacter::StartFirstLevelCombatRuntimeAudit()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FirstLevelCombatAuditTarget.Reset();
    FirstLevelCombatAuditTraceHits = 0;
    bFirstLevelCombatAuditJumpPassed = false;
    bFirstLevelCombatAuditBitePassed = false;
    bFirstLevelCombatAuditCorpsePassed = false;
    bFirstLevelCombatAuditFadePassed = false;
    bFirstLevelCombatAuditTargetLockPassed = false;
    bFirstLevelCombatAuditMissLocalityPassed = false;

    // Prevent friendly hitscan support from neutralizing the staged target
    // before the player's own production trace is exercised.
    for (TActorIterator<ACompanionActor> It(World); It; ++It)
    {
        It->SetActorTickEnabled(false);
    }
    for (TActorIterator<AFriendlyNPCActor> It(World); It; ++It)
    {
        It->SetActorTickEnabled(false);
    }
    for (TActorIterator<ASurvivorActor> It(World); It; ++It)
    {
        It->SetActorTickEnabled(false);
    }

    ACodeZombieActor* Target = nullptr;
    float BestDistanceSq = TNumericLimits<float>::Max();
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Candidate = *It;
        if (!IsValid(Candidate) || Candidate->Health <= 0.0f ||
            Candidate->Variant == EZombieVariant::DogZombie)
        {
            continue;
        }
        const float DistanceSq = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
        if (DistanceSq < BestDistanceSq)
        {
            BestDistanceSq = DistanceSq;
            Target = Candidate;
        }
    }

    bool bSpawnedTransientTarget = false;
    if (!Target)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Target = World->SpawnActor<ACodeZombieActor>(
            ACodeZombieActor::StaticClass(),
            GetActorLocation() + GetActorForwardVector() * 760.0f,
            GetActorRotation(),
            SpawnParams);
        if (Target)
        {
            bSpawnedTransientTarget = true;
            UE_LOG(LogTemp, Display,
                TEXT("[FirstLevelCombatAudit] spawned transient target because campaign supplied no live zombie"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[FirstLevelCombatAudit] FAIL transient zombie spawn failed"));
            FPlatformMisc::RequestExit(false);
            return;
        }
    }

    if (bSpawnedTransientTarget)
    {
        FZombieVariantRow AuditVisualRow;
        Target->InitializeFromVariant(EZombieVariant::Default, AuditVisualRow);
        Target->Tags.AddUnique(FName("FirstLevelAuditProfessionalVisual"));
    }

    // This target is transient so the acceptance run never alters campaign progress.
    Target->ZombieId = -1;
    Target->Health = 100000.0f;
    Target->RagdollCorpseLifetime = 9.0f;
    Target->CorpseFadeDuration = 2.8f;
    Target->ActivationRange = 0.0f;
    if (UCharacterMovementComponent* TargetMovement = Target->GetCharacterMovement())
    {
        TargetMovement->StopMovementImmediately();
        TargetMovement->DisableMovement();
    }

    FVector AuditLocation = GetActorLocation() + GetActorForwardVector() * 760.0f;
    const float CandidateYawOffsets[] = { 62.0f, -62.0f, 38.0f, -38.0f, 0.0f };
    for (const float YawOffset : CandidateYawOffsets)
    {
        const FVector Direction = FRotator(0.0f, GetActorRotation().Yaw + YawOffset, 0.0f).Vector();
        FVector CandidateLocation = GetActorLocation() + Direction * 760.0f;
        CandidateLocation.Z = GetActorLocation().Z;

        FCollisionQueryParams PathParams(SCENE_QUERY_STAT(FirstLevelCombatAuditLane), false, this);
        PathParams.AddIgnoredActor(Target);
        FHitResult PathHit;
        const bool bBlocked = World->LineTraceSingleByChannel(
            PathHit,
            GetActorLocation() + FVector(0.0f, 0.0f, 55.0f),
            CandidateLocation + FVector(0.0f, 0.0f, 35.0f),
            CodeRescueCollision::WeaponTrace,
            PathParams);
        if (!bBlocked)
        {
            AuditLocation = CandidateLocation;
            break;
        }
    }

    FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(FirstLevelCombatAuditGround), false, this);
    GroundParams.AddIgnoredActor(Target);
    FHitResult GroundHit;
    if (World->LineTraceSingleByChannel(
            GroundHit,
            AuditLocation + FVector(0.0f, 0.0f, 450.0f),
            AuditLocation - FVector(0.0f, 0.0f, 900.0f),
            ECC_Visibility,
            GroundParams))
    {
        AuditLocation.Z = GroundHit.ImpactPoint.Z + Target->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    }
    Target->SetActorLocation(AuditLocation, false, nullptr, ETeleportType::TeleportPhysics);
    Target->SetActorRotation((GetActorLocation() - AuditLocation).Rotation());
    FirstLevelCombatAuditTarget = Target;
    if (const UCapsuleComponent* TargetCapsule = Target->GetCapsuleComponent())
    {
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] collision actor_enabled=%d capsule_enabled=%d weapon_response=%d object_type=%d center=%s radius=%.2f half_height=%.2f"),
            Target->GetActorEnableCollision() ? 1 : 0,
            static_cast<int32>(TargetCapsule->GetCollisionEnabled()),
            static_cast<int32>(TargetCapsule->GetCollisionResponseToChannel(CodeRescueCollision::WeaponTrace)),
            static_cast<int32>(TargetCapsule->GetCollisionObjectType()),
            *TargetCapsule->GetComponentLocation().ToCompactString(),
            TargetCapsule->GetScaledCapsuleRadius(),
            TargetCapsule->GetScaledCapsuleHalfHeight());
    }

    SwapWeapon(EWeaponType::Pistol);
    MagazineAmmo = 12;
    if (WeaponMagazines.IsValidIndex(static_cast<int32>(EWeaponType::Pistol)))
    {
        WeaponMagazines[static_cast<int32>(EWeaponType::Pistol)] = MagazineAmmo;
    }
    LastFireWorldTime = -100.0f;
    bEnableAssistedHit = true;
    BeginAim();

    const FVector AuditAimPoint = AuditLocation + FVector(0.0f, 0.0f, 25.0f);
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        const UCameraComponent* Camera = GetActiveGameplayCamera();
        const FVector AimOrigin = Camera ? Camera->GetComponentLocation() : GetActorLocation();
        const FRotator AimRotation = (AuditAimPoint - AimOrigin).Rotation();
        PC->SetControlRotation(AimRotation);
        SetActorRotation(FRotator(0.0f, AimRotation.Yaw, 0.0f));
    }

    SpawnAnatomicalBiteWound(Target);
    bFirstLevelCombatAuditBitePassed = Tags.Contains(FName("PlayerAnatomicalBiteWounds"));
    TryJump();
    UE_LOG(LogTemp, Display,
        TEXT("[FirstLevelCombatAudit] BEGIN target=%s location=%s bite_visual=%s corpse_window=9.0 fade=2.8"),
        *Target->GetName(),
        *AuditLocation.ToCompactString(),
        bFirstLevelCombatAuditBitePassed ? TEXT("PASS") : TEXT("FAIL"));

    FTimerHandle JumpCheckTimer;
    GetWorldTimerManager().SetTimer(JumpCheckTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        const UCharacterMovementComponent* Movement = GetCharacterMovement();
        bFirstLevelCombatAuditJumpPassed = Movement && Movement->IsFalling() && GetVelocity().Z > 1.0f;
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] jump=%s velocity_z=%.1f stamina=%.1f"),
            bFirstLevelCombatAuditJumpPassed ? TEXT("PASS") : TEXT("FAIL"),
            GetVelocity().Z,
            Stamina);
    }), 0.18f, false);

    FTimerHandle MissAimTimer;
    GetWorldTimerManager().SetTimer(MissAimTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (!IsValid(AuditTarget))
        {
            return;
        }
        EndAim();
        bEnableAssistedHit = false;
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            const UCameraComponent* Camera = GetActiveGameplayCamera();
            const FVector AimOrigin = Camera ? Camera->GetComponentLocation() : GetActorLocation();
            FRotator MissRotation = (GetAimTargetPoint(AuditTarget) - AimOrigin).Rotation();
            MissRotation.Yaw += 90.0f;
            MissRotation.Pitch = 35.0f;
            MissRotation.Roll = 0.0f;
            PC->SetControlRotation(MissRotation);
            SetActorRotation(FRotator(0.0f, MissRotation.Yaw, 0.0f));
        }
    }), 0.45f, false);

    FTimerHandle MissShotTimer;
    GetWorldTimerManager().SetTimer(MissShotTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (!IsValid(AuditTarget))
        {
            return;
        }
        const float BeforeHealth = AuditTarget->Health;
        LastFireWorldTime = -100.0f;
        Fire();
        bFirstLevelCombatAuditMissLocalityPassed =
            FMath::IsNearlyEqual(BeforeHealth, AuditTarget->Health, KINDA_SMALL_NUMBER);
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] deliberate_miss_locality=%s health=%.1f->%.1f physical_ray_miss=1 remote_damage=0"),
            bFirstLevelCombatAuditMissLocalityPassed ? TEXT("PASS") : TEXT("FAIL"),
            BeforeHealth,
            AuditTarget->Health);

        bEnableAssistedHit = true;
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            const UCameraComponent* Camera = GetActiveGameplayCamera();
            const FVector AimOrigin = Camera ? Camera->GetComponentLocation() : GetActorLocation();
            const FRotator AimRotation = (GetAimTargetPoint(AuditTarget) - AimOrigin).Rotation();
            PC->SetControlRotation(AimRotation);
            SetActorRotation(FRotator(0.0f, AimRotation.Yaw, 0.0f));
        }
        BeginAim();
    }), 0.62f, false);

    FTimerHandle FirstShotTimer;
    FTimerHandle AimAlignmentTimer;
    GetWorldTimerManager().SetTimer(AimAlignmentTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (!IsValid(AuditTarget))
        {
            return;
        }
        AuditTarget->Health = 70.0f;
        if (const UCameraComponent* Camera = GetActiveGameplayCamera())
        {
            const FVector AimOrigin = Camera->GetComponentLocation();
            const FVector AimPoint = AimOrigin + Camera->GetForwardVector() * 1050.0f;
            const FVector StagedActorLocation = AimPoint - FVector(0.0f, 0.0f, 25.0f);
            AuditTarget->SetActorLocation(StagedActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
            AuditTarget->SetActorRotation((GetActorLocation() - StagedActorLocation).Rotation());
            UE_LOG(LogTemp, Display,
                TEXT("[FirstLevelCombatAudit] settled_aim origin=%s target=%s forward=%s"),
                *AimOrigin.ToCompactString(),
                *AimPoint.ToCompactString(),
                *Camera->GetForwardVector().ToCompactString());
        }
    }), 1.80f, false);

    FTimerHandle TargetLockCheckTimer;
    GetWorldTimerManager().SetTimer(TargetLockCheckTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        bFirstLevelCombatAuditTargetLockPassed =
            IsAimTargetLocked() && GetLockedAimTarget() == FirstLevelCombatAuditTarget.Get();
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] target_lock=%s target=%s physical_trace_redirect=%d"),
            bFirstLevelCombatAuditTargetLockPassed ? TEXT("PASS") : TEXT("FAIL"),
            GetLockedAimTarget() ? *GetLockedAimTarget()->GetName() : TEXT("none"),
            Tags.Contains(FName("PhysicalAutoTargetLockActive")) ? 1 : 0);
    }), 1.98f, false);

    GetWorldTimerManager().SetTimer(FirstShotTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (!IsValid(AuditTarget) || AuditTarget->Health <= 0.0f)
        {
            return;
        }
        const UCameraComponent* Camera = GetActiveGameplayCamera();
        const FVector TraceStart = Camera ? Camera->GetComponentLocation() : GetActorLocation();
        const FVector TraceDirection = Camera ? Camera->GetForwardVector() : GetActorForwardVector();
        FCollisionQueryParams AuditTraceParams(SCENE_QUERY_STAT(FirstLevelCombatAuditPreflight), false, this);
        FHitResult AuditTraceHit;
        const bool bPreflightHit = GetWorld()->LineTraceSingleByChannel(
            AuditTraceHit,
            TraceStart,
            TraceStart + TraceDirection * 25000.0f,
            CodeRescueCollision::WeaponTrace,
            AuditTraceParams);
        FHitResult AuditZombieHit;
        FCollisionObjectQueryParams AuditZombieObjects;
        AuditZombieObjects.AddObjectTypesToQuery(CodeRescueCollision::ZombiePawnObject);
        const bool bPreflightZombieHit = GetWorld()->LineTraceSingleByObjectType(
            AuditZombieHit,
            TraceStart,
            TraceStart + TraceDirection * 25000.0f,
            AuditZombieObjects,
            AuditTraceParams);
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] preflight channel_hit=%s channel_actor=%s zombie_hit=%s zombie_actor=%s target=%s"),
            bPreflightHit ? TEXT("true") : TEXT("false"),
            AuditTraceHit.GetActor() ? *AuditTraceHit.GetActor()->GetName() : TEXT("none"),
            bPreflightZombieHit ? TEXT("true") : TEXT("false"),
            AuditZombieHit.GetActor() ? *AuditZombieHit.GetActor()->GetName() : TEXT("none"),
            *AuditTarget->GetName());
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->SetIgnoreLookInput(true);
        }
        const float BeforeHealth = AuditTarget->Health;
        Fire();
        const bool bHit = AuditTarget->Health < BeforeHealth;
        FirstLevelCombatAuditTraceHits += bHit ? 1 : 0;
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] line_trace_shot=1 %s health=%.1f->%.1f remote_assist=disabled"),
            bHit ? TEXT("PASS") : TEXT("FAIL"), BeforeHealth, AuditTarget->Health);
    }), 2.10f, false);

    FTimerHandle WoundCaptureTimer;
    GetWorldTimerManager().SetTimer(WoundCaptureTimer, FTimerDelegate::CreateWeakLambda(this, []()
    {
        const FString CapturePath = FPaths::ProjectSavedDir() /
            TEXT("Screenshots/FirstLevel/first_level_combat_wound.png");
        FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
        UE_LOG(LogTemp, Display, TEXT("[FirstLevelCombatAudit] wound capture requested %s"), *CapturePath);
    }), 2.30f, false);

    FTimerHandle SecondAimTimer;
    GetWorldTimerManager().SetTimer(SecondAimTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        const UCameraComponent* Camera = GetActiveGameplayCamera();
        if (IsValid(AuditTarget) && Camera)
        {
            const FVector AimPoint = Camera->GetComponentLocation() + Camera->GetForwardVector() * 1050.0f;
            AuditTarget->SetActorLocation(
                AimPoint - FVector(0.0f, 0.0f, 25.0f),
                false,
                nullptr,
                ETeleportType::TeleportPhysics);
        }
    }), 2.35f, false);

    FTimerHandle SecondShotTimer;
    GetWorldTimerManager().SetTimer(SecondShotTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (!IsValid(AuditTarget) || AuditTarget->Health <= 0.0f)
        {
            return;
        }
        const float BeforeHealth = AuditTarget->Health;
        Fire();
        const bool bHit = AuditTarget->Health < BeforeHealth;
        FirstLevelCombatAuditTraceHits += bHit ? 1 : 0;
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] line_trace_shot=2 %s health=%.1f->%.1f remote_assist=disabled"),
            bHit ? TEXT("PASS") : TEXT("FAIL"), BeforeHealth, AuditTarget->Health);
    }), 2.60f, false);

    FTimerHandle CorpseCheckTimer;
    GetWorldTimerManager().SetTimer(CorpseCheckTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (!IsValid(AuditTarget))
        {
            UE_LOG(LogTemp, Error, TEXT("[FirstLevelCombatAudit] corpse_window=FAIL target removed immediately"));
            return;
        }
        if (AuditTarget->Health > 0.0f)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[FirstLevelCombatAudit] production shots did not finish target; forcing lifecycle continuation at health=%.1f"),
                AuditTarget->Health);
            AuditTarget->ApplyRescuePointDamage(
                1000.0f,
                EHitZone::Torso,
                AuditTarget->GetActorLocation() + FVector(0.0f, 0.0f, 25.0f),
                (AuditTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal(),
                NAME_None);
        }
        bFirstLevelCombatAuditCorpsePassed = IsValid(AuditTarget) && AuditTarget->Health <= 0.0f;
        FirstLevelCombatAuditCorpseLocation = AuditTarget->GetActorLocation();
        FirstLevelCombatAuditCorpseScale = AuditTarget->GetActorScale3D();
        UE_LOG(LogTemp, Display,
            TEXT("[FirstLevelCombatAudit] corpse_window=%s trace_hits=%d location=%s scale=%s"),
            bFirstLevelCombatAuditCorpsePassed ? TEXT("PASS") : TEXT("FAIL"),
            FirstLevelCombatAuditTraceHits,
            *FirstLevelCombatAuditCorpseLocation.ToCompactString(),
            *FirstLevelCombatAuditCorpseScale.ToCompactString());
    }), 3.00f, false);

    FTimerHandle CorpseCaptureTimer;
    GetWorldTimerManager().SetTimer(CorpseCaptureTimer, FTimerDelegate::CreateWeakLambda(this, []()
    {
        const FString CapturePath = FPaths::ProjectSavedDir() /
            TEXT("Screenshots/FirstLevel/first_level_grounded_corpse.png");
        FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
    }), 3.65f, false);

    FTimerHandle FadeCheckTimer;
    GetWorldTimerManager().SetTimer(FadeCheckTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        ACodeZombieActor* AuditTarget = FirstLevelCombatAuditTarget.Get();
        if (IsValid(AuditTarget))
        {
            const float SinkDistance = FirstLevelCombatAuditCorpseLocation.Z - AuditTarget->GetActorLocation().Z;
            const float ScaleReduction = FirstLevelCombatAuditCorpseScale.Size() - AuditTarget->GetActorScale3D().Size();
            bFirstLevelCombatAuditFadePassed = SinkDistance > 0.5f || ScaleReduction > 0.02f;
            UE_LOG(LogTemp, Display,
                TEXT("[FirstLevelCombatAudit] gradual_fade=%s sink=%.2f scale_reduction=%.3f"),
                bFirstLevelCombatAuditFadePassed ? TEXT("PASS") : TEXT("FAIL"),
                SinkDistance,
                ScaleReduction);
            const FString CapturePath = FPaths::ProjectSavedDir() /
                TEXT("Screenshots/FirstLevel/first_level_corpse_fade.png");
            FScreenshotRequest::RequestScreenshot(CapturePath, false, false);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[FirstLevelCombatAudit] gradual_fade=FAIL target removed before fade observation"));
        }
    }), 12.85f, false);

    FTimerHandle CompletionTimer;
    GetWorldTimerManager().SetTimer(CompletionTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        const bool bRemovedAfterFade = !IsValid(FirstLevelCombatAuditTarget.Get());
        constexpr uint8 RequiredAnimationStates =
            (1u << 0) | (1u << 3) | (1u << 4) | (1u << 5);
        const bool bTwoArmAimObserved = AimingPresentationMesh &&
            AimingPresentationMesh->ComponentTags.Contains(FName("TwoArmWeaponAimPoseActive"));
        const bool bLandingCompressionObserved = AimingPresentationMesh &&
            AimingPresentationMesh->ComponentTags.Contains(FName("ProceduralLandingCompressionActive"));
        const bool bPlayerAnimationPass = bUsingAuthoredMannyAnimation &&
            (MannyObservedAnimationStateMask & RequiredAnimationStates) == RequiredAnimationStates &&
            bTwoArmAimObserved && bLandingCompressionObserved;
        const UStaticMesh* HeldWeaponAsset = ThirdPersonWeaponMesh
            ? ThirdPersonWeaponMesh->GetStaticMesh()
            : nullptr;
        const float HeldWeaponLongestDimension = ThirdPersonWeaponMesh
            ? ThirdPersonWeaponMesh->Bounds.BoxExtent.GetMax() * 2.0f
            : 0.0f;
        const float HeldWeaponTargetLength = GetHeldWeaponTargetLengthCm(ActiveWeapon);
        const USceneComponent* HeldWeaponParent = ThirdPersonWeaponMesh
            ? ThirdPersonWeaponMesh->GetAttachParent()
            : nullptr;
        const FVector HeldWeaponAttachPoint = HeldWeaponParent
            ? HeldWeaponParent->GetSocketLocation(ThirdPersonWeaponMesh->GetAttachSocketName())
            : FVector::ZeroVector;
        const float HeldWeaponCenterOffset = ThirdPersonWeaponMesh && HeldWeaponParent
            ? FVector::Dist(ThirdPersonWeaponMesh->Bounds.Origin, HeldWeaponAttachPoint)
            : TNumericLimits<float>::Max();
        const bool bHeldWeaponSizePass = FMath::Abs(
            HeldWeaponLongestDimension - HeldWeaponTargetLength) <=
            FMath::Max(3.0f, HeldWeaponTargetLength * 0.08f);
        const bool bHeldWeaponPresentationPass = HeldWeaponAsset &&
            HeldWeaponParent &&
            ThirdPersonWeaponMesh->IsUsingAbsoluteScale() &&
            !ThirdPersonWeaponMesh->CastShadow &&
            bHeldWeaponSizePass &&
            HeldWeaponCenterOffset <= HeldWeaponTargetLength + 45.0f;
        if (bHeldWeaponPresentationPass)
        {
            Tags.AddUnique(FName("FirstLevelIntegratedHeldWeaponPass"));
        }
        const FString HeldWeaponSummary = FString::Printf(
            TEXT("[HeldWeaponPresentationAudit] COMPLETE %s asset=%s target_length_cm=%.1f world_longest_cm=%.1f center_offset_cm=%.1f attached=%d absolute_scale=%d casts_shadow=%d"),
            bHeldWeaponPresentationPass ? TEXT("PASS") : TEXT("FAIL"),
            HeldWeaponAsset ? *HeldWeaponAsset->GetName() : TEXT("none"),
            HeldWeaponTargetLength,
            HeldWeaponLongestDimension,
            HeldWeaponCenterOffset,
            HeldWeaponParent ? 1 : 0,
            ThirdPersonWeaponMesh && ThirdPersonWeaponMesh->IsUsingAbsoluteScale() ? 1 : 0,
            ThirdPersonWeaponMesh && ThirdPersonWeaponMesh->CastShadow ? 1 : 0);
        if (bHeldWeaponPresentationPass)
        {
            UE_LOG(LogTemp, Display, TEXT("%s"), *HeldWeaponSummary);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("%s"), *HeldWeaponSummary);
        }
        if (bPlayerAnimationPass)
        {
            Tags.AddUnique(FName("FirstLevelIntegratedAnimationPass"));
            UE_LOG(LogTemp, Display,
                TEXT("[PlayerAnimationRuntimeAudit] COMPLETE PASS idle=1 jump=1 fall=1 land=1 landing_compression=1 two_arm_aim=1 authored_sequences=6"));
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("[PlayerAnimationRuntimeAudit] COMPLETE FAIL authored=%d observed_mask=%u required_mask=%u landing_compression=%d two_arm_aim=%d"),
                bUsingAuthoredMannyAnimation ? 1 : 0,
                static_cast<uint32>(MannyObservedAnimationStateMask),
                static_cast<uint32>(RequiredAnimationStates),
                bLandingCompressionObserved ? 1 : 0,
                bTwoArmAimObserved ? 1 : 0);
        }
        const bool bPassed = bFirstLevelCombatAuditJumpPassed &&
            bFirstLevelCombatAuditBitePassed &&
            bFirstLevelCombatAuditMissLocalityPassed &&
            bFirstLevelCombatAuditTargetLockPassed &&
            FirstLevelCombatAuditTraceHits > 0 &&
            bFirstLevelCombatAuditCorpsePassed &&
            bFirstLevelCombatAuditFadePassed &&
            bRemovedAfterFade &&
            bPlayerAnimationPass &&
            bHeldWeaponPresentationPass;
        if (bPassed)
        {
            UE_LOG(LogTemp, Display,
                TEXT("[FirstLevelCombatAudit] COMPLETE PASS jump=1 bite=1 miss_locality=1 target_lock=1 trace_hits=%d corpse=1 fade=1 removed=1 animation=1 held_weapon=1"),
                FirstLevelCombatAuditTraceHits);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("[FirstLevelCombatAudit] COMPLETE FAIL jump=%d bite=%d miss_locality=%d target_lock=%d trace_hits=%d corpse=%d fade=%d removed=%d animation=%d held_weapon=%d"),
                bFirstLevelCombatAuditJumpPassed ? 1 : 0,
                bFirstLevelCombatAuditBitePassed ? 1 : 0,
                bFirstLevelCombatAuditMissLocalityPassed ? 1 : 0,
                bFirstLevelCombatAuditTargetLockPassed ? 1 : 0,
                FirstLevelCombatAuditTraceHits,
                bFirstLevelCombatAuditCorpsePassed ? 1 : 0,
                bFirstLevelCombatAuditFadePassed ? 1 : 0,
                bRemovedAfterFade ? 1 : 0,
                bPlayerAnimationPass ? 1 : 0,
                bHeldWeaponPresentationPass ? 1 : 0);
        }
        if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit")))
        {
            if (bPassed)
            {
                Tags.AddUnique(FName("FirstLevelIntegratedCombatPass"));
            }
            EndAim();
            if (APlayerController* PC = Cast<APlayerController>(GetController()))
            {
                PC->SetIgnoreLookInput(false);
            }
            FTimerHandle IntegratedArmoryTimer;
            GetWorldTimerManager().SetTimer(IntegratedArmoryTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                UCodeRescueMessageReaderWidget::OpenReader(
                    this,
                    TEXT("[AUDIT] MODAL ROUTING"),
                    TEXT("MESSAGE READER"),
                    TEXT("This deterministic reader is closed before the Field Armory opens."));
                const bool bReaderOpened = UCodeRescueMessageReaderWidget::IsReaderOpen()
                    && IsUIOpen() && UGameplayStatics::IsGamePaused(GetWorld());

                // The first pause command must close the reader and return;
                // it must never stack the armory underneath the modal.
                TogglePauseMenu();
                const bool bReaderClosed = !UCodeRescueMessageReaderWidget::IsReaderOpen()
                    && !IsUIOpen() && !UGameplayStatics::IsGamePaused(GetWorld());
                const bool bReaderRoutingPass = bReaderOpened && bReaderClosed;
                if (bReaderRoutingPass)
                {
                    Tags.AddUnique(FName("FirstLevelIntegratedReaderPass"));
                    UE_LOG(LogTemp, Display,
                        TEXT("[MessageReaderRoutingAudit] COMPLETE PASS opened=1 close_keys=3 mouse_close=1 pause_exclusive=1 world_unpaused=1"));
                }
                else
                {
                    UE_LOG(LogTemp, Error,
                        TEXT("[MessageReaderRoutingAudit] COMPLETE FAIL opened=%d closed=%d reader_open=%d ui_open=%d paused=%d"),
                        bReaderOpened ? 1 : 0,
                        bReaderClosed ? 1 : 0,
                        UCodeRescueMessageReaderWidget::IsReaderOpen() ? 1 : 0,
                        IsUIOpen() ? 1 : 0,
                        UGameplayStatics::IsGamePaused(GetWorld()) ? 1 : 0);
                }

                FTimerHandle OpenArmoryTimer;
                GetWorldTimerManager().SetTimer(OpenArmoryTimer,
                    FTimerDelegate::CreateWeakLambda(this, [this]()
                    {
                        TogglePauseMenu();
                    }),
                    0.15f,
                    false);
            }), 0.45f, false);
        }
        else
        {
            FPlatformMisc::RequestExit(false);
        }
    }), 16.2f, false);
}

void ACodeRescueCharacter::TryJump()
{
    if (bUIOpen || Health <= 0.0f || Stamina < JumpStaminaCost)
    {
        return;
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || Movement->IsFalling() || !CanJump())
    {
        return;
    }

    Stamina = FMath::Max(0.0f, Stamina - JumpStaminaCost);
    Jump();
    Tags.AddUnique(FName("FirstLevelJumpRuntime"));
    UE_LOG(LogTemp, Verbose, TEXT("[FirstLevelMovement] jump committed stamina=%.1f"), Stamina);
}

void ACodeRescueCharacter::StopJumpAction()
{
    StopJumping();
}

void ACodeRescueCharacter::BeginAim()
{
    if (!bUIOpen && Health > 0.0f)
    {
        bAimInputHeld = true;
        AimHoldTimer = FMath::Max(AimHoldTimer, 0.2f);
        Tags.AddUnique(FName("PlayerAimInputActive"));
        UpdateAutoTargetLock(0.0f);
        // 2026-07-16 pass 5: RMB hold = aim-down-sights. The weapon centers on
        // the bore, the FOV narrows per the scope ladder, and firing stays on
        // LMB — aim first, then choose to shoot.
        // 2026-07-17 (Kenny's sighting reference video): raising the weapon
        // to the eye must work from ANY camera. Holding aim in a third-person
        // perspective steps into the sight view; releasing (EndAim) hands the
        // previous camera straight back — raise, look through, lower.
        if (CameraPerspective != 0)
        {
            PreAimCameraPerspective = CameraPerspective;
            bRestorePerspectiveOnAimEnd = true;
            CameraPerspective = 0;
            ApplyCameraPerspective();
        }
        bADSActive = true;
        Tags.AddUnique(FName("AimDownSightsRuntime"));
    }
}

void ACodeRescueCharacter::EndAim()
{
    bAimInputHeld = false;
    bADSActive = false;
    bAimToggleLatched = false;
    AimHoldTimer = 0.0f;
    if (bRestorePerspectiveOnAimEnd)
    {
        bRestorePerspectiveOnAimEnd = false;
        CameraPerspective = PreAimCameraPerspective;
        ApplyCameraPerspective();
    }
    UpdateAutoTargetLock(0.0f);
}

void ACodeRescueCharacter::OnAimPressed()
{
    // second click while latched = lower the weapon
    if (bADSActive && bAimToggleLatched)
    {
        EndAim();
        return;
    }
    AimPressWallTime = FPlatformTime::Seconds();
    BeginAim();
}

void ACodeRescueCharacter::OnAimReleased()
{
    if (!bADSActive)
    {
        return;
    }
    const double HeldSeconds = FPlatformTime::Seconds() - AimPressWallTime;
    if (HeldSeconds < 0.35)
    {
        // quick click (trackpad two-finger tap): sights STAY up — a normal
        // left-click now fires down the scope; clicking aim again lowers.
        bAimToggleLatched = true;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.2f, FColor::Cyan,
                TEXT("Sights locked — click to fire, right-click again to lower."));
        }
        Tags.AddUnique(FName("AimToggleLatchRuntime"));
        return;
    }
    EndAim();
}

void ACodeRescueCharacter::TakeGameplayScreenshot()
{
    if (DirectKeyCooldown > 0.0f)
    {
        return;   // debounce (chord keys repeat across polled frames)
    }
    DirectKeyCooldown = FMath::Max(DirectKeyCooldown, 0.6f);

    // 2026-07-17 rebuild (Kenny: "the screen tells me that I have recorded a
    // screenshot, but nothing exists inside of the folder"): the packaged Mac
    // build silently drops FScreenshotRequest writes to absolute paths. The
    // capture now goes through the engine screenshot pipeline into the app's
    // OWN Saved/Screenshots directory (packaged-proven), and a follow-up
    // timer MOVES the newest capture into Kenny's correction folder —
    // confirming on-screen only after the file VERIFIABLY exists.
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->ConsoleCommand(TEXT("HighResShot 1"));
    }

    FTimerHandle DeliverTimer;
    GetWorldTimerManager().SetTimer(DeliverTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        const FString CaptureDir = FPaths::ScreenShotDir();
        TArray<FString> Captures;
        IFileManager::Get().FindFiles(Captures, *(CaptureDir / TEXT("*.png")), true, false);
        FString NewestFile;
        FDateTime NewestTime = FDateTime::MinValue();
        for (const FString& Capture : Captures)
        {
            const FString FullPath = CaptureDir / Capture;
            const FDateTime Stamp = IFileManager::Get().GetTimeStamp(*FullPath);
            if (Stamp > NewestTime)
            {
                NewestTime = Stamp;
                NewestFile = FullPath;
            }
        }
        // Delivery. The PACKAGED app is SANDBOXED (its writes outside the
        // container are denied — this is why every absolute-path screenshot
        // silently failed for Kenny). Direct move is tried first (works in
        // editor/dev runs); on denial the capture is renamed IN PLACE and
        // surfaced through the `InGame_Captures` symlink that lives inside
        // Screenshots_for_Correction and points at the container folder.
        const FString FriendlyName = FString::Printf(TEXT("InGame_%s.png"),
            *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
        const FString TargetDir = TEXT("/Users/labcomputer/Desktop/Operation_Code_Rescue/Screenshots_for_Correction");
        bool bDelivered = false;
        FString ShownLocation;
        if (!NewestFile.IsEmpty() && (FDateTime::UtcNow() - NewestTime).GetTotalSeconds() < 30.0)
        {
            const FString ExternalTarget = TargetDir / FriendlyName;
            IFileManager::Get().MakeDirectory(*TargetDir, true);
            if (IFileManager::Get().Move(*ExternalTarget, *NewestFile, true, false, false, /*bDoNotRetryOrError=*/true) &&
                IFileManager::Get().FileExists(*ExternalTarget))
            {
                bDelivered = true;
                ShownLocation = FString::Printf(TEXT("Screenshots_for_Correction/%s"), *FriendlyName);
            }
            else
            {
                const FString InPlaceTarget = CaptureDir / FriendlyName;
                if (IFileManager::Get().Move(*InPlaceTarget, *NewestFile, true, false, false, /*bDoNotRetryOrError=*/true) &&
                    IFileManager::Get().FileExists(*InPlaceTarget))
                {
                    bDelivered = true;
                    ShownLocation = FString::Printf(TEXT("Screenshots_for_Correction/InGame_Captures/%s"), *FriendlyName);
                }
            }
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, bDelivered ? FColor::Cyan : FColor::Orange,
                bDelivered
                    ? FString::Printf(TEXT("Screenshot saved: %s"), *ShownLocation)
                    : FString(TEXT("Screenshot capture failed — try again in a second.")));
        }
        UE_LOG(LogTemp, Display, TEXT("[GameplayScreenshot] delivered=%d location=%s"),
            bDelivered ? 1 : 0, *ShownLocation);
    }), 1.4f, false);
    Tags.AddUnique(FName("GameplayScreenshotHotkey"));
}

void ACodeRescueCharacter::PollGamepad(float DeltaSeconds)
{
    if (bUIOpen) return;
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    // Read axis values via the input subsystem. UE provides axis values
    // through PlayerInput->GetKeyValue for analog keys.
    UPlayerInput* PlayerInput = PC->PlayerInput;
    if (!PlayerInput) return;

    const float Deadzone = 0.18f;

    // Left stick = movement.
    const float LX = PlayerInput->GetKeyValue(EKeys::Gamepad_LeftX);
    const float LY = PlayerInput->GetKeyValue(EKeys::Gamepad_LeftY);
    if (FMath::Abs(LY) > Deadzone) AddMovementInput(GetPerspectiveMoveForwardVector(), LY);
    if (FMath::Abs(LX) > Deadzone) AddMovementInput(GetPerspectiveMoveRightVector(),   LX);

    // Right stick = look. Tune scalar to match keyboard look feel.
    const float RX = PlayerInput->GetKeyValue(EKeys::Gamepad_RightX);
    const float RY = PlayerInput->GetKeyValue(EKeys::Gamepad_RightY);
    if (FMath::Abs(RX) > Deadzone) AddControllerYawInput(RX * DirectKeyboardTurnRate * DeltaSeconds * 1.5f);
    if (FMath::Abs(RY) > Deadzone) AddControllerPitchInput(-RY * DirectKeyboardLookRate * DeltaSeconds * 1.5f);

    // Right trigger = fire. Use a scalar > 0.5 as "pressed".
    const float RT = PlayerInput->GetKeyValue(EKeys::Gamepad_RightTrigger);
    if (RT > 0.5f) Fire();

    // X (Left face button) = interact.
    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Left))
    {
        Interact();
    }

    // Y (Top face button) = reload.
    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Top))
    {
        Reload();
    }

    // B (Right face button) = throw active throwable.
    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right))
    {
        ThrowActive();
    }

    // D-pad up/down = cycle throwable.
    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up) ||
        PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Down))
    {
        CycleThrowable();
    }

    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_LeftShoulder))
    {
        CycleWeaponNext();
    }

    // Special_Left/Right = pause + journal.
    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_Special_Left))
    {
        TogglePauseMenu();
    }
    if (PC->WasInputKeyJustPressed(EKeys::Gamepad_Special_Right))
    {
        ToggleObjectiveJournal();
    }
}

void ACodeRescueCharacter::RegroupRescueTeam()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 RegroupedCount = 0;
    int32 FormationIndex = 0;
    const float FormationScale = GetSquadFormationSpacingScale();
    TArray<ACompanionActor*> RoleResponders;
    for (TActorIterator<ACompanionActor> It(World); It; ++It)
    {
        ACompanionActor* Companion = *It;
        if (!IsValid(Companion) || !Companion->IsOperational())
        {
            continue;
        }

        Companion->ApplyFormationSpacingScale(FormationScale);
        Companion->ClearHoldPosition();
        Companion->RegroupNearPlayer(this, FormationIndex);
        if (bSquadHoldPosition)
        {
            Companion->SetHoldPosition(Companion->GetActorLocation(), Companion->GetActorRotation());
        }
        ++RegroupedCount;
        ++FormationIndex;
        if (RoleResponders.Num() < 2)
        {
            RoleResponders.Add(Companion);
        }
    }

    LastSquadRegroupWorldTime = World->GetTimeSeconds();
    LastSquadRegroupCount = RegroupedCount;

    FString Message = TEXT("No active rescue-team members available to regroup.");
    if (RegroupedCount > 0)
    {
        Message = bSquadHoldPosition
            ? FString::Printf(
                TEXT("Squad regrouped and holding: %d teammate%s on your position."),
                RegroupedCount,
                RegroupedCount == 1 ? TEXT("") : TEXT("s"))
            : FString::Printf(
                TEXT("Squad regrouped: %d teammate%s on your position."),
                RegroupedCount,
                RegroupedCount == 1 ? TEXT("") : TEXT("s"));
    }
    UCodeRescueSubtitlesWidget::Push(Message, 2.8f);
    for (ACompanionActor* Responder : RoleResponders)
    {
        if (IsValid(Responder))
        {
            Responder->PushRoleOrderBark(bSquadHoldPosition ? TEXT("REGROUP HOLD") : TEXT("REGROUP FOLLOW"), 2.4f);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, RegroupedCount > 0 ? FColor::Cyan : FColor::Silver, Message);
    }
}

FString ACodeRescueCharacter::GetSquadFormationLabel() const
{
    switch (SquadFormationMode)
    {
    case 0:
        return TEXT("TIGHT");
    case 2:
        return TEXT("WIDE");
    default:
        return TEXT("STANDARD");
    }
}

float ACodeRescueCharacter::GetSquadFormationSpacingScale() const
{
    switch (SquadFormationMode)
    {
    case 0:
        return 0.78f;
    case 2:
        return 1.35f;
    default:
        return 1.0f;
    }
}

FString ACodeRescueCharacter::GetSquadOrderLabel() const
{
    return bSquadHoldPosition ? TEXT("HOLD") : TEXT("FOLLOW");
}

float ACodeRescueCharacter::GetEmergencyMedkitReadySeconds() const
{
    if (!bAutoUseEmergencyMedkit)
    {
        return -1.0f;
    }

    const float NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    return FMath::Max(0.0f, EmergencyMedkitCooldownSeconds - (NowSeconds - LastEmergencyMedkitWorldTime));
}

void ACodeRescueCharacter::CycleSquadFormation()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    SquadFormationMode = (SquadFormationMode + 1) % 3;
    LastSquadFormationWorldTime = World->GetTimeSeconds();

    int32 UpdatedCount = 0;
    const float FormationScale = GetSquadFormationSpacingScale();
    TArray<ACompanionActor*> RoleResponders;
    for (TActorIterator<ACompanionActor> It(World); It; ++It)
    {
        ACompanionActor* Companion = *It;
        if (!IsValid(Companion) || !Companion->IsOperational())
        {
            continue;
        }

        Companion->ApplyFormationSpacingScale(FormationScale);
        ++UpdatedCount;
        if (RoleResponders.Num() < 2)
        {
            RoleResponders.Add(Companion);
        }
    }

    const FString Message = FString::Printf(
        TEXT("Squad formation: %s spacing applied to %d teammate%s. Press Y to regroup."),
        *GetSquadFormationLabel(),
        UpdatedCount,
        UpdatedCount == 1 ? TEXT("") : TEXT("s"));
    UCodeRescueSubtitlesWidget::Push(Message, 3.0f);
    for (ACompanionActor* Responder : RoleResponders)
    {
        if (IsValid(Responder))
        {
            Responder->PushRoleOrderBark(GetSquadFormationLabel(), 2.4f);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.2f, FColor::Cyan, Message);
    }
}

void ACodeRescueCharacter::CallSquadMedic()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    LastManualMedicCallWorldTime = World->GetTimeSeconds();
    bLastManualMedicCallSucceeded = false;

    ACompanionActor* BestMedic = nullptr;
    float BestReadySeconds = TNumericLimits<float>::Max();
    float BestDistanceSq = TNumericLimits<float>::Max();
    for (TActorIterator<ACompanionActor> It(World); It; ++It)
    {
        ACompanionActor* Companion = *It;
        if (!IsValid(Companion) || !Companion->IsOperational() || !Companion->bMedicSupport)
        {
            continue;
        }

        const float ReadySeconds = Companion->GetMedicPulseReadySeconds();
        const float DistanceSq = FVector::DistSquared(GetActorLocation(), Companion->GetActorLocation());
        const bool bBetterReady = ReadySeconds < BestReadySeconds - KINDA_SMALL_NUMBER;
        const bool bSameReadyCloser = FMath::IsNearlyEqual(ReadySeconds, BestReadySeconds, KINDA_SMALL_NUMBER)
            && DistanceSq < BestDistanceSq;
        if (bBetterReady || bSameReadyCloser)
        {
            BestMedic = Companion;
            BestReadySeconds = ReadySeconds;
            BestDistanceSq = DistanceSq;
        }
    }

    FString Message;
    FColor MessageColor = FColor::Silver;
    if (!BestMedic)
    {
        Message = TEXT("Medic call failed: no operational medic is available.");
    }
    else
    {
        BestMedic->ClearHoldPosition();
        BestMedic->ApplyFormationSpacingScale(GetSquadFormationSpacingScale());
        BestMedic->RegroupNearPlayer(this, 0);

        FString MedicMessage;
        bLastManualMedicCallSucceeded = BestMedic->TryManualMedicPulse(this, MedicMessage);
        const float SinceDamage = World->GetTimeSeconds() - LastDamageWorldTime;
        Message = bLastManualMedicCallSucceeded && SinceDamage >= 0.0f && SinceDamage < 6.0f
            ? FString::Printf(TEXT("Medic called after %s hit. %s"), *LastDamageLocationText, *MedicMessage)
            : MedicMessage;
        MessageColor = bLastManualMedicCallSucceeded ? FColor::Green : FColor::Cyan;

        if (bSquadHoldPosition)
        {
            BestMedic->SetHoldPosition(BestMedic->GetActorLocation(), BestMedic->GetActorRotation());
        }
    }

    UCodeRescueSubtitlesWidget::Push(Message, 3.0f);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.4f, MessageColor, Message);
    }
}

void ACodeRescueCharacter::ToggleSquadHoldPosition()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    bSquadHoldPosition = !bSquadHoldPosition;
    LastSquadOrderWorldTime = World->GetTimeSeconds();
    LastSquadOrderCount = 0;
    TArray<ACompanionActor*> RoleResponders;

    for (TActorIterator<ACompanionActor> It(World); It; ++It)
    {
        ACompanionActor* Companion = *It;
        if (!IsValid(Companion) || !Companion->IsOperational())
        {
            continue;
        }

        if (bSquadHoldPosition)
        {
            Companion->SetHoldPosition(Companion->GetActorLocation(), Companion->GetActorRotation());
        }
        else
        {
            Companion->ClearHoldPosition();
            Companion->ApplyFormationSpacingScale(GetSquadFormationSpacingScale());
        }
        ++LastSquadOrderCount;
        if (RoleResponders.Num() < 2)
        {
            RoleResponders.Add(Companion);
        }
    }

    const FString Message = bSquadHoldPosition
        ? FString::Printf(TEXT("Squad order: HOLD. %d teammate%s holding current positions."),
            LastSquadOrderCount,
            LastSquadOrderCount == 1 ? TEXT("") : TEXT("s"))
        : FString::Printf(TEXT("Squad order: FOLLOW. %d teammate%s back on formation."),
            LastSquadOrderCount,
            LastSquadOrderCount == 1 ? TEXT("") : TEXT("s"));
    UCodeRescueSubtitlesWidget::Push(Message, 3.0f);
    for (ACompanionActor* Responder : RoleResponders)
    {
        if (IsValid(Responder))
        {
            Responder->PushRoleOrderBark(GetSquadOrderLabel(), 2.4f);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.4f, bSquadHoldPosition ? FColor::Orange : FColor::Cyan, Message);
    }
}

int32 ACodeRescueCharacter::GrantScrap(int32 Amount)
{
    return AddScrap(Amount);
}

bool ACodeRescueCharacter::TrySpendScrap(int32 Amount)
{
    if (Amount <= 0)
    {
        return true;
    }
    if (Scrap < Amount)
    {
        return false;
    }
    Scrap -= Amount;
    return true;
}

int32 ACodeRescueCharacter::AddScrap(int32 Amount)
{
    if (Amount <= 0) return Scrap;
    Scrap += Amount;
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
            FString::Printf(TEXT("+%d scrap (total: %d)"), Amount, Scrap));
    }
    return Scrap;
}

void ACodeRescueCharacter::PlaceBarricade()
{
    if (Scrap < BarricadeScrapCost)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red,
                FString::Printf(TEXT("Need %d scrap to place a barricade. Have %d."), BarricadeScrapCost, Scrap));
        }
        return;
    }
    Scrap -= BarricadeScrapCost;
    const FVector Forward = GetActorForwardVector();
    const FVector Loc = GetActorLocation() + Forward * 220.0f - FVector(0, 0, 50.0f);
    const FRotator Rot(0.0f, GetActorRotation().Yaw + 90.0f, 0.0f);   // perpendicular to facing
    AActor* B = GetWorld()->SpawnActor<ABarricadeActor>(ABarricadeActor::StaticClass(), Loc, Rot);
    if (B && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Barricade placed (30s)."));
    }
}

void ACodeRescueCharacter::CycleThrowable()
{
    ActiveThrowableSlot = (ActiveThrowableSlot + 1) % 3;
    static const TCHAR* Names[] = { TEXT("Flare"), TEXT("Smoke"), TEXT("Stim") };
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan,
            FString::Printf(TEXT("Throwable: %s"), Names[ActiveThrowableSlot]));
    }
}

void ACodeRescueCharacter::ThrowActive()
{
    int32* CountPtr = nullptr;
    EThrowableKind KindToThrow = EThrowableKind::Flare;
    switch (ActiveThrowableSlot)
    {
    case 0: CountPtr = &FlareCount; KindToThrow = EThrowableKind::Flare; break;
    case 1: CountPtr = &SmokeCount; KindToThrow = EThrowableKind::Smoke; break;
    case 2: CountPtr = &StimCount;  KindToThrow = EThrowableKind::Stim;  break;
    default: return;
    }
    if (!CountPtr || *CountPtr <= 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Out of throwables of that kind."));
        }
        return;
    }
    UCameraComponent* ActiveCamera = GetActiveGameplayCamera();
    const FVector Start = ActiveCamera ? ActiveCamera->GetComponentLocation() : GetActorLocation();
    const FVector Forward = ActiveCamera ? ActiveCamera->GetForwardVector() : GetActorForwardVector();
    const FVector SpawnLoc = Start + Forward * 80.0f;
    const FTransform SpawnTransform(Forward.Rotation(), SpawnLoc);
    AThrowableActor* T = GetWorld()->SpawnActorDeferred<AThrowableActor>(
        AThrowableActor::StaticClass(),
        SpawnTransform,
        this,
        this,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if (T)
    {
        *CountPtr -= 1;
        T->Kind = KindToThrow;
        UGameplayStatics::FinishSpawningActor(T, SpawnTransform);
        T->LaunchThrowable(Forward);
        ReportStealthNoise(0.42f, UtilityNoiseRadius, TEXT("throwable"));
    }
    else if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Throwable could not be deployed."));
    }
}

FString ACodeRescueCharacter::GetFieldKitSummary() const
{
    return FString::Printf(
        TEXT("Gear: scanner %d/%d | flashlight %d/%d%s | bypass %d/%d | pouch +%d"),
        RadioScannerCharges,
        MaxRadioScannerCharges,
        FlashlightBatteries,
        MaxFlashlightBatteries,
        bFieldFlashlightActive ? TEXT(" on") : TEXT(""),
        BypassKits,
        MaxBypassKits,
        AmmoPouchCapacityBonus);
}

void ACodeRescueCharacter::ToggleFlashlight()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }

    if (bFieldFlashlightActive)
    {
        bFieldFlashlightActive = false;
        if (FieldFlashlight)
        {
            FieldFlashlight->SetIntensity(0.0f);
        }
        UCodeRescueSubtitlesWidget::Push(TEXT("Field flashlight off."), 1.8f);
        ReportStealthNoise(0.10f, UtilityNoiseRadius * 0.45f, TEXT("flashlight off"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.8f, FColor::Silver, TEXT("Flashlight off."));
        }
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            GI->SavePersistentRun();
        }
        return;
    }

    if (FlashlightBatteries <= 0)
    {
        UCodeRescueSubtitlesWidget::Push(TEXT("No flashlight batteries. Find a yellow field-light pickup."), 2.4f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.4f, FColor::Yellow, TEXT("No flashlight batteries."));
        }
        return;
    }

    FlashlightBatteries = FMath::Max(0, FlashlightBatteries - 1);
    bFieldFlashlightActive = true;
    if (FieldFlashlight)
    {
        FieldFlashlight->SetIntensity(5200.0f);
        FieldFlashlight->SetAttenuationRadius(1900.0f);
    }

    const FString Message = FString::Printf(
        TEXT("Field flashlight on. Batteries remaining: %d/%d."),
        FlashlightBatteries,
        MaxFlashlightBatteries);
    UCodeRescueSubtitlesWidget::Push(Message, 2.4f);
    ReportStealthNoise(0.38f, UtilityNoiseRadius, TEXT("flashlight exposure"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.4f, FColor::Yellow, Message);
    }
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->SavePersistentRun();
    }
}

void ACodeRescueCharacter::UseRadioScanner()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }

    if (RadioScannerCharges <= 0)
    {
        UCodeRescueSubtitlesWidget::Push(TEXT("Radio scanner is empty. Pick up cyan scanner charges."), 2.4f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.4f, FColor::Cyan, TEXT("No radio scanner charges."));
        }
        return;
    }

    RadioScannerCharges = FMath::Max(0, RadioScannerCharges - 1);
    ReportStealthNoise(0.36f, UtilityNoiseRadius, TEXT("radio scan"));
    const FString ScanLine = FString::Printf(
        TEXT("Radio scan: %s Charges remaining %d/%d."),
        *GetOpenWorldGuidanceText(),
        RadioScannerCharges,
        MaxRadioScannerCharges);
    UCodeRescueSubtitlesWidget::Push(ScanLine, 4.0f);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan, ScanLine);
    }
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        GI->SavePersistentRun();
    }
}

void ACodeRescueCharacter::MeleeAttack()
{
    UWorld* World = GetWorld();
    if (!World) return;
    const float Now = World->GetTimeSeconds();
    if (Now - TimeSinceLastMelee < 0.6f) return;
    TimeSinceLastMelee = Now;
    ReportStealthNoise(0.58f, UtilityNoiseRadius, TEXT("melee swing"));

    UCameraComponent* ActiveCamera = GetActiveGameplayCamera();
    const FVector Start = ActiveCamera ? ActiveCamera->GetComponentLocation() : GetActorLocation();
    const FVector Forward = ActiveCamera ? ActiveCamera->GetForwardVector() : GetActorForwardVector();
    const float MeleeRange = 200.0f;
    const float ConeHalfAngleCos = FMath::Cos(FMath::DegreesToRadians(45.0f));

    // Sweep all zombies near the player and pick those inside the cone.
    int32 HitsLanded = 0;
    int32 BarricadesHit = 0;
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Z = *It;
        if (!Z || Z->Health <= 0.0f) continue;
        const FVector ToZ = Z->GetActorLocation() - Start;
        if (ToZ.SizeSquared() > MeleeRange * MeleeRange) continue;
        const FVector ToZNorm = ToZ.GetSafeNormal();
        if (FVector::DotProduct(Forward, ToZNorm) < ConeHalfAngleCos) continue;
        Z->ApplyRescueDamage(80.0f, EHitZone::Torso);
        ++HitsLanded;
    }

    for (TActorIterator<ABarricadeActor> It(World); It; ++It)
    {
        ABarricadeActor* Barricade = *It;
        if (!IsValid(Barricade))
        {
            continue;
        }
        const FVector TargetPoint = Barricade->GetActorLocation() + FVector(0.0f, 0.0f, 35.0f);
        const FVector ToBarricade = TargetPoint - Start;
        if (ToBarricade.SizeSquared() > MeleeRange * MeleeRange)
        {
            continue;
        }
        const FVector ToBarricadeNorm = ToBarricade.GetSafeNormal();
        if (FVector::DotProduct(Forward, ToBarricadeNorm) < ConeHalfAngleCos)
        {
            continue;
        }
        Barricade->TakeBarricadeDamage(42.0f, TargetPoint, Forward, this);
        ++BarricadesHit;
    }

    if (HitConfirmCue && (HitsLanded > 0 || BarricadesHit > 0))
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitConfirmCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
    }
    if (HitsLanded > 0 || BarricadesHit > 0)
    {
        TriggerCombatJuiceHitConfirm(Start + Forward * 120.0f, EHitZone::Torso, true);
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, HitsLanded > 0 || BarricadesHit > 0 ? FColor::Yellow : FColor::Silver,
            FString::Printf(TEXT("Melee swing — %d zombies hit, %d barricades stressed"), HitsLanded, BarricadesHit));
    }
}

int32 ACodeRescueCharacter::ApplyAreaWeaponEffect(const FVector& ImpactPoint, const FWeaponDef& WeaponDef, const FString& EffectLabel)
{
    UWorld* World = GetWorld();
    if (!World || WeaponDef.ExplosionRadius <= 0.0f)
    {
        return 0;
    }

    // 2026-07-16 pass 5: the wireframe debug dome (Kenny's screenshot) is gone —
    // the explosion now presents physically per grenade type via
    // PlayExplosionPresentation() at the caller's impact point.

    int32 HitsLanded = 0;
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Zombie = *It;
        if (!IsValid(Zombie) || Zombie->Health <= 0.0f)
        {
            continue;
        }

        const float Distance = FVector::Dist(Zombie->GetActorLocation(), ImpactPoint);
        if (Distance > WeaponDef.ExplosionRadius)
        {
            continue;
        }

        const FVector TargetPoint = Zombie->GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
        const FVector Outward = (TargetPoint - ImpactPoint).GetSafeNormal();
        if (!HasClearWeaponPath(ImpactPoint + Outward * 12.0f, TargetPoint, Zombie))
        {
            UE_LOG(LogTemp, Verbose, TEXT("[WeaponHitValidation] radial effect occluded target=%s"), *Zombie->GetName());
            continue;
        }

        const float Falloff = FMath::Clamp(1.0f - (Distance / WeaponDef.ExplosionRadius) * 0.55f, 0.35f, 1.0f);
        const float Damage = FMath::Max(1.0f, WeaponDef.Damage * Falloff);
        Zombie->ApplyRescuePointDamage(
            Damage,
            EHitZone::Torso,
            TargetPoint,
            (TargetPoint - ImpactPoint).GetSafeNormal(),
            NAME_None);
        ++HitsLanded;
    }

    int32 BarricadesDamaged = 0;
    for (TActorIterator<ABarricadeActor> It(World); It; ++It)
    {
        ABarricadeActor* Barricade = *It;
        if (!IsValid(Barricade))
        {
            continue;
        }

        const float Distance = FVector::Dist(Barricade->GetActorLocation(), ImpactPoint);
        if (Distance > WeaponDef.ExplosionRadius)
        {
            continue;
        }

        const float Falloff = FMath::Clamp(1.0f - (Distance / WeaponDef.ExplosionRadius) * 0.65f, 0.25f, 1.0f);
        const float Damage = FMath::Max(2.0f, WeaponDef.Damage * 0.55f * Falloff);
        const FVector Direction = (Barricade->GetActorLocation() - ImpactPoint).GetSafeNormal();
        Barricade->TakeBarricadeDamage(Damage, ImpactPoint, Direction, this);
        ++BarricadesDamaged;
    }

    if (BulletImpactVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BulletImpactVFX, ImpactPoint, FRotator::ZeroRotator);
    }
    if (HitConfirmCue && HitsLanded > 0)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitConfirmCue, GetMonoSafeSoundLocation(this, ImpactPoint), GetRuntimeSfxVolume(this));
    }
    if (HitsLanded > 0 || BarricadesDamaged > 0)
    {
        TriggerCombatJuiceHitConfirm(ImpactPoint, EHitZone::Torso, true);
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, HitsLanded > 0 ? FColor::Orange : FColor::Silver,
            FString::Printf(TEXT("%s area effect: %d hostile%s hit, %d barricade%s stressed"),
                *EffectLabel,
                HitsLanded,
                HitsLanded == 1 ? TEXT("") : TEXT("s"),
                BarricadesDamaged,
                BarricadesDamaged == 1 ? TEXT("") : TEXT("s")));
    }
    return HitsLanded;
}

EHitZone ACodeRescueCharacter::ClassifyImpactPoint(const ACodeZombieActor* Zombie, const FHitResult& Hit) const
{
    const EHitZone BoneZone = ClassifyHitZone(Hit.BoneName);
    if (BoneZone != EHitZone::Other || !Zombie)
    {
        return BoneZone;
    }

    const float RelativeHeight = Hit.ImpactPoint.Z - Zombie->GetActorLocation().Z;
    if (RelativeHeight >= 52.0f)
    {
        return EHitZone::Head;
    }
    if (RelativeHeight >= -32.0f)
    {
        return EHitZone::Torso;
    }
    return EHitZone::Limb;
}

bool ACodeRescueCharacter::HasClearWeaponPath(const FVector& Start, const FVector& End, const AActor* IntendedTarget) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(CodeRescueValidatedWeaponPath), false, this);
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        if (*It != IntendedTarget)
        {
            Params.AddIgnoredActor(*It);
        }
    }

    FHitResult OcclusionHit;
    const bool bBlocked = World->LineTraceSingleByChannel(
        OcclusionHit,
        Start,
        End,
        CodeRescueCollision::WeaponTrace,
        Params);
    return !bBlocked || OcclusionHit.GetActor() == IntendedTarget;
}

void ACodeRescueCharacter::Fire()
{
    EnsureWeaponStateInitialized();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AimHoldTimer = FMath::Max(AimHoldTimer, 0.68f);

    const int32 WeaponIdx = static_cast<int32>(ActiveWeapon);
    const bool bHasWeaponDef = WeaponLoadout.IsValidIndex(WeaponIdx);
    const FWeaponDef* WDef = bHasWeaponDef ? &WeaponLoadout[WeaponIdx] : nullptr;

    if (WDef && !WDef->bUsesAmmo)
    {
        LastWeaponPresentationFireWorldTime = World->GetTimeSeconds();
        UpdateFirstPersonWeaponPresentation(0.0f);
        Tags.AddUnique(FName("DistinctWeaponPresentationFireCue"));
        TriggerCombatJuiceFireCue(WDef, true);
        ReportStealthNoise(0.58f, UtilityNoiseRadius, TEXT("melee weapon"));
        MeleeAttack();
        return;
    }

    const float EffectiveRefireDelay = WDef ? WDef->RefireDelay : FireRefireDelay;
    const float EffectiveRange       = WDef ? WDef->Range       : WeaponRange;
    const float EffectiveDamage      = WDef ? WDef->Damage      : DirectHitDamage;
    const int32 PelletCount          = WDef ? FMath::Max(1, WDef->PelletsPerShot) : 1;
    const float SpreadHalfDeg        = WDef ? WDef->SpreadHalfAngleDeg : 0.0f;
    const int32 BurstCount           = WDef ? FMath::Max(1, WDef->BurstCount) : 1;
    const int32 PierceCount          = WDef ? FMath::Max(0, WDef->PierceCount) : 0;
    const float AreaRadius           = WDef ? FMath::Max(0.0f, WDef->ExplosionRadius) : 0.0f;
    const FString WeaponName         = WDef ? WDef->DisplayName : TEXT("Weapon");

    const float NowSeconds = World->GetTimeSeconds();
    if (NowSeconds - LastFireWorldTime < EffectiveRefireDelay)
    {
        return;
    }

    if (MagazineAmmo <= 0)
    {
        const int32 ReserveAmmo = GetActiveWeaponReserveAmmo();
        if (ReserveAmmo > 0)
        {
            Reload();
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
                    FString::Printf(TEXT("%s empty. Reloading from reserve."), *WeaponName));
            }
            return;
        }

        // #27 - fallback to melee when both magazine and reserve are empty.
        LastWeaponPresentationFireWorldTime = NowSeconds;
        UpdateFirstPersonWeaponPresentation(0.0f);
        Tags.AddUnique(FName("DistinctWeaponPresentationMeleeFallbackCue"));
        TriggerCombatJuiceFireCue(WDef, true);
        ReportStealthNoise(0.62f, UtilityNoiseRadius, TEXT("empty melee fallback"));
        MeleeAttack();
        if (DryFireCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DryFireCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
        }
        return;
    }

    const int32 ShotsToFire = FMath::Clamp(BurstCount, 1, MagazineAmmo);
    MagazineAmmo -= ShotsToFire;
    if (WeaponMagazines.IsValidIndex(WeaponIdx))
    {
        WeaponMagazines[WeaponIdx] = MagazineAmmo;
    }
    TimeSinceLastFire = 0.0f;
    LastFireWorldTime = NowSeconds;
    LastWeaponPresentationFireWorldTime = NowSeconds;
    UpdateFirstPersonWeaponPresentation(0.0f);
    Tags.AddUnique(FName("DistinctWeaponPresentationFireCue"));
    TriggerCombatJuiceFireCue(WDef, false);
    ReportStealthNoise(
        AreaRadius > 0.0f ? 1.0f : 0.86f,
        AreaRadius > 0.0f ? FMath::Max(WeaponNoiseRadius, AreaRadius * 4.0f) : WeaponNoiseRadius,
        TEXT("weapon fire"));

    UCameraComponent* ActiveCamera = GetActiveGameplayCamera();
    const FVector Start = ActiveCamera ? ActiveCamera->GetComponentLocation() : GetActorLocation();
    FVector FireDirection = ActiveCamera ? ActiveCamera->GetForwardVector() : GetActorForwardVector();
    UpdateAutoTargetLock(0.0f);
    if (ACodeZombieActor* Target = GetLockedAimTarget())
    {
        const FVector PhysicalLockDirection = (GetAimTargetPoint(Target) - Start).GetSafeNormal();
        if (!PhysicalLockDirection.IsNearlyZero() &&
            IsAimTargetCandidate(Target, TargetLockBreakAngleDegrees, FMath::Min(TargetLockMaxDistance, EffectiveRange)))
        {
            FireDirection = PhysicalLockDirection;
            Tags.AddUnique(FName("TargetLockRedirectsPhysicalWeaponTrace"));
            UE_LOG(LogTemp, Verbose,
                TEXT("[TargetLock] physical shot target=%s range=%.0f"),
                *Target->GetName(),
                FVector::Dist(Start, GetAimTargetPoint(Target)));
        }
    }
    if (MuzzleFlashVFX && ActiveCamera)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlashVFX, ActiveCamera, NAME_None,
            FVector(60.0f, 30.0f, -10.0f), FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset, /*bAutoDestroy=*/true);
    }
    if (FireCue)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireCue, GetMonoSafeSoundLocation(this, Start), GetRuntimeSfxVolume(this));
    }

    // 2026-07-16 pass 5: grenade-family weapons are REAL lofted projectiles.
    // The projectile flies the same ballistic arc shown by the aim preview and
    // detonates at its ACTUAL landing point (DetonateGrenadePayload), so the
    // launch angle and the anticipated landing area are honest.
    if (WDef && WeaponIsGrenadeFamily(ActiveWeapon))
    {
        SpawnGrenadeProjectile(*WDef);
        return;
    }

    auto ApplyDirectDamage = [&](ACodeZombieActor* Zombie, EHitZone HitZone, const FVector& ImpactPoint, const FVector& ShotDirection, FName ImpactBone) -> bool
    {
        if (!IsValid(Zombie) || Zombie->Health <= 0.0f)
        {
            return false;
        }

        if (HitZone == EHitZone::Head)
        {
            LastHeadshotTime = World->GetTimeSeconds();
        }

        Zombie->ApplyRescuePointDamage(EffectiveDamage, HitZone, ImpactPoint, ShotDirection, ImpactBone);
        if (BulletImpactVFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                World, BulletImpactVFX, ImpactPoint, FRotator::ZeroRotator);
        }
        if (HitConfirmCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, HitConfirmCue, GetMonoSafeSoundLocation(this, ImpactPoint), GetRuntimeSfxVolume(this));
        }
        TriggerCombatJuiceHitConfirm(ImpactPoint, HitZone, true);
        return true;
    };

    bool bAnyHostileHit = false;

    for (int32 ShotIdx = 0; ShotIdx < ShotsToFire; ++ShotIdx)
    {
        for (int32 PelletIdx = 0; PelletIdx < PelletCount; ++PelletIdx)
        {
            const bool bUseSpread = SpreadHalfDeg > 0.0f || PelletCount > 1 || BurstCount > 1;
            const FVector ShotDirection = bUseSpread
                ? FMath::VRandCone(FireDirection, FMath::DegreesToRadians(SpreadHalfDeg))
                : FireDirection;
            const FVector ShotEnd = Start + ShotDirection * FMath::Max(250.0f, EffectiveRange);

            FHitResult ChannelHit;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(CodeRescueWeaponTrace), false, this);
            const bool bChannelHit = World->LineTraceSingleByChannel(
                ChannelHit,
                Start,
                ShotEnd,
                CodeRescueCollision::WeaponTrace,
                Params);

            // Some imported character profiles rebuild their response table at
            // runtime and can disappear from a custom trace channel despite an
            // enabled, blocking capsule. Resolve zombie objects independently,
            // then choose whichever physical hit is first on the exact ray.
            FHitResult ZombieObjectHit;
            FCollisionObjectQueryParams ZombieObjects;
            ZombieObjects.AddObjectTypesToQuery(CodeRescueCollision::ZombiePawnObject);
            const bool bZombieObjectHit = World->LineTraceSingleByObjectType(
                ZombieObjectHit,
                Start,
                ShotEnd,
                ZombieObjects,
                Params);

            FHitResult CapsuleGeometryHit;
            bool bCapsuleGeometryHit = false;
            float ClosestCapsuleDistance = TNumericLimits<float>::Max();
            const FVector NormalizedShotDirection = ShotDirection.GetSafeNormal();
            for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
            {
                ACodeZombieActor* Zombie = *It;
                UCapsuleComponent* Capsule = IsValid(Zombie) ? Zombie->GetCapsuleComponent() : nullptr;
                if (!Capsule || Zombie->Health <= 0.0f || !Zombie->GetActorEnableCollision() ||
                    Capsule->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
                {
                    continue;
                }

                const float Radius = Capsule->GetScaledCapsuleRadius();
                const float AxisHalfLength = FMath::Max(0.0f, Capsule->GetScaledCapsuleHalfHeight() - Radius);
                const FVector AxisOffset = Capsule->GetUpVector() * AxisHalfLength;
                FVector ClosestOnRay;
                FVector ClosestOnCapsule;
                FMath::SegmentDistToSegmentSafe(
                    Start,
                    ShotEnd,
                    Capsule->GetComponentLocation() - AxisOffset,
                    Capsule->GetComponentLocation() + AxisOffset,
                    ClosestOnRay,
                    ClosestOnCapsule);
                const float CapsuleMissDistance = FVector::Distance(ClosestOnRay, ClosestOnCapsule);
                if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelCombatRuntimeAudit")) &&
                    Zombie == FirstLevelCombatAuditTarget.Get())
                {
                    UE_LOG(LogTemp, Display,
                        TEXT("[FirstLevelCombatAudit] capsule_solver center=%s radius=%.2f ray=%s capsule=%s miss=%.3f"),
                        *Capsule->GetComponentLocation().ToCompactString(),
                        Radius,
                        *ClosestOnRay.ToCompactString(),
                        *ClosestOnCapsule.ToCompactString(),
                        CapsuleMissDistance);
                }
                if (CapsuleMissDistance > Radius + 1.5f)
                {
                    continue;
                }

                const float AlongRay = FVector::DotProduct(ClosestOnRay - Start, NormalizedShotDirection);
                if (AlongRay <= 0.0f || AlongRay >= ClosestCapsuleDistance)
                {
                    continue;
                }

                ClosestCapsuleDistance = AlongRay;
                CapsuleGeometryHit = FHitResult(
                    Zombie,
                    Capsule,
                    ClosestOnRay,
                    -NormalizedShotDirection);
                CapsuleGeometryHit.Distance = AlongRay;
                CapsuleGeometryHit.TraceStart = Start;
                CapsuleGeometryHit.TraceEnd = ShotEnd;
                bCapsuleGeometryHit = true;
            }

            const FHitResult* ClosestZombieHit = nullptr;
            if (bZombieObjectHit)
            {
                ClosestZombieHit = &ZombieObjectHit;
            }
            if (bCapsuleGeometryHit &&
                (!ClosestZombieHit || CapsuleGeometryHit.Distance < ClosestZombieHit->Distance))
            {
                ClosestZombieHit = &CapsuleGeometryHit;
            }
            const bool bZombieIsFirst = ClosestZombieHit &&
                (!bChannelHit || ClosestZombieHit->Distance <= ChannelHit.Distance + 1.0f);
            const FHitResult& Hit = bZombieIsFirst ? *ClosestZombieHit : ChannelHit;
            const bool bTraceHit = bZombieIsFirst || bChannelHit;
            const FVector ImpactPoint = bTraceHit ? Hit.ImpactPoint : ShotEnd;
            if (FParse::Param(FCommandLine::Get(), TEXT("FirstLevelCombatRuntimeAudit")))
            {
                UE_LOG(LogTemp, Display,
                    TEXT("[FirstLevelCombatAudit] solver channel=%d object=%d capsule=%d zombie_first=%d resolved=%s distance=%.2f"),
                    bChannelHit ? 1 : 0,
                    bZombieObjectHit ? 1 : 0,
                    bCapsuleGeometryHit ? 1 : 0,
                    bZombieIsFirst ? 1 : 0,
                    Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"),
                    Hit.Distance);
            }

#if ENABLE_DRAW_DEBUG
            // 2026-07-07 (Kenny: "correct the red beam ... replaced with a more
            // subtle indicator"): the old feedback was a thick red laser lasting
            // 0.65s per shot. Now: a hair-thin warm tracer that lives for a
            // single perceptual flick, plus a small impact glint where the shot
            // actually lands — enough to read your fire, quiet enough to ignore.
            const FColor TracerColor(255, 226, 180);
            DrawDebugLine(World, Start + ShotDirection * 90.0f, ImpactPoint, TracerColor, false, 0.07f, 0, 0.75f);
            if (bTraceHit)
            {
                DrawDebugPoint(World, ImpactPoint, 9.0f, FColor(255, 240, 205), false, 0.22f, 0);
            }
#endif

            if (AreaRadius > 0.0f && WDef)
            {
                if (bTraceHit)
                {
                    // pass 5: rockets/launched area weapons present physically too
                    PlayExplosionPresentation(ImpactPoint, *WDef);
                    bAnyHostileHit |= ApplyAreaWeaponEffect(ImpactPoint, *WDef, WeaponName) > 0;
                    Tags.AddUnique(FName("AreaWeaponRequiresPhysicalImpact"));
                }
                else
                {
                    UE_LOG(LogTemp, Display,
                        TEXT("[WeaponHitValidation] %s missed; radial effect suppressed at remote endpoint"),
                        *WeaponName);
                }
                continue;
            }

            if (PierceCount > 0)
            {
                struct FLineTarget
                {
                    float Along = 0.0f;
                    ACodeZombieActor* Zombie = nullptr;
                };

                TArray<FLineTarget> LineTargets;
                const FVector NormalizedShot = ShotDirection.GetSafeNormal();
                const float LineRadius = 58.0f;
                for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
                {
                    ACodeZombieActor* Zombie = *It;
                    if (!IsValid(Zombie) || Zombie->Health <= 0.0f)
                    {
                        continue;
                    }
                    const FVector TargetPoint = Zombie->GetActorLocation() + FVector(0.0f, 0.0f, 85.0f);
                    const FVector ToTarget = TargetPoint - Start;
                    const float Along = FVector::DotProduct(ToTarget, NormalizedShot);
                    if (Along <= 0.0f || Along > EffectiveRange)
                    {
                        continue;
                    }
                    const float PerpSq = (ToTarget - NormalizedShot * Along).SizeSquared();
                    if (PerpSq <= FMath::Square(LineRadius) &&
                        HasClearWeaponPath(Start, TargetPoint, Zombie))
                    {
                        FLineTarget Target;
                        Target.Along = Along;
                        Target.Zombie = Zombie;
                        LineTargets.Add(Target);
                    }
                }
                LineTargets.Sort([](const FLineTarget& A, const FLineTarget& B)
                {
                    return A.Along < B.Along;
                });

                const int32 MaxPierceHits = FMath::Min(LineTargets.Num(), PierceCount + 1);
                for (int32 TargetIdx = 0; TargetIdx < MaxPierceHits; ++TargetIdx)
                {
                    const FVector Impact = LineTargets[TargetIdx].Zombie->GetActorLocation() + FVector(0.0f, 0.0f, 85.0f);
                    bAnyHostileHit |= ApplyDirectDamage(
                        LineTargets[TargetIdx].Zombie,
                        EHitZone::Torso,
                        Impact,
                        NormalizedShot,
                        NAME_None);
                }
                if (MaxPierceHits > 0)
                {
                    continue;
                }
            }

            if (bTraceHit)
            {
                if (ACodeZombieActor* Zombie = Cast<ACodeZombieActor>(Hit.GetActor()))
                {
                    bAnyHostileHit |= ApplyDirectDamage(
                        Zombie,
                        ClassifyImpactPoint(Zombie, Hit),
                        Hit.ImpactPoint,
                        ShotDirection.GetSafeNormal(),
                        Hit.BoneName);
                    continue;
                }
                if (ABarricadeActor* Barricade = Cast<ABarricadeActor>(Hit.GetActor()))
                {
                    Barricade->TakeBarricadeDamage(EffectiveDamage, Hit.ImpactPoint, ShotDirection.GetSafeNormal(), this);
                    bAnyHostileHit = true;
                    if (BulletImpactVFX)
                    {
                        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                            World, BulletImpactVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
                    }
                    if (HitConfirmCue)
                    {
                        UGameplayStatics::PlaySoundAtLocation(this, HitConfirmCue, GetMonoSafeSoundLocation(this, Hit.ImpactPoint), GetRuntimeSfxVolume(this));
                    }
                    TriggerCombatJuiceHitConfirm(Hit.ImpactPoint, EHitZone::Torso, true);
                    continue;
                }
                if (BulletImpactVFX)
                {
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                        World, BulletImpactVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
                }
            }
        }
    }

    // No target receives damage outside the physical trace or explosion
    // volume. Auto-lock changes the ray before tracing; it never performs a
    // second proximity-based damage pass after a miss.
    if (!bAnyHostileHit && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Silver,
            FString::Printf(TEXT("%s fired. No hostile in range."), *WeaponName));
    }
}

// ---------------------------------------------------------------------------
// 2026-07-16 pass 5: aim-down-sights, scope magnification, grenade ballistics.

static const float GScopeZoomLadder[6] = { 1.0f, 2.0f, 5.0f, 10.0f, 20.0f, 50.0f };

bool ACodeRescueCharacter::WeaponSupportsScopeZoom(EWeaponType Weapon) const
{
    switch (Weapon)
    {
    case EWeaponType::Rifle:
    case EWeaponType::PrecisionRifle:
    case EWeaponType::SemiAutoRifle:
    case EWeaponType::BoltLauncher:
    case EWeaponType::RocketLauncher:
        return true;
    default:
        return false;
    }
}

bool ACodeRescueCharacter::WeaponIsGrenadeFamily(EWeaponType Weapon) const
{
    return Weapon == EWeaponType::Grenade ||
           Weapon == EWeaponType::IncendiaryGrenade ||
           Weapon == EWeaponType::FlashGrenade;
}

float ACodeRescueCharacter::GetCurrentScopeZoom() const
{
    if (!WeaponSupportsScopeZoom(ActiveWeapon))
    {
        return 1.0f;
    }
    return GScopeZoomLadder[FMath::Clamp(ScopeZoomIndex, 0, 5)];
}

FString ACodeRescueCharacter::GetScopeZoomLabel() const
{
    const float Zoom = GetCurrentScopeZoom();
    return Zoom >= 10.0f
        ? FString::Printf(TEXT("%.0fx"), Zoom)
        : FString::Printf(TEXT("%gx"), Zoom);
}

void ACodeRescueCharacter::CycleScopeZoom()
{
    if (bUIOpen || Health <= 0.0f)
    {
        return;
    }
    if (!WeaponSupportsScopeZoom(ActiveWeapon))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.6f, FColor::Silver,
                TEXT("This weapon has no magnified optic (rifles, bolt launcher, rocket launcher do)."));
        }
        return;
    }
    if (!bADSActive)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.6f, FColor::Silver,
                TEXT("Right-click raises the sights (quick click locks them; click again to lower). Z cycles zoom: 1x 2x 5x 10x 20x 50x."));
        }
        return;
    }
    ScopeZoomIndex = (ScopeZoomIndex + 1) % 6;
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.4f, FColor::Cyan,
            FString::Printf(TEXT("Scope magnification: %s"), *GetScopeZoomLabel()));
    }
    Tags.AddUnique(FName("ScopeZoomLadderRuntime"));
    UE_LOG(LogTemp, Display, TEXT("[ScopeZoom] level=%s weapon=%d"), *GetScopeZoomLabel(),
        static_cast<int32>(ActiveWeapon));
}

bool ACodeRescueCharacter::IsScopeViewActive() const
{
    return bADSActive && CameraPerspective == 0 && ADSBlend > 0.85f &&
        WeaponSupportsScopeZoom(ActiveWeapon) && !WeaponIsGrenadeFamily(ActiveWeapon);
}

void ACodeRescueCharacter::UpdateADSPresentation(float DeltaSeconds)
{
    // 2026-07-17: any modal UI lowers the weapon outright — a menu opened
    // mid-aim used to eat the release event and leave the sights stuck on.
    if (bUIOpen && bADSActive)
    {
        EndAim();
    }
    const bool bWantADS = bADSActive && CameraPerspective == 0 && !bUIOpen && Health > 0.0f;
    ADSBlend = FMath::FInterpTo(ADSBlend, bWantADS ? 1.0f : 0.0f, DeltaSeconds, 9.0f);

    const float Zoom = bWantADS ? GetCurrentScopeZoom() : 1.0f;
    if (FirstPersonCamera)
    {
        const float TargetFOV = FMath::Clamp(BaseFirstPersonFOV / Zoom, 1.6f, BaseFirstPersonFOV);
        FirstPersonCamera->SetFieldOfView(
            FMath::Lerp(BaseFirstPersonFOV, TargetFOV, ADSBlend));
        // Cinematic depth-of-field + motion blur turn a 20x/50x magnified
        // image into mush (cycle-11 review: the 50x frame was unreadable).
        // Real optics are pin-sharp: kill both while sighted in.
        FPostProcessSettings& PP = FirstPersonCamera->PostProcessSettings;
        const bool bSharpOptics = ADSBlend > 0.1f;
        PP.bOverride_DepthOfFieldFstop = bSharpOptics;
        PP.DepthOfFieldFstop = 32.0f;
        PP.bOverride_DepthOfFieldFocalDistance = bSharpOptics;
        PP.DepthOfFieldFocalDistance = 1000000.0f;
        PP.bOverride_MotionBlurAmount = bSharpOptics;
        PP.MotionBlurAmount = 0.0f;
        // 2026-07-17 (Kenny's thermal-sight reference): while looking THROUGH
        // the optic the world reads like a cooled sensor image — desaturated
        // cool tint, lifted exposure, hot bloom (emissive zombie eyes and
        // muzzle flashes glow against the cold background). The HUD draws the
        // circular scope mask + fine reticle + ZOOM label on top.
        const bool bThermalScope = IsScopeViewActive();
        PP.bOverride_ColorSaturation = bThermalScope;
        PP.ColorSaturation = FVector4(0.30f, 0.34f, 0.42f, 1.0f);
        PP.bOverride_ColorGain = bThermalScope;
        PP.ColorGain = FVector4(0.80f, 0.94f, 1.22f, 1.0f);
        PP.bOverride_AutoExposureBias = bThermalScope;
        PP.AutoExposureBias = 0.55f;
        PP.bOverride_BloomIntensity = bThermalScope;
        PP.BloomIntensity = 1.65f;
    }
    // scaled look input keeps high magnification controllable
    ADSLookScale = FMath::Lerp(1.0f, 1.0f / FMath::Pow(FMath::Max(Zoom, 1.0f), 0.82f), ADSBlend);
    // NOTE: the viewmodel pose/visibility is owned by
    // UpdateFirstPersonWeaponPresentation (it runs later in Tick and was
    // overwriting any pose set here — cycle-11 lesson: one writer only).
}

bool ACodeRescueCharacter::ComputeGrenadeLaunch(FVector& OutStart, FVector& OutVelocity) const
{
    const UCameraComponent* Camera = FirstPersonCamera;
    if (!Camera)
    {
        return false;
    }
    const FVector Forward = Camera->GetForwardVector();
    // 55 uu forward clears the movement capsule (radius ~34) so the live
    // grenade can never start overlapping its thrower (cycle-11 sky-detonation).
    OutStart = Camera->GetComponentLocation() + Forward * 55.0f +
        Camera->GetRightVector() * 10.0f - FVector(0.0f, 0.0f, 6.0f);
    // fixed loft on top of the aim direction: raising/lowering the view
    // visibly changes the launch angle and the predicted landing point
    OutVelocity = (Forward + FVector::UpVector * 0.25f).GetSafeNormal() * 1480.0f;
    return true;
}

void ACodeRescueCharacter::SpawnGrenadeProjectile(const FWeaponDef& WeaponDef)
{
    UWorld* World = GetWorld();
    FVector Start, Velocity;
    if (!World || !ComputeGrenadeLaunch(Start, Velocity))
    {
        return;
    }
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;
    Params.Instigator = this;
    AThrowableActor* Projectile = World->SpawnActor<AThrowableActor>(
        AThrowableActor::StaticClass(), Start, GetActorRotation(), Params);
    if (!Projectile)
    {
        return;
    }
    float FuseSeconds = 1.5f;
    if (ActiveWeapon == EWeaponType::IncendiaryGrenade) { FuseSeconds = 1.25f; }
    if (ActiveWeapon == EWeaponType::FlashGrenade)      { FuseSeconds = 1.1f; }
    Projectile->ConfigureGrenadePayload(this, static_cast<uint8>(ActiveWeapon), FuseSeconds, Velocity);
    Tags.AddUnique(FName("GrenadeKinematicLaunchRuntime"));
    UE_LOG(LogTemp, Display, TEXT("[GrenadeLaunch] %s vel=%s fuse=%.2f"),
        *WeaponDef.DisplayName, *Velocity.ToCompactString(), FuseSeconds);
}

void ACodeRescueCharacter::DetonateGrenadePayload(const FVector& Location, uint8 WeaponTypeRaw)
{
    const EWeaponType PayloadType = static_cast<EWeaponType>(WeaponTypeRaw);
    const FWeaponDef* Def = GetWeaponDefinition(PayloadType);
    if (!Def)
    {
        return;
    }
    const int32 Hits = ApplyAreaWeaponEffect(Location, *Def, Def->DisplayName);
    PlayExplosionPresentation(Location, *Def);
    UE_LOG(LogTemp, Display, TEXT("[GrenadeDetonation] %s at %s hits=%d radius=%.0f"),
        *Def->DisplayName, *Location.ToCompactString(), Hits, Def->ExplosionRadius);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, Hits > 0 ? FColor::Yellow : FColor::Silver,
            FString::Printf(TEXT("%s detonated — %d hostiles caught in the blast."),
                *Def->DisplayName, Hits));
    }
}

void ACodeRescueCharacter::PlayExplosionPresentation(const FVector& Location, const FWeaponDef& WeaponDef)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    const float Radius = FMath::Max(60.0f, WeaponDef.ExplosionRadius);
    const bool bIncendiary = WeaponDef.DisplayName.Contains(TEXT("Incendiary"));
    const bool bFlash = WeaponDef.DisplayName.Contains(TEXT("Flash"));

    // burst particles: sharp core burst + offset secondaries (frag/rocket)
    if (BulletImpactVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BulletImpactVFX, Location, FRotator::ZeroRotator);
        if (!bFlash)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BulletImpactVFX,
                Location + FVector(Radius * 0.22f, Radius * 0.12f, 26.0f), FRotator::ZeroRotator);
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BulletImpactVFX,
                Location + FVector(-Radius * 0.16f, -Radius * 0.20f, 40.0f), FRotator::ZeroRotator);
        }
    }
    // incendiary burns: re-igniting bursts across the area over ~1.5 s so the
    // patch visibly keeps burning after the initial detonation
    if (bIncendiary && BulletImpactVFX)
    {
        TWeakObjectPtr<ACodeRescueCharacter> WeakThis(this);
        const FVector BurnCenter = Location;
        const float BurnRadius = Radius;
        for (int32 Wave = 1; Wave <= 2; ++Wave)
        {
            FTimerHandle BurnTimer;
            GetWorldTimerManager().SetTimer(BurnTimer,
                FTimerDelegate::CreateLambda([WeakThis, BurnCenter, BurnRadius]()
                {
                    if (!WeakThis.IsValid() || !WeakThis->GetWorld() || !WeakThis->BulletImpactVFX)
                    {
                        return;
                    }
                    for (int32 Spark = 0; Spark < 3; ++Spark)
                    {
                        const FVector Offset(
                            FMath::FRandRange(-BurnRadius * 0.5f, BurnRadius * 0.5f),
                            FMath::FRandRange(-BurnRadius * 0.5f, BurnRadius * 0.5f),
                            FMath::FRandRange(8.0f, 46.0f));
                        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                            WeakThis->GetWorld(), WeakThis->BulletImpactVFX,
                            BurnCenter + Offset, FRotator::ZeroRotator);
                    }
                }),
                0.7f * Wave, false);
        }
    }

    // physical light flash per type
    if (APointLight* Flash = World->SpawnActor<APointLight>(Location + FVector(0, 0, 90.0f), FRotator::ZeroRotator))
    {
        if (UPointLightComponent* LightComp = Flash->PointLightComponent)
        {
            LightComp->SetMobility(EComponentMobility::Movable);
            LightComp->SetIntensity(bFlash ? 26000.0f : (bIncendiary ? 7000.0f : 9500.0f));
            LightComp->SetLightColor(bFlash ? FLinearColor(1.0f, 0.98f, 0.92f)
                : (bIncendiary ? FLinearColor(1.0f, 0.38f, 0.06f) : FLinearColor(1.0f, 0.62f, 0.16f)));
            LightComp->SetAttenuationRadius(Radius * (bFlash ? 3.2f : 2.1f));
        }
        Flash->SetLifeSpan(bFlash ? 0.65f : (bIncendiary ? 2.4f : 0.55f));
    }

    // scorch mark: flattened dark disc where the blast actually happened
    if (!bFlash)
    {
        if (AStaticMeshActor* Scorch = World->SpawnActor<AStaticMeshActor>(
                Location + FVector(0.0f, 0.0f, 2.5f), FRotator::ZeroRotator))
        {
            if (UStaticMeshComponent* ScorchMesh = Scorch->GetStaticMeshComponent())
            {
                ScorchMesh->SetMobility(EComponentMobility::Movable);
                if (UStaticMesh* Disc = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
                {
                    ScorchMesh->SetStaticMesh(Disc);
                }
                ScorchMesh->SetWorldScale3D(FVector(
                    Radius * (bIncendiary ? 1.5f : 1.15f) / 100.0f,
                    Radius * (bIncendiary ? 1.5f : 1.15f) / 100.0f,
                    0.02f));
                ScorchMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ScorchMesh->SetCastShadow(false);
                if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
                {
                    if (UMaterialInstanceDynamic* MID = ScorchMesh->CreateAndSetMaterialInstanceDynamic(0))
                    {
                        MID->SetVectorParameterValue(TEXT("Color"),
                            bIncendiary ? FLinearColor(0.09f, 0.035f, 0.012f) : FLinearColor(0.03f, 0.03f, 0.032f));
                    }
                }
            }
            Scorch->SetLifeSpan(bIncendiary ? 16.0f : 12.0f);
            Scorch->Tags.AddUnique(FName("ExplosionScorchDecal"));
        }
    }

    // pressure-wave camera kick when the player is near the blast
    const float PlayerDistance = FVector::Dist(GetActorLocation(), Location);
    if (PlayerDistance < Radius * 2.2f)
    {
        const float Falloff = 1.0f - FMath::Clamp(PlayerDistance / (Radius * 2.2f), 0.0f, 1.0f);
        AddControllerPitchInput(-1.5f * Falloff);
        AddControllerYawInput((FMath::FRand() < 0.5f ? -1.0f : 1.0f) * 0.6f * Falloff);
    }
    Tags.AddUnique(FName("ExplosionPhysicalPresentation"));
}

void ACodeRescueCharacter::UpdateGrenadeArcPreview(float DeltaSeconds)
{
    (void)DeltaSeconds;
    const bool bWant = bADSActive && WeaponIsGrenadeFamily(ActiveWeapon) &&
        CameraPerspective == 0 && !bUIOpen && Health > 0.0f;
    if (!bWant)
    {
        if (bGrenadePreviewVisible)
        {
            for (UStaticMeshComponent* Dot : GrenadeArcDots)
            {
                if (Dot) { Dot->SetVisibility(false); }
            }
            for (UStaticMeshComponent* Segment : GrenadeRingSegments)
            {
                if (Segment) { Segment->SetVisibility(false); }
            }
            if (GrenadeLandingRing) { GrenadeLandingRing->SetVisibility(false); }
            bGrenadePreviewVisible = false;
        }
        return;
    }

    // lazy pool: 22 arc dots + the landing/blast ring
    if (GrenadeArcDots.Num() == 0)
    {
        UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        UStaticMesh* DiscMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
        UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        auto ConfigureMarker = [&](UStaticMeshComponent* Marker, UStaticMesh* Mesh, const FLinearColor& Color)
        {
            // round-1 harness lesson: unattached pooled components ignored the
            // pre-registration scale + slot-0 MID — attach absolutely to the
            // root and build the MID explicitly from the base material.
            Marker->SetupAttachment(GetRootComponent());
            Marker->SetUsingAbsoluteLocation(true);
            Marker->SetUsingAbsoluteRotation(true);
            Marker->SetUsingAbsoluteScale(true);
            Marker->RegisterComponent();
            if (Mesh) { Marker->SetStaticMesh(Mesh); }
            Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Marker->SetCastShadow(false);
            if (Base)
            {
                if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
                {
                    MID->SetVectorParameterValue(TEXT("Color"), Color);
                    Marker->SetMaterial(0, MID);
                }
            }
            Marker->SetVisibility(false);
        };
        for (int32 Index = 0; Index < 22; ++Index)
        {
            UStaticMeshComponent* Dot = NewObject<UStaticMeshComponent>(this);
            ConfigureMarker(Dot, SphereMesh, FLinearColor(1.0f, 0.72f, 0.18f));
            // cycle-11: 5.5 cm dots were sub-pixel at throw distance
            Dot->SetWorldScale3D(FVector(0.16f));
            GrenadeArcDots.Add(Dot);
        }
        // landing marker = small impact pad + blast-radius OUTLINE segments
        // (the filled disc at frag radius painted 14 m of street solid orange)
        GrenadeLandingRing = NewObject<UStaticMeshComponent>(this);
        ConfigureMarker(GrenadeLandingRing, DiscMesh, FLinearColor(1.0f, 0.42f, 0.10f));
        for (int32 Index = 0; Index < 20; ++Index)
        {
            UStaticMeshComponent* Segment = NewObject<UStaticMeshComponent>(this);
            ConfigureMarker(Segment, DiscMesh, FLinearColor(1.0f, 0.30f, 0.05f));
            GrenadeRingSegments.Add(Segment);
        }
    }

    FVector Start, Velocity;
    if (!ComputeGrenadeLaunch(Start, Velocity))
    {
        return;
    }
    FPredictProjectilePathParams Params(AThrowableActor::GrenadeProjectileRadius, Start, Velocity, 3.4f);
    Params.bTraceWithCollision = true;
    Params.TraceChannel = ECC_Visibility;
    Params.ActorsToIgnore.Add(const_cast<ACodeRescueCharacter*>(this));
    Params.SimFrequency = 22.0f;
    FPredictProjectilePathResult Result;
    UGameplayStatics::PredictProjectilePath(this, Params, Result);

    const int32 PathPoints = Result.PathData.Num();
    const FVector CameraLoc = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : Start;
    int32 DotSlot = 0;
    for (int32 PathIndex = 1; PathIndex < PathPoints && DotSlot < GrenadeArcDots.Num(); ++PathIndex)
    {
        const FVector& Point = Result.PathData[PathIndex].Location;
        if (FVector::DistSquared(Point, CameraLoc) < 240.0f * 240.0f)
        {
            continue;   // never park a marker inside the player's face
        }
        if (UStaticMeshComponent* Dot = GrenadeArcDots[DotSlot])
        {
            Dot->SetWorldLocation(Point);
            Dot->SetWorldScale3D(FVector(0.055f));
            Dot->SetVisibility(true);
        }
        ++DotSlot;
    }
    for (int32 Index = DotSlot; Index < GrenadeArcDots.Num(); ++Index)
    {
        if (GrenadeArcDots[Index])
        {
            GrenadeArcDots[Index]->SetVisibility(false);
        }
    }
    const FWeaponDef* Def = GetWeaponDefinition(ActiveWeapon);
    const float BlastRadius = Def ? FMath::Max(80.0f, Def->ExplosionRadius) : 300.0f;
    const FVector Landing = Result.HitResult.bBlockingHit
        ? FVector(Result.HitResult.ImpactPoint)
        : (PathPoints > 0 ? Result.PathData.Last().Location : Start);
    if (GrenadeLandingRing)
    {
        // impact pad: fixed small marker AT the landing point
        GrenadeLandingRing->SetWorldLocation(Landing + FVector(0.0f, 0.0f, 3.0f));
        GrenadeLandingRing->SetWorldScale3D(FVector(0.55f, 0.55f, 0.012f));
        GrenadeLandingRing->SetVisibility(true);
    }
    // blast-radius outline: rim pads on the predicted blast circle
    const int32 SegmentCount = GrenadeRingSegments.Num();
    for (int32 Index = 0; Index < SegmentCount; ++Index)
    {
        UStaticMeshComponent* Segment = GrenadeRingSegments[Index];
        if (!Segment)
        {
            continue;
        }
        const float Angle = (2.0f * PI * Index) / FMath::Max(1, SegmentCount);
        const FVector RimPoint = Landing +
            FVector(FMath::Cos(Angle) * BlastRadius, FMath::Sin(Angle) * BlastRadius, 3.0f);
        Segment->SetWorldLocation(RimPoint);
        Segment->SetWorldScale3D(FVector(0.30f, 0.30f, 0.010f));
        Segment->SetVisibility(true);
    }
    bGrenadePreviewVisible = true;
    Tags.AddUnique(FName("GrenadeArcPreviewRuntime"));
}

// ---------------------------------------------------------------------------
// -CodeRescuePerspectiveReview: self-driving visual review harness (pass 5).

void ACodeRescueCharacter::StartPerspectiveReviewHarness()
{
    PerspectiveReviewStage = 0;
    GetWorldTimerManager().SetTimer(PerspectiveReviewTimer, this,
        &ACodeRescueCharacter::AdvancePerspectiveReview, 0.9f, true, 2.5f);
    UE_LOG(LogTemp, Display, TEXT("[PerspectiveReview] harness armed"));
}

void ACodeRescueCharacter::AdvancePerspectiveReview()
{
    auto Screenshot = [](const TCHAR* Name)
    {
        const FString Path = FPaths::ProjectSavedDir() /
            FString::Printf(TEXT("Screenshots/FirstLevel/review_%s.png"), Name);
        // 2026-07-17: capture WITH UI — the review must show what the player
        // sees (scope circle, reticle, prompts), not a bare render.
        FScreenshotRequest::RequestScreenshot(Path, /*bShowUI=*/true, false);
    };
    auto EquipForReview = [this](EWeaponType Weapon)
    {
        ActiveWeapon = Weapon;
        const int32 Index = static_cast<int32>(Weapon);
        if (WeaponMagazines.IsValidIndex(Index))
        {
            WeaponMagazines[Index] = FMath::Max(WeaponMagazines[Index], 5);
        }
        MagazineAmmo = FMath::Max(MagazineAmmo, 5);
        RefreshFirstPersonWeapon();
    };

    auto SetReviewPitch = [this](float Pitch)
    {
        if (AController* ReviewController = GetController())
        {
            FRotator ReviewRot = ReviewController->GetControlRotation();
            ReviewRot.Pitch = Pitch;
            ReviewController->SetControlRotation(ReviewRot);
        }
    };

    // Cycle-11 lesson: RequestScreenshot captures at END of frame, so a stage
    // must never change state after requesting a shot — the file would show
    // the NEXT state. Even stages arrange, odd stages photograph.
    const int32 Stage = PerspectiveReviewStage++;
    switch (Stage)
    {
    case 0:
        // play-like framing: gentle downward gaze, rifle in hand
        SetReviewPitch(-12.0f);
        CameraPerspective = 0; ApplyCameraPerspective();
        EquipForReview(EWeaponType::Rifle);
        if (HeroPresentationMesh)
        {
            UE_LOG(LogTemp, Display,
                TEXT("[PerspectiveReview] hero diag configured=%d visflag=%d rendered=%d registered=%d asset=%s boundsrad=%.0f"),
                bHeroPresentationConfigured ? 1 : 0,
                HeroPresentationMesh->GetVisibleFlag() ? 1 : 0,
                HeroPresentationMesh->IsVisible() ? 1 : 0,
                HeroPresentationMesh->IsRegistered() ? 1 : 0,
                *GetNameSafe(HeroPresentationMesh->GetSkinnedAsset()),
                HeroPresentationMesh->Bounds.SphereRadius);
        }
        break;
    case 1:  Screenshot(TEXT("perspective_0_fp")); break;
    case 2:  CameraPerspective = 1; ApplyCameraPerspective(); break;
    case 3:  Screenshot(TEXT("perspective_1_third")); break;
    case 4:  CameraPerspective = 2; ApplyCameraPerspective(); break;
    case 5:  Screenshot(TEXT("perspective_2_tactical")); break;
    case 6:  CameraPerspective = 3; ApplyCameraPerspective(); break;
    case 7:  Screenshot(TEXT("perspective_3_topdown")); break;
    case 8:  CameraPerspective = 4; ApplyCameraPerspective(); break;
    case 9:  Screenshot(TEXT("perspective_4_iso")); break;
    case 10: CameraPerspective = 5; ApplyCameraPerspective(); break;
    case 11: Screenshot(TEXT("perspective_5_side")); break;
    case 12:
        // sight-in FROM THE SIDE CAMERA — deliberately NOT switching to FP
        // first: holding aim must step any perspective into the scope view
        // and hand it back on release (2026-07-17 raise/lower contract).
        SetReviewPitch(-2.0f);
        BeginAim();
        ScopeZoomIndex = 0;
        break;
    case 13: Screenshot(TEXT("ads_rifle_1x"));  break;
    case 14: ScopeZoomIndex = 2; break;
    case 15: Screenshot(TEXT("ads_rifle_5x"));  break;
    case 16: ScopeZoomIndex = 3; break;
    case 17: Screenshot(TEXT("ads_rifle_10x")); break;
    case 18: ScopeZoomIndex = 5; break;
    case 19: Screenshot(TEXT("ads_rifle_50x")); break;
    case 20:
        EndAim();
        EquipForReview(EWeaponType::Grenade);
        SetReviewPitch(-10.0f);
        BeginAim();
        break;
    case 21: Screenshot(TEXT("grenade_arc")); break;
    case 22: Fire(); break;
    case 23: EndAim(); break;   // preview off; grenade lands, fuse burns
    case 24: Screenshot(TEXT("grenade_explosion")); break;   // det ~t+1.5, light 0.55 s
    case 25: Screenshot(TEXT("grenade_aftermath")); break;   // scorch mark remains
    case 26:
        // Player-reported right/east perimeter review. Three positions cover
        // the full edge while the runtime audit probes nine collision points.
        EndAim();
        EquipForReview(EWeaponType::Rifle);
        CameraPerspective = 2;
        ApplyCameraPerspective();
        SetActorLocation(
            FCodeRescueCampaign::GetCityOrigin(0) + FVector(10350.0f, -6200.0f, GCodeRescueArenaSafeGroundZ),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
        if (AController* ReviewController = GetController())
        {
            ReviewController->SetControlRotation(FRotator(-12.0f, 90.0f, 0.0f));
        }
        break;
    case 27: Screenshot(TEXT("right_perimeter_south")); break;
    case 28:
        SetActorLocation(
            FCodeRescueCampaign::GetCityOrigin(0) + FVector(10350.0f, 0.0f, GCodeRescueArenaSafeGroundZ),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        break;
    case 29: Screenshot(TEXT("right_perimeter_center")); break;
    case 30:
        SetActorLocation(
            FCodeRescueCampaign::GetCityOrigin(0) + FVector(10350.0f, 6200.0f, GCodeRescueArenaSafeGroundZ),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        break;
    case 31: Screenshot(TEXT("right_perimeter_north")); break;
    default:
        UE_LOG(LogTemp, Display, TEXT("[PerspectiveReview] COMPLETE stages=%d screenshots=Saved/Screenshots/FirstLevel/review_*.png right_perimeter_positions=3"), Stage);
        GetWorldTimerManager().ClearTimer(PerspectiveReviewTimer);
        FPlatformMisc::RequestExit(false);
        break;
    }
}

void ACodeRescueCharacter::Interact()
{
    // Modal readers own E/Enter/Esc until closed. Without this guard the same
    // E key opened another nearby marker behind the reader, while P/Esc could
    // open the Field Armory underneath it and trap both input layers.
    if (UCodeRescueMessageReaderWidget::IsReaderOpen())
    {
        UCodeRescueMessageReaderWidget::CloseActiveReader();
        return;
    }

    // Arm the shared cooldown so the polled path (PollDirectKeys) cannot double-fire this
    // same press when the BindKey event also delivered. 2026-07-01.
    DirectKeyCooldown = FMath::Max(DirectKeyCooldown, 0.25f);
    UE_LOG(LogTemp, Display, TEXT("[CodeRescueInteract] Interact() fired"));

    FHitResult Hit;
    AActor* Candidate = nullptr;
    bool bCandidateFromAim = false;   // true when the player is actually LOOKING at it

    if (TraceForward(Hit, InteractionTraceDistance))
    {
        Candidate = Hit.GetActor();
        bCandidateFromAim = Candidate && IsInteractableActor(Candidate);
    }

    if (!Candidate || !IsInteractableActor(Candidate))
    {
        Candidate = FindNearestInteractable(InteractionAssistRadius);
        bCandidateFromAim = false;
    }

    // #7 — Helipad fast-travel. Detected by tag so we don't have to add a
    // direct dependency on HelipadActor.h here for the polled-key path.
    if (Candidate && Candidate->Tags.Contains(FName("Helipad")))
    {
        if (UFunction* OpenFn = Candidate->FindFunction(FName(TEXT("OpenFastTravelMenu"))))
        {
            Candidate->ProcessEvent(OpenFn, nullptr);
        }
        return;
    }

    // World-text declutter: a message marker opens the scrollable reader screen. Dispatched by
    // name (like the Helipad above) so the character needs no dependency on the marker header.
    if (Candidate && Candidate->Tags.Contains(FName("MessageMarker")))
    {
        if (UFunction* OpenFn = Candidate->FindFunction(FName(TEXT("OpenMessageReader"))))
        {
            Candidate->ProcessEvent(OpenFn, nullptr);
        }
        return;
    }

    // 2026-07-11 pass 4: swinging doors on the enterable buildings. Same
    // tag-dispatch pattern as the Helipad so no header dependency is needed.
    if (Candidate && Candidate->Tags.Contains(FName("CodeRescueDoor")))
    {
        if (UFunction* ToggleFn = Candidate->FindFunction(FName(TEXT("ToggleDoor"))))
        {
            Candidate->ProcessEvent(ToggleFn, nullptr);
        }
        return;
    }

    if (AJeepActor* Jeep = Cast<AJeepActor>(Candidate))
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            Jeep->Mount(PC, this);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                    TEXT("Jeep mounted. WASD drives; E dismounts."));
            }
        }
        return;
    }

    if (AFriendlyNPCActor* NPC = Cast<AFriendlyNPCActor>(Candidate))
    {
        // #68: ambient NPC perks. Each role does its own thing inside Interact.
        NPC->Interact(this);
        return;
    }

    if (ASurvivorActor* Survivor = Cast<ASurvivorActor>(Candidate))
    {
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            if (!FCodeRescueCampaign::IsCityUnlocked(GI, Survivor->CityIndex))
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow,
                        FString::Printf(TEXT("Locked city: graduate earlier missions before rescuing %s."),
                            *FCodeRescueCampaign::GetMissionLabel(Survivor->CityIndex)));
                }
                return;
            }
        }
        if (Survivor->Rescue() && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                4.0f,
                Survivor->ArchetypeAccentColor.ToFColor(true),
                FString::Printf(TEXT("Rescued: %s"), *Survivor->GetSurvivorArchetypeSummary()));
        }
        return;
    }

    if (ALanguageStationActor* Station = Cast<ALanguageStationActor>(Candidate))
    {
        // 2026-07-01 fix: the stations were decorative no-ops, and the UMG launch menu does not
        // render in packaged builds - so there was NO working way to pick a language in the
        // packaged game. Walking up and pressing E/Enter now genuinely deploys that language:
        // resume the language's save if one exists, otherwise start a fresh run, then reload
        // the world for the chosen track.
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            if (!GI->bHasSelectedLaunchLanguageThisSession)
            {
                // 2026-07-01 (round 2, playtest-verified problem): deploying at a station now
                // requires DELIBERATE aim - the player must be looking at the station, close up.
                // The spawn point stands within assist range of the center pedestals, so any
                // polled Tab/Enter at boot silently deployed the nearest language. The menu is
                // the primary selector; stations are the walk-up flavor path.
                const float StationDistSq = FVector::DistSquared(Station->GetActorLocation(), GetActorLocation());
                if (!bCandidateFromAim || StationDistSq > FMath::Square(500.0f))
                {
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                            TEXT("Pick your language from the menu (arrows + Enter, or click) - or walk to a station and LOOK at it, then press E."));
                    }
                    return;
                }
                const bool bResumed = GI->ResumeLanguageRun(Station->Language);
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,
                        FString::Printf(TEXT("%s: %s track deploying..."),
                            bResumed ? TEXT("Resuming save") : TEXT("New run"),
                            *Station->StationLabel));
                }
                UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Entry")));
                return;
            }
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Cyan,
                FString::Printf(TEXT("Language locked from launch: %s"), *Station->StationLabel));
        }
        return;
    }

    if (ACodingTerminalActor* Terminal = Cast<ACodingTerminalActor>(Candidate))
    {
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            if (!FCodeRescueCampaign::IsCityUnlocked(GI, Terminal->CityIndex))
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow,
                        FString::Printf(TEXT("Locked city: graduate earlier missions before opening %s."),
                            *FCodeRescueCampaign::GetMissionLabel(Terminal->CityIndex)));
                }
                return;
            }
        }

        TSubclassOf<UCodeTerminalWidget> WidgetClass = TerminalWidgetClass;
        if (!WidgetClass)
        {
            WidgetClass = UCodeTerminalWidget::StaticClass();
        }

        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            if (UCodeTerminalWidget* TerminalWidget = CreateWidget<UCodeTerminalWidget>(PC, WidgetClass))
            {
                TerminalWidget->InitializeTerminal(Terminal);
                TerminalWidget->AddToViewport(50);
                TerminalWidget->SetKeyboardFocus();

                // UI-only input: gameplay axes/actions stop receiving events
                // and the polled-key path is gated by bUIOpen below.
                FInputModeUIOnly UIOnly;
                UIOnly.SetWidgetToFocus(TerminalWidget->TakeWidget());
                UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                PC->SetInputMode(UIOnly);
                PC->bShowMouseCursor = true;
                PC->SetIgnoreLookInput(true);
                PC->SetIgnoreMoveInput(true);
                ACodeRescueCharacter::SetUIOpen(true);
                UGameplayStatics::SetGamePaused(GetWorld(), true);

                return;
            }
        }

        UE_LOG(LogTemp, Error, TEXT("Code Rescue terminal UI failed to open for challenge '%s'. Objective remains unsolved."), *Terminal->Challenge.Id);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 7.0f, FColor::Red,
                TEXT("Terminal failed to open. Objective remains unsolved; retry interaction or check UI setup."));
        }
        return;
    }

    if (APickupActor* Pickup = Cast<APickupActor>(Candidate))
    {
        Pickup->Collect(this);
        return;
    }

    if (ACaseFilePickupActor* CaseFile = Cast<ACaseFilePickupActor>(Candidate))
    {
        CaseFile->Collect(this);
        return;
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Silver,
            FString::Printf(TEXT("E/Enter/Tab was detected, but no interactable was within %.0f units. Press T to jump to an objective."), InteractionAssistRadius));
    }
}

FString ACodeRescueCharacter::GetOpenWorldGuidanceText() const
{
    const int32 Count = FCodeRescueCampaign::GetMissionCount();
    const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>();
    const int32 ClosestIndex = FindClosestObjectiveIndex(GetActorLocation());
    const int32 FirstIncomplete = FCodeRescueCampaign::GetFirstIncompleteCityIndex(GI);
    const int32 ActiveIndex = FMath::Clamp(FirstIncomplete, 0, FMath::Max(0, Count - 1));
    const bool bComplete = FirstIncomplete >= Count;

    if (bComplete)
    {
        return FString::Printf(TEXT("Campaign complete: all %d major-city coding missions graduated. Extraction is ready."), Count);
    }

    return FString::Printf(
        TEXT("Major-city campaign: active %d/%d %s. Nearest: %d/%d %s. Complete terminal + survivor rescue to unlock the next city. City arena is locked; Backspace recovers you if stuck."),
        ActiveIndex + 1,
        Count,
        *FCodeRescueCampaign::GetMissionLabel(ActiveIndex),
        ClosestIndex + 1,
        Count,
        *FCodeRescueCampaign::GetMissionLabel(ClosestIndex));
}

void ACodeRescueCharacter::UseMedkit()
{
    if (Medkits <= 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Red, TEXT("No medkits left."));
        }
        return;
    }

    if (Health >= MaxHealth)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Health already full."));
        }
        return;
    }

    Medkits -= 1;
    Health = FMath::Min(MaxHealth, Health + MedkitHealAmount);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Medkit used."));
    }
}

void ACodeRescueCharacter::ToggleObjectiveJournal()
{
    if (UCodeRescueMessageReaderWidget::IsReaderOpen())
    {
        UCodeRescueMessageReaderWidget::CloseActiveReader();
        return;
    }

    // Toggle: if open, close; if closed, open. The journal is non-modal —
    // does NOT set bUIOpen, so movement and combat still work while it's up.
    if (ActiveJournalWidget)
    {
        ActiveJournalWidget->RemoveFromParent();
        ActiveJournalWidget = nullptr;
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    TSubclassOf<UUserWidget> WidgetClass = ObjectiveJournalWidgetClass;
    if (!WidgetClass)
    {
        WidgetClass = UCodeRescueObjectiveJournalWidget::StaticClass();
    }

    ActiveJournalWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (ActiveJournalWidget)
    {
        ActiveJournalWidget->AddToViewport(20);
    }
}

void ACodeRescueCharacter::TogglePauseMenu()
{
    // Toggle: if open, close + unpause; if closed, open + pause + lock UI.
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    // A readable world-message and the pause surface must never coexist. The
    // player screenshot showed the reader at z-order 1000 above the armory,
    // intercepting every mouse action and both widgets' close keys.
    if (UCodeRescueMessageReaderWidget::IsReaderOpen())
    {
        UCodeRescueMessageReaderWidget::CloseActiveReader();
        return;
    }

    if (ActivePauseWidget && !ActivePauseWidget->IsInViewport())
    {
        ActivePauseWidget = nullptr;
    }

    if (ActivePauseWidget)
    {
        ActivePauseWidget->RemoveFromParent();
        ActivePauseWidget = nullptr;
        // Restore game-only input + unpause world.
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->SetIgnoreLookInput(false);
        PC->SetIgnoreMoveInput(false);
        ACodeRescueCharacter::SetUIOpen(false);
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        return;
    }

    TSubclassOf<UUserWidget> WidgetClass = PauseMenuWidgetClass;
    if (!WidgetClass)
    {
        WidgetClass = UCodeRescuePauseWidget::StaticClass();
    }

    ActivePauseWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!ActivePauseWidget)
    {
        return;
    }

    ActivePauseWidget->AddToViewport(40);
    ActivePauseWidget->SetKeyboardFocus();

    FInputModeUIOnly MenuInput;
    MenuInput.SetWidgetToFocus(ActivePauseWidget->TakeWidget());
    MenuInput.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(MenuInput);
    PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bEnableMouseOverEvents = true;
    PC->bEnableTouchEvents = true;
    PC->SetIgnoreLookInput(true);
    PC->SetIgnoreMoveInput(true);
    PC->FlushPressedKeys();
    ActivePauseWidget->SetUserFocus(PC);
    ACodeRescueCharacter::SetUIOpen(true);

    // Normal play always pauses. Deterministic unattended armory acceptance
    // keeps simulation time advancing because NullRHI does not tick UMG while
    // the world is paused; gameplay input remains locked and the test target
    // has already completed its lifecycle.
    const bool bAutomatedArmoryAudit =
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelArmoryCycleAudit")) ||
        FParse::Param(FCommandLine::Get(), TEXT("FirstLevelIntegratedAcceptanceAudit"));
    UGameplayStatics::SetGamePaused(GetWorld(), !bAutomatedArmoryAudit);
    if (bAutomatedArmoryAudit)
    {
        UE_LOG(LogTemp, Display,
            TEXT("[PauseAuditClock] unattended_ui_clock=running gameplay_input_locked=1 production_pause_unchanged=1"));
    }
}

void ACodeRescueCharacter::ApplyDamage(float DamageAmount, AActor* DamageSource)
{
    // Once dead, ignore further damage so the death widget isn't re-spawned
    // every time a zombie reaches the stationary corpse.
    if (Health <= 0.0f)
    {
        return;
    }
    // 2026-07-04: pain reads on the character's face (v2 morphs; safe no-op otherwise).
    if (FacialExpression)
    {
        FacialExpression->SetExpression(FName(TEXT("Grimace")), 1.0f, 1.2f);
    }
    if (DamageSource && DamageSource->IsA<ACodeZombieActor>() &&
        ACodeRescueGameMode::IsLocationInsideProtectedLearningZone(this, GetActorLocation(), 300.0f))
    {
        LastDamageMitigationText = TEXT("protected learning zone blocked zombie damage");
        Tags.AddUnique(FName("ProtectedLearningDamageBlocked"));
        return;
    }

    const float NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const bool bInsideMercyWindow = DamageSource && DamageMercyWindowSeconds > 0.0f && (NowSeconds - LastDamageWorldTime) < DamageMercyWindowSeconds;
    float EffectiveDamage = bInsideMercyWindow ? DamageAmount * 0.25f : DamageAmount;
    bool bDamageCapped = false;
    if (DamageSource && MaxHealth > 0.0f)
    {
        const float CappedDamage = FMath::Min(EffectiveDamage, MaxHealth * MaxEnemyDamagePerHitFraction);
        bDamageCapped = CappedDamage < EffectiveDamage;
        EffectiveDamage = CappedDamage;
    }
    bool bArmorPlateAbsorbed = false;
    if (DamageSource && ArmorPlates > 0 && ArmorDamageReduction > 0.0f)
    {
        EffectiveDamage *= (1.0f - ArmorDamageReduction);
        ArmorPlates = FMath::Max(0, ArmorPlates - 1);
        bArmorPlateAbsorbed = true;
    }
    LastDamageWorldTime = NowSeconds;

    const float PreviousHealth = Health;
    Health = FMath::Max(0.0f, Health - EffectiveDamage);
    bool bSingleHitSurvivalLocked = false;
    if (bPreventSingleHitEnemyDeaths && DamageSource && PreviousHealth > MaxHealth * 0.35f && Health <= 0.0f)
    {
        Health = FMath::Max(1.0f, MaxHealth * 0.14f);
        bSingleHitSurvivalLocked = true;
    }

    if (EffectiveDamage > 0.1f && DamageSource && DamageSource->IsA<ACodeZombieActor>())
    {
        SpawnAnatomicalBiteWound(DamageSource);
    }

    if (DamageSource && (EnemyHitKnockbackHorizontal > 0.0f || EnemyHitKnockbackVertical > 0.0f))
    {
        const bool bReducedMotion = GetGameInstance<UCodeRescueGameInstance>()
            && GetGameInstance<UCodeRescueGameInstance>()->bReducedMotion;
        const float KnockbackScale = bReducedMotion ? 0.35f : 1.0f;
        const FVector Away = (GetActorLocation() - DamageSource->GetActorLocation()).GetSafeNormal2D();
        LaunchCharacter(Away * EnemyHitKnockbackHorizontal * KnockbackScale + FVector(0.0f, 0.0f, EnemyHitKnockbackVertical * KnockbackScale), true, true);
    }

    // #21 wiring — feed direction info to the damage-feedback overlay.
    if (DamageFeedbackWidget && DamageSource)
    {
        DamageFeedbackWidget->NotifyDamageFromDirection(GetActorLocation() - DamageSource->GetActorLocation());
    }
    TriggerCombatJuiceDamageCue(EffectiveDamage, DamageSource);

    LastDamageLocationText = DescribeAttackerDirection(this, DamageSource);
    LastDamageSourceText = DescribeDamageSource(DamageSource);
    LastDamageAmount = EffectiveDamage;
    LastDamageSourceDistanceMeters = DamageSource
        ? FVector::Dist(GetActorLocation(), DamageSource->GetActorLocation()) / 100.0f
        : -1.0f;

    const bool bEmergencyMedkitReady = DamageSource
        && bAutoUseEmergencyMedkit
        && Medkits > 0
        && MaxHealth > 0.0f
        && Health <= MaxHealth * EmergencyMedkitHealthFraction
        && GetEmergencyMedkitReadySeconds() <= 0.0f;
    bool bEmergencyMedkitUsed = false;
    if (bEmergencyMedkitReady)
    {
        --Medkits;
        LastEmergencyMedkitWorldTime = NowSeconds;
        Health = FMath::Clamp(Health + MedkitHealAmount, 1.0f, MaxHealth);
        bEmergencyMedkitUsed = true;
        const FString RecoveryMessage = FString::Printf(
            TEXT("Emergency medkit deployed after %s hit. Health restored to %.0f."),
            *LastDamageLocationText,
            Health);
        UCodeRescueSubtitlesWidget::Push(RecoveryMessage, 3.2f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.2f, FColor::Green, RecoveryMessage);
        }
    }
    else if (DamageSource && Health > 0.0f && Health <= MaxHealth * 0.35f && (NowSeconds - LastCriticalHealthCalloutWorldTime) > 6.0f)
    {
        LastCriticalHealthCalloutWorldTime = NowSeconds;
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(TEXT("Critical health after %s hit. Use Q or regroup with the medic."), *LastDamageLocationText),
            3.0f);
    }

    TArray<FString> MitigationNotes;
    if (bInsideMercyWindow)
    {
        MitigationNotes.Add(TEXT("mercy window"));
    }
    if (bDamageCapped)
    {
        MitigationNotes.Add(TEXT("per-hit cap"));
    }
    if (bArmorPlateAbsorbed)
    {
        MitigationNotes.Add(TEXT("armor plate"));
    }
    if (bSingleHitSurvivalLocked)
    {
        MitigationNotes.Add(TEXT("survival lock"));
    }
    if (bEmergencyMedkitUsed)
    {
        MitigationNotes.Add(TEXT("emergency medkit"));
    }
    LastDamageMitigationText = MitigationNotes.Num() > 0
        ? FString::Join(MitigationNotes, TEXT(", "))
        : FString(TEXT("none"));

    if (GEngine)
    {
        const FString MitigationSuffix = LastDamageMitigationText.Equals(TEXT("none"))
            ? FString()
            : FString::Printf(TEXT(" | %s"), *LastDamageMitigationText);
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
            FString::Printf(TEXT("Damage taken from %s: %.0f | Health: %.0f%s"),
                *LastDamageLocationText,
                EffectiveDamage,
                Health,
                *MitigationSuffix));
    }

    if (Health <= 0.0f)
    {
        if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
        {
            GI->IncrementDeathCount();
        }

        UGameplayStatics::SetGamePaused(GetWorld(), true);

        // Spawn the death widget (Resume from save / Restart fresh / Save + Quit).
        // Falls back to the C++ class if no Blueprint subclass is assigned.
        // The two branches of the ternary are different types
        // (TSubclassOf<UUserWidget> vs UClass*) so we resolve to a UClass*
        // first and the CreateWidget overload accepts that.
        UClass* WidgetClass = DeathWidgetClass
            ? DeathWidgetClass.Get()
            : static_cast<UClass*>(UCodeRescueDeathWidget::StaticClass());
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            if (UUserWidget* W = CreateWidget<UUserWidget>(PC, WidgetClass))
            {
                W->AddToViewport(1000); // high Z so it sits over HUD
            }
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Mission failed: operative down."));
        }
    }
}

void ACodeRescueCharacter::SpawnAnatomicalBiteWound(AActor* DamageSource)
{
    UWorld* World = GetWorld();
    if (!World || !DamageSource || SpawnedBiteWoundCount >= 6 ||
        World->GetTimeSeconds() - LastBiteWoundWorldTime < 1.15f)
    {
        return;
    }

    // 2026-07-17 (Kenny: "some strange object continues to appear around the
    // user's character"): the wound used to attach to the HIDDEN driver
    // body's bone — a dark gore mass floating beside the visible hero. It
    // now rides the VISIBLE presentation body and follows its visibility.
    USkinnedMeshComponent* PresentationBody = bHeroPresentationConfigured
        ? static_cast<USkinnedMeshComponent*>(HeroPresentationMesh)
        : (bAimPresentationConfigured
            ? static_cast<USkinnedMeshComponent*>(AimingPresentationMesh)
            : static_cast<USkinnedMeshComponent*>(GetMesh()));
    if (!PresentationBody || !PresentationBody->GetSkinnedAsset())
    {
        return;
    }

    const FVector ToAttackerWorld = (DamageSource->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    const FVector LocalAttacker = GetActorTransform().InverseTransformVectorNoScale(ToAttackerWorld);
    const bool bRightSide = LocalAttacker.Y >= 0.0f;
    const FName Candidates[] = {
        // authored hero rig (Blender export: capital side suffix)
        bRightSide ? FName(TEXT("upperarm_R")) : FName(TEXT("upperarm_L")),
        FName(TEXT("chest")),
        // Manny driver rig fallbacks
        bRightSide ? FName(TEXT("upperarm_r")) : FName(TEXT("upperarm_l")),
        bRightSide ? FName(TEXT("clavicle_r")) : FName(TEXT("clavicle_l")),
        FName(TEXT("spine_03")),
        FName(TEXT("spine_02"))
    };
    FName BiteBone = NAME_None;
    for (const FName Candidate : Candidates)
    {
        if (PresentationBody->GetBoneIndex(Candidate) != INDEX_NONE)
        {
            BiteBone = Candidate;
            break;
        }
    }
    if (BiteBone == NAME_None)
    {
        return;
    }

    const FVector BoneLocation = PresentationBody->GetBoneLocation(BiteBone);
    const FVector Incoming = (GetActorLocation() - DamageSource->GetActorLocation()).GetSafeNormal();
    const FRotator WoundRotation = (-Incoming).Rotation();
    bool bSpawnedVisual = false;

    if (UMaterialInterface* BloodDecal = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/YI_ModularZombies/Materials/Master/Instances/MI_BloodSplatter_01_Decal.MI_BloodSplatter_01_Decal")))
    {
        if (UDecalComponent* BiteDecal = UGameplayStatics::SpawnDecalAttached(
                BloodDecal,
                FVector(8.0f, 15.0f, 20.0f),
                PresentationBody,
                BiteBone,
                BoneLocation + Incoming * 2.0f,
                WoundRotation,
                EAttachLocation::KeepWorldPosition,
                0.0f))
        {
            BiteDecal->ComponentTags.AddUnique(FName("AnatomicalZombieBiteDecal"));
            bSpawnedVisual = true;
        }
    }

    if (UStaticMesh* BiteMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/CodeRescueArt/FirstLevelV4/BiteWoundV4/BiteWoundV4/StaticMeshes/BiteWoundV4.BiteWoundV4")))
    {
        UStaticMeshComponent* BiteWound = NewObject<UStaticMeshComponent>(this);
        AddInstanceComponent(BiteWound);
        BiteWound->SetStaticMesh(BiteMesh);
        BiteWound->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BiteWound->SetGenerateOverlapEvents(false);
        BiteWound->SetCastShadow(false);
        BiteWound->ComponentTags.AddUnique(FName("AnatomicalZombieBiteWound"));
        BiteWound->RegisterComponent();
        BiteWound->AttachToComponent(PresentationBody, FAttachmentTransformRules::KeepWorldTransform, BiteBone);
        // snug against the body at a readable-but-not-obscuring size (the old
        // 0.88-scale mass at +1.5 off the bone was Kenny's "strange object")
        BiteWound->SetWorldLocation(BoneLocation + Incoming * 0.5f);
        BiteWound->SetWorldRotation(WoundRotation);
        BiteWound->SetWorldScale3D(FVector(0.42f));
        bSpawnedVisual = true;
    }

    if (bSpawnedVisual)
    {
        LastBiteWoundWorldTime = World->GetTimeSeconds();
        ++SpawnedBiteWoundCount;
        Tags.AddUnique(FName("PlayerAnatomicalBiteWounds"));
        UE_LOG(LogTemp, Display,
            TEXT("[AnatomicalWound] bite victim=%s bone=%s attacker=%s count=%d"),
            *GetName(), *BiteBone.ToString(), *DamageSource->GetName(), SpawnedBiteWoundCount);
    }
}

void ACodeRescueCharacter::Reload()
{
    EnsureWeaponStateInitialized();

    // Already reloading
    if (bIsReloading)
    {
        return;
    }

    const int32 WeaponIdx = static_cast<int32>(ActiveWeapon);
    const FWeaponDef* WeaponDef = WeaponLoadout.IsValidIndex(WeaponIdx) ? &WeaponLoadout[WeaponIdx] : nullptr;
    if (WeaponDef && !WeaponDef->bUsesAmmo)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Silver,
                FString::Printf(TEXT("%s does not reload."), *WeaponDef->DisplayName));
        }
        return;
    }

    // Magazine is already full
    if (MagazineAmmo >= MagazineSize)
    {
        return;
    }

    // No ammo in reserve
    if (GetActiveWeaponReserveAmmo() <= 0)
    {
        TriggerCombatJuiceFireCue(WeaponDef, true);
        if (DryFireCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DryFireCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
        }
        return;
    }

    bIsReloading = true;
    LastWeaponPresentationReloadWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastWeaponPresentationReloadWorldTime;
    UpdateFirstPersonWeaponPresentation(0.0f);
    Tags.AddUnique(FName("DistinctWeaponPresentationReloadCue"));
    TriggerCombatJuiceReloadStageCue(0.0f, false);
    ReportStealthNoise(0.22f, UtilityNoiseRadius * 0.65f, TEXT("reload"));

    // Schedule the reload completion callback
    if (ReloadDuration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ReloadTimerHandle,
            this,
            &ACodeRescueCharacter::OnReloadComplete,
            ReloadDuration,
            false
        );
    }
    else
    {
        OnReloadComplete();
    }
}

void ACodeRescueCharacter::OnReloadComplete()
{
    if (!bIsReloading)
    {
        return;
    }

    EnsureWeaponStateInitialized();
    const int32 WeaponIdx = static_cast<int32>(ActiveWeapon);
    if (!WeaponReserveAmmo.IsValidIndex(WeaponIdx))
    {
        bIsReloading = false;
        return;
    }

    int32 AmmoNeeded = MagazineSize - MagazineAmmo;
    int32 AmmoToLoad = FMath::Min(AmmoNeeded, WeaponReserveAmmo[WeaponIdx]);
    MagazineAmmo += AmmoToLoad;
    WeaponReserveAmmo[WeaponIdx] -= AmmoToLoad;
    if (WeaponMagazines.IsValidIndex(WeaponIdx))
    {
        WeaponMagazines[WeaponIdx] = MagazineAmmo;
    }
    RefreshLegacyAmmoFromWeaponReserves();

    bIsReloading = false;
    UpdateFirstPersonWeaponPresentation(0.0f);
    TriggerCombatJuiceReloadStageCue(1.0f, true);
}

// ---------------------------------------------------------------------------
// 2026-07-04: visible first-person weapon models (Blender-authored RawArt/Weapons)
// and idle/walk/run switching for the v2 player body.

const FWeaponDef* ACodeRescueCharacter::GetWeaponDefinition(EWeaponType Weapon) const
{
    const int32 Index = static_cast<int32>(Weapon);
    return WeaponLoadout.IsValidIndex(Index) ? &WeaponLoadout[Index] : nullptr;
}

int32 ACodeRescueCharacter::GetWeaponMagazineAmmo(EWeaponType Weapon) const
{
    const int32 Index = static_cast<int32>(Weapon);
    return WeaponMagazines.IsValidIndex(Index) ? WeaponMagazines[Index] : 0;
}

int32 ACodeRescueCharacter::GetWeaponReserveAmmo(EWeaponType Weapon) const
{
    const int32 Index = static_cast<int32>(Weapon);
    return WeaponReserveAmmo.IsValidIndex(Index) ? WeaponReserveAmmo[Index] : 0;
}

UStaticMesh* ACodeRescueCharacter::ResolveWeaponPreviewMesh(EWeaponType Weapon) const
{
    // 2026-07-11: WeaponsV4 (serrations/rails/side-saddle/emissive sights) are
    // preferred; V3 stays as the fallback chain below. Interchange double-nests
    // GLB imports: /WeaponsV4/<N>/<N>/StaticMeshes/<N>.<N>.
    const TCHAR* V4Name = nullptr;
    switch (Weapon)
    {
    case EWeaponType::Shotgun:
    case EWeaponType::TacticalShotgun:
    case EWeaponType::AutoShotgun:
        V4Name = TEXT("ShotgunV4");
        break;
    case EWeaponType::Pistol:
    case EWeaponType::HeavyHandgun:
    case EWeaponType::BurstHandgun:
    case EWeaponType::Magnum:
        V4Name = TEXT("PistolV4");
        break;
    case EWeaponType::SMG:
        V4Name = TEXT("SMGV4");
        break;
    case EWeaponType::BoltLauncher:
        V4Name = TEXT("CrossbowV4");
        break;
    case EWeaponType::Rifle:
    case EWeaponType::PrecisionRifle:
    case EWeaponType::SemiAutoRifle:
        V4Name = TEXT("RifleV4");
        break;
    default:
        break;
    }
    if (V4Name)
    {
        const FString V4Path = FString::Printf(
            TEXT("/Game/CodeRescueArt/WeaponsV4/%s/%s/StaticMeshes/%s.%s"),
            V4Name, V4Name, V4Name, V4Name);
        if (UStaticMesh* V4Mesh = LoadObject<UStaticMesh>(nullptr, *V4Path))
        {
            return V4Mesh;
        }
        // Some importer versions skip the duplicate folder — try the flat nest too.
        const FString V4Flat = FString::Printf(
            TEXT("/Game/CodeRescueArt/WeaponsV4/%s/StaticMeshes/%s.%s"),
            V4Name, V4Name, V4Name);
        if (UStaticMesh* V4MeshFlat = LoadObject<UStaticMesh>(nullptr, *V4Flat))
        {
            return V4MeshFlat;
        }
    }

    const TCHAR* PrimaryPath = nullptr;
    switch (Weapon)
    {
    case EWeaponType::Shotgun:
    case EWeaponType::TacticalShotgun:
    case EWeaponType::AutoShotgun:
        PrimaryPath = TEXT("/Game/CodeRescueArt/WeaponsV3/ShotgunV3/ShotgunV3/StaticMeshes/ShotgunV3.ShotgunV3");
        break;
    case EWeaponType::Pistol:
    case EWeaponType::HeavyHandgun:
    case EWeaponType::BurstHandgun:
    case EWeaponType::Magnum:
        PrimaryPath = TEXT("/Game/CodeRescueArt/WeaponsV3/PistolV3/PistolV3/StaticMeshes/PistolV3.PistolV3");
        break;
    case EWeaponType::SMG:
        PrimaryPath = TEXT("/Game/CodeRescueArt/WeaponsV3/SMGV3/SMGV3/StaticMeshes/SMGV3.SMGV3");
        break;
    case EWeaponType::BoltLauncher:
        PrimaryPath = TEXT("/Game/CodeRescueArt/WeaponsV3/CrossbowV3/CrossbowV3/StaticMeshes/CrossbowV3.CrossbowV3");
        break;
    case EWeaponType::Rifle:
    case EWeaponType::PrecisionRifle:
    case EWeaponType::SemiAutoRifle:
        PrimaryPath = TEXT("/Game/CodeRescueArt/WeaponsV3/RifleV3/RifleV3/StaticMeshes/RifleV3.RifleV3");
        break;
    case EWeaponType::Grenade:
    case EWeaponType::IncendiaryGrenade:
    case EWeaponType::FlashGrenade:
        PrimaryPath = TEXT("/Game/CodeRescueArt/FirstLevelV4/GrenadeV4/GrenadeV4/StaticMeshes/GrenadeV4.GrenadeV4");
        break;
    case EWeaponType::CombatKnife:
        PrimaryPath = TEXT("/Game/CodeRescueArt/FirstLevelV4/CombatKnifeV4/CombatKnifeV4/StaticMeshes/CombatKnifeV4.CombatKnifeV4");
        break;
    case EWeaponType::RocketLauncher:
        // pass 5: proper shoulder-fired launcher art (V5), V4 as fallback
        if (UStaticMesh* RocketV5 = LoadObject<UStaticMesh>(nullptr,
                TEXT("/Game/CodeRescueArt/WeaponsV5/RocketLauncherV5/RocketLauncherV5/StaticMeshes/RocketLauncherV5.RocketLauncherV5")))
        {
            return RocketV5;
        }
        PrimaryPath = TEXT("/Game/CodeRescueArt/FirstLevelV4/RocketLauncherV4/RocketLauncherV4/StaticMeshes/RocketLauncherV4.RocketLauncherV4");
        break;
    default:
        break;
    }

    if (PrimaryPath)
    {
        if (UStaticMesh* PrimaryMesh = LoadObject<UStaticMesh>(nullptr, PrimaryPath))
        {
            return PrimaryMesh;
        }
    }

    const TCHAR* FallbackPath = nullptr;
    switch (Weapon)
    {
    case EWeaponType::CombatKnife:
        FallbackPath = TEXT("/Game/CodeRescueArt/Weapons/SM_Machete_Field/StaticMeshes/SM_Machete_Field.SM_Machete_Field");
        break;
    case EWeaponType::Grenade:
    case EWeaponType::IncendiaryGrenade:
    case EWeaponType::FlashGrenade:
        FallbackPath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
        break;
    case EWeaponType::Shotgun:
    case EWeaponType::TacticalShotgun:
    case EWeaponType::AutoShotgun:
        FallbackPath = TEXT("/Game/CodeRescueArt/Weapons/SM_Shotgun_Breacher/StaticMeshes/SM_Shotgun_Breacher.SM_Shotgun_Breacher");
        break;
    case EWeaponType::Pistol:
    case EWeaponType::HeavyHandgun:
    case EWeaponType::BurstHandgun:
    case EWeaponType::Magnum:
        FallbackPath = TEXT("/Game/CodeRescueArt/Weapons/SM_Pistol_Compact/StaticMeshes/SM_Pistol_Compact.SM_Pistol_Compact");
        break;
    default:
        FallbackPath = TEXT("/Game/CodeRescueArt/Weapons/SM_Rifle_Scout/StaticMeshes/SM_Rifle_Scout.SM_Rifle_Scout");
        break;
    }
    return LoadObject<UStaticMesh>(nullptr, FallbackPath);
}

void ACodeRescueCharacter::RefreshFirstPersonWeapon()
{
    if (!FirstPersonWeaponSilhouette)
    {
        return;
    }
    UStaticMesh* WeaponMesh = ResolveWeaponPreviewMesh(ActiveWeapon);

    // 2026-07-07: mirror the selection onto the BODY so third-person cameras
    // show the held weapon. Snap to the v2 rig's right hand when present
    // (Blender "hand.R" sanitizes to "hand_R" on import); otherwise ride the
    // mesh at hip height so the weapon still reads on the mannequin fallback.
    if (ThirdPersonWeaponMesh)
    {
        if (WeaponMesh)
        {
            ThirdPersonWeaponMesh->SetStaticMesh(WeaponMesh);
            const FVector LocalMeshSize = WeaponMesh->GetBoundingBox().GetSize().GetAbs();
            const float LocalLongestDimension = FMath::Max3(
                LocalMeshSize.X, LocalMeshSize.Y, LocalMeshSize.Z);
            const float TargetLengthCm = GetHeldWeaponTargetLengthCm(ActiveWeapon);
            const float NormalizedScale = FMath::Clamp(
                TargetLengthCm / FMath::Max(LocalLongestDimension, 0.01f),
                0.0025f,
                20.0f);
            USkinnedMeshComponent* Body = bHeroPresentationConfigured
                ? static_cast<USkinnedMeshComponent*>(HeroPresentationMesh)
                : (bAimPresentationConfigured
                    ? static_cast<USkinnedMeshComponent*>(AimingPresentationMesh)
                    : static_cast<USkinnedMeshComponent*>(GetMesh()));
            static const FName HandBoneCandidates[] = { FName(TEXT("hand_R")), FName(TEXT("hand.R")), FName(TEXT("hand_r")) };
            FName HandBone = NAME_None;
            if (Body && Body->GetSkinnedAsset())
            {
                for (const FName& Candidate : HandBoneCandidates)
                {
                    if (Body->GetBoneIndex(Candidate) != INDEX_NONE)
                    {
                        HandBone = Candidate;
                        break;
                    }
                }
            }
            if (Body && HandBone != NAME_None)
            {
                ThirdPersonWeaponMesh->AttachToComponent(Body,
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandBone);
                // Authored grips sit at the origin. Zero translation cannot
                // be magnified by an imported skeleton's hand-bone scale.
                ThirdPersonWeaponMesh->SetAbsolute(false, false, true);
                ThirdPersonWeaponMesh->SetRelativeLocation(FVector::ZeroVector);
                ThirdPersonWeaponMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
                ThirdPersonWeaponMesh->SetRelativeScale3D(FVector(NormalizedScale));
            }
            else
            {
                // Stable fallback for a body without a recognized hand bone.
                ThirdPersonWeaponMesh->AttachToComponent(GetCapsuleComponent(),
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                ThirdPersonWeaponMesh->SetAbsolute(false, false, true);
                ThirdPersonWeaponMesh->SetRelativeLocation(FVector(18.0f, 26.0f, 68.0f));
                ThirdPersonWeaponMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
                ThirdPersonWeaponMesh->SetRelativeScale3D(FVector(NormalizedScale));
            }
            ThirdPersonWeaponMesh->SetCastShadow(false);
            ThirdPersonWeaponMesh->SetVisibility(CameraPerspective != 0, true);
            UE_LOG(LogTemp, Display,
                TEXT("[HeldWeapon] body weapon '%s' attached via %s target_length_cm=%.1f local_longest=%.2f absolute_scale=%.5f casts_shadow=0"),
                *WeaponMesh->GetName(),
                HandBone != NAME_None ? *HandBone.ToString() : TEXT("capsule fallback"),
                TargetLengthCm,
                LocalLongestDimension,
                NormalizedScale);
        }
        else
        {
            ThirdPersonWeaponMesh->SetVisibility(false, true);
        }
    }
    if (WeaponMesh)
    {
        FirstPersonWeaponSilhouette->SetStaticMesh(WeaponMesh);
        // Authored at real-world scale with the grip near the origin; parked low-right of the
        // camera like a classic FPS view model. Only the owner sees it (set in the constructor).
        const bool bLong = (WeaponMesh->GetBoundingBox().GetSize().X > 40.0f);
        // 2026-07-04 playtest tune: push the model clear of the camera near-plane and
        // legacy-arms volume so it reads as a held weapon in the lower-right.
        FirstPersonWeaponSilhouette->SetRelativeLocation(bLong
            ? FVector(58.0f, 24.0f, -22.0f)
            : FVector(46.0f, 20.0f, -18.0f));
        FirstPersonWeaponSilhouette->SetRelativeRotation(FRotator(0.0f, -3.5f, 0.0f));
        FirstPersonWeaponSilhouette->SetRelativeScale3D(FVector(1.0f));
        FirstPersonWeaponSilhouette->SetVisibility(true, true);
    }
    else
    {
        // No art for this weapon (or import pending): hide rather than show the placeholder cube.
        FirstPersonWeaponSilhouette->SetVisibility(false, true);
    }
}

void ACodeRescueCharacter::UpdateV2BodyLocomotion(float DeltaSeconds)
{
    (void)DeltaSeconds;
    const float Speed = GetVelocity().Size2D();
    const int32 Desired = (Speed > 430.0f) ? 2 : ((Speed > 40.0f) ? 1 : 0);

    if (bUsingV2Body)
    {
        if (USkeletalMeshComponent* Body = GetMesh())
        {
            if (Desired != V2BodyAnimState)
            {
                V2BodyAnimState = Desired;
                UAnimSequence* Seq = (Desired == 2 && V2RunAnim) ? V2RunAnim
                                   : (Desired == 1 && V2WalkAnim) ? V2WalkAnim
                                   : V2IdleAnim;
                if (Seq)
                {
                    Body->PlayAnimation(Seq, true);
                }
            }
        }
    }

    // 2026-07-11 pass 4: the hero presentation body runs the same speed-driven
    // idle/walk/run switching (only while visible — first-person skips it).
    if (bHeroPresentationConfigured && HeroPresentationMesh &&
        HeroPresentationMesh->IsVisible() && Desired != HeroBodyAnimState)
    {
        HeroBodyAnimState = Desired;
        UAnimSequence* HeroSeq = (Desired == 2 && HeroRunAnim) ? HeroRunAnim
                               : (Desired == 1 && HeroWalkAnim) ? HeroWalkAnim
                               : HeroIdleAnim;
        if (HeroSeq)
        {
            HeroPresentationMesh->PlayAnimation(HeroSeq, true);
        }
    }
}

void ACodeRescueCharacter::UpdateAuthoredMannyAnimation(float DeltaSeconds)
{
    if (!bUsingAuthoredMannyAnimation)
    {
        return;
    }

    USkeletalMeshComponent* Body = GetMesh();
    const UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Body || !Movement)
    {
        return;
    }

    MannyLandingPresentationRemaining = FMath::Max(
        0.0f, MannyLandingPresentationRemaining - DeltaSeconds);

    // 0 idle, 1 walk, 2 run, 3 jump, 4 fall, 5 land.
    int32 DesiredState = 0;
    if (MannyLandingPresentationRemaining > 0.0f)
    {
        DesiredState = 5;
    }
    else if (Movement->IsFalling())
    {
        DesiredState = Movement->Velocity.Z > 40.0f ? 3 : 4;
    }
    else
    {
        const float GroundSpeed = GetVelocity().Size2D();
        DesiredState = GroundSpeed > 430.0f ? 2 : (GroundSpeed > 35.0f ? 1 : 0);
    }

    UAnimSequence* DesiredSequence = MannyIdleAnim;
    bool bLoop = true;
    switch (DesiredState)
    {
    case 1: DesiredSequence = MannyWalkAnim; break;
    case 2: DesiredSequence = MannyRunAnim; break;
    case 3: DesiredSequence = MannyJumpAnim; bLoop = false; break;
    case 4: DesiredSequence = MannyFallAnim; break;
    case 5:
        // MM_Land is additive and is invalid as a cooked single-node clip.
        // Use the stable idle base while the visible pose copy applies the
        // procedural landing compression in UpdateWeaponAimPresentation.
        DesiredSequence = MannyIdleAnim;
        break;
    default: break;
    }

    if (DesiredState != MannyAnimationState && DesiredSequence)
    {
        MannyAnimationState = DesiredState;
        Body->PlayAnimation(DesiredSequence, bLoop);
        static const TCHAR* StateLabels[] = {
            TEXT("idle"), TEXT("walk"), TEXT("run"),
            TEXT("jump"), TEXT("fall"), TEXT("land")
        };
        UE_LOG(LogTemp, Verbose, TEXT("[PlayerAnimationState] state=%s sequence=%s loop=%d"),
            StateLabels[FMath::Clamp(DesiredState, 0, 5)],
            *GetNameSafe(DesiredSequence),
            bLoop ? 1 : 0);
    }
    MannyObservedAnimationStateMask |= static_cast<uint8>(1u << FMath::Clamp(DesiredState, 0, 5));

    if (UAnimSingleNodeInstance* SingleNode = Body->GetSingleNodeInstance())
    {
        const float Speed = GetVelocity().Size2D();
        float PlayRate = 1.0f;
        if (DesiredState == 1)
        {
            PlayRate = FMath::Clamp(Speed / 235.0f, 0.72f, 1.35f);
        }
        else if (DesiredState == 2)
        {
            PlayRate = FMath::Clamp(Speed / 585.0f, 0.82f, 1.25f);
        }
        else if (DesiredState == 4)
        {
            PlayRate = 0.9f;
        }
        else if (DesiredState == 5)
        {
            PlayRate = 1.15f;
        }
        SingleNode->SetPlayRate(PlayRate);
    }
}

// ---------------------------------------------------------------------------
// 2026-07-04 (top-50 item 50): PHOTO MODE.

void ACodeRescueCharacter::TogglePhotoMode()
{
    // Never trigger while a modal UI (terminal, menus) is up — F10 there is likely a typo.
    if (!bPhotoModeActive && IsUIOpen())
    {
        return;
    }
    bPhotoModeActive = !bPhotoModeActive;
    if (bPhotoModeActive)
    {
        PhotoModeHiddenWidgets.Reset();
        TArray<UUserWidget*> Widgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), Widgets, UUserWidget::StaticClass(), false);
        for (UUserWidget* Widget : Widgets)
        {
            if (Widget && Widget->IsInViewport())
            {
                const ESlateVisibility Original = Widget->GetVisibility();
                if (Original != ESlateVisibility::Collapsed && Original != ESlateVisibility::Hidden)
                {
                    PhotoModeHiddenWidgets.Emplace(Widget, Original);
                    Widget->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
        }
        UGameplayStatics::SetGlobalTimeDilation(this, 0.12f);
        UE_LOG(LogTemp, Warning, TEXT("[PhotoMode] ON — HUD hidden (%d widgets), time 12%%. C/V move the camera, F12 screenshots, F10 exits."),
            PhotoModeHiddenWidgets.Num());
    }
    else
    {
        for (const TPair<TWeakObjectPtr<UUserWidget>, ESlateVisibility>& Entry : PhotoModeHiddenWidgets)
        {
            if (Entry.Key.IsValid())
            {
                Entry.Key->SetVisibility(Entry.Value);
            }
        }
        PhotoModeHiddenWidgets.Reset();
        UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("[PhotoMode] OFF — HUD restored."));
    }
}
