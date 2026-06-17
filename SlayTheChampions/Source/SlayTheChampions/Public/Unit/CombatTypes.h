// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Card/CardDataTypes.h"
#include "CombatTypes.generated.h"

class AUnit;

// 팀 소속
UENUM(BlueprintType)
enum class ETeam : uint8
{
    Ally    UMETA(DisplayName = "Ally"),
    Enemy   UMETA(DisplayName = "Enemy"),
    Neutral UMETA(DisplayName = "Neutral")
};

// 적 등급
UENUM(BlueprintType)
enum class EEnemyTier : uint8
{
    Normal  UMETA(DisplayName = "Normal"),
    Elite   UMETA(DisplayName = "Elite"),
    Boss    UMETA(DisplayName = "Boss"),
    Unkown  UMETA(DisplayName = "Unknown")
};

/**
 * EIntentKind
 * 적 행동 의도 종류. IntentComponent UI 표시 및 ExecuteEnemyAction 분기에 사용.
 * NoAttack=0 고정 (기본값 = 아무것도 안 함).
 */
UENUM(BlueprintType)
enum class EIntentKind : uint8
{
    NoAttack = 0 UMETA(DisplayName = "NoAttack"),  // 이번 턴 행동 없음 (충전, 대기 등)
    Attack       UMETA(DisplayName = "Attack"),
    Defend       UMETA(DisplayName = "Defend"),
    Buff         UMETA(DisplayName = "Buff"),
    Debuff       UMETA(DisplayName = "Debuff"),
    Shield       UMETA(DisplayName = "Shield"),    // 기믹 전용 쉴드 (GimmickComponent 사용)
    Unknown      UMETA(DisplayName = "Unknown"),
    Question     UMETA(DisplayName = "Question")   // 랜덤/미확정 행동 표시용
};

/**
 * FIntent
 * IntentComponent가 보유하는 "이번 턴 적 행동 예고" 데이터.
 * DrawPhase에 PlanNextAction()으로 채워지고, UI가 읽어 머리 위에 표시한다.
 */
USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FIntent
{
    GENERATED_BODY()

    // ── 패턴이 결정하는 의도 (모든 적 공통) ──────────────────────────────
    UPROPERTY(BlueprintReadOnly) EIntentKind Kind  = EIntentKind::Unknown;
    UPROPERTY(BlueprintReadOnly) int32       Value = 0;      // 예상 데미지 or 보호막량
    UPROPERTY(BlueprintReadOnly) int32       Hits  = 1;
    UPROPERTY(BlueprintReadOnly) ETargetType TargetType = ETargetType::SingleEnemy;

    // 해결된 단일 대상. 광역(AllEnemies/AllAllies)일 때는 무시되고 null일 수 있음
    UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AUnit> Target;
    UPROPERTY(BlueprintReadOnly) FText DisplayText;

    // ── 이 행동에 딸린 상태효과 (None이면 효과 없음) ─────────────────────
    // CombatManager가 StatusEffectComponent::SetEffectValue(Type, Value)로 전달
    UPROPERTY(BlueprintReadOnly) uint8 EffectType    = 0;   // EEffectType 값 (None=0)
    UPROPERTY(BlueprintReadOnly) int32 EffectValue   = 0;
    UPROPERTY(BlueprintReadOnly) int32 EffectDuration = 0;

    // ── 보스 기믹 경고 (보스 전용, 선택) ─────────────────────────────────
    UPROPERTY(BlueprintReadOnly) bool  bHasGimmickWarning = false;  // 경고 아이콘 표시용
    UPROPERTY(BlueprintReadOnly) FText GimmickWarningText;
};

/**
 * EPatternMode
 * 적 행동 패턴 순서 결정 방식.
 */
UENUM(BlueprintType)
enum class EPatternMode : uint8
{
    Sequential UMETA(DisplayName = "Sequential"), // 순차적으로 반복
    Weighted   UMETA(DisplayName = "Weighted")    // 가중치 기반 랜덤
};

/**
 * FEnemyAction
 * EnemyPatternData 배열의 한 항목. NPCBrainComponent가 선택해 PendingAction에 저장.
 */
USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEnemyAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) EIntentKind IntentKind = EIntentKind::Attack;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32       Value      = 0;   // 데미지 or 보호막량
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32       Hits       = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ETargetType TargetType = ETargetType::SingleEnemy;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float       Weight     = 1.f; // Weighted 모드에서만 사용
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText       DisplayName;
};

/**
 * EGimmickTrigger
 * GimmickComponent 페이즈 발동 조건.
 */
UENUM(BlueprintType)
enum class EGimmickTrigger : uint8
{
    HPThresholdBelow UMETA(DisplayName = "HP Below Threshold"), // HP가 X% 이하
    TurnCountReached UMETA(DisplayName = "Turn Count Reached"), // N턴 경과
    OnDamaged        UMETA(DisplayName = "On Damaged")          // 피격 시
};

/**
 * FGimmickPhase
 * 보스/엘리트의 페이즈 전환 조건과 연출 데이터.
 */
USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FGimmickPhase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName           PhaseName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EGimmickTrigger Trigger      = EGimmickTrigger::HPThresholdBelow;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float           TriggerValue = 0.5f; // HP는 0~1 비율, Turn이면 몇 턴
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText           AnnounceText;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool            bOneShot     = true; // true면 한 번만 발동
};

/**
 * EElementType
 *
 * 마법사가 사용하는 원소 종류
 * FieldElement에서 현재 필드에 깔린 원소를 추적하는데 사용
 */
UENUM(BlueprintType)
enum class ECardElementType : uint8
{
    None    UMETA(DisplayName = "None"),
    Fire    UMETA(DisplayName = "Fire"),
    Ice     UMETA(DisplayName = "Ice"),
    Thunder UMETA(DisplayName = "Thunder")
};