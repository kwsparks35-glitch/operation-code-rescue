#include "CompanionActor.h"
#include "CodeRescueAnimationBudget.h"
#include "CodeRescueRetargetRig.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameMode.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CodeZombieActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
FString GetFirstName(const FString& DisplayName)
{
    FString Left;
    FString Right;
    if (DisplayName.Split(TEXT(" "), &Left, &Right) && !Left.IsEmpty())
    {
        return Left;
    }
    return DisplayName.IsEmpty() ? FString(TEXT("Team")) : DisplayName;
}

FString GetRoleCode(const FString& RoleLabel)
{
    const FString UpperRole = RoleLabel.ToUpper();
    if (UpperRole.Contains(TEXT("MEDIC"))) return TEXT("MED");
    if (UpperRole.Contains(TEXT("ENGINEER"))) return TEXT("ENG");
    if (UpperRole.Contains(TEXT("SCOUT"))) return TEXT("SCT");
    if (UpperRole.Contains(TEXT("HEAVY"))) return TEXT("HVY");
    return TEXT("RIF");
}

FString GetRoleDutyShort(const FString& RoleLabel)
{
    const FString UpperRole = RoleLabel.ToUpper();
    if (UpperRole.Contains(TEXT("MEDIC"))) return TEXT("triage");
    if (UpperRole.Contains(TEXT("ENGINEER"))) return TEXT("route");
    if (UpperRole.Contains(TEXT("SCOUT"))) return TEXT("flank");
    if (UpperRole.Contains(TEXT("HEAVY"))) return TEXT("guard");
    return TEXT("overwatch");
}

FString GetSupportFireBark(const FString& DisplayName, const FString& RoleLabel, const FString& BarkStyle)
{
    if (RoleLabel.Contains(TEXT("Medic")))
    {
        return FString::Printf(TEXT("[%s | Medic]: Vitals covered. %s"), *DisplayName, *BarkStyle);
    }
    if (RoleLabel.Contains(TEXT("Engineer")))
    {
        return FString::Printf(TEXT("[%s | Engineer]: Route lane clear. %s"), *DisplayName, *BarkStyle);
    }
    if (RoleLabel.Contains(TEXT("Scout")))
    {
        return FString::Printf(TEXT("[%s | Scout]: Contact marked on the flank. %s"), *DisplayName, *BarkStyle);
    }
    if (RoleLabel.Contains(TEXT("Heavy")))
    {
        return FString::Printf(TEXT("[%s | Heavy]: Rear guard holding. %s"), *DisplayName, *BarkStyle);
    }
    return FString::Printf(TEXT("[%s | Rifle]: Support fire on target. %s"), *DisplayName, *BarkStyle);
}

FString GetRoleOrderResponse(const FString& DisplayName, const FString& RoleLabel, const FString& OrderLabel)
{
    const FString UpperRole = RoleLabel.ToUpper();
    const FString UpperOrder = OrderLabel.ToUpper();
    if (UpperRole.Contains(TEXT("MEDIC")))
    {
        return UpperOrder.Contains(TEXT("HOLD"))
            ? FString::Printf(TEXT("[%s | Medic]: Holding triage line."), *DisplayName)
            : FString::Printf(TEXT("[%s | Medic]: Moving with you. Call N if health drops."), *DisplayName);
    }
    if (UpperRole.Contains(TEXT("ENGINEER")))
    {
        return UpperOrder.Contains(TEXT("HOLD"))
            ? FString::Printf(TEXT("[%s | Engineer]: Holding the access lane."), *DisplayName)
            : FString::Printf(TEXT("[%s | Engineer]: Route tools packed, following."), *DisplayName);
    }
    if (UpperRole.Contains(TEXT("SCOUT")))
    {
        return UpperOrder.Contains(TEXT("TIGHT"))
            ? FString::Printf(TEXT("[%s | Scout]: Pulling in from wide watch."), *DisplayName)
            : FString::Printf(TEXT("[%s | Scout]: Flank watch online."), *DisplayName);
    }
    if (UpperRole.Contains(TEXT("HEAVY")))
    {
        return UpperOrder.Contains(TEXT("WIDE"))
            ? FString::Printf(TEXT("[%s | Heavy]: Wide rear guard set."), *DisplayName)
            : FString::Printf(TEXT("[%s | Heavy]: Guarding the rear."), *DisplayName);
    }
    return FString::Printf(TEXT("[%s | Rifle]: Overwatch acknowledged."), *DisplayName);
}
}

