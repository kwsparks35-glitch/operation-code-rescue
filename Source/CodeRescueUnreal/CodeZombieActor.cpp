#include "CodeZombieActor.h"
#include "BarricadeActor.h"
#include "Animation/AnimSequence.h"
#include "CodeRescueAnimationBudget.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescueRetargetRig.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueGameMode.h"
#include "CodeRescuePhysicsStability.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CodeRescueAIController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "HAL/IConsoleManager.h"

namespace
{
int32 GCodeRescueActiveRagdollCorpses = 0;
constexpr int32 CodeRescueMaxActiveRagdollCorpses = 10;

float GetWeatherVisibilityScale()
{
    static const IConsoleVariable* WeatherVisibility =
        IConsoleManager::Get().FindConsoleVariable(TEXT("cr.WeatherVisibilityScale"));
    if (WeatherVisibility)
    {
        return FMath::Clamp(WeatherVisibility->GetFloat(), 0.50f, 1.0f);
    }
    return 1.0f;
}

// 2026-07-11 art+physics v3 kill switch: authored CharactersV3 zombies keep
// their import-time physics asset (ragdoll + physical hit reactions). Set 0 to
// restore the legacy behavior (asset cleared, primitive/frozen corpses only).
static TAutoConsoleVariable<int32> CVarAuthoredBodyPhysics(
    TEXT("cr.AuthoredBodyPhysics"), 1,
    TEXT("1 = CharactersV3 authored zombies keep their physics asset (ragdoll + hit reactions). 0 = legacy v2 behavior."));

// Percent of ELIGIBLE variants (generic walkers + chargers) that wear authored
// CharactersV3 bodies instead of their pack mesh — brings the keyed
// attack/flinch/death anims, morph faces, emissive eyes, and authored-body
// ragdoll into the everyday horde rather than only the no-pack fallback.
static TAutoConsoleVariable<int32> CVarAuthoredZombieShare(
    TEXT("cr.AuthoredZombieShare"), 35,
    TEXT("0-100: percent of eligible zombies (BaseMesh/UrbanZombie4/BusinessSuit/EliteCharger) dressed in authored CharactersV3 bodies. 0 disables."));

namespace
{
// Legacy FBX importer names animation takes "<File>_Anim_<File>_<Action>"; two
// earlier import styles produced "<File>_<Action>" and "<File><File>_<Action>".
// Try all three so the code survives importer naming drift.
UAnimSequence* LoadAuthoredV3Anim(const TCHAR* CharName, const TCHAR* ActionName)
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

USkeletalMesh* LoadAuthoredV3Mesh(const TCHAR* CharName)
{
    return LoadObject<USkeletalMesh>(nullptr, *FString::Printf(
        TEXT("/Game/CodeRescueArt/CharactersV3/%s/%s.%s"), CharName, CharName, CharName));
}
}

float GetZombieRuntimeSfxVolume(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI ? GI->GetSfxVolumeScalar() : 1.0f;
}

bool IsRuntimeMonoAudioEnabled(const UObject* Context)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    const UCodeRescueGameInstance* GI = World ? World->GetGameInstance<UCodeRescueGameInstance>() : nullptr;
    return GI && GI->bMonoAudio;
}

FVector GetZombieMonoSafeSoundLocation(const UObject* Context, const FVector& RequestedLocation)
{
    UWorld* World = Context ? Context->GetWorld() : nullptr;
    if (!IsRuntimeMonoAudioEnabled(Context))
    {
        return RequestedLocation;
    }

    if (const AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        return PlayerActor->GetActorLocation();
    }
    return RequestedLocation;
}

#define GetRuntimeSfxVolume GetZombieRuntimeSfxVolume
#define GetMonoSafeSoundLocation GetZombieMonoSafeSoundLocation

FString ZombieThreatVariantLabel(EZombieVariant Variant)
{
    switch (Variant)
    {
    case EZombieVariant::DogZombie: return TEXT("dog infected");
    case EZombieVariant::UrbanZombie4: return TEXT("urban infected");
    case EZombieVariant::BusinessSuit: return TEXT("business infected");
    case EZombieVariant::BloatedFemale: return TEXT("bloated infected");
    case EZombieVariant::NurseFemale: return TEXT("nurse infected");
    case EZombieVariant::BaseMesh: return TEXT("base-mesh infected");
    case EZombieVariant::EliteSpitter: return TEXT("spitter");
    case EZombieVariant::EliteCharger: return TEXT("charger");
    case EZombieVariant::EliteBoomer: return TEXT("boomer");
    default: return TEXT("infected");
    }
}

FString ZombieThreatRolePrefix(ECodeRescueZombieEncounterRole Role)
{
    switch (Role)
    {
    case ECodeRescueZombieEncounterRole::Anchor: return TEXT("anchor ");
    case ECodeRescueZombieEncounterRole::Flanker: return TEXT("flanker ");
    case ECodeRescueZombieEncounterRole::Pressure: return TEXT("pressure ");
    case ECodeRescueZombieEncounterRole::Sentinel: return TEXT("sentinel ");
    default: return TEXT("");
    }
}

bool IsPawnInsideProtectedLearningZone(APawn* Pawn, float Expansion = 300.0f)
{
    return Pawn &&
        ACodeRescueGameMode::IsLocationInsideProtectedLearningZone(Pawn, Pawn->GetActorLocation(), Expansion);
}

FString DirectionLabelFromZombieToPlayer(const ACodeZombieActor* Zombie, const APawn* PlayerPawn)
{
    if (!Zombie || !PlayerPawn)
    {
        return TEXT("nearby");
    }

    FVector ToThreat = Zombie->GetActorLocation() - PlayerPawn->GetActorLocation();
    ToThreat.Z = 0.0f;
    if (ToThreat.SizeSquared() <= KINDA_SMALL_NUMBER)
    {
        return TEXT("here");
    }

    const FVector ThreatDir = ToThreat.GetSafeNormal();
    const float ForwardDot = FVector::DotProduct(PlayerPawn->GetActorForwardVector(), ThreatDir);
    const float RightDot = FVector::DotProduct(PlayerPawn->GetActorRightVector(), ThreatDir);

    if (ForwardDot > 0.72f) return TEXT("ahead");
    if (ForwardDot < -0.72f) return TEXT("behind");
    return RightDot >= 0.0f ? TEXT("right") : TEXT("left");
}

ABarricadeActor* FindBlockingBarricadeBetween(const ACodeZombieActor* Zombie, APawn* PlayerPawn, FHitResult& OutHit)
{
    if (!Zombie || !PlayerPawn || !Zombie->GetWorld())
    {
        return nullptr;
    }

    const FVector Start = Zombie->GetActorLocation() + FVector(0.0f, 0.0f, 72.0f);
    const FVector End = PlayerPawn->GetActorLocation() + FVector(0.0f, 0.0f, 72.0f);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CodeRescueZombieBarricadeTrace), false, Zombie);
    Params.AddIgnoredActor(PlayerPawn);
    const bool bHit = Zombie->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, CodeRescueCollision::AISightTrace, Params);
    if (!bHit)
    {
        return nullptr;
    }

    return Cast<ABarricadeActor>(OutHit.GetActor());
}

}

ACodeZombieActor::ACodeZombieActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Use inherited capsule and skeletal mesh from ACharacter
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    GetCapsuleComponent()->SetCollisionObjectType(CodeRescueCollision::ZombiePawnObject);
    GetCapsuleComponent()->SetCollisionResponseToChannel(CodeRescueCollision::WeaponTrace, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(CodeRescueCollision::AISightTrace, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Ignore);
    GetCapsuleComponent()->ComponentTags.AddUnique(FName("CollisionChannel_ZombiePawnObject"));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CodeRescueAnimationBudget::ApplySkeletalMeshBudget(
        GetMesh(), ECodeRescueAnimationBudgetProfile::CrowdZombie, this);
    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        GetMesh(), ECodeRescueRetargetRigProfile::ZombieCrowd, this);

    // Procedural fallback: cube body + sphere head
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveBody"));
    Body->SetupAttachment(GetCapsuleComponent());
    Body->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.55f));

    Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveHead"));
    Head->SetupAttachment(Body);
    Head->SetRelativeLocation(FVector(0, 0, 115));
    Head->SetRelativeScale3D(FVector(0.55f));

    Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("InfectionGlow"));
    Glow->SetupAttachment(GetCapsuleComponent());
    Glow->SetRelativeLocation(FVector(0, 0, 110));
    Glow->SetLightColor(FLinearColor(0.2f, 1.0f, 0.25f));
    Glow->SetIntensity(950.0f);
    Glow->SetAttenuationRadius(240.0f);

    PhysicalHitReactionComponent = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("ZombiePhysicalHitReaction"));
    PhysicalHitReactionComponent->ComponentTags.AddUnique(FName("ZombiePhysicalAnimationHitReactionComponent"));
    PhysicalHitReactionComponent->ComponentTags.AddUnique(FName("GamePhysicsDeepDive"));
    PhysicalHitReactionComponent->ComponentTags.AddUnique(FName("CharacterAnimationDeepDive"));

    // Persistent audio component for ambient growls. We don't autoplay; the
    // Tick-driven scheduler retriggers it at random intervals.
    GrowlAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("GrowlAudio"));
    GrowlAudio->SetupAttachment(GetCapsuleComponent());
    GrowlAudio->bAutoActivate = false;
    GrowlAudio->VolumeMultiplier = 1.0f;
    // #10 — 3D spatial audio. Enable spatialization unconditionally so when
    // a USoundAttenuation asset (or USoundCue with attenuation) is supplied,
    // distance falloff is honored. The attached components inherit listener-
    // distance attenuation from this flag.
    GrowlAudio->bAllowSpatialization = true;
    GrowlAudio->bOverrideAttenuation = false; // honor whatever the cue declares

    // Configure CharacterMovementComponent
    RefreshMovementSettings();

    // Set AI controller
    AIControllerClass = ACodeRescueAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CubeMesh.Succeeded()) Body->SetStaticMesh(CubeMesh.Object);
    if (SphereMesh.Succeeded()) Head->SetStaticMesh(SphereMesh.Object);
}

void ACodeZombieActor::BeginPlay()
{
    Super::BeginPlay();
    RefreshMovementSettings();

    // 2026-07-07 (Kenny: "characters do not position directly on top of these
    // regions") / upgraded 2026-07-11 (Kenny: "characters are floating"):
    // spawners place zombies at layout-table Z which can float above or sink
    // below the layered street kit. The shared robust snap ignores catch
    // floors and other characters, and falls back to ECC_WorldStatic when the
    // visibility channel finds nothing.
    ACodeRescueGameMode::SnapCharacterBaseToGround(this);

    if (ProfessionalZombieMesh)
    {
        ApplyProfessionalVisuals();
    }
    else
    {
        // Distinct materials for body (rotted dark green/brown) and head
        // (sickly emissive green) so a zombie reads as a creature instead
        // of a red box. Done at BeginPlay because dynamic materials need
        // a valid world.
        if (UMaterialInstanceDynamic* BodyMat = Body->CreateAndSetMaterialInstanceDynamic(0))
        {
            const FLinearColor Rot = FLinearColor(0.18f, 0.22f, 0.10f);
            BodyMat->SetVectorParameterValue(TEXT("Color"),         Rot);
            BodyMat->SetVectorParameterValue(TEXT("BaseColor"),     Rot);
            BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), Rot * 0.3f);
        }
        if (UMaterialInstanceDynamic* HeadMat = Head->CreateAndSetMaterialInstanceDynamic(0))
        {
            const FLinearColor Sickly = FLinearColor(0.30f, 0.85f, 0.20f);
            HeadMat->SetVectorParameterValue(TEXT("Color"),         Sickly);
            HeadMat->SetVectorParameterValue(TEXT("BaseColor"),     Sickly);
            HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), Sickly * 1.6f);
        }
    }

    // Variant/mesh setup can reapply serialized collision profiles after the
    // C++ constructor. Reassert the gameplay trace contract only after that
    // setup so live bullets always resolve the zombie capsule first.
    SetActorEnableCollision(true);
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        Capsule->SetCollisionObjectType(CodeRescueCollision::ZombiePawnObject);
        Capsule->SetCollisionResponseToChannel(CodeRescueCollision::WeaponTrace, ECR_Block);
        Capsule->SetCollisionResponseToChannel(CodeRescueCollision::AISightTrace, ECR_Block);
        Capsule->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Ignore);
        Capsule->ComponentTags.AddUnique(FName("RuntimeWeaponTraceBlocking"));
    }

    CacheMotionReadabilityBasePose(true);
    BindPhysicalHitReactionComponent();
    ApplyMonoAudioAccessibility(IsRuntimeMonoAudioEnabled(this));

    if (InfectionAuraVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(InfectionAuraVFX, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
    }

    // Begin the ambient growl loop. ScheduleNextGrowl no-ops gracefully if
    // GrowlCue is null (no audio in pack), so this is safe for all variants.
    ScheduleNextGrowl();
}

