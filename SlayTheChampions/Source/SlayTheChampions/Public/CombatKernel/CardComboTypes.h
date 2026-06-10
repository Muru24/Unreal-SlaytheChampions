#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Card/CardDataTypes.h"      // ECardType, ECardRarity, EJobClass
#include "CombatKernel/EffectTypes.h" // FCardEffect, EEffectTargetType
#include "CardComboTypes.generated.h"

/**
 * 카드 히스토리 콤보 시스템 — 데이터 타입.
 *
 * 전투 중 사용한 카드 히스토리(ACombatManager::ActionQueue)의 **최근 N장(기본 10)** 을 보고,
 * 디자이너가 DT_CardCombos 에 정의한 조건이 충족되면 보상 효과를 발동한다.
 *
 * 설계 결정 (COMBO_SYSTEM_DESIGN.md):
 *  - 집계 범위: 전투 전체 누적이되 **큐 최근 10장 한정**(슬라이딩 윈도우). 별도 윈도우 enum 없음.
 *  - 파티 공용: ActionQueue는 모든 플레이어 카드 합산.
 *  - 효과 대상: 데미지/디버프는 적 전체, 버프는 아군 전체 (FCardEffect 개별 TargetType로 지정).
 *  - 중복 발동 허용: 한 카드로 여러 룰이 동시 충족되면 전부 발동.
 */

/** 무엇을 기준으로 "같다"고 볼지 — 히스토리 카드 그룹화 키 */
UENUM(BlueprintType)
enum class ECardComboMatchKey : uint8
{
	CardID          UMETA(DisplayName = "Same Card (CardID)"),       // 완전히 같은 카드 N개
	CardType        UMETA(DisplayName = "Same Type"),                // Attack / Defense / Skill ...
	Rarity          UMETA(DisplayName = "Same Rarity"),
	JobClass        UMETA(DisplayName = "Same Job"),
	SpecificCardSet UMETA(DisplayName = "Specific Card Set (IDs)"),  // 지정한 카드 ID들의 조합
};

/**
 * FCardComboCondition
 * 콤보 발동 조건. 최근 10장 윈도우 안에서 평가된다.
 */
USTRUCT(BlueprintType)
struct FCardComboCondition
{
	GENERATED_BODY()

	// 그룹화/비교 기준
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition")
	ECardComboMatchKey MatchKey = ECardComboMatchKey::CardType;

	// false: "같은 것 아무거나 N개" (예: 같은 타입 아무거나 3개)
	// true : 아래 지정값과 일치하는 카드 N개 (예: 'Attack' 타입 정확히 3개)
	// (MatchKey==SpecificCardSet 모드에서는 무시됨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "MatchKey != ECardComboMatchKey::SpecificCardSet"))
	bool bRequireSpecificValue = false;

	// bRequireSpecificValue=true일 때, MatchKey에 해당하는 칸만 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "bRequireSpecificValue"))
	FName SpecificCardID;                                   // MatchKey==CardID

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "bRequireSpecificValue"))
	ECardType SpecificCardType = ECardType::Attack;         // MatchKey==CardType

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "bRequireSpecificValue"))
	ECardRarity SpecificRarity = ECardRarity::Normal;       // MatchKey==Rarity

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "bRequireSpecificValue"))
	EJobClass SpecificJob = EJobClass::Any;                  // MatchKey==JobClass

	// 발동에 필요한 개수 — 조건에 맞는 카드가 큐에서 '연속(인접)'으로 N장 이어져야 발동.
	// (SpecificCardSet 모드에서는 무시됨)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (ClampMin = "2", ClampMax = "10",
			EditCondition = "MatchKey != ECardComboMatchKey::SpecificCardSet"))
	int32 RequiredCount = 3;

	// ── MatchKey==SpecificCardSet 전용 ──────────────────────────────────
	// 이 카드 ID들이 큐에서 '연속(인접)' 구간으로 모두 등장하면 발동 (사이에 다른 카드 없이).
	// 예: [101, 102, 103] → 세 카드가 붙어서 사용되면 발동 / [101, 101] → 101 두 장 연속
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "MatchKey == ECardComboMatchKey::SpecificCardSet"))
	TArray<FName> RequiredCardIDs;

	// true면 RequiredCardIDs가 적힌 '순서대로' 연속 등장해야 발동 (101→102→103)
	// false면 순서 무관, 단 연속 구간 안에 모두 있어야 발동
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Condition",
		meta = (EditCondition = "MatchKey == ECardComboMatchKey::SpecificCardSet"))
	bool bRequireOrder = false;
};

/**
 * FCardComboReward
 * 콤보 발동 시 적용할 보상. 기존 FCardEffect를 재사용한다.
 */
USTRUCT(BlueprintType)
struct FCardComboReward
{
	GENERATED_BODY()

	// 추가 데미지 (데미지는 EEffectType이 아니라 별도 경로라 분리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Reward",
		meta = (ClampMin = "0"))
	int32 BonusDamage = 0;

	// BonusDamage 대상 (기본: 적 전체)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Reward")
	EEffectTargetType DamageTarget = EEffectTargetType::AllEnemies;

	// 버프/디버프/회복/실드 등 — 각 효과의 TargetType으로 대상 지정 (버프=AllAllies, 디버프=AllEnemies)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Reward")
	TArray<FCardEffect> Effects;
};

/**
 * FCardComboRuleRow
 * DT_CardCombos DataTable의 Row Struct. 디자이너가 행 단위로 콤보를 정의한다.
 */
USTRUCT(BlueprintType)
struct FCardComboRuleRow : public FTableRowBase
{
	GENERATED_BODY()

	// 식별 ──────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Identity")
	FName ComboID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Identity")
	FText Description;

	// 규칙 ──────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Rule")
	FCardComboCondition Condition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Rule")
	FCardComboReward Reward;

	// 밸런싱 중 임시 차단용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Rule")
	bool bEnabled = true;
};

// ── 라이브 진행도 위젯(WBP_ComboLive)용 뷰 구조체 ──────────────────────────

/**
 * FCardComboSlot
 * 콤보 한 칸의 표시 데이터. (카드 이름 + 현재 큐 꼬리로 충족됐는지)
 */
USTRUCT(BlueprintType)
struct FCardComboSlot
{
	GENERATED_BODY()

	// 칸에 표시할 텍스트 (카드 이름 / 타입명 등)
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	FText Label;

	// 현재 큐 꼬리 기준으로 이 칸이 충족됐는지 (true면 빛나게)
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	bool bMatched = false;
};

/**
 * FCardComboProgress
 * 진행 중(또는 이번 카드에 막 완성된) 콤보 1개의 표시 데이터.
 * GetComboProgress가 표시할 것만 걸러서 반환한다.
 */
USTRUCT(BlueprintType)
struct FCardComboProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	FName ComboID;

	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	FText RewardText;

	// 콤보 전체 칸 + 각 칸의 매칭 여부
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	TArray<FCardComboSlot> Slots;

	// 현재 충족된 칸 수
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	int32 MatchedCount = 0;

	// 이번 카드에 막 완성됐는지 (true면 강조 — 다음 카드 사용 시 목록에서 사라짐)
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	bool bJustCompleted = false;
};
