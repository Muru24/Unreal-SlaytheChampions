#include "Relic/RelicSubsystem.h"

#include "CombatKernel/EffectManager.h"
#include "Engine/DataTable.h"
#include "Party/PartyInstance.h"
#include "Unit/StatComponent.h"
#include "Unit/Unit.h"
#include "UObject/EnumProperty.h"

void URelicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	static const FSoftObjectPath RelicTablePath(TEXT("/Game/04_Data/RelicDataRow.RelicDataRow"));
	static const FSoftObjectPath RelicEffectTablePath(TEXT("/Game/04_Data/RelicEffectRow.RelicEffectRow"));

	if (UDataTable* LoadedRelicTable = Cast<UDataTable>(RelicTablePath.TryLoad()))
	{
		RelicInfoTable = LoadedRelicTable;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[RelicSubsystem] Failed to load relic data table: %s"), *RelicTablePath.ToString());
	}

	if (UDataTable* LoadedRelicEffectTable = Cast<UDataTable>(RelicEffectTablePath.TryLoad()))
	{
		RelicEffectsTable = LoadedRelicEffectTable;
	}

	RebuildRelicCache();
}

FSourceEffectData URelicSubsystem::MakeRelicEffectData(const FSourceEffectRow& EffectRow)
{
	FSourceEffectData EffectData;
	EffectData.Order = EffectRow.Order;
	EffectData.EffectType = EffectRow.EffectType;
	EffectData.Value = EffectRow.Value;
	EffectData.TargetScope = EffectRow.TargetScope;
	EffectData.ApplyTiming = EffectRow.ApplyTiming;
	EffectData.TriggerCondition = EffectRow.TriggerCondition;
	EffectData.TriggerValue = EffectRow.TriggerValue;
	EffectData.TriggerUsageType = EffectRow.TriggerUsageType;
	EffectData.SelectionGroup = EffectRow.SelectionGroup;
	EffectData.SelectionMode = EffectRow.SelectionMode;
	return EffectData;
}

void URelicSubsystem::FillRelicRuntimeData(const FRelicDataRow& RelicRow, TArray<FSourceEffectData>&& Effects, FRelic& OutRelicData)
{
	OutRelicData.RelicID = RelicRow.RelicID;
	OutRelicData.RelicName = RelicRow.RelicName;
	OutRelicData.Description = RelicRow.Description;
	OutRelicData.Rarity = RelicRow.Rarity;
	OutRelicData.RelicSourceType = RelicRow.RelicSourceType;
	OutRelicData.TargetScope = RelicRow.TargetScope;
	OutRelicData.ApplyTiming = RelicRow.ApplyTiming;
	OutRelicData.Effects = MoveTemp(Effects);
}

bool URelicSubsystem::GetCachedRelicData(FName InRelicID, FRelic& OutRelicData) const
{
	if (const FRelic* FoundRelic = Map_Relics.Find(InRelicID))
	{
		OutRelicData = *FoundRelic;
		return true;
	}

	return false;
}

TArray<FRelic> URelicSubsystem::GetCachedRelics() const
{
	return Relics;
}

FRelic URelicSubsystem::GetRelics(FName _RelicId)
{
	const FRelic* FoundRelic = Relics.FindByPredicate([_RelicId](const FRelic& Relic)
		{
			return Relic.RelicID == _RelicId;
		});

	return (FoundRelic != nullptr) ? *FoundRelic : FRelic();
}

FName URelicSubsystem::GetRandomShopRelic()
{
	if (ShopRelics.IsEmpty())
	{
		return NAME_None;
	}

	const int32 Index = FMath::RandRange(0, ShopRelics.Num() - 1);
	return ShopRelics[Index].RelicID;
}

FName URelicSubsystem::GetRandomCommonRelic()
{
	if (CommonRelics.IsEmpty())
	{
		return NAME_None;
	}
	
	const int32 Index = FMath::RandRange(0, CommonRelics.Num() - 1);
	return CommonRelics[Index].RelicID;
}

