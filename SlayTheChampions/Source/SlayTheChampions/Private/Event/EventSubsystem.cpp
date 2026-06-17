#include "Event/EventSubsystem.h"

#include "Engine/DataTable.h"
#include "Party/PartyInstance.h"
#include "Potion/PotionSubsystem.h"
#include "Relic/RelicSubsystem.h"

void UEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	static const FSoftObjectPath EventTablePath(TEXT("/Game/04_Data/EventDataRow.EventDataRow"));
	static const FSoftObjectPath EventChoiceTablePath(TEXT("/Game/04_Data/EventChoiceDataRow.EventChoiceDataRow"));
	static const FSoftObjectPath EventOutcomeTablePath(TEXT("/Game/04_Data/EventOutcomeDataRow.EventOutcomeDataRow"));

	if (UDataTable* LoadedEventTable = Cast<UDataTable>(EventTablePath.TryLoad()))
	{
		EventInfoTable = LoadedEventTable;
	}

	if (UDataTable* LoadedEventChoiceTable = Cast<UDataTable>(EventChoiceTablePath.TryLoad()))
	{
		EventChoiceTable = LoadedEventChoiceTable;
	}

	if (UDataTable* LoadedEventOutcomeTable = Cast<UDataTable>(EventOutcomeTablePath.TryLoad()))
	{
		EventOutcomeTable = LoadedEventOutcomeTable;
	}

	RebuildEventCache();
}

void UEventSubsystem::RebuildEventCache()
{
	Events.Reset();
	CommonEvents.Reset();
	SpecialEvents.Reset();
	Map_Events.Reset();
	Map_Choices.Reset();

	if (!EventInfoTable || !EventChoiceTable || !EventOutcomeTable)
	{
		return;
	}

	TMap<FName, TArray<FEventOutcomeData>> OutcomesByChoiceID;
	for (const FName& RowName : EventOutcomeTable->GetRowNames())
	{
		const FEventOutcomeDataRow* OutcomeRow = EventOutcomeTable->FindRow<FEventOutcomeDataRow>(RowName, TEXT("UEventSubsystem::RebuildEventCache"));
		if (!OutcomeRow || OutcomeRow->ChoiceID.IsNone())
		{
			continue;
		}

		OutcomesByChoiceID.FindOrAdd(OutcomeRow->ChoiceID).Add(MakeOutcomeData(*OutcomeRow));
	}

	for (TPair<FName, TArray<FEventOutcomeData>>& Pair : OutcomesByChoiceID)
	{
		Pair.Value.Sort([](const FEventOutcomeData& A, const FEventOutcomeData& B)
		{
			return A.OutcomeOrder < B.OutcomeOrder;
		});
	}

	TMap<FName, TArray<FEventChoiceData>> ChoicesByEventID;
	for (const FName& RowName : EventChoiceTable->GetRowNames())
	{
		const FEventChoiceDataRow* ChoiceRow = EventChoiceTable->FindRow<FEventChoiceDataRow>(RowName, TEXT("UEventSubsystem::RebuildEventCache"));
		if (!ChoiceRow || ChoiceRow->EventID.IsNone())
		{
			continue;
		}

		FEventChoiceData ChoiceData = MakeChoiceData(*ChoiceRow);
		if (TArray<FEventOutcomeData>* FoundOutcomes = OutcomesByChoiceID.Find(ChoiceData.ChoiceID))
		{
			ChoiceData.Outcomes = *FoundOutcomes;
		}

		Map_Choices.Add(ChoiceData.ChoiceID, ChoiceData);
		ChoicesByEventID.FindOrAdd(ChoiceData.EventID).Add(ChoiceData);
	}

	for (TPair<FName, TArray<FEventChoiceData>>& Pair : ChoicesByEventID)
	{
		Pair.Value.Sort([](const FEventChoiceData& A, const FEventChoiceData& B)
		{
			return A.ChoiceOrder < B.ChoiceOrder;
		});
	}

	for (const FName& RowName : EventInfoTable->GetRowNames())
	{
		const FEventDataRow* EventRow = EventInfoTable->FindRow<FEventDataRow>(RowName, TEXT("UEventSubsystem::RebuildEventCache"));
		if (!EventRow || EventRow->EventID.IsNone())
		{
			continue;
		}

		FEventData EventData = MakeEventData(*EventRow);
		if (TArray<FEventChoiceData>* FoundChoices = ChoicesByEventID.Find(EventData.EventID))
		{
			EventData.Choices = *FoundChoices;
		}

		Map_Events.Add(EventData.EventID, EventData);
		Events.Add(EventData);
		AddEventToSourceCache(EventData);
	}
}

bool UEventSubsystem::GetCachedEventData(FName InEventID, FEventData& OutEventData) const
{
	if (const FEventData* FoundEvent = Map_Events.Find(InEventID))
	{
		OutEventData = *FoundEvent;
		return true;
	}

	return false;
}

