#include "CodeRescueMacCompatibilityValidator.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "CodeRescueMacCompatibilityValidator"

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

bool IsGroomLikeAsset(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("Groom"))
        || ReviewText.Contains(TEXT("/Game/Grooms"))
        || ReviewText.Contains(TEXT("HairStrands"));
}

bool IsMacCharacterCandidate(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("/Game/Characters"))
        || ReviewText.Contains(TEXT("/Game/YI_ModularZombies"))
        || ReviewText.Contains(TEXT("/Game/Zombie"))
        || ReviewText.Contains(TEXT("/Game/ZombieFemale"))
        || ReviewText.Contains(TEXT("/Game/DogZombie"))
        || ReviewText.Contains(TEXT("/Game/UrbanZombie4"))
        || ReviewText.Contains(TEXT("MetaHuman"));
}

bool IsMacWorldCandidate(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("/Game/World/"))
        || ReviewText.Contains(TEXT("/Game/ModernBridges"))
        || ReviewText.Contains(TEXT("/Game/Parallax_Night_Building_Material"))
        || ReviewText.Contains(TEXT("/Game/CodeRescueAssets/World"))
        || ReviewText.Contains(TEXT("Nanite"))
        || ReviewText.Contains(TEXT("HLOD"));
}

bool IsMacMaterialCandidate(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("/Game/Parallax_Night_Building_Material"))
        || ReviewText.Contains(TEXT("/Game/ModernBridges"))
        || ReviewText.Contains(TEXT("/Game/World/"))
        || ReviewText.Contains(TEXT("/Game/Grooms"))
        || ReviewText.Contains(TEXT("Shader"))
        || ReviewText.Contains(TEXT("VFX"));
}

bool IsStrictMacRuntimePromotion(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("MacRuntime"))
        || ReviewText.Contains(TEXT("MacCompatible"))
        || ReviewText.Contains(TEXT("RuntimeReady"))
        || ReviewText.Contains(TEXT("Promoted"))
        || ReviewText.Contains(TEXT("MacHairCardRuntimeReady"))
        || ReviewText.Contains(TEXT("MacNonNaniteFallbackReady"));
}

bool HasMacFallbackToken(const FString& ReviewText)
{
    return ReviewText.Contains(TEXT("Fallback"))
        || ReviewText.Contains(TEXT("Card"))
        || ReviewText.Contains(TEXT("MeshHair"))
        || ReviewText.Contains(TEXT("MacHairCardRuntimeReady"))
        || ReviewText.Contains(TEXT("MacNonNaniteFallbackReady"));
}

void ReportMacError(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FText& Message)
{
    Validator->AssetMessage(AssetData, EMessageSeverity::Error, Message);
}

void ReportMacWarning(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FText& Message)
{
    Validator->AssetMessage(AssetData, EMessageSeverity::Warning, Message);
}
}

bool UCodeRescueMacCompatibilityValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    if (!InAsset)
    {
        return false;
    }

    const FString ReviewText = AssetReviewText(InAssetData, InAsset);
    return IsGroomLikeAsset(ReviewText)
        || (InAsset->IsA<USkeletalMesh>() && IsMacCharacterCandidate(ReviewText))
        || (InAsset->IsA<UStaticMesh>() && IsMacWorldCandidate(ReviewText))
        || (InAsset->IsA<UMaterialInterface>() && IsMacMaterialCandidate(ReviewText));
}

EDataValidationResult UCodeRescueMacCompatibilityValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
    if (!InAsset)
    {
        return EDataValidationResult::NotValidated;
    }

    const FString ReviewText = AssetReviewText(InAssetData, InAsset);
    const bool bStrictPromotion = IsStrictMacRuntimePromotion(ReviewText);

    if (IsGroomLikeAsset(ReviewText))
    {
        if (bStrictPromotion && !HasMacFallbackToken(ReviewText))
        {
            ReportMacError(
                this,
                InAssetData,
                LOCTEXT("GroomNeedsMacFallback", "Mac runtime-promoted character hair cannot be a strand Groom alone; author a hair-card or mesh fallback before promotion."));
            return EDataValidationResult::Invalid;
        }

        ReportMacWarning(
            this,
            InAssetData,
            LOCTEXT("GroomReviewOnly", "Groom and HairStrands assets are GroomStrandReviewOnlyMac inputs until a card or mesh fallback is documented."));
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(InAsset))
    {
        bool bValid = true;
        const int32 LODCount = SkeletalMesh->GetLODNum();

        if (LODCount <= 0)
        {
            ReportMacError(
                this,
                InAssetData,
                LOCTEXT("SkeletalMeshNeedsLods", "Mac-promoted skeletal meshes must contain renderable LOD data."));
            bValid = false;
        }
        else if (LODCount < 2)
        {
            if (bStrictPromotion)
            {
                ReportMacError(
                    this,
                    InAssetData,
                    LOCTEXT("StrictSkeletalMeshNeedsLodChain", "Runtime-promoted Mac skeletal meshes must document an authored LOD chain or explicit hero-character exception."));
                bValid = false;
            }
            else
            {
                ReportMacWarning(
                    this,
                    InAssetData,
                    LOCTEXT("SkeletalMeshLodReview", "Skeletal mesh Mac promotion should document LOD chain, bone influence, URO, and close-range hero exceptions before scaling."));
            }
        }

        if (bValid)
        {
            AssetPasses(InAsset);
            return EDataValidationResult::Valid;
        }

        return EDataValidationResult::Invalid;
    }

    if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(InAsset))
    {
        bool bValid = true;
        const int32 LODCount = StaticMesh->GetNumLODs();
        const bool bLooksNaniteOnly = ReviewText.Contains(TEXT("Nanite")) && !HasMacFallbackToken(ReviewText);

        if (LODCount <= 0)
        {
            ReportMacError(
                this,
                InAssetData,
                LOCTEXT("StaticMeshNeedsLods", "Mac-promoted static meshes must contain renderable LOD data."));
            bValid = false;
        }

        if (bStrictPromotion && bLooksNaniteOnly)
        {
            ReportMacError(
                this,
                InAssetData,
                LOCTEXT("NaniteNeedsMacFallback", "Nanite/SM6 world content requires MacNaniteSM6ReviewGate evidence plus a MacNonNaniteFallbackReady path before runtime promotion."));
            bValid = false;
        }
        else if (bLooksNaniteOnly)
        {
            ReportMacWarning(
                this,
                InAssetData,
                LOCTEXT("NaniteReviewOnly", "Nanite-named content remains review-only on Mac until target-hardware evidence and non-Nanite fallback are documented."));
        }

        if (LODCount == 1)
        {
            ReportMacWarning(
                this,
                InAssetData,
                LOCTEXT("StaticMeshLodReview", "Static mesh Mac promotion should document an LOD chain, Nanite target-hardware review, or non-Nanite fallback path."));
        }

        if (bValid)
        {
            AssetPasses(InAsset);
            return EDataValidationResult::Valid;
        }

        return EDataValidationResult::Invalid;
    }

    if (UMaterialInterface* Material = Cast<UMaterialInterface>(InAsset))
    {
        ReportMacWarning(
            this,
            InAssetData,
            LOCTEXT("MaterialShaderReview", "Mac-promoted materials should retain texture-memory, shader-complexity, VFX/translucency, and packaged render-smoke evidence before runtime promotion."));
        AssetPasses(Material);
        return EDataValidationResult::Valid;
    }

    return EDataValidationResult::NotValidated;
}

#undef LOCTEXT_NAMESPACE
