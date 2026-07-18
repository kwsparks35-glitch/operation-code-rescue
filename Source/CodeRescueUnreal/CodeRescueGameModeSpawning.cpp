#include "CodeRescueGameMode.h"
#include "CodeRescueCampaign.h"
#include "CodeRescueMaterialUtils.h"
#include "CodeZombieActor.h"
#include "CompanionActor.h"
#include "FriendlyNPCActor.h"
#include "SurvivorActor.h"

#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"

namespace
{
    // 2026-07-04 world-physics pass: authored glTF/FBX meshes ship without collision primitives, so
    // "solid" walls were walk-through. Complex-as-simple gives every imported mesh real blocking
    // collision without hand-authored hulls (fine at this poly budget; meshes are Movable).
    void EnsureComplexAsSimpleCollision(UStaticMesh* Mesh)
    {
        if (Mesh)
        {
            if (UBodySetup* Body = Mesh->GetBodySetup())
            {
                if (Body->CollisionTraceFlag != CTF_UseComplexAsSimple)
                {
                    Body->CollisionTraceFlag = CTF_UseComplexAsSimple;
                }
            }
        }
    }
}

AActor* ACodeRescueGameMode::SpawnBlock(
    const FVector& Location,
    const FVector& Scale,
    const FLinearColor& Color,
    const FString& Name,
    bool bEnableCollision)
{
    AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator::ZeroRotator);
    if (!Block || !CubeMesh)
    {
        return nullptr;
    }

    UStaticMeshComponent* MeshComp = Block->GetStaticMeshComponent();
    MeshComp->SetMobility(EComponentMobility::Movable);
    MeshComp->SetStaticMesh(CubeMesh);
    MeshComp->SetWorldScale3D(Scale);
    MeshComp->SetCastShadow(false);
    if (!bEnableCollision)
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Block->SetActorEnableCollision(false);
    }

    CodeRescueMaterials::ApplyTintedMaterial(MeshComp, 0, Block, Color, 1.0f);

#if WITH_EDITOR
    Block->SetActorLabel(Name);
#endif

    RegisterStreamedActor(Block);
    return Block;
}

AActor* ACodeRescueGameMode::SpawnRotatedBlock(
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale,
    const FLinearColor& Color,
    const FString& Name,
    bool bEnableCollision)
{
    AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        Rotation);
    if (!Block || !CubeMesh)
    {
        return nullptr;
    }

    UStaticMeshComponent* MeshComp = Block->GetStaticMeshComponent();
    MeshComp->SetMobility(EComponentMobility::Movable);
    MeshComp->SetStaticMesh(CubeMesh);
    MeshComp->SetWorldScale3D(Scale);
    MeshComp->SetCastShadow(true);
    if (!bEnableCollision)
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Block->SetActorEnableCollision(false);
    }

    CodeRescueMaterials::ApplyTintedMaterial(MeshComp, 0, Block, Color, 1.0f);

#if WITH_EDITOR
    Block->SetActorLabel(Name);
#endif

    RegisterStreamedActor(Block);
    return Block;
}

AActor* ACodeRescueGameMode::SpawnKitMesh(
    const FString& MeshObjectPath,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale,
    const FString& Name,
    bool bEnableCollision,
    const TCHAR* MaterialPath)
{
    // Load the cooked authored mesh (packaged via DirectoriesToAlwaysCook=/Game/CodeRescueArt).
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshObjectPath);
    if (!Mesh)
    {
        return nullptr;
    }
    AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(), Location, Rotation);
    if (!Actor)
    {
        return nullptr;
    }
    UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
    MeshComp->SetMobility(EComponentMobility::Movable);
    if (bEnableCollision)
    {
        EnsureComplexAsSimpleCollision(Mesh);   // 2026-07-04: imported art must actually block
    }
    MeshComp->SetStaticMesh(Mesh);
    MeshComp->SetWorldScale3D(Scale);
    MeshComp->SetCastShadow(true);
    // 2026-07-01 (round 5): the authored glTF kit imported without a usable surface material, so it
    // rendered as the engine checkerboard. Apply a solid cooked material (StarterContent concrete/
    // brick/metal) across every slot so the buildings read as real architecture, not placeholders.
    if (MaterialPath)
    {
        if (UMaterialInterface* KitMaterial = CodeRescueMaterials::LoadMaterial(MaterialPath))
        {
            const int32 SlotCount = FMath::Max(1, MeshComp->GetNumMaterials());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot)
            {
                MeshComp->SetMaterial(Slot, KitMaterial);
            }
        }
    }
    if (!bEnableCollision)
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Actor->SetActorEnableCollision(false);
    }
#if WITH_EDITOR
    Actor->SetActorLabel(Name);
#endif
    RegisterStreamedActor(Actor);
    return Actor;
}

