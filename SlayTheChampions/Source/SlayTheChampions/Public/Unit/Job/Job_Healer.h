// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/Job/JobDetail.h"
#include "Job_Healer.generated.h"

/**
 * 회복(Card.Healer.Heal) 또는 보호(Card.Healer.Shield)카드를 사용하면
 * HealStack이 쌓인다.
 * HealStack>= HealStackThreshold(기본3)이되면 대형회복준비(bHealReady) 상태
 * 다음 회복/보호 카드 사용시 대형회복후 스택 초기화
 * 
 * [GameTag]
 * Card.Healer.Heal
 * Card.Healer.Sheild
 * Card.Healer.Buff
 */
UCLASS()
class SLAYTHECHAMPIONS_API UJob_Healer : public UJobDetail
{
	GENERATED_BODY()
	
public:
	//에디터 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Healer")
	int32 HealStackThreshold = 3;

	//대형회복
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Healer")
	int32 HealMultiplier = 2;
	
	//런타임 상태 접근
	//현재 힐 누적상태
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Healer")
	int32 GetHealStack() const { return HealStack; }

	//대형버프 준비여부
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Healer")
	bool IsHealReady() const { return bHealReady; }

	//힐러 인터페이스
	UFUNCTION(BlueprintCallable, Category = "Healer")
	int32 ModifyHealAmount(FGameplayTag CardTag, int32 BaseHeal);

	//JobDetail인터페이스
	virtual void Initialize_Implementation(UActorComponent* OwnerComponent) override;

	virtual void OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue) override;

	virtual float ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage) override;

	virtual void OnTurnStart_Implementation() override;
	virtual void OnTurnEnd_Implementation() override;
private:
	UPROPERTY()
	int32 HealStack = 0;

	UPROPERTY()
	bool bHealReady = false;

	/** GameplayTag 캐시 */
	FGameplayTag TagHeal;
	FGameplayTag TagShield;
	FGameplayTag TagBuff;
};
