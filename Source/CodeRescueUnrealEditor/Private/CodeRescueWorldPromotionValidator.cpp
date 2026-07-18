#include "CodeRescueWorldPromotionValidator.h"

#include "Engine/StaticMesh.h"
#include "Misc/DataValidation.h"
#include "PhysicsEngine/BodySetup.h"

#define LOCTEXT_NAMESPACE "CodeRescueWorldPromotionValidator"

namespace
{
FString AssetReviewText(const FAssetData& AssetData, const UObject* Asset)
{
    FString Text = AssetData.PackageName.ToString();
    Text += TEXT(" ");
    Text += AssetData.AssetName.ToString();
    if (Asset && Asset->GetClass())
    {
        Text += TEXT(" ");
        Text += Asset->GetClass()->GetName();
    }
    return Text;
}

bool IsWorldStaticMeshCandidate(const FAssetData& AssetData)
{
    const FString PackageName = AssetData.PackageName.ToString();
    return PackageName.Contains(TEXT("/Game/World/"))
        || PackageName.Contains(TEXT("/Game/CodeRescueAssets/World"))
        || PackageName.Contains(TEXT("/Game/ModernBridges"))
        || PackageName.Contains(TEXT("/Game/Parallax_Night_Building_Material"));
}

bool IsWorldFrameworkAsset(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("PackedLevelActor"))
        || ReviewText.Contains(TEXT("LevelInstance"))
        || ReviewText.Contains(TEXT("HLOD"))
        || ReviewText.Contains(TEXT("WorldPartition"))
        || ReviewText.Contains(TEXT("DataLayer"))
        || ReviewText.Contains(TEXT("PCG"));
}

bool IsStrictRuntimePromotion(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("WorldPromotion"))
        || ReviewText.Contains(TEXT("Promoted"))
        || ReviewText.Contains(TEXT("RuntimeReady"))
        || ReviewText.Contains(TEXT("CollisionRequired"))
        || ReviewText.Contains(TEXT("PlayerBlocker"))
        || ReviewText.Contains(TEXT("NavigationBlocker"))
        || ReviewText.Contains(TEXT("WalkableCityModule"));
}

bool HasSimpleCollision(const UStaticMesh* StaticMesh)
{
    const UBodySetup* BodySetup = StaticMesh ? StaticMesh->GetBodySetup() : nullptr;
    return BodySetup && BodySetup->AggGeom.GetElementCount() > 0;
}

bool UsesComplexAsSimple(const UStaticMesh* StaticMesh)
{
    const UBodySetup* BodySetup = StaticMesh ? StaticMesh->GetBodySetup() : nullptr;
    return BodySetup && BodySetup->CollisionTraceFlag == CTF_UseComplexAsSimple;
}

void ReportWorldError(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FText& Message)
{
    Validator->AssetMessage(AssetData, EMessageSeverity::Error, Message);
}

void ReportWorldWarning(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FText& Message)
{
    Validator->AssetMessage(AssetData, EMessageSeverity::Warning, Message);
}
}

bool UCodeRescueWorldPromotionValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    if (!InAsset)
    {
        return false;
    }

    const FString ReviewText = AssetReviewText(InAssetData, InAsset);
    return (InAsset->IsA<UStaticMesh>() && IsWorldStaticMeshCandidate(InAssetData))
        || IsWorldFrameworkAsset(ReviewText);
}

EDataValidationResult UCodeRescueWorldPromotionValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
    if (!InAsset)
    {
        return EDataValidationResult::NotValidated;
    }

    const FString ReviewText = AssetReviewText(InAssetData, InAsset);
    const bool bStrictPromotion = IsStrictRuntimePromotion(ReviewText);

    if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(InAsset))
    {
        bool bValid = true;
        const int32 LODCount = StaticMesh->GetNumLODs();
        const int32 MaterialCount = StaticMesh->GetStaticMaterials().Num();
        const bool bHasSimpleCollision = HasSimpleCollision(StaticMesh);
        const bool bUsesComplexAsSimple = UsesComplexAsSimple(StaticMesh);

        if (LODCount <= 0)
        {
            ReportWorldError(
                this,
                InAssetData,
                LOCTEXT("WorldMeshNeedsRenderData", "Promoted city module Static Meshes must contain renderable LOD data."));
            bValid = false;
        }

        if (MaterialCount <= 0)
        {
            if (bStrictPromotion)
            {
                ReportWorldError(
                    this,
                    InAssetData,
                    LOCTEXT("WorldMeshNeedsMaterial", "Runtime-promoted city modules must have at least one material slot so trim/master-material review can be performed."));
                bValid = false;
            }
            else
            {
                ReportWorldWarning(
                    this,
                    InAssetData,
                    LOCTEXT("WorldMeshMaterialReview", "City mesh candidate has no material slot; keep it review-only until trim/master-material assignment is complete."));
            }
        }

        if (!bHasSimpleCollision)
        {
            if (bStrictPromotion)
            {
                ReportWorldError(
                    this,
                    InAssetData,
                    LOCTEXT("WorldMeshNeedsSimpleCollision", "Runtime-promoted city modules must provide simple collision for player, AI, weapon, and accessibility traces."));
                bValid = false;
            }
            else
            {
                ReportWorldWarning(
                    this,
                    InAssetData,
                    LOCTEXT("WorldMeshCollisionReview", "City mesh candidate is review-only until simple collision is authored or explicitly waived in the world promotion manifest."));
            }
        }

        if (bUsesComplexAsSimple)
        {
            if (bStrictPromotion)
            {
                ReportWorldError(
                    this,
                    InAssetData,
                    LOCTEXT("WorldMeshNoComplexAsSimple", "Runtime-promoted city modules must not rely on complex-as-simple collision for walkable/blocking gameplay surfaces."));
                bValid = false;
            }
            else
            {
                ReportWorldWarning(
                    this,
                    InAssetData,
                    LOCTEXT("WorldMeshComplexCollisionReview", "City mesh candidate uses complex-as-simple collision; keep it review-only unless the manifest grants an explicit architectural-surface exception."));
            }
        }

        if (LODCount < 2)
        {
            ReportWorldWarning(
                this,
                InAssetData,
                LOCTEXT("WorldMeshLodReview", "City mesh promotion should document an authored LOD chain, Nanite target-hardware review, or a non-Nanite fallback path before scaling across the campaign."));
        }

        if (bValid)
        {
            AssetPasses(InAsset);
            return EDataValidationResult::Valid;
        }

        return EDataValidationResult::Invalid;
    }

    if (IsWorldFrameworkAsset(ReviewText))
    {
        ReportWorldWarning(
            this,
            InAssetData,
            LOCTEXT("WorldFrameworkManualGate", "World Partition, PCG, HLOD, Packed Level Actor, and Data Layer promotion must document streaming budgets, authored safe beats, fallback C++ coverage, and Mac render/package evidence in the world promotion manifest."));
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    return EDataValidationResult::NotValidated;
}

#undef LOCTEXT_NAMESPACE