AActor* ACodeRescueGameMode::SpawnZombieReadabilityMarker(
    ACodeZombieActor* Zombie,
    const FLinearColor& AccentColor,
    const FString& Name,
    float Scale)
{
    if (!IsValid(Zombie))
    {
        return nullptr;
    }
    float CapsuleHalfHeight = 88.0f;
    if (const UCapsuleComponent* Capsule = Zombie->GetCapsuleComponent())
    {
        CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    }
    const FVector ZombieLocation = Zombie->GetActorLocation();
    const FVector MarkerLocation(
        ZombieLocation.X,
        ZombieLocation.Y,
        ZombieLocation.Z - CapsuleHalfHeight + 2.5f);
    AActor* Marker = SpawnKitMesh(
        TEXT("/Game/CodeRescueArt/WorldLootWeatherV6/ThreatGroundRingV6/ThreatGroundRingV6/StaticMeshes/ThreatGroundRingV6.ThreatGroundRingV6"),
        MarkerLocation,
        FRotator::ZeroRotator,
        FVector(FMath::Clamp(Scale, 0.45f, 2.25f)),
        Name,
        false,
        nullptr);
    if (!Marker)
    {
        return nullptr;
    }
    Marker->AttachToActor(Zombie, FAttachmentTransformRules::KeepWorldTransform);
    Marker->Tags.AddUnique(FName("CompactZombieGroundMarker"));
    Marker->Tags.AddUnique(FName("BlenderAuthoredThreatSymbol"));
    Marker->Tags.AddUnique(FName("ThreatMarkerAttachedToZombie"));
    Marker->Tags.AddUnique(FName("NoZombieEnclosureCube"));
    Marker->Tags.AddUnique(FName(*FString::Printf(
        TEXT("ThreatAccent_%02X%02X%02X"),
        AccentColor.ToFColor(true).R,
        AccentColor.ToFColor(true).G,
        AccentColor.ToFColor(true).B)));
    if (AStaticMeshActor* StaticMarker = Cast<AStaticMeshActor>(Marker))
    {
        if (UStaticMeshComponent* MarkerMesh = StaticMarker->GetStaticMeshComponent())
        {
            MarkerMesh->SetCastShadow(false);
            MarkerMesh->SetReceivesDecals(false);
            MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    return Marker;
}

AActor* ACodeRescueGameMode::SpawnTexturedBlock(
    const FVector& Location,
    const FVector& Scale,
    const FLinearColor& FallbackColor,
    const FString& Name,
    const TCHAR* MaterialPath,
    bool bEnableCollision)
{
    AActor* Actor = SpawnBlock(Location, Scale, FallbackColor, Name, bEnableCollision);
    if (!Actor)
    {
        return nullptr;
    }

    if (UMaterialInterface* Material = CodeRescueMaterials::LoadMaterial(MaterialPath))
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
            {
                MeshComp->SetMaterial(0, Material);
                MeshComp->SetCastShadow(true);
            }
        }
    }
    return Actor;
}

AActor* ACodeRescueGameMode::SpawnStaticMeshProp(
    UStaticMesh* Mesh,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& BlockScale,
    const FString& Name,
    bool bEnableCollision,
    const TCHAR* MaterialOverride)
{
    if (!Mesh)
    {
        return nullptr;
    }

    AStaticMeshActor* Prop = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        Rotation);
    if (!Prop)
    {
        return nullptr;
    }

    UStaticMeshComponent* MeshComp = Prop->GetStaticMeshComponent();
    MeshComp->SetMobility(EComponentMobility::Movable);
    if (bEnableCollision)
    {
        EnsureComplexAsSimpleCollision(Mesh);   // 2026-07-04: imported art must actually block
    }
    MeshComp->SetStaticMesh(Mesh);
    MeshComp->SetCastShadow(true);

    const FBox MeshBox = Mesh->GetBoundingBox();
    const FVector MeshSize = MeshBox.GetSize();
    const FVector TargetSize(
        FMath::Max(1.0f, BlockScale.X * 100.0f),
        FMath::Max(1.0f, BlockScale.Y * 100.0f),
        FMath::Max(1.0f, BlockScale.Z * 100.0f));
    const FVector PropScale(
        MeshSize.X > 1.0f ? TargetSize.X / MeshSize.X : 1.0f,
        MeshSize.Y > 1.0f ? TargetSize.Y / MeshSize.Y : 1.0f,
        MeshSize.Z > 1.0f ? TargetSize.Z / MeshSize.Z : 1.0f);

    MeshComp->SetWorldScale3D(PropScale);
    MeshComp->SetRelativeLocation(-MeshBox.GetCenter() * PropScale);

    if (!bEnableCollision)
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Prop->SetActorEnableCollision(false);
    }

    // 2026-07-02: kill the engine checkerboard. Some content-pack props (notably the StarterContent
    // glass panes used as safehouse/observation panels) render as the teal/magenta checker in a
    // cooked build. When a caller passes MaterialOverride, force that solid material onto EVERY slot
    // (deterministic fix for known-bad props). Otherwise, fill only genuinely-null slots with a
    // solid concrete fallback so intended materials are never overridden.
    if (MaterialOverride)
    {
        if (UMaterialInterface* Forced = CodeRescueMaterials::LoadMaterial(MaterialOverride))
        {
            const int32 SlotCount = FMath::Max(1, MeshComp->GetNumMaterials());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot)
            {
                MeshComp->SetMaterial(Slot, Forced);
            }
        }
    }
    else
    {
        const int32 SlotCount = MeshComp->GetNumMaterials();
        for (int32 Slot = 0; Slot < SlotCount; ++Slot)
        {
            UMaterialInterface* Current = MeshComp->GetMaterial(Slot);
            if (!Current)
            {
                // Empty slot -> checkerboard. Fill with solid concrete.
                if (UMaterialInterface* Fallback = CodeRescueMaterials::LoadMaterial(
                        TEXT("/Game/StarterContent/Materials/M_Concrete_Poured.M_Concrete_Poured")))
                {
                    MeshComp->SetMaterial(Slot, Fallback);
                }
            }
            else if (Current->GetName().Contains(TEXT("M_Glass")))
            {
                // 2026-07-02 (diag-confirmed): the StarterContent SM_GlassWindow panes default to
                // M_Glass, which renders as a translucent teal grid that read as the residual
                // "checker" eyesore across the safehouse. Swap to solid steel so the windows read
                // as clean opaque panels.
                if (UMaterialInterface* Steel = CodeRescueMaterials::LoadMaterial(
                        TEXT("/Game/StarterContent/Materials/M_Metal_Steel.M_Metal_Steel")))
                {
                    MeshComp->SetMaterial(Slot, Steel);
                }
            }
        }
    }

#if WITH_EDITOR
    Prop->SetActorLabel(Name);
