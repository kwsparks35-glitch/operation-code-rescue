#include "FriendlyNPCActor.h"
#include "CodeRescueAnimationBudget.h"
#include "CodeRescueFacialExpressionComponent.h"
#include "CodeRescueGameMode.h"
#include "Animation/AnimSequence.h"
#include "CodeRescueCollisionChannels.h"
#include "CodeRescueRetargetRig.h"
#include "CodeRescueCharacter.h"
#include "CodeRescueGameInstance.h"
#include "CodeRescueSubtitlesWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
FLinearColor RoleTint(EFriendlyNPCRole Role)
{
    switch (Role)
    {
    case EFriendlyNPCRole::Engineer:  return FLinearColor(1.0f, 0.6f, 0.1f);  // amber
    case EFriendlyNPCRole::Medic:     return FLinearColor(1.0f, 0.2f, 0.2f);  // red cross
    case EFriendlyNPCRole::Scientist: return FLinearColor(0.4f, 0.6f, 1.0f);  // lab blue
    case EFriendlyNPCRole::Trader:    return FLinearColor(0.2f, 1.0f, 0.4f);  // ledger green
    }
    return FLinearColor::White;
}

FString RoleName(EFriendlyNPCRole Role)
{
    switch (Role)
    {
    case EFriendlyNPCRole::Engineer:  return TEXT("Engineer");
    case EFriendlyNPCRole::Medic:     return TEXT("Medic");
    case EFriendlyNPCRole::Scientist: return TEXT("Scientist");
    case EFriendlyNPCRole::Trader:    return TEXT("Trader");
    }
    return TEXT("Civilian");
}

FString RoleKey(EFriendlyNPCRole Role)
{
    switch (Role)
    {
    case EFriendlyNPCRole::Engineer:  return TEXT("Engineer");
    case EFriendlyNPCRole::Medic:     return TEXT("Medic");
    case EFriendlyNPCRole::Scientist: return TEXT("Scientist");
    case EFriendlyNPCRole::Trader:    return TEXT("Trader");
    }
    return TEXT("Civilian");
}

FString RoleServiceBenefit(EFriendlyNPCRole Role)
{
    switch (Role)
    {
    case EFriendlyNPCRole::Engineer:
        return TEXT("+1 scrap for repairs");
    case EFriendlyNPCRole::Medic:
        return TEXT("+25 health when injured");
    case EFriendlyNPCRole::Scientist:
        return TEXT("+1 research point");
    case EFriendlyNPCRole::Trader:
        return TEXT("trade 5 scrap for +1 research");
    }
    return TEXT("field support");
}

FString RoleWorldNote(EFriendlyNPCRole Role)
{
    switch (Role)
    {
    case EFriendlyNPCRole::Engineer:
        return TEXT("I keep barricades, generators, and terminal housings alive.");
    case EFriendlyNPCRole::Medic:
        return TEXT("I triage the injured while your code opens the safe routes.");
    case EFriendlyNPCRole::Scientist:
        return TEXT("I turn each solved test case into field research.");
    case EFriendlyNPCRole::Trader:
        return TEXT("I barter parts between shelters so the lessons keep moving.");
    }
    return TEXT("Every person here has a job in the rescue chain.");
}
}

