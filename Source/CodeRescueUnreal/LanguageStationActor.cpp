#include "LanguageStationActor.h"
#include "CodeRescueGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ALanguageStationActor::ALanguageStationActor()
{
    PrimaryActorTick.bCanEverTick = false;

    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    RootComponent = StationMesh;
    StationMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    StationMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.6f));

    StationLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StationLight"));
    StationLight->SetupAttachment(StationMesh);
    StationLight->SetRelativeLocation(FVector(0, 0, 180));
    StationLight->SetLightColor(FLinearColor(0.96f, 0.58f, 0.20f));
    StationLight->SetIntensity(1100.0f);
    StationLight->SetAttenuationRadius(360.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded()) StationMesh->SetStaticMesh(CylinderMesh.Object);
}

void ALanguageStationActor::ActivateStation()
{
    if (UCodeRescueGameInstance* GI = GetGameInstance<UCodeRescueGameInstance>())
    {
        if (GI->SelectedLanguage == Language)
        {
            GI->SavePersistentRun();
        }
    }
}
