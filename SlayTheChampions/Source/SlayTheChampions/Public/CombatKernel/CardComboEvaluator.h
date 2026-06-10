#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CombatKernel/CardComboTypes.h"   // FCardComboRuleRow/Condition/Reward, EEffectTargetType
#include "CardComboEvaluator.generated.h"

class ACombatManager;
class UDataTable;
class AUnit;
struct FQueuedAction;

/**
 * UCardComboEvaluator
 * 카드 히스토리 콤보 평가기. ACombatManager가 소유한다.
 *
 * 동작:
 *  - 카드가 ActionQueue(최근 10장)에 추가될 때마다 EvaluateAfterCardPlayed() 호출.
 *  - DT_CardCombos의 각 룰에 대해 큐 안에서 조건 충족 개수를 세고,
 *    조건이 *새로 충족되는 순간(rising edge)* 1회 보상을 발동한다.
 *  - 큐에서 카드가 밀려나 조건이 깨졌다가 다시 충족되면 재발동.
 *  - 한 카드로 여러 룰이 동시 충족되면 전부 발동(중복 허용).
 *
 * 수명: CombatManager와 함께 생성/소멸 → 전투 시작 시 자동으로 상태 초기화.
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardComboEvaluator : public UObject
{
	GENERATED_BODY()

public:
	// Owner(CombatManager)와 콤보 DataTable 주입
	void Initialize(ACombatManager* InOwner, UDataTable* InComboTable);

	// 카드가 큐에 추가된 직후 호출 — 전체 큐(최근 10장)를 평가하고 충족 콤보를 발동
	void EvaluateAfterCardPlayed(int32 CasterIndex);

private:
	UPROPERTY()
	ACombatManager* Owner = nullptr;

	UPROPERTY()
	UDataTable* ComboTable = nullptr;

	// rising edge 추적 — 현재 충족 중인 콤보 ID 집합
	TSet<FName> MetCombos;

	// 조건 충족 여부 — MatchKey에 따라 카운트 방식 / 카드셋 방식으로 분기
	bool IsConditionMet(const TArray<FQueuedAction>& History, const FCardComboCondition& Cond) const;

	// 큐 안에서 조건에 맞는 카드 수 집계 (Same Card/Type/Rarity/Job 모드)
	//  - bRequireSpecificValue=true: 지정값과 일치하는 카드 수
	//  - false: MatchKey 기준 최대 동일 그룹 크기
	int32 CountMatching(const TArray<FQueuedAction>& History, const FCardComboCondition& Cond) const;

	// SpecificCardSet 모드 — RequiredCardIDs가 큐에 모두 존재하는지 (bRequireOrder면 연속 순서)
	bool IsCardSetMet(const TArray<FQueuedAction>& History, const FCardComboCondition& Cond) const;

	// 보상 적용 (BonusDamage + Effects) + OnComboTriggered 브로드캐스트
	void FireCombo(const FCardComboRuleRow& Rule, int32 CasterIndex);

	// 콤보 효과 대상 해석 (콤보엔 TargetOverride 없음 → SingleEnemy=0번 적, SingleAlly=0번 아군)
	TArray<AUnit*> ResolveTargets(EEffectTargetType TargetType, int32 CasterIndex) const;
};
