// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
///
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Unit/CombatTypes.h"
#include "NPCBrainComponent.generated.h"

class UEnemyPatternData;
class AUnit;

// ActionQueue와의 연결점, CombatManager가 이것을 사용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionEmitted, const FEnemyAction&, Action);

/*
* 적 유닛의 행동을 결정하고 Intent를 내보내는 컴포넌트
* [흐름]
* DrawPhase -> PlanNextAction(TargetParty, FriendlyParty)
*				PickNext(): 패턴에서 다음 FEnemyAction 선택
*				PickTarget() : PartyComponent 프리미티브로 대상 결정
*				IntentComponent::SetIntent() 갱신
* EnemyPhase -> EmitActionEvent()
*				OnActionEmitted 브로드캐스트 → CombatManager ActionQueue 에 등록
*/

UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class SLAYTHECHAMPIONS_API UNPCBrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNPCBrainComponent();

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Brain")
	UEnemyPatternData* Pattern = nullptr;

	//Sequential모드에서 다음 행동 인덱스 추적
	UPROPERTY(BlueprintReadOnly,Category = "Brain")
	int32 SequenceIndex = 0;

	//결정된 행동 EmitActionEvent에서 사용
	UPROPERTY(BlueprintReadOnly, Category = "Brain")
	FEnemyAction PendingAction;	

	/**
	* 턴 시작 시 CombatManager 가 호출 → 행동 결정 + IntentComponent 갱신.
	
	*/
	UFUNCTION(BlueprintCallable, Category = "Brain")
	void PlanNextAction(const TArray<AUnit*>& Allies, const TArray<AUnit*>& Enemies);


	//적 턴이 되면 CombatManager가 호출 ->ActionQueue로 송출
	UFUNCTION(BlueprintCallable, Category = "Brain")
	void EmitActionEvent();
	
	//기믹 컴포넌트에서 페이즈 전환시 호출
	// 추후 구현
	// GimmickComponent에서 페이즈 전환 시 호출
	/*
	UFUNCTION(BlueprintCallable, Category = "Brain")
	void SwapPattern(UEnemyPatternData* NewPattern);
	*/

	UPROPERTY(BlueprintAssignable, Category = "Brain")
	FOnActionEmitted OnActionEmitted;

private:
	FEnemyAction PickNext();
	AUnit* PickTarget(ETargetType TargetType,
		const TArray<AUnit*>& Allies,
		const TArray<AUnit*>& Enemies) const;

		
};