ACompanionActor::ACompanionActor()
{
    PrimaryActorTick.bCanEverTick = true;
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionProfileName(TEXT("Pawn"));
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Capsule->SetCollisionObjectType(ECC_Pawn);
        Capsule->SetCollisionResponseToAllChannels(ECR_Block);
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CodeRescueAnimationBudget::ApplySkeletalMeshBudget(
        GetMesh(), ECodeRescueAnimationBudgetProfile::HeroNPC, this);
    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        GetMesh(), ECodeRescueRetargetRigProfile::CompanionHero, this);
    GetCharacterMovement()->MaxWalkSpeed = 700.0f;
    GetCharacterMovement()->RotationRate.Yaw = 540.0f;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseRVOAvoidance = true;
    GetCharacterMovement()->AvoidanceConsiderationRadius = 260.0f;

    RoleSignalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CompanionRoleSignalLight"));
    RoleSignalLight->SetupAttachment(RootComponent);
    RoleSignalLight->SetRelativeLocation(FVector(0.0f, 0.0f, 128.0f));
    RoleSignalLight->SetIntensity(RoleSignalLightBaseIntensity);
    RoleSignalLight->SetAttenuationRadius(430.0f);
    RoleSignalLight->SetLightColor(RoleAccentColor);

    ResponderPack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlenderAuthoredResponderPack"));
    ResponderPack->SetupAttachment(RootComponent);
    ResponderPack->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ResponderPack->SetCastShadow(true);
    ResponderPack->SetReceivesDecals(false);
    ResponderPack->SetRelativeLocation(FVector(-27.0f, 0.0f, 58.0f));
    ResponderPack->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    ResponderPack->SetRelativeScale3D(FVector(0.72f));
    ResponderPack->ComponentTags.AddUnique(FName("BlenderAuthoredResponderGear"));
}

