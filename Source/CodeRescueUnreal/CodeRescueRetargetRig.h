#pragma once

#include "CoreMinimal.h"

class AActor;
class USkeletalMeshComponent;

enum class ECodeRescueRetargetRigProfile : uint8
{
    PlayerOperator,
    FirstPersonArms,
    SurvivorHero,
    FriendlyNPC,
    CompanionHero,
    ZombieCrowd,
    BossWarden
};

namespace CodeRescueRetargetRig
{
    void ApplyRuntimeRetargetRigSlots(
        USkeletalMeshComponent* Mesh,
        ECodeRescueRetargetRigProfile Profile,
        AActor* OwnerForTags);

    void ApplyFootGroundingReview(
        USkeletalMeshComponent* Mesh,
        ECodeRescueRetargetRigProfile Profile,
        AActor* OwnerForTags);
}
