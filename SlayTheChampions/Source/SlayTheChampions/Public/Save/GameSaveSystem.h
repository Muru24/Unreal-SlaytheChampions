// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Party/ChampionStruct.h"
#include "Map/MapStruct.h"
#include "GameSaveSystem.generated.h"

/**
 * UGameSaveSystem
 *
 * 寃뚯엫 ?꾩껜 ?몄씠釉??곗씠?곕? ?대뒗 SaveGame ?대옒??
 * STCGameInstance::SaveGameData / LoadGameData 瑜??듯빐 ???濡쒕뱶.
 * ?щ’紐? "SaveData_Slot0"
 */
UCLASS()
class SLAYTHECHAMPIONS_API UGameSaveSystem : public USaveGame
{
	GENERATED_BODY()

public:
	/* ?뚮젅?댁뼱 梨뷀뵾???뺣낫 */
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FSavePartyInfo SavedChampionsInfo;

	/* Map愿?????*/
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FSaveMapInfo SavedMapInfo;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 SavedCurrentNodeIndex;

	// ???곗씠?곕뒗 UCardSaveGame ("PlayerDeckSave" ?щ’) ?먯꽌 蹂꾨룄 愿由?
};

