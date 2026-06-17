#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Event/EventStruct.h"
#include "EventSubsystem.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventOutcomeApplied, const FEventOutcomeData&, OutcomeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventHpLossRequested, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEventCardRewardRequested, ERewardTypes, RewardType, FName, RewardID, int32, Amount);

UCLASS()
class SLAYTHECHAMPIONS_API UEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<UDataTable> EventInfoTable;

	UPROPERTY()
	TObjectPtr<UDataTable> EventChoiceTable;

	UPROPERTY()
	TObjectPtr<UDataTable> EventOutcomeTable;

	UPROPERTY()
	TArray<FEventData> Events;

	UPROPERTY()
	TArray<FEventData> CommonEvents;

	UPROPERTY()
	TArray<FEventData> SpecialEvents;

	TMap<FName, FEventData> Map_Events;
	TMap<FName, FEventChoiceData> Map_Choices;
	TSet<FName> UsedEventIDs;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnEventOutcomeApplied OnEventOutcomeApplied;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnEventHpLossRequested OnEventHpLossRequested;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnEventCardRewardRequested OnEventCardRewardRequested;

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	void RebuildEventCache();

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	bool GetCachedEventData(FName InEventID, FEventData& OutEventData) const;

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	bool GetCachedChoiceData(FName InChoiceID, FEventChoiceData& OutChoiceData) const;

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	TArray<FEventData> GetCachedEvents() const { return Events; }

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	TArray<FEventChoiceData> GetEventChoices(FName InEventID) const;

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	TArray<FEventOutcomeData> GetChoiceOutcomes(FName InChoiceID) const;

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	FName GetRandomEvent();

	UFUNCTION(BlueprintCallable, Category = "Event|Query")
	FName GetRandomEventBySource(EEventSourceType InSourceType);

	UFUNCTION(BlueprintCallable, Category = "Event|Apply")
	bool ApplyEventChoice(FName InChoiceID);

	UFUNCTION(BlueprintCallable, Category = "Event|Runtime")
	void ResetUsedEvents();

private:
	static FEventData MakeEventData(const FEventDataRow& EventRow);
	static FEventChoiceData MakeChoiceData(const FEventChoiceDataRow& ChoiceRow);
	static FEventOutcomeData MakeOutcomeData(const FEventOutcomeDataRow& OutcomeRow);

	void AddEventToSourceCache(const FEventData& InEventData);
	FName PickWeightedEventID(const TArray<FEventData>& EventPool);
	bool ApplyOutcome(const FEventOutcomeData& OutcomeData);
	bool ApplyRewardOutcome(const FEventOutcomeData& OutcomeData);

	FName ResolveRelicRewardID(FName RewardID) const;
	FName ResolvePotionRewardID(FName RewardID) const;
};
