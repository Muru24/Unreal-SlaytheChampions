#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Relic/RelicStruct.h"
#include "RelicSubsystem.generated.h"

class UDataTable;

UCLASS()
class SLAYTHECHAMPIONS_API URelicSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

private:
    UPROPERTY()
    TObjectPtr<UDataTable> RelicInfoTable;

    UPROPERTY()
    TObjectPtr<UDataTable> RelicEffectsTable;

    UPROPERTY()
    TArray<FRelic> Relics;

    UPROPERTY()
    TArray<FRelic> DefaultRelics;

    UPROPERTY()
    TArray<FRelic> ShopRelics;

    UPROPERTY()
    TArray<FRelic> EventRelics;

    UPROPERTY()
    TArray<FRelic> CommonRelics;

    TMap<FName, FRelic> Map_Relics;

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    bool GetCachedRelicData(FName InRelicID, FRelic& OutRelicData) const;

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    TArray<FRelic> GetCachedRelics() const;

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    TArray<FRelic> GetCachedRelicsBySource(ERelicSourceType InSourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    TArray<FRelic> GetDefaultRelics() const { return DefaultRelics; }

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    TArray<FRelic> GetShopRelics() const { return ShopRelics; }

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    TArray<FRelic> GetEventRelics() const { return EventRelics; }

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    TArray<FRelic> GetCommonRelics() const { return CommonRelics; }

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    FRelic GetRelics(FName _RelicId);

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    FName GetRandomShopRelic();

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    FName GetRandomCommonRelic();

    UFUNCTION(BlueprintCallable, Category = "Relic|Query")
    FName GetRandomShopAvailableRelic();

private:
    void RebuildRelicCache();

    void AddRelicToSourceCache(const FRelic& InRelicData);

    static FSourceEffectData MakeRelicEffectData(const FSourceEffectRow& EffectRow);

    static void FillRelicRuntimeData(const FRelicDataRow& RelicRow, TArray<FSourceEffectData>&& Effects, FRelic& OutRelicData);

   


};
