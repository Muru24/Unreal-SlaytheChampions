#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Unit/StatusEffectComponent.h"
#include "EffectManager.generated.h"

class AUnit;

/**
 * UEffectManager
 * 전투 중 효과 부여 및 데미지 처리를 담당하는 정적 유틸 클래스.
 *
 * 데미지 파이프라인:
 *   공격자 AttackUp(+) → Weak(-25%) → 대상 Vulnerable(+50%) → Shield 흡수 → HP 감소
 *
 * Shield 부여 파이프라인:
 *   기본 Block + DefenseUp(+) → Frail(-25%) → Shield 누적
 */
UCLASS()
class SLAYTHECHAMPIONS_API UEffectManager : public UObject
{
	GENERATED_BODY()

public:
	// 수치형 효과를 대상에게 누적 부여
	// Type == Shield이면 DefenseUp 보너스 · Frail 패널티 자동 반영
	UFUNCTION(BlueprintCallable, Category = "Effect")
	static void ApplyEffect(AUnit* Target, EEffectType Type, int32 Value);

	// 버프(100~199) 스택 누적 부여
	UFUNCTION(BlueprintCallable, Category = "Effect")
	static void ApplyBuff(AUnit* Target, EEffectType BuffType, int32 Stacks);

	// 디버프(200~299) 스택 누적 부여
	UFUNCTION(BlueprintCallable, Category = "Effect")
	static void ApplyDebuff(AUnit* Target, EEffectType DebuffType, int32 Stacks);

	// 턴 시작 틱: Regen 회복, Burn 데미지, 시간제 스택 1 감소
	// AttackUp / DefenseUp은 감소하지 않음
	UFUNCTION(BlueprintCallable, Category = "Effect")
	static void TickEffects(AUnit* Unit);

	// 데미지 파이프라인: 공격자 버프/디버프 → 대상 Vulnerable → Shield 흡수 → HP 감소
	// bIsAttack=false면 도트(Burn 등)로 간주 — 피격 트리거(Bleed)를 발동하지 않는다
	static void ProcessDamage(AUnit* Target, int32 Damage, AUnit* Attacker, bool bIsAttack = true);

	// ── 표시용 계산 (카드 UI가 실제 파이프라인과 같은 값을 보이도록 공용 사용) ──
	// 공격자 측 데미지 보정: + AttackUp, Weak면 ×0.75 (대상 Vulnerable은 타겟 의존이라 제외)
	UFUNCTION(BlueprintPure, Category = "Effect")
	static int32 ModifyOutgoingDamage(AUnit* Attacker, int32 BaseDamage);

	// 방어도 획득 보정: + DefenseUp, Frail이면 ×0.75
	UFUNCTION(BlueprintPure, Category = "Effect")
	static int32 ModifyBlockGain(AUnit* Owner, int32 BaseBlock);
};
