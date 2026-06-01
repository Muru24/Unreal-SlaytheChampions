// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Relic/RelicStruct.h"
#include "Potion/PotionStruct.h"
#include "ChampionStruct.generated.h"

class AUnit;

/*Champion Á¤º¸*/
USTRUCT(BlueprintType)
struct FSavePartyInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "ChampionInstance", meta = (AllowPrivateAccess = "true"))
	int32 MaxGold = 9999;

	UPROPERTY(BlueprintReadOnly, Category = "ChampionInstance", meta = (AllowPrivateAccess = "true"))
	int32 Gold =0;

	UPROPERTY(BlueprintReadOnly, Category = "ChampionInstance", meta = (AllowPrivateAccess = "true"))
	TArray<AUnit*> Champions;

	UPROPERTY(BlueprintReadOnly, Category = "ChampionInstance", meta = (AllowPrivateAccess = "true"))
	TArray<int32> Deck;

	UPROPERTY(BlueprintReadOnly, Category = "ChampionInstance", meta = (AllowPrivateAccess = "true"))
	TArray<FRelic> Relics;

	UPROPERTY(BlueprintReadOnly, Category = "ChampionInstance", meta = (AllowPrivateAccess = "true"))
	TArray<FPotionData> Potions;
public:

	FSavePartyInfo()
	{
	}

	void InitSavePartyInfo()
	{
		MaxGold = 9999;
		Gold = 0;
		Champions.Empty();
		Deck.Empty();
		Relics.Empty();
		Potions.Empty();
	}
};