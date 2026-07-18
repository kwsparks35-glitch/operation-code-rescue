#include "CodeRescueMaterialUtils.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace CodeRescueMaterials
{
UMaterialInterface* LoadMaterial(const TCHAR* MaterialPath)
{
    return MaterialPath ? LoadObject<UMaterialInterface>(nullptr, MaterialPath) : nullptr;
}

UMaterialInterface* ResolveDynamicMaterialParent(UMaterialInterface* Material)
{
    if (!Material)
    {
        return nullptr;
    }

    if (UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(Material))
    {
        if (UMaterialInterface* Parent = Dynamic->Parent)
        {
            return ResolveDynamicMaterialParent(Parent);
        }
        return Dynamic->GetBaseMaterial();
    }

    return Material;
}

UMaterialInstanceDynamic* CreateTintedDynamicMaterial(
    UMaterialInterface* Material,
    UObject* Outer,
    const FLinearColor& Tint,
    float EmissiveScale)
{
    UMaterialInterface* Parent = ResolveDynamicMaterialParent(Material);
    if (!Parent)
    {
        return nullptr;
    }

    UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Parent, Outer);
    if (!MID)
    {
        return nullptr;
    }

    MID->SetVectorParameterValue(TEXT("Color"), Tint);
    MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
    MID->SetVectorParameterValue(TEXT("EmissiveColor"), Tint * EmissiveScale);
    return MID;
}

UMaterialInstanceDynamic* ApplyTintedMaterial(
    UMeshComponent* Component,
    int32 Slot,
    UObject* Outer,
    const FLinearColor& Tint,
    float EmissiveScale)
{
    if (!Component || Slot < 0)
    {
        return nullptr;
    }

    UMaterialInstanceDynamic* MID = CreateTintedDynamicMaterial(
        Component->GetMaterial(Slot),
        Outer ? Outer : Component,
        Tint,
        EmissiveScale);
    if (MID)
    {
        Component->SetMaterial(Slot, MID);
    }
    return MID;
}
}
