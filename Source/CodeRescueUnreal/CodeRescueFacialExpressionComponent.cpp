// CodeRescueFacialExpressionComponent.cpp -- see header. 2026-07-04 pass.

#include "CodeRescueFacialExpressionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UCodeRescueFacialExpressionComponent::UCodeRescueFacialExpressionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f;
}

void UCodeRescueFacialExpressionComponent::BeginPlay()
{
    Super::BeginPlay();
    RefreshFaces();
    NextBlinkIn = FMath::FRandRange(2.0f, 5.0f);
}

void UCodeRescueFacialExpressionComponent::RefreshFaces()
{
    Faces.Reset();
    if (AActor* Owner = GetOwner())
    {
        TArray<USkeletalMeshComponent*> Comps;
        Owner->GetComponents<USkeletalMeshComponent>(Comps);
        for (USkeletalMeshComponent* C : Comps)
        {
            if (C && C->GetSkeletalMeshAsset())
            {
                Faces.Add(C);
            }
        }
    }
}

void UCodeRescueFacialExpressionComponent::ApplyMorph(FName Morph, float Weight)
{
    for (USkeletalMeshComponent* C : Faces)
    {
        if (IsValid(C))
        {
            // SetMorphTarget is a silent no-op when the mesh lacks the morph -> graceful
            // degradation on mannequin / content-pack meshes.
            C->SetMorphTarget(Morph, Weight);
        }
    }
}

void UCodeRescueFacialExpressionComponent::SetExpression(FName MorphName, float Weight, float HoldSeconds)
{
    if (MorphName.IsNone())
    {
        return;
    }
    if (Faces.Num() == 0)
    {
        RefreshFaces();
    }
    // Replace an in-flight instance of the same morph so repeated hits refresh the hold.
    for (FActiveExpression& E : Active)
    {
        if (E.Morph == MorphName)
        {
            E.Weight = Weight;
            E.HoldSeconds = HoldSeconds;
            E.Elapsed = 0.0f;
            return;
        }
    }
    FActiveExpression E;
    E.Morph = MorphName;
    E.Weight = Weight;
    E.HoldSeconds = HoldSeconds;
    Active.Add(E);
}

void UCodeRescueFacialExpressionComponent::TriggerOnActor(AActor* Actor, FName MorphName, float Weight, float HoldSeconds)
{
    if (!IsValid(Actor))
    {
        return;
    }
    if (UCodeRescueFacialExpressionComponent* Face =
            Actor->FindComponentByClass<UCodeRescueFacialExpressionComponent>())
    {
        Face->SetExpression(MorphName, Weight, HoldSeconds);
    }
}

void UCodeRescueFacialExpressionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Expression envelopes: hold at Weight, then fade to zero.
    for (int32 i = Active.Num() - 1; i >= 0; --i)
    {
        FActiveExpression& E = Active[i];
        E.Elapsed += DeltaTime;
        float W = E.Weight;
        if (E.Elapsed > E.HoldSeconds)
        {
            const float FadeT = (E.Elapsed - E.HoldSeconds) / FMath::Max(0.05f, E.FadeSeconds);
            W = E.Weight * FMath::Clamp(1.0f - FadeT, 0.0f, 1.0f);
        }
        ApplyMorph(E.Morph, W);
        if (E.Elapsed > E.HoldSeconds + E.FadeSeconds)
        {
            ApplyMorph(E.Morph, 0.0f);
            Active.RemoveAt(i);
        }
    }

    // Autonomous blink: quick down-up envelope on the Blink morph.
    if (bAutoBlink)
    {
        if (BlinkPhase >= 0.0f)
        {
            BlinkPhase += DeltaTime;
            const float T = BlinkPhase / BlinkDuration;
            const float W = (T >= 1.0f) ? 0.0f : FMath::Sin(FMath::Clamp(T, 0.0f, 1.0f) * PI);
            ApplyMorph(FName("Blink"), W);
            if (T >= 1.0f)
            {
                BlinkPhase = -1.0f;
                NextBlinkIn = FMath::FRandRange(2.2f, 5.5f);
            }
        }
        else
        {
            NextBlinkIn -= DeltaTime;
            if (NextBlinkIn <= 0.0f)
            {
                BlinkPhase = 0.0f;
            }
        }
    }
}