bool UEventSubsystem::GetCachedChoiceData(FName InChoiceID, FEventChoiceData& OutChoiceData) const
{
	if (const FEventChoiceData* FoundChoice = Map_Choices.Find(InChoiceID))
	{
		OutChoiceData = *FoundChoice;
		return true;
	}

	return false;
}

TArray<FEventChoiceData> UEventSubsystem::GetEventChoices(FName InEventID) const
{
	if (const FEventData* FoundEvent = Map_Events.Find(InEventID))
	{
		return FoundEvent->Choices;
	}

	return TArray<FEventChoiceData>();
}

TArray<FEventOutcomeData> UEventSubsystem::GetChoiceOutcomes(FName InChoiceID) const
{
	if (const FEventChoiceData* FoundChoice = Map_Choices.Find(InChoiceID))
	{
		return FoundChoice->Outcomes;
	}

	return TArray<FEventOutcomeData>();
}

FName UEventSubsystem::GetRandomEvent()
{
	return PickWeightedEventID(Events);
}

FName UEventSubsystem::GetRandomEventBySource(EEventSourceType InSourceType)
{
	switch (InSourceType)
	{
	case EEventSourceType::Common:
		return PickWeightedEventID(CommonEvents);
	case EEventSourceType::Event:
		return PickWeightedEventID(SpecialEvents);
	default:
		return NAME_None;
	}
}

bool UEventSubsystem::ApplyEventChoice(FName InChoiceID)
{
	const FEventChoiceData* FoundChoice = Map_Choices.Find(InChoiceID);
	if (!FoundChoice)
	{
		return false;
	}

	bool bAllApplied = true;
	for (const FEventOutcomeData& OutcomeData : FoundChoice->Outcomes)
	{
		bAllApplied &= ApplyOutcome(OutcomeData);
		OnEventOutcomeApplied.Broadcast(OutcomeData);
	}

	if (FoundChoice->bEndsEvent && !FoundChoice->EventID.IsNone())
	{
		if (const FEventData* FoundEvent = Map_Events.Find(FoundChoice->EventID))
		{
			if (!FoundEvent->bCanRepeat)
			{
				UsedEventIDs.Add(FoundChoice->EventID);
			}
		}
	}

	return bAllApplied;
}

void UEventSubsystem::ResetUsedEvents()
{
	UsedEventIDs.Reset();
}

FEventData UEventSubsystem::MakeEventData(const FEventDataRow& EventRow)
{
	FEventData EventData;
	EventData.EventID = EventRow.EventID;
	EventData.EventTitle = EventRow.EventTitle;
	EventData.EventDescription = EventRow.EventDescription;
	EventData.EventSourceType = EventRow.EventSourceType;
	EventData.Weight = EventRow.Weight;
	EventData.bCanRepeat = EventRow.bCanRepeat;
	return EventData;
}

FEventChoiceData UEventSubsystem::MakeChoiceData(const FEventChoiceDataRow& ChoiceRow)
{
	FEventChoiceData ChoiceData;
	ChoiceData.ChoiceID = ChoiceRow.ChoiceID;
	ChoiceData.EventID = ChoiceRow.EventID;
	ChoiceData.ChoiceOrder = ChoiceRow.ChoiceOrder;
	ChoiceData.ChoiceText = ChoiceRow.ChoiceText;
	ChoiceData.ResultText = ChoiceRow.ResultText;
	ChoiceData.RequirementType = ChoiceRow.RequirementType;
	ChoiceData.RequirementValue = ChoiceRow.RequirementValue;
	ChoiceData.bEndsEvent = ChoiceRow.bEndsEvent;
	return ChoiceData;
}

FEventOutcomeData UEventSubsystem::MakeOutcomeData(const FEventOutcomeDataRow& OutcomeRow)
{
	FEventOutcomeData OutcomeData;
	OutcomeData.OutcomeID = OutcomeRow.OutcomeID;
	OutcomeData.ChoiceID = OutcomeRow.ChoiceID;
	OutcomeData.OutcomeOrder = OutcomeRow.OutcomeOrder;
	OutcomeData.OutcomeType = OutcomeRow.OutcomeType;
	OutcomeData.RewardType = OutcomeRow.RewardType;
	OutcomeData.RewardID = OutcomeRow.RewardID;
	OutcomeData.Amount = OutcomeRow.Amount;
	return OutcomeData;
}

void UEventSubsystem::AddEventToSourceCache(const FEventData& InEventData)
{
	switch (InEventData.EventSourceType)
	{
	case EEventSourceType::Common:
		CommonEvents.Add(InEventData);
		break;
	case EEventSourceType::Event:
		SpecialEvents.Add(InEventData);
		break;
	default:
		break;
	}
}