AFriendlyNPCActor::AFriendlyNPCActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("NPCRoot"));
    RootComponent = Root;

    // Procedural body — same silhouette pattern as ASurvivorActor so the
    // player reads NPCs and survivors as the same kind of figure visually.
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveBody"));
    Body->SetupAttachment(Root);
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Body->SetCollisionResponseToChannel(CodeRescueCollision::InteractionTrace, ECR_Block);
    Body->ComponentTags.AddUnique(FName("CollisionChannel_InteractionTraceTarget"));
    Body->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.40f));

    Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrimitiveHead"));
    Head->SetupAttachment(Body);
    Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Head->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
    Head->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.55f));

    RoleBadge = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoleBadge"));
    RoleBadge->SetupAttachment(Root);
    RoleBadge->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoleBadge->SetRelativeLocation(FVector(38.0f, -22.0f, 118.0f));
    RoleBadge->SetRelativeScale3D(FVector(0.18f, 0.06f, 0.18f));

    RoleProp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoleProp"));
    RoleProp->SetupAttachment(Root);
    RoleProp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoleProp->SetRelativeLocation(FVector(62.0f, -48.0f, 62.0f));
    RoleProp->SetRelativeScale3D(FVector(0.38f, 0.20f, 0.22f));

    RoleIconA = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoleIconA"));
    RoleIconA->SetupAttachment(Root);
    RoleIconA->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoleIconA->SetRelativeLocation(FVector(42.0f, -28.0f, 150.0f));
    RoleIconA->SetRelativeScale3D(FVector(0.28f, 0.05f, 0.05f));

    RoleIconB = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoleIconB"));
    RoleIconB->SetupAttachment(Root);
    RoleIconB->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoleIconB->SetRelativeLocation(FVector(42.0f, -28.0f, 150.0f));
    RoleIconB->SetRelativeScale3D(FVector(0.05f, 0.05f, 0.28f));

    SkeletalBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalNPCBody"));
    SkeletalBody->SetupAttachment(Root);
    SkeletalBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletalBody->SetVisibility(false);
    SkeletalBody->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    SkeletalBody->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    CodeRescueAnimationBudget::ApplySkeletalMeshBudget(
        SkeletalBody, ECodeRescueAnimationBudgetProfile::HeroNPC, this);
    CodeRescueRetargetRig::ApplyRuntimeRetargetRigSlots(
        SkeletalBody, ECodeRescueRetargetRigProfile::FriendlyNPC, this);

    RoleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RoleLight"));
    RoleLight->SetupAttachment(Root);
    RoleLight->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    // Warm, wider safe-hub glow so the friendly-NPC plaza reads as a
    // welcoming refuge that draws the player in, day or night.
    RoleLight->SetIntensity(3200.0f);
    RoleLight->SetLightColor(FLinearColor(1.0f, 0.92f, 0.72f));
    RoleLight->SetAttenuationRadius(950.0f);

    // Static mesh defaults — same Engine cube/sphere lookup as the survivor
    // class so we don't introduce a new asset dependency.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CubeFinder.Succeeded())
    {
        Body->SetStaticMesh(CubeFinder.Object);
        RoleBadge->SetStaticMesh(CubeFinder.Object);
        RoleProp->SetStaticMesh(CubeFinder.Object);
        RoleIconA->SetStaticMesh(CubeFinder.Object);
        RoleIconB->SetStaticMesh(CubeFinder.Object);
    }
    if (SphereFinder.Succeeded())
    {
        Head->SetStaticMesh(SphereFinder.Object);
    }
    if (CylinderFinder.Succeeded())
    {
        RoleBadge->SetStaticMesh(CylinderFinder.Object);
    }
}

