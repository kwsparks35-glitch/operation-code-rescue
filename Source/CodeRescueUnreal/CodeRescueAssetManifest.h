#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "CodeRescueAssetManifest.generated.h"

UCLASS(BlueprintType)
class CODERESCUEUNREAL_API UCodeRescueAssetManifest : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Characters|Zombies")
    TSoftObjectPtr<USkeletalMesh> ZombieSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Characters|Survivors")
    TSoftObjectPtr<USkeletalMesh> SurvivorSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Environment")
    TArray<TSoftObjectPtr<UStaticMesh>> CityBuildingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Environment")
    TArray<TSoftObjectPtr<UStaticMesh>> BarricadeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> MuzzleFlashVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> BulletImpactVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> FireAndSmokeVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX|Niagara")
    TSoftObjectPtr<UNiagaraSystem> InfectionCloudVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
    TSoftObjectPtr<USoundBase> RadioBriefingSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
    TSoftObjectPtr<USoundBase> ZombieAttackSound;
};
