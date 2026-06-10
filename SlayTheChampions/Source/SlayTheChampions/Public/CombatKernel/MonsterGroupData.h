#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MonsterGroupData.generated.h"

class AUnit;

/**
 * FMonsterSlotData
 * 몬스터 그룹 데이터 에셋의 슬롯 하나.
 * MonsterClass로 스폰할 BP를 지정하고, UnitName·MaxHP를 설정한다.
 */
USTRUCT(BlueprintType)
struct FMonsterSlotData
{
	GENERATED_BODY()

	// 스폰할 몬스터 Blueprint 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	TSubclassOf<AUnit> MonsterClass;

	// 유닛 이름 — CombatStatWidget의 Text_Name에 표시됨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	FName UnitName;

	// 최대 HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "1"))
	int32 MaxHP = 100;
};

/**
 * UMonsterGroupData
 * 한 전투에서 등장하는 몬스터 구성을 정의하는 DataAsset.
 * CombatManager의 MonsterGroup 슬롯에 지정하면 InitCombat 시 자동 스폰.
 * Monsters 배열 크기가 EnemyCount를 대체하므로 최대 3개까지 등록.
 */
UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UMonsterGroupData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 이 전투에 등장하는 몬스터 슬롯 목록 (최대 3개)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<FMonsterSlotData> Monsters;
};