#endif

    RegisterStreamedActor(Prop);
    return Prop;
}

AActor* ACodeRescueGameMode::SpawnDecorativeCivilian(
    const FVector& Location,
    const FRotator& Rotation,
    bool bUseQuinn,
    const FLinearColor& BadgeColor,
    const FString& Name,
    const FString& DisplayLabel)
{
    USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(
        nullptr,
        bUseQuinn
            ? TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn")
            : TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
    UClass* AnimClass = LoadClass<UAnimInstance>(
        nullptr,
        bUseQuinn
            ? TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C")
            : TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C"));

    if (!Mesh)
    {
        return SpawnBlock(
            Location + FVector(0.0f, 0.0f, 80.0f),
            FVector(0.55f, 0.55f, 1.45f),
            BadgeColor,
            Name + TEXT(" Fallback Civilian"),
            false);
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Civilian = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, Params);
    if (!Civilian)
    {
        return nullptr;
    }
    // 2026-07-11: tag so the post-build GroundSpawnedCharacters pass can find
    // and re-ground these gravity-less decorative actors.
    Civilian->Tags.AddUnique(FName("DecorativeCivilian"));

    USceneComponent* Root = NewObject<USceneComponent>(Civilian, TEXT("DecorativeCivilianRoot"));
    Civilian->AddInstanceComponent(Root);
    Civilian->SetRootComponent(Root);
    Root->RegisterComponent();
    Root->SetWorldLocation(Location);
    Root->SetWorldRotation(Rotation);

    USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(Civilian, TEXT("DecorativeCivilianMesh"));
    Civilian->AddInstanceComponent(MeshComp);
    MeshComp->SetupAttachment(Root);
    MeshComp->RegisterComponent();
    MeshComp->SetSkeletalMesh(Mesh);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetCastShadow(true);
    MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    if (AnimClass)
    {
        MeshComp->SetAnimInstanceClass(AnimClass);
    }

    UStaticMesh* CubeMeshLocal = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto TintComponent = [this, &BadgeColor](UStaticMeshComponent* Component, float EmissiveScale)
    {
        if (!Component)
        {
            return;
        }
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CodeRescueMaterials::ApplyTintedMaterial(Component, 0, this, BadgeColor, EmissiveScale);
    };
    if (CubeMeshLocal)
    {
        UStaticMeshComponent* ChestBadge = NewObject<UStaticMeshComponent>(Civilian, TEXT("CharacterRecognitionBadge"));
        Civilian->AddInstanceComponent(ChestBadge);
        ChestBadge->SetupAttachment(Root);
        ChestBadge->RegisterComponent();
        ChestBadge->SetStaticMesh(CubeMeshLocal);
        ChestBadge->SetRelativeLocation(FVector(40.0f, -25.0f, 122.0f));
        ChestBadge->SetRelativeScale3D(FVector(0.18f, 0.045f, 0.18f));
        TintComponent(ChestBadge, 0.9f);

        UStaticMeshComponent* ShoulderSash = NewObject<UStaticMeshComponent>(Civilian, TEXT("CharacterRecognitionSash"));
        Civilian->AddInstanceComponent(ShoulderSash);
        ShoulderSash->SetupAttachment(Root);
        ShoulderSash->RegisterComponent();
        ShoulderSash->SetStaticMesh(CubeMeshLocal);
        ShoulderSash->SetRelativeLocation(FVector(25.0f, -30.0f, 102.0f));
        ShoulderSash->SetRelativeRotation(FRotator(0.0f, 0.0f, -22.0f));
        ShoulderSash->SetRelativeScale3D(FVector(0.10f, 0.035f, 0.55f));
        TintComponent(ShoulderSash, 0.55f);

        // City wardrobe pieces are deterministic per civilian name, so streamed
        // city reloads keep outfits stable while still using each city palette.
        if (ActiveCityWardrobePalette.Num() > 0)
        {
            const uint32 OutfitHash = GetTypeHash(Name);
            const FLinearColor Outfit = ActiveCityWardrobePalette[OutfitHash % static_cast<uint32>(ActiveCityWardrobePalette.Num())];
            const FLinearColor OutfitAccent = ActiveCityWardrobePalette[(OutfitHash / 7u) % static_cast<uint32>(ActiveCityWardrobePalette.Num())];

            auto AddWardrobePiece = [&](const TCHAR* PieceName, const FVector& Loc, const FRotator& Rot, const FVector& Scale, const FLinearColor& Color)
            {
                UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(Civilian, PieceName);
                Civilian->AddInstanceComponent(Piece);
                Piece->SetupAttachment(Root);
                Piece->RegisterComponent();
                Piece->SetStaticMesh(CubeMeshLocal);
                Piece->SetRelativeLocation(Loc);
                Piece->SetRelativeRotation(Rot);
                Piece->SetRelativeScale3D(Scale);
                CodeRescueMaterials::ApplyTintedMaterial(Piece, 0, this, Color, 0.12f);
            };

            AddWardrobePiece(TEXT("CityWardrobeJacketFront"), FVector(34.0f, 0.0f, 118.0f), FRotator::ZeroRotator, FVector(0.10f, 0.42f, 0.62f), Outfit);
            AddWardrobePiece(TEXT("CityWardrobeJacketBack"), FVector(-34.0f, 0.0f, 118.0f), FRotator::ZeroRotator, FVector(0.10f, 0.44f, 0.64f), Outfit * 0.85f);
            AddWardrobePiece(TEXT("CityWardrobeLegBand"), FVector(0.0f, 0.0f, 28.0f), FRotator::ZeroRotator, FVector(0.30f, 0.40f, 0.46f), OutfitAccent * 0.6f);

            const FString& Acc = ActiveCityWardrobeAccessory;
            if (Acc == TEXT("Beanie"))
            {
                AddWardrobePiece(TEXT("CityWardrobeBeanie"), FVector(0.0f, 0.0f, 186.0f), FRotator::ZeroRotator, FVector(0.24f, 0.26f, 0.14f), OutfitAccent);
            }
            else if (Acc == TEXT("SunHat"))
            {
                AddWardrobePiece(TEXT("CityWardrobeSunHatBrim"), FVector(0.0f, 0.0f, 182.0f), FRotator::ZeroRotator, FVector(0.46f, 0.46f, 0.035f), FLinearColor(0.82f, 0.74f, 0.52f));
                AddWardrobePiece(TEXT("CityWardrobeSunHatCrown"), FVector(0.0f, 0.0f, 194.0f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.12f), FLinearColor(0.82f, 0.74f, 0.52f) * 0.9f);
            }
            else if (Acc == TEXT("CowboyHat"))
            {
                AddWardrobePiece(TEXT("CityWardrobeCowboyBrim"), FVector(0.0f, 0.0f, 184.0f), FRotator(0.0f, 0.0f, 4.0f), FVector(0.55f, 0.40f, 0.04f), FLinearColor(0.38f, 0.26f, 0.14f));
                AddWardrobePiece(TEXT("CityWardrobeCowboyCrown"), FVector(0.0f, 0.0f, 198.0f), FRotator::ZeroRotator, FVector(0.26f, 0.20f, 0.15f), FLinearColor(0.32f, 0.22f, 0.12f));
            }
            else if (Acc == TEXT("Scarf"))
            {
                AddWardrobePiece(TEXT("CityWardrobeScarf"), FVector(0.0f, 0.0f, 148.0f), FRotator(0.0f, 0.0f, 8.0f), FVector(0.30f, 0.34f, 0.10f), OutfitAccent * 1.2f);
            }
            else if (Acc == TEXT("ParkaHood"))
            {
                AddWardrobePiece(TEXT("CityWardrobeParkaHood"), FVector(-22.0f, 0.0f, 178.0f), FRotator::ZeroRotator, FVector(0.20f, 0.34f, 0.30f), Outfit * 1.15f);
                AddWardrobePiece(TEXT("CityWardrobeParkaTrim"), FVector(0.0f, 0.0f, 146.0f), FRotator::ZeroRotator, FVector(0.32f, 0.36f, 0.06f), FLinearColor(0.62f, 0.58f, 0.50f));
            }
            else if (Acc == TEXT("Backpack"))
            {
                AddWardrobePiece(TEXT("CityWardrobeBackpack"), FVector(-44.0f, 0.0f, 116.0f), FRotator::ZeroRotator, FVector(0.16f, 0.34f, 0.46f), OutfitAccent);
            }
            else if (Acc == TEXT("Lanyard"))
            {
                AddWardrobePiece(TEXT("CityWardrobeLanyard"), FVector(40.0f, 8.0f, 110.0f), FRotator::ZeroRotator, FVector(0.035f, 0.07f, 0.30f), FLinearColor(0.10f, 0.45f, 0.60f));
            }
            else
            {
                AddWardrobePiece(TEXT("CityWardrobeCapCrown"), FVector(0.0f, 0.0f, 190.0f), FRotator::ZeroRotator, FVector(0.24f, 0.24f, 0.10f), OutfitAccent);
                AddWardrobePiece(TEXT("CityWardrobeCapBrim"), FVector(26.0f, 0.0f, 184.0f), FRotator::ZeroRotator, FVector(0.20f, 0.20f, 0.03f), OutfitAccent * 0.8f);
            }
        }
    }

    SpawnBlock(
        Location + FVector(0.0f, 0.0f, 190.0f),
        FVector(0.42f, 0.42f, 0.045f),
        BadgeColor * 2.4f,
        Name + TEXT(" Civilian Presence Halo"),
        false);
    if (!DisplayLabel.IsEmpty())
    {
        SpawnGuideText(
            DisplayLabel,
            Location + FVector(0.0f, 0.0f, 335.0f),
            BadgeColor.ToFColor(true),
            30.0f);
    }

#if WITH_EDITOR
    Civilian->SetActorLabel(Name);
#endif

    RegisterStreamedActor(Civilian);
    return Civilian;
}

// ---------------------------------------------------------------------------
// 2026-07-04 world-physics QA gate: `cr.AuditWorldSolidity [fix]`
//
// Scans every StaticMesh actor in the world and reports (a) how many have no
// collision and (b) how many collision-enabled actors FLOAT above the ground
// (bounds bottom > 60uu over the ground trace). With the "fix" argument it
// snaps floaters down onto the ground. Decor that intentionally hovers
// (beacons, terminal halos, sky layer) has no collision, so it is never
// counted as a floater.
static void CodeRescueAuditWorldSolidity(const TArray<FString>& Args, UWorld* World)
{
    if (!World)
    {
        return;
    }
    int32 Total = 0, NoCollision = 0, Floaters = 0, Fixed = 0;
    const bool bFix = Args.ContainsByPredicate([](const FString& A) { return A.Equals(TEXT("fix"), ESearchCase::IgnoreCase); });
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Actor = *It;
        UStaticMeshComponent* Comp = Actor ? Actor->GetStaticMeshComponent() : nullptr;
        if (!Comp || !Comp->GetStaticMesh())
        {
            continue;
        }
        ++Total;
        if (Comp->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
            ++NoCollision;
            continue;   // intentional decor (halos, beacons, sky) may hover
        }
        if (Actor->Tags.Contains(FName("SkyLayer")))
        {
            continue;
        }
        const FBox Bounds = Actor->GetComponentsBoundingBox(true);
        FHitResult Hit;
        FCollisionQueryParams Params(FName(TEXT("CRAudit")), false);
        Params.AddIgnoredActor(Actor);
        const FVector Probe(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Min.Z + 5.0f);
        if (World->LineTraceSingleByChannel(Hit, Probe, Probe - FVector(0, 0, 100000.0f), ECC_WorldStatic, Params))
        {
            const float Gap = Bounds.Min.Z - Hit.ImpactPoint.Z;
            if (Gap > 60.0f)
            {
                ++Floaters;
                if (bFix)
                {
                    Actor->AddActorWorldOffset(FVector(0.0f, 0.0f, -(Gap - 2.0f)));
                    ++Fixed;
                }
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[SolidityAudit] meshActors=%d noCollision=%d floaters=%d fixed=%d"),
        Total, NoCollision, Floaters, Fixed);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, Floaters > 0 ? FColor::Orange : FColor::Green,
            FString::Printf(TEXT("Solidity: %d meshes, %d no-collision (decor), %d floaters%s"),
                Total, NoCollision, Floaters,
                bFix ? *FString::Printf(TEXT(", %d snapped down"), Fixed) : TEXT("")));
    }
}

static FAutoConsoleCommandWithWorldAndArgs GCodeRescueAuditWorldSolidity(
    TEXT("cr.AuditWorldSolidity"),
    TEXT("Report StaticMesh actors with no collision and collision-enabled actors floating above ground. Pass 'fix' to snap floaters down."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&CodeRescueAuditWorldSolidity));

void ACodeRescueGameMode::GroundFloatingMeshes(int32 CityIndex)
{
    // Every city owns a canonical mission floor at Origin.Z. The previous
    // downward trace ignored that floor, hit the emergency catch floor, and
    // moved the mission floor from Z=0 to roughly Z=-145. On a resumed city
    // this produced the captured Z=-42 arena-recovery loop. Normalize only
    // broad near-ground slabs against the city origin; never infer playable
    // height from another collision actor below them.
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector Origin = FCodeRescueCampaign::GetCityOrigin(CityIndex);
    int32 FlattenedRegions = 0;
    int32 ElevatedRegions = 0;
    bool bCanonicalFloorFound = false;
    float CanonicalFloorTop = -TNumericLimits<float>::Max();

    auto IsNearGroundRegion = [&Origin](const FBox& Bounds)
    {
        if (!Bounds.IsValid)
        {
            return false;
        }
        const FVector Size = Bounds.GetSize();
        return Size.X >= 320.0f &&
            Size.Y >= 320.0f &&
            Size.Z <= 140.0f &&
            Bounds.Max.Z > Origin.Z + 28.0f &&
            Bounds.Max.Z < Origin.Z + 420.0f;
    };

    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Actor = *It;
        UStaticMeshComponent* Comp = Actor ? Actor->GetStaticMeshComponent() : nullptr;
        if (!Actor || !Comp || !Comp->GetStaticMesh() || Actor->IsHidden())
        {
            continue;
        }

        const FBox Bounds = Actor->GetComponentsBoundingBox(true);
        const FVector BoundsCenter = Bounds.GetCenter();
        if (!FCodeRescueCampaign::IsLocationInsideCityArenaXY(CityIndex, BoundsCenter, true))
        {
            continue;
        }

        if (Actor->Tags.Contains(FName("CanonicalMissionGround")))
        {
            bCanonicalFloorFound = true;
            CanonicalFloorTop = Bounds.Max.Z;
            continue;
        }

        if (Comp->GetCollisionEnabled() == ECollisionEnabled::NoCollision ||
            Actor->Tags.Contains(FName("SkyLayer")) ||
            Actor->Tags.Contains(FName("FallRecoveryCatchFloor")) ||
            Actor->Tags.Contains(FName("ArenaLockWall")) ||
            Actor->Tags.Contains(FName("GameplayArenaConfinement")))
        {
            continue;
        }

        if (!IsNearGroundRegion(Bounds))
        {
            continue;
        }

        if (Comp->Mobility != EComponentMobility::Movable)
        {
            Comp->SetMobility(EComponentMobility::Movable);
        }
        const float OffsetZ = Origin.Z + 12.0f - Bounds.Max.Z;
        Actor->AddActorWorldOffset(
            FVector(0.0f, 0.0f, OffsetZ),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        Actor->Tags.AddUnique(FName("CanonicalGroundedRegion"));
        ++FlattenedRegions;
    }

    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Actor = *It;
        UStaticMeshComponent* Comp = Actor ? Actor->GetStaticMeshComponent() : nullptr;
        if (!Actor || !Comp || !Comp->GetStaticMesh() || Actor->IsHidden() ||
            Comp->GetCollisionEnabled() == ECollisionEnabled::NoCollision ||
            Actor->Tags.Contains(FName("FallRecoveryCatchFloor")) ||
            Actor->Tags.Contains(FName("GameplayArenaConfinement")))
        {
            continue;
        }
        const FBox Bounds = Actor->GetComponentsBoundingBox(true);
        if (FCodeRescueCampaign::IsLocationInsideCityArenaXY(CityIndex, Bounds.GetCenter(), true) &&
            IsNearGroundRegion(Bounds))
        {
            ++ElevatedRegions;
        }
    }

    const bool bFloorLevel = bCanonicalFloorFound &&
        FMath::Abs(CanonicalFloorTop - Origin.Z) <= 8.0f;
    const bool bPass = bFloorLevel && ElevatedRegions == 0;
    if (bPass)
    {
        Tags.AddUnique(FName("CityGroundContinuityPass"));
        Tags.AddUnique(FName(*FString::Printf(TEXT("CityGroundContinuityCity%dPass"), CityIndex)));
        if (CityIndex == 0)
        {
            Tags.AddUnique(FName("FirstLevelIntegratedGroundPass"));
        }
        UE_LOG(LogTemp, Display,
            TEXT("[CityGroundContinuity] COMPLETE PASS city=%d canonical_floor=1 floor_top=%.2f flattened_regions=%d remaining_elevated=0"),
            CityIndex, CanonicalFloorTop, FlattenedRegions);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[CityGroundContinuity] COMPLETE FAIL city=%d canonical_floor=%d floor_top=%.2f flattened_regions=%d remaining_elevated=%d"),
            CityIndex,
            bCanonicalFloorFound ? 1 : 0,
            CanonicalFloorTop,
            FlattenedRegions,
            ElevatedRegions);
    }
}

