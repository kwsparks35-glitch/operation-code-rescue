#include "CodeRescueAnimationBudget.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
FName ProfileTag(ECodeRescueAnimationBudgetProfile Profile)
{
    switch (Profile)
    {
    case ECodeRescueAnimationBudgetProfile::PlayerBody:
        return FName("AnimationBudget_PlayerBody");
    case ECodeRescueAnimationBudgetProfile::FirstPersonArms:
        return FName("AnimationBudget_FirstPersonArms");
    case ECodeRescueAnimationBudgetProfile::HeroNPC:
        return FName("AnimationBudget_HeroNPC");
    case ECodeRescueAnimationBudgetProfile::CrowdZombie:
        return FName("AnimationBudget_CrowdZombie");
    }
    return FName("AnimationBudget_Unknown");
}
}

namespace CodeRescueAnimationBudget
{
void ApplySkeletalMeshBudget(
    USkeletalMeshComponent* Mesh,
    ECodeRescueAnimationBudgetProfile Profile,
    AActor* OwnerForTags)
{
    if (!Mesh)
    {
        return;
    }

    Mesh->bComponentUseFixedSkelBounds = true;
    Mesh->SetBoundsScale(1.15f);

    switch (Profile)
    {
    case ECodeRescueAnimationBudgetProfile::PlayerBody:
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        Mesh->bEnableUpdateRateOptimizations = false;
        Mesh->PrimaryComponentTick.TickInterval = 0.0f;
        break;
    case ECodeRescueAnimationBudgetProfile::FirstPersonArms:
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        Mesh->bEnableUpdateRateOptimizations = false;
        Mesh->PrimaryComponentTick.TickInterval = 0.0f;
        Mesh->SetBoundsScale(1.35f);
        break;
    case ECodeRescueAnimationBudgetProfile::HeroNPC:
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
        Mesh->bEnableUpdateRateOptimizations = true;
        Mesh->PrimaryComponentTick.TickInterval = 0.0f;
        break;
    case ECodeRescueAnimationBudgetProfile::CrowdZombie:
        Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
        Mesh->bEnableUpdateRateOptimizations = true;
        Mesh->PrimaryComponentTick.TickInterval = 1.0f / 30.0f;
        break;
    }

    Mesh->ComponentTags.AddUnique(FName("AnimationBudget_Runtime"));
    Mesh->ComponentTags.AddUnique(ProfileTag(Profile));
    Mesh->ComponentTags.AddUnique(FName("CharacterAnimationDeepDive"));

    if (OwnerForTags)
    {
        OwnerForTags->Tags.AddUnique(FName("AnimationBudget_Runtime"));
        OwnerForTags->Tags.AddUnique(ProfileTag(Profile));
        OwnerForTags->Tags.AddUnique(FName("CharacterAnimationDeepDive"));
    }
}
}
