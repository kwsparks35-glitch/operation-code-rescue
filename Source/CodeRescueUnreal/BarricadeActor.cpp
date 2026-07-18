#include "BarricadeActor.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescuePhysicsStability.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ABarricadeActor::ABarricadeActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarricadeBody"));
    SetRootComponent(Body);
    // 200u wide doorway-blocker, 100u tall, thin.
    Body->SetWorldScale3D(FVector(2.0f, 0.3f, 1.0f));
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Body->SetCollisionObjectType(CodeRescueCollision::CoverObject);
    Body->SetCollisionResponseToChannel(CodeRescueCollision::WeaponTrace, ECR_Block);
    Body->SetCollisionResponseToChannel(CodeRescueCollision::AISightTrace, ECR_Block);
    Body->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Ignore);
    Body->SetLinearDamping(0.30f);
    Body->SetAngularDamping(0.45f);
    Body->SetNotifyRigidBodyCollision(true);
    Body->bReturnMaterialOnMove = true;
    Body->OnComponentHit.AddDynamic(this, &ABarricadeActor::OnBarricadeHit);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Body->SetStaticMesh(CubeMesh.Object);
    }

    Tags.Add(FName("Barricade"));
    Tags.Add(FName("DestructibleCover"));
    Tags.Add(FName("ChaosDestructionFallback"));
    Tags.Add(FName("SurfaceWood"));
    Tags.Add(FName("GamePhysicsDeepDive"));
    Tags.Add(FName("WorldDevelopmentDeepDive"));
    Tags.Add(FName("MacPhysicsBudgetReviewGate"));
    Tags.Add(FName("CollisionChannel_CoverObject"));
}

void ABarricadeActor::BeginPlay()
{
    Super::BeginPlay();

    MaxHealth = FMath::Max(1.0f, MaxHealth);
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);

    // Brown wooden tint so it reads as a barricade, not a generic block.
    if (Body)
    {
        CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
            Body,
            this,
            FName("CoverFixedStepBody"),
            60.0f,
            0.30f,
            0.45f,
            false);
        BodyMaterial = Body->CreateAndSetMaterialInstanceDynamic(0);
        UpdateDamageStateVisual();
    }

    if (Lifetime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(ExpireTimer, this, &ABarricadeActor::Expire, Lifetime, false);
    }
}

void ABarricadeActor::Expire()
{
    Destroy();
}

float ABarricadeActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const FVector SourceLocation = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation() - GetActorForwardVector() * 120.0f;
    const FVector ImpulseDirection = (GetActorLocation() - SourceLocation).GetSafeNormal();
    TakeBarricadeDamage(DamageAmount, GetActorLocation(), ImpulseDirection, DamageCauser);
    return FMath::Max(0.0f, DamageAmount);
}

void ABarricadeActor::TakeBarricadeDamage(float DamageAmount, const FVector& ImpactPoint, const FVector& ImpulseDirection, AActor* DamageSource)
{
    if (bBroken || DamageAmount <= 0.0f)
    {
        return;
    }

    Health = FMath::Max(0.0f, Health - DamageAmount);
    Tags.AddUnique(FName("BarricadeDamaged"));
    Tags.AddUnique(FName("DestructibleCoverDamaged"));
    Tags.AddUnique(FName("ReadableDamageState"));

    const FVector SafeImpactPoint = ImpactPoint.IsNearlyZero() ? GetActorLocation() : ImpactPoint;
    const FVector SafeImpulseDirection = ImpulseDirection.IsNearlyZero()
        ? GetActorForwardVector()
        : ImpulseDirection.GetSafeNormal();

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueDestructibleCover] Barricade '%s' took %.1f damage from %s; health %.1f / %.1f."),
        *GetName(),
        DamageAmount,
        DamageSource ? *DamageSource->GetName() : TEXT("world impact"),
        Health,
        MaxHealth);

    if (Health <= 0.0f)
    {
        BreakApart(SafeImpactPoint, SafeImpulseDirection, DamageSource);
        return;
    }

    UpdateDamageStateVisual();
    if (DamageAmount >= MinDamageToChip)
    {
        SpawnDebrisChunk(0, SafeImpactPoint, SafeImpulseDirection);
    }
}

