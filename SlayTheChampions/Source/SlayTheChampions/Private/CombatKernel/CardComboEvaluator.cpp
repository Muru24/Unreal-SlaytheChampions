#include "CombatKernel/CardComboEvaluator.h"
#include "CombatKernel/CombatManager.h"
#include "CombatKernel/EffectManager.h"
#include "Card/CardUserComponent.h"
#include "Unit/Unit.h"
#include "Unit/StatComponent.h"
#include "Engine/DataTable.h"

void UCardComboEvaluator::Initialize(ACombatManager* InOwner, UDataTable* InComboTable)
{
	Owner = InOwner;
	ComboTable = InComboTable;
	MetCombos.Reset();
}

void UCardComboEvaluator::EvaluateAfterCardPlayed(int32 CasterIndex)
{
	if (!Owner || !ComboTable) return;

	const TArray<FQueuedAction>& History = Owner->GetActionHistory();

	TArray<FCardComboRuleRow*> Rows;
	ComboTable->GetAllRows<FCardComboRuleRow>(TEXT("CardComboEvaluator"), Rows);

	for (const FCardComboRuleRow* Rule : Rows)
	{
		if (!Rule || !Rule->bEnabled || Rule->ComboID.IsNone()) continue;

		const bool bMet    = IsConditionMet(History, Rule->Condition);
		const bool bWasMet = MetCombos.Contains(Rule->ComboID);

		// rising edge: 새로 충족되는 순간에만 1회 발동
		if (bMet && !bWasMet)
		{
			FireCombo(*Rule, CasterIndex);
			MetCombos.Add(Rule->ComboID);
		}
		else if (!bMet && bWasMet)
		{
			// 큐에서 밀려나 조건이 깨짐 → 다음에 다시 충족되면 재발동 허용
			MetCombos.Remove(Rule->ComboID);
		}
	}
}

bool UCardComboEvaluator::IsConditionMet(const TArray<FQueuedAction>& History, const FCardComboCondition& Cond) const
{
	// 지정 카드 ID 조합 모드
	if (Cond.MatchKey == ECardComboMatchKey::SpecificCardSet)
		return IsCardSetMet(History, Cond);

	// 같은 카드/타입/희귀도/직업 N개 모드
	return CountMatching(History, Cond) >= Cond.RequiredCount;
}

// MatchKey 기준으로 카드의 그룹 키를 문자열로 추출
static FString ComboKeyOf(const FCardDataRow& C, ECardComboMatchKey MatchKey)
{
	switch (MatchKey)
	{
		case ECardComboMatchKey::CardID:   return C.CardID.ToString();
		case ECardComboMatchKey::CardType: return FString::FromInt(static_cast<int32>(C.CardType));
		case ECardComboMatchKey::Rarity:   return FString::FromInt(static_cast<int32>(C.Rarity));
		case ECardComboMatchKey::JobClass: return FString::FromInt(static_cast<int32>(C.RequiredClass));
		default:                           return FString();
	}
}

bool UCardComboEvaluator::IsCardSetMet(const TArray<FQueuedAction>& History, const FCardComboCondition& Cond) const
{
	// None 제외한 요구 카드 목록
	TArray<FName> Req;
	for (const FName& ID : Cond.RequiredCardIDs)
		if (!ID.IsNone()) Req.Add(ID);
	if (Req.Num() == 0) return false;

	const int32 N = Req.Num();
	if (History.Num() < N) return false;

	// 콤보는 큐에서 '붙어있어야' 발동 — 길이 N의 연속 구간만 검사
	if (Cond.bRequireOrder)
	{
		// 순서 필수: 정확히 Req 순서대로 연속 등장
		for (int32 Start = 0; Start + N <= History.Num(); ++Start)
		{
			bool bMatch = true;
			for (int32 K = 0; K < N; ++K)
				if (History[Start + K].Card.CardID != Req[K]) { bMatch = false; break; }
			if (bMatch) return true;
		}
		return false;
	}

	// 순서 무관: 길이 N의 연속 구간의 카드 멀티셋이 Req 멀티셋과 정확히 일치
	TMap<FName, int32> NeedCount;
	for (const FName& ID : Req) ++NeedCount.FindOrAdd(ID);

	for (int32 Start = 0; Start + N <= History.Num(); ++Start)
	{
		TMap<FName, int32> WinCount;
		for (int32 K = 0; K < N; ++K)
			++WinCount.FindOrAdd(History[Start + K].Card.CardID);

		if (WinCount.Num() != NeedCount.Num()) continue;
		bool bEqual = true;
		for (const TPair<FName, int32>& Pair : NeedCount)
			if (WinCount.FindRef(Pair.Key) != Pair.Value) { bEqual = false; break; }
		if (bEqual) return true;
	}
	return false;
}