void ACodeZombieActor::ApplyProfessionalVisuals()
{
    // 2026-07-04 (top-50 item 42): when no zombie-pack mesh resolved (procedural
    // fallback variant), dress the zombie in one of the authored v2 infected
    // (ShamblerV2 / BruteV2, alternating) with their own shamble loop — cubes
    // and spheres no longer stand in for the horde.
    bUsingV2ZombieBody = false;
    bAuthoredBodyPhysicsReady = false;
    AuthoredLoopAnim = nullptr;
    AuthoredAttackAnim = nullptr;
    AuthoredFlinchAnim = nullptr;
    AuthoredDeathAnim = nullptr;
    UAnimSequence* V2ZombieLoop = nullptr;

    // 2026-07-11 v3: a deterministic share of the everyday horde trades its
    // pack mesh for an authored V3 body (keyed one-shots + authored ragdoll).
    const int32 AuthoredShare = FMath::Clamp(CVarAuthoredZombieShare.GetValueOnGameThread(), 0, 100);
    const bool bAuthoredEligibleVariant =
        Variant == EZombieVariant::BaseMesh ||
        Variant == EZombieVariant::UrbanZombie4 ||
        Variant == EZombieVariant::BusinessSuit ||
        Variant == EZombieVariant::EliteCharger;
    const bool bForceAuthoredBody = ProfessionalZombieMesh && bAuthoredEligibleVariant &&
        static_cast<int32>(GetUniqueID() % 100u) < AuthoredShare;

    if (!ProfessionalZombieMesh || bForceAuthoredBody)
    {
        // Three authored infected silhouettes. Chargers sprint,
        // so they wear the lean RunnerV3 (with its frantic Run loop); the rest
        // of the fallback horde rotates Brute/Shambler/Runner deterministically.
        const TCHAR* V3Name;
        bool bV3RunLoop = false;
        if (Variant == EZombieVariant::EliteCharger)
        {
            V3Name = TEXT("ZombieRunnerV3");
            bV3RunLoop = true;
        }
        else
        {
            switch (GetUniqueID() % 3)
            {
            case 0:  V3Name = TEXT("ZombieBruteV3"); break;
            case 1:  V3Name = TEXT("ZombieShamblerV3"); break;
            default: V3Name = TEXT("ZombieRunnerV3"); bV3RunLoop = true; break;
            }
        }
        if (USkeletalMesh* V3Mesh = LoadAuthoredV3Mesh(V3Name))
        {
            ProfessionalZombieMesh = V3Mesh;
            bUsingV2ZombieBody = true;   // same single-node presentation path as v2
            AuthoredLoopAnim = LoadAuthoredV3Anim(V3Name, bV3RunLoop ? TEXT("Run") : TEXT("Walk"));
            if (!AuthoredLoopAnim)
            {
                AuthoredLoopAnim = LoadAuthoredV3Anim(V3Name, TEXT("Walk"));
            }
            if (!AuthoredLoopAnim)
            {
                AuthoredLoopAnim = LoadAuthoredV3Anim(V3Name, TEXT("Idle"));
            }
            AuthoredAttackAnim = LoadAuthoredV3Anim(V3Name, TEXT("Attack"));
            AuthoredFlinchAnim = LoadAuthoredV3Anim(V3Name, TEXT("Flinch"));
            AuthoredDeathAnim = LoadAuthoredV3Anim(V3Name, TEXT("Death"));
            V2ZombieLoop = AuthoredLoopAnim;
            UE_LOG(LogTemp, Display,
                TEXT("[ZombieV3] %s wears %s (loop=%d attack=%d flinch=%d death=%d)"),
                *GetName(), V3Name, AuthoredLoopAnim != nullptr, AuthoredAttackAnim != nullptr,
                AuthoredFlinchAnim != nullptr, AuthoredDeathAnim != nullptr);
        }
    }
    if (!ProfessionalZombieMesh)
    {
        const bool bBrute = (GetUniqueID() % 2) == 0;
        const TCHAR* MeshPath = bBrute
            ? TEXT("/Game/CodeRescueArt/CharactersV2/ZombieBruteV2/ZombieBruteV2.ZombieBruteV2")
            : TEXT("/Game/CodeRescueArt/CharactersV2/ZombieShamblerV2/ZombieShamblerV2.ZombieShamblerV2");
        const TCHAR* WalkPath = bBrute
            ? TEXT("/Game/CodeRescueArt/CharactersV2/ZombieBruteV2/ZombieBruteV2_Anim_ZombieBruteV2_Walk.ZombieBruteV2_Anim_ZombieBruteV2_Walk")
            : TEXT("/Game/CodeRescueArt/CharactersV2/ZombieShamblerV2/ZombieShamblerV2_Anim_ZombieShamblerV2_Walk.ZombieShamblerV2_Anim_ZombieShamblerV2_Walk");
        const TCHAR* IdlePath = bBrute
            ? TEXT("/Game/CodeRescueArt/CharactersV2/ZombieBruteV2/ZombieBruteV2_Anim_ZombieBruteV2_Idle.ZombieBruteV2_Anim_ZombieBruteV2_Idle")
            : TEXT("/Game/CodeRescueArt/CharactersV2/ZombieShamblerV2/ZombieShamblerV2_Anim_ZombieShamblerV2_Idle.ZombieShamblerV2_Anim_ZombieShamblerV2_Idle");
        ProfessionalZombieMesh = LoadObject<USkeletalMesh>(nullptr, MeshPath);
        if (ProfessionalZombieMesh)
        {
            bUsingV2ZombieBody = true;
            V2ZombieLoop = LoadObject<UAnimSequence>(nullptr, WalkPath);
            if (!V2ZombieLoop)
            {
                V2ZombieLoop = LoadObject<UAnimSequence>(nullptr, IdlePath);
            }
        }
    }

    if (!ProfessionalZombieMesh || !GetMesh())
    {
        return;
    }

    // 2026-07-04 crash fix (part B): a profile bound to the PREVIOUS mesh keeps
    // body-indexed drive data; swapping the mesh resets Bodies and the next
    // physical-anim tick asserts. Detach before the swap; the impact path
    // reattaches only after it has created and validated compatible bodies.
    if (PhysicalHitReactionComponent)
    {
        PhysicalHitReactionComponent->SetSkeletalMeshComponent(nullptr);
    }
    GetMesh()->SetSkeletalMesh(ProfessionalZombieMesh);
    GetMesh()->SetVisibility(true);
    const float HalfHeight = GetCapsuleComponent()
        ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
        : 88.0f;
    GetMesh()->SetRelativeLocationAndRotation(
        FVector(0.0f, 0.0f, -HalfHeight),
        FRotator(0.0f, -90.0f, 0.0f));
    Body->SetVisibility(false);
    Head->SetVisibility(false);

    // Hook up the AnimBP that ships with the zombie pack so the model actually
    // animates instead of standing in T-pose when a compatible class exists.
    if (bUsingV2ZombieBody)
    {
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        if (V2ZombieLoop)
        {
            GetMesh()->PlayAnimation(V2ZombieLoop, true);
        }

        // 2026-07-11 art+physics v3: the V3 characters import with a deliberate
        // physics asset. When every body maps onto a real bone (and the kill
        // switch is on), KEEP the asset so authored zombies get ragdoll deaths
        // and physical hit reactions again. Safe because the 07-04 part-A tick
        // guard, the deferred bind (Bodies.Num()==0 -> defer), and the 07-11
        // reset/death full-detach all remain in force.
        int32 MatchedBodies = 0;
        int32 TotalBodies = 0;
        const UPhysicsAsset* ImportedPA = GetMesh()->GetPhysicsAsset();
        if (!ImportedPA && ProfessionalZombieMesh)
        {
            // Component override may linger from an earlier state; fall back to
            // the asset-level physics asset and bind it explicitly.
            ImportedPA = ProfessionalZombieMesh->GetPhysicsAsset();
            if (ImportedPA)
            {
                GetMesh()->SetPhysicsAsset(const_cast<UPhysicsAsset*>(ImportedPA), /*bForceReInit=*/true);
            }
        }
        if (ImportedPA)
        {
            for (const USkeletalBodySetup* Setup : ImportedPA->SkeletalBodySetups)
            {
                ++TotalBodies;
                if (Setup && GetMesh()->GetBoneIndex(Setup->BoneName) != INDEX_NONE)
                {
                    ++MatchedBodies;
                }
            }
        }
        const bool bAllowAuthoredPhysics = CVarAuthoredBodyPhysics.GetValueOnGameThread() != 0;
        UE_LOG(LogTemp, Display, TEXT("[ZombieV3] %s physics-eval pa=%s matched=%d total=%d cvar=%d"),
            *GetName(), ImportedPA ? *ImportedPA->GetName() : TEXT("NULL"),
            MatchedBodies, TotalBodies, bAllowAuthoredPhysics ? 1 : 0);
        if (bAllowAuthoredPhysics && MatchedBodies >= 6 && MatchedBodies == TotalBodies)
        {
            bAuthoredBodyPhysicsReady = true;
            Tags.AddUnique(FName("ZombieAuthoredBodyPhysicsReady"));
            UE_LOG(LogTemp, Display, TEXT("[ZombieV3] %s authored physics asset kept (%d bodies)"),
                *GetName(), MatchedBodies);
        }
        else
        {
            // CRASH FIX (2026-07-04 playtest): the legacy FBX import auto-generates a
            // physics asset for the v2 meshes; it passes Bind's null-check but its body
            // set doesn't match what the physical-animation profile expects, and
            // UPhysicalAnimationComponent::UpdatePhysicsEngineImp asserts (index 1 into
            // size 0) on the first tick. Such zombies run WITHOUT the physical
            // hit-reaction layer (and without ragdoll: physics asset cleared) — the
            // pack-mesh zombies keep both.
            GetMesh()->SetPhysicsAsset(nullptr, /*bForceReInit=*/true);
            if (PhysicalHitReactionComponent)
            {
                PhysicalHitReactionComponent->SetSkeletalMeshComponent(nullptr);
                PhysicalHitReactionComponent->Deactivate();
                PhysicalHitReactionComponent->SetComponentTickEnabled(false);
            }
            Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        }
    }
    else if (ProfessionalZombieAnimClass)
    {
        GetMesh()->SetAnimInstanceClass(ProfessionalZombieAnimClass);
    }
    else
    {
        const TCHAR* FallbackAnimationPath = nullptr;
        switch (Variant)
        {
        case EZombieVariant::DogZombie:
            FallbackAnimationPath = TEXT("/Game/DogZombie/Animations/anim_Dog_Trot_InPlace.anim_Dog_Trot_InPlace");
            break;
        case EZombieVariant::UrbanZombie4:
            FallbackAnimationPath = TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Walk_F_1_Loop_IPC.Zombie_Walk_F_1_Loop_IPC");
            break;
        case EZombieVariant::BusinessSuit:
        case EZombieVariant::EliteCharger:
            FallbackAnimationPath = TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Walk_F_1_Loop_IPC.Zombie_Walk_F_1_Loop_IPC");
            break;
        case EZombieVariant::BloatedFemale:
        case EZombieVariant::EliteSpitter:
            FallbackAnimationPath = TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Idle_2_IPC.Zombie_Idle_2_IPC");
            break;
        case EZombieVariant::NurseFemale:
            FallbackAnimationPath = TEXT("/Game/ZombieFemale/Asset/Animations/ANMS_ZombieFemaleWalk01Forward.ANMS_ZombieFemaleWalk01Forward");
            break;
        case EZombieVariant::BaseMesh:
            FallbackAnimationPath = TEXT("/Game/Zombie/Demo/Animations/ThirdPersonWalk.ThirdPersonWalk");
            break;
        case EZombieVariant::EliteBoomer:
            FallbackAnimationPath = TEXT("/Game/YI_ModularZombies/Animation/MocapOnline/UE4/In_Place/Zombie_Idle_2_IPC.Zombie_Idle_2_IPC");
            break;
        default:
            break;
        }

        if (FallbackAnimationPath)
        {
            if (UAnimationAsset* FallbackAnimation = LoadObject<UAnimationAsset>(nullptr, FallbackAnimationPath))
            {
                GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
                GetMesh()->SetAnimation(FallbackAnimation);
                GetMesh()->Play(true);
                Tags.Add(FName("BespokeAnimationClipFallback"));
            }
        }
    }

    ACodeRescueGameMode::AlignCharacterVisualFeetToCapsule(this);
    CacheMotionReadabilityBasePose(true);
    BindPhysicalHitReactionComponent();
}

void ACodeZombieActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ReleaseRagdollBudget();
    Super::EndPlay(EndPlayReason);
}

bool ACodeZombieActor::RefreshGroundedVisualPose()
{
    const bool bAligned = ACodeRescueGameMode::AlignCharacterVisualFeetToCapsule(this);
    CacheMotionReadabilityBasePose(true);
    return bAligned;
}

void ACodeZombieActor::ScheduleNextGrowl()
{
    if (bIsDying || !GrowlCue || !GrowlAudio)
    {
        return;
    }
    // Random 6–14 sec between growls — pleasantly chatty without being
    // annoying. The actual play happens via lambda so the timer fires once.
    const float Delay = FMath::RandRange(6.0f, 14.0f);
    GetWorldTimerManager().SetTimer(
        GrowlTimer,
        FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            if (bIsDying || !GrowlAudio || !GrowlCue) return;
            GrowlAudio->SetSound(GrowlCue);
            GrowlAudio->SetVolumeMultiplier(GetRuntimeSfxVolume(this));
            ApplyMonoAudioAccessibility(IsRuntimeMonoAudioEnabled(this));
            GrowlAudio->Play();
            PushThreatCaption(TEXT("growl"), 3600.0f, 7.0f);
            ScheduleNextGrowl();
        }),
        Delay, /*bLoop=*/false);
}

void ACodeZombieActor::ApplyMonoAudioAccessibility(bool bMonoAudioEnabled)
{
    if (GrowlAudio)
    {
        GrowlAudio->bAllowSpatialization = !bMonoAudioEnabled;
    }
    if (bMonoAudioEnabled)
    {
        Tags.AddUnique(FName("MonoAudioCenteredThreatCue"));
    }
}

void ACodeZombieActor::PushThreatCaption(const FString& EventLabel, float RadiusUU, float CooldownSeconds)
{
    UWorld* W = GetWorld();
    if (!W)
    {
        return;
    }

    const float Now = W->TimeSeconds;
    if (Now - LastThreatCaptionWorldTime < FMath::Max(0.25f, CooldownSeconds))
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(W, 0);
    if (!PlayerPawn)
    {
        return;
    }

    const float DistanceUU = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistanceUU > FMath::Max(100.0f, RadiusUU))
    {
        return;
    }

    LastThreatCaptionWorldTime = Now;
    const float DistanceMeters = DistanceUU / 100.0f;
    const FString Direction = DirectionLabelFromZombieToPlayer(this, PlayerPawn);
    const FString Role = ZombieThreatRolePrefix(EncounterRole);
    const FString VariantLabel = ZombieThreatVariantLabel(Variant);

    UCodeRescueSubtitlesWidget::Push(
        FString::Printf(TEXT("[Threat %s]: %s%s %s, %.0fm."),
            *Direction,
            *Role,
            *VariantLabel,
            *EventLabel,
            DistanceMeters),
        2.6f);
}

void ACodeZombieActor::ApplyStandardDirectPursuitProfile()
{
    bStandardDirectPursuitEnabled = true;
    StandardPursuitAttackCooldown = FMath::Clamp(StandardPursuitAttackCooldown, 0.9f, 1.8f);
    StandardPursuitReadabilityRange = FMath::Max(StandardPursuitReadabilityRange, AttackRange * 4.0f);
    StandardPursuitClosePressureRange = FMath::Max(StandardPursuitClosePressureRange, AttackRange * 2.2f);
    AttackTelegraphRangeMultiplier = FMath::Clamp(FMath::Max(AttackTelegraphRangeMultiplier, 1.35f), 1.0f, 3.0f);
    AttackTelegraphLeadSeconds = FMath::Clamp(FMath::Max(AttackTelegraphLeadSeconds, 0.38f), 0.0f, 1.25f);
    HitReactionImpulseStrength = FMath::Max(HitReactionImpulseStrength, 120.0f);

    Tags.AddUnique(FName("StandardDirectPursuitZombie"));
    Tags.AddUnique(FName("ZombiePursuitReadableRuntime"));
    Tags.AddUnique(FName("FairSurvivalPressure"));
    Tags.AddUnique(FName("NoLearningZonePressure"));
    Tags.AddUnique(FName("AttackWindupReadable"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("GamePhysicsDeepDive"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("Top50Recommendation40StealthAvoidance"));
    Tags.AddUnique(FName("StealthAvoidanceParticipant"));
}

float ACodeZombieActor::GetStandardPursuitAttackCooldown() const
{
    return bStandardDirectPursuitEnabled
        ? FMath::Clamp(StandardPursuitAttackCooldown, 0.5f, 4.0f)
        : 1.25f;
}

FString ACodeZombieActor::GetStandardPursuitStateSummary(float DistanceToPlayerUU) const
{
    if (!bStandardDirectPursuitEnabled)
    {
        return TEXT("unprofiled");
    }
    if (bIsDying || Health <= 0.0f)
    {
        return TEXT("neutralized");
    }
    if (DistanceToPlayerUU < 0.0f)
    {
        return TEXT("direct pursuit ready");
    }
    if (DistanceToPlayerUU > ActivationRange * GetWeatherVisibilityScale())
    {
        return TEXT("dormant");
    }
    if (DistanceToPlayerUU <= AttackRange * AttackTelegraphRangeMultiplier)
    {
        return TEXT("attack windup");
    }
    if (DistanceToPlayerUU <= StandardPursuitClosePressureRange)
    {
        return TEXT("close pursuit");
    }
    if (DistanceToPlayerUU <= StandardPursuitReadabilityRange)
    {
        return TEXT("direct pursuit");
    }
    return TEXT("tracking");
}

void ACodeZombieActor::UpdateStandardPursuitReadability(float DeltaSeconds, APawn* PlayerPawn, float DistanceToPlayer, bool bTelegraphingAttack)
{
    (void)DeltaSeconds;
    if (!bStandardDirectPursuitEnabled || !PlayerPawn || bIsDying)
    {
        return;
    }

    ApplyStandardDirectPursuitProfile();

    FName StateTag("StandardPursuitState_Tracking");
    FString CaptionLabel = TEXT("tracking");
    if (bTelegraphingAttack)
    {
        StateTag = FName("StandardPursuitState_AttackWindup");
        CaptionLabel = TEXT("attack windup");
        Tags.AddUnique(FName("StandardPursuitAttackTelegraph"));
    }
    else if (DistanceToPlayer <= StandardPursuitClosePressureRange)
    {
        StateTag = FName("StandardPursuitState_Close");
        CaptionLabel = TEXT("closing");
    }
    else if (DistanceToPlayer <= StandardPursuitReadabilityRange)
    {
        StateTag = FName("StandardPursuitState_DirectChase");
        CaptionLabel = TEXT("pursuing");
    }

    Tags.AddUnique(StateTag);
    if (StateTag == LastStandardPursuitStateTag)
    {
        return;
    }

    LastStandardPursuitStateTag = StateTag;
    UWorld* W = GetWorld();
    if (!W || DistanceToPlayer > StandardPursuitReadabilityRange)
    {
        return;
    }

    const float Now = W->TimeSeconds;
    if (Now - LastStandardPursuitCaptionWorldTime < 1.2f)
    {
        return;
    }

    LastStandardPursuitCaptionWorldTime = Now;
    PushThreatCaption(CaptionLabel, StandardPursuitReadabilityRange + 300.0f, bTelegraphingAttack ? 1.0f : 2.4f);
}

void ACodeZombieActor::RefreshMovementSettings()
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = FMath::Max(0.0f, MoveSpeed);
        Movement->RotationRate.Yaw = 640.0f;
        Movement->bUseControllerDesiredRotation = false;
        Movement->bOrientRotationToMovement = true;
    }
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void ACodeZombieActor::ConfigureEncounterDirective(ECodeRescueZombieEncounterRole InRole, const FVector& AnchorLocation, float LeashRadius, float FlankOffset, float PressureScale)
{
    bHasEncounterDirective = true;
    EncounterRole = InRole;
    EncounterAnchorLocation = AnchorLocation;
    EncounterLeashRadius = FMath::Max(0.0f, LeashRadius);
    EncounterFlankOffset = FMath::Clamp(FlankOffset, 0.0f, 1800.0f);
    EncounterDirectivePressureScale = FMath::Clamp(PressureScale, 0.25f, 3.0f);

    Tags.AddUnique(FName("AIDirectedEncounter"));
    Tags.AddUnique(FName("EncounterDirectedZombie"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));

    switch (EncounterRole)
    {
    case ECodeRescueZombieEncounterRole::Anchor:
        Tags.AddUnique(FName("EncounterRole_Anchor"));
        break;
    case ECodeRescueZombieEncounterRole::Flanker:
        Tags.AddUnique(FName("EncounterRole_Flanker"));
        break;
    case ECodeRescueZombieEncounterRole::Pressure:
        Tags.AddUnique(FName("EncounterRole_Pressure"));
        break;
    case ECodeRescueZombieEncounterRole::Sentinel:
        Tags.AddUnique(FName("EncounterRole_Sentinel"));
        break;
    default:
        Tags.AddUnique(FName("EncounterRole_Default"));
        break;
    }
}

FVector ACodeZombieActor::ResolveEncounterMoveTarget(const FVector& PlayerLocation) const
{
    if (!bHasEncounterDirective)
    {
        return PlayerLocation;
    }

    const FVector ActorLocation = GetActorLocation();
    const float SafeLeashRadius = FMath::Max(0.0f, EncounterLeashRadius);
    auto ClampToLeash = [&](const FVector& Candidate) -> FVector
    {
        if (SafeLeashRadius <= KINDA_SMALL_NUMBER)
        {
            return Candidate;
        }

        FVector FromAnchor = Candidate - EncounterAnchorLocation;
        FromAnchor.Z = 0.0f;
        const float DistanceFromAnchor = FromAnchor.Size();
        if (DistanceFromAnchor <= SafeLeashRadius || DistanceFromAnchor <= KINDA_SMALL_NUMBER)
        {
            return Candidate;
        }

        FVector Clamped = EncounterAnchorLocation + FromAnchor.GetSafeNormal() * SafeLeashRadius;
        Clamped.Z = Candidate.Z;
        return Clamped;
    };

    FVector AnchorToPlayer = PlayerLocation - EncounterAnchorLocation;
    AnchorToPlayer.Z = 0.0f;
    if (AnchorToPlayer.IsNearlyZero())
    {
        AnchorToPlayer = PlayerLocation - ActorLocation;
        AnchorToPlayer.Z = 0.0f;
    }
    const FVector Forward = AnchorToPlayer.IsNearlyZero() ? GetActorForwardVector() : AnchorToPlayer.GetSafeNormal();
    const float SideSign = (ZombieId % 2 == 0) ? 1.0f : -1.0f;
    const FVector Right(-Forward.Y * SideSign, Forward.X * SideSign, 0.0f);
    const float PlayerDistanceToAnchor = FVector::Dist2D(PlayerLocation, EncounterAnchorLocation);
    const float ActorDistanceToAnchor = FVector::Dist2D(ActorLocation, EncounterAnchorLocation);

    switch (EncounterRole)
    {
    case ECodeRescueZombieEncounterRole::Anchor:
        if (SafeLeashRadius > KINDA_SMALL_NUMBER && ActorDistanceToAnchor > SafeLeashRadius * 0.72f)
        {
            return EncounterAnchorLocation;
        }
        if (SafeLeashRadius <= KINDA_SMALL_NUMBER || PlayerDistanceToAnchor <= SafeLeashRadius * 1.15f)
        {
            return PlayerLocation;
        }
        return EncounterAnchorLocation + Right * 120.0f;

    case ECodeRescueZombieEncounterRole::Flanker:
        return ClampToLeash(PlayerLocation - Forward * 180.0f + Right * FMath::Max(160.0f, EncounterFlankOffset));

    case ECodeRescueZombieEncounterRole::Pressure:
        return PlayerLocation;

    case ECodeRescueZombieEncounterRole::Sentinel:
        if (SafeLeashRadius <= KINDA_SMALL_NUMBER || PlayerDistanceToAnchor <= SafeLeashRadius * 1.55f)
        {
            return PlayerLocation;
        }
        return EncounterAnchorLocation + Right * 180.0f;

    default:
        return PlayerLocation;
    }
}

void ACodeZombieActor::FaceMovementTarget(const FVector& TargetLocation, float DeltaSeconds)
{
    FVector ToTarget = TargetLocation - GetActorLocation();
    ToTarget.Z = 0.0f;
    if (ToTarget.IsNearlyZero())
    {
        return;
    }

    const FRotator DesiredRotation = ToTarget.Rotation();
    const float InterpSpeed = DeltaSeconds > 0.0f ? 12.0f : 1.0f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaSeconds, InterpSpeed));
}

