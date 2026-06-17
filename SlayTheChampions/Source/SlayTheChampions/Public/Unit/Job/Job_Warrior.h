// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/Job/JobDetail.h"
#include "Job_Warrior.generated.h"

/**
 * UJob_Warrior
 * 검사 로직
 * 
 * [기획 요약]
 * 카드를 사용하면 EnhanceStack이 쌓인다.
 * EnhanceStack >= EnhanceStackThreshold(기본3)이 되면 다음 공격이 강화된다.
 * ->ModifyCardDamage 에서 데미지 배율 적용 후 스택을 0으로 리셋
 * 
 * [GameplayTag]
 * Card.Warrior.Enhance ->강화카드
 * Card.Warrior.Attack ->공격카드 (강화적용대상)
 * 
 * [팀원참고]
 * UJobComponent* Job = Unit->FindComponentByClass<UJobComponent>();
 * UJob_Warrior* Warrior = Cast<UJob_Warrior>(Job->GetDetail());
 * if(Warrior){int32 Stack = Warrior->GetEnhanceStack();}
 */
UCLASS()
class SLAYTHECHAMPIONS_API UJob_Warrior : public UJobDetail
{
	GENERATED_BODY()
	
public:
	//에디터 설정

	//강화 공격에 필요한 강화 스택수 기본값은 3
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Warrior")
	int32 EnhanceStackThreshold = 3;

	// 강화 공격시 데미지 배율
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Warrior")
	int32 DamageMultiplier = 2;

	//런타임 상태
	//현재 스택 상태
	UFUNCTION(BlueprintCallable, BlueprintPure,Category = "Warrior")
	int32 GetEnhanceStack() const { return EnhanceStack; }

	//다음 공격이 강화인지 여부
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Warrior")
	bool IsEnhanceAttackReady() const { return bEnhanceAttackReady; }

	//UJobDetail 인터페이스 override
	virtual void Initialize_Implementation(UActorComponent* OwnerComponent) override;

	/*
	 * Card.Warrior.Enhance 태그인 경우 EnhanceStack을 증가시킨다.
	 * 스택이 Threshold에 도달하면 bPromotedAttackReady = true.
	 */
	virtual void OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue) override;

	/*
	 * Card.Warrior.Attack 태그이고 bPromotedAttackReady가 true이면
	 * 데미지에 PromotedDamageMultiplier를 곱하고 스택을 초기화한다.
	 */
	virtual float ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage) override;

	/* 턴 시작 시 특별 처리 없음 (확장 여지). */
	virtual void OnTurnStart_Implementation() override;

	/* 턴 종료 시 특별 처리 없음 (확장 여지). */
	virtual void OnTurnEnd_Implementation() override;
private:
	//현재 강화 스택
	UPROPERTY()
	int32 EnhanceStack;

	//다음 공격 강화여부
	UPROPERTY()
	bool bEnhanceAttackReady = false;


	FGameplayTag TagEnhance;
	FGameplayTag TagAttack;
};
