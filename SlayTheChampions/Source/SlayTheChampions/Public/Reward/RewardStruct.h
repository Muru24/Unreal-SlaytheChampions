// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RewardStruct.generated.h"

UENUM(BlueprintType)
/*보상 종류*/
enum class ERewardTypes : uint8
{
	None =0,
	NomalCard,
	RareCard,
	LegendCard,
	Gold,
	Potion,
	Relic
};

USTRUCT(BlueprintType)
/*보상 데이터*/
struct FRewardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERewardTypes RewardType = ERewardTypes::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardId;
};

UENUM(BlueprintType)
/* 골드 획득량 */
enum class EGoldAmount : uint8
{
	None = 0,
	Min_Gold = 10,
	Nomal_Gold = 30,
	Elite_Gold = 50,
	Boss_Gold = 100,
};

UENUM(BlueprintType)
/* 유물 확률 */
enum class ERelicChance : uint8
{
	None = 0,
	Nomal_Relic = 60,
	Rare_Relic = 30,
	Legend_Relic = 10,
};

UENUM(BlueprintType)
/* 카드 확률 */
enum class ECardChance : uint8
{
	None = 0,
	Nomal_Card = 60,
	Rare_Card = 30,
	Legend_Card = 10,
};

UENUM(BlueprintType)
/* 포션 확률 */
enum class EPotionChance : uint8
{
	None = 0,
	Nomal_Potion = 80,
	Rare_Potion = 20,
};

namespace PotionRewardChances
{
	const float Normal = 40.f;
	const float Elite = 52.5f;
	const float Boss = 60.0f;
}