void ACodeZombieActor::CacheMotionReadabilityBasePose(bool bForce)
{
    if (!bForce && bMotionReadabilityBasePoseCached)
    {
        return;
    }

    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        SkeletalMotionBaseLocation = SkeletalMesh->GetRelativeLocation();
        SkeletalMotionBaseRotation = SkeletalMesh->GetRelativeRotation();
        SkeletalMotionBaseScale = SkeletalMesh->GetRelativeScale3D();
        SkeletalMesh->ComponentTags.AddUnique(FName("ZombieMotionReadabilityComponent"));
        SkeletalMesh->ComponentTags.AddUnique(FName("AdditivePoseReadabilityRuntime"));
    }
    if (Body)
    {
        PrimitiveBodyMotionBaseLocation = Body->GetRelativeLocation();
        PrimitiveBodyMotionBaseRotation = Body->GetRelativeRotation();
        PrimitiveBodyMotionBaseScale = Body->GetRelativeScale3D();
        Body->ComponentTags.AddUnique(FName("ZombieMotionReadabilityComponent"));
        Body->ComponentTags.AddUnique(FName("AdditivePoseReadabilityRuntime"));
    }
    if (Head)
    {
        PrimitiveHeadMotionBaseLocation = Head->GetRelativeLocation();
        PrimitiveHeadMotionBaseRotation = Head->GetRelativeRotation();
        PrimitiveHeadMotionBaseScale = Head->GetRelativeScale3D();
        Head->ComponentTags.AddUnique(FName("ZombieMotionReadabilityComponent"));
    }
    if (Glow)
    {
        GlowMotionBaseLocation = Glow->GetRelativeLocation();
    }

    Tags.AddUnique(FName("ZombieMotionReadabilityRuntime"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    bMotionReadabilityBasePoseCached = true;
}

void ACodeZombieActor::ResetMotionReadabilityPose()
{
    if (!bMotionReadabilityBasePoseCached)
    {
        return;
    }

    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        if (!SkeletalMesh->IsSimulatingPhysics())
        {
            SkeletalMesh->SetRelativeLocationAndRotation(SkeletalMotionBaseLocation, SkeletalMotionBaseRotation);
            SkeletalMesh->SetRelativeScale3D(SkeletalMotionBaseScale);
        }
    }
    if (Body && !Body->IsSimulatingPhysics())
    {
        Body->SetRelativeLocationAndRotation(PrimitiveBodyMotionBaseLocation, PrimitiveBodyMotionBaseRotation);
        Body->SetRelativeScale3D(PrimitiveBodyMotionBaseScale);
    }
    if (Head && !Head->IsSimulatingPhysics())
    {
        Head->SetRelativeLocationAndRotation(PrimitiveHeadMotionBaseLocation, PrimitiveHeadMotionBaseRotation);
        Head->SetRelativeScale3D(PrimitiveHeadMotionBaseScale);
    }
    if (Glow)
    {
        Glow->SetRelativeLocation(GlowMotionBaseLocation);
    }
}

