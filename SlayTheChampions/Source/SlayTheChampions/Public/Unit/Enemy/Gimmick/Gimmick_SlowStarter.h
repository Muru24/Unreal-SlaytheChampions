// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/Enemy/GimmickComponent.h"
#include "Gimmick_SlowStarter.generated.h"

/**
 * 수면 각성
 * 일정시간 행동이 스킵되고, 깨어나면 강력하다.
 * NPCBrainComponent.PlanNextAction에서 IsSleeping()을 체크하여 행동 스킵 처리
 */

 // 각성 시 CombatManager가 구독하여 버프 적용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossAwakened);

UCLASS(ClassGroup = (Unit), meta = (BlueprintSpawnableComponent))
class SLAYTHECHAMPIONS_API UGimmick_SlowStarter : public UGimmickComponent
{
	GENERATED_BODY()
	

public:
	//수면 지속시간
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Gimmick|SlowStarter")
	int32 SleepDuration = 3;

	// 각성 시 적용할 버프 강도 (StatusEffect 스택 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sleep")
	int32 AwakenBuffValue = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sleep")
	FText AwakenAnnounce;

	UPROPERTY(BlueprintReadOnly, Category = "Gimmick|Sleep")
	bool bSleeping = true;

	// NPCBrainComponent가 호출하여 행동 여부 판단
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gimmick|Sleep")
	bool IsSleeping() const { return bSleeping; }

	// 피격 시 조기 각성 옵션 (데이터로 ON/OFF)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Sleep")
	bool bWakeOnDamage = false;

	UPROPERTY(BlueprintAssignable, Category = "Gimmick|Sleep")
	FOnBossAwakened OnAwakened;

protected:
	virtual void OnGimmickTurnStart() override;
	virtual void HandleHPChanged(int32 OldHP, int32 NewHP) override;

private:
	void Awaken();
};
