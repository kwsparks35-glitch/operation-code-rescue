#pragma once

// 2026-07-11 art+physics v3: deliberate physics-asset builder for the authored
// CharactersV3 skeletal meshes. The FBX importer's auto-create (default
// FPhysAssetCreateParams) produced only TWO bodies for the 17-bone authored
// rig — unusable for ragdoll and correctly rejected by CodeZombieActor's
// MatchedBodies >= 6 gate. This library regenerates the mesh's physics asset
// with explicit parameters (small MinBoneSize, capsule geometry, constraints)
// so every major limb gets a body. Called from Scripts/fix_v3_physics_assets.py.

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CodeRescueV3PhysicsLibrary.generated.h"

class USkeletalMesh;

UCLASS()
class CODERESCUEUNREALEDITOR_API UCodeRescueV3PhysicsLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Rebuild (or create) the physics asset for an authored skeletal mesh.
     *  Returns the resulting body count, or -1 on failure. */
    UFUNCTION(BlueprintCallable, Category="CodeRescue|Physics")
    static int32 RebuildAuthoredPhysicsAsset(USkeletalMesh* Mesh, float MinBoneSize = 5.0f);
};
