#include "CodeRescueRetargetRig.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
FName ProfileTag(ECodeRescueRetargetRigProfile Profile)
{
    switch (Profile)
    {
    case ECodeRescueRetargetRigProfile::PlayerOperator:
        return FName("RetargetProfile_PlayerOperator");
    case ECodeRescueRetargetRigProfile::FirstPersonArms:
        return FName("RetargetProfile_FirstPersonArms");
    case ECodeRescueRetargetRigProfile::SurvivorHero:
        return FName("RetargetProfile_SurvivorHero");
    case ECodeRescueRetargetRigProfile::FriendlyNPC:
        return FName("RetargetProfile_FriendlyNPC");
    case ECodeRescueRetargetRigProfile::CompanionHero:
        return FName("RetargetProfile_CompanionHero");
    case ECodeRescueRetargetRigProfile::ZombieCrowd:
        return FName("RetargetProfile_ZombieCrowd");
    case ECodeRescueRetargetRigProfile::BossWarden:
        return FName("RetargetProfile_BossWarden");
    }
    return FName("RetargetProfile_Unknown");
}

void AddSlotTag(USkeletalMeshComponent* Mesh, AActor* OwnerForTags, const FName& Tag)
{
    if (Mesh)
    {
        Mesh->ComponentTags.AddUnique(Tag);
    }
    if (OwnerForTags)
    {
        OwnerForTags->Tags.AddUnique(Tag);
    }
}
}

namespace CodeRescueRetargetRig
{
void ApplyFootGroundingReview(
    USkeletalMeshComponent* Mesh,
    ECodeRescueRetargetRigProfile Profile,
    AActor* OwnerForTags)
{
    if (!Mesh)
    {
        return;
    }

    if (Profile == ECodeRescueRetargetRigProfile::FirstPersonArms)
    {
        AddSlotTag(Mesh, OwnerForTags, FName("FootGroundingExcluded_FirstPersonArms"));
        return;
    }

    AddSlotTag(Mesh, OwnerForTags, FName("FootGroundingRuntimeContract"));
    AddSlotTag(Mesh, OwnerForTags, FName("FootIKGroundingReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("FootPlantTraceReady"));
    AddSlotTag(Mesh, OwnerForTags, FName("PelvisOffsetReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFootContactReady"));
    AddSlotTag(Mesh, OwnerForTags, FName("RetargetFootContactPoseReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("CharacterAnimationDeepDive"));

    switch (Profile)
    {
    case ECodeRescueRetargetRigProfile::PlayerOperator:
        AddSlotTag(Mesh, OwnerForTags, FName("PlayerFootGroundingReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("MannyFootChainReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("WeaponStanceFootPlantReview"));
        break;
    case ECodeRescueRetargetRigProfile::SurvivorHero:
        AddSlotTag(Mesh, OwnerForTags, FName("SurvivorFootGroundingReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("RescueGestureFootPlantReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("QuinnFootChainReview"));
        break;
    case ECodeRescueRetargetRigProfile::FriendlyNPC:
        AddSlotTag(Mesh, OwnerForTags, FName("FriendlyNPCFootGroundingReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("ServiceGestureFootPlantReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("MannyQuinnFootChainReview"));
        break;
    case ECodeRescueRetargetRigProfile::CompanionHero:
        AddSlotTag(Mesh, OwnerForTags, FName("CompanionFootGroundingReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("FormationLocomotionFootPlantReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("QuinnFootChainReview"));
        break;
    case ECodeRescueRetargetRigProfile::ZombieCrowd:
        AddSlotTag(Mesh, OwnerForTags, FName("ZombieFootGroundingReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("ShambleFootPlantReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("ZombiePackFootChainReview"));
        break;
    case ECodeRescueRetargetRigProfile::BossWarden:
        AddSlotTag(Mesh, OwnerForTags, FName("BossFootGroundingReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("HeavyPhaseFootPlantReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("BossFootChainReview"));
        break;
    case ECodeRescueRetargetRigProfile::FirstPersonArms:
        break;
    }
}

void ApplyRuntimeRetargetRigSlots(
    USkeletalMeshComponent* Mesh,
    ECodeRescueRetargetRigProfile Profile,
    AActor* OwnerForTags)
{
    if (!Mesh)
    {
        return;
    }

    AddSlotTag(Mesh, OwnerForTags, FName("IKRetargetRuntimeSlot"));
    AddSlotTag(Mesh, OwnerForTags, FName("ControlRigRuntimeSlot"));
    AddSlotTag(Mesh, OwnerForTags, FName("RetargetControlRigRuntimeContract"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaCharacterCleanupTarget"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaCharacterCleanupRuntimeContract"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaFbxExportReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaSkeletonNamingReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaBindPoseOriginReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaAnimationTakeCleanupReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaSocketAuthoringReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaLODMaterialBudgetReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("MayaPhysicsAssetReview"));
    AddSlotTag(Mesh, OwnerForTags, FName("CharacterAnimationDeepDive"));
    AddSlotTag(Mesh, OwnerForTags, ProfileTag(Profile));

    switch (Profile)
    {
    case ECodeRescueRetargetRigProfile::PlayerOperator:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_UE5Manny"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFullBodySlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("WeaponSocketRetargetReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("CameraPerspectiveRetargetReview"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaWeaponSocketCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaCameraSocketCleanup"));
        break;
    case ECodeRescueRetargetRigProfile::FirstPersonArms:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_UE5Manny"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigArmsSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("FirstPersonArmsRetargetSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("WeaponIKSocketSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaFirstPersonArmsCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaGripPoseCleanup"));
        break;
    case ECodeRescueRetargetRigProfile::SurvivorHero:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_UE5Quinn"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFullBodySlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFacialSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("FootIKReviewSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MetaHumanPromotionSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaFacialRigCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaWardrobeSocketCleanup"));
        break;
    case ECodeRescueRetargetRigProfile::FriendlyNPC:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_UE5MannyQuinn"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFullBodySlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFacialSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("RoleGestureRetargetSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaRoleGestureCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaWardrobeSocketCleanup"));
        break;
    case ECodeRescueRetargetRigProfile::CompanionHero:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_UE5Quinn"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigFullBodySlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("FormationLocomotionRetargetSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("CompanionOrderGestureSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaFormationLocomotionCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaCommandGestureCleanup"));
        break;
    case ECodeRescueRetargetRigProfile::ZombieCrowd:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_ZombiePack"));
        AddSlotTag(Mesh, OwnerForTags, FName("ZombieLocomotionRetargetSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("ZombieAttackRetargetSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("RagdollPhysicsAssetReviewSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("FootIKReviewSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaZombieAttackCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaRagdollBodyCleanup"));
        break;
    case ECodeRescueRetargetRigProfile::BossWarden:
        AddSlotTag(Mesh, OwnerForTags, FName("RetargetSource_ZombiePack"));
        AddSlotTag(Mesh, OwnerForTags, FName("ControlRigBossRevealSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("BossPhaseMontageSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("BossHitReactionRetargetSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("RagdollPhysicsAssetReviewSlot"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaBossMontageCleanup"));
        AddSlotTag(Mesh, OwnerForTags, FName("MayaCinematicRevealCleanup"));
        break;
    }

    ApplyFootGroundingReview(Mesh, Profile, OwnerForTags);
}
}