void AFriendlyNPCActor::BeginPlay()
{
    Super::BeginPlay();

    // 2026-07-07 elevation fix / upgraded 2026-07-11: NPCs stand ON the
    // layered street surfaces (shared robust snap; see CodeRescueGameMode).
    ACodeRescueGameMode::SnapCharacterBaseToGround(this);

    const FLinearColor Tint = RoleTint(NPCRole);
    if (RoleLight)
    {
        RoleLight->SetLightColor(Tint);
    }
    // Apply a dynamic-material tint to the procedural body so each role is
    // identifiable at distance even before authored meshes drop in.
    if (Body && Body->GetStaticMesh())
    {
        if (UMaterialInterface* Mat = Body->GetMaterial(0))
        {
            if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Mat, this))
            {
                MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
                Body->SetMaterial(0, MID);
            }
        }
    }

    // Production NPCs use the complete Manny/Quinn rigs. Prototype character
    // studies remain available through an explicit review command-line flag.
    const bool bFemaleRole =
        NPCRole == EFriendlyNPCRole::Medic ||
        NPCRole == EFriendlyNPCRole::Scientist;
    UAnimSequence* V2NPCIdle = nullptr;
    bool bV2NPCBody = false;
    const bool bUsePrototypeCharacter =
        FParse::Param(FCommandLine::Get(), TEXT("CodeRescueUsePrototypeCharacters"));
    if (!ProfessionalNPCMesh && !bUsePrototypeCharacter)
    {
        // 2026-07-11 v3: authored survivors (morph faces, gear) are the default
        // street NPCs now; grey mannequins only remain as the missing-asset
        // fallback below.
        const TCHAR* V3Name = bFemaleRole ? TEXT("SurvivorMayaV3") : TEXT("SurvivorKennyV3");
        const FString V3MeshPath = FString::Printf(
            TEXT("/Game/CodeRescueArt/CharactersV3/%s/%s.%s"), V3Name, V3Name, V3Name);
        ProfessionalNPCMesh = LoadObject<USkeletalMesh>(nullptr, *V3MeshPath);
        if (ProfessionalNPCMesh)
        {
            const FString IdleCandidates[3] = {
                FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/%s/%s_Anim_%s_Idle.%s_Anim_%s_Idle"),
                    V3Name, V3Name, V3Name, V3Name, V3Name),
                FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/%s/%s_Idle.%s_Idle"),
                    V3Name, V3Name, V3Name),
                FString::Printf(TEXT("/Game/CodeRescueArt/CharactersV3/%s/%s%s_Idle.%s%s_Idle"),
                    V3Name, V3Name, V3Name, V3Name, V3Name)};
            for (const FString& Path : IdleCandidates)
            {
                V2NPCIdle = LoadObject<UAnimSequence>(nullptr, *Path);
                if (V2NPCIdle)
                {
                    break;
                }
            }
            bV2NPCBody = true;
            UE_LOG(LogTemp, Display, TEXT("[FriendlyNPCV3] %s wears %s (idle=%d)"),
                *GetName(), V3Name, V2NPCIdle != nullptr);
        }
        else
        {
            ProfessionalNPCMesh = LoadObject<USkeletalMesh>(
                nullptr,
                bFemaleRole
                    ? TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn")
                    : TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
        }
    }
    if (!ProfessionalNPCMesh && bUsePrototypeCharacter)
    {
        const TCHAR* V2Mesh = bFemaleRole
            ? TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorMaya.SurvivorMaya")
            : TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorKenny.SurvivorKenny");
        const TCHAR* V2Idle = bFemaleRole
            ? TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorMayaSurvivorMaya_Idle.SurvivorMayaSurvivorMaya_Idle")
            : TEXT("/Game/CodeRescueArt/CharactersV2/SurvivorKennySurvivorKenny_Idle.SurvivorKennySurvivorKenny_Idle");
        ProfessionalNPCMesh = LoadObject<USkeletalMesh>(nullptr, V2Mesh);
        if (ProfessionalNPCMesh)
        {
            V2NPCIdle = LoadObject<UAnimSequence>(nullptr, V2Idle);
            bV2NPCBody = true;
        }
    }
    if (!ProfessionalNPCMesh)
    {
        ProfessionalNPCMesh = LoadObject<USkeletalMesh>(
            nullptr,
            bFemaleRole
                ? TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn")
                : TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
    }

    if (!ProfessionalNPCAnimClass && !bV2NPCBody)
    {
        ProfessionalNPCAnimClass = LoadClass<UAnimInstance>(
            nullptr,
            bFemaleRole
                ? TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C")
                : TEXT("/Game/YI_ModularZombies/Demo/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C"));
    }

    // Swap to the professional skeletal mesh. If local mannequin assets are
    // missing, the procedural body remains as a readable fallback.
    if (ProfessionalNPCMesh && SkeletalBody)
    {
        SkeletalBody->SetSkeletalMesh(ProfessionalNPCMesh);
        if (bV2NPCBody)
        {
            SkeletalBody->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            if (V2NPCIdle)
            {
                SkeletalBody->PlayAnimation(V2NPCIdle, true);
            }
            // Facial life: autonomous blinking via the morph driver (no-op without morphs).
            if (UCodeRescueFacialExpressionComponent* Face =
                    NewObject<UCodeRescueFacialExpressionComponent>(this, TEXT("FacialExpression")))
            {
                Face->RegisterComponent();
            }
        }
        else if (ProfessionalNPCAnimClass)
        {
            SkeletalBody->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            SkeletalBody->SetAnimInstanceClass(ProfessionalNPCAnimClass);
        }
        UE_LOG(LogTemp, Display, TEXT("[CharacterPresentation] %s NPC uses %s rig."),
            *RoleName(NPCRole), bV2NPCBody ? TEXT("prototype character") : TEXT("production mannequin"));
        SkeletalBody->SetVisibility(true);
        Body->SetVisibility(false);
        Head->SetVisibility(false);
    }

    ApplyRoleVisualIdentity();
    CacheServiceGestureBasePose(true);
    Tags.AddUnique(FName("FriendlySafehouseNPCService"));
    Tags.AddUnique(FName("SelectedLanguageSupportSave"));
    Tags.AddUnique(FName("SafehouseNPCServiceLoop"));
    Tags.AddUnique(FName(*FString::Printf(TEXT("NPCService_%s"), *RoleKey(NPCRole))));
    ApplySavedServiceState();
}

void AFriendlyNPCActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateServiceGesture(DeltaSeconds);
}

FString AFriendlyNPCActor::GetServiceId() const
{
    return FString::Printf(TEXT("FriendlyNPC_City%03d_%s"), CityIndex, *RoleKey(NPCRole));
}

FString AFriendlyNPCActor::GetRoleDisplayName() const
{
    return RoleName(NPCRole);
}

FString AFriendlyNPCActor::GetServiceSummary() const
{
    return FString::Printf(
        TEXT("%s: %s once per day-cycle"),
        *RoleName(NPCRole),
        *RoleServiceBenefit(NPCRole));
}

bool AFriendlyNPCActor::IsServiceOnCooldown() const
{
    if (bPerkUsedThisDay)
    {
        return true;
    }

    if (UWorld* World = GetWorld())
    {
        if (const UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>())
        {
            return GI->HasFriendlyNPCServiceCooldown(GetServiceId());
        }
    }
    return false;
}

FString AFriendlyNPCActor::GetInteractionPrompt() const
{
    if (IsServiceOnCooldown())
    {
        return FString::Printf(
            TEXT("[E] %s used - resets next day/night shift"),
            *RoleName(NPCRole));
    }

    return FString::Printf(
        TEXT("[E] %s"),
        *GetServiceSummary());
}

void AFriendlyNPCActor::ApplySavedServiceState()
{
    if (UWorld* World = GetWorld())
    {
        if (const UCodeRescueGameInstance* GI = World->GetGameInstance<UCodeRescueGameInstance>())
        {
            bPerkUsedThisDay = GI->HasFriendlyNPCServiceCooldown(GetServiceId());
        }
    }
}

void AFriendlyNPCActor::ResetDailyPerk()
{
    bPerkUsedThisDay = false;
    Tags.AddUnique(FName("SafehouseNPCDailyRefreshReady"));
}