// ---- 2026-07-11 floating-character fix ------------------------------------
// Kenny's packaged report: "many of the game's characters are not positioned
// on the ground; they appear to be floating above the ground." Root causes:
//   1. GroundFloatingMeshes LOWERS walkable slabs (by up to ~400 uu) AFTER
//      every character already self-snapped in BeginPlay, stranding anything
//      that stood on a moved slab in mid-air.
//   2. Survivors / friendly NPCs / decorative civilians are plain AActors —
//      no capsule, no CharacterMovement, NO GRAVITY — so one missed trace
//      floats forever. Companions and civilians never snapped at all.
//   3. The old per-class snaps traced ECC_Visibility only, silently no-op'd
//      on a miss, and happily snapped onto fall-recovery catch floors or
//      other characters.
// SnapCharacterBaseToGround is the one shared, robust snap; the city pass
// GroundSpawnedCharacters runs it for every character AFTER all geometry
// (including the slab flatten) is final.

float ACodeRescueGameMode::ComputeCharacterBottomOffset(const AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return 0.0f;
    }
    if (const ACharacter* AsCharacter = Cast<const ACharacter>(Actor))
    {
        if (const UCapsuleComponent* Capsule = AsCharacter->GetCapsuleComponent())
        {
            return Capsule->GetScaledCapsuleHalfHeight();
        }
    }
    // AActor-based characters: measure to the lowest visible mesh bottom so
    // the FEET land on the surface (light/audio components would inflate
    // GetActorBounds and are deliberately excluded).
    float LowestBottom = TNumericLimits<float>::Max();
    for (const UActorComponent* Component : Actor->GetComponents())
    {
        const UMeshComponent* MeshComponent = Cast<const UMeshComponent>(Component);
        if (!MeshComponent || !MeshComponent->IsRegistered() || !MeshComponent->IsVisible())
        {
            continue;
        }
        const FBoxSphereBounds& ComponentBounds = MeshComponent->Bounds;
        LowestBottom = FMath::Min(LowestBottom, static_cast<float>(ComponentBounds.Origin.Z - ComponentBounds.BoxExtent.Z));
    }
    if (LowestBottom == TNumericLimits<float>::Max())
    {
        return 0.0f;
    }
    return Actor->GetActorLocation().Z - LowestBottom;
}

