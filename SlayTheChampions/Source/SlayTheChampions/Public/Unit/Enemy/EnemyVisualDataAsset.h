// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Unit/UnitAnimDataAsset.h"
#include "EnemyVisualDataAsset.generated.h"

class USkeletalMesh;
class UAnimInstnace;
/**
 * 적 한종류의 비주얼 전체 메시 +AnimBP+몽타주 세트를 담는 DataAsset
 *
 */
UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UEnemyVisualDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	//메시/셰이더
	UPROPERTY(EditAnywhere, Category = "Visual|Mesh")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Visual|Mesh")
	TSubclassOf<UAnimInstance> AnimBPClass;

	UPROPERTY(EditAnywhere, Category = "Visual|Mesh")
	FVector MeshScale = FVector(1.f);

	UPROPERTY(EditAnywhere, Category = "Visual|Mesh")
	FVector MeshOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual|Mesh")
	FRotator MeshRotation = FRotator::ZeroRotator;

	//몽타주 세트
	UPROPERTY(EditAnywhere, Category = "Visual|Animation")
	TObjectPtr<UUnitAnimDataAsset> AnimData;
};