void ABarricadeActor::UpdateDamageStateVisual()
{
    if (!BodyMaterial)
    {
        return;
    }

    const float HealthPct = MaxHealth > 0.0f ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
    FLinearColor Tint(0.45f, 0.30f, 0.18f);
    float EmissiveScale = 0.05f;
    if (HealthPct <= 0.25f)
    {
        Tint = FLinearColor(0.82f, 0.22f, 0.08f);
        EmissiveScale = 0.35f;
        Tags.AddUnique(FName("BarricadeCritical"));
    }
    else if (HealthPct <= 0.58f)
    {
        Tint = FLinearColor(0.78f, 0.52f, 0.18f);
        EmissiveScale = 0.16f;
        Tags.AddUnique(FName("BarricadeCracked"));
    }

    BodyMaterial->SetVectorParameterValue(TEXT("Color"), Tint);
    BodyMaterial->SetVectorParameterValue(TEXT("BaseColor"), Tint);
    BodyMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Tint * EmissiveScale);
}

void ABarricadeActor::BreakApart(const FVector& ImpactPoint, const FVector& ImpulseDirection, AActor* DamageSource)
{
    if (bBroken)
    {
        return;
    }

    bBroken = true;
    Tags.AddUnique(FName("BarricadeBroken"));
    Tags.AddUnique(FName("DestructibleCoverBroken"));
    Tags.AddUnique(FName("ChaosReadableDestruction"));
    GetWorldTimerManager().ClearTimer(ExpireTimer);

    const int32 SafeDebrisCount = FMath::Clamp(DebrisCount, 0, 20);
    for (int32 Index = 0; Index < SafeDebrisCount; ++Index)
    {
        SpawnDebrisChunk(Index, ImpactPoint, ImpulseDirection);
    }

    UE_LOG(LogTemp, Display,
        TEXT("[CodeRescueDestructibleCover] Barricade '%s' broke into %d Chaos-simulated debris chunks after %s."),
        *GetName(),
        SafeDebrisCount,
        DamageSource ? *DamageSource->GetName() : TEXT("impact damage"));

    Destroy();
}

void ABarricadeActor::SpawnDebrisChunk(int32 Index, const FVector& ImpactPoint, const FVector& ImpulseDirection)
{
    UWorld* World = GetWorld();
    if (!World || !Body || !Body->GetStaticMesh())
    {
        return;
    }

    const FVector SafeImpulseDirection = ImpulseDirection.IsNearlyZero()
        ? GetActorForwardVector()
        : ImpulseDirection.GetSafeNormal();
    const FVector LocalScatter(
        FMath::FRandRange(-72.0f, 72.0f),
        FMath::FRandRange(-28.0f, 28.0f),
        FMath::FRandRange(-22.0f, 54.0f));
    const FVector SpawnLocation = (ImpactPoint.IsNearlyZero() ? GetActorLocation() : ImpactPoint) + GetActorRotation().RotateVector(LocalScatter);
    const FRotator SpawnRotation = GetActorRotation() + FRotator(
        FMath::FRandRange(-22.0f, 22.0f),
        FMath::FRandRange(-35.0f, 35.0f),
        FMath::FRandRange(-28.0f, 28.0f));

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AStaticMeshActor* Debris = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
    if (!Debris)
    {
        return;
    }

    Debris->SetLifeSpan(DebrisLifetime);
    Debris->Tags.Add(FName("BarricadeDebris"));
    Debris->Tags.Add(FName("DestructibleCoverDebris"));
    Debris->Tags.Add(FName("ChaosDestructionFallback"));
    Debris->Tags.Add(FName("GamePhysicsDeepDive"));
    Debris->Tags.Add(FName("SurfaceWood"));
    Debris->Tags.Add(FName("MacPhysicsBudgetReviewGate"));
    Debris->Tags.Add(FName("ChaosDebrisSleepDisableFallback"));

    UStaticMeshComponent* DebrisMesh = Debris->GetStaticMeshComponent();
    if (!DebrisMesh)
    {
        return;
    }

    DebrisMesh->SetStaticMesh(Body->GetStaticMesh());
    DebrisMesh->SetWorldScale3D(FVector(
        FMath::FRandRange(0.24f, 0.46f),
        FMath::FRandRange(0.05f, 0.13f),
        FMath::FRandRange(0.10f, 0.24f)));
    DebrisMesh->SetMobility(EComponentMobility::Movable);
    DebrisMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    DebrisMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DebrisMesh->SetNotifyRigidBodyCollision(true);
    DebrisMesh->SetLinearDamping(0.42f);
    DebrisMesh->SetAngularDamping(0.55f);
    CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
        DebrisMesh,
        Debris,
        FName("CoverDebrisFixedStepBody"),
        7.5f + Index * 0.9f,
        0.42f,
        0.55f,
        false);
    if (UMaterialInterface* BaseMaterial = Body->GetMaterial(0))
    {
        if (UMaterialInstanceDynamic* DebrisMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Debris))
        {
            const FLinearColor ChipTint = FLinearColor(0.36f, 0.22f, 0.12f) + FLinearColor(0.08f, 0.04f, 0.01f) * FMath::FRand();
            DebrisMaterial->SetVectorParameterValue(TEXT("Color"), ChipTint);
            DebrisMaterial->SetVectorParameterValue(TEXT("BaseColor"), ChipTint);
            DebrisMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), ChipTint * 0.08f);
            DebrisMesh->SetMaterial(0, DebrisMaterial);
        }
    }
    DebrisMesh->SetSimulatePhysics(true);

    const FVector Scatter = FVector(
        FMath::FRandRange(-0.45f, 0.45f),
        FMath::FRandRange(-0.55f, 0.55f),
        FMath::FRandRange(0.18f, 0.85f)).GetSafeNormal();
    const FVector FinalImpulse = (SafeImpulseDirection * 0.74f + Scatter * 0.58f + FVector::UpVector * 0.36f).GetSafeNormal() * DebrisImpulseStrength;
    DebrisMesh->AddImpulseAtLocation(FinalImpulse, SpawnLocation, NAME_None);
    DebrisMesh->AddAngularImpulseInDegrees(FVector(
        FMath::FRandRange(-18000.0f, 18000.0f),
        FMath::FRandRange(-14000.0f, 14000.0f),
        FMath::FRandRange(-22000.0f, 22000.0f)), NAME_None, true);

    ScheduleDebrisSleepDisable(Debris, DebrisMesh);
}