bool ACodeRescueGameMode::AlignCharacterVisualFeetToCapsule(ACharacter* Character, float ContactLift)
{
    if (!IsValid(Character) ||
        Character->Tags.Contains(FName("CorpsePersistenceWindow")) ||
        Character->Tags.Contains(FName("ZombieDeathPhysicsActive")) ||
        Character->Tags.Contains(FName("NPCVisibleCorpseWindow")))
    {
        return false;
    }

    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
    if (!Capsule || !SkeletalMesh || !SkeletalMesh->IsRegistered() ||
        !SkeletalMesh->IsVisible() || !SkeletalMesh->GetSkinnedAsset())
    {
        return false;
    }

    SkeletalMesh->UpdateComponentToWorld();
    SkeletalMesh->UpdateBounds();
    const float CapsuleBaseZ = Capsule->Bounds.Origin.Z - Capsule->Bounds.BoxExtent.Z;
    const float VisibleBottomZ = SkeletalMesh->Bounds.Origin.Z - SkeletalMesh->Bounds.BoxExtent.Z;
    const float DeltaZ = CapsuleBaseZ + ContactLift - VisibleBottomZ;
    if (FMath::Abs(DeltaZ) > 0.5f && FMath::Abs(DeltaZ) <= 280.0f)
    {
        SkeletalMesh->AddWorldOffset(FVector(0.0f, 0.0f, DeltaZ), false, nullptr, ETeleportType::TeleportPhysics);
        SkeletalMesh->UpdateComponentToWorld();
        SkeletalMesh->UpdateBounds();
    }

    const float ResolvedBottomZ = SkeletalMesh->Bounds.Origin.Z - SkeletalMesh->Bounds.BoxExtent.Z;
    const bool bAligned = FMath::Abs(ResolvedBottomZ - (CapsuleBaseZ + ContactLift)) <= 4.0f;
    if (bAligned)
    {
        Character->Tags.AddUnique(FName("VisibleFeetAlignedToCapsule"));
        SkeletalMesh->ComponentTags.AddUnique(FName("VisibleFootGroundContactTarget"));
    }
    return bAligned;
}