FName UEventSubsystem::PickWeightedEventID(const TArray<FEventData>& EventPool)
{
	int32 TotalWeight = 0;
	for (const FEventData& EventData : EventPool)
	{
		if (!EventData.bCanRepeat && UsedEventIDs.Contains(EventData.EventID))
		{
			continue;
		}

		TotalWeight += FMath::Max(0, EventData.Weight);
	}

	if (TotalWeight <= 0)
	{
		return NAME_None;
	}

	int32 Roll = FMath::RandRange(1, TotalWeight);
	for (const FEventData& EventData : EventPool)
	{
		if (!EventData.bCanRepeat && UsedEventIDs.Contains(EventData.EventID))
		{
			continue;
		}

		Roll -= FMath::Max(0, EventData.Weight);
		if (Roll <= 0)
		{
			return EventData.EventID;
		}
	}

	return NAME_None;
}

bool UEventSubsystem::ApplyOutcome(const FEventOutcomeData& OutcomeData)
{
	switch (OutcomeData.OutcomeType)
	{
	case EEventOutcomeType::None:
		return true;
	case EEventOutcomeType::LoseHP:
		OnEventHpLossRequested.Broadcast(OutcomeData.Amount);
		return true;
	case EEventOutcomeType::LoseGold:
		if (UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr)
		{
			PartyInstance->UseGold(OutcomeData.Amount);
			return true;
		}
		return false;
	case EEventOutcomeType::Reward:
		return ApplyRewardOutcome(OutcomeData);
	default:
		return false;
	}
}

bool UEventSubsystem::ApplyRewardOutcome(const FEventOutcomeData& OutcomeData)
{
	UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	if (!PartyInstance)
	{
		return false;
	}

	switch (OutcomeData.RewardType)
	{
	case ERewardTypes::Gold:
		PartyInstance->AddGold(OutcomeData.Amount);
		return true;
	case ERewardTypes::Relic:
	{
		URelicSubsystem* RelicSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URelicSubsystem>() : nullptr;
		if (!RelicSubsystem)
		{
			return false;
		}

		FRelic RelicData;
		const FName RelicID = ResolveRelicRewardID(OutcomeData.RewardID);
		if (!RelicSubsystem->GetCachedRelicData(RelicID, RelicData))
		{
			return false;
		}

		const int32 Count = FMath::Max(1, OutcomeData.Amount);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			PartyInstance->AddRelic(RelicData);
		}
		return true;
	}
	case ERewardTypes::Potion:
	{
		const UPotionSubsystem* PotionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPotionSubsystem>() : nullptr;
		if (!PotionSubsystem)
		{
			return false;
		}

		FPotionData PotionData;
		const FName PotionID = ResolvePotionRewardID(OutcomeData.RewardID);
		if (!PotionSubsystem->GetCachedPotionData(PotionID, PotionData))
		{
			return false;
		}

		const int32 Count = FMath::Max(1, OutcomeData.Amount);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			PartyInstance->AddPotion(PotionData);
		}
		return true;
	}
	case ERewardTypes::NomalCard:
	case ERewardTypes::RareCard:
	case ERewardTypes::LegendCard:
		OnEventCardRewardRequested.Broadcast(OutcomeData.RewardType, OutcomeData.RewardID, OutcomeData.Amount);
		return true;
	default:
		return false;
	}
}

FName UEventSubsystem::ResolveRelicRewardID(FName RewardID) const
{
	if (RewardID.IsNone())
	{
		return NAME_None;
	}

	URelicSubsystem* RelicSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URelicSubsystem>() : nullptr;
	if (!RelicSubsystem)
	{
		return RewardID;
	}

	if (RewardID == TEXT("RandomEventRelic"))
	{
		const TArray<FRelic> EventRelics = RelicSubsystem->GetEventRelics();
		return EventRelics.IsEmpty() ? NAME_None : EventRelics[FMath::RandRange(0, EventRelics.Num() - 1)].RelicID;
	}

	if (RewardID == TEXT("RandomCommonRelic"))
	{
		return RelicSubsystem->GetRandomCommonRelic();
	}

	if (RewardID == TEXT("RandomShopRelic"))
	{
		return RelicSubsystem->GetRandomShopRelic();
	}

	if (RewardID == TEXT("RandomRelic"))
	{
		const TArray<FRelic> AllRelics = RelicSubsystem->GetCachedRelics();
		return AllRelics.IsEmpty() ? NAME_None : AllRelics[FMath::RandRange(0, AllRelics.Num() - 1)].RelicID;
	}

	return RewardID;
}

FName UEventSubsystem::ResolvePotionRewardID(FName RewardID) const
{
	if (RewardID.IsNone())
	{
		return NAME_None;
	}

	const UPotionSubsystem* PotionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPotionSubsystem>() : nullptr;
	if (!PotionSubsystem)
	{
		return RewardID;
	}

	if (RewardID == TEXT("RandomPotion"))
	{
		return PotionSubsystem->GetRandomAnyPotion();
	}

	if (RewardID == TEXT("RandomCommonPotion"))
	{
		return PotionSubsystem->GetRandomCommonPotion();
	}

	if (RewardID == TEXT("RandomEventPotion"))
	{
		return PotionSubsystem->GetRandomEventPotion();
	}

	return RewardID;
}