// 콤보는 큐에서 '붙어있어야' 발동 — 조건에 맞는 카드가 연속으로 이어진 최대 길이를 센다
int32 UCardComboEvaluator::CountMatching(const TArray<FQueuedAction>& History, const FCardComboCondition& Cond) const
{
	if (Cond.bRequireSpecificValue)
	{
		// 지정값과 일치하는 카드가 '연속으로' 몇 장인지 (최대 런)
		auto Matches = [&](const FCardDataRow& C) -> bool
		{
			switch (Cond.MatchKey)
			{
				case ECardComboMatchKey::CardID:   return C.CardID == Cond.SpecificCardID;
				case ECardComboMatchKey::CardType: return C.CardType == Cond.SpecificCardType;
				case ECardComboMatchKey::Rarity:   return C.Rarity == Cond.SpecificRarity;
				case ECardComboMatchKey::JobClass: return C.RequiredClass == Cond.SpecificJob;
				default:                           return false;
			}
		};

		int32 Best = 0, Run = 0;
		for (const FQueuedAction& Action : History)
		{
			if (Matches(Action.Card)) { ++Run; Best = FMath::Max(Best, Run); }
			else                       { Run = 0; }
		}
		return Best;
	}

	// bRequireSpecificValue=false: 인접 카드의 키가 같은 '연속' 최대 런
	int32 Best = 0, Run = 0;
	FString Prev;
	bool bHasPrev = false;
	for (const FQueuedAction& Action : History)
	{
		const FString Key = ComboKeyOf(Action.Card, Cond.MatchKey);
		if (bHasPrev && Key == Prev) ++Run;
		else                          Run = 1;
		Best = FMath::Max(Best, Run);
		Prev = Key;
		bHasPrev = true;
	}
	return Best;
}

void UCardComboEvaluator::FireCombo(const FCardComboRuleRow& Rule, int32 CasterIndex)
{
	if (!Owner) return;

	const TArray<AUnit*>& Players = Owner->GetSpawnedPlayers();
	AUnit* Caster = Players.IsValidIndex(CasterIndex) ? Players[CasterIndex] : nullptr;

	// ── 1. 추가 데미지 ──────────────────────────────────────────────
	if (Rule.Reward.BonusDamage > 0)
	{
		for (AUnit* Target : ResolveTargets(Rule.Reward.DamageTarget, CasterIndex))
		{
			if (Target && Target->IsAlive())
				UEffectManager::ProcessDamage(Target, Rule.Reward.BonusDamage, Caster);
		}
	}

	// ── 2. 버프/디버프/회복/실드/드로우 ────────────────────────────────
	for (const FCardEffect& Effect : Rule.Reward.Effects)
	{
		if (Effect.EffectType == EEffectType::None || Effect.Value <= 0) continue;

		// DrawCard: 시전자만 (타겟 무관)
		if (Effect.EffectType == EEffectType::DrawCard)
		{
			if (Caster)
				if (UCardUserComponent* CardComp = Caster->FindComponentByClass<UCardUserComponent>())
					CardComp->DrawCards(Effect.Value);
			continue;
		}

		const uint8 TypeVal = static_cast<uint8>(Effect.EffectType);
		for (AUnit* Target : ResolveTargets(Effect.TargetType, CasterIndex))
		{
			if (!Target || !Target->IsAlive()) continue;

			if (Effect.EffectType == EEffectType::Heal)
			{
				if (UStatComponent* Stat = Target->GetStat())
					Stat->Heal(Effect.Value);
				continue;
			}

			if (TypeVal >= 200)      UEffectManager::ApplyDebuff(Target, Effect.EffectType, Effect.Value);
			else if (TypeVal >= 100) UEffectManager::ApplyBuff(Target, Effect.EffectType, Effect.Value);
			else                     UEffectManager::ApplyEffect(Target, Effect.EffectType, Effect.Value);
		}
	}

	// ── 3. UI/VFX 연출용 브로드캐스트 ──────────────────────────────────
	Owner->OnComboTriggered.Broadcast(Rule.ComboID, CasterIndex);

	UE_LOG(LogTemp, Warning, TEXT("[Combo] 발동: %s (Caster=%d)"), *Rule.ComboID.ToString(), CasterIndex);
}

TArray<AUnit*> UCardComboEvaluator::ResolveTargets(EEffectTargetType TargetType, int32 CasterIndex) const
{
	if (!Owner) return {};

	const TArray<AUnit*>& Players = Owner->GetSpawnedPlayers();
	const TArray<AUnit*>& Enemies = Owner->GetSpawnedEnemies();
	AUnit* Caster = Players.IsValidIndex(CasterIndex) ? Players[CasterIndex] : nullptr;

	switch (TargetType)
	{
		case EEffectTargetType::SingleEnemy:
			if (Enemies.IsValidIndex(0)) return { Enemies[0] };
			return {};
		case EEffectTargetType::AllEnemies:
			return Enemies;
		case EEffectTargetType::Self:
			return Caster ? TArray<AUnit*>{ Caster } : TArray<AUnit*>{};
		case EEffectTargetType::SingleAlly:
			if (Players.IsValidIndex(0)) return { Players[0] };
			return {};
		case EEffectTargetType::AllAllies:
			return Players;
		case EEffectTargetType::UseCardDefault:
		default:
			// 콤보 기본 대상: 적 전체
			return Enemies;
	}
}