bool ACodeRescueGameMode::SnapCharacterBaseToGround(AActor* Actor, float MinGroundZ, bool bSnapToFallbackOnMiss, float FallbackGroundZ)
{
    if (!IsValid(Actor))
    {
        return false;
    }
    UWorld* World = Actor->GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector Here = Actor->GetActorLocation();
    const FVector TraceStart = Here + FVector(0.0f, 0.0f, 250.0f);
    const FVector TraceEnd(Here.X, Here.Y, Here.Z - 4000.0f);

    // Only real world geometry counts as ground. Catch floors and other
    // characters are re-traced through instead of pre-scanning the world.
    auto IsGroundActor = [](const AActor* HitActor)
    {
        if (!HitActor)
        {
            return true;    // landscape/BSP
        }
        if (HitActor->Tags.Contains(FName("FallRecoveryCatchFloor")) ||
            HitActor->Tags.Contains(FName("DecorativeCivilian")))
        {
            return false;
        }
        return !HitActor->IsA<APawn>()
            && !HitActor->IsA<ASurvivorActor>()
            && !HitActor->IsA<AFriendlyNPCActor>();
    };

    bool bValidHit = false;
    float GroundZ = 0.0f;
    const ECollisionChannel Channels[] = { ECC_Visibility, ECC_WorldStatic };
    for (ECollisionChannel Channel : Channels)
    {
        FCollisionQueryParams Params(SCENE_QUERY_STAT(CRCharacterGroundSnap), false, Actor);
        for (int32 Attempt = 0; Attempt < 4; ++Attempt)
        {
            FHitResult Hit;
            if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, Channel, Params))
            {
                break;
            }
            if (!IsGroundActor(Hit.GetActor()))
            {
                Params.AddIgnoredActor(Hit.GetActor());
                continue;
            }
            if (Hit.ImpactPoint.Z >= MinGroundZ)
            {
                GroundZ = Hit.ImpactPoint.Z;
                bValidHit = true;
            }
            break;
        }
        if (bValidHit)
        {
            break;
        }
    }

    if (!bValidHit)
    {
        if (!bSnapToFallbackOnMiss)
        {
            return false;
        }
        GroundZ = FallbackGroundZ;
    }

    const float BottomOffset = ComputeCharacterBottomOffset(Actor);
    const FVector Snapped(Here.X, Here.Y, GroundZ + BottomOffset + 2.0f);
    if (FMath::Abs(Snapped.Z - Here.Z) <= 3.0f)
    {
        return false;
    }
    Actor->SetActorLocation(Snapped, false, nullptr, ETeleportType::TeleportPhysics);
    return true;
}

