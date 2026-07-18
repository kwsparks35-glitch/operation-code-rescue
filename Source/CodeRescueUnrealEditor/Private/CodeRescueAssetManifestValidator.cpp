#include "CodeRescueAssetManifestValidator.h"

#include "CodeRescueUnreal/CodeRescueAssetManifest.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "CodeRescueAssetManifestValidator"

namespace
{
template <typename AssetType>
bool ValidateSoftReference(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const TSoftObjectPtr<AssetType>& Reference,
    const FText& Label)
{
    if (Reference.IsNull() || !Reference.ToSoftObjectPath().IsValid())
    {
        Validator->AssetMessage(
            AssetData,
            EMessageSeverity::Error,
            FText::Format(LOCTEXT("MissingSoftReference", "{0} is required before this manifest can be promoted."), Label));
        return false;
    }

    return true;
}

template <typename AssetType>
bool ValidateSoftReferenceArray(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const TArray<TSoftObjectPtr<AssetType>>& References,
    const FText& Label)
{
    if (References.IsEmpty())
    {
        Validator->AssetMessage(
            AssetData,
            EMessageSeverity::Error,
            FText::Format(LOCTEXT("MissingSoftReferenceArray", "{0} requires at least one reviewed asset."), Label));
        return false;
    }

    bool bValid = true;
    for (int32 Index = 0; Index < References.Num(); ++Index)
    {
        if (References[Index].IsNull() || !References[Index].ToSoftObjectPath().IsValid())
        {
            Validator->AssetMessage(
                AssetData,
                EMessageSeverity::Error,
                FText::Format(
                    LOCTEXT("NullSoftReferenceArrayEntry", "{0} entry {1} is empty and must be removed or assigned."),
                    Label,
                    FText::AsNumber(Index)));
            bValid = false;
        }
    }

    return bValid;
}
}

bool UCodeRescueAssetManifestValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    return InAsset && InAsset->IsA<UCodeRescueAssetManifest>();
}

EDataValidationResult UCodeRescueAssetManifestValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
    UCodeRescueAssetManifest* Manifest = Cast<UCodeRescueAssetManifest>(InAsset);
    if (!Manifest)
    {
        return EDataValidationResult::NotValidated;
    }

    bool bValid = true;
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->ZombieSkeletalMesh, LOCTEXT("ZombieSkeletalMesh", "Zombie skeletal mesh"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->SurvivorSkeletalMesh, LOCTEXT("SurvivorSkeletalMesh", "Survivor skeletal mesh"));
    bValid &= ValidateSoftReferenceArray(this, InAssetData, Manifest->CityBuildingMeshes, LOCTEXT("CityBuildingMeshes", "City building mesh list"));
    bValid &= ValidateSoftReferenceArray(this, InAssetData, Manifest->BarricadeMeshes, LOCTEXT("BarricadeMeshes", "Barricade mesh list"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->MuzzleFlashVFX, LOCTEXT("MuzzleFlashVFX", "Muzzle flash VFX"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->BulletImpactVFX, LOCTEXT("BulletImpactVFX", "Bullet impact VFX"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->FireAndSmokeVFX, LOCTEXT("FireAndSmokeVFX", "Fire and smoke VFX"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->InfectionCloudVFX, LOCTEXT("InfectionCloudVFX", "Infection cloud VFX"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->RadioBriefingSound, LOCTEXT("RadioBriefingSound", "Radio briefing sound"));
    bValid &= ValidateSoftReference(this, InAssetData, Manifest->ZombieAttackSound, LOCTEXT("ZombieAttackSound", "Zombie attack sound"));

    if (bValid)
    {
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    return EDataValidationResult::Invalid;
}

#undef LOCTEXT_NAMESPACE
