#include "Potion/PotionSubsystem.h"

#include "Engine/DataTable.h"

void UPotionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	static const FSoftObjectPath PotionTablePath(TEXT("/Game/04_Data/PotionDataRow.PotionDataRow"));
	static const FSoftObjectPath PotionEffectTablePath(TEXT("/Game/04_Data/PotionEffectRow.PotionEffectRow"));

	if (UDataTable* LoadedPotionTable = Cast<UDataTable>(PotionTablePath.TryLoad()))
	{
		PotionInfoTable = LoadedPotionTable;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PotionSubsystem] Failed to load potion data table: %s"), *PotionTablePath.ToString());
	}

	if (UDataTable* LoadedPotionEffectTable = Cast<UDataTable>(PotionEffectTablePath.TryLoad()))
	{
		PotionEffectsTable = LoadedPotionEffectTable;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PotionSubsystem] Failed to load potion effect data table: %s"), *PotionEffectTablePath.ToString());
	}

	RebuildPotionCache();
}

bool UPotionSubsystem::GetCachedPotionData(FName InPotionID, FPotionData& OutPotionData) const
{
	if (const FPotionData* FoundPotion = Map_Potions.Find(InPotionID))
	{
		OutPotionData = *FoundPotion;
		return true;
	}

	return false;
}

FPotionData UPotionSubsystem::GetPotion(FName InPotionID) const
{
	if (const FPotionData* FoundPotion = Map_Potions.Find(InPotionID))
	{
		return *FoundPotion;
	}

	return FPotionData();
}

TArray<FPotionData> UPotionSubsystem::GetCachedPotions() const
{
	return Potions;
}

TArray<FPotionData> UPotionSubsystem::GetCachedPotionsBySource(EPotionSourceType InSourceType) const
{
	switch (InSourceType)
	{
	case EPotionSourceType::Common:
		return CommonPotions;
	case EPotionSourceType::Event:
		return EventPotions;
	default:
		return TArray<FPotionData>();
	}
}

FName UPotionSubsystem::GetRandomCommonPotion() const
{
	if (CommonPotions.IsEmpty())
	{
		return NAME_None;
	}

	const int32 Index = FMath::RandRange(0, CommonPotions.Num() - 1);
	return CommonPotions[Index].PotionID;
}

FName UPotionSubsystem::GetRandomEventPotion() const
{
	if (EventPotions.IsEmpty())
	{
		return NAME_None;
	}

	const int32 Index = FMath::RandRange(0, EventPotions.Num() - 1);
	return EventPotions[Index].PotionID;
}

FName UPotionSubsystem::GetRandomAnyPotion() const
{
	if (Potions.IsEmpty())
	{
		return NAME_None;
	}

	const int32 Index = FMath::RandRange(0, Potions.Num() - 1);
	return Potions[Index].PotionID;
}

void UPotionSubsystem::RebuildPotionCache()
{
	Potions.Reset();
	CommonPotions.Reset();
	EventPotions.Reset();
	Map_Potions.Reset();

	if (!PotionInfoTable || !PotionEffectsTable)
	{
		return;
	}

	for (const FName& RowName : PotionInfoTable->GetRowNames())
	{
		const FPotionDataRow* PotionRow = PotionInfoTable->FindRow<FPotionDataRow>(RowName, TEXT("UPotionSubsystem::RebuildPotionCache"));
		if (!PotionRow)
		{
			continue;
		}

		TArray<FSourceEffectData> Effects;
		for (const FName& EffectRowName : PotionEffectsTable->GetRowNames())
		{
			const FSourceEffectRow* EffectRow = PotionEffectsTable->FindRow<FSourceEffectRow>(EffectRowName, TEXT("UPotionSubsystem::RebuildPotionCache"));
			if (!EffectRow || EffectRow->SourceID != PotionRow->PotionID)
			{
				continue;
			}

			Effects.Add(MakePotionEffectData(*EffectRow));
		}

		Effects.Sort([](const FSourceEffectData& A, const FSourceEffectData& B)
		{
			return A.Order < B.Order;
		});

		FPotionData RuntimeData;
		FillPotionRuntimeData(*PotionRow, MoveTemp(Effects), RuntimeData);
		Map_Potions.Add(PotionRow->PotionID, RuntimeData);
		Potions.Add(RuntimeData);
		AddPotionToSourceCache(RuntimeData);
	}
}

void UPotionSubsystem::AddPotionToSourceCache(const FPotionData& InPotionData)
{
	switch (InPotionData.PotionSourceType)
	{
	case EPotionSourceType::Common:
		CommonPotions.Add(InPotionData);
		break;
	case EPotionSourceType::Event:
		EventPotions.Add(InPotionData);
		break;
	default:
		break;
	}
}

FSourceEffectData UPotionSubsystem::MakePotionEffectData(const FSourceEffectRow& EffectRow)
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

void UPotionSubsystem::FillPotionRuntimeData(const FPotionDataRow& PotionRow, TArray<FSourceEffectData>&& Effects, FPotionData& OutPotionData)
{
	OutPotionData.PotionID = PotionRow.PotionID;
	OutPotionData.PotionName = PotionRow.PotionName;
	OutPotionData.Description = PotionRow.Description;
	OutPotionData.Rarity = PotionRow.Rarity;
	OutPotionData.PotionSourceType = PotionRow.PotionSourceType;
	OutPotionData.UseTiming = PotionRow.UseTiming;
	OutPotionData.TargetScope = PotionRow.TargetScope;
	OutPotionData.Effects = MoveTemp(Effects);
}
