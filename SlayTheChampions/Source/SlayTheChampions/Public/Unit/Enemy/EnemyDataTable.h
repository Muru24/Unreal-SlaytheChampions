// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Unit/Enemy/GimmickComponent.h"
#include "Unit/Enemy/EnemyVisualDataAsset.h"
#include "Unit/StatusEffect.h"
#include "Unit/CombatTypes.h"
#include "Unit/Enemy/EnemyPatternData.h"
#include "EnemyDataTable.generated.h"

class UGimmickData;


//전투 시작시 미리 부여할 상태이상
USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FStartingEffect
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<UStatusEffect> EffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Stacks = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Duration = -1; // -1 = 무한
};

//몬스터 1종의 모든 데이터를 담는 Struct
//UEnemyTableData 배열의 원소로 사용됨
//EnemyInitializerComponent::InitializeFromDefinition() 에 const ref로 전달

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEnemyDefinition
{
    GENERATED_BODY()

	//식별
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity") FName EnemyID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity") FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity") EEnemyTier Tier = EEnemyTier::Normal;

	//스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")	int32 MaxHP = 100;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")	int32 BaseDefence = 0;

	//행동패턴
	//NPCBrainComponent에 주입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<UEnemyPatternData> Pattern;

	//전투시작상태이상부여
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	TArray<FStartingEffect> StartingEffects;


	//기믹
	//null이면 일반 몬스터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
	TSubclassOf<UGimmickComponent> GimmickClass;

	// 같은 기믹이지만 수치를 다르게 하고 싶을때만 사용
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
	TObjectPtr<UGimmickData> GimmickData;
	

	//비주얼
	// 스켈레탈 메시 + AnimBP + 몽타주 세트를 담은 DA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UEnemyVisualDataAsset> VisualData;
};

/**
 * 전반적인 적의 데이터를 모두 담는 테이블
 * 프로젝트에 DA를 만들어두고 Entries배열에 몬스터 추가하면 된다.
 */

UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UEnemyDataTable : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// 에디터에서 이 배열을 펼치면 모든 몬스터 데이터를 인라인으로 편집 가능
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Table")
	TArray<FEnemyDefinition> Entries;

	//검색 유틸
	//EnemyID로 찾기
	
	const FEnemyDefinition* FindByID(FName EnemyID) const
	{
		return Entries.FindByPredicate([&](const FEnemyDefinition& E)
			{
				return E.EnemyID == EnemyID;
			});
	}
	//Tier로 찾기
	UFUNCTION(BlueprintCallable, Category = "Enemy Table")
	TArray<FEnemyDefinition> FindByTier(EEnemyTier Tier) const
	{
		TArray<FEnemyDefinition> Result;
		for (const FEnemyDefinition& E : Entries)
		{
			if (E.Tier == Tier) Result.Add(E);
		}
		return Result;
	}
};
