#include "CombatKernel/CardComboLibrary.h"
#include "CombatKernel/CombatManager.h"
#include "Card/CardSubsystem.h"
#include "Card/CardDataTypes.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

// ── enum → 한글 라벨 (족보/라이브 텍스트 전용. enum DisplayName은 영어 유지) ──
namespace
{
	FText KoCardType(ECardType T)
	{
		switch (T)
		{
			case ECardType::Attack:  return FText::FromString(TEXT("공격"));
			case ECardType::Defense: return FText::FromString(TEXT("방어"));
			case ECardType::Skill:   return FText::FromString(TEXT("스킬"));
			case ECardType::Buff:    return FText::FromString(TEXT("버프"));
			case ECardType::AOE:     return FText::FromString(TEXT("광역"));
			case ECardType::Status:  return FText::FromString(TEXT("상태"));
			default:                 return FText::FromString(TEXT("?"));
		}
	}

	FText KoCardRarity(ECardRarity R)
	{
		switch (R)
		{
			case ECardRarity::Normal:    return FText::FromString(TEXT("일반"));
			case ECardRarity::Rare:      return FText::FromString(TEXT("희귀"));
			case ECardRarity::Legendary: return FText::FromString(TEXT("전설"));
			case ECardRarity::Status:    return FText::FromString(TEXT("상태"));
			default:                     return FText::FromString(TEXT("?"));
		}
	}

	FText KoJobClass(EJobClass J)
	{
		switch (J)
		{
			case EJobClass::Any:     return FText::FromString(TEXT("공용"));
			case EJobClass::Warrior: return FText::FromString(TEXT("전사"));
			case EJobClass::Mage:    return FText::FromString(TEXT("마법사"));
			case EJobClass::Healer:  return FText::FromString(TEXT("힐러"));
			default:                 return FText::FromString(TEXT("?"));
		}
	}

	FText KoTargetType(EEffectTargetType T)
	{
		switch (T)
		{
			case EEffectTargetType::UseCardDefault: return FText::FromString(TEXT("카드 기본"));
			case EEffectTargetType::SingleEnemy:    return FText::FromString(TEXT("단일 적"));
			case EEffectTargetType::AllEnemies:     return FText::FromString(TEXT("적 전체"));
			case EEffectTargetType::Self:           return FText::FromString(TEXT("자신"));
			case EEffectTargetType::SingleAlly:     return FText::FromString(TEXT("단일 아군"));
			case EEffectTargetType::AllAllies:      return FText::FromString(TEXT("아군 전체"));
			default:                                return FText::FromString(TEXT("?"));
		}
	}

	FText KoEffectType(EEffectType E)
	{
		switch (E)
		{
			case EEffectType::Shield:            return FText::FromString(TEXT("방어막"));
			case EEffectType::Heal:              return FText::FromString(TEXT("회복"));
			case EEffectType::MaxHPUp:           return FText::FromString(TEXT("최대 HP 증가"));
			case EEffectType::DrawCard:          return FText::FromString(TEXT("카드 뽑기"));
			case EEffectType::GainEnergy:        return FText::FromString(TEXT("에너지 획득"));
			case EEffectType::Buff_AttackUp:     return FText::FromString(TEXT("공격력 증가"));
			case EEffectType::Buff_DefenseUp:    return FText::FromString(TEXT("방어력 증가"));
			case EEffectType::Buff_Regen:        return FText::FromString(TEXT("재생"));
			case EEffectType::Debuff_Weak:       return FText::FromString(TEXT("약화"));
			case EEffectType::Debuff_Vulnerable: return FText::FromString(TEXT("취약"));
			case EEffectType::Debuff_Burn:       return FText::FromString(TEXT("화상"));
			case EEffectType::Debuff_Frail:      return FText::FromString(TEXT("손상"));
			case EEffectType::Debuff_Bleed:      return FText::FromString(TEXT("출혈"));
			default:
				// 매핑 안 된 값은 영어 DisplayName 폴백
				return StaticEnum<EEffectType>()->GetDisplayNameTextByValue(static_cast<int64>(E));
		}
	}

	// WorldContextObject로 CardSubsystem 획득
	UCardSubsystem* GetCardSubsystem(const UObject* WCO)
	{
		if (!WCO || !GEngine) return nullptr;
		UWorld* W = GEngine->GetWorldFromContextObject(WCO, EGetWorldErrorMode::ReturnNull);
		if (!W) return nullptr;
		UGameInstance* GI = W->GetGameInstance();
		return GI ? GI->GetSubsystem<UCardSubsystem>() : nullptr;
	}