void ACompanionActor::BeginPlay()
{
    Super::BeginPlay();

    if (USkeletalMesh* CompanionMesh = LoadObject<USkeletalMesh>(
            nullptr,
            TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn")))
    {
        GetMesh()->SetSkeletalMesh(CompanionMesh);
        const float HalfHeight = GetCapsuleComponent()
            ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
        GetMesh()->SetRelativeLocationAndRotation(
            FVector(0.0f, 0.0f, -HalfHeight), FRotator(0.0f, -90.0f, 0.0f));
        GetMesh()->SetVisibility(true, true);

        if (UClass* AnimBP = LoadClass<UAnimInstance>(
                nullptr,
                TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C")))
        {
            GetMesh()->SetAnimInstanceClass(AnimBP);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CodeRescueCharacterRoster: companion mannequin mesh missing; companion remains functional but visually reduced."));
    }

    if (ResponderPack)
    {
        if (UStaticMesh* PackMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/CodeRescueArt/WorldLootWeatherV6/ResponderPackV6/ResponderPackV6/StaticMeshes/ResponderPackV6.ResponderPackV6")))
        {
            ResponderPack->SetStaticMesh(PackMesh);
            Tags.AddUnique(FName("BlenderAuthoredResponderGear"));
            Tags.AddUnique(FName("CharacterWorldAestheticV6"));
        }
    }

    RefreshRoleSignalLight();

    // 2026-07-11 floating-character fix: companions previously relied on CMC
    // gravity alone and could hang in the air when spawned over a gap or a
    // slab that GroundFloatingMeshes later lowered. Snap at spawn like every
    // other character; the city-build pass re-grounds after geometry settles.
    ACodeRescueGameMode::SnapCharacterBaseToGround(this);
    ACodeRescueGameMode::AlignCharacterVisualFeetToCapsule(this);
    CacheCompanionGestureBasePose(true);
}

void ACompanionActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bIsDead)
    {
        UpdateCorpseFade(DeltaSeconds);
        return;
    }

    TimeSinceShot += DeltaSeconds;
    TimeSinceSupportPulse += DeltaSeconds;
    TimeSinceRoleBark += DeltaSeconds;
    SupportFireGestureTimer = FMath::Max(0.0f, SupportFireGestureTimer - DeltaSeconds);
    MedicPulseGestureTimer = FMath::Max(0.0f, MedicPulseGestureTimer - DeltaSeconds);
    OrderGestureTimer = FMath::Max(0.0f, OrderGestureTimer - DeltaSeconds);
    DamageGestureTimer = FMath::Max(0.0f, DamageGestureTimer - DeltaSeconds);
    if (MagazineAmmo == 0)
    {
        TimeSinceReload += DeltaSeconds;
        if (TimeSinceReload > 1.5f)
        {
            MagazineAmmo = MagazineSize;
            TimeSinceReload = 0.0f;
        }
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    // Follow in a staggered formation, or hold the last ordered position.
    const FVector ToPlayer = GetActorLocation() - PlayerPawn->GetActorLocation();
    if (!bHoldPosition && ToPlayer.SizeSquared2D() < FMath::Square(PersonalSpaceRadius))
    {
        FVector ClearanceDirection = ToPlayer.GetSafeNormal2D();
        if (ClearanceDirection.IsNearlyZero())
        {
            const float SideSign = LateralFollowOffset >= 0.0f ? 1.0f : -1.0f;
            ClearanceDirection = PlayerPawn->GetActorRightVector() * SideSign;
        }
        const float SideSign = LateralFollowOffset >= 0.0f ? 1.0f : -1.0f;
        const FVector SideBias = PlayerPawn->GetActorRightVector() * SideSign * 0.35f;
        AddMovementInput((ClearanceDirection + SideBias).GetSafeNormal2D(), 1.0f);
    }

    const FVector FormationTarget = bHoldPosition
        ? HoldLocation
        : PlayerPawn->GetActorLocation()
            - PlayerPawn->GetActorForwardVector() * FollowOffset
            + PlayerPawn->GetActorRightVector() * LateralFollowOffset;
    const FVector ToTarget = FormationTarget - GetActorLocation();
    if (ToTarget.SizeSquared2D() > 6000.0f * 6000.0f)
    {
        SetActorLocation(FormationTarget + FVector(0.0f, 0.0f, 30.0f), false, nullptr, ETeleportType::TeleportPhysics);
    }
    else if (ToTarget.SizeSquared2D() > 200.0f * 200.0f)
    {
        const FVector Direction = ToTarget.GetSafeNormal2D();
        SetActorRotation(Direction.Rotation());
        AddMovementInput(Direction, 1.0f);
    }
    else if (bHoldPosition)
    {
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->StopMovementImmediately();
        }
        SetActorRotation(HoldRotation);
    }

    TryFireAtNearbyZombie(PlayerPawn);
    TrySupportPlayer(PlayerPawn);
    UpdateCompanionGesture(DeltaSeconds);
}

bool ACompanionActor::RefreshGroundedVisualPose()
{
    const bool bAligned = ACodeRescueGameMode::AlignCharacterVisualFeetToCapsule(this);
    CacheCompanionGestureBasePose(true);
    return bAligned;
}

void ACompanionActor::TryFireAtNearbyZombie(APawn* PlayerPawn)
{
    if (TimeSinceShot < RefireDelay || MagazineAmmo == 0) return;

    UWorld* World = GetWorld();
    ACodeZombieActor* Best = nullptr;
    float BestDistSq = DetectionRange * DetectionRange;
    for (TActorIterator<ACodeZombieActor> It(World); It; ++It)
    {
        ACodeZombieActor* Z = *It;
        if (!Z || Z->Health <= 0.0f) continue;
        const float DistSq = FVector::DistSquared(Z->GetActorLocation(), GetActorLocation());
        if (DistSq < BestDistSq)
        {
            // Visibility check
            FHitResult Hit;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(CompanionFire), false, this);
            Params.AddIgnoredActor(Z);
            const bool bBlocked = World->LineTraceSingleByChannel(Hit, GetActorLocation() + FVector(0,0,60),
                Z->GetActorLocation() + FVector(0,0,60), ECC_Visibility, Params);
            if (!bBlocked)
            {
                Best = Z;
                BestDistSq = DistSq;
            }
        }
    }
    if (!Best) return;
    Best->ApplyRescueDamage(CombatDamage, EHitZone::Torso);
    TriggerSupportFireGesture();
    --MagazineAmmo;
    TimeSinceShot = 0.0f;
    if (TimeSinceRoleBark > 9.0f)
    {
        TimeSinceRoleBark = 0.0f;
        UCodeRescueSubtitlesWidget::Push(GetSupportFireBark(DisplayName, RoleLabel, BarkStyle), 2.2f);
    }
}

void ACompanionActor::TrySupportPlayer(APawn* PlayerPawn)
{
    if (!bMedicSupport || TimeSinceSupportPulse < MedicPulseCooldown)
    {
        return;
    }

    ACodeRescueCharacter* Player = Cast<ACodeRescueCharacter>(PlayerPawn);
    if (!Player || Player->Health <= 0.0f || Player->MaxHealth <= 0.0f)
    {
        return;
    }

    if (FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) > FMath::Square(1200.0f))
    {
        return;
    }

    if (Player->Health > Player->MaxHealth * 0.55f)
    {
        return;
    }

    Player->Health = FMath::Min(Player->MaxHealth, Player->Health + MedicHealAmount);
    TimeSinceSupportPulse = 0.0f;
    TriggerMedicPulseGesture();
    UCodeRescueSubtitlesWidget::Push(
        FString::Printf(TEXT("[%s]: Medic pulse applied. Stay moving."), *DisplayName),
        3.0f);
}

float ACompanionActor::GetMedicPulseReadySeconds() const
{
    if (!bMedicSupport)
    {
        return -1.0f;
    }
    return FMath::Max(0.0f, MedicPulseCooldown - TimeSinceSupportPulse);
}

bool ACompanionActor::TryManualMedicPulse(ACodeRescueCharacter* Player, FString& OutMessage)
{
    if (!bMedicSupport)
    {
        OutMessage = FString::Printf(TEXT("[%s]: I am not equipped for medic support."), *DisplayName);
        return false;
    }
    if (!Player || Player->Health <= 0.0f || Player->MaxHealth <= 0.0f)
    {
        OutMessage = FString::Printf(TEXT("[%s]: Medic call unavailable."), *DisplayName);
        return false;
    }

    const float ReadySeconds = GetMedicPulseReadySeconds();
    if (ReadySeconds > 0.0f)
    {
        OutMessage = FString::Printf(TEXT("[%s]: Medic kit recharging %.0fs."), *DisplayName, ReadySeconds);
        return false;
    }
    if (Player->Health >= Player->MaxHealth - 1.0f)
    {
        OutMessage = FString::Printf(TEXT("[%s]: Health is already stable."), *DisplayName);
        return false;
    }

    Player->Health = FMath::Min(Player->MaxHealth, Player->Health + MedicHealAmount);
    TimeSinceSupportPulse = 0.0f;
    TriggerMedicPulseGesture();
    OutMessage = FString::Printf(
        TEXT("[%s]: Manual medic pulse applied. Health %.0f / %.0f."),
        *DisplayName,
        Player->Health,
        Player->MaxHealth);
    return true;
}

void ACompanionActor::RegroupNearPlayer(APawn* PlayerPawn, int32 FormationIndex)
{
    if (!PlayerPawn || bIsDead)
    {
        return;
    }

    const float SpacingScale = FMath::Clamp(FormationSpacingScale, 0.72f, 1.55f);
    const int32 PairIndex = FormationIndex / 2;
    const float SideSign = (FormationIndex % 2 == 0) ? -1.0f : 1.0f;
    const float SideOffset = SideSign * (260.0f + PairIndex * 150.0f) * SpacingScale;
    const float BackOffset = (420.0f + FormationIndex * 90.0f) * SpacingScale;
    const FVector Target =
        PlayerPawn->GetActorLocation()
        - PlayerPawn->GetActorForwardVector() * BackOffset
        + PlayerPawn->GetActorRightVector() * SideOffset
        + FVector(0.0f, 0.0f, 45.0f);

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }

    SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
    SetActorRotation(PlayerPawn->GetActorRotation());
    TriggerOrderGesture();
}

void ACompanionActor::SetHoldPosition(const FVector& NewHoldLocation, const FRotator& NewHoldRotation)
{
    bHoldPosition = true;
    HoldLocation = NewHoldLocation;
    HoldRotation = NewHoldRotation;
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }
    TriggerOrderGesture();
}