bool AFriendlyNPCActor::Interact(APawn* PlayerPawn)
{
    ACodeRescueCharacter* Player = Cast<ACodeRescueCharacter>(PlayerPawn);
    if (!Player)
    {
        return false;
    }

    UCodeRescueGameInstance* GI = Player->GetGameInstance<UCodeRescueGameInstance>();
    ApplySavedServiceState();

    if (bPerkUsedThisDay)
    {
        TriggerServiceDeniedGesture();
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(TEXT("[%s - %s]: Come back after the day-night shift. This %s service is saved in the %s profile."),
                            *NPCName,
                            *RoleName(NPCRole),
                            *RoleName(NPCRole),
                            GI ? *GI->GetLanguageName() : TEXT("current language")),
            3.5f);
        return false;
    }

    // Always speak the greeting first so the player knows the interaction landed.
    UCodeRescueSubtitlesWidget::Push(
        FString::Printf(TEXT("[%s - %s]: %s %s"),
                        *NPCName, *RoleName(NPCRole), *GreetingLine, *RoleWorldNote(NPCRole)),
        4.0f);

    bool bGranted = false;
    switch (NPCRole)
    {
    case EFriendlyNPCRole::Engineer:
        {
            const int32 NewScrap = Player->GrantScrap(1);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                    FString::Printf(TEXT("Engineer gave you scrap. Total: %d"), NewScrap));
            }
            bGranted = true;
            break;
        }
    case EFriendlyNPCRole::Medic:
        {
            if (Player->Health < Player->MaxHealth)
            {
                Player->Health = FMath::Min(Player->MaxHealth, Player->Health + 25.0f);
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
                        FString::Printf(TEXT("Medic patched you up. Health: %.0f / %.0f"),
                                        Player->Health, Player->MaxHealth));
                }
                bGranted = true;
            }
            else
            {
                TriggerServiceDeniedGesture();
                UCodeRescueSubtitlesWidget::Push(
                    FString::Printf(TEXT("[%s]: You're not hurt. Save it for someone who is."), *NPCName),
                    3.0f);
                // Don't burn the cooldown when there's nothing to heal.
                return false;
            }
            break;
        }
    case EFriendlyNPCRole::Scientist:
        {
            if (GI)
            {
                GI->ResearchPoints += 1;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
                        FString::Printf(TEXT("Scientist shared a study. RP: %d"),
                                        GI->ResearchPoints));
                }
                bGranted = true;
            }
            break;
        }
    case EFriendlyNPCRole::Trader:
        {
            if (GI && Player->GetScrap() >= 5 && Player->TrySpendScrap(5))
            {
                GI->ResearchPoints += 1;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                        FString::Printf(TEXT("Trader: -5 scrap, +1 RP. (Scrap %d / RP %d)"),
                                        Player->GetScrap(), GI->ResearchPoints));
                }
                bGranted = true;
            }
            else
            {
                TriggerServiceDeniedGesture();
                UCodeRescueSubtitlesWidget::Push(
                    FString::Printf(TEXT("[%s]: Bring me five scrap and we'll talk."), *NPCName),
                    3.0f);
                return false;   // don't burn the cooldown if we couldn't trade
            }
            break;
        }
    }

    if (bGranted)
    {
        TriggerServiceGrantGesture();
        bPerkUsedThisDay = true;
        if (GI)
        {
            GI->MarkFriendlyNPCServiceUsed(GetServiceId());
        }
        UCodeRescueSubtitlesWidget::Push(
            FString::Printf(TEXT("Support saved to %s profile: %s"),
                            GI ? *GI->GetLanguageName() : TEXT("current language"),
                            *GetServiceSummary()),
            3.0f);
    }
    return bGranted;
}

void AFriendlyNPCActor::CacheServiceGestureBasePose(bool bTagForAudit)
{
    if (bServiceGestureBasePoseCached && !bTagForAudit)
    {
        return;
    }

    if (SkeletalBody)
    {
        SkeletalGestureBaseTransform = SkeletalBody->GetRelativeTransform();
        SkeletalBody->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
        SkeletalBody->ComponentTags.AddUnique(FName("SafehouseNPCProfessionalGestureTarget"));
    }
    if (Head)
    {
        HeadGestureBaseTransform = Head->GetRelativeTransform();
        Head->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
    }
    if (RoleBadge)
    {
        RoleBadgeGestureBaseTransform = RoleBadge->GetRelativeTransform();
        RoleBadge->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
    }
    if (RoleProp)
    {
        RolePropGestureBaseTransform = RoleProp->GetRelativeTransform();
        RoleProp->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
    }
    if (RoleIconA)
    {
        RoleIconAGestureBaseTransform = RoleIconA->GetRelativeTransform();
        RoleIconA->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
    }
    if (RoleIconB)
    {
        RoleIconBGestureBaseTransform = RoleIconB->GetRelativeTransform();
        RoleIconB->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
    }
    if (RoleLight)
    {
        RoleLightGestureBaseTransform = RoleLight->GetRelativeTransform();
        RoleLightGestureBaseIntensity = RoleLight->Intensity;
        RoleLight->ComponentTags.AddUnique(FName("FriendlyNPCGestureReadabilityComponent"));
        RoleLight->ComponentTags.AddUnique(FName("SafehouseNPCServiceLightPulse"));
    }

    bServiceGestureBasePoseCached = true;

    if (bTagForAudit)
    {
        Tags.AddUnique(FName("FriendlyNPCGestureReadabilityRuntime"));
        Tags.AddUnique(FName("SafehouseNPCIdleServicePose"));
        Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    }
}