	// 카드 ID → 카드 이름 (CardSubsystem 경유). 못 찾으면 ID 문자열 폴백
	FText CardNameFromID(UCardSubsystem* CS, FName CardID)
	{
		if (CS)
		{
			FName Row = CS->GetRowNameByCardID(CardID);
			if (Row.IsNone()) Row = CardID;
			if (const FCardDataRow* R = CS->GetCard(Row))
				if (!R->CardName.IsEmpty())
					return R->CardName;
		}
		return FText::FromName(CardID);
	}
}

void UCardComboLibrary::GetComboRules(UDataTable* Table, TArray<FCardComboRuleRow>& OutRules)
{
	OutRules.Reset();
	if (!Table) return;

	TArray<FCardComboRuleRow*> Rows;
	Table->GetAllRows<FCardComboRuleRow>(TEXT("CardComboLibrary"), Rows);
	for (const FCardComboRuleRow* Row : Rows)
		if (Row) OutRules.Add(*Row);
}

FText UCardComboLibrary::DescribeComboCondition(const FCardComboCondition& Condition, UObject* WorldContextObject)
{
	UCardSubsystem* CS = GetCardSubsystem(WorldContextObject);

	// ── 지정 카드 조합 ──────────────────────────────────────────────
	if (Condition.MatchKey == ECardComboMatchKey::SpecificCardSet)
	{
		TArray<FString> Names;
		for (const FName& Id : Condition.RequiredCardIDs)
			if (!Id.IsNone()) Names.Add(CardNameFromID(CS, Id).ToString());

		if (Names.Num() == 0)
			return FText::FromString(TEXT("(카드 미지정)"));

		const FString Joined = FString::Join(Names, Condition.bRequireOrder ? TEXT(" → ") : TEXT(" + "));
		return FText::FromString(Condition.bRequireOrder
			? FString::Printf(TEXT("%s 순서"), *Joined)
			: FString::Printf(TEXT("%s 조합"), *Joined));
	}

	const int32 N = Condition.RequiredCount;

	// ── 지정값 N연속 ────────────────────────────────────────────────
	if (Condition.bRequireSpecificValue)
	{
		FString Value;
		switch (Condition.MatchKey)
		{
			case ECardComboMatchKey::CardID:
				Value = CardNameFromID(CS, Condition.SpecificCardID).ToString();
				break;
			case ECardComboMatchKey::CardType:
				Value = KoCardType(Condition.SpecificCardType).ToString();
				break;
			case ECardComboMatchKey::Rarity:
				Value = KoCardRarity(Condition.SpecificRarity).ToString();
				break;
			case ECardComboMatchKey::JobClass:
				Value = KoJobClass(Condition.SpecificJob).ToString();
				break;
			default:
				break;
		}
		return FText::FromString(FString::Printf(TEXT("%s %d연속"), *Value, N));
	}

	// ── 같은 X N연속 ────────────────────────────────────────────────
	FString Kind;
	switch (Condition.MatchKey)
	{
		case ECardComboMatchKey::CardID:   Kind = TEXT("같은 카드");   break;
		case ECardComboMatchKey::CardType: Kind = TEXT("같은 타입");   break;
		case ECardComboMatchKey::Rarity:   Kind = TEXT("같은 희귀도"); break;
		case ECardComboMatchKey::JobClass: Kind = TEXT("같은 직업");   break;
		default: break;
	}
	return FText::FromString(FString::Printf(TEXT("%s %d연속"), *Kind, N));
}

FText UCardComboLibrary::DescribeComboReward(const FCardComboReward& Reward)
{
	TArray<FString> Parts;

	if (Reward.BonusDamage > 0)
	{
		const FString Tgt = KoTargetType(Reward.DamageTarget).ToString();
		Parts.Add(FString::Printf(TEXT("%s %d 데미지"), *Tgt, Reward.BonusDamage));
	}

	for (const FCardEffect& Effect : Reward.Effects)
	{
		if (Effect.EffectType == EEffectType::None || Effect.Value <= 0) continue;
		const FString Eff = KoEffectType(Effect.EffectType).ToString();
		Parts.Add(FString::Printf(TEXT("%s %d"), *Eff, Effect.Value));
	}

	if (Parts.Num() == 0)
		return FText::FromString(TEXT("(효과 없음)"));

	return FText::FromString(FString::Join(Parts, TEXT(" + ")));
}

