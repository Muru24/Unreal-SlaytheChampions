#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Potion/PotionStruct.h"
#include "PotionSubsystem.generated.h"

class UDataTable;

UCLASS()
class SLAYTHECHAMPIONS_API UPotionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<UDataTable> PotionInfoTable;

	UPROPERTY()
	TObjectPtr<UDataTable> PotionEffectsTable;

	UPROPERTY()
	TArray<FPotionData> Potions;

	UPROPERTY()
	TArray<FPotionData> CommonPotions;

	UPROPERTY()
	TArray<FPotionData> EventPotions;

	TMap<FName, FPotionData> Map_Potions;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	bool GetCachedPotionData(FName InPotionID, FPotionData& OutPotionData) const;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	FPotionData GetPotion(FName InPotionID) const;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	TArray<FPotionData> GetCachedPotions() const;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	TArray<FPotionData> GetCachedPotionsBySource(EPotionSourceType InSourceType) const;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	TArray<FPotionData> GetCommonPotions() const { return CommonPotions; }

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	TArray<FPotionData> GetEventPotions() const { return EventPotions; }

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	FName GetRandomCommonPotion() const;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	FName GetRandomEventPotion() const;

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	FName GetRandomSpecialPotion() const { return GetRandomEventPotion(); }

	UFUNCTION(BlueprintCallable, Category = "Potion|Query")
	FName GetRandomAnyPotion() const;

private:
	void RebuildPotionCache();

	void AddPotionToSourceCache(const FPotionData& InPotionData);

	static FSourceEffectData MakePotionEffectData(const FSourceEffectRow& EffectRow);

	static void FillPotionRuntimeData(const FPotionDataRow& PotionRow, TArray<FSourceEffectData>&& Effects, FPotionData& OutPotionData);
};
