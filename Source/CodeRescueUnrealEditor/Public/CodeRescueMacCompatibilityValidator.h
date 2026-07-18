#pragma once

#include "EditorValidatorBase.h"

#include "CodeRescueMacCompatibilityValidator.generated.h"

class FDataValidationContext;
struct FAssetData;
class UObject;

UCLASS()
class CODERESCUEUNREALEDITOR_API UCodeRescueMacCompatibilityValidator : public UEditorValidatorBase
{
    GENERATED_BODY()

protected:
    virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const override;
    virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) override;
};