void ABarricadeActor::ScheduleDebrisSleepDisable(AStaticMeshActor* Debris, UStaticMeshComponent* DebrisMesh)
{
    UWorld* World = GetWorld();
    if (!World || !Debris || !DebrisMesh)
    {
        return;
    }

    const float SafeDelay = FMath::Clamp(DebrisSleepDisableDelay, 0.25f, 8.0f);
    TWeakObjectPtr<AStaticMeshActor> DebrisPtr(Debris);
    TWeakObjectPtr<UStaticMeshComponent> DebrisMeshPtr(DebrisMesh);

    FTimerHandle SleepDisableTimer;
    World->GetTimerManager().SetTimer(
        SleepDisableTimer,
        FTimerDelegate::CreateLambda([DebrisPtr, DebrisMeshPtr]()
        {
            AStaticMeshActor* DebrisActor = DebrisPtr.Get();
            UStaticMeshComponent* Mesh = DebrisMeshPtr.Get();
            if (!DebrisActor || !Mesh)
            {
                return;
            }

            Mesh->PutRigidBodyToSleep(NAME_None);
            Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            Mesh->SetSimulatePhysics(false);
            Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DebrisActor->Tags.AddUnique(FName("ChaosDebrisSleepDisabled"));
        }),
        SafeDelay,
        false);
}

void ABarricadeActor::OnBarricadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bBroken || !OtherActor || OtherActor == this || !OtherComp || !GetWorld())
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastImpactDamageTime < 0.18f)
    {
        return;
    }

    const float ImpactSpeed = OtherComp->GetComponentVelocity().Size();
    if (ImpactSpeed < ImpactDamageSpeedThreshold)
    {
        return;
    }

    LastImpactDamageTime = Now;
    const float Damage = FMath::Clamp((ImpactSpeed - ImpactDamageSpeedThreshold) * ImpactDamageScale, 3.0f, 42.0f);
    const FVector ImpactPoint = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
    const FVector ImpulseDirection = OtherComp->GetComponentVelocity().IsNearlyZero()
        ? -Hit.ImpactNormal.GetSafeNormal()
        : OtherComp->GetComponentVelocity().GetSafeNormal();
    TakeBarricadeDamage(Damage, ImpactPoint, ImpulseDirection, OtherActor);
}
