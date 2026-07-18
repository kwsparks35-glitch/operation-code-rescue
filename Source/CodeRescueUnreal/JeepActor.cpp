#include "JeepActor.h"
#include "Camera/CameraComponent.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescuePhysicsStability.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/ConstructorHelpers.h"

AJeepActor::AJeepActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = true;

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JeepBody"));
    SetRootComponent(Body);
    Body->SetWorldScale3D(FVector(2.5f, 1.4f, 1.0f));
    Body->SetCollisionProfileName(TEXT("Pawn"));
    Body->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    Body->SetCollisionResponseToChannel(CodeRescueCollision::AISightTrace, ECR_Block);
    Body->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));
    Body->bReturnMaterialOnMove = true;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded()) Body->SetStaticMesh(CubeMesh.Object);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("JeepCamera"));
    Camera->SetupAttachment(Body);
    Camera->SetRelativeLocation(FVector(-300.0f, 0.0f, 200.0f));
    Camera->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

    SurfaceCueLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SurfaceCueLight"));
    SurfaceCueLight->SetupAttachment(Body);
    SurfaceCueLight->SetRelativeLocation(FVector(-40.0f, 0.0f, -72.0f));
    SurfaceCueLight->SetIntensity(850.0f);
    SurfaceCueLight->SetAttenuationRadius(420.0f);
    SurfaceCueLight->SetCastShadows(false);

    Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("JeepMovement"));
    Movement->MaxSpeed = MaxJeepSpeed;
    Movement->Acceleration = BaseAcceleration;
    Movement->Deceleration = BaseDeceleration;

    Tags.Add(FName("Jeep"));
    Tags.Add(FName("VehiclePhysicsFallback"));
    Tags.Add(FName("ChaosVehicleReadyFallback"));
    Tags.Add(FName("SurfaceAwareVehicle"));
    Tags.Add(FName("GamePhysicsDeepDive"));
}

void AJeepActor::BeginPlay()
{
    Super::BeginPlay();
    CodeRescuePhysicsStability::ApplyRuntimeBodyContract(
        Body,
        this,
        FName("JeepFallbackFixedStepBody"),
        950.0f,
        0.42f,
        0.62f,
        false);
}

void AJeepActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    float Forward = 0.0f, Right = 0.0f, Turn = 0.0f;
    if (PC->IsInputKeyDown(EKeys::W)) Forward += 1.0f;
    if (PC->IsInputKeyDown(EKeys::S)) Forward -= 1.0f;
    if (PC->IsInputKeyDown(EKeys::A)) Turn -= 1.0f;
    if (PC->IsInputKeyDown(EKeys::D)) Turn += 1.0f;

    ApplySurfaceTuning(DeltaSeconds, Forward, Turn);

    if (Forward != 0.0f) AddMovementInput(GetActorForwardVector(), Forward * CurrentTraction);
    if (Right   != 0.0f) AddMovementInput(GetActorRightVector(),   Right);
    if (Turn    != 0.0f) AddActorWorldRotation(FRotator(0, Turn * TurnRateDegPerSec * CurrentTurnScale * DeltaSeconds, 0));

    if (PC->WasInputKeyJustPressed(EKeys::E))
    {
        Dismount();
    }
}

void AJeepActor::Mount(APlayerController* PC, APawn* PreviousPawn)
{
    if (!PC) return;
    CachedPreviousPawn = PreviousPawn;
    PC->Possess(this);
    Tags.AddUnique(FName("JeepMountedSurfaceAware"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.8f, FColor::Green,
            TEXT("Jeep mounted: surface-aware traction active."));
    }
}

void AJeepActor::Dismount()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !CachedPreviousPawn) return;
    const FVector RestoreLoc = GetActorLocation() + GetActorRightVector() * 250.0f + FVector(0, 0, 80);
    CachedPreviousPawn->SetActorLocation(RestoreLoc);
    PC->Possess(CachedPreviousPawn);
    CachedPreviousPawn = nullptr;
}

void AJeepActor::UpdateGroundSurface()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
    const FVector End = Start - FVector(0.0f, 0.0f, SurfaceProbeDistance);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CodeRescueJeepGroundSurfaceProbe), true, this);
    Params.bReturnPhysicalMaterial = true;

    FHitResult Hit;
    if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        CurrentGroundSurface = ResolveGroundSurface(Hit);
    }
    else
    {
        CurrentGroundSurface = SurfaceType_Default;
    }

    CurrentTraction = GetSurfaceTraction(CurrentGroundSurface);
    CurrentSpeedScale = GetSurfaceSpeedScale(CurrentGroundSurface);
    CurrentTurnScale = GetSurfaceTurnScale(CurrentGroundSurface);

    if (Movement)
    {
        Movement->MaxSpeed = MaxJeepSpeed * CurrentSpeedScale;
        Movement->Acceleration = BaseAcceleration * FMath::Lerp(0.72f, 1.18f, CurrentTraction);
        Movement->Deceleration = BaseDeceleration * FMath::Lerp(0.70f, 1.24f, CurrentTraction);
    }

    if (SurfaceCueLight)
    {
        const float Speed = Movement ? Movement->Velocity.Size() : GetVelocity().Size();
        SurfaceCueLight->SetLightColor(GetSurfaceCueColor(CurrentGroundSurface));
        SurfaceCueLight->SetIntensity(FMath::Clamp(650.0f + Speed * 0.42f, 650.0f, 2400.0f));
        SurfaceCueLight->SetAttenuationRadius(FMath::Clamp(300.0f + Speed * 0.08f, 300.0f, 680.0f));
    }
}

