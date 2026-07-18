#include "CodeRescuePhysicsPromotionValidator.h"

#include "Misc/DataValidation.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

#define LOCTEXT_NAMESPACE "CodeRescuePhysicsPromotionValidator"

namespace
{
bool IsGeometryCollectionAsset(const UObject* Asset)
{
    return Asset && Asset->GetClass() && Asset->GetClass()->GetName().Contains(TEXT("GeometryCollection"));
}

bool IsLikelyRuntimeRagdollAsset(const FString& PackageName)
{
    return PackageName.Contains(TEXT("Zombie"))
        && !PackageName.Contains(TEXT("Limb"))
        && !PackageName.Contains(TEXT("Clothing"))
        && !PackageName.Contains(TEXT("Hair"));
}

void ReportPhysicsError(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FText& Message)
{
    Validator->AssetMessage(AssetData, EMessageSeverity::Error, Message);
}

void ReportPhysicsWarning(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FText& Message)
{
    Validator->AssetMessage(AssetData, EMessageSeverity::Warning, Message);
}
}

bool UCodeRescuePhysicsPromotionValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    return InAsset && (InAsset->IsA<UPhysicsAsset>() || IsGeometryCollectionAsset(InAsset));
}

EDataValidationResult UCodeRescuePhysicsPromotionValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
    if (!InAsset)
    {
        return EDataValidationResult::NotValidated;
    }

    if (UPhysicsAsset* PhysicsAsset = Cast<UPhysicsAsset>(InAsset))
    {
        bool bValid = true;
        const FString PackageName = InAssetData.PackageName.ToString();
        const int32 BodyCount = PhysicsAsset->SkeletalBodySetups.Num();
        const int32 ConstraintCount = PhysicsAsset->ConstraintSetup.Num();
        const bool bRuntimeRagdollCandidate = IsLikelyRuntimeRagdollAsset(PackageName);

        if (BodyCount <= 0)
        {
            ReportPhysicsError(
                this,
                InAssetData,
                LOCTEXT("PhysicsAssetNeedsBodies", "Physics Assets promoted for Code Rescue must contain at least one authored rigid body."));
            bValid = false;
        }

        for (int32 Index = 0; Index < PhysicsAsset->SkeletalBodySetups.Num(); ++Index)
        {
            const USkeletalBodySetup* BodySetup = PhysicsAsset->SkeletalBodySetups[Index];
            if (!BodySetup)
            {
                ReportPhysicsError(
                    this,
                    InAssetData,
                    FText::Format(LOCTEXT("PhysicsAssetNullBody", "Physics Asset body setup {0} is null."), FText::AsNumber(Index)));
                bValid = false;
                continue;
            }

            if (BodySetup->AggGeom.GetElementCount() <= 0)
            {
                ReportPhysicsError(
                    this,
                    InAssetData,
                    FText::Format(LOCTEXT("PhysicsAssetNoSimpleCollision", "Physics Asset body setup {0} has no simple collision primitive."), FText::AsNumber(Index)));
                bValid = false;
            }
        }

        if (bRuntimeRagdollCandidate && BodyCount < 6)
        {
            ReportPhysicsError(
                this,
                InAssetData,
                LOCTEXT("RuntimeRagdollNeedsBodies", "Runtime zombie ragdoll Physics Assets must contain at least six simple bodies before promotion."));
            bValid = false;
        }

        if (bRuntimeRagdollCandidate && BodyCount > 1 && ConstraintCount <= 0)
        {
            ReportPhysicsError(
                this,
                InAssetData,
                LOCTEXT("RuntimeRagdollNeedsConstraints", "Runtime zombie ragdoll Physics Assets must include joint constraints before promotion."));
            bValid = false;
        }

        if (bValid)
        {
            AssetPasses(InAsset);
            return EDataValidationResult::Valid;
        }

        return EDataValidationResult::Invalid;
    }

    if (IsGeometryCollectionAsset(InAsset))
    {
        ReportPhysicsWarning(
            this,
            InAssetData,
            LOCTEXT("GeometryCollectionManualGate", "Geometry Collection promotion must document fixed fracture seed, live-piece budget, cached set-piece path, and sleep/disable retirement in the physics promotion manifest."));
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    return EDataValidationResult::NotValidated;
}

#undef LOCTEXT_NAMESPACE
