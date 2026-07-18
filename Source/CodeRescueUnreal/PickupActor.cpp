#include "PickupActor.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescueGameInstance.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
const TCHAR* PickupKindLabel(EPickupKind Kind)
{
    switch (Kind)
    {
    case EPickupKind::Ammo: return TEXT("ammo");
    case EPickupKind::Medkit: return TEXT("medkit");
    case EPickupKind::Flare: return TEXT("flare");
    case EPickupKind::Smoke: return TEXT("smoke");
    case EPickupKind::Stim: return TEXT("stim");
    case EPickupKind::Scrap: return TEXT("scrap");
    case EPickupKind::ArmorPlate: return TEXT("armor plate");
    case EPickupKind::RadioScanner: return TEXT("radio scanner charge");
    case EPickupKind::FlashlightBattery: return TEXT("flashlight battery");
    case EPickupKind::AmmoPouch: return TEXT("ammo pouch capacity");
    case EPickupKind::BypassKit: return TEXT("bypass kit");
    default: return TEXT("supplies");
    }
}

FLinearColor PickupKindColor(EPickupKind Kind)
{
    switch (Kind)
    {
    case EPickupKind::Ammo: return FLinearColor(0.2f, 0.6f, 1.0f);
    case EPickupKind::Medkit: return FLinearColor(0.2f, 1.0f, 0.4f);
    case EPickupKind::Flare: return FLinearColor(1.0f, 0.36f, 0.12f);
    case EPickupKind::Smoke: return FLinearColor(0.64f, 0.72f, 0.78f);
    case EPickupKind::Stim: return FLinearColor(0.88f, 0.22f, 1.0f);
    case EPickupKind::Scrap: return FLinearColor(1.0f, 0.78f, 0.28f);
    case EPickupKind::ArmorPlate: return FLinearColor(0.62f, 0.72f, 0.80f);
    case EPickupKind::RadioScanner: return FLinearColor(0.12f, 0.92f, 1.0f);
    case EPickupKind::FlashlightBattery: return FLinearColor(1.0f, 0.92f, 0.48f);
    case EPickupKind::AmmoPouch: return FLinearColor(0.48f, 0.74f, 1.0f);
    case EPickupKind::BypassKit: return FLinearColor(0.98f, 0.62f, 0.20f);
    default: return FLinearColor(0.8f, 0.8f, 0.8f);
    }
}
}

APickupActor::APickupActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f / 30.0f;

    // Mesh is the visible cube the player reads as a grounded supply marker.
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    RootComponent = MeshComp;
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));

    // Trigger is the overlap volume that grants the resource on touch. Sized
    // generously so the player doesn't have to walk pixel-perfect onto it.
    TriggerComp = CreateDefaultSubobject<USphereComponent>(TEXT("PickupTrigger"));
    TriggerComp->SetupAttachment(MeshComp);
    TriggerComp->InitSphereRadius(220.0f);
    TriggerComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerComp->SetCollisionObjectType(CodeRescueCollision::PickupObject);
    TriggerComp->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    TriggerComp->ComponentTags.AddUnique(FName("CollisionChannel_PickupObject"));
    TriggerComp->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));
    TriggerComp->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::OnTriggerOverlap);

    // Soft glow so pickups read against the dark world.
    GlowComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("PickupGlow"));
    GlowComp->SetupAttachment(MeshComp);
    GlowComp->SetIntensity(800.0f);
    GlowComp->SetAttenuationRadius(280.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded()) MeshComp->SetStaticMesh(CubeMesh.Object);
}

void APickupActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bAuthoredPresentationReady && PresentationSpinDegreesPerSecond > 0.0f &&
        !Tags.Contains(FName("WorldLootWeatherVisualReviewOnly")))
    {
        AddActorLocalRotation(FRotator(0.0f, PresentationSpinDegreesPerSecond * DeltaSeconds, 0.0f));
    }
}

void APickupActor::BeginPlay()
{
    Super::BeginPlay();

    RefreshPresentation();
    SnapToGround();

    // Most callers assign Kind after SpawnActor returns. For actors spawned
    // into an active world that assignment happens after BeginPlay, so resolve
    // the final authored symbol and contact height on the following frame.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            RefreshPresentation();
            SnapToGround();
        }));
    }
}

const TCHAR* APickupActor::ResolvePresentationAssetName() const
{
    switch (Kind)
    {
    case EPickupKind::Ammo:
    case EPickupKind::AmmoPouch:
        return TEXT("PickupAmmoV6");
    case EPickupKind::Medkit:
    case EPickupKind::Stim:
        return TEXT("PickupMedicalV6");
    case EPickupKind::ArmorPlate:
        return TEXT("PickupArmorV6");
    case EPickupKind::RadioScanner:
    case EPickupKind::BypassKit:
        return TEXT("PickupTechV6");
    case EPickupKind::Scrap:
        return TEXT("PickupSalvageV6");
    case EPickupKind::Flare:
    case EPickupKind::Smoke:
    case EPickupKind::FlashlightBattery:
    default:
        return TEXT("PickupUtilityV6");
    }
}

const TCHAR* APickupActor::ResolvePresentationStyleToken() const
{
    switch (Kind)
    {
    case EPickupKind::Ammo:
    case EPickupKind::AmmoPouch: return TEXT("AMMO_BULLETS");
    case EPickupKind::Medkit:
    case EPickupKind::Stim: return TEXT("MEDICAL_CROSS");
    case EPickupKind::ArmorPlate: return TEXT("ARMOR_SHIELD");
    case EPickupKind::RadioScanner:
    case EPickupKind::BypassKit: return TEXT("TECH_SCANNER");
    case EPickupKind::Scrap: return TEXT("SALVAGE_GEAR");
    default: return TEXT("UTILITY_BOLT");
    }
}

void APickupActor::RefreshPresentation()
{
    if (!MeshComp)
    {
        return;
    }

    static const FName StyleTags[] = {
        FName("PickupStyle_AMMO_BULLETS"),
        FName("PickupStyle_MEDICAL_CROSS"),
        FName("PickupStyle_ARMOR_SHIELD"),
        FName("PickupStyle_TECH_SCANNER"),
        FName("PickupStyle_SALVAGE_GEAR"),
        FName("PickupStyle_UTILITY_BOLT"),
    };
    for (const FName& StyleTag : StyleTags)
    {
        Tags.Remove(StyleTag);
    }

    const FString AssetName(ResolvePresentationAssetName());
    const FString AssetPath = FString::Printf(
        TEXT("/Game/CodeRescueArt/WorldLootWeatherV6/%s/%s/StaticMeshes/%s.%s"),
        *AssetName, *AssetName, *AssetName, *AssetName);
    if (UStaticMesh* AuthoredMesh = LoadObject<UStaticMesh>(nullptr, *AssetPath))
    {
        MeshComp->SetStaticMesh(AuthoredMesh);
        MeshComp->SetRelativeScale3D(FVector::OneVector);
        MeshComp->SetRelativeRotation(FRotator::ZeroRotator);
        bAuthoredPresentationReady = true;
        Tags.AddUnique(FName("BlenderAuthoredPickupPackaging"));
        Tags.AddUnique(FName("IconFirstPickupPresentation"));
        Tags.AddUnique(FName("NoParagraphPickupLabel"));
    }
    else
    {
        bAuthoredPresentationReady = false;
        if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
        {
            MeshComp->SetStaticMesh(CubeMesh);
            MeshComp->SetRelativeScale3D(FVector(0.6f));
        }
    }

    PresentationStyleToken = ResolvePresentationStyleToken();
    Tags.AddUnique(FName(*FString::Printf(TEXT("PickupStyle_%s"), *PresentationStyleToken)));
    const FLinearColor Color = PickupKindColor(Kind);
    GlowComp->SetLightColor(Color);
    GlowComp->SetIntensity(bAuthoredPresentationReady ? 520.0f : 800.0f);

    // Imported assets carry their authored multi-material presentation. Only
    // tint the engine-cube fallback so refreshing never flattens their symbols.
    if (!bAuthoredPresentationReady)
    {
        if (UMaterialInstanceDynamic* Mat = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
        {
            Mat->SetVectorParameterValue(TEXT("Color"), Color * 4.0f);
            Mat->SetVectorParameterValue(TEXT("BaseColor"), Color * 4.0f);
            Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 4.0f);
        }
    }
}

