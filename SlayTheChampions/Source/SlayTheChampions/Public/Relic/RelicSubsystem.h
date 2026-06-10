#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Relic/RelicStruct.h"
#include "Reward/RewardStruct.h"
#include "RelicSubsystem.generated.h"

class UDataTable;
class AUnit;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRelicEffectTriggered, const FRelic&, RelicData, const FSourceEffectData&, EffectData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRelicCardRewardRequested, ERewardTypes, RewardType, FName, RewardID, int32, Amount);

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

    UPROPERTY(BlueprintAssignable, Category = "Relic|Effect")
    FOnRelicEffectTriggered OnRelicEffectTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Relic|Effect")
    FOnRelicCardRewardRequested OnRelicCardRewardRequested;

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

    UFUNCTION(BlueprintCallable, Category = "Relic|Effect")
    bool TriggerRelicEffectsByTiming(const FRelic& RelicData, EEffectApplyTiming ApplyTiming, const TArray<AUnit*>& Targets);

    UFUNCTION(BlueprintCallable, Category = "Relic|Effect")
    bool TriggerOwnedRelicEffectsByTiming(EEffectApplyTiming ApplyTiming, const TArray<AUnit*>& Targets);

private:
    void RebuildRelicCache();

    void AddRelicToSourceCache(const FRelic& InRelicData);

    static FSourceEffectData MakeRelicEffectData(const FSourceEffectRow& EffectRow);

    static void FillRelicRuntimeData(const FRelicDataRow& RelicRow, TArray<FSourceEffectData>&& Effects, FRelic& OutRelicData);

    bool ApplyRelicEffect(const FRelic& RelicData, const FSourceEffectData& EffectData, const TArray<AUnit*>& Targets);

    TArray<FSourceEffectData> ResolveEffectSelection(const TArray<FSourceEffectData>& Effects) const;


};