// ── GetComboProgress 내부 헬퍼 ──────────────────────────────────────────
namespace
{
	FString ProgressKeyOf(const FCardDataRow& C, ECardComboMatchKey K)
	{
		switch (K)
		{
			case ECardComboMatchKey::CardID:   return C.CardID.ToString();
			case ECardComboMatchKey::CardType: return FString::FromInt(static_cast<int32>(C.CardType));
			case ECardComboMatchKey::Rarity:   return FString::FromInt(static_cast<int32>(C.Rarity));
			case ECardComboMatchKey::JobClass: return FString::FromInt(static_cast<int32>(C.RequiredClass));
			default:                           return FString();
		}
	}

	bool MatchesSpecific(const FCardDataRow& C, const FCardComboCondition& Cond)
	{
		switch (Cond.MatchKey)
		{
			case ECardComboMatchKey::CardID:   return C.CardID == Cond.SpecificCardID;
			case ECardComboMatchKey::CardType: return C.CardType == Cond.SpecificCardType;
			case ECardComboMatchKey::Rarity:   return C.Rarity == Cond.SpecificRarity;
			case ECardComboMatchKey::JobClass: return C.RequiredClass == Cond.SpecificJob;
			default:                           return false;
		}
	}

	// 같은 X 비지정 모드에서 '마지막 카드의 키'를 라벨로
	FText KeyLabel(UCardSubsystem* CS, const FCardDataRow& C, ECardComboMatchKey K)
	{
		switch (K)
		{
			case ECardComboMatchKey::CardID:   return CardNameFromID(CS, C.CardID);
			case ECardComboMatchKey::CardType: return KoCardType(C.CardType);
			case ECardComboMatchKey::Rarity:   return KoCardRarity(C.Rarity);
			case ECardComboMatchKey::JobClass: return KoJobClass(C.RequiredClass);
			default:                           return FText::GetEmpty();
		}
	}

	// 콤보가 표시할 칸 수
	int32 SlotCount(const FCardComboCondition& Cond)
	{
		if (Cond.MatchKey == ECardComboMatchKey::SpecificCardSet)
		{
			int32 N = 0;
			for (const FName& Id : Cond.RequiredCardIDs)
				if (!Id.IsNone()) ++N;
			return N;
		}
		return Cond.RequiredCount;
	}

	// History의 [0, Len) 범위에서 '꼬리(마지막)' 기준 충족 칸 수
	int32 TrailingMatch(const TArray<FQueuedAction>& H, int32 Len, const FCardComboCondition& Cond)
	{
		if (Len <= 0) return 0;
		const int32 End = Len - 1;

		if (Cond.MatchKey == ECardComboMatchKey::SpecificCardSet)
		{
			TArray<FName> Req;
			for (const FName& Id : Cond.RequiredCardIDs)
				if (!Id.IsNone()) Req.Add(Id);
			if (Req.Num() == 0) return 0;
			const int32 N = Req.Num();

			if (Cond.bRequireOrder)
			{
				// 큐 접미사가 콤보 접두사와 일치하는 최대 길이
				int32 Best = 0;
				for (int32 L = 1; L <= FMath::Min(N, Len); ++L)
				{
					bool bOk = true;
					for (int32 K = 0; K < L; ++K)
						if (H[Len - L + K].Card.CardID != Req[K]) { bOk = false; break; }
					if (bOk) Best = L;
				}
				return Best;
			}

			// 순서 무관: 끝쪽 N칸 안에 요구 카드가 몇 개 들어있는지 (멀티셋 교집합)
			const int32 Take = FMath::Min(N, Len);
			TMap<FName, int32> Need; for (const FName& Id : Req) ++Need.FindOrAdd(Id);
			TMap<FName, int32> Have; for (int32 i = Len - Take; i < Len; ++i) ++Have.FindOrAdd(H[i].Card.CardID);
			int32 Matched = 0;
			for (const TPair<FName, int32>& P : Need)
				Matched += FMath::Min(P.Value, Have.FindRef(P.Key));
			return Matched;
		}

		// 같은 카드/타입/희귀도/직업: 끝에서부터 연속 길이
		const int32 N = Cond.RequiredCount;
		int32 Run = 0;
		if (Cond.bRequireSpecificValue)
		{
			for (int32 i = End; i >= 0; --i)
			{
				if (MatchesSpecific(H[i].Card, Cond)) ++Run;
				else break;
			}
		}
		else
		{
			const FString Key = ProgressKeyOf(H[End].Card, Cond.MatchKey);
			for (int32 i = End; i >= 0; --i)
			{
				if (ProgressKeyOf(H[i].Card, Cond.MatchKey) == Key) ++Run;
				else break;
			}
		}
		return FMath::Min(Run, N);
	}

