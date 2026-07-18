#pragma once

#include "CoreMinimal.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMeshComponent;

namespace CodeRescueMaterials
{
    UMaterialInterface* LoadMaterial(const TCHAR* MaterialPath);

    /** Returns a valid parent for UMaterialInstanceDynamic::Create.
     *  UE rejects dynamic material instances as parents, so this unwraps them
     *  to their original material/constant parent before creating a new MID. */
    UMaterialInterface* ResolveDynamicMaterialParent(UMaterialInterface* Material);

    UMaterialInstanceDynamic* CreateTintedDynamicMaterial(
        UMaterialInterface* Material,
        UObject* Outer,
        const FLinearColor& Tint,
        float EmissiveScale);

    UMaterialInstanceDynamic* ApplyTintedMaterial(
        UMeshComponent* Component,
        int32 Slot,
        UObject* Outer,
        const FLinearColor& Tint,
        float EmissiveScale);
}
