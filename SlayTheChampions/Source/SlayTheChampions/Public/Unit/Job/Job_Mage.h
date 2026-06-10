// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/Job/JobDetail.h"
#include "Job_Mage.generated.h"

/**
 * 원소카드를 사용하면 FieldElement가 설정
 * 필드 원소가 설정된 상태에서 다른 원소카드를 사용시 원소반응 발생
 * ModifyCardDamage에서 반응 배율을 적용하고  FieldElement초기화
 * 광역카드는 항상 전체 적에게 적용 (TargetType은 카드 데이터가 담당)
 * 
 * [GameplayTag]
 * Card.Mage.Element.Fire
 * Card.Mage.Element.Ice
 * Card.Mage.Element.Thunder
 * Card.Mage.AOE 광역카드
 * 
 * [참고]
 * UJob_Mage* Mage = Cast<UJob_Mage>(Job->GetDetail());
 * if(Mage) {EElementType Field = Mage->GetFieldElement();}
 */


UCLASS()
class SLAYTHECHAMPIONS_API UJob_Mage : public UJobDetail
{
	GENERATED_BODY()
public:
	//에디터 설정
	//원소반응 발생시 데미지 배율
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Mage")
	int32 ReactionDamageMultiplier = 2;

	//런타임 상태 접근
	//현재 필드 원소 UI 아이콘
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mage")
	ECardElementType GetFieldElement() const { return FieldElement; }

	//원소반응 준비 여부
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mage")
	bool IsReactionReady() const { return bReactionReady; }

	//UJobDetail 인터페이스 오버라이드
	virtual void Initialize_Implementation(UActorComponent* OwnerComponent) override;

	/*
	* 원소카드 사용시 FieldElement갱신
	* 이미 필드에 원소가 있으면 bReactionReady = true
	*/
	virtual void OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue) override;

	/*
	* bReactionReady가 True이면 ReactionDamageMultiplier 곱한뒤
	* FieldElement와 bReactionReady초기화
	*/
	virtual float ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage) override;

	/** 턴 시작 시 특별 처리 없음. */
	virtual void OnTurnStart_Implementation() override;

	/** 턴 종료 시 특별 처리 없음. */
	virtual void OnTurnEnd_Implementation() override;


private:
	//현재 필드에 깔린 원소
	UPROPERTY()
	ECardElementType FieldElement = ECardElementType::None;

	//다음 원소카드 사용시 반응 적용 여부
	UPROPERTY()
	bool bReactionReady = false;

	/** GameplayTag 캐시 */
	FGameplayTag TagFire;
	FGameplayTag TagIce;
	FGameplayTag TagThunder;

	/** CardTag로부터 원소 종류를 판별한다. */
	ECardElementType ResolveElement(const FGameplayTag& CardTag) const;
};
