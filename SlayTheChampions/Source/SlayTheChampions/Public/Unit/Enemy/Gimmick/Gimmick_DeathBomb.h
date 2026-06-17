// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/Enemy/GimmickComponent.h"
#include "Gimmick_DeathBomb.generated.h"

/**
 * 피해를 누적하고 죽으면 그 피해를 입히는 기믹
 */
UCLASS(ClassGroup = (Unit), meta = (BlueprintSpawnableComponent))
class SLAYTHECHAMPIONS_API UGimmick_DeathBomb : public UGimmickComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Gimmick|Bomb")
	int32 AccumulatedDamage = 0;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Gimmick|Bomb")
	FText DeathAnnounce;

protected:
	virtual void HandleHPChanged(int32 OldHP, int32 NewHP) override;
	virtual void HandleOwnerDied(AUnit* Unit) override;
};