void APickupActor::SnapToGround()
{
    if (!bSnapToGround || !GetWorld())
    {
        return;
    }

    const FVector CurrentLocation = GetActorLocation();
    const FVector Start = CurrentLocation + FVector(0.0f, 0.0f, 300.0f);
    const FVector End = CurrentLocation - FVector(0.0f, 0.0f, 1500.0f);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CodeRescuePickupGroundSnap), false, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        float BottomOffset = GroundClearance;
        if (bAuthoredPresentationReady && MeshComp && MeshComp->GetStaticMesh())
        {
            const FBox LocalBounds = MeshComp->GetStaticMesh()->GetBoundingBox();
            const float ScaleZ = FMath::Abs(MeshComp->GetComponentScale().Z);
            BottomOffset = FMath::Max(0.0f, -static_cast<float>(LocalBounds.Min.Z) * ScaleZ) + 2.0f;
        }
        SetActorLocation(Hit.ImpactPoint + FVector(0.0f, 0.0f, BottomOffset));
        Tags.AddUnique(FName("PickupGroundSnapped"));
        Tags.AddUnique(FName("PickupGroundContactVerified"));
    }
}

void APickupActor::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                    bool bFromSweep, const FHitResult& SweepResult)
{
    ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    Collect(Character);
}

bool APickupActor::Collect(ACodeRescueCharacter* Character)
{
    if (!Character)
    {
        return false;
    }

    int32 Granted = 0;
    if (Kind == EPickupKind::Ammo)
    {
        Granted = Character->AddAmmo(Amount);
        if (Granted <= 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Cyan, TEXT("Ammo already full."));
            }
            return false;
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Cyan,
                FString::Printf(TEXT("+%d ammo (now %d / %d)"), Granted, Character->Ammo, Character->MaxAmmo));
        }
    }
    else if (Kind == EPickupKind::Medkit)
    {
        Granted = Character->AddMedkits(Amount);
        if (Granted <= 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green, TEXT("Medkits already full."));
            }
            return false;
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Green,
                FString::Printf(TEXT("+%d medkit (now %d / %d)"), Granted, Character->Medkits, Character->MaxMedkits));
        }
    }
    else
    {
        switch (Kind)
        {
        case EPickupKind::Flare:
            Granted = Character->AddFlares(Amount);
            break;
        case EPickupKind::Smoke:
            Granted = Character->AddSmokes(Amount);
            break;
        case EPickupKind::Stim:
            Granted = Character->AddStims(Amount);
            break;
        case EPickupKind::Scrap:
            Granted = Character->GrantScrap(Amount) >= 0 ? Amount : 0;
            break;
        case EPickupKind::ArmorPlate:
            Granted = Character->AddArmorPlates(Amount);
            break;
        case EPickupKind::RadioScanner:
            Granted = Character->AddRadioScannerCharges(Amount);
            break;
        case EPickupKind::FlashlightBattery:
            Granted = Character->AddFlashlightBatteries(Amount);
            break;
        case EPickupKind::AmmoPouch:
            Granted = Character->AddAmmoPouch(Amount);
            break;
        case EPickupKind::BypassKit:
            Granted = Character->AddBypassKits(Amount);
            break;
        default:
            break;
        }

        if (Granted <= 0)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.5f, PickupKindColor(Kind).ToFColor(true),
                    FString::Printf(TEXT("%s already full."), PickupKindLabel(Kind)));
            }
            return false;
        }
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.5f, PickupKindColor(Kind).ToFColor(true),
                FString::Printf(TEXT("+%d %s"), Granted, PickupKindLabel(Kind)));
        }
    }

    if (UWorld* World = Character->GetWorld())
    {
        if (UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>())
        {
            GI->SavePersistentRun();
        }
    }

    Destroy();
    return true;
}
