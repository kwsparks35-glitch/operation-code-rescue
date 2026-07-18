#include "CodeRescueZombieVariantTableValidator.h"

#include "CodeRescueUnreal/CodeRescueTypes.h"
#include "Engine/DataTable.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "CodeRescueZombieVariantTableValidator"

namespace
{
bool IsExplicitFallbackRow(const FName& RowName, const FZombieVariantRow& Row)
{
    return Row.Variant == EZombieVariant::Default
        || RowName == TEXT("Default")
        || RowName == TEXT("BaseMesh");
}

void ReportRowError(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FName& RowName,
    const FText& Message)
{
    Validator->AssetMessage(
        AssetData,
        EMessageSeverity::Error,
        FText::Format(
            LOCTEXT("ZombieVariantRowError", "Zombie variant row '{0}': {1}"),
            FText::FromName(RowName),
            Message));
}

bool ContainsMacIncompatibleGroomPath(const FSoftObjectPath& AssetPath)
{
    const FString PathString = AssetPath.ToString();
    return PathString.Contains(TEXT("/Game/Grooms"))
        || PathString.Contains(TEXT("GroomStrands"))
        || PathString.Contains(TEXT("StrandGroom"));
}

bool ValidateLoadablePath(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FName& RowName,
    FSoftObjectPath AssetPath,
    const FText& Label,
    const bool bRequired)
{
    if (!AssetPath.IsValid())
    {
        if (bRequired)
        {
            ReportRowError(
                Validator,
                AssetData,
                RowName,
                FText::Format(LOCTEXT("MissingCharacterPath", "{0} is required for promoted runtime variants."), Label));
        }
        return !bRequired;
    }

    if (ContainsMacIncompatibleGroomPath(AssetPath))
    {
        ReportRowError(
            Validator,
            AssetData,
            RowName,
            FText::Format(LOCTEXT("MacGroomPath", "{0} points at a strand groom path; use a skeletal mesh, card, or mesh fallback for Mac runtime."), Label));
        return false;
    }

    UObject* LoadedAsset = AssetPath.TryLoad();
    if (!LoadedAsset)
    {
        ReportRowError(
            Validator,
            AssetData,
            RowName,
            FText::Format(LOCTEXT("UnresolvedCharacterPath", "{0} could not be loaded from {1}."), Label, FText::FromString(AssetPath.ToString())));
        return false;
    }

    return true;
}

bool ValidateMultiplier(
    UEditorValidatorBase* Validator,
    const FAssetData& AssetData,
    const FName& RowName,
    const float Value,
    const FText& Label)
{
    if (Value < 0.1f || Value > 5.0f)
    {
        ReportRowError(
            Validator,
            AssetData,
            RowName,
            FText::Format(LOCTEXT("BadMultiplier", "{0} must stay inside the authored 0.1 to 5.0 tuning range."), Label));
        return false;
    }

    return true;
}
}

bool UCodeRescueZombieVariantTableValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
    const UDataTable* DataTable = Cast<UDataTable>(InAsset);
    return DataTable && DataTable->GetRowStruct() == FZombieVariantRow::StaticStruct();
}

EDataValidationResult UCodeRescueZombieVariantTableValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
    const UDataTable* DataTable = Cast<UDataTable>(InAsset);
    if (!DataTable || DataTable->GetRowStruct() != FZombieVariantRow::StaticStruct())
    {
        return EDataValidationResult::NotValidated;
    }

    const TMap<FName, uint8*>& RowMap = DataTable->GetRowMap();
    if (RowMap.IsEmpty())
    {
        AssetMessage(
            InAssetData,
            EMessageSeverity::Error,
            LOCTEXT("EmptyZombieVariantTable", "Zombie variant DataTable must contain at least one authored runtime row."));
        return EDataValidationResult::Invalid;
    }

    bool bValid = true;
    int32 PromotedVariantCount = 0;

    for (const TPair<FName, uint8*>& RowPair : RowMap)
    {
        const FName RowName = RowPair.Key;
        const FZombieVariantRow* Row = reinterpret_cast<const FZombieVariantRow*>(RowPair.Value);
        if (!Row)
        {
            ReportRowError(this, InAssetData, RowName, LOCTEXT("NullRow", "row data is null."));
            bValid = false;
            continue;
        }

        const bool bFallbackRow = IsExplicitFallbackRow(RowName, *Row);
        if (!bFallbackRow)
        {
            ++PromotedVariantCount;
        }

        if (Row->DisplayName.TrimStartAndEnd().IsEmpty())
        {
            ReportRowError(this, InAssetData, RowName, LOCTEXT("MissingDisplayName", "DisplayName is required for HUD/debug readability."));
            bValid = false;
        }

        bValid &= ValidateMultiplier(this, InAssetData, RowName, Row->HealthMultiplier, LOCTEXT("HealthMultiplier", "HealthMultiplier"));
        bValid &= ValidateMultiplier(this, InAssetData, RowName, Row->DamageMultiplier, LOCTEXT("DamageMultiplier", "DamageMultiplier"));
        bValid &= ValidateMultiplier(this, InAssetData, RowName, Row->SpeedMultiplier, LOCTEXT("SpeedMultiplier", "SpeedMultiplier"));
        bValid &= ValidateMultiplier(this, InAssetData, RowName, Row->MeshScale, LOCTEXT("MeshScale", "MeshScale"));

        bValid &= ValidateLoadablePath(this, InAssetData, RowName, Row->SkeletalMesh.ToSoftObjectPath(), LOCTEXT("SkeletalMesh", "SkeletalMesh"), !bFallbackRow);
        bValid &= ValidateLoadablePath(this, InAssetData, RowName, Row->AnimBPClass.ToSoftObjectPath(), LOCTEXT("AnimBPClass", "AnimBPClass"), !bFallbackRow);

        for (const TPair<int32, float>& ZoneWeight : Row->ZoneWeights)
        {
            if (ZoneWeight.Key < 0)
            {
                ReportRowError(this, InAssetData, RowName, LOCTEXT("NegativeZoneKey", "ZoneWeights cannot use negative zone indexes."));
                bValid = false;
            }
            if (ZoneWeight.Value < 0.0f || ZoneWeight.Value > 10.0f)
            {
                ReportRowError(this, InAssetData, RowName, LOCTEXT("BadZoneWeight", "ZoneWeights must stay in the 0.0 to 10.0 authored spawn range."));
                bValid = false;
            }
        }
    }

    if (PromotedVariantCount <= 0)
    {
        AssetMessage(
            InAssetData,
            EMessageSeverity::Error,
            LOCTEXT("NoPromotedZombieVariants", "Zombie variant DataTable must contain at least one promoted non-fallback variant."));
        bValid = false;
    }

    if (bValid)
    {
        AssetPasses(InAsset);
        return EDataValidationResult::Valid;
    }

    return EDataValidationResult::Invalid;
}

#undef LOCTEXT_NAMESPACE
