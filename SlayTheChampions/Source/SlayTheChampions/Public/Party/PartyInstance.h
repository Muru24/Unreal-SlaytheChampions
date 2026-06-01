// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Party/ChampionStruct.h"
#include "PartyInstance.generated.h"

class AUnit;

UCLASS()
class SLAYTHECHAMPIONS_API UPartyInstance : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, Category = "PartyInstance", meta = (AllowPrivateAccess = "true"))
	FSavePartyInfo PartyInfo;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void UpdatePartyInfo(FSavePartyInfo _info) { PartyInfo = _info; }
	
	const FSavePartyInfo& GetPartyInfo() const { return PartyInfo; }
	void SetPartyInfo(FSavePartyInfo _info) { PartyInfo = _info; }

	UFUNCTION(BlueprintPure)
	int32 GetPartyMemberCount() const { return PartyInfo.Champions.Num(); }

	/*파티 초기화*/
	UFUNCTION(BlueprintCallable)
	void InitParty();

	/*골드 획득*/
	UFUNCTION(BlueprintCallable)
	void AddGold(int32 _Gold);

	/*골드 사용*/
	UFUNCTION(BlueprintCallable)
	void UseGold(int32 _Price);

	/*유물 획득*/
	UFUNCTION(BlueprintCallable)
	void AddRelic(FRelic _Relic);

	/*유물 잃어버림*/
	UFUNCTION(BlueprintCallable)
	void LostRelic(FName _RelicName);

	/*포션 획득*/
	UFUNCTION(BlueprintCallable)
	void AddPotion(FPotionData _Potion);

	/*포션 사용/제거*/
	UFUNCTION(BlueprintCallable)
	void LostPotion(FName _PotionName);

	UFUNCTION(BlueprintPure)
	const TArray<FPotionData>& GetPotions() const { return PartyInfo.Potions; }
};

