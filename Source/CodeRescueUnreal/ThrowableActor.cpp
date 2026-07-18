#include "ThrowableActor.h"
#include "BarricadeActor.h"
#include "CodeRescueCharacter.h"
#include "CodeRescuePhysicsStability.h"
#include "CodeZombieActor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

TArray<TWeakObjectPtr<AThrowableActor>> AThrowableActor::StaticActiveLures;

namespace
{
const TCHAR* DescribeThrowableKind(EThrowableKind Kind)
{
    switch (Kind)
    {
    case EThrowableKind::Flare:
        return TEXT("Flare");
    case EThrowableKind::Smoke:
        return TEXT("Smoke");
    case EThrowableKind::Stim:
        return TEXT("Stim");
    default:
        return TEXT("Unknown");
    }
}
}

AThrowableActor::AThrowableActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    SetRootComponent(Body);
    Body->SetWorldScale3D(FVector(0.4f, 0.4f, 0.4f));
    Body->SetCollisionProfileName(TEXT("PhysicsActor"));
    Body->SetSimulatePhysics(true);
    Body->SetLinearDamping(0.22f);
    Body->SetAngularDamping(0.28f);
    Body->SetNotifyRigidBodyCollision(true);
    Body->bReturnMaterialOnMove = true;
    Body->OnComponentHit.AddDynamic(this, &AThrowableActor::OnThrowableImpact);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Body->SetStaticMesh(SphereMesh.Object);
    }

    GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
    GlowLight->SetupAttachment(Body);
    GlowLight->SetIntensity(4000.0f);
    GlowLight->SetAttenuationRadius(800.0f);
    GlowLight->SetCastShadows(false);
}

void AThrowableActor::ConfigureGrenadePayload(ACodeRescueCharacter* InInstigator,
    uint8 InPayloadWeapon, float InFuseSeconds, const FVector& InitialVelocity)
{
    bGrenadePayload = true;
    PayloadWeapon = InPayloadWeapon;
    PayloadInstigator = InInstigator;
    Kind = EThrowableKind::Flare;          // reuse glow visuals; lure disabled below
    LureRadius = 0.0f;                     // a live grenade is not a lure
    Lifetime = FMath::Max(InFuseSeconds + 2.0f, 4.0f);

    if (Body)
    {
        if (!Body->IsSimulatingPhysics())
        {
            Body->SetSimulatePhysics(true);
        }
        // Cycle-11 review: the grenade spawned overlapping the thrower's
        // capsule, the impact handler fired a ~2000 uu/s velocity-change
        // impulse off the contact normal, and the "grenade" detonated 30 m
        // in the SKY. A live grenade must never collide with its thrower.
        if (InInstigator)
        {
            Body->IgnoreActorWhenMoving(InInstigator, true);
        }
        Body->SetEnableGravity(true);            // ballistic — matches the arc prediction
        Body->SetLinearDamping(0.01f);           // the prediction integrates drag-free
        Body->SetAngularDamping(0.6f);
        Body->SetWorldScale3D(FVector(0.13f));   // palm-size munition, not a lure boulder
        if (UMaterialInstanceDynamic* MID = Body->CreateAndSetMaterialInstanceDynamic(0))
        {
            MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.055f, 0.075f, 0.045f));
        }
        // DETERMINISTIC launch: the aim-arc prediction integrates this exact
        // initial velocity under world gravity, so the preview and the real
        // flight agree (impulse/mass launches would drift with mesh mass).
        Body->SetPhysicsLinearVelocity(InitialVelocity);
        Body->SetPhysicsAngularVelocityInDegrees(FVector(240.0f, 180.0f, 90.0f));
    }
    if (GlowLight)
    {
        // faint fuse glow instead of the flare-lure floodlight
        GlowLight->SetIntensity(650.0f);
        GlowLight->SetAttenuationRadius(220.0f);
        GlowLight->SetLightColor(FLinearColor(1.0f, 0.35f, 0.08f));
    }
    // A live grenade is ordnance, not a utility lure: no zombie-lure pulse,
    // no lure registration (BeginPlay armed both before this configure ran).
    GetWorldTimerManager().ClearTimer(UtilityPulseTimer);
    StaticActiveLures.RemoveAll([this](const TWeakObjectPtr<AThrowableActor>& W) { return W.Get() == this; });
    GetWorldTimerManager().SetTimer(GrenadeFuseTimer, this,
        &AThrowableActor::DetonateGrenade, FMath::Max(0.35f, InFuseSeconds), false);
    Tags.AddUnique(FName("LiveGrenadeProjectile"));
}