void AFriendlyNPCActor::UpdateServiceGesture(float DeltaSeconds)
{
    if (!bServiceGestureBasePoseCached)
    {
        CacheServiceGestureBasePose(true);
    }

    if (!bEnableServiceGestureReadability || !bServiceGestureBasePoseCached)
    {
        return;
    }

    ServiceGesturePhase += DeltaSeconds * (0.9f + 0.08f * static_cast<float>(static_cast<uint8>(NPCRole)));
    ServiceGrantGestureTimer = FMath::Max(0.0f, ServiceGrantGestureTimer - DeltaSeconds);
    ServiceDeniedGestureTimer = FMath::Max(0.0f, ServiceDeniedGestureTimer - DeltaSeconds);

    const float RolePhase = ServiceGesturePhase + 0.72f * static_cast<float>(static_cast<uint8>(NPCRole));
    const float IdleSway = FMath::Sin(RolePhase) * ServiceIdleGestureScale;
    const float IdleLift = FMath::Abs(FMath::Sin(RolePhase * 0.73f)) * ServiceIdleGestureScale;
    const float GrantProgress = ServiceGrantGestureTimer > 0.0f
        ? 1.0f - (ServiceGrantGestureTimer / FMath::Max(0.01f, ServiceGrantGestureDuration))
        : 1.0f;
    const float DeniedProgress = ServiceDeniedGestureTimer > 0.0f
        ? 1.0f - (ServiceDeniedGestureTimer / FMath::Max(0.01f, ServiceDeniedGestureDuration))
        : 1.0f;
    const float GrantPulse = ServiceGrantGestureTimer > 0.0f
        ? FMath::Sin(GrantProgress * PI)
        : 0.0f;
    const float DeniedPulse = ServiceDeniedGestureTimer > 0.0f
        ? FMath::Sin(DeniedProgress * PI)
        : 0.0f;
    const float DeniedShake = ServiceDeniedGestureTimer > 0.0f
        ? FMath::Sin(DeniedProgress * PI * 5.0f)
        : 0.0f;
    const float CooldownDim = bPerkUsedThisDay ? 0.72f : 1.0f;

    if (SkeletalBody)
    {
        FTransform Pose = SkeletalGestureBaseTransform;
        Pose.SetLocation(
            SkeletalGestureBaseTransform.GetLocation() +
            FVector(0.0f, 2.5f * IdleSway, 1.5f * IdleLift + 6.0f * GrantPulse - 2.0f * DeniedPulse));
        const FRotator BaseRot = SkeletalGestureBaseTransform.GetRotation().Rotator();
        Pose.SetRotation((BaseRot + FRotator(-4.0f * GrantPulse, 1.5f * IdleSway + 8.0f * DeniedShake, 1.1f * IdleSway)).Quaternion());
        SkeletalBody->SetRelativeTransform(Pose);
    }

    if (Head)
    {
        FTransform Pose = HeadGestureBaseTransform;
        Pose.SetLocation(
            HeadGestureBaseTransform.GetLocation() +
            FVector(0.0f, 0.0f, 1.5f * IdleLift + 3.0f * GrantPulse - 1.5f * DeniedPulse));
        const FRotator BaseRot = HeadGestureBaseTransform.GetRotation().Rotator();
        Pose.SetRotation((BaseRot + FRotator(-9.0f * GrantPulse, 3.0f * IdleSway + 12.0f * DeniedShake, 0.0f)).Quaternion());
        Head->SetRelativeTransform(Pose);
    }

    if (RoleProp)
    {
        FTransform Pose = RolePropGestureBaseTransform;
        Pose.SetLocation(
            RolePropGestureBaseTransform.GetLocation() +
            FVector(0.0f, 0.0f, 2.0f * IdleLift + 12.0f * GrantPulse - 4.0f * DeniedPulse));
        const FRotator BaseRot = RolePropGestureBaseTransform.GetRotation().Rotator();
        Pose.SetRotation((BaseRot + FRotator(0.0f, 4.0f * DeniedShake, 5.0f * IdleSway + 10.0f * GrantPulse)).Quaternion());
        RoleProp->SetRelativeTransform(Pose);
    }

    const float BadgePulse = 1.0f + 0.025f * IdleLift + 0.16f * GrantPulse - 0.035f * DeniedPulse;
    if (RoleBadge)
    {
        FTransform Pose = RoleBadgeGestureBaseTransform;
        Pose.SetScale3D(RoleBadgeGestureBaseTransform.GetScale3D() * BadgePulse);
        RoleBadge->SetRelativeTransform(Pose);
    }
    if (RoleIconA)
    {
        FTransform Pose = RoleIconAGestureBaseTransform;
        Pose.SetLocation(RoleIconAGestureBaseTransform.GetLocation() + FVector(0.0f, 0.0f, 3.0f * GrantPulse));
        Pose.SetScale3D(RoleIconAGestureBaseTransform.GetScale3D() * (1.0f + 0.08f * GrantPulse - 0.025f * DeniedPulse));
        RoleIconA->SetRelativeTransform(Pose);
    }
    if (RoleIconB)
    {
        FTransform Pose = RoleIconBGestureBaseTransform;
        Pose.SetLocation(RoleIconBGestureBaseTransform.GetLocation() + FVector(0.0f, 0.0f, 3.0f * GrantPulse));
        Pose.SetScale3D(RoleIconBGestureBaseTransform.GetScale3D() * (1.0f + 0.08f * GrantPulse - 0.025f * DeniedPulse));
        RoleIconB->SetRelativeTransform(Pose);
    }

    if (RoleLight)
    {
        FTransform Pose = RoleLightGestureBaseTransform;
        Pose.SetLocation(RoleLightGestureBaseTransform.GetLocation() + FVector(0.0f, 0.0f, 9.0f * GrantPulse));
        RoleLight->SetRelativeTransform(Pose);
        RoleLight->SetIntensity(RoleLightGestureBaseIntensity * FMath::Max(0.35f, CooldownDim + 0.04f * IdleLift + 0.35f * GrantPulse - 0.18f * DeniedPulse));
    }
}

