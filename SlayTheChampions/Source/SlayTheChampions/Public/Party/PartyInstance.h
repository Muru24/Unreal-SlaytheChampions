// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Party/ChampionStruct.h"
#include "Card/CardDataTypes.h"   // EJobClass
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
	void SetPartyInfo(FSavePartyInfo _info);

	UFUNCTION(BlueprintPure)
	int32 GetPartyMemberCount() const;

	UFUNCTION(BlueprintPure, Category = "PartyInstance|Gold")
	int32 GetGold() const { return PartyInfo.Gold; }

	// 배틀 레벨 진입 시 플레이어 액터가 자신을 등록. BP_TestPlayer 등 BeginPlay에서 호출
	UFUNCTION(BlueprintCallable)
	void RegisterChampion(AUnit* Unit);

	// 레벨 전환 전 Champions 초기화 (이전 레벨 액터 참조 제거)
	UFUNCTION(BlueprintCallable)
	void ClearChampions() { PartyInfo.Champions.Empty(); }

	// ── 스폰용 챔피언 직업 목록 ────────────────────────────────────
	// 레벨에 플레이어 액터를 배치하지 않고 CombatManager가 단일 BP_Player를 직업별로 스폰.
	// 메뉴·테스트 BP에서 AddChampion으로 직업을 채워두면 전투 레벨에서 자동 스폰됨.
	// 배열 순서 = PlayerBox 순서 = PawnIndex (0번/1번/2번 플레이어).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PartyInstance")
	TArray<EJobClass> ChampionJobs;

	// 스폰할 챔피언(직업) 추가
	UFUNCTION(BlueprintCallable)
	void AddChampion(EJobClass Job) { ChampionJobs.Add(Job); }

	// 스폰할 챔피언 직업 목록 초기화
	UFUNCTION(BlueprintCallable)
	void ClearChampionJobs() { ChampionJobs.Empty(); }

	// CombatManager가 읽는 직업 목록
	const TArray<EJobClass>& GetChampionJobs() const { return ChampionJobs; }

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

