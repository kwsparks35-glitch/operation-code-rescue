#include "CodeRescuePhysicsStability.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"

namespace CodeRescuePhysicsStability
{
void ApplyRuntimeBodyContract(
    UPrimitiveComponent* Component,
    AActor* Owner,
    FName RoleTag,
    float MassKg,
    float MinLinearDamping,
    float MinAngularDamping,
    bool bEnableCCD)
{
    if (!Component)
    {
        return;
    }
    if ((Owner && Owner->HasAnyFlags(RF_ClassDefaultObject)) ||
        (Component->GetOwner() && Component->GetOwner()->HasAnyFlags(RF_ClassDefaultObject)))
    {
        return;
    }

    Component->SetLinearDamping(FMath::Max(Component->GetLinearDamping(), MinLinearDamping));
    Component->SetAngularDamping(FMath::Max(Component->GetAngularDamping(), MinAngularDamping));
    Component->SetMaxDepenetrationVelocity(NAME_None, DefaultMaxDepenetrationVelocity);
    Component->SetUseCCD(bEnableCCD, NAME_None);

    FBodyInstance& BodyInstance = Component->BodyInstance;
    BodyInstance.SleepFamily = ESleepFamily::Custom;
    BodyInstance.CustomSleepThresholdMultiplier = DefaultSleepThresholdMultiplier;
    BodyInstance.StabilizationThresholdMultiplier = DefaultStabilizationMultiplier;

    if (MassKg > 0.0f)
    {
        Component->SetMassOverrideInKg(NAME_None, MassKg, true);
    }

    Component->ComponentTags.AddUnique(FName("FixedStepPhysicsBody"));
    Component->ComponentTags.AddUnique(FName("ChaosSubstepStability"));
    Component->ComponentTags.AddUnique(FName("PhysicsDeterminismReview"));
    if (RoleTag != NAME_None)
    {
        Component->ComponentTags.AddUnique(RoleTag);
    }
    if (bEnableCCD)
    {
        Component->ComponentTags.AddUnique(FName("ContinuousCollisionDetectionEnabled"));
    }

    if (Owner)
    {
        Owner->Tags.AddUnique(FName("FixedStepPhysicsContract"));
        Owner->Tags.AddUnique(FName("SubsteppedPhysicsRuntime"));
        Owner->Tags.AddUnique(FName("Top50Recommendation25"));
        Owner->Tags.AddUnique(FName("GamePhysicsDeepDive"));
        if (RoleTag != NAME_None)
        {
            Owner->Tags.AddUnique(RoleTag);
        }
    }
}

FString GetRuntimeContractSummary()
{
    return FString::Printf(
        TEXT("sync substepping %.6fs, max physics delta %.6fs, max substeps %d, async fixed-step review %.6fs"),
        FixedStepSeconds,
        MaxPhysicsDeltaSeconds,
        MaxSubstepCount,
        FixedStepSeconds);
}
}