	// 콤보 칸 라벨 배열 생성 (전부 bMatched=false 상태)
	TArray<FCardComboSlot> BuildSlots(UCardSubsystem* CS, const TArray<FQueuedAction>& H, int32 Len, const FCardComboCondition& Cond)
	{
		TArray<FCardComboSlot> Slots;

		if (Cond.MatchKey == ECardComboMatchKey::SpecificCardSet)
		{
			for (const FName& Id : Cond.RequiredCardIDs)
			{
				if (Id.IsNone()) continue;
				FCardComboSlot S; S.Label = CardNameFromID(CS, Id); Slots.Add(S);
			}
			return Slots;
		}

		// 같은 X — 라벨 결정
		FText Label;
		if (Cond.bRequireSpecificValue)
		{
			switch (Cond.MatchKey)
			{
				case ECardComboMatchKey::CardID:   Label = CardNameFromID(CS, Cond.SpecificCardID); break;
				case ECardComboMatchKey::CardType: Label = KoCardType(Cond.SpecificCardType); break;
				case ECardComboMatchKey::Rarity:   Label = KoCardRarity(Cond.SpecificRarity); break;
				case ECardComboMatchKey::JobClass: Label = KoJobClass(Cond.SpecificJob); break;
				default: break;
			}
		}
		else if (Len > 0)
		{
			// 비지정: 마지막 카드의 키를 라벨로 (지금 쌓고 있는 그룹)
			Label = KeyLabel(CS, H[Len - 1].Card, Cond.MatchKey);
		}

		for (int32 i = 0; i < Cond.RequiredCount; ++i)
		{
			FCardComboSlot S; S.Label = Label; Slots.Add(S);
		}
		return Slots;
	}
}

void UCardComboLibrary::GetComboProgress(ACombatManager* CombatManager, UDataTable* Table, TArray<FCardComboProgress>& OutProgress)
{
	OutProgress.Reset();
	if (!CombatManager || !Table) return;

	const TArray<FQueuedAction>& History = CombatManager->GetActionHistory();
	const int32 Len = History.Num();

	UCardSubsystem* CS = CombatManager->GetGameInstance()
		? CombatManager->GetGameInstance()->GetSubsystem<UCardSubsystem>()
		: nullptr;

	TArray<FCardComboRuleRow*> Rows;
	Table->GetAllRows<FCardComboRuleRow>(TEXT("CardComboProgress"), Rows);

	for (const FCardComboRuleRow* Rule : Rows)
	{
		if (!Rule || !Rule->bEnabled || Rule->ComboID.IsNone()) continue;

		const int32 Total = SlotCount(Rule->Condition);
		if (Total <= 0) continue;

		const int32 MatchedNow  = TrailingMatch(History, Len,     Rule->Condition);
		const int32 MatchedPrev = TrailingMatch(History, Len - 1, Rule->Condition);

		// 진행 중이 아니면 제외
		if (MatchedNow < 1) continue;

		const bool bComplete    = MatchedNow  >= Total;
		const bool bWasComplete = MatchedPrev >= Total;

		// 이미 지난 카드에 완성된 콤보는 숨김 (다음 카드 사용 시 사라지는 효과)
		if (bComplete && bWasComplete) continue;

		FCardComboProgress P;
		P.ComboID        = Rule->ComboID;
		P.DisplayName    = Rule->DisplayName;
		P.RewardText     = DescribeComboReward(Rule->Reward);
		P.MatchedCount   = MatchedNow;
		P.bJustCompleted = bComplete && !bWasComplete;
		P.Slots          = BuildSlots(CS, History, Len, Rule->Condition);

		// 앞에서부터 MatchedNow 칸을 빛나게
		for (int32 i = 0; i < P.Slots.Num() && i < MatchedNow; ++i)
			P.Slots[i].bMatched = true;

		OutProgress.Add(P);
	}
}