void AFriendlyNPCActor::TriggerServiceGrantGesture()
{
    ServiceGrantGestureTimer = FMath::Max(ServiceGrantGestureTimer, ServiceGrantGestureDuration);
    ServiceDeniedGestureTimer = 0.0f;
    Tags.AddUnique(FName("FriendlyNPCGestureReadabilityRuntime"));
    Tags.AddUnique(FName("SafehouseNPCServiceGrantPose"));
    Tags.AddUnique(FName("SelectedLanguageSupportSaveGesture"));
}

void AFriendlyNPCActor::TriggerServiceDeniedGesture()
{
    ServiceDeniedGestureTimer = FMath::Max(ServiceDeniedGestureTimer, ServiceDeniedGestureDuration);
    Tags.AddUnique(FName("FriendlyNPCGestureReadabilityRuntime"));
    Tags.AddUnique(FName("SafehouseNPCServiceDeniedPose"));
}

void AFriendlyNPCActor::SetComponentTint(UStaticMeshComponent* Component, const FLinearColor& Tint, float EmissiveScale)
{
    if (!Component || !Component->GetStaticMesh())
    {
        return;
    }

    UMaterialInterface* BaseMat = Component->GetMaterial(0);
    if (!BaseMat)
    {
        return;
    }

    if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, this))
    {
        MID->SetVectorParameterValue(TEXT("Color"), Tint);
        MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
        MID->SetVectorParameterValue(TEXT("EmissiveColor"), Tint * EmissiveScale);
        Component->SetMaterial(0, MID);
    }
}

