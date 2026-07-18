#pragma once

#include "CoreMinimal.h"

class AActor;
class USkeletalMeshComponent;

enum class ECodeRescueAnimationBudgetProfile : uint8
{
    PlayerBody,
    FirstPersonArms,
    HeroNPC,
    CrowdZombie
};

namespace CodeRescueAnimationBudget
{
    void ApplySkeletalMeshBudget(
        USkeletalMeshComponent* Mesh,
        ECodeRescueAnimationBudgetProfile Profile,
        AActor* OwnerForTags);
}
