#include "CaseFilePickupActor.h"

#include "CodeRescueCharacter.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueSubtitlesWidget.h"
#include "CollisionQueryParams.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
FString CaseFileSnippet(FString Body)
{
    Body.ReplaceInline(TEXT("\n"), TEXT(" "));
    Body.ReplaceInline(TEXT("\r"), TEXT(" "));
    while (Body.Contains(TEXT("  ")))
    {
        Body.ReplaceInline(TEXT("  "), TEXT(" "));
    }
    return Body.Len() > 180 ? Body.Left(177) + TEXT("...") : Body;
}

FString BuildCaseFileSubtitle(
    const FString& Title,
    const FString& Body,
    const UCodeRescueGameInstance* GI)
{
    const FString LanguageName = GI ? GI->GetLanguageName() : FString(TEXT("selected language"));
    const FString CollectionSummary = GI ? GI->GetCaseFileCollectionSummary() : FString(TEXT("Case files: profile unavailable"));
    return FString::Printf(
        TEXT("CASE FILE: %s\n%s\nSaved to %s run. %s"),
        *Title,
        *CaseFileSnippet(Body),
        *LanguageName,
        *CollectionSummary);
}
}

ACaseFilePickupActor::ACaseFilePickupActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaseFileMesh"));
    RootComponent = MeshComp;
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetRelativeScale3D(FVector(0.72f, 0.48f, 0.08f));

    TriggerComp = CreateDefaultSubobject<USphereComponent>(TEXT("CaseFileTrigger"));
    TriggerComp->SetupAttachment(MeshComp);
    TriggerComp->InitSphereRadius(210.0f);
    TriggerComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerComp->SetCollisionObjectType(CodeRescueCollision::PickupObject);
    TriggerComp->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    TriggerComp->ComponentTags.AddUnique(FName("CollisionChannel_PickupObject"));
    TriggerComp->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));
    TriggerComp->OnComponentBeginOverlap.AddDynamic(this, &ACaseFilePickupActor::OnTriggerOverlap);

    GlowComp = CreateDefaultSubobject<UPointLightComponent>(TEXT("CaseFileGlow"));
    GlowComp->SetupAttachment(MeshComp);
    GlowComp->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));
    GlowComp->SetIntensity(1450.0f);
    GlowComp->SetAttenuationRadius(360.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComp->SetStaticMesh(CubeMesh.Object);
    }

    Tags.AddUnique(FName("CollectibleCaseFile"));
    Tags.AddUnique(FName("CaseFilePickup"));
    Tags.AddUnique(FName("WorldDevelopmentDeepDive"));
    Tags.AddUnique(FName("Top50Recommendations"));
    Tags.AddUnique(FName("ReleaseDossier"));
}

void ACaseFilePickupActor::BeginPlay()
{
    Super::BeginPlay();

    if (const UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        if (GI->HasCollectedCaseFile(CaseFileId))
        {
            bCollected = true;
            SetCollectedVisualState();
            return;
        }
    }

    SnapToGround();
    ApplyVisualState();
}

void ACaseFilePickupActor::SnapToGround()
{
    if (!bSnapToGround || !GetWorld())
    {
        return;
    }

    const FVector CurrentLocation = GetActorLocation();
    const FVector Start = CurrentLocation + FVector(0.0f, 0.0f, 300.0f);
    const FVector End = CurrentLocation - FVector(0.0f, 0.0f, 1500.0f);

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CodeRescueCaseFileGroundSnap), false, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        SetActorLocation(Hit.ImpactPoint + FVector(0.0f, 0.0f, GroundClearance));
        Tags.AddUnique(FName("CaseFileGroundSnapped"));
    }
}

void ACaseFilePickupActor::ApplyVisualState()
{
    if (GlowComp)
    {
        GlowComp->SetLightColor(CaseFileTint);
    }

    if (MeshComp)
    {
        if (UMaterialInstanceDynamic* Mat = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
        {
            Mat->SetVectorParameterValue(TEXT("Color"), CaseFileTint * 3.2f);
            Mat->SetVectorParameterValue(TEXT("BaseColor"), CaseFileTint * 1.6f);
            Mat->SetVectorParameterValue(TEXT("EmissiveColor"), CaseFileTint * 2.8f);
        }
    }
}

void ACaseFilePickupActor::SetCollectedVisualState()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    if (MeshComp)
    {
        MeshComp->SetVisibility(false, true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (TriggerComp)
    {
        TriggerComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        TriggerComp->SetGenerateOverlapEvents(false);
    }
    if (GlowComp)
    {
        GlowComp->SetVisibility(false);
        GlowComp->SetIntensity(0.0f);
    }
}

void ACaseFilePickupActor::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                            bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACodeRescueCharacter* Character = Cast<ACodeRescueCharacter>(OtherActor))
    {
        Collect(Character);
    }
}

bool ACaseFilePickupActor::Collect(ACodeRescueCharacter* Character)
{
    if (!Character || CaseFileId.IsEmpty())
    {
        return false;
    }

    UCodeRescueGameInstance* GI = Character->GetGameInstance<UCodeRescueGameInstance>();
    if (!GI)
    {
        return false;
    }

    if (GI->HasCollectedCaseFile(CaseFileId))
    {
        bCollected = true;
        SetCollectedVisualState();
        return false;
    }

    GI->RecordCaseFileCollected(CaseFileId, CaseFileTitle);
    GI->SavePersistentRun();
    bCollected = true;

    UCodeRescueSubtitlesWidget::Push(BuildCaseFileSubtitle(CaseFileTitle, CaseFileBody, GI), 6.0f);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.0f,
            CaseFileTint.ToFColor(true),
            FString::Printf(TEXT("Case file collected: %s"), *CaseFileTitle));
    }

    Destroy();
    return true;
}