void AThrowableActor::DetonateGrenade()
{
    if (ACodeRescueCharacter* Character = PayloadInstigator.Get())
    {
        Character->DetonateGrenadePayload(GetActorLocation(), PayloadWeapon);
    }
    Destroy();
}

void AThrowableActor::LaunchThrowable(const FVector& Direction, float StrengthScale)
{
    if (!Body)
    {
        return;
    }

    if (!Body->IsSimulatingPhysics())
    {
        Body->SetSimulatePhysics(true);
    }

    const FVector Aim = Direction.IsNearlyZero()
        ? GetActorForwardVector()
        : Direction.GetSafeNormal();
    const float ClampedScale = FMath::Clamp(StrengthScale, 0.15f, 3.0f);
    const FVector LaunchImpulse = Aim * ThrowImpulseStrength * ClampedScale + FVector::UpVector * ThrowUpwardImpulse;

    Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Body->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    Body->AddImpulse(LaunchImpulse, NAME_None, true);
    Body->AddAngularImpulseInDegrees(FVector(0.0f, 0.0f, 620.0f) * ClampedScale, NAME_None, true);
    Tags.AddUnique(FName("ThrowableSubstepLaunch"));

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescuePhysicsVerticalSlice] %s throwable launched with impulse %.0f and upward boost %.0f."),
        DescribeThrowableKind(Kind),
        ThrowImpulseStrength * ClampedScale,
        ThrowUpwardImpulse);
}

void AThrowableActor::BeginPlay()
{
    Super::BeginPlay();
    CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
        Body,
        this,
        FName("ThrowableFixedStepBody"),
        1.8f,
        0.26f,
        0.34f,
        true);

    // Color and behavior depend on kind.
    switch (Kind)
    {
    case EThrowableKind::Flare:
        if (GlowLight) GlowLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.0f));   // orange
        StaticActiveLures.Add(this);
        GetWorldTimerManager().SetTimer(UtilityPulseTimer, this, &AThrowableActor::FireUtilityPulse, UtilityPulseDelay, false);
        break;
    case EThrowableKind::Smoke:
        if (GlowLight) GlowLight->SetLightColor(FLinearColor(0.6f, 0.6f, 0.6f));   // gray
        if (GlowLight) GlowLight->SetIntensity(800.0f);
        GetWorldTimerManager().SetTimer(UtilityPulseTimer, this, &AThrowableActor::FireUtilityPulse, UtilityPulseDelay, false);
        break;
    case EThrowableKind::Stim:
        if (GlowLight) GlowLight->SetLightColor(FLinearColor(0.2f, 1.0f, 0.4f));   // green
        // Stim applies its effect on contact (we apply on a 0.5s delay so
        // the player has time to see the throw arc).
        GetWorldTimerManager().SetTimer(ExpireTimer, this, &AThrowableActor::ApplyStimEffect, 0.5f, false);
        return;   // stim doesn't use the standard lifetime path
    }

    GetWorldTimerManager().SetTimer(ExpireTimer, this, &AThrowableActor::Expire, Lifetime, false);
}

