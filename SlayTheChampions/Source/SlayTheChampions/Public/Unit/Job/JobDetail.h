// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Unit/CombatTypes.h"
#include "Card/CardDataTypes.h"
#include "JobDetail.generated.h"

/**
 * UJobDetail
 * 직업별 실제 로직을 담고있는 UObject기반 클래스
 * UJobComponent가 외부 인터페이스를 받아서 이 객체에 위임한다.
 * 
 * [설계의도]
 * ActorComponent가 아니기 때문에 Unit액터 컴포넌트 슬롯을 차지하지 않는다.
 * UJobComponent 하나만 Unit에 붙이고, 에디터 드롭다운으로선택하면
 * BeginPlay에서 이 객체의 서브클래스가 동적생성된다.
 * 직업을 추가할때 이 클래스를 상속하여 Override하면 된다.
 * 
 * [사용흐름]
 * UJobComponent::BeginPlay()
 *	->JobClass(EJobClass) 기반으로 UJobDetail 서브클래스 NewObject
 *	->Detail = NewObject<UJobDetail_Warrior>(...)
 * UJobComponent::OnCardPlayed()
 *	->Detail->OnCardPlayed(...)
 * 
 */
UCLASS(Abstract, BlueprintType,Blueprintable)
class SLAYTHECHAMPIONS_API UJobDetail : public UObject
{
	GENERATED_BODY()
	
public:

	/**
	 * 직업 로직을 초기화한다.
	 * UJobComponent::BeginPlay()에서 NewObject 직후 한 번 호출된다.
	 * @param OwnerComponent 이 Detail을 소유하는 JobComponent (Outer와 같음)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Job")
	void Initialize(UActorComponent* OwnerComponent);
	virtual void Initialize_Implementation(UActorComponent* OwnerComponent) {}

	/**
	 * 카드가 사용될 때 호출된다.
	 * 직업 고유 자원(예: 검사의 강화 스택) 갱신에 사용한다.
	 * @param CardTag  사용된 카드의 GameplayTag
	 * @param CardValue 카드 기본 수치 (데미지/방어량 등)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Job")
	void OnCardPlayed(FGameplayTag CardTag, int32 CardValue);
	virtual void OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue) {}

	/**
	 * 카드 데미지 계산 시점에 호출된다.
	 * 직업 고유 계수(예: 강화 스택 배율)를 반영해 최종 데미지를 반환한다.
	 * @param CardTag    사용된 카드의 GameplayTag
	 * @param BaseDamage EffectManager가 전달한 기본 데미지
	 * @return 직업 보정이 적용된 최종 데미지
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Job")
	float ModifyCardDamage(FGameplayTag CardTag, float BaseDamage);
	virtual float ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage)
	{
		return BaseDamage;
	}

	/** 턴 시작 시 호출된다. (재생/쿨타임/자원 초기화 등) */
	UFUNCTION(BlueprintNativeEvent, Category = "Job")
	void OnTurnStart();
	virtual void OnTurnStart_Implementation() {}

	/** 턴 종료 시 호출된다. (스택 감소/만료 처리 등) */
	UFUNCTION(BlueprintNativeEvent, Category = "Job")
	void OnTurnEnd();
	virtual void OnTurnEnd_Implementation() {}
};
