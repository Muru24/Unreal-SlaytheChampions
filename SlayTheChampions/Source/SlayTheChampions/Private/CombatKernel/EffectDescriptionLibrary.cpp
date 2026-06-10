#include "CombatKernel/EffectDescriptionLibrary.h"

// 모든 텍스트는 클래스 static 멤버 안에서만 생성한다.
// (free/익명 namespace 함수를 두지 않아 유니티 빌드 이름 충돌을 원천 차단)

FText UEffectDescriptionLibrary::GetEffectName(EEffectType Type)
{
	switch (Type)
	{
		// ── 즉발 / 수치형 ───────────────────────────────────────────────
		case EEffectType::Shield:            return FText::FromString(TEXT("방어막"));
		case EEffectType::Heal:              return FText::FromString(TEXT("회복"));
		case EEffectType::MaxHPUp:           return FText::FromString(TEXT("최대 HP 증가"));
		case EEffectType::DrawCard:          return FText::FromString(TEXT("카드 뽑기"));
		case EEffectType::GainEnergy:        return FText::FromString(TEXT("에너지 획득"));
		// ── 버프 ────────────────────────────────────────────────────────
		case EEffectType::Buff_AttackUp:     return FText::FromString(TEXT("공격력 증가"));
		case EEffectType::Buff_DefenseUp:    return FText::FromString(TEXT("방어력 증가"));
		case EEffectType::Buff_Regen:        return FText::FromString(TEXT("재생"));
		// ── 디버프 ──────────────────────────────────────────────────────
		case EEffectType::Debuff_Weak:       return FText::FromString(TEXT("약화"));
		case EEffectType::Debuff_Vulnerable: return FText::FromString(TEXT("취약"));
		case EEffectType::Debuff_Burn:       return FText::FromString(TEXT("화상"));
		case EEffectType::Debuff_Frail:      return FText::FromString(TEXT("손상"));
		case EEffectType::Debuff_Bleed:      return FText::FromString(TEXT("출혈"));
		default:
			// 매핑 안 된 값은 enum DisplayName(영어) 폴백
			return StaticEnum<EEffectType>()->GetDisplayNameTextByValue(static_cast<int64>(Type));
	}
}

FText UEffectDescriptionLibrary::GetEffectDescription(EEffectType Type)
{
	switch (Type)
	{
		// ── 즉발 / 수치형 ───────────────────────────────────────────────
		case EEffectType::Shield:
			return FText::FromString(TEXT("데미지를 흡수합니다. 턴이 시작되면 사라집니다."));
		case EEffectType::Heal:
			return FText::FromString(TEXT("체력을 회복합니다."));
		case EEffectType::MaxHPUp:
			return FText::FromString(TEXT("최대 체력이 증가합니다."));
		case EEffectType::DrawCard:
			return FText::FromString(TEXT("카드를 뽑습니다."));
		case EEffectType::GainEnergy:
			return FText::FromString(TEXT("코스트를 얻습니다."));
		// ── 버프 ────────────────────────────────────────────────────────
		case EEffectType::Buff_AttackUp:
			return FText::FromString(TEXT("주는 데미지가 스택만큼 증가합니다."));
		case EEffectType::Buff_DefenseUp:
			return FText::FromString(TEXT("얻는 방어막이 스택만큼 증가합니다."));
		case EEffectType::Buff_Regen:
			return FText::FromString(TEXT("매 턴 시작 시 스택만큼 체력을 회복하고, 스택이 1 감소합니다."));
		// ── 디버프 ──────────────────────────────────────────────────────
		case EEffectType::Debuff_Weak:
			return FText::FromString(TEXT("주는 데미지가 25% 감소합니다. 매 턴 스택이 1 감소합니다."));
		case EEffectType::Debuff_Vulnerable:
			return FText::FromString(TEXT("받는 데미지가 50% 증가합니다. 매 턴 스택이 1 감소합니다."));
		case EEffectType::Debuff_Burn:
			return FText::FromString(TEXT("매 턴 시작 시 스택만큼 데미지를 받고, 스택이 1 감소합니다."));
		case EEffectType::Debuff_Frail:
			return FText::FromString(TEXT("얻는 방어막이 25% 감소합니다. 매 턴 스택이 1 감소합니다."));
		case EEffectType::Debuff_Bleed:
			return FText::FromString(TEXT("공격받을 때마다 1 데미지를 받고 스택이 1 감소합니다. 드로우 단계마다 스택이 1 감소합니다."));
		default:
			return FText::GetEmpty();
	}
}

FEffectInfo UEffectDescriptionLibrary::GetEffectInfo(EEffectType Type)
{
	FEffectInfo Info;
	Info.DisplayName = GetEffectName(Type);
	Info.Description = GetEffectDescription(Type);
	return Info;
}