void ACompanionActor::ClearHoldPosition()
{
    bHoldPosition = false;
    TriggerOrderGesture();
}

void ACompanionActor::ApplyFormationSpacingScale(float NewScale)
{
    if (!bHasCapturedBaseFormation)
    {
        BaseFollowOffset = FollowOffset;
        BaseLateralFollowOffset = LateralFollowOffset;
        BasePersonalSpaceRadius = PersonalSpaceRadius;
        bHasCapturedBaseFormation = true;
    }

    FormationSpacingScale = FMath::Clamp(NewScale, 0.72f, 1.55f);
    FollowOffset = BaseFollowOffset * FormationSpacingScale;
    LateralFollowOffset = BaseLateralFollowOffset * FormationSpacingScale;
    PersonalSpaceRadius = FMath::Clamp(BasePersonalSpaceRadius * FormationSpacingScale, 220.0f, 520.0f);

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->AvoidanceConsiderationRadius = FMath::Clamp(260.0f * FormationSpacingScale, 220.0f, 520.0f);
    }
}

void ACompanionActor::ConfigureSquadPersonality(
    const FString& InDisplayName,
    const FString& InRoleLabel,
    const FString& InMechanicalIdentity,
    const FString& InBarkStyle,
    const FLinearColor& InRoleAccentColor)
{
    DisplayName = InDisplayName;
    RoleLabel = InRoleLabel;
    MechanicalIdentity = InMechanicalIdentity;
    BarkStyle = InBarkStyle;
    RoleAccentColor = InRoleAccentColor;
    Tags.AddUnique(FName("SquadPersonalityRuntime"));
    Tags.AddUnique(FName(*FString::Printf(TEXT("CompanionRole_%s"), *GetRoleCode(RoleLabel))));
    RefreshRoleSignalLight();
}