void ACodeRescueGameMode::GroundSpawnedCharacters(int32 CityIndex, const FVector& Origin, const FString& CityLabel)
{
    const float MinGroundZ = Origin.Z - 18.0f;
    const float CitySpanRadius = 20000.0f;   // generous: covers the scaled city footprint
    int32 Checked = 0;
    int32 Regrounded = 0;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    auto GroundOne = [&](AActor* CharacterActor)
    {
        if (!IsValid(CharacterActor) || CharacterActor == PlayerPawn)
        {
            return;
        }
        // Only this city's population; other cities run their own pass with
        // their own origin plane.
        if (FVector::Dist2D(CharacterActor->GetActorLocation(), Origin) > CitySpanRadius)
        {
            return;
        }
        if (CharacterActor->Tags.Contains(FName("CorpsePersistenceWindow")) ||
            CharacterActor->Tags.Contains(FName("ZombieDeathPhysicsActive")) ||
            CharacterActor->Tags.Contains(FName("NPCVisibleCorpseWindow")))
        {
            return;
        }
        ++Checked;
        Regrounded += SnapCharacterBaseToGround(CharacterActor, MinGroundZ, true, Origin.Z) ? 1 : 0;
        if (ACodeZombieActor* Zombie = Cast<ACodeZombieActor>(CharacterActor))
        {
            Zombie->RefreshGroundedVisualPose();
        }
        else if (ACompanionActor* Companion = Cast<ACompanionActor>(CharacterActor))
        {
            Companion->RefreshGroundedVisualPose();
        }
        else if (ACharacter* Character = Cast<ACharacter>(CharacterActor))
        {
            AlignCharacterVisualFeetToCapsule(Character);
        }
    };

    TArray<AActor*> CityCharacters;
    auto Collect = [&](AActor* CharacterActor)
    {
        if (IsValid(CharacterActor) && CharacterActor != PlayerPawn &&
            FVector::Dist2D(CharacterActor->GetActorLocation(), Origin) <= CitySpanRadius)
        {
            CityCharacters.Add(CharacterActor);
        }
    };
    for (TActorIterator<ACodeZombieActor> It(GetWorld()); It; ++It) { Collect(*It); }      // includes the boss subclass
    for (TActorIterator<ASurvivorActor> It(GetWorld()); It; ++It)   { Collect(*It); }
    for (TActorIterator<AFriendlyNPCActor> It(GetWorld()); It; ++It){ Collect(*It); }
    for (TActorIterator<ACompanionActor> It(GetWorld()); It; ++It)  { Collect(*It); }
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (It->Tags.Contains(FName("DecorativeCivilian")))
        {
            Collect(*It);
        }
    }

    for (AActor* CharacterActor : CityCharacters)
    {
        GroundOne(CharacterActor);
    }

    // Verification sweep: after snapping, NOBODY may still hang more than a
    // hand's width above the surface beneath them. This is the check the
    // packaged "floating characters" report proved was missing.
    int32 FloatingAfter = 0;
    int32 VisibleFootFailures = 0;
    int32 VisibleFootChecks = 0;
    auto TraceSupportingSurface = [this, MinGroundZ, Origin](AActor* Actor, float& OutSurfaceZ)
    {
        const FVector Location = Actor->GetActorLocation();
        const FVector Start(Location.X, Location.Y, Location.Z + 120.0f);
        const FVector End(Location.X, Location.Y, MinGroundZ - 120.0f);
        const ECollisionChannel Channels[] = { ECC_Visibility, ECC_WorldStatic };
        for (ECollisionChannel Channel : Channels)
        {
            FCollisionQueryParams Params(SCENE_QUERY_STAT(CRCharacterGroundAudit), false, Actor);
            for (int32 Attempt = 0; Attempt < 5; ++Attempt)
            {
                FHitResult Hit;
                if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, Params))
                {
                    break;
                }
                const AActor* HitActor = Hit.GetActor();
                if (HitActor && (HitActor->IsA<APawn>() ||
                    HitActor->Tags.Contains(FName("FallRecoveryCatchFloor")) ||
                    HitActor->Tags.Contains(FName("DecorativeCivilian"))))
                {
                    Params.AddIgnoredActor(HitActor);
                    continue;
                }
                if (Hit.ImpactPoint.Z >= MinGroundZ)
                {
                    OutSurfaceZ = Hit.ImpactPoint.Z;
                    return true;
                }
                break;
            }
        }
        OutSurfaceZ = Origin.Z;
        return false;
    };
    for (AActor* CharacterActor : CityCharacters)
    {
        if (!IsValid(CharacterActor) ||
            CharacterActor->Tags.Contains(FName("CorpsePersistenceWindow")) ||
            CharacterActor->Tags.Contains(FName("ZombieDeathPhysicsActive")) ||
            CharacterActor->Tags.Contains(FName("NPCVisibleCorpseWindow")))
        {
            continue;
        }
        const float BottomOffset = ComputeCharacterBottomOffset(CharacterActor);
        const float BaseZ = CharacterActor->GetActorLocation().Z - BottomOffset;
        float SurfaceZ = Origin.Z;
        TraceSupportingSurface(CharacterActor, SurfaceZ);
        if (FMath::Abs(BaseZ - FMath::Max(SurfaceZ, MinGroundZ)) > 25.0f)
        {
            ++FloatingAfter;
            UE_LOG(LogTemp, Warning,
                TEXT("[CharacterGroundingAudit] still_floating actor=%s base_z=%.1f surface_z=%.1f"),
                *CharacterActor->GetName(), BaseZ, SurfaceZ);
        }
        if (ACharacter* Character = Cast<ACharacter>(CharacterActor))
        {
            if (USkeletalMeshComponent* SkeletalMesh = Character->GetMesh())
            {
                if (SkeletalMesh->IsVisible() && SkeletalMesh->GetSkinnedAsset())
                {
                    ++VisibleFootChecks;
                    SkeletalMesh->UpdateComponentToWorld();
                    SkeletalMesh->UpdateBounds();
                    const float VisibleBottomZ = SkeletalMesh->Bounds.Origin.Z - SkeletalMesh->Bounds.BoxExtent.Z;
                    const float VisibleGap = VisibleBottomZ - FMath::Max(SurfaceZ, MinGroundZ);
                    if (FMath::Abs(VisibleGap) > 14.0f)
                    {
                        ++VisibleFootFailures;
                        UE_LOG(LogTemp, Warning,
                            TEXT("[CharacterGroundingAudit] visual_feet_misaligned actor=%s visible_bottom_z=%.1f surface_z=%.1f gap=%.1f"),
                            *CharacterActor->GetName(), VisibleBottomZ, SurfaceZ, VisibleGap);
                    }
                }
            }
        }
    }

    const bool bGroundingPass = FloatingAfter == 0 && VisibleFootFailures == 0;
    if (bGroundingPass && CityIndex == 0)
    {
        Tags.AddUnique(FName("FirstLevelIntegratedCharacterGroundPass"));
        Tags.AddUnique(FName("FirstLevelIntegratedVisibleFootGroundPass"));
    }
    if (bGroundingPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("[CharacterGroundingAudit] COMPLETE PASS city=%d label=%s characters=%d regrounded=%d floating_after=0 visible_feet=%d/%d min_ground_z=%.1f"),
            CityIndex, *CityLabel, Checked, Regrounded, VisibleFootChecks, VisibleFootChecks, MinGroundZ);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[CharacterGroundingAudit] COMPLETE FAIL city=%d label=%s characters=%d regrounded=%d floating_after=%d visible_foot_failures=%d/%d min_ground_z=%.1f"),
            CityIndex, *CityLabel, Checked, Regrounded, FloatingAfter, VisibleFootFailures, VisibleFootChecks, MinGroundZ);
    }
}