void AThrowableActor::EndPlay(const EEndPlayReason::Type Reason)
{
    GetWorldTimerManager().ClearTimer(UtilityPulseTimer);
    StaticActiveLures.RemoveAll([](const TWeakObjectPtr<AThrowableActor>& W) { return !W.IsValid(); });
    StaticActiveLures.RemoveAll([this](const TWeakObjectPtr<AThrowableActor>& W) { return W.Get() == this; });
    Super::EndPlay(Reason);
}

void AThrowableActor::Expire()
{
    Destroy();
}

EPhysicalSurface AThrowableActor::ResolveImpactSurface(const FHitResult& Hit, AActor* OtherActor, UPrimitiveComponent* OtherComp) const
{
    if (Hit.PhysMaterial.IsValid())
    {
        const EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
        if (SurfaceType != SurfaceType_Default)
        {
            return SurfaceType;
        }
    }

    const auto HasSurfaceTag = [OtherActor, OtherComp](const FName& Tag)
    {
        return (OtherActor && OtherActor->Tags.Contains(Tag)) ||
            (OtherComp && OtherComp->ComponentTags.Contains(Tag));
    };

    if (HasSurfaceTag(FName("SurfaceConcrete"))) return SurfaceType1;
    if (HasSurfaceTag(FName("SurfaceMetal"))) return SurfaceType2;
    if (HasSurfaceTag(FName("SurfaceWood"))) return SurfaceType3;
    if (HasSurfaceTag(FName("SurfaceGlass"))) return SurfaceType4;
    if (HasSurfaceTag(FName("SurfaceFlesh"))) return SurfaceType5;
    if (HasSurfaceTag(FName("SurfaceDirt"))) return SurfaceType6;
    return SurfaceType_Default;
}

FLinearColor AThrowableActor::GetSurfaceImpactColor(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return FLinearColor(0.72f, 0.70f, 0.62f);
    case SurfaceType2: return FLinearColor(1.0f, 0.72f, 0.18f);
    case SurfaceType3: return FLinearColor(0.72f, 0.38f, 0.12f);
    case SurfaceType4: return FLinearColor(0.65f, 0.92f, 1.0f);
    case SurfaceType5: return FLinearColor(0.82f, 0.04f, 0.02f);
    case SurfaceType6: return FLinearColor(0.44f, 0.28f, 0.12f);
    default: return Kind == EThrowableKind::Smoke ? FLinearColor(0.70f, 0.70f, 0.70f) : FLinearColor(1.0f, 0.40f, 0.08f);
    }
}

float AThrowableActor::GetSurfaceImpactImpulseScale(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return 1.0f;
    case SurfaceType2: return 1.18f;
    case SurfaceType3: return 0.92f;
    case SurfaceType4: return 0.74f;
    case SurfaceType5: return 0.52f;
    case SurfaceType6: return 0.66f;
    default: return 0.86f;
    }
}

const TCHAR* AThrowableActor::DescribeImpactSurface(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return TEXT("Concrete");
    case SurfaceType2: return TEXT("Metal");
    case SurfaceType3: return TEXT("Wood");
    case SurfaceType4: return TEXT("Glass");
    case SurfaceType5: return TEXT("Flesh");
    case SurfaceType6: return TEXT("Dirt");
    default: return TEXT("Default");
    }
}

FName AThrowableActor::GetSurfaceImpactTag(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return FName("SurfaceImpact_Concrete");
    case SurfaceType2: return FName("SurfaceImpact_Metal");
    case SurfaceType3: return FName("SurfaceImpact_Wood");
    case SurfaceType4: return FName("SurfaceImpact_Glass");
    case SurfaceType5: return FName("SurfaceImpact_Flesh");
    case SurfaceType6: return FName("SurfaceImpact_Dirt");
    default: return FName("SurfaceImpact_Default");
    }
}

