#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Map/MapEnum.h"   // EAreaType
#include "StageEncounterTypes.generated.h"

/**
 * FStageEncounterRow
 * 스테이지 인카운터 테이블의 한 행 = "어떤 방 타입/층에 어떤 적 구성이 가중치 얼마로 등장하는가".
 * CombatManager가 방 타입(CombatAreaType)·층(CombatFloor)에 맞는 행들 중 가중치 랜덤으로 하나를 뽑아,
 * 그 행의 EnemyID들로 적을 스폰한다.
 *
 * 데이터 계층: EnemyTable(도감) → 이 테이블(등장 규칙 + 적 구성 직접 기재)
 */
USTRUCT(BlueprintType)
struct FStageEncounterRow : public FTableRowBase
{
	GENERATED_BODY()

	// 이 인카운터가 등장하는 방 타입 (Normal / Elite / Boss ...)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	EAreaType AreaType = EAreaType::Normal;

	// 이 행에서 스폰할 적 EnemyID 목록 (EnemyTable에 있는 ID, 최대 3, 박스 순서대로)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TArray<FName> EnemyIDs;

	// 가중치 — 클수록 자주 등장 (0이면 후보에서 사실상 제외)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage", meta = (ClampMin = "0"))
	float Weight = 1.f;

	// 등장 가능 최소/최대 층 (CombatManager.CombatFloor 기준)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 MinFloor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 MaxFloor = 99;
};
