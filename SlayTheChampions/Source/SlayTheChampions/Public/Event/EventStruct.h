#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Reward/RewardStruct.h"
#include "EventStruct.generated.h"

UENUM(BlueprintType)
enum class EEventSourceType : uint8
{
	Common = 0 UMETA(DisplayName = "Common"),
	Event = 1 UMETA(DisplayName = "Event"),
	None = 99 UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class EEventRequirementType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Gold = 1 UMETA(DisplayName = "Gold"),
	MinHP = 2 UMETA(DisplayName = "Min HP"),
};

UENUM(BlueprintType)
enum class EEventOutcomeType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Reward = 1 UMETA(DisplayName = "Reward"),
	LoseHP = 2 UMETA(DisplayName = "Lose HP"),
	LoseGold = 3 UMETA(DisplayName = "Lose Gold"),
};

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEventDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EventTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EventDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEventSourceType EventSourceType = EEventSourceType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Weight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanRepeat = false;
};

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEventChoiceDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ChoiceID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChoiceOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ResultText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEventRequirementType RequirementType = EEventRequirementType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequirementValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEndsEvent = true;
};

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEventOutcomeDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName OutcomeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ChoiceID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OutcomeOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEventOutcomeType OutcomeType = EEventOutcomeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERewardTypes RewardType = ERewardTypes::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RewardID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEventOutcomeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName OutcomeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ChoiceID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OutcomeOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEventOutcomeType OutcomeType = EEventOutcomeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERewardTypes RewardType = ERewardTypes::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RewardID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEventChoiceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ChoiceID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChoiceOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ResultText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEventRequirementType RequirementType = EEventRequirementType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequirementValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEndsEvent = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FEventOutcomeData> Outcomes;
};

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FEventData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EventTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EventDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEventSourceType EventSourceType = EEventSourceType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Weight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanRepeat = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FEventChoiceData> Choices;
};
