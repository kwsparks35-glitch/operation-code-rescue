#include "CodeRescueV3PhysicsLibrary.h"

#include "Engine/SkeletalMesh.h"
#include "PhysicsAssetUtils.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "ReferenceSkeleton.h"

// The engine's vert-info auto-fitter reduced the authored 17-bone rigs to a
// 2-body (pelvis+chest) asset regardless of parameters, so this builder takes
// full control: capsules are derived analytically from the REFERENCE SKELETON
// (bone origin -> child-bone origin), spheres cover the leaf bones, and every
// joint gets a cone-limited constraint. Deterministic and mesh-independent.

namespace
{
struct FCapsuleDef { const TCHAR* Bone; const TCHAR* Child; float Radius; };
struct FSphereDef  { const TCHAR* Bone; float Radius; };
struct FJointDef   { const TCHAR* Child; const TCHAR* Parent; float Swing; float Twist; };

const FCapsuleDef GCapsules[] = {
    { TEXT("pelvis"),     TEXT("spine"),     13.0f },
    { TEXT("spine"),      TEXT("chest"),     12.0f },
    { TEXT("chest"),      TEXT("neck"),      14.0f },
    { TEXT("upperarm_R"), TEXT("forearm_R"),  6.0f },
    { TEXT("upperarm_L"), TEXT("forearm_L"),  6.0f },
    { TEXT("forearm_R"),  TEXT("hand_R"),     5.0f },
    { TEXT("forearm_L"),  TEXT("hand_L"),     5.0f },
    { TEXT("thigh_R"),    TEXT("shin_R"),     8.0f },
    { TEXT("thigh_L"),    TEXT("shin_L"),     8.0f },
    { TEXT("shin_R"),     TEXT("foot_R"),     6.0f },
    { TEXT("shin_L"),     TEXT("foot_L"),     6.0f },
};
const FSphereDef GSpheres[] = {
    { TEXT("head"),   11.0f },
    { TEXT("hand_R"),  6.0f },
    { TEXT("hand_L"),  6.0f },
    { TEXT("foot_R"),  7.0f },
    { TEXT("foot_L"),  7.0f },
};
const FJointDef GJoints[] = {
    { TEXT("spine"),      TEXT("pelvis"),     20.0f, 15.0f },
    { TEXT("chest"),      TEXT("spine"),      20.0f, 15.0f },
    { TEXT("head"),       TEXT("chest"),      35.0f, 25.0f },
    { TEXT("upperarm_R"), TEXT("chest"),      70.0f, 30.0f },
    { TEXT("upperarm_L"), TEXT("chest"),      70.0f, 30.0f },
    { TEXT("forearm_R"),  TEXT("upperarm_R"), 60.0f, 20.0f },
    { TEXT("forearm_L"),  TEXT("upperarm_L"), 60.0f, 20.0f },
    { TEXT("hand_R"),     TEXT("forearm_R"),  40.0f, 20.0f },
    { TEXT("hand_L"),     TEXT("forearm_L"),  40.0f, 20.0f },
    { TEXT("thigh_R"),    TEXT("pelvis"),     55.0f, 25.0f },
    { TEXT("thigh_L"),    TEXT("pelvis"),     55.0f, 25.0f },
    { TEXT("shin_R"),     TEXT("thigh_R"),    70.0f, 15.0f },
    { TEXT("shin_L"),     TEXT("thigh_L"),    70.0f, 15.0f },
    { TEXT("foot_R"),     TEXT("shin_R"),     30.0f, 15.0f },
    { TEXT("foot_L"),     TEXT("shin_L"),     30.0f, 15.0f },
};

int32 FindBoneTolerant(const FReferenceSkeleton& Ref, const TCHAR* Name)
{
    int32 Index = Ref.FindBoneIndex(FName(Name));
    if (Index == INDEX_NONE)
    {
        // Blender "." separators survive on some importer versions.
        FString Dotted(Name);
        Dotted = Dotted.Replace(TEXT("_"), TEXT("."));
        Index = Ref.FindBoneIndex(FName(*Dotted));
    }
    return Index;
}

FTransform ComponentSpace(const FReferenceSkeleton& Ref, int32 BoneIndex)
{
    FTransform T = FTransform::Identity;
    while (BoneIndex != INDEX_NONE)
    {
        T = T * Ref.GetRefBonePose()[BoneIndex];
        BoneIndex = Ref.GetParentIndex(BoneIndex);
    }
    return T;
}
}