FString ACompanionActor::GetHudCallsign() const
{
    return FString::Printf(TEXT("%s %s"), *GetFirstName(DisplayName), *GetRoleCode(RoleLabel));
}

FString ACompanionActor::GetRoleStatusLabel() const
{
    return FString::Printf(
        TEXT("%s %s %s"),
        *GetFirstName(DisplayName),
        bHoldPosition ? TEXT("hold") : TEXT("follow"),
        *GetRoleDutyShort(RoleLabel));
}

FString ACompanionActor::GetOrderResponseBark(const FString& OrderLabel) const
{
    return GetRoleOrderResponse(DisplayName, RoleLabel, OrderLabel);
}

void ACompanionActor::PushRoleOrderBark(const FString& OrderLabel, float DurationSeconds)
{
    TriggerOrderGesture();
    UCodeRescueSubtitlesWidget::Push(GetOrderResponseBark(OrderLabel), DurationSeconds);
}

void ACompanionActor::TakeCompanionDamage(float Amount)
{
    if (bIsDead) return;
    Health -= Amount;
    TriggerDamageGesture();
    if (Health <= 0.0f)
    {
        bIsDead = true;
        UCodeRescueSubtitlesWidget::Push(FString::Printf(TEXT("[%s]: ...I'm sorry. I should've—"), *DisplayName), 5.0f);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Silver,
                FString::Printf(TEXT("Companion %s has fallen."), *DisplayName));
        }
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            Movement->StopMovementImmediately();
            Movement->DisableMovement();
        }
        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        if (RoleSignalLight)
        {
            RoleSignalLight->SetIntensity(120.0f);
        }
        Tags.AddUnique(FName("NPCVisibleCorpseWindow"));
        FTimerHandle T;
        GetWorldTimerManager().SetTimer(T, this, &ACompanionActor::BeginCorpseFade, 8.0f, false);
    }
}

