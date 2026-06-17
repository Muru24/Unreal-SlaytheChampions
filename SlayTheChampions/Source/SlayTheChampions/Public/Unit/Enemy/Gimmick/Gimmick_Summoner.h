// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/Enemy/GimmickComponent.h"
#include "Gimmick_Summoner.generated.h"

/**
 * 하수인을 소환하고 살아있는 동안 보스가 버프/힐을 받음
 * 실제 스폰은 CombatManager가 OnMinionSpawnRequested를 구독하여 처리
 * 스폰 후 CombatManager가 RegisterMinion을 호출하여 추적 등록
 */

class AUnit;

// CombatManager가 구독: 하수인 스폰 요청
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinionSpawnRequested, int32, Count);

UCLASS(ClassGroup = (Unit), meta = (BlueprintSpawnableComponent))
class SLAYTHECHAMPIONS_API UGimmick_Summoner : public UGimmickComponent
{
	GENERATED_BODY()
	

public:
	// 소환할 하수인 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Minion")
	int32 MinionCount = 2;

	// 하수인이 살아있는 동안 매 턴 회복량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Minion")
	int32 HealPerTurn = 5;

	// 소환 쿨타임 (0이면 전멸 후 즉시 재소환)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Minion")
	int32 ResummonCooldown = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Minion")
	FText SummonAnnounce;

	// 하수인 존재 여부
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gimmick|Minion")
	bool HasAliveMinion() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gimmick|Minion")
	int32 GetAliveMinionCount() const;

	// CombatManager가 스폰 후 호출
	UFUNCTION(BlueprintCallable, Category = "Gimmick|Minion")
	void RegisterMinion(AUnit* Minion);

	UPROPERTY(BlueprintAssignable, Category = "Gimmick|Minion")
	FOnMinionSpawnRequested OnMinionSpawnRequested;

protected:
	virtual void BeginPlay() override;
	virtual void OnGimmickTurnStart() override;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AUnit>> SpawnedMinions;

	// 마지막 소환 이후 경과 턴
	int32 TurnsSinceLastSummon = 0;
	bool bHasSummoned = false;
};
