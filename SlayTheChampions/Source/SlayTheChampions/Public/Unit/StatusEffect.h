// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StatusEffect.generated.h"

class AUnit;
/**
 * 여기에서 상태이상 관련 베이스 클래스
 */
UCLASS()
class SLAYTHECHAMPIONS_API UStatusEffect : public UObject
{
	GENERATED_BODY()

public:
	// 중첩 수
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	int32 Stacks = 0;

	// 지속시간 -1 infinite 0> decrease every turn
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	int32 Duration = -1;

	// 소유자
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	AUnit* Owner = nullptr;

	virtual void OnApplied() {}
	virtual void OnRemoved() {}
	virtual void OnTurnEnd(); // Duration 감소 처리
	
	bool IsExpired() const { return Duration == 0 || Stacks <= 0; }
};


// 상태이상 종류 목록
// 1. 방어도 증가
// 2. 공격력 증가 디버프
// 3. 매 턴 데미지 반사
// 4. 최대 체력 감소 디버프