void AFriendlyNPCActor::ApplyRoleVisualIdentity()
{
    const FLinearColor Tint = RoleTint(NPCRole);
    const FLinearColor Dark = Tint * 0.35f;
    const FLinearColor Bright = Tint * 1.8f;

    SetComponentTint(RoleBadge, Bright, 0.6f);
    SetComponentTint(RoleProp, Dark, 0.1f);
    SetComponentTint(RoleIconA, Bright, 0.8f);
    SetComponentTint(RoleIconB, Bright, 0.8f);

    switch (NPCRole)
    {
    case EFriendlyNPCRole::Engineer:
        RoleBadge->SetRelativeLocation(FVector(38.0f, -24.0f, 122.0f));
        RoleBadge->SetRelativeScale3D(FVector(0.16f, 0.045f, 0.16f));
        RoleProp->SetRelativeLocation(FVector(68.0f, -54.0f, 54.0f));
        RoleProp->SetRelativeScale3D(FVector(0.48f, 0.24f, 0.24f));
        RoleIconA->SetRelativeLocation(FVector(42.0f, -31.0f, 154.0f));
        RoleIconA->SetRelativeRotation(FRotator(0.0f, 0.0f, 25.0f));
        RoleIconA->SetRelativeScale3D(FVector(0.34f, 0.05f, 0.06f));
        RoleIconB->SetRelativeLocation(FVector(42.0f, -31.0f, 154.0f));
        RoleIconB->SetRelativeRotation(FRotator(0.0f, 0.0f, -25.0f));
        RoleIconB->SetRelativeScale3D(FVector(0.34f, 0.05f, 0.06f));
        break;

    case EFriendlyNPCRole::Medic:
        RoleBadge->SetRelativeLocation(FVector(38.0f, -24.0f, 122.0f));
        RoleBadge->SetRelativeScale3D(FVector(0.18f, 0.045f, 0.18f));
        RoleProp->SetRelativeLocation(FVector(66.0f, -52.0f, 60.0f));
        RoleProp->SetRelativeScale3D(FVector(0.42f, 0.22f, 0.30f));
        RoleIconA->SetRelativeLocation(FVector(42.0f, -32.0f, 154.0f));
        RoleIconA->SetRelativeRotation(FRotator::ZeroRotator);
        RoleIconA->SetRelativeScale3D(FVector(0.30f, 0.045f, 0.055f));
        RoleIconB->SetRelativeLocation(FVector(42.0f, -32.0f, 154.0f));
        RoleIconB->SetRelativeRotation(FRotator::ZeroRotator);
        RoleIconB->SetRelativeScale3D(FVector(0.055f, 0.045f, 0.30f));
        break;

    case EFriendlyNPCRole::Scientist:
        RoleBadge->SetRelativeLocation(FVector(38.0f, -24.0f, 122.0f));
        RoleBadge->SetRelativeScale3D(FVector(0.14f, 0.045f, 0.22f));
        RoleProp->SetRelativeLocation(FVector(64.0f, -54.0f, 70.0f));
        RoleProp->SetRelativeScale3D(FVector(0.34f, 0.08f, 0.42f));
        RoleIconA->SetRelativeLocation(FVector(44.0f, -32.0f, 156.0f));
        RoleIconA->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
        RoleIconA->SetRelativeScale3D(FVector(0.22f, 0.04f, 0.22f));
        RoleIconB->SetRelativeLocation(FVector(44.0f, -32.0f, 173.0f));
        RoleIconB->SetRelativeRotation(FRotator::ZeroRotator);
        RoleIconB->SetRelativeScale3D(FVector(0.08f, 0.04f, 0.08f));
        break;

    case EFriendlyNPCRole::Trader:
        RoleBadge->SetRelativeLocation(FVector(38.0f, -24.0f, 122.0f));
        RoleBadge->SetRelativeScale3D(FVector(0.20f, 0.045f, 0.20f));
        RoleProp->SetRelativeLocation(FVector(64.0f, -54.0f, 58.0f));
        RoleProp->SetRelativeScale3D(FVector(0.36f, 0.22f, 0.44f));
        RoleIconA->SetRelativeLocation(FVector(42.0f, -31.0f, 154.0f));
        RoleIconA->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
        RoleIconA->SetRelativeScale3D(FVector(0.24f, 0.045f, 0.045f));
        RoleIconB->SetRelativeLocation(FVector(42.0f, -31.0f, 166.0f));
        RoleIconB->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
        RoleIconB->SetRelativeScale3D(FVector(0.24f, 0.045f, 0.045f));
        break;
    }
}
