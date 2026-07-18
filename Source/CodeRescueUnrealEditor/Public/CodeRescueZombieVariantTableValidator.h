#pragma once

#include "EditorValidatorBase.h"
#include "CodeRescueZombieVariantTableValidator.generated.h"

class FDataValidationContext;

/**
 * Validates the promoted zombie variant DataTable used by runtime spawning.
 */
UCLASS()
class CODERESCUEUNREALEDITOR_API UCodeRescueZombieVariantTableValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

protected:
    virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const override;
    virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) override;
};
