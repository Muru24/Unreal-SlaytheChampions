#include "CombatKernel/EffectManager.h"
#include "Unit/Unit.h"
#include "Unit/StatComponent.h"
#include "Unit/StatusEffectComponent.h"

// ── 효과 부여 ────────────────────────────────────────────────────────────────

void UEffectManager::ApplyEffect(AUnit* Target, EEffectType Type, int32 Value)
{
	if (!Target || Value <= 0) return;

	UStatusEffectComponent* SEC = Target->FindComponentByClass<UStatusEffectComponent>();
	if (!SEC) return;

	if (Type == EEffectType::Shield)
	{
		// 카드 표시와 동일 계산 공유 (DefenseUp +, Frail ×0.75)
		const int32 FinalBlock = ModifyBlockGain(Target, Value);
		SEC->SetEffectValue(EEffectType::Shield, SEC->GetEffectValue(EEffectType::Shield) + FinalBlock);

		UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s Shield +%d (total: %d)"),
			*Target->GetName(), FinalBlock, SEC->GetEffectValue(EEffectType::Shield));
		return;
	}

	// 버프/디버프(100~299)는 999 상한
	const int32 NewVal = SEC->GetEffectValue(Type) + Value;
	const uint8 TypeVal = static_cast<uint8>(Type);
	SEC->SetEffectValue(Type, TypeVal >= 100 ? FMath::Min(NewVal, 999) : NewVal);
}

// 버프(100~199) 스택 누적 부여
void UEffectManager::ApplyBuff(AUnit* Target, EEffectType BuffType, int32 Stacks)
{
	const uint8 Val = static_cast<uint8>(BuffType);
	if (Val < 100 || Val > 199)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EffectManager] ApplyBuff: invalid type %d"), Val);
		return;
	}

	ApplyEffect(Target, BuffType, Stacks);

	UStatusEffectComponent* SEC = Target->FindComponentByClass<UStatusEffectComponent>();
	const int32 After = SEC ? SEC->GetEffectValue(BuffType) : 0;

	UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s Buff[%d] +%d (total: %d)"),
		*Target->GetName(), Val, Stacks, After);
}

// 디버프(200~299) 스택 누적 부여
void UEffectManager::ApplyDebuff(AUnit* Target, EEffectType DebuffType, int32 Stacks)
{
	const uint8 Val = static_cast<uint8>(DebuffType);
	if (Val < 200)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EffectManager] ApplyDebuff: invalid type %d"), Val);
		return;
	}

	ApplyEffect(Target, DebuffType, Stacks);

	UStatusEffectComponent* SEC = Target->FindComponentByClass<UStatusEffectComponent>();
	const int32 After = SEC ? SEC->GetEffectValue(DebuffType) : 0;

	UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s Debuff[%d] +%d (total: %d)"),
		*Target->GetName(), Val, Stacks, After);
}

// ── 턴 틱 ────────────────────────────────────────────────────────────────────

// 턴 시작 시 유닛의 버프/디버프 처리
// - Regen : 스택 수만큼 HP 회복 후 스택 1 감소
// - Burn  : 스택 수만큼 데미지 후 스택 1 감소
// - Weak / Vulnerable / Frail : 스택 1 감소
// - AttackUp / DefenseUp : 변경 없음 (영구)
void UEffectManager::TickEffects(AUnit* Unit)
{
	if (!Unit) return;

	UStatusEffectComponent* SEC = Unit->FindComponentByClass<UStatusEffectComponent>();
	if (!SEC) return;

	UStatComponent* Stat = Unit->GetStat();

	// Regen: 회복 후 스택 감소
	const int32 Regen = SEC->GetEffectValue(EEffectType::Buff_Regen);
	if (Regen > 0)
	{
		if (Stat) Stat->Heal(Regen);
		SEC->SetEffectValue(EEffectType::Buff_Regen, Regen - 1);
		UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s Regen 발동: +%d HP (스택 %d -> %d)"),
			*Unit->GetName(), Regen, Regen, Regen - 1);
	}

	// Burn: 데미지 후 스택 감소
	const int32 Burn = SEC->GetEffectValue(EEffectType::Debuff_Burn);
	if (Burn > 0)
	{
		// 도트 데미지 — 피격(Bleed) 트리거 없이 처리 (Bleed는 아래 시간제 틱에서 별도 1 감소)
		ProcessDamage(Unit, Burn, nullptr, /*bIsAttack=*/false);
		if (Unit->IsAlive())
		{
			SEC->SetEffectValue(EEffectType::Debuff_Burn, Burn - 1);
			UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s Burn 발동: -%d HP (스택 %d -> %d)"),
				*Unit->GetName(), Burn, Burn, Burn - 1);
		}
	}

	// 시간제 스택 감소
	auto TickLog = [&](EEffectType Type, const TCHAR* Name)
	{
		const int32 Val = SEC->GetEffectValue(Type);
		if (Val > 0)
		{
			SEC->SetEffectValue(Type, Val - 1);
			UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s %s 감소: %d -> %d"),
				*Unit->GetName(), Name, Val, Val - 1);
		}
	};

	TickLog(EEffectType::Debuff_Weak,       TEXT("Weak"));
	TickLog(EEffectType::Debuff_Vulnerable, TEXT("Vulnerable"));
	TickLog(EEffectType::Debuff_Frail,      TEXT("Frail"));
	TickLog(EEffectType::Debuff_Bleed,      TEXT("Bleed"));
}