void ACompanionActor::BeginCorpseFade()
{
    if (!bIsDead || bCorpseFadeActive)
    {
        return;
    }
    bCorpseFadeActive = true;
    CorpseFadeElapsed = 0.0f;
    CorpseFadeStartLocation = GetActorLocation();
    Tags.AddUnique(FName("NPCGradualCorpseFade"));
    UE_LOG(LogTemp, Display, TEXT("[CorpseLifecycle] companion=%s begins %.1fs fade"), *DisplayName, CorpseFadeDuration);
}

void ACompanionActor::UpdateCorpseFade(float DeltaSeconds)
{
    if (!bCorpseFadeActive)
    {
        return;
    }
    CorpseFadeElapsed += DeltaSeconds;
    const float Alpha = FMath::Clamp(CorpseFadeElapsed / FMath::Max(0.25f, CorpseFadeDuration), 0.0f, 1.0f);
    const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
    SetActorLocation(CorpseFadeStartLocation - FVector(0.0f, 0.0f, 65.0f * SmoothAlpha));
    SetActorScale3D(FMath::Lerp(FVector::OneVector, FVector(0.18f), SmoothAlpha));
    if (RoleSignalLight)
    {
        RoleSignalLight->SetIntensity(FMath::Lerp(120.0f, 0.0f, SmoothAlpha));
    }
    if (Alpha >= 1.0f)
    {
        Destroy();
    }
}

void ACompanionActor::CacheCompanionGestureBasePose(bool bTagForAudit)
{
    if (bCompanionGestureBasePoseCached && !bTagForAudit)
    {
        return;
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshGestureBaseTransform = MeshComp->GetRelativeTransform();
        MeshComp->ComponentTags.AddUnique(FName("CompanionGestureReadabilityComponent"));
        MeshComp->ComponentTags.AddUnique(FName("CompanionFormationLocomotionPoseTarget"));
        MeshComp->ComponentTags.AddUnique(FName("CompanionOrderGestureSlotRuntime"));
    }

    if (RoleSignalLight)
    {
        RoleSignalLightBaseTransform = RoleSignalLight->GetRelativeTransform();
        RoleSignalLightBaseIntensity = RoleSignalLight->Intensity;
        RoleSignalLight->ComponentTags.AddUnique(FName("CompanionGestureReadabilityComponent"));
        RoleSignalLight->ComponentTags.AddUnique(FName("CompanionRoleSignalLightRuntime"));
    }

    bCompanionGestureBasePoseCached = true;
    if (bTagForAudit)
    {
        Tags.AddUnique(FName("CompanionGestureReadabilityRuntime"));
        Tags.AddUnique(FName("CompanionIdleFollowHoldPose"));
        Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    }
}

void ACompanionActor::UpdateCompanionGesture(float DeltaSeconds)
{
    if (!bCompanionGestureBasePoseCached)
    {
        CacheCompanionGestureBasePose(true);
    }

    if (!bEnableCompanionGestureReadability || !bCompanionGestureBasePoseCached)
    {
        return;
    }

    CompanionGesturePhase += DeltaSeconds * (bHoldPosition ? 0.78f : 1.12f);
    const float Speed = GetVelocity().Size2D();
    const float MaxSpeed = GetCharacterMovement() ? FMath::Max(1.0f, GetCharacterMovement()->MaxWalkSpeed) : 700.0f;
    const float MoveAlpha = FMath::Clamp(Speed / MaxSpeed, 0.0f, 1.0f);
    const float IdleSway = FMath::Sin(CompanionGesturePhase) * CompanionIdleGestureScale;
    const float IdleLift = FMath::Abs(FMath::Sin(CompanionGesturePhase * 0.67f)) * CompanionIdleGestureScale;
    const float SupportPulse = SupportFireGestureTimer > 0.0f
        ? FMath::Sin((1.0f - SupportFireGestureTimer / FMath::Max(0.01f, SupportFireGestureDuration)) * PI)
        : 0.0f;
    const float MedicPulse = MedicPulseGestureTimer > 0.0f
        ? FMath::Sin((1.0f - MedicPulseGestureTimer / FMath::Max(0.01f, MedicPulseGestureDuration)) * PI)
        : 0.0f;
    const float OrderPulse = OrderGestureTimer > 0.0f
        ? FMath::Sin((1.0f - OrderGestureTimer / FMath::Max(0.01f, OrderGestureDuration)) * PI)
        : 0.0f;
    const float DamagePulse = DamageGestureTimer > 0.0f
        ? FMath::Sin((1.0f - DamageGestureTimer / FMath::Max(0.01f, DamageGestureDuration)) * PI)
        : 0.0f;
    const float HoldPose = bHoldPosition ? 1.0f : 0.0f;

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        FTransform Pose = MeshGestureBaseTransform;
        Pose.SetLocation(
            MeshGestureBaseTransform.GetLocation() +
            FVector(
                -5.0f * SupportPulse + 3.0f * DamagePulse,
                1.4f * IdleSway + 2.0f * OrderPulse,
                1.8f * IdleLift + 2.5f * MoveAlpha + 6.0f * MedicPulse - 3.0f * DamagePulse));
        const FRotator BaseRot = MeshGestureBaseTransform.GetRotation().Rotator();
        Pose.SetRotation((
            BaseRot +
            FRotator(
                -5.0f * SupportPulse + 4.0f * MedicPulse - 5.0f * DamagePulse,
                1.6f * IdleSway + 5.0f * OrderPulse,
                2.5f * MoveAlpha + 2.0f * HoldPose * IdleSway)).Quaternion());
        MeshComp->SetRelativeTransform(Pose);
    }

    if (RoleSignalLight)
    {
        FTransform Pose = RoleSignalLightBaseTransform;
        Pose.SetLocation(
            RoleSignalLightBaseTransform.GetLocation() +
            FVector(0.0f, 0.0f, 4.0f * IdleLift + 9.0f * MedicPulse + 5.0f * OrderPulse));
        RoleSignalLight->SetRelativeTransform(Pose);
        const float Pulse =
            1.0f +
            0.10f * IdleLift +
            0.48f * SupportPulse +
            0.75f * MedicPulse +
            0.36f * OrderPulse -
            0.24f * DamagePulse +
            0.18f * HoldPose;
        RoleSignalLight->SetIntensity(RoleSignalLightBaseIntensity * FMath::Max(0.25f, Pulse));
    }
}

void ACompanionActor::TriggerSupportFireGesture()
{
    SupportFireGestureTimer = FMath::Max(SupportFireGestureTimer, SupportFireGestureDuration);
    Tags.AddUnique(FName("CompanionGestureReadabilityRuntime"));
    Tags.AddUnique(FName("CompanionSupportFirePose"));
}

void ACompanionActor::TriggerMedicPulseGesture()
{
    MedicPulseGestureTimer = FMath::Max(MedicPulseGestureTimer, MedicPulseGestureDuration);
    Tags.AddUnique(FName("CompanionGestureReadabilityRuntime"));
    Tags.AddUnique(FName("CompanionMedicPulsePose"));
}

void ACompanionActor::TriggerOrderGesture()
{
    OrderGestureTimer = FMath::Max(OrderGestureTimer, OrderGestureDuration);
    Tags.AddUnique(FName("CompanionGestureReadabilityRuntime"));
    Tags.AddUnique(FName("CompanionOrderAcknowledgedPose"));
}

void ACompanionActor::TriggerDamageGesture()
{
    DamageGestureTimer = FMath::Max(DamageGestureTimer, DamageGestureDuration);
    Tags.AddUnique(FName("CompanionGestureReadabilityRuntime"));
    Tags.AddUnique(FName("CompanionDamageFlinchPose"));
}

void ACompanionActor::RefreshRoleSignalLight()
{
    if (!RoleSignalLight)
    {
        return;
    }

    RoleSignalLight->SetLightColor(RoleAccentColor);
    RoleSignalLight->SetIntensity(RoleSignalLightBaseIntensity);
    Tags.AddUnique(FName("CompanionRoleSignalLightRuntime"));
}