void ACodeZombieActor::TriggerAttackMotionCue()
{
    if (!bEnableMotionReadability)
    {
        return;
    }

    AttackLungePoseTimer = FMath::Max(AttackLungePoseTimer, FMath::Max(0.05f, AttackLungePoseDuration));
    Tags.AddUnique(FName("ZombieAttackLungePose"));
    Tags.AddUnique(FName("AttackMontageFallbackMotionCue"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
}

void ACodeZombieActor::TriggerHitReactionMotionCue(EHitZone HitZone, float FinalDamage)
{
    if (!bEnableMotionReadability || FinalDamage <= 0.0f)
    {
        return;
    }

    const float ZoneBonus = HitZone == EHitZone::Head ? 0.10f : 0.0f;
    const float DamageBonus = FMath::Clamp(FinalDamage / 100.0f, 0.0f, 0.16f);
    HitReactionPoseTimer = FMath::Max(
        HitReactionPoseTimer,
        FMath::Max(0.05f, HitReactionPoseDuration + ZoneBonus + DamageBonus));

    Tags.AddUnique(FName("ZombieHitReactPoseFallback"));
    Tags.AddUnique(FName("HitReactionMontageFallbackMotionCue"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
}

void ACodeZombieActor::UpdateMotionReadability(float DeltaSeconds, const APawn* PlayerPawn, float DistanceToPlayer, bool bTelegraphingAttack, bool bProtectedLearningHold)
{
    if (!bEnableMotionReadability)
    {
        ResetMotionReadabilityPose();
        return;
    }

    CacheMotionReadabilityBasePose();

    HitReactionPoseTimer = FMath::Max(0.0f, HitReactionPoseTimer - DeltaSeconds);
    AttackLungePoseTimer = FMath::Max(0.0f, AttackLungePoseTimer - DeltaSeconds);

    const float SafeMoveSpeed = FMath::Max(1.0f, MoveSpeed);
    const float SpeedAlpha = FMath::Clamp(GetVelocity().Size2D() / SafeMoveSpeed, 0.0f, 1.0f);
    const float ReadabilityScale = FMath::Clamp(MotionReadabilitySwayScale, 0.0f, 2.0f);
    const bool bPlayerInReadRange = PlayerPawn && DistanceToPlayer <= FMath::Max(ActivationRange, StandardPursuitReadabilityRange);
    const float ChaseAlpha = bPlayerInReadRange && !bProtectedLearningHold ? FMath::Max(SpeedAlpha, bTelegraphingAttack ? 0.5f : 0.0f) : 0.0f;
    const float WindupAlpha = bTelegraphingAttack ? 1.0f : 0.0f;
    const float LungeAlpha = AttackLungePoseDuration > KINDA_SMALL_NUMBER
        ? FMath::Clamp(AttackLungePoseTimer / AttackLungePoseDuration, 0.0f, 1.0f)
        : 0.0f;
    const float HitAlpha = HitReactionPoseDuration > KINDA_SMALL_NUMBER
        ? FMath::Clamp(HitReactionPoseTimer / HitReactionPoseDuration, 0.0f, 1.0f)
        : 0.0f;
    const float HoldAlpha = bProtectedLearningHold ? 1.0f : 0.0f;

    MotionReadabilityPhase += DeltaSeconds * FMath::Lerp(1.45f, 7.2f, FMath::Max(SpeedAlpha, WindupAlpha));
    const float Step = FMath::Sin(MotionReadabilityPhase * 2.0f);
    const float Side = FMath::Sin(MotionReadabilityPhase);

    const float ForwardOffset =
        ((5.0f * ChaseAlpha) - (13.0f * WindupAlpha) + (28.0f * LungeAlpha) - (16.0f * HitAlpha) - (8.0f * HoldAlpha)) *
        ReadabilityScale;
    const float SideOffset = Side * 4.0f * ChaseAlpha * ReadabilityScale;
    const float HeightOffset =
        ((FMath::Abs(Step) * 5.0f * ChaseAlpha) - (4.0f * WindupAlpha) + (3.0f * LungeAlpha) + (5.0f * HitAlpha)) *
        ReadabilityScale;
    const float PitchOffset =
        ((-5.0f * ChaseAlpha) + (-12.0f * WindupAlpha) + (14.0f * LungeAlpha) + (11.0f * HitAlpha) + (8.0f * HoldAlpha)) *
        ReadabilityScale;
    const float RollOffset = (Side * 5.5f * ChaseAlpha + 5.0f * HitAlpha) * ReadabilityScale;
    const float ScalePulse = 1.0f + ((0.018f * WindupAlpha) + (0.035f * LungeAlpha) - (0.018f * HitAlpha)) * ReadabilityScale;

    const FVector VisualOffset(ForwardOffset, SideOffset, HeightOffset);

    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        if (!SkeletalMesh->IsSimulatingPhysics())
        {
            SkeletalMesh->SetRelativeLocation(SkeletalMotionBaseLocation + VisualOffset);
            SkeletalMesh->SetRelativeRotation(FRotator(
                SkeletalMotionBaseRotation.Pitch + PitchOffset,
                SkeletalMotionBaseRotation.Yaw,
                SkeletalMotionBaseRotation.Roll + RollOffset));
            SkeletalMesh->SetRelativeScale3D(SkeletalMotionBaseScale * ScalePulse);
        }
    }
    if (Body && !Body->IsSimulatingPhysics())
    {
        Body->SetRelativeLocation(PrimitiveBodyMotionBaseLocation + VisualOffset);
        Body->SetRelativeRotation(FRotator(
            PrimitiveBodyMotionBaseRotation.Pitch + PitchOffset,
            PrimitiveBodyMotionBaseRotation.Yaw,
            PrimitiveBodyMotionBaseRotation.Roll + RollOffset));
        Body->SetRelativeScale3D(PrimitiveBodyMotionBaseScale * ScalePulse);
    }
    if (Head && !Head->IsSimulatingPhysics())
    {
        Head->SetRelativeLocation(PrimitiveHeadMotionBaseLocation + VisualOffset + FVector(0.0f, SideOffset * 0.3f, HeightOffset * 0.35f));
        Head->SetRelativeRotation(FRotator(
            PrimitiveHeadMotionBaseRotation.Pitch + PitchOffset * 0.6f,
            PrimitiveHeadMotionBaseRotation.Yaw,
            PrimitiveHeadMotionBaseRotation.Roll + RollOffset * 1.25f));
    }
    if (Glow)
    {
        Glow->SetRelativeLocation(GlowMotionBaseLocation + FVector(ForwardOffset * 0.55f, SideOffset * 0.45f, HeightOffset * 0.6f));
    }

    Tags.AddUnique(FName("ZombieMotionReadabilityRuntime"));
    if (ChaseAlpha > 0.15f)
    {
        Tags.AddUnique(FName("ZombieLocomotionSway"));
    }
    if (WindupAlpha > 0.0f)
    {
        Tags.AddUnique(FName("ZombieAttackWindupPose"));
    }
    if (LungeAlpha > 0.05f)
    {
        Tags.AddUnique(FName("ZombieAttackLungePose"));
    }
    if (HitAlpha > 0.05f)
    {
        Tags.AddUnique(FName("ZombieHitReactPoseFallback"));
    }
    if (HoldAlpha > 0.0f)
    {
        Tags.AddUnique(FName("ZombieProtectedZoneHoldPose"));
    }
}

FVector ACodeZombieActor::ComputeDeathPhysicsImpulse(EHitZone HitZone, float FinalDamage, float Strength) const
{
    const float ZoneScale =
        HitZone == EHitZone::Head ? 1.35f :
        HitZone == EHitZone::Limb ? 0.85f :
        1.0f;
    const float DamageScale = FMath::Clamp(FinalDamage / 55.0f, 0.72f, 1.32f);
    FVector Direction = LastIncomingShotDirection.IsNearlyZero()
        ? -GetActorForwardVector()
        : LastIncomingShotDirection.GetSafeNormal();
    Direction.Z = FMath::Clamp(Direction.Z + 0.08f, -0.18f, 0.28f);
    Direction.Normalize();
    const float GroundedStrength = FMath::Clamp(
        FMath::Max(0.0f, Strength) * ZoneScale * DamageScale,
        0.0f,
        620.0f);
    return Direction * GroundedStrength;
}

void ACodeZombieActor::PlayAuthoredOneShot(UAnimSequence* Anim, bool bResumeLoop)
{
    USkeletalMeshComponent* SkeletalMesh = GetMesh();
    if (!Anim || !SkeletalMesh || !bUsingV2ZombieBody || !SkeletalMesh->GetSkeletalMeshAsset())
    {
        return;
    }
    SkeletalMesh->PlayAnimation(Anim, false);
    GetWorldTimerManager().ClearTimer(AuthoredOneShotTimer);
    if (bResumeLoop && AuthoredLoopAnim)
    {
        TWeakObjectPtr<ACodeZombieActor> WeakThis(this);
        GetWorldTimerManager().SetTimer(AuthoredOneShotTimer,
            FTimerDelegate::CreateLambda([WeakThis]()
            {
                if (WeakThis.IsValid() && !WeakThis->bIsDying &&
                    WeakThis->GetMesh() && WeakThis->AuthoredLoopAnim)
                {
                    WeakThis->GetMesh()->PlayAnimation(WeakThis->AuthoredLoopAnim, true);
                }
            }),
            FMath::Max(0.1f, Anim->GetPlayLength()), false);
    }
}

void ACodeZombieActor::BindPhysicalHitReactionComponent()
{
    if (!PhysicalHitReactionComponent)
    {
        return;
    }
    // 2026-07-04 crash fix: never bind physical animation to authored bodies
    // UNLESS the 2026-07-11 v3 path validated their import-time physics asset.
    if (bUsingV2ZombieBody && !bAuthoredBodyPhysicsReady)
    {
        Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = GetMesh();
    const UPhysicsAsset* PhysicsAsset = SkeletalMesh ? SkeletalMesh->GetPhysicsAsset() : nullptr;
    if (!SkeletalMesh || !SkeletalMesh->GetSkeletalMeshAsset() || !PhysicsAsset ||
        PhysicsAsset->SkeletalBodySetups.IsEmpty() || ResolvePhysicalHitReactionRootBone() == NAME_None)
    {
        Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        return;
    }

    // The presentation mesh remains collision-free during ordinary movement,
    // so body instances are intentionally created lazily by the hit/death path.
    // Do not poll or warn at spawn; Bind is called again after that physics state
    // exists. This avoids hundreds of retries for distant campaign zombies.
    if (SkeletalMesh->Bodies.Num() == 0)
    {
        Tags.AddUnique(FName("ZombiePhysicalAnimationDeferredUntilImpact"));
        return;
    }

    const FName RootBone = ResolvePhysicalHitReactionRootBone();
    PhysicalHitReactionComponent->SetSkeletalMeshComponent(SkeletalMesh);
    PhysicalHitReactionComponent->SetStrengthMultiplyer(0.0f);

    SkeletalMesh->ComponentTags.AddUnique(FName("ZombiePhysicalAnimationHitReactionMesh"));
    SkeletalMesh->ComponentTags.AddUnique(FName("PhysicalAnimationComponentBound"));
    if (RootBone == NAME_None)
    {
        Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        return;
    }

    Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionReady"));
    Tags.AddUnique(FName("PhysicalAnimationComponentRuntime"));
    Tags.AddUnique(FName("GamePhysicsDeepDive"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
}

FName ACodeZombieActor::ResolvePhysicalHitReactionRootBone() const
{
    const USkeletalMeshComponent* SkeletalMesh = GetMesh();
    const UPhysicsAsset* PhysicsAsset = SkeletalMesh ? SkeletalMesh->GetPhysicsAsset() : nullptr;
    if (!SkeletalMesh || !PhysicsAsset)
    {
        return NAME_None;
    }

    auto HasPhysicsBody = [&](const FName BoneName) -> bool
    {
        return BoneName != NAME_None &&
            SkeletalMesh->GetBoneIndex(BoneName) != INDEX_NONE &&
            PhysicsAsset->FindBodyIndex(BoneName) != INDEX_NONE;
    };

    if (HasPhysicsBody(PhysicalHitReactionRootBone))
    {
        return PhysicalHitReactionRootBone;
    }

    static const FName CandidateBones[] = {
        FName(TEXT("spine_01")),
        FName(TEXT("spine")),
        FName(TEXT("pelvis")),
        FName(TEXT("root"))
    };

    for (const FName& CandidateBone : CandidateBones)
    {
        if (HasPhysicsBody(CandidateBone))
        {
            return CandidateBone;
        }
    }

    for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
    {
        if (BodySetup && HasPhysicsBody(BodySetup->BoneName))
        {
            return BodySetup->BoneName;
        }
    }

    return NAME_None;
}

FName ACodeZombieActor::ResolvePhysicalHitReactionImpactBone(EHitZone HitZone) const
{
    const USkeletalMeshComponent* SkeletalMesh = GetMesh();
    const UPhysicsAsset* PhysicsAsset = SkeletalMesh ? SkeletalMesh->GetPhysicsAsset() : nullptr;
    if (!SkeletalMesh || !PhysicsAsset)
    {
        return ResolvePhysicalHitReactionRootBone();
    }

    auto HasPhysicsBody = [&](const FName BoneName) -> bool
    {
        return BoneName != NAME_None &&
            SkeletalMesh->GetBoneIndex(BoneName) != INDEX_NONE &&
            PhysicsAsset->FindBodyIndex(BoneName) != INDEX_NONE;
    };

    if (HitZone == EHitZone::Head)
    {
        static const FName HeadBones[] = {
            FName(TEXT("head")),
            FName(TEXT("Head")),
            FName(TEXT("neck_01"))
        };
        for (const FName& HeadBone : HeadBones)
        {
            if (HasPhysicsBody(HeadBone))
            {
                return HeadBone;
            }
        }
    }

    if (HitZone == EHitZone::Limb)
    {
        static const FName LimbBones[] = {
            FName(TEXT("upperarm_l")),
            FName(TEXT("upperarm_r")),
            FName(TEXT("thigh_l")),
            FName(TEXT("thigh_r"))
        };
        for (const FName& LimbBone : LimbBones)
        {
            if (HasPhysicsBody(LimbBone))
            {
                return LimbBone;
            }
        }
    }

    return ResolvePhysicalHitReactionRootBone();
}

bool ACodeZombieActor::TriggerPhysicalAnimationHitReaction(EHitZone HitZone, float FinalDamage)
{
    if (!bEnablePhysicalHitReaction || bIsDying || FinalDamage <= 0.0f)
    {
        return false;
    }

    USkeletalMeshComponent* SkeletalMesh = GetMesh();
    const UPhysicsAsset* PhysicsAsset = SkeletalMesh ? SkeletalMesh->GetPhysicsAsset() : nullptr;
    if (!PhysicalHitReactionComponent ||
        !SkeletalMesh ||
        !SkeletalMesh->GetSkeletalMeshAsset() ||
        !PhysicsAsset ||
        PhysicsAsset->SkeletalBodySetups.IsEmpty() ||
        SkeletalMesh->IsSimulatingPhysics())
    {
        return false;
    }

    const FName RootBone = ResolvePhysicalHitReactionRootBone();
    if (RootBone == NAME_None)
    {
        Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        return false;
    }

    // Create body instances only when an impact needs them. Imported zombie
    // meshes are collision-free during locomotion, so Bodies is legitimately
    // empty at spawn even when the physics asset is valid.
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (SkeletalMesh->Bodies.Num() == 0)
    {
        SkeletalMesh->RecreatePhysicsState();
    }
    if (SkeletalMesh->Bodies.Num() == 0)
    {
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        return false;
    }

    BindPhysicalHitReactionComponent();
    if (!Tags.Contains(FName("ZombiePhysicalAnimationHitReactionReady")))
    {
        Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionFallbackOnly"));
        return false;
    }

    const FName ImpactBone = ResolvePhysicalHitReactionImpactBone(HitZone);
    const float ZoneHeight =
        HitZone == EHitZone::Head ? 142.0f :
        HitZone == EHitZone::Limb ? 58.0f :
        92.0f;
    const FVector ImpactLocation = GetActorLocation() + FVector(0.0f, 0.0f, ZoneHeight);
    const FVector ImpactImpulse = ComputeDeathPhysicsImpulse(
        HitZone,
        FinalDamage,
        PhysicalHitReactionImpulseStrength);

    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetGenerateOverlapEvents(false);
    CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
        SkeletalMesh,
        this,
        FName("ZombiePhysicalHitReactionFixedStepBody"),
        -1.0f,
        0.18f,
        0.26f,
        false);
    FPhysicalAnimationData HitReactDrive;
    HitReactDrive.bIsLocalSimulation = true;
    HitReactDrive.OrientationStrength = 850.0f;
    HitReactDrive.AngularVelocityStrength = 130.0f;
    HitReactDrive.PositionStrength = 0.0f;
    HitReactDrive.VelocityStrength = 0.0f;
    HitReactDrive.MaxLinearForce = 0.0f;
    HitReactDrive.MaxAngularForce = 65000.0f;
    PhysicalHitReactionComponent->ApplyPhysicalAnimationSettingsBelow(RootBone, HitReactDrive, true);
    SkeletalMesh->SetAllBodiesBelowSimulatePhysics(RootBone, true, true);
    SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(
        RootBone,
        FMath::Clamp(PhysicalHitReactionBlendWeight, 0.0f, 1.0f),
        false,
        true);
    PhysicalHitReactionComponent->SetStrengthMultiplyer(FMath::Clamp(PhysicalHitReactionBlendWeight, 0.0f, 1.0f));
    SkeletalMesh->AddImpulseAtLocation(ImpactImpulse, ImpactLocation, ImpactBone);

    PhysicalHitReactionTimer = FMath::Max(PhysicalHitReactionTimer, FMath::Max(0.05f, PhysicalHitReactionDuration));
    bPhysicalHitReactionActive = true;

    Tags.AddUnique(FName("ZombiePhysicalAnimationHitReaction"));
    Tags.AddUnique(FName("PhysicalAnimationHitReactionRuntime"));
    Tags.AddUnique(FName("HitReactionPhysicsBlend"));
    Tags.AddUnique(FName("GamePhysicsDeepDive"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    return true;
}

void ACodeZombieActor::UpdatePhysicalAnimationHitReaction(float DeltaSeconds)
{
    if (!bPhysicalHitReactionActive)
    {
        return;
    }

    USkeletalMeshComponent* SkeletalMesh = GetMesh();
    // 2026-07-11 failsafe: if ANY external path tore the body instances down
    // while a reaction is active (Bodies empty), settle and detach immediately
    // instead of letting the physical-animation update assert on a stale
    // body index.
    if (!PhysicalHitReactionComponent || !SkeletalMesh || bIsDying ||
        SkeletalMesh->Bodies.Num() == 0)
    {
        ResetPhysicalAnimationHitReaction();
        return;
    }

    PhysicalHitReactionTimer = FMath::Max(0.0f, PhysicalHitReactionTimer - DeltaSeconds);
    const float SafeDuration = FMath::Max(0.05f, PhysicalHitReactionDuration);
    const float RawAlpha = FMath::Clamp(PhysicalHitReactionTimer / SafeDuration, 0.0f, 1.0f);
    const float BlendAlpha = RawAlpha * RawAlpha * (3.0f - 2.0f * RawAlpha);
    const float BlendWeight = FMath::Clamp(PhysicalHitReactionBlendWeight, 0.0f, 1.0f) * BlendAlpha;
    const FName RootBone = ResolvePhysicalHitReactionRootBone();

    PhysicalHitReactionComponent->SetStrengthMultiplyer(BlendWeight);
    SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(RootBone, BlendWeight, false, true);

    if (PhysicalHitReactionTimer <= KINDA_SMALL_NUMBER)
    {
        ResetPhysicalAnimationHitReaction();
    }
}

void ACodeZombieActor::ResetPhysicalAnimationHitReaction()
{
    if (PhysicalHitReactionComponent)
    {
        PhysicalHitReactionComponent->SetStrengthMultiplyer(0.0f);
        // 2026-07-11 LAUNCH-CRASH ROOT FIX (Kenny: "game immediately closes"
        // when resuming his real save): this reset used to leave the physical-
        // animation component BOUND while the line below destroys every body
        // instance (SetCollisionEnabled(NoCollision) tears down the physics
        // state). The component keeps ticking with non-empty drive data, and
        // the next time its physics-engine update is re-armed (physics-state
        // recreation delegates, the death ragdoll's profile swap, or the next
        // hit) UPhysicalAnimationComponent::UpdatePhysicsEngineImp indexes
        // SkeletalMeshComponent->Bodies[ChildBodyIdx] on a SHORTER (or empty)
        // Bodies array -> "Array index out of bounds" assert -> instant exit.
        // Reproduced deterministically with -CodeRescueAutoResumeLanguage=Cpp
        // on the real save (spawn-adjacent zombie dies on frame 2, assert on
        // frame 3). DETACH FULLY here; TriggerPhysicalAnimationHitReaction
        // re-binds through BindPhysicalHitReactionComponent on the next hit
        // after it has recreated and validated compatible bodies.
        PhysicalHitReactionComponent->SetSkeletalMeshComponent(nullptr);
    }

    if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
    {
        const FName RootBone = ResolvePhysicalHitReactionRootBone();
        if (RootBone != NAME_None)
        {
            SkeletalMesh->SetAllBodiesBelowPhysicsBlendWeight(RootBone, 0.0f, false, true);
            SkeletalMesh->SetAllBodiesBelowSimulatePhysics(RootBone, false, true);
        }
        if (!bIsDying)
        {
            SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    PhysicalHitReactionTimer = 0.0f;
    bPhysicalHitReactionActive = false;
    Tags.AddUnique(FName("ZombiePhysicalAnimationHitReactionSettled"));
}

void ACodeZombieActor::ApplyHitReadabilityImpulse(EHitZone HitZone, float FinalDamage)
{
    if (bIsDying || FinalDamage <= 0.0f || !GetCharacterMovement())
    {
        return;
    }

    const float ZoneScale =
        HitZone == EHitZone::Head ? 1.3f :
        HitZone == EHitZone::Limb ? 0.75f :
        1.0f;
    const float DamageScale = FMath::Clamp(FinalDamage / 20.0f, 0.35f, 1.25f);
    const FVector ReadableNudge =
        (-GetActorForwardVector() * HitReactionImpulseStrength * ZoneScale * DamageScale) +
        FVector(0.0f, 0.0f, HitReactionImpulseStrength * 0.18f * ZoneScale);

    LaunchCharacter(ReadableNudge, false, false);
    Tags.AddUnique(FName("ZombieHitPhysicsReadability"));
    if (!TriggerPhysicalAnimationHitReaction(HitZone, FinalDamage))
    {
        Tags.AddUnique(FName("PhysicalHitReactionFallback"));
    }

    if (Glow)
    {
        Glow->SetLightColor(HitZone == EHitZone::Head ? FLinearColor(1.0f, 0.78f, 0.12f) : FLinearColor(1.0f, 0.18f, 0.08f));
        Glow->SetIntensity(4200.0f);
        Glow->SetAttenuationRadius(360.0f);
    }
}

void ACodeZombieActor::DisableGameplayCollisionForDeath()
{
    ResetPhysicalAnimationHitReaction();
    // 2026-07-11 launch-crash fix: the death paths detach/recreate the mesh's
    // physics state (ragdoll profile swap, primitive corpse, settle/fade). A
    // dying zombie never re-binds (bIsDying guards the trigger), so make the
    // detach explicit and unconditional before ANY of that teardown runs.
    if (PhysicalHitReactionComponent)
    {
        PhysicalHitReactionComponent->SetSkeletalMeshComponent(nullptr);
        PhysicalHitReactionComponent->SetComponentTickEnabled(false);
    }

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Capsule->SetGenerateOverlapEvents(false);
    }

    // The selected death path activates either skeletal ragdoll or primitive
    // corpse physics after this shared gameplay-collision shutdown.
}

bool ACodeZombieActor::TryActivateDeathRagdoll(EHitZone HitZone, float FinalDamage)
{
    if (!bEnableDeathRagdoll || GCodeRescueActiveRagdollCorpses >= CodeRescueMaxActiveRagdollCorpses)
    {
        return false;
    }

    USkeletalMeshComponent* SkeletalMesh = GetMesh();
    const UPhysicsAsset* PhysicsAsset = SkeletalMesh ? SkeletalMesh->GetPhysicsAsset() : nullptr;
    if (!SkeletalMesh || !SkeletalMesh->GetSkeletalMeshAsset() || !PhysicsAsset ||
        PhysicsAsset->SkeletalBodySetups.IsEmpty() || ResolvePhysicalHitReactionRootBone() == NAME_None)
    {
        return false;
    }

    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (SkeletalMesh->Bodies.Num() == 0)
    {
        SkeletalMesh->RecreatePhysicsState();
    }
    if (SkeletalMesh->Bodies.Num() == 0)
    {
        SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Tags.AddUnique(FName("ZombieDeathRagdollFallbackOnly"));
        return false;
    }

    DisableGameplayCollisionForDeath();
    SkeletalMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    SkeletalMesh->SetCollisionProfileName(TEXT("Ragdoll"));
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletalMesh->SetGenerateOverlapEvents(false);
    CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
        SkeletalMesh,
        this,
        FName("ZombieRagdollFixedStepBody"),
        -1.0f,
        0.22f,
        0.30f,
        false);
    SkeletalMesh->SetAllBodiesSimulatePhysics(true);
    SkeletalMesh->SetSimulatePhysics(true);
    SkeletalMesh->WakeAllRigidBodies();
    SkeletalMesh->AddImpulse(ComputeDeathPhysicsImpulse(HitZone, FinalDamage, RagdollImpulseStrength), NAME_None, true);

    if (Body)
    {
        Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (Head)
    {
        Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    ++GCodeRescueActiveRagdollCorpses;
    bCountedActiveRagdoll = true;
    Tags.AddUnique(FName("ZombieDeathRagdoll"));
    Tags.AddUnique(FName("GamePhysicsDeepDive"));
    Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    Tags.AddUnique(FName("DeathPhysicsReadable"));
    return true;
}

bool ACodeZombieActor::ActivatePrimitiveDeathPhysics(EHitZone HitZone, float FinalDamage)
{
    if (!bEnablePrimitiveCorpsePhysics ||
        ((!Body || !Body->IsVisible()) && (!Head || !Head->IsVisible())))
    {
        return false;
    }

    DisableGameplayCollisionForDeath();
    const FVector BaseImpulse = ComputeDeathPhysicsImpulse(HitZone, FinalDamage, PrimitiveCorpseImpulseStrength);
    bool bActivatedAnyCorpsePart = false;

    auto ActivateCorpsePart = [&](UStaticMeshComponent* Part, const FVector& ExtraImpulse, const FVector& AngularImpulse)
    {
        if (!Part || !Part->GetStaticMesh())
        {
            return;
        }

        Part->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        Part->SetCollisionProfileName(TEXT("PhysicsActor"));
        Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Part->SetGenerateOverlapEvents(false);
        CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
            Part,
            this,
            FName("ZombiePrimitiveCorpseFixedStepBody"),
            18.0f,
            0.36f,
            0.50f,
            false);
        Part->SetSimulatePhysics(true);
        Part->WakeAllRigidBodies();
        Part->AddImpulse(BaseImpulse + ExtraImpulse, NAME_None, true);
        Part->AddAngularImpulseInDegrees(AngularImpulse, NAME_None, true);
        bActivatedAnyCorpsePart = true;
    };

    ActivateCorpsePart(Body, FVector(0.0f, 0.0f, 90.0f), FVector(0.0f, 210.0f, 120.0f));
    ActivateCorpsePart(Head, FVector(0.0f, 0.0f, HitZone == EHitZone::Head ? 165.0f : 115.0f), FVector(150.0f, 90.0f, 260.0f));

    if (bActivatedAnyCorpsePart)
    {
        Tags.AddUnique(FName("ZombiePrimitiveCorpsePhysics"));
        Tags.AddUnique(FName("GamePhysicsDeepDive"));
        Tags.AddUnique(FName("DeathPhysicsReadable"));
    }
    return bActivatedAnyCorpsePart;
}

void ACodeZombieActor::Tick(float DeltaSeconds)
{
    // 2026-07-04 crash fix (part A): PhysicalAnimation ticks post-physics; we tick
    // pre-physics, so this guard deterministically disarms it any frame the mesh's
    // body instances are missing (spawn races, mesh swaps) before it can assert.
    if (PhysicalHitReactionComponent && GetMesh() && GetMesh()->Bodies.Num() == 0
        && PhysicalHitReactionComponent->GetSkeletalMesh() != nullptr)
    {
        PhysicalHitReactionComponent->SetSkeletalMeshComponent(nullptr);
    }
    Super::Tick(DeltaSeconds);
    RefreshMovementSettings();
    UpdatePhysicalAnimationHitReaction(DeltaSeconds);

    // Once Health hits 0, freeze movement/AI but keep Tick running so the
    // death montage can finish and the destroy timer can fire.
    if (bIsDying)
    {
        UpdateCorpseFade(DeltaSeconds);
        ResetMotionReadabilityPose();
        GetCharacterMovement()->StopMovementImmediately();
        return;
    }

    TimeSinceAttack += DeltaSeconds;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
    ToPlayer.Z = 0;
    const float Distance = ToPlayer.Size();
    if (IsPawnInsideProtectedLearningZone(PlayerPawn))
    {
        Tags.AddUnique(FName("NoZombieLearningZoneRespected"));
        if (bStandardDirectPursuitEnabled)
        {
            Tags.AddUnique(FName("StandardPursuitProtectedCodingSpaceHold"));
            Tags.AddUnique(FName("StandardPursuitNoLearningZonePressure"));
            PushThreatCaption(TEXT("holds outside coding zone"), 2400.0f, 5.0f);
        }
        TimeSinceAttack = 0.0f;
        DistantTickAccumulator = 0.0f;

        FVector AwayFromLearningZone = -ToPlayer;
        AwayFromLearningZone.Z = 0.0f;
        if (Distance < 950.0f && !AwayFromLearningZone.IsNearlyZero())
        {
            const FVector RetreatDirection = AwayFromLearningZone.GetSafeNormal();
            FaceMovementTarget(GetActorLocation() + RetreatDirection * 500.0f, DeltaSeconds);
            AddMovementInput(RetreatDirection, 0.65f);
        }
        else if (GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
        }

        if (Glow)
        {
            Glow->SetLightColor(FLinearColor(0.0f, 0.85f, 1.0f));
            Glow->SetIntensity(520.0f);
            Glow->SetAttenuationRadius(180.0f);
        }
        UpdateMotionReadability(DeltaSeconds, PlayerPawn, Distance, false, true);
        return;
    }

    const float AttackCooldown = GetStandardPursuitAttackCooldown();

    // #20 — distant-zombie tick throttle. Beyond 10000 units, drop to ~0.5 Hz
    // by skipping most ticks. This is a low-cost win on the 50x city since
    // the city map can have dozens of off-screen zombies idling.
    if (Distance > 10000.0f)
    {
        DistantTickAccumulator += DeltaSeconds;
        if (DistantTickAccumulator < 2.0f)
        {
            return;
        }
        DistantTickAccumulator = 0.0f;
    }
    else
    {
        DistantTickAccumulator = 0.0f;
    }

    if (Distance > ActivationRange * GetWeatherVisibilityScale())
    {
        GetCharacterMovement()->StopMovementImmediately();
        if (Glow)
        {
            Glow->SetLightColor(FLinearColor(0.2f, 1.0f, 0.25f));
            Glow->SetIntensity(950.0f);
            Glow->SetAttenuationRadius(240.0f);
        }
        UpdateMotionReadability(DeltaSeconds, PlayerPawn, Distance, false, false);
        return;
    }

    const bool bTelegraphingAttack =
        Distance <= AttackRange * AttackTelegraphRangeMultiplier &&
        TimeSinceAttack >= FMath::Max(0.0f, AttackCooldown - AttackTelegraphLeadSeconds);
    UpdateStandardPursuitReadability(DeltaSeconds, PlayerPawn, Distance, bTelegraphingAttack);
    UpdateMotionReadability(DeltaSeconds, PlayerPawn, Distance, bTelegraphingAttack, false);
    if (Glow)
    {
        Glow->SetLightColor(bTelegraphingAttack ? FLinearColor(1.0f, 0.10f, 0.02f) : FLinearColor(0.2f, 1.0f, 0.25f));
        Glow->SetIntensity(bTelegraphingAttack ? 5200.0f : 950.0f);
        Glow->SetAttenuationRadius(bTelegraphingAttack ? 460.0f : 240.0f);
    }

    // #29 — elite-variant behaviors get a chance to run before regular chase.
    if (TickEliteBehavior(DeltaSeconds, PlayerPawn, Distance))
    {
        return;
    }

    FHitResult BarricadeHit;
    if (ABarricadeActor* BlockingBarricade = FindBlockingBarricadeBetween(this, PlayerPawn, BarricadeHit))
    {
        const FVector BarricadePoint = BarricadeHit.ImpactPoint.IsNearlyZero()
            ? BlockingBarricade->GetActorLocation()
            : FVector(BarricadeHit.ImpactPoint);
        const float BarricadeDistance = FVector::Dist2D(GetActorLocation(), BarricadePoint);
        if (BarricadeDistance <= AttackRange * 1.45f && TimeSinceAttack > FMath::Max(0.75f, AttackCooldown - 0.15f))
        {
            TimeSinceAttack = 0.0f;
            FaceMovementTarget(BlockingBarricade->GetActorLocation(), DeltaSeconds);
            TriggerAttackMotionCue();
            if (AttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
            {
                GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage);
            }
            PlayAuthoredOneShot(AuthoredAttackAnim, true);   // v3 authored swipe
            if (AttackCue)
            {
                UGameplayStatics::PlaySoundAtLocation(this, AttackCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
            }
            PushThreatCaption(TEXT("strikes cover"), 2800.0f, 2.5f);
            const FVector Direction = (BlockingBarricade->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            BlockingBarricade->TakeBarricadeDamage(AttackDamage * 1.18f, BarricadePoint, Direction, this);
            Tags.AddUnique(FName("ZombieAttackedDestructibleCover"));
            return;
        }
    }

    if (Distance > 15.0f)
    {
        const FVector Direction = ToPlayer.GetSafeNormal();
        FaceMovementTarget(PlayerPawn->GetActorLocation(), DeltaSeconds);
        if (!Cast<ACodeRescueAIController>(GetController()))
        {
            // Keep directly advancing even if the AIController is unavailable.
            AddMovementInput(Direction, 1.0f);
        }
    }

    if (Distance <= AttackRange && TimeSinceAttack > AttackCooldown)
    {
        TimeSinceAttack = 0.0f;
        if (bStandardDirectPursuitEnabled)
        {
            Tags.AddUnique(FName("StandardPursuitAttackCommit"));
        }
        // Attack visual + audio
        TriggerAttackMotionCue();
        if (AttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
        {
            GetMesh()->GetAnimInstance()->Montage_Play(AttackMontage);
        }
        PlayAuthoredOneShot(AuthoredAttackAnim, true);   // v3 authored swipe
        if (AttackCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, AttackCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
        }
        PushThreatCaption(TEXT("attack"), 2600.0f, 2.25f);
        if (ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(PlayerPawn))
        {
            Character->ApplyDamage(AttackDamage, this);   // pass self as instigator for #21 wiring
        }
    }
}

void ACodeZombieActor::InitializeFromVariant(EZombieVariant InVariant, const FZombieVariantRow& Row)
{
    Variant = InVariant;

    // Stat multipliers stack on top of whatever GameMode set at spawn time
    // (which already includes the EGameDifficulty multiplier). Clamp the
    // variant Health/Damage multipliers so a tanky variant on Hard doesn't
    // produce a 200 HP zombie that takes a magazine to drop. Speed is left
    // uncapped — a fast zombie should genuinely feel fast on Hard.
    //   [item 18 in roadmap; see Documentation/zombie_system/18_difficulty_cap.md]
    const float ClampedHealthMul = FMath::Clamp(Row.HealthMultiplier, 0.4f, 1.30f);
    const float ClampedDamageMul = FMath::Clamp(Row.DamageMultiplier, 0.5f, 1.40f);
    Health       *= ClampedHealthMul;
    AttackDamage *= ClampedDamageMul;
    MoveSpeed    *= Row.SpeedMultiplier;
    RefreshMovementSettings();

    // Soft-load the skeletal mesh + AnimBP synchronously here. SpawnWorld
    // already runs once at level start so the brief hitch is acceptable, and
    // it keeps the BeginPlay code path simple (it just checks the now-resolved
    // ProfessionalZombieMesh / ProfessionalZombieAnimClass pointers).
    if (!Row.SkeletalMesh.IsNull())
    {
        if (USkeletalMesh* Mesh = Row.SkeletalMesh.LoadSynchronous())
        {
            ProfessionalZombieMesh = Mesh;
        }
    }
    if (!Row.AnimBPClass.IsNull())
    {
        if (UClass* AnimClass = Row.AnimBPClass.LoadSynchronous())
        {
            ProfessionalZombieAnimClass = AnimClass;
        }
    }
    ApplyProfessionalVisuals();

    if (GetMesh() && Row.MeshScale > 0.0f && FMath::Abs(Row.MeshScale - 1.0f) > KINDA_SMALL_NUMBER)
    {
        GetMesh()->SetRelativeScale3D(FVector(Row.MeshScale));
    }
    CacheMotionReadabilityBasePose(true);

    // Resolve montage + audio soft refs synchronously. SpawnWorld runs once
    // at level start, so the brief hitch is acceptable. nullptr is fine for
    // any of these — playback paths early-out gracefully.
    if (!Row.HitReactMontage.IsNull())
    {
        HitReactMontage = Row.HitReactMontage.LoadSynchronous();
    }
    if (!Row.DeathMontage.IsNull())
    {
        DeathMontage = Row.DeathMontage.LoadSynchronous();
    }
    if (!Row.AttackMontage.IsNull())
    {
        AttackMontage = Row.AttackMontage.LoadSynchronous();
    }
    if (!Row.GrowlCue.IsNull())
    {
        GrowlCue = Row.GrowlCue.LoadSynchronous();
    }
    if (!Row.AttackCue.IsNull())
    {
        AttackCue = Row.AttackCue.LoadSynchronous();
    }
    if (!Row.DeathCue.IsNull())
    {
        DeathCue = Row.DeathCue.LoadSynchronous();
    }
}

void ACodeZombieActor::ApplyRescuePointDamage(
    float DamageAmount,
    EHitZone HitZone,
    const FVector& ImpactPoint,
    const FVector& ShotDirection,
    FName ImpactBone)
{
    if (bIsDying || DamageAmount <= 0.0f)
    {
        return;
    }

    LastImpactPoint = ImpactPoint;
    LastIncomingShotDirection = ShotDirection.GetSafeNormal();
    LastImpactBone = ImpactBone;
    bApplyingPointDamage = true;
    SpawnLocalizedWound(HitZone, ImpactPoint, ShotDirection, ImpactBone);
    ApplyRescueDamage(DamageAmount, HitZone);
    bApplyingPointDamage = false;
}

void ACodeZombieActor::SpawnLocalizedWound(
    EHitZone HitZone,
    const FVector& ImpactPoint,
    const FVector& ShotDirection,
    FName ImpactBone)
{
    if (SpawnedWoundCount >= 8 || !GetWorld())
    {
        return;
    }

    USceneComponent* AttachParent = nullptr;
    if (GetMesh() && GetMesh()->GetSkinnedAsset() && GetMesh()->IsVisible())
    {
        AttachParent = GetMesh();
        if (ImpactBone == NAME_None || GetMesh()->GetBoneIndex(ImpactBone) == INDEX_NONE)
        {
            const FName ZoneCandidates[] = {
                HitZone == EHitZone::Head ? FName(TEXT("head")) :
                HitZone == EHitZone::Limb ? FName(TEXT("thigh_r")) : FName(TEXT("spine_03")),
                HitZone == EHitZone::Head ? FName(TEXT("neck_01")) : FName(TEXT("spine_02")),
                FName(TEXT("pelvis"))
            };
            for (const FName Candidate : ZoneCandidates)
            {
                if (GetMesh()->GetBoneIndex(Candidate) != INDEX_NONE)
                {
                    ImpactBone = Candidate;
                    break;
                }
            }
        }
    }
    else
    {
        AttachParent = Body ? static_cast<USceneComponent*>(Body) : static_cast<USceneComponent*>(RootComponent);
        ImpactBone = NAME_None;
    }
    if (!AttachParent)
    {
        return;
    }

    const FVector SafeImpact = ImpactPoint.IsNearlyZero()
        ? GetActorLocation() + FVector(0.0f, 0.0f, HitZone == EHitZone::Head ? 72.0f : 35.0f)
        : ImpactPoint;
    const FVector Incoming = ShotDirection.IsNearlyZero() ? -GetActorForwardVector() : ShotDirection.GetSafeNormal();
    const FRotator WoundRotation = (-Incoming).Rotation();
    bool bSpawnedVisual = false;

    if (UMaterialInterface* BloodDecal = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/YI_ModularZombies/Materials/Master/Instances/MI_BloodSplatter_01_Decal.MI_BloodSplatter_01_Decal")))
    {
        if (UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
                BloodDecal,
                HitZone == EHitZone::Head ? FVector(8.0f, 13.0f, 13.0f) : FVector(10.0f, 17.0f, 17.0f),
                AttachParent,
                ImpactBone,
                SafeImpact + Incoming * 1.5f,
                WoundRotation,
                EAttachLocation::KeepWorldPosition,
                0.0f))
        {
            Decal->ComponentTags.AddUnique(FName("LocalizedBulletWoundDecal"));
            bSpawnedVisual = true;
        }
    }

    if (UStaticMesh* WoundMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/CodeRescueArt/FirstLevelV4/WoundCavityV4/WoundCavityV4/StaticMeshes/WoundCavityV4.WoundCavityV4")))
    {
        UStaticMeshComponent* Wound = NewObject<UStaticMeshComponent>(this);
        AddInstanceComponent(Wound);
        Wound->SetStaticMesh(WoundMesh);
        Wound->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Wound->SetGenerateOverlapEvents(false);
        Wound->SetCastShadow(false);
        Wound->ComponentTags.AddUnique(FName("LocalizedBulletWoundCavity"));
        Wound->RegisterComponent();
        Wound->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepWorldTransform, ImpactBone);
        Wound->SetWorldLocation(SafeImpact + Incoming * 1.0f);
        Wound->SetWorldRotation(WoundRotation);
        const float ZoneScale = HitZone == EHitZone::Head ? 0.72f : HitZone == EHitZone::Limb ? 0.82f : 1.0f;
        Wound->SetWorldScale3D(FVector(ZoneScale));
        bSpawnedVisual = true;
    }

    if (bSpawnedVisual)
    {
        ++SpawnedWoundCount;
        Tags.AddUnique(FName("LocalizedAnatomicalWeaponWounds"));
        UE_LOG(LogTemp, Display,
            TEXT("[AnatomicalWound] zombie=%s zone=%d bone=%s point=%s count=%d"),
            *GetName(),
            static_cast<int32>(HitZone),
            *ImpactBone.ToString(),
            *SafeImpact.ToCompactString(),
            SpawnedWoundCount);
    }
}

void ACodeZombieActor::ReleaseRagdollBudget()
{
    if (bCountedActiveRagdoll)
    {
        GCodeRescueActiveRagdollCorpses = FMath::Max(0, GCodeRescueActiveRagdollCorpses - 1);
        bCountedActiveRagdoll = false;
    }
}

void ACodeZombieActor::BeginCorpseFade()
{
    if (bCorpseFadeActive || !bIsDying)
    {
        return;
    }

    ReleaseRagdollBudget();
    bCorpseFadeActive = true;
    CorpseFadeElapsed = 0.0f;
    CorpseFadeActorStart = GetActorLocation();
    CorpseFadeActorScale = GetActorScale3D();

    USkeletalMeshComponent* SourceCorpseMesh = GetMesh();
    if (!FrozenCorpsePose && SourceCorpseMesh && SourceCorpseMesh->GetSkeletalMeshAsset() &&
        SourceCorpseMesh->IsVisible())
    {
        FrozenCorpsePose = NewObject<UPoseableMeshComponent>(this, TEXT("FrozenCorpsePose"));
        AddInstanceComponent(FrozenCorpsePose);
        FrozenCorpsePose->SetSkinnedAssetAndUpdate(SourceCorpseMesh->GetSkeletalMeshAsset());
        FrozenCorpsePose->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        FrozenCorpsePose->SetGenerateOverlapEvents(false);
        FrozenCorpsePose->SetCastShadow(SourceCorpseMesh->CastShadow);
        FrozenCorpsePose->RegisterComponent();
        FrozenCorpsePose->SetWorldTransform(SourceCorpseMesh->GetComponentTransform());
        FrozenCorpsePose->CopyPoseFromSkeletalComponent(SourceCorpseMesh);
        for (int32 MaterialIndex = 0; MaterialIndex < SourceCorpseMesh->GetNumMaterials(); ++MaterialIndex)
        {
            FrozenCorpsePose->SetMaterial(MaterialIndex, SourceCorpseMesh->GetMaterial(MaterialIndex));
        }
        FrozenCorpsePose->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        FrozenCorpsePose->ComponentTags.AddUnique(FName("FrozenRagdollFadePose"));
        SourceCorpseMesh->SetVisibility(false, true);
        Tags.AddUnique(FName("CorpsePosePreservedDuringFade"));
        UE_LOG(LogTemp, Display, TEXT("[CorpseLifecycle] zombie=%s captured final skeletal pose for fade"), *GetName());
    }

    auto SettleCorpsePart = [this](UPrimitiveComponent* Component)
    {
        if (!Component)
        {
            return;
        }
        Component->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Component->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Component->SetSimulatePhysics(false);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    };
    SettleCorpsePart(GetMesh());
    SettleCorpsePart(Body);
    SettleCorpsePart(Head);
    SetActorEnableCollision(false);
    Tags.AddUnique(FName("CorpseVisibleThenGradualFade"));
    Tags.AddUnique(FName("CorpseFadeLifecycleActive"));
    UE_LOG(LogTemp, Display,
        TEXT("[CorpseLifecycle] zombie=%s visible-settle complete; %.1fs gradual sink/fade started"),
        *GetName(), CorpseFadeDuration);
}

void ACodeZombieActor::UpdateCorpseFade(float DeltaSeconds)
{
    if (!bCorpseFadeActive)
    {
        return;
    }

    CorpseFadeElapsed += DeltaSeconds;
    const float Alpha = FMath::Clamp(CorpseFadeElapsed / FMath::Max(0.25f, CorpseFadeDuration), 0.0f, 1.0f);
    const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
    SetActorLocation(CorpseFadeActorStart - FVector(0.0f, 0.0f, 72.0f * SmoothAlpha));
    SetActorScale3D(FMath::Lerp(CorpseFadeActorScale, CorpseFadeActorScale * 0.16f, SmoothAlpha));

    if (Alpha >= 1.0f)
    {
        UE_LOG(LogTemp, Display, TEXT("[CorpseLifecycle] zombie=%s fade complete; removing corpse"), *GetName());
        Destroy();
    }
}

void ACodeZombieActor::ApplyRescueDamage(float DamageAmount, EHitZone HitZone)
{
    if (bIsDying)
    {
        // Already in the dying-->destroy pipeline; ignore further hits so we
        // don't replay the death anim or double-mark in the save system.
        return;
    }

    if (!bApplyingPointDamage)
    {
        LastIncomingShotDirection = -GetActorForwardVector();
        LastImpactPoint = GetActorLocation() + FVector(0.0f, 0.0f, HitZone == EHitZone::Head ? 72.0f : 35.0f);
        LastImpactBone = NAME_None;
    }

    // Apply headshot multiplier
    float FinalDamage = DamageAmount;
    if (HitZone == EHitZone::Head)
    {
        FinalDamage = DamageAmount * 2.5f; // 2.5x multiplier for headshots
    }
    else if (HitZone == EHitZone::Limb)
    {
        FinalDamage = DamageAmount * 0.75f; // 0.75x multiplier for limbs
    }

    Health -= FinalDamage;

    if (Health > 0.0f)
    {
        ApplyHitReadabilityImpulse(HitZone, FinalDamage);
        TriggerHitReactionMotionCue(HitZone, FinalDamage);
    }

    // Trigger stagger on high-damage hits (>15 damage)
    if (FinalDamage > 15.0f)
    {
        if (ACodeRescueAIController* AIController = Cast<ACodeRescueAIController>(GetController()))
        {
            AIController->EnterStagger();
        }
    }

    if (HitVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, LastImpactPoint);
    }

    // Hit-react flinch on a non-fatal hit so the player gets clear visual
    // feedback that the shot landed.
    if (Health > 0.0f && HitReactMontage && GetMesh() && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Play(HitReactMontage);
    }
    // v3 authored bodies: play the keyed flinch when the physical-animation
    // layer didn't take the hit (montages don't run on single-node meshes).
    else if (Health > 0.0f && !bPhysicalHitReactionActive)
    {
        PlayAuthoredOneShot(AuthoredFlinchAnim, true);
    }

    if (Health <= 0.0f)
    {
        bIsDying = true;

        // #29 — Boomer death effect fires before generic cleanup so it can
        // see the still-valid world position.
        if (Variant == EZombieVariant::EliteBoomer)
        {
            OnBoomerDeath();
        }

        // Stop ambient growls; clear timer.
        GetWorldTimerManager().ClearTimer(GrowlTimer);

        if (DeathVFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DeathVFX, GetActorLocation() + FVector(0, 0, 75));
        }
        if (DeathCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, DeathCue, GetMonoSafeSoundLocation(this, GetActorLocation()), GetRuntimeSfxVolume(this));
        }
        PushThreatCaption(TEXT("down"), 2600.0f, 1.8f);
        if (IsValid(VisualMarkerActor))
        {
            VisualMarkerActor->Destroy();
            VisualMarkerActor = nullptr;
        }

        // Mark + save IMMEDIATELY so the gameplay state is consistent even
        // if the player quits or dies during the death animation.
        if (ZombieId >= 0)
        {
            if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
            {
                GI->MarkZombieNeutralized(ZombieId);
                GI->SavePersistentRun();
            }
            if (ACodeRescueGameMode* GameMode = GetWorld()->GetAuthGameMode<ACodeRescueGameMode>())
            {
                GameMode->SpawnZombieDeathSupply(ZombieId, GetActorLocation());
            }
        }

        const bool bActivatedDeathPhysics =
            TryActivateDeathRagdoll(HitZone, FinalDamage) ||
            ActivatePrimitiveDeathPhysics(HitZone, FinalDamage);

        // Corpses remain readable in the encounter before a slow sink/fade.
        float Delay = FMath::Max(5.0f, RagdollCorpseLifetime);
        if (!bActivatedDeathPhysics && AuthoredDeathAnim && bUsingV2ZombieBody && GetMesh())
        {
            // v3 authored collapse: fills the gap when the ragdoll budget is
            // spent (authored bodies hide the primitive Body/Head, so the
            // primitive-corpse path can never take them).
            PlayAuthoredOneShot(AuthoredDeathAnim, false);
            Delay = FMath::Max(Delay, AuthoredDeathAnim->GetPlayLength() + 0.5f);
            Tags.AddUnique(FName("ZombieDeathAuthoredCollapse"));
        }
        else if (!bActivatedDeathPhysics && DeathMontage && GetMesh() && GetMesh()->GetAnimInstance())
        {
            const float Played = GetMesh()->GetAnimInstance()->Montage_Play(DeathMontage);
            if (Played > 0.0f)
            {
                Delay = FMath::Max(Delay, Played);
            }
        }
        Tags.AddUnique(bActivatedDeathPhysics ? FName("ZombieDeathPhysicsActive") : FName("ZombieDeathMontageFallback"));

        // Disable collision so the corpse-in-progress doesn't keep blocking
        // the player or pals during the animation.
        if (!bActivatedDeathPhysics && Body)
        {
            Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        GetWorldTimerManager().SetTimer(
            DeathDestroyTimer,
            this,
            &ACodeZombieActor::BeginCorpseFade,
            Delay, /*bLoop=*/false);
        Tags.AddUnique(FName("CorpsePersistenceWindow"));
        UE_LOG(LogTemp, Display,
            TEXT("[CorpseLifecycle] zombie=%s remains visible %.1fs then fades %.1fs impulse=%s"),
            *GetName(), Delay, CorpseFadeDuration,
            *ComputeDeathPhysicsImpulse(HitZone, FinalDamage, RagdollImpulseStrength).ToCompactString());
    }
}

#undef GetRuntimeSfxVolume
#undef GetMonoSafeSoundLocation

// ---- #29 elite-variant behavior -------------------------------------------
bool ACodeZombieActor::TickEliteBehavior(float DeltaSeconds, APawn* PlayerPawn, float DistanceToPlayer)
{
    if (!PlayerPawn) return false;
    TimeSinceEliteAbility += DeltaSeconds;
    UWorld* W = GetWorld();
    if (!W) return false;
    if (IsPawnInsideProtectedLearningZone(PlayerPawn))
    {
        Tags.AddUnique(FName("NoZombieLearningZoneRespected"));
        return true;
    }

    switch (Variant)
    {
    case EZombieVariant::EliteSpitter:
    {
        // Ranged acid: every 3.5s when in 350-1500u range and visible, do
        // a hitscan trace and apply 12 damage with no headshot multiplier.
        if (DistanceToPlayer < 350.0f || DistanceToPlayer > 1500.0f) return false;
        if (TimeSinceEliteAbility < 3.5f) return false;
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(SpitterAcid), false, this);
        const FVector Eye = GetActorLocation() + FVector(0, 0, 80);
        const FVector PlayerEye = PlayerPawn->GetActorLocation() + FVector(0, 0, 60);
        if (!W->LineTraceSingleByChannel(Hit, Eye, PlayerEye, CodeRescueCollision::WeaponTrace, Params))
        {
            return false;   // not visible
        }
        TimeSinceEliteAbility = 0.0f;
        if (ACodeRescueCharacter* C = Cast<ACodeRescueCharacter>(PlayerPawn))
        {
            C->ApplyDamage(12.0f, this);
        }
        PushThreatCaption(TEXT("acid spit"), 3800.0f, 2.0f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Spitter spits acid!"));
        }
        return true;
    }
    case EZombieVariant::EliteCharger:
    {
        // Sprint dash: every 6s, if in 600-2500u range, dash forward for 1.5s
        // at 2.5x speed. On contact (within 200u), knockback the player.
        if (DistanceToPlayer < 200.0f)
        {
            // Knockback hit
            if (TimeSinceEliteAbility >= 0.5f)   // small cooldown so we don't spam
            {
                TimeSinceEliteAbility = 0.0f;
                if (ACodeRescueCharacter* C = Cast<ACodeRescueCharacter>(PlayerPawn))
                {
                    C->ApplyDamage(20.0f, this);
                    const FVector KB = (C->GetActorLocation() - GetActorLocation()).GetSafeNormal2D() * 800.0f + FVector(0, 0, 400);
                    C->LaunchCharacter(KB, true, true);
                }
            }
            return true;
        }
        if (DistanceToPlayer > 600.0f && DistanceToPlayer < 2500.0f && TimeSinceEliteAbility > 6.0f)
        {
            // Begin a dash burst: temporarily multiply MaxWalkSpeed for 1.5s.
            TimeSinceEliteAbility = 0.0f;
            GetCharacterMovement()->MaxWalkSpeed = MoveSpeed * 2.5f;
            PushThreatCaption(TEXT("charging"), 3600.0f, 2.0f);
            FTimerHandle TmpHandle;
            GetWorldTimerManager().SetTimer(TmpHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                if (!bIsDying) GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
            }), 1.5f, false);
        }
        return false;
    }
    default:
        return false;
    }
}

void ACodeZombieActor::OnBoomerDeath()
{
    UWorld* W = GetWorld();
    if (!W) return;
    const FVector Center = GetActorLocation();
    PushThreatCaption(TEXT("explosion releases small infected"), 4200.0f, 1.0f);

    // AoE damage to player if in 600u radius.
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(W, 0))
    {
        const float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), Center);
        if (Dist <= 600.0f)
        {
            if (IsPawnInsideProtectedLearningZone(PlayerPawn))
            {
                Tags.AddUnique(FName("NoZombieLearningZoneRespected"));
            }
            else
            {
                const float DmgScale = 1.0f - (Dist / 600.0f);
                if (ACodeRescueCharacter* C = Cast<ACodeRescueCharacter>(PlayerPawn))
                {
                    C->ApplyDamage(40.0f * DmgScale, this);
                }
            }
        }
    }

    // Spawn 3 small zombies (Default variant, half stats) in a triangle.
    for (int32 i = 0; i < 3; ++i)
    {
        const float Angle = (i / 3.0f) * 2.0f * PI;
        const FVector Loc = Center + FVector(FMath::Cos(Angle) * 250.0f, FMath::Sin(Angle) * 250.0f, 50.0f);
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        ACodeZombieActor* Spawn = W->SpawnActor<ACodeZombieActor>(ACodeZombieActor::StaticClass(), Loc, FRotator::ZeroRotator, SpawnParams);
        if (Spawn)
        {
            Spawn->ZombieId = 200000 + ZombieId * 10 + i;
            Spawn->Health = 25.0f;
            Spawn->AttackDamage = 5.0f;
            Spawn->MoveSpeed = MoveSpeed * 0.7f;
            Spawn->AttackRange = AttackRange;
            Spawn->ActivationRange = ActivationRange;
            Spawn->Variant = EZombieVariant::Default;
            Spawn->Tags.AddUnique(FName("ZombieFamilyVariantRuntime"));
            Spawn->Tags.AddUnique(FName("CityZombieFamilyVariant"));
            Spawn->Tags.AddUnique(FName("ZombieFamily_Default"));
            Spawn->Tags.AddUnique(FName("EliteBoomerSpawnFamily"));
            Spawn->Tags.AddUnique(FName("Top50Recommendations"));
            Spawn->SetActorScale3D(FVector(0.6f, 0.6f, 0.6f));
            Spawn->RefreshMovementSettings();
            Spawn->ApplyStandardDirectPursuitProfile();
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("BOOMER EXPLODES — small zombies released"));
    }
}