FName URelicSubsystem::GetRandomShopAvailableRelic()
{
	TArray<FRelic> AvailableRelics;
	AvailableRelics.Append(ShopRelics);
	AvailableRelics.Append(CommonRelics);

	if (AvailableRelics.IsEmpty())
	{
		return NAME_None;
	}

	const int32 Index = FMath::RandRange(0, AvailableRelics.Num() - 1);
	return AvailableRelics[Index].RelicID;
}

bool URelicSubsystem::TriggerRelicEffectsByTiming(const FRelic& RelicData, EEffectApplyTiming ApplyTiming, const TArray<AUnit*>& Targets)
{
	bool bAnyApplied = false;
	const TArray<FSourceEffectData> SelectedEffects = ResolveEffectSelection(RelicData.Effects);
	for (const FSourceEffectData& EffectData : SelectedEffects)
	{
		if (EffectData.ApplyTiming != ApplyTiming)
		{
			continue;
		}

		bAnyApplied |= ApplyRelicEffect(RelicData, EffectData, Targets);
	}

	return bAnyApplied;
}

bool URelicSubsystem::TriggerOwnedRelicEffectsByTiming(EEffectApplyTiming ApplyTiming, const TArray<AUnit*>& Targets)
{
	const UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	if (!PartyInstance)
	{
		return false;
	}

	bool bAnyApplied = false;
	for (const FRelic& RelicData : PartyInstance->GetPartyInfo().Relics)
	{
		bAnyApplied |= TriggerRelicEffectsByTiming(RelicData, ApplyTiming, Targets);
	}

	return bAnyApplied;
}

TArray<FRelic> URelicSubsystem::GetCachedRelicsBySource(ERelicSourceType InSourceType) const
{
	switch (InSourceType)
	{
	case ERelicSourceType::Default:
		return DefaultRelics;
	case ERelicSourceType::Shop:
		return ShopRelics;
	case ERelicSourceType::Event:
		return EventRelics;
	case ERelicSourceType::Common:
		return CommonRelics;
	default:
		return TArray<FRelic>();
	}
}

void URelicSubsystem::RebuildRelicCache()
{
	Relics.Reset();
	DefaultRelics.Reset();
	ShopRelics.Reset();
	EventRelics.Reset();
	CommonRelics.Reset();
	Map_Relics.Reset();

	if (!RelicInfoTable || !RelicEffectsTable)
	{
		return;
	}

	for (const FName& RowName : RelicInfoTable->GetRowNames())
	{
		const FRelicDataRow* RelicRow = RelicInfoTable->FindRow<FRelicDataRow>(RowName, TEXT("URelicSubsystem::RebuildRelicCache"));
		if (!RelicRow)
		{
			continue;
		}

		TArray<FSourceEffectData> Effects;
		for (const FName& EffectRowName : RelicEffectsTable->GetRowNames())
		{
			const FSourceEffectRow* EffectRow = RelicEffectsTable->FindRow<FSourceEffectRow>(EffectRowName, TEXT("URelicSubsystem::RebuildRelicCache"));
			if (!EffectRow || EffectRow->SourceID != RelicRow->RelicID)
			{
				continue;
			}

			Effects.Add(MakeRelicEffectData(*EffectRow));
		}

		Effects.Sort([](const FSourceEffectData& A, const FSourceEffectData& B)
		{
			return A.Order < B.Order;
		});

		FRelic RuntimeData;
		FillRelicRuntimeData(*RelicRow, MoveTemp(Effects), RuntimeData);
		Map_Relics.Add(RelicRow->RelicID, RuntimeData);
		Relics.Add(RuntimeData);
		AddRelicToSourceCache(RuntimeData);
	}
}