void AThrowableActor::OnThrowableImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HitComponent || !GetWorld())
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastSurfaceImpactTime < SurfaceImpactCooldown)
    {
        return;
    }

    const float ImpactSpeed = Body ? Body->GetPhysicsLinearVelocity().Size() : NormalImpulse.Size();
    if (ImpactSpeed < MinSurfaceImpactSpeed)
    {
        return;
    }
    LastSurfaceImpactTime = Now;

    const EPhysicalSurface SurfaceType = ResolveImpactSurface(Hit, OtherActor, OtherComp);
    const float SurfaceScale = GetSurfaceImpactImpulseScale(SurfaceType);
    const FVector ImpactPoint = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
    const FVector SurfaceNormal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : Hit.ImpactNormal.GetSafeNormal();

    if (ABarricadeActor* Barricade = Cast<ABarricadeActor>(OtherActor))
    {
        const float BarricadeDamage = FMath::Clamp(ImpactSpeed * 0.045f * SurfaceScale, 6.0f, 54.0f);
        const FVector ImpactDirection = Body && !Body->GetPhysicsLinearVelocity().IsNearlyZero()
            ? Body->GetPhysicsLinearVelocity().GetSafeNormal()
            : -SurfaceNormal;
        Barricade->TakeBarricadeDamage(BarricadeDamage, ImpactPoint, ImpactDirection, this);
        Tags.AddUnique(FName("ThrowableDamagedDestructibleCover"));
    }

    if (OtherComp && OtherComp != Body && OtherComp->IsSimulatingPhysics())
    {
        OtherComp->AddImpulseAtLocation(-SurfaceNormal * SurfaceImpactImpulseStrength * SurfaceScale, ImpactPoint, NAME_None);
    }
    // The self "bounce boost" is lure feel — on a live grenade it injects
    // unpredicted energy and the flight diverges from the aim-arc preview.
    if (Body && Body->IsSimulatingPhysics() && !bGrenadePayload)
    {
        Body->AddImpulse(SurfaceNormal * SurfaceImpactImpulseStrength * SurfaceScale * 0.18f, NAME_None, true);
    }

    if (GlowLight)
    {
        GlowLight->SetLightColor(GetSurfaceImpactColor(SurfaceType));
        GlowLight->SetIntensity(FMath::Clamp(ImpactSpeed * 7.0f, 900.0f, 6400.0f));
        GlowLight->SetAttenuationRadius(FMath::Clamp(ImpactSpeed * 0.7f, 240.0f, 960.0f));
    }

    Tags.AddUnique(FName("SurfaceImpactFeedback"));
    Tags.AddUnique(FName("PhysicalMaterialSurfaceReaction"));
    Tags.AddUnique(GetSurfaceImpactTag(SurfaceType));
    if (OtherActor)
    {
        OtherActor->Tags.AddUnique(FName("SurfaceImpactResponder"));
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueSurfaceImpact] %s throwable hit %s at %.0f uu/s; impulse scale %.2f."),
        DescribeThrowableKind(Kind),
        DescribeImpactSurface(SurfaceType),
        ImpactSpeed,
        SurfaceScale);
}

