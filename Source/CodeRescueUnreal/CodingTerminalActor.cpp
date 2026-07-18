#include "CodingTerminalActor.h"
#include "CodeRescueCollisionChannels.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ACodingTerminalActor::ACodingTerminalActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ConsoleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConsoleMesh"));
    RootComponent = ConsoleMesh;
    ConsoleMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    ConsoleMesh->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    ConsoleMesh->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));
    ConsoleMesh->SetRelativeScale3D(FVector(1.2f, 0.35f, 0.9f));

    TerminalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TerminalLight"));
    TerminalLight->SetupAttachment(ConsoleMesh);
    TerminalLight->SetRelativeLocation(FVector(0, 0, 120));
    TerminalLight->SetLightColor(FLinearColor(0.0f, 0.7f, 1.0f));
    TerminalLight->SetIntensity(1000.0f);
    TerminalLight->SetAttenuationRadius(350.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded()) ConsoleMesh->SetStaticMesh(CubeMesh.Object);

    Challenge.Id = TEXT("generator_sum");
    Challenge.Title = TEXT("Generator Sum Repair");
    Challenge.MissionBrief = TEXT("Write the function required to restore grid power.");
    Challenge.Language = ECodingLanguage::Java;
    Challenge.StarterCode = TEXT("public static int totalPower(int a, int b, int c) {\n    return a + b + c;\n}\n");
}

void ACodingTerminalActor::MarkSolved()
{
    bSolved = true;
    ClearHelperActors();
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

void ACodingTerminalActor::AddHelperActor(AActor* HelperActor)
{
    if (IsValid(HelperActor) && HelperActor != this)
    {
        HelperActors.Add(HelperActor);
    }
}

void ACodingTerminalActor::ClearHelperActors()
{
    for (TWeakObjectPtr<AActor>& HelperPtr : HelperActors)
    {
        if (AActor* Helper = HelperPtr.Get())
        {
            Helper->Destroy();
        }
    }
    HelperActors.Reset();
}