void URelicSubsystem::AddRelicToSourceCache(const FRelic& InRelicData)
{
	switch (InRelicData.RelicSourceType)
	{
	case ERelicSourceType::Default:
		DefaultRelics.Add(InRelicData);
		break;
	case ERelicSourceType::Shop:
		ShopRelics.Add(InRelicData);
		break;
	case ERelicSourceType::Event:
		EventRelics.Add(InRelicData);
		break;
	case ERelicSourceType::Common:
		CommonRelics.Add(InRelicData);
		break;
	default:
		break;
	}
}

bool URelicSubsystem::ApplyRelicEffect(const FRelic& RelicData, const FSourceEffectData& EffectData, const TArray<AUnit*>& Targets)
{
	OnRelicEffectTriggered.Broadcast(RelicData, EffectData);

	if (EffectData.EffectType == EEffectType::None || EffectData.Value <= 0)
	{
		return false;
	}

	if (EffectData.EffectType == EEffectType::LegendaryCardReward)
	{
		OnRelicCardRewardRequested.Broadcast(ERewardTypes::LegendCard, TEXT("RandomLegendCard"), EffectData.Value);
		return true;
	}

	bool bAppliedToUnit = false;
	for (AUnit* Target : Targets)
	{
		if (!IsValid(Target))
		{
			continue;
		}

		if (EffectData.EffectType == EEffectType::MaxHPUp)
		{
			if (UStatComponent* Stat = Target->GetStat())
			{
				Stat->MaxHP += EffectData.Value;
				Stat->Heal(EffectData.Value);
				bAppliedToUnit = true;
			}
			continue;
		}

		if (EffectData.EffectType == EEffectType::Damage)
		{
			UEffectManager::ProcessDamage(Target, EffectData.Value, nullptr);
			bAppliedToUnit = true;
			continue;
		}

		if (EffectData.EffectType == EEffectType::Shield)
		{
			UEffectManager::ApplyEffect(Target, EffectData.EffectType, EffectData.Value);
			bAppliedToUnit = true;
			continue;
		}

		if (static_cast<uint8>(EffectData.EffectType) >= static_cast<uint8>(EEffectType::Buff_AttackUp) &&
			static_cast<uint8>(EffectData.EffectType) < static_cast<uint8>(EEffectType::Debuff_Weak))
		{
			UEffectManager::ApplyBuff(Target, EffectData.EffectType, EffectData.Value);
			bAppliedToUnit = true;
			continue;
		}

		if (static_cast<uint8>(EffectData.EffectType) >= static_cast<uint8>(EEffectType::Debuff_Weak))
		{
			UEffectManager::ApplyDebuff(Target, EffectData.EffectType, EffectData.Value);
			bAppliedToUnit = true;
		}
	}

	// 전투/상점 전용 패시브는 실제 훅이 붙는 쪽에서 OnRelicEffectTriggered를 받아 처리한다.
	return bAppliedToUnit || Targets.IsEmpty();
}

TArray<FSourceEffectData> URelicSubsystem::ResolveEffectSelection(const TArray<FSourceEffectData>& Effects) const
{
	TArray<FSourceEffectData> ResolvedEffects;
	TMap<FName, TArray<FSourceEffectData>> RandomGroups;

	for (const FSourceEffectData& EffectData : Effects)
	{
		if (!EffectData.SelectionGroup.IsNone() && EffectData.SelectionMode == EEffectSelectionMode::RandomOne)
		{
			RandomGroups.FindOrAdd(EffectData.SelectionGroup).Add(EffectData);
			continue;
		}

		ResolvedEffects.Add(EffectData);
	}

	for (const TPair<FName, TArray<FSourceEffectData>>& Pair : RandomGroups)
	{
		if (!Pair.Value.IsEmpty())
		{
			ResolvedEffects.Add(Pair.Value[FMath::RandRange(0, Pair.Value.Num() - 1)]);
		}
	}

	ResolvedEffects.Sort([](const FSourceEffectData& A, const FSourceEffectData& B)
	{
		return A.Order < B.Order;
	});

	return ResolvedEffects;
}

