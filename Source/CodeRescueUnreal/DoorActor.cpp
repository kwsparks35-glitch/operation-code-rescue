#include "DoorActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADoorActor::ADoorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    HingeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
    HingeRoot->SetupAttachment(Root);

    Leaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Leaf"));
    Leaf->SetupAttachment(HingeRoot);
    Leaf->SetMobility(EComponentMobility::Movable);
    Leaf->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Leaf->SetGenerateOverlapEvents(false);
    Leaf->ComponentTags.AddUnique(FName("RuntimeWeaponTraceBlocking"));

    Tags.AddUnique(FName("CodeRescueDoor"));
    Tags.AddUnique(FName("WorldDevelopment"));
    Tags.AddUnique(FName("FirstLevelEnvironmentPhysics"));
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();

    if (!Leaf->GetStaticMesh())
    {
        static const TCHAR* MeshCandidates[] = {
            TEXT("/Game/CodeRescueArt/WorldKitV4/SM_Door_Steel/SM_Door_Steel/StaticMeshes/SM_Door_Steel.SM_Door_Steel"),
            TEXT("/Game/CodeRescueArt/WorldKitV4/SM_Door_Steel/StaticMeshes/SM_Door_Steel.SM_Door_Steel"),
            TEXT("/Engine/BasicShapes/Cube.Cube")};
        for (const TCHAR* Path : MeshCandidates)
        {
            if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path))
            {
                Leaf->SetStaticMesh(Mesh);
                if (FCString::Strstr(Path, TEXT("BasicShapes")))
                {
                    // primitive fallback approximating a leaf
                    Leaf->SetRelativeScale3D(FVector(1.0f, 0.06f, 2.1f));
                    Leaf->SetRelativeLocation(FVector(52.0f, 0.0f, 105.0f));
                }
                break;
            }
        }
    }
}

void ADoorActor::ConfigureLeaf(float WidthScale, float HeightScale, bool bMirrorSwing)
{
    if (Leaf && Leaf->GetStaticMesh() && !FCString::Strstr(*Leaf->GetStaticMesh()->GetPathName(), TEXT("BasicShapes")))
    {
        Leaf->SetRelativeScale3D(FVector(WidthScale, 1.0f, HeightScale));
    }
    OpenYawDelta = bMirrorSwing ? -105.0f : 105.0f;
}

void ADoorActor::SetDoorOpenInstant(bool bNewOpen)
{
    bOpen = bNewOpen;
    AutoCloseRemaining = 0.0f;   // audit-held state: no auto-close countdown
    HingeRoot->SetRelativeRotation(FRotator(0.0f, bOpen ? OpenYawDelta : 0.0f, 0.0f));
}

void ADoorActor::ToggleDoor()
{
    bOpen = !bOpen;
    AutoCloseRemaining = bOpen ? AutoCloseSeconds : 0.0f;
    UE_LOG(LogTemp, Display, TEXT("[DoorActor] %s -> %s"), *GetName(),
        bOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

void ADoorActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bOpen && AutoCloseSeconds > 0.0f)
    {
        AutoCloseRemaining -= DeltaSeconds;
        if (AutoCloseRemaining <= 0.0f)
        {
            bOpen = false;
            UE_LOG(LogTemp, Verbose, TEXT("[DoorActor] %s auto-closed"), *GetName());
        }
    }

    const float TargetYaw = bOpen ? OpenYawDelta : 0.0f;
    const FRotator Current = HingeRoot->GetRelativeRotation();
    if (!FMath::IsNearlyEqual(Current.Yaw, TargetYaw, 0.05f))
    {
        const float NewYaw = FMath::FInterpConstantTo(Current.Yaw, TargetYaw, DeltaSeconds, 220.0f);
        HingeRoot->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));
    }
}