void AThrowableActor::FireUtilityPulse()
{
    if (bUtilityPulseFired || !GetWorld())
    {
        return;
    }
    bUtilityPulseFired = true;

    const FVector PulseOrigin = GetActorLocation();
    const float Radius = FMath::Max(100.0f, UtilityPulseRadius);
    const float KindImpulseScale =
        Kind == EThrowableKind::Smoke ? 1.25f :
        Kind == EThrowableKind::Flare ? 0.72f :
        0.55f;
    const float ImpulseStrength = UtilityPulseImpulseStrength * KindImpulseScale;

    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
    ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CodeRescueThrowableUtilityPulse), false, this);
    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        PulseOrigin,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(Radius),
        QueryParams);

    TSet<UPrimitiveComponent*> PulsedComponents;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        UPrimitiveComponent* Component = Overlap.GetComponent();
        if (!Component || Component == Body || !Component->IsSimulatingPhysics() || PulsedComponents.Contains(Component))
        {
            continue;
        }

        Component->AddRadialImpulse(PulseOrigin, Radius, ImpulseStrength, ERadialImpulseFalloff::RIF_Linear, true);
        CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
            Component,
            Overlap.GetActor(),
            FName("ThrowablePulseFixedStepTarget"),
            -1.0f,
            0.30f,
            0.42f,
            false);
        PulsedComponents.Add(Component);
    }

    const float ZombieDamage = Kind == EThrowableKind::Smoke ? SmokePulseDamage : FlarePulseDamage;
    ApplyZombieUtilityPulse(ZombieDamage);

    int32 BarricadesDamaged = 0;
    for (TActorIterator<ABarricadeActor> It(GetWorld()); It; ++It)
    {
        ABarricadeActor* Barricade = *It;
        if (!IsValid(Barricade))
        {
            continue;
        }

        const float Distance = FVector::Dist(Barricade->GetActorLocation(), PulseOrigin);
        if (Distance > Radius)
        {
            continue;
        }

        const float Falloff = FMath::Clamp(1.0f - Distance / Radius, 0.18f, 1.0f);
        const float BarricadeDamage = FMath::Max(2.0f, ZombieDamage * 0.45f * Falloff);
        const FVector Direction = (Barricade->GetActorLocation() - PulseOrigin).GetSafeNormal();
        Barricade->TakeBarricadeDamage(BarricadeDamage, PulseOrigin, Direction, this);
        ++BarricadesDamaged;
    }

    if (GEngine)
    {
        const FColor DebugColor = Kind == EThrowableKind::Smoke ? FColor::Silver : FColor(255, 140, 40);
        GEngine->AddOnScreenDebugMessage(-1, 1.8f, DebugColor,
            FString::Printf(TEXT("%s pulse: %d physics targets nudged, %d barricades stressed"),
                DescribeThrowableKind(Kind),
                PulsedComponents.Num(),
                BarricadesDamaged));
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescuePhysicsVerticalSlice] %s utility pulse radius %.0f affected %d physics components and %d destructible barricades."),
        DescribeThrowableKind(Kind),
        Radius,
        PulsedComponents.Num(),
        BarricadesDamaged);
}

void AThrowableActor::ApplyZombieUtilityPulse(float DamageAmount)
{
    if (!GetWorld() || DamageAmount <= 0.0f)
    {
        return;
    }

    const FVector PulseOrigin = GetActorLocation();
    const float RadiusSq = FMath::Square(FMath::Max(100.0f, UtilityPulseRadius));
    for (TActorIterator<ACodeZombieActor> It(GetWorld()); It; ++It)
    {
        ACodeZombieActor* Zombie = *It;
        if (!Zombie || Zombie->Health <= 0.0f)
        {
            continue;
        }

        const FVector ToZombie = Zombie->GetActorLocation() - PulseOrigin;
        const float DistanceSq = ToZombie.SizeSquared();
        if (DistanceSq > RadiusSq)
        {
            continue;
        }

        const float Distance = FMath::Sqrt(DistanceSq);
        const float Falloff = 1.0f - FMath::Clamp(Distance / UtilityPulseRadius, 0.0f, 1.0f);
        const float FinalDamage = FMath::Max(1.0f, DamageAmount * (0.45f + Falloff * 0.55f));
        Zombie->ApplyRescueDamage(FinalDamage, EHitZone::Torso);

        const FVector Away = ToZombie.GetSafeNormal2D();
        Zombie->LaunchCharacter(Away * (260.0f + Falloff * 260.0f) + FVector::UpVector * 80.0f, false, false);
    }
}

void AThrowableActor::ApplyStimEffect()
{
    if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        if (ACodeRescueCharacter* P = Cast<ACodeRescueCharacter>(Pawn))
        {
            // Refill stamina to max + small over-heal of health.
            P->Stamina = P->MaxStamina;
            P->Health = FMath::Min(P->MaxHealth, P->Health + 25.0f);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Stim: stamina refilled, +25 HP"));
            }
        }
    }
    Destroy();
}