// ── 표시/계산 공용 보정 ───────────────────────────────────────────────────────

int32 UEffectManager::ModifyOutgoingDamage(AUnit* Attacker, int32 BaseDamage)
{
	if (BaseDamage <= 0) return 0;
	UStatusEffectComponent* SEC = Attacker ? Attacker->FindComponentByClass<UStatusEffectComponent>() : nullptr;
	if (!SEC) return BaseDamage;

	int32 D = BaseDamage + SEC->GetEffectValue(EEffectType::Buff_AttackUp);
	if (SEC->GetEffectValue(EEffectType::Debuff_Weak) > 0)
		D = FMath::FloorToInt(D * 0.75f);
	return FMath::Max(0, D);
}

int32 UEffectManager::ModifyBlockGain(AUnit* Owner, int32 BaseBlock)
{
	if (BaseBlock <= 0) return 0;
	UStatusEffectComponent* SEC = Owner ? Owner->FindComponentByClass<UStatusEffectComponent>() : nullptr;
	if (!SEC) return BaseBlock;

	int32 B = BaseBlock + SEC->GetEffectValue(EEffectType::Buff_DefenseUp);
	if (SEC->GetEffectValue(EEffectType::Debuff_Frail) > 0)
		B = FMath::FloorToInt(B * 0.75f);
	return FMath::Max(0, B);
}

// ── 데미지 파이프라인 ─────────────────────────────────────────────────────────

// 순서: 공격자 AttackUp(+) → Weak(-25%) → 대상 Vulnerable(+50%) → Shield 흡수 → HP 감소
void UEffectManager::ProcessDamage(AUnit* Target, int32 Damage, AUnit* Attacker, bool bIsAttack)
{
	if (!Target || Damage <= 0) return;

	// 공격자 측 보정 (카드 표시와 동일 계산 공유)
	Damage = ModifyOutgoingDamage(Attacker, Damage);

	if (Damage <= 0) return;

	UStatusEffectComponent* SEC = Target->FindComponentByClass<UStatusEffectComponent>();
	if (SEC)
	{
		if (SEC->GetEffectValue(EEffectType::Debuff_Vulnerable) > 0)
			Damage = FMath::CeilToInt(Damage * 1.5f);

		const int32 Shield = SEC->GetEffectValue(EEffectType::Shield);
		if (Shield > 0)
		{
			const int32 Absorbed = FMath::Min(Shield, Damage);
			SEC->SetEffectValue(EEffectType::Shield, Shield - Absorbed);
			Damage -= Absorbed;
			UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s Shield 흡수: %d (남은 Shield: %d, 잔여 데미지: %d)"),
				*Target->GetName(), Absorbed, Shield - Absorbed, Damage);
		}
	}

	// 실드 통과분이 남아있을 때만 HP 감소 (전부 흡수되면 HP는 그대로)
	if (Damage > 0)
	{
		if (UStatComponent* Stat = Target->GetStat())
			Stat->TakeDamage(Damage, Attacker);
	}

	// ── 출혈(Bleed) 발동 ─────────────────────────────────────────────────────
	// 피격(실제 공격) 1회마다 스택 1 감소 + 1 데미지 (Shield 흡수 여부와 무관, 직접 HP 감소로 Shield 관통)
	// 도트(Burn 등 bIsAttack=false)는 피격으로 치지 않아 Bleed를 발동하지 않는다
	if (bIsAttack && SEC && Target->IsAlive())
	{
		const int32 Bleed = SEC->GetEffectValue(EEffectType::Debuff_Bleed);
		if (Bleed > 0)
		{
			SEC->SetEffectValue(EEffectType::Debuff_Bleed, Bleed - 1);
			if (UStatComponent* BleedStat = Target->GetStat())
				BleedStat->TakeDamage(1, nullptr);
			UE_LOG(LogTemp, Log, TEXT("[EffectManager] %s 출혈 발동: -1 HP (스택 %d -> %d)"),
				*Target->GetName(), Bleed, Bleed - 1);
		}
	}
}