// 2026-07-07 forensic helper: name what is physically around the player.
// Logs class, mesh asset, bounds size and distance for everything within
// Radius (default 900uu). Built to identify the recurring "cyan lattice dome"
// that swallowed the camera at the safehouse.
static void CodeRescueDumpNearbyActors(const TArray<FString>& Args, UWorld* World)
{
    if (!World)
    {
        return;
    }
    const float Radius = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 900.0f;
    APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!Player)
    {
        return;
    }
    const FVector Here = Player->GetActorLocation();
    int32 Count = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor) || Actor == Player)
        {
            continue;
        }
        const float Dist = FVector::Dist(Actor->GetActorLocation(), Here);
        if (Dist > Radius)
        {
            continue;
        }
        FVector BOrigin, BExtent;
        Actor->GetActorBounds(false, BOrigin, BExtent);
        FString MeshName = TEXT("-");
        if (const AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Actor))
        {
            if (SMA->GetStaticMeshComponent() && SMA->GetStaticMeshComponent()->GetStaticMesh())
            {
                MeshName = SMA->GetStaticMeshComponent()->GetStaticMesh()->GetName();
            }
        }
        FString TagList;
        for (const FName& Tag : Actor->Tags)
        {
            TagList += Tag.ToString() + TEXT(",");
        }
        UE_LOG(LogTemp, Warning, TEXT("[NearbyDump] %4.0fuu %s cls=%s mesh=%s extent=(%.0f,%.0f,%.0f) tags=[%s]"),
            Dist, *Actor->GetName(), *Actor->GetClass()->GetName(), *MeshName,
            BExtent.X, BExtent.Y, BExtent.Z, *TagList);
        ++Count;
    }
    UE_LOG(LogTemp, Warning, TEXT("[NearbyDump] %d actors within %.0fuu"), Count, Radius);
}

static FAutoConsoleCommandWithWorldAndArgs GCodeRescueDumpNearbyActors(
    TEXT("cr.DumpNearbyActors"),
    TEXT("Log class/mesh/bounds of every actor within [radius] of the player (default 900)."),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&CodeRescueDumpNearbyActors));