int32 UCodeRescueV3PhysicsLibrary::RebuildAuthoredPhysicsAsset(USkeletalMesh* Mesh, float MinBoneSize)
{
    (void)MinBoneSize;
    if (!Mesh)
    {
        return -1;
    }
    UPhysicsAsset* PhysicsAsset = Mesh->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("[V3PhysFix] %s has no physics asset (import with create_physics_asset first)"),
            *Mesh->GetName());
        return -1;
    }
    const FReferenceSkeleton& Ref = Mesh->GetRefSkeleton();

    // Wipe whatever the auto-fitter produced.
    while (PhysicsAsset->ConstraintSetup.Num() > 0)
    {
        FPhysicsAssetUtils::DestroyConstraint(PhysicsAsset, 0);
    }
    while (PhysicsAsset->SkeletalBodySetups.Num() > 0)
    {
        FPhysicsAssetUtils::DestroyBody(PhysicsAsset, 0);
    }

    FPhysAssetCreateParams Params;
    Params.GeomType = EFG_Sphyl;
    Params.bCreateConstraints = false;

    int32 BodiesMade = 0;
    auto ResolveBoneName = [&Ref](const TCHAR* Name) -> FName
    {
        const int32 Index = FindBoneTolerant(Ref, Name);
        return Index != INDEX_NONE ? Ref.GetBoneName(Index) : NAME_None;
    };

    for (const FCapsuleDef& Def : GCapsules)
    {
        const int32 BoneIndex = FindBoneTolerant(Ref, Def.Bone);
        const int32 ChildIndex = FindBoneTolerant(Ref, Def.Child);
        if (BoneIndex == INDEX_NONE || ChildIndex == INDEX_NONE)
        {
            UE_LOG(LogTemp, Error, TEXT("[V3PhysFix] %s: bone pair %s->%s missing, skipped"),
                *Mesh->GetName(), Def.Bone, Def.Child);
            continue;
        }
        const FName BoneName = Ref.GetBoneName(BoneIndex);
        const int32 BodyIndex = FPhysicsAssetUtils::CreateNewBody(PhysicsAsset, BoneName, Params);
        USkeletalBodySetup* Setup = PhysicsAsset->SkeletalBodySetups[BodyIndex];
        const FTransform BoneCS = ComponentSpace(Ref, BoneIndex);
        const FTransform ChildCS = ComponentSpace(Ref, ChildIndex);
        const FVector LocalChild = BoneCS.InverseTransformPosition(ChildCS.GetLocation());
        const float SegLength = static_cast<float>(LocalChild.Size());
        const FVector Dir = SegLength > KINDA_SMALL_NUMBER ? LocalChild / SegLength : FVector::ZAxisVector;

        Setup->AggGeom.EmptyElements();
        FKSphylElem Capsule(Def.Radius, FMath::Max(2.0f, SegLength - Def.Radius));
        Capsule.Center = LocalChild * 0.5f;
        Capsule.Rotation = FRotationMatrix::MakeFromZ(Dir).Rotator();
        Setup->AggGeom.SphylElems.Add(Capsule);
        Setup->InvalidatePhysicsData();
        ++BodiesMade;
    }

    for (const FSphereDef& Def : GSpheres)
    {
        const int32 BoneIndex = FindBoneTolerant(Ref, Def.Bone);
        if (BoneIndex == INDEX_NONE)
        {
            continue;
        }
        const FName BoneName = Ref.GetBoneName(BoneIndex);
        const int32 BodyIndex = FPhysicsAssetUtils::CreateNewBody(PhysicsAsset, BoneName, Params);
        USkeletalBodySetup* Setup = PhysicsAsset->SkeletalBodySetups[BodyIndex];
        Setup->AggGeom.EmptyElements();
        FKSphereElem Sphere(Def.Radius);
        FVector Center = FVector::ZeroVector;
        const int32 ParentIndex = Ref.GetParentIndex(BoneIndex);
        if (ParentIndex != INDEX_NONE)
        {
            // Push the sphere away from the parent joint, along the bone.
            const FTransform BoneCS = ComponentSpace(Ref, BoneIndex);
            const FTransform ParentCS = ComponentSpace(Ref, ParentIndex);
            const FVector FromParent = BoneCS.InverseTransformPosition(ParentCS.GetLocation());
            if (!FromParent.IsNearlyZero())
            {
                Center = -FromParent.GetSafeNormal() * (Def.Radius * 0.65f);
            }
        }
        Sphere.Center = Center;
        Setup->AggGeom.SphereElems.Add(Sphere);
        Setup->InvalidatePhysicsData();
        ++BodiesMade;
    }

    // CreateNewBody may have auto-added constraints on some engine versions —
    // clear again and author the joint set deliberately.
    while (PhysicsAsset->ConstraintSetup.Num() > 0)
    {
        FPhysicsAssetUtils::DestroyConstraint(PhysicsAsset, 0);
    }
    PhysicsAsset->UpdateBodySetupIndexMap();

    int32 JointsMade = 0;
    for (const FJointDef& Def : GJoints)
    {
        const FName ChildName = ResolveBoneName(Def.Child);
        const FName ParentName = ResolveBoneName(Def.Parent);
        if (ChildName.IsNone() || ParentName.IsNone() ||
            PhysicsAsset->FindBodyIndex(ChildName) == INDEX_NONE ||
            PhysicsAsset->FindBodyIndex(ParentName) == INDEX_NONE)
        {
            continue;
        }
        const int32 ConstraintIndex = FPhysicsAssetUtils::CreateNewConstraint(PhysicsAsset, ChildName);
        UPhysicsConstraintTemplate* Template = PhysicsAsset->ConstraintSetup[ConstraintIndex];
        FConstraintInstance& Instance = Template->DefaultInstance;
        Instance.ConstraintBone1 = ChildName;
        Instance.ConstraintBone2 = ParentName;
        const FTransform ChildCS = ComponentSpace(Ref, FindBoneTolerant(Ref, Def.Child));
        const FTransform ParentCS = ComponentSpace(Ref, FindBoneTolerant(Ref, Def.Parent));
        Instance.SetRefFrame(EConstraintFrame::Frame1, FTransform::Identity);
        Instance.SetRefFrame(EConstraintFrame::Frame2, ChildCS.GetRelativeTransform(ParentCS));
        Instance.SetAngularSwing1Limit(ACM_Limited, Def.Swing);
        Instance.SetAngularSwing2Limit(ACM_Limited, Def.Swing);
        Instance.SetAngularTwistLimit(ACM_Limited, Def.Twist);
        Instance.SetDisableCollision(true);
        PhysicsAsset->DisableCollision(
            PhysicsAsset->FindBodyIndex(ChildName), PhysicsAsset->FindBodyIndex(ParentName));
        ++JointsMade;
    }

    PhysicsAsset->UpdateBodySetupIndexMap();
    PhysicsAsset->UpdateBoundsBodiesArray();
    PhysicsAsset->MarkPackageDirty();
    Mesh->MarkPackageDirty();

    TArray<FString> BodyBones;
    for (const USkeletalBodySetup* Setup : PhysicsAsset->SkeletalBodySetups)
    {
        if (Setup)
        {
            BodyBones.Add(Setup->BoneName.ToString());
        }
    }
    UE_LOG(LogTemp, Error, TEXT("[V3PhysFix] %s -> %d bodies (%s), %d constraints"),
        *Mesh->GetName(), BodyBones.Num(), *FString::Join(BodyBones, TEXT(",")), JointsMade);
    return BodyBones.Num();
}
