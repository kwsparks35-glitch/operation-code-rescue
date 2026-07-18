#pragma once

#include "CoreMinimal.h"

class AActor;
class UPrimitiveComponent;

namespace CodeRescuePhysicsStability
{
inline constexpr float FixedStepSeconds = 1.0f / 60.0f;
inline constexpr float MaxPhysicsDeltaSeconds = 1.0f / 30.0f;
inline constexpr int32 MaxSubstepCount = 6;
inline constexpr float DefaultSleepThresholdMultiplier = 1.35f;
inline constexpr float DefaultStabilizationMultiplier = 1.25f;
inline constexpr float DefaultMaxDepenetrationVelocity = 900.0f;

void ApplyRuntimeBodyContract(
    UPrimitiveComponent* Component,
    AActor* Owner,
    FName RoleTag,
    float MassKg = -1.0f,
    float MinLinearDamping = 0.30f,
    float MinAngularDamping = 0.42f,
    bool bEnableCCD = false);

FString GetRuntimeContractSummary();
}
