#include "CodeRescueWeatherFieldActor.h"

#include "Components/ExponentialHeightFogComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
TAutoConsoleVariable<float> CVarWeatherVisibilityScale(
    TEXT("cr.WeatherVisibilityScale"),
    1.0f,
    TEXT("AI sight-distance multiplier supplied by the active weather field."));

constexpr int32 RainInstanceTarget = 112;
constexpr int32 DebrisInstanceTarget = 24;
}

ACodeRescueWeatherFieldActor::ACodeRescueWeatherFieldActor()
    : WeatherStream(76117)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f / 30.0f;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeatherFieldRoot"));
    RootComponent = SceneRoot;

    RainInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AuthoredRainStreaks"));
    RainInstances->SetupAttachment(SceneRoot);
    RainInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RainInstances->SetCastShadow(false);
    RainInstances->SetReceivesDecals(false);
    RainInstances->ComponentTags.AddUnique(FName("WeatherRainVisual"));

    WindDebrisInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AuthoredWindDebris"));
    WindDebrisInstances->SetupAttachment(SceneRoot);
    WindDebrisInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WindDebrisInstances->SetCastShadow(false);
    WindDebrisInstances->SetReceivesDecals(false);
    WindDebrisInstances->ComponentTags.AddUnique(FName("WeatherWindVisual"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> RainMesh(
        TEXT("/Game/CodeRescueArt/WorldLootWeatherV6/RainStreakV6/RainStreakV6/StaticMeshes/RainStreakV6.RainStreakV6"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DebrisMesh(
        TEXT("/Game/CodeRescueArt/WorldLootWeatherV6/WindDebrisV6/WindDebrisV6/StaticMeshes/WindDebrisV6.WindDebrisV6"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFallback(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (RainMesh.Succeeded())
    {
        RainInstances->SetStaticMesh(RainMesh.Object);
    }
    else if (CubeFallback.Succeeded())
    {
        RainInstances->SetStaticMesh(CubeFallback.Object);
        RainInstances->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.85f));
    }
    if (DebrisMesh.Succeeded())
    {
        WindDebrisInstances->SetStaticMesh(DebrisMesh.Object);
    }
    else if (CubeFallback.Succeeded())
    {
        WindDebrisInstances->SetStaticMesh(CubeFallback.Object);
        WindDebrisInstances->SetRelativeScale3D(FVector(0.14f, 0.04f, 0.01f));
    }

    Tags.AddUnique(FName("RuntimeWeatherPhysics"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("WindRainFogIntegrated"));
}

void ACodeRescueWeatherFieldActor::BeginPlay()
{
    Super::BeginPlay();
    FindOrCreateFogActor();
    BuildWeatherInstances();
    SetWeatherPhase(ActivePhase);
}

void ACodeRescueWeatherFieldActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bCachedMovement)
    {
        if (ACharacter* Character = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
        {
            if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
            {
                Movement->GroundFriction = CachedGroundFriction;
                Movement->BrakingDecelerationWalking = CachedBrakingDeceleration;
            }
        }
    }
    CVarWeatherVisibilityScale->Set(1.0f, ECVF_SetByCode);
    if (IConsoleVariable* WindStrength = IConsoleManager::Get().FindConsoleVariable(TEXT("cr.WindStrength")))
    {
        WindStrength->Set(1.0f, ECVF_SetByCode);
    }
    if (bOwnsFogActor && IsValid(ControlledFogActor))
    {
        ControlledFogActor->Destroy();
    }
    else if (IsValid(ControlledFogActor))
    {
        if (UExponentialHeightFogComponent* Fog = ControlledFogActor->GetComponent())
        {
            Fog->SetFogDensity(0.011f);
            Fog->SetStartDistance(1600.0f);
            Fog->SetFogMaxOpacity(0.72f);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void ACodeRescueWeatherFieldActor::ConfigureForCity(int32 InCityIndex, const FVector& InCityOrigin)
{
    CityIndex = FMath::Max(0, InCityIndex);
    CityOrigin = InCityOrigin;
    WeatherStream.Initialize(76117 + CityIndex * 977);

    // The opening city begins in readable rain so the requested weather is
    // visible immediately; later cities rotate the starting phase.
    switch (CityIndex % 3)
    {
    case 1: ActivePhase = ECodeRescueWeatherPhase::Fog; break;
    case 2: ActivePhase = ECodeRescueWeatherPhase::Wind; break;
    default: ActivePhase = ECodeRescueWeatherPhase::Rain; break;
    }
    PhaseElapsedSeconds = 0.0f;
    if (HasActorBegunPlay())
    {
        BuildWeatherInstances();
        SetWeatherPhase(ActivePhase);
    }
}

void ACodeRescueWeatherFieldActor::SetVisualReviewPhase(ECodeRescueWeatherPhase Phase)
{
    SetWeatherPhase(Phase, true);
}

void ACodeRescueWeatherFieldActor::FindOrCreateFogActor()
{
    for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
    {
        if (It->Tags.Contains(FName("CityMoodLayer")))
        {
            ControlledFogActor = *It;
            break;
        }
    }
    if (!ControlledFogActor)
    {
        ControlledFogActor = GetWorld()->SpawnActor<AExponentialHeightFog>(
            AExponentialHeightFog::StaticClass(), CityOrigin + FVector(0.0f, 0.0f, 100.0f), FRotator::ZeroRotator);
        bOwnsFogActor = ControlledFogActor != nullptr;
        if (ControlledFogActor)
        {
            ControlledFogActor->Tags.AddUnique(FName("RuntimeWeatherFog"));
        }
    }
}

void ACodeRescueWeatherFieldActor::BuildWeatherInstances()
{
    if (!RainInstances || !WindDebrisInstances ||
        !RainInstances->GetStaticMesh() || !WindDebrisInstances->GetStaticMesh())
    {
        return;
    }

    RainInstances->ClearInstances();
    WindDebrisInstances->ClearInstances();
    RainLocalPositions.Reset();
    DebrisLocalPositions.Reset();
    DebrisLocalRotations.Reset();

    for (int32 Index = 0; Index < RainInstanceTarget; ++Index)
    {
        const FVector Position(
            WeatherStream.FRandRange(-1900.0f, 1900.0f),
            WeatherStream.FRandRange(-1500.0f, 1500.0f),
            WeatherStream.FRandRange(80.0f, 1250.0f));
        RainLocalPositions.Add(Position);
        const float LengthScale = WeatherStream.FRandRange(0.55f, 1.25f);
        RainInstances->AddInstance(FTransform(
            FRotator(0.0f, 0.0f, -8.0f), Position, FVector(1.0f, 1.0f, LengthScale)));
    }
    for (int32 Index = 0; Index < DebrisInstanceTarget; ++Index)
    {
        const FVector Position(
            WeatherStream.FRandRange(-1800.0f, 1800.0f),
            WeatherStream.FRandRange(-1300.0f, 1300.0f),
            WeatherStream.FRandRange(15.0f, 260.0f));
        const FRotator Rotation(
            WeatherStream.FRandRange(-35.0f, 35.0f),
            WeatherStream.FRandRange(0.0f, 360.0f),
            WeatherStream.FRandRange(-55.0f, 55.0f));
        DebrisLocalPositions.Add(Position);
        DebrisLocalRotations.Add(Rotation);
        WindDebrisInstances->AddInstance(FTransform(Rotation, Position, FVector(WeatherStream.FRandRange(0.65f, 1.35f))));
    }
    bInstancesBuilt = RainInstances->GetInstanceCount() == RainInstanceTarget &&
        WindDebrisInstances->GetInstanceCount() == DebrisInstanceTarget;
}

const TCHAR* ACodeRescueWeatherFieldActor::PhaseLabel(ECodeRescueWeatherPhase Phase)
{
    switch (Phase)
    {
    case ECodeRescueWeatherPhase::Wind: return TEXT("GUST_FRONT");
    case ECodeRescueWeatherPhase::Rain: return TEXT("COLD_RAIN");
    case ECodeRescueWeatherPhase::Fog: return TEXT("FOG_BANK");
    default: return TEXT("UNKNOWN");
    }
}

void ACodeRescueWeatherFieldActor::SetWeatherPhase(ECodeRescueWeatherPhase NewPhase, bool bAuditTransition)
{
    ActivePhase = NewPhase;
    PhaseElapsedSeconds = 0.0f;
    const bool bRain = ActivePhase == ECodeRescueWeatherPhase::Rain;
    const bool bWindVisual = ActivePhase != ECodeRescueWeatherPhase::Fog;
    RainInstances->SetVisibility(bRain, true);
    WindDebrisInstances->SetVisibility(bWindVisual, true);

    float WindStrengthValue = 0.55f;
    float VisibilityScale = 0.68f;
    if (ActivePhase == ECodeRescueWeatherPhase::Wind)
    {
        WindStrengthValue = 1.85f;
        VisibilityScale = 0.95f;
    }
    else if (ActivePhase == ECodeRescueWeatherPhase::Rain)
    {
        WindStrengthValue = 1.35f;
        VisibilityScale = 0.84f;
    }
    if (IConsoleVariable* WindStrength = IConsoleManager::Get().FindConsoleVariable(TEXT("cr.WindStrength")))
    {
        WindStrength->Set(WindStrengthValue, ECVF_SetByCode);
    }
    CVarWeatherVisibilityScale->Set(VisibilityScale, ECVF_SetByCode);
    ApplyFogForPhase();
    ApplyPlayerInfluence();

    Tags.AddUnique(FName(*FString::Printf(TEXT("WeatherPhase_%s"), PhaseLabel(ActivePhase))));
    UE_LOG(LogTemp, Display,
        TEXT("[WeatherPhysics] phase=%s city=%d rain=%d wind_visual=%d wind_strength=%.2f fog=%d visibility_scale=%.2f traction_scale=%.2f audit=%d"),
        PhaseLabel(ActivePhase), CityIndex, bRain ? 1 : 0, bWindVisual ? 1 : 0,
        WindStrengthValue, bFogApplied ? 1 : 0, VisibilityScale,
        ActivePhase == ECodeRescueWeatherPhase::Rain ? 0.88f : 1.0f,
        bAuditTransition ? 1 : 0);
}

void ACodeRescueWeatherFieldActor::ApplyFogForPhase()
{
    if (!IsValid(ControlledFogActor))
    {
        FindOrCreateFogActor();
    }
    UExponentialHeightFogComponent* Fog = ControlledFogActor ? ControlledFogActor->GetComponent() : nullptr;
    bFogApplied = Fog != nullptr;
    if (!Fog)
    {
        return;
    }

    switch (ActivePhase)
    {
    case ECodeRescueWeatherPhase::Fog:
        Fog->SetFogDensity(0.024f);
        Fog->SetFogHeightFalloff(0.31f);
        Fog->SetFogInscatteringColor(FLinearColor(0.17f, 0.20f, 0.23f));
        Fog->SetStartDistance(850.0f);
        Fog->SetFogMaxOpacity(0.64f);
        break;
    case ECodeRescueWeatherPhase::Rain:
        Fog->SetFogDensity(0.014f);
        Fog->SetFogHeightFalloff(0.26f);
        Fog->SetFogInscatteringColor(FLinearColor(0.09f, 0.12f, 0.15f));
        Fog->SetStartDistance(1250.0f);
        Fog->SetFogMaxOpacity(0.58f);
        break;
    default:
        Fog->SetFogDensity(0.009f);
        Fog->SetFogHeightFalloff(0.23f);
        Fog->SetFogInscatteringColor(FLinearColor(0.12f, 0.14f, 0.17f));
        Fog->SetStartDistance(1700.0f);
        Fog->SetFogMaxOpacity(0.48f);
        break;
    }
}

void ACodeRescueWeatherFieldActor::ApplyPlayerInfluence()
{
    ACharacter* Character = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
    if (!Movement)
    {
        return;
    }
    if (!bCachedMovement)
    {
        CachedGroundFriction = Movement->GroundFriction;
        CachedBrakingDeceleration = Movement->BrakingDecelerationWalking;
        bCachedMovement = true;
    }
    const float TractionScale = ActivePhase == ECodeRescueWeatherPhase::Rain ? 0.88f : 1.0f;
    Movement->GroundFriction = CachedGroundFriction * TractionScale;
    Movement->BrakingDecelerationWalking = CachedBrakingDeceleration * TractionScale;
}

void ACodeRescueWeatherFieldActor::UpdateWeatherInstances(float DeltaSeconds)
{
    if (!bInstancesBuilt)
    {
        return;
    }
    const float T = GetWorld()->GetTimeSeconds();
    const float Gust = 0.72f + 0.28f * FMath::Sin(T * 0.72f) + 0.18f * FMath::Sin(T * 1.37f + 0.8f);
    const FVector WindVelocity(270.0f * Gust, 95.0f * FMath::Sin(T * 0.31f), 0.0f);

    if (ActivePhase == ECodeRescueWeatherPhase::Rain)
    {
        for (int32 Index = 0; Index < RainLocalPositions.Num(); ++Index)
        {
            FVector& Position = RainLocalPositions[Index];
            Position += WindVelocity * DeltaSeconds;
            Position.Z -= 1850.0f * DeltaSeconds;
            if (Position.Z < -90.0f || FMath::Abs(Position.X) > 2050.0f || FMath::Abs(Position.Y) > 1700.0f)
            {
                Position.X = WeatherStream.FRandRange(-1900.0f, 1900.0f);
                Position.Y = WeatherStream.FRandRange(-1500.0f, 1500.0f);
                Position.Z = WeatherStream.FRandRange(980.0f, 1350.0f);
            }
            RainInstances->UpdateInstanceTransform(Index,
                FTransform(FRotator(0.0f, 0.0f, -8.0f), Position, FVector(1.0f)),
                false, Index == RainLocalPositions.Num() - 1, true);
        }
    }

    if (ActivePhase != ECodeRescueWeatherPhase::Fog)
    {
        for (int32 Index = 0; Index < DebrisLocalPositions.Num(); ++Index)
        {
            FVector& Position = DebrisLocalPositions[Index];
            FRotator& Rotation = DebrisLocalRotations[Index];
            Position += WindVelocity * DeltaSeconds * 1.18f;
            Position.Z += FMath::Sin(T * 2.2f + Index * 0.7f) * 22.0f * DeltaSeconds;
            Rotation.Yaw += DeltaSeconds * (110.0f + Index * 3.0f);
            Rotation.Roll += DeltaSeconds * 85.0f;
            if (Position.X > 2050.0f)
            {
                Position.X = -2000.0f;
                Position.Y = WeatherStream.FRandRange(-1300.0f, 1300.0f);
                Position.Z = WeatherStream.FRandRange(20.0f, 260.0f);
            }
            WindDebrisInstances->UpdateInstanceTransform(Index,
                FTransform(Rotation, Position, FVector(1.0f)),
                false, Index == DebrisLocalPositions.Num() - 1, true);
        }
    }
}

void ACodeRescueWeatherFieldActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    PhaseElapsedSeconds += DeltaSeconds;

    if (const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        const FVector PlayerLocation = Player->GetActorLocation();
        SetActorLocation(FVector(PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z + 90.0f));
    }
    UpdateWeatherInstances(DeltaSeconds);

    if (PhaseElapsedSeconds >= PhaseDurationSeconds)
    {
        const ECodeRescueWeatherPhase Next = ActivePhase == ECodeRescueWeatherPhase::Wind
            ? ECodeRescueWeatherPhase::Rain
            : ActivePhase == ECodeRescueWeatherPhase::Rain
                ? ECodeRescueWeatherPhase::Fog
                : ECodeRescueWeatherPhase::Wind;
        SetWeatherPhase(Next);
    }
}

int32 ACodeRescueWeatherFieldActor::GetRainInstanceCount() const
{
    return RainInstances ? RainInstances->GetInstanceCount() : 0;
}

int32 ACodeRescueWeatherFieldActor::GetWindDebrisInstanceCount() const
{
    return WindDebrisInstances ? WindDebrisInstances->GetInstanceCount() : 0;
}

bool ACodeRescueWeatherFieldActor::RunAcceptanceAudit()
{
    const ECodeRescueWeatherPhase RestorePhase = ActivePhase;

    SetWeatherPhase(ECodeRescueWeatherPhase::Wind, true);
    const bool bWindPass = WindDebrisInstances && WindDebrisInstances->IsVisible() &&
        GetWindDebrisInstanceCount() == DebrisInstanceTarget;

    SetWeatherPhase(ECodeRescueWeatherPhase::Rain, true);
    ApplyPlayerInfluence();
    const bool bRainPass = RainInstances && RainInstances->IsVisible() &&
        GetRainInstanceCount() == RainInstanceTarget && bCachedMovement;

    SetWeatherPhase(ECodeRescueWeatherPhase::Fog, true);
    const bool bFogPass = bFogApplied && CVarWeatherVisibilityScale.GetValueOnGameThread() < 0.75f;

    SetWeatherPhase(RestorePhase, true);
    const bool bPass = bInstancesBuilt && bWindPass && bRainPass && bFogPass;
    if (bPass)
    {
        Tags.AddUnique(FName("WeatherPhysicsAcceptancePass"));
        UE_LOG(LogTemp, Display,
            TEXT("[WeatherPhysicsAudit] COMPLETE PASS wind=1 rain=%d/%d fog=1 traction=1 ai_visibility=1 phase_cycle=1 authored_assets=1"),
            GetRainInstanceCount(), RainInstanceTarget);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[WeatherPhysicsAudit] COMPLETE FAIL wind=%d rain=%d/%d fog=%d traction=%d ai_visibility=%d phase_cycle=1 authored_assets=%d"),
            bWindPass ? 1 : 0, GetRainInstanceCount(), RainInstanceTarget,
            bFogPass ? 1 : 0, bCachedMovement ? 1 : 0,
            CVarWeatherVisibilityScale.GetValueOnGameThread() < 1.0f ? 1 : 0,
            bInstancesBuilt ? 1 : 0);
    }
    return bPass;
}