EPhysicalSurface AJeepActor::ResolveGroundSurface(const FHitResult& Hit) const
{
    if (Hit.PhysMaterial.IsValid())
    {
        const EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
        if (SurfaceType != SurfaceType_Default)
        {
            return SurfaceType;
        }
    }

    const AActor* HitActor = Hit.GetActor();
    const UPrimitiveComponent* HitComp = Hit.GetComponent();
    const auto HasSurfaceTag = [HitActor, HitComp](const FName& Tag)
    {
        return (HitActor && HitActor->Tags.Contains(Tag)) ||
            (HitComp && HitComp->ComponentTags.Contains(Tag));
    };

    if (HasSurfaceTag(FName("SurfaceConcrete"))) return SurfaceType1;
    if (HasSurfaceTag(FName("SurfaceMetal"))) return SurfaceType2;
    if (HasSurfaceTag(FName("SurfaceWood"))) return SurfaceType3;
    if (HasSurfaceTag(FName("SurfaceGlass"))) return SurfaceType4;
    if (HasSurfaceTag(FName("SurfaceFlesh"))) return SurfaceType5;
    if (HasSurfaceTag(FName("SurfaceDirt"))) return SurfaceType6;
    return SurfaceType_Default;
}

void AJeepActor::ApplySurfaceTuning(float DeltaSeconds, float ForwardInput, float TurnInput)
{
    UWorld* World = GetWorld();
    const float Now = World ? World->GetTimeSeconds() : 0.0f;
    if (Now - LastSurfaceProbeTime >= SurfaceProbeInterval)
    {
        LastSurfaceProbeTime = Now;
        UpdateGroundSurface();
    }

    if (Movement)
    {
        const FVector Right = GetActorRightVector();
        const float LateralSpeed = FVector::DotProduct(Movement->Velocity, Right);
        const float LateralDamping = FMath::Lerp(LowTractionLateralDamping, HighTractionLateralDamping, CurrentTraction);
        const float DampingAlpha = 1.0f - FMath::Exp(-LateralDamping * FMath::Max(0.0f, DeltaSeconds));
        Movement->Velocity -= Right * LateralSpeed * DampingAlpha;
    }

    if (Camera)
    {
        const float TargetRoll = FMath::Clamp(-TurnInput * 2.8f * CurrentTraction, -4.0f, 4.0f);
        const float TargetPitch = FMath::Clamp(-ForwardInput * 1.6f, -2.2f, 2.2f);
        const FRotator CurrentRot = Camera->GetRelativeRotation();
        const FRotator TargetRot(-15.0f + TargetPitch, 0.0f, TargetRoll);
        Camera->SetRelativeRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, 7.0f));
    }

    Tags.AddUnique(FName("JeepSurfaceTractionActive"));
}

float AJeepActor::GetSurfaceTraction(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return 1.0f;  // Concrete
    case SurfaceType2: return 0.76f; // Metal
    case SurfaceType3: return 0.86f; // Wood
    case SurfaceType4: return 0.58f; // Glass
    case SurfaceType5: return 0.70f; // Flesh
    case SurfaceType6: return 0.64f; // Dirt
    default: return 0.92f;
    }
}

float AJeepActor::GetSurfaceSpeedScale(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return 1.0f;
    case SurfaceType2: return 0.90f;
    case SurfaceType3: return 0.88f;
    case SurfaceType4: return 0.62f;
    case SurfaceType5: return 0.74f;
    case SurfaceType6: return 0.78f;
    default: return 0.94f;
    }
}

float AJeepActor::GetSurfaceTurnScale(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return 1.0f;
    case SurfaceType2: return 0.74f;
    case SurfaceType3: return 0.86f;
    case SurfaceType4: return 0.52f;
    case SurfaceType5: return 0.72f;
    case SurfaceType6: return 0.80f;
    default: return 0.92f;
    }
}

FLinearColor AJeepActor::GetSurfaceCueColor(EPhysicalSurface SurfaceType) const
{
    switch (SurfaceType)
    {
    case SurfaceType1: return FLinearColor(0.74f, 0.76f, 0.70f);
    case SurfaceType2: return FLinearColor(1.0f, 0.70f, 0.22f);
    case SurfaceType3: return FLinearColor(0.72f, 0.40f, 0.16f);
    case SurfaceType4: return FLinearColor(0.58f, 0.88f, 1.0f);
    case SurfaceType5: return FLinearColor(0.86f, 0.12f, 0.05f);
    case SurfaceType6: return FLinearColor(0.46f, 0.30f, 0.14f);
    default: return FLinearColor(0.48f, 0.88f, 0.58f);
    }
}

const TCHAR* AJeepActor::DescribeSurface(EPhysicalSurface SurfaceType) const
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
