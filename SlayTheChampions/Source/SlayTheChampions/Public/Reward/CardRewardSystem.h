#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Card/CardDataTypes.h"
#include "Reward/RewardStruct.h"
#include "CardRewardSystem.generated.h"

class UCardSubsystem;
class UPartyInstance;

UCLASS()
class SLAYTHECHAMPIONS_API UCardRewardSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /*카드 ID 받아오기*/
    UFUNCTION(BlueprintCallable, Category = "CardReward")
    TArray<FName> GetRewardCardChoices(int32 PawnIndex) const;

    /*카드 ID로 데이터 받아오기*/
    UFUNCTION(BlueprintCallable, Category = "CardReward")
    bool GetCardDataById(FName CardID, FCardDataRow& OutCardData) const;

    /*파티원 직업 enum 받아오기*/
    UFUNCTION(BlueprintPure, Category = "CardReward")
    EJobClass GetRewardJobClass(int32 PawnIndex) const;

    /*파티원 직업 이름 받아오기*/
    UFUNCTION(BlueprintPure, Category = "CardReward")
    FText GetRewardJobName(int32 PawnIndex) const;

    /*파티 인원수 받아오기*/
    UFUNCTION(BlueprintPure, Category = "CardReward")
    int32 GetPartyRewardTargetCount() const;

    /*카드 보상 수량지정*/
    UFUNCTION(BlueprintCallable, Category = "CardReward")
    void SetCardRewardChoiceCount(int32 InCount);

    /*카드 보상 수량 가져오기*/
    UFUNCTION(BlueprintPure, Category = "CardReward")
    int32 GetCardRewardChoiceCount() const { return CardRewardChoiceCount; }

private:
    UCardSubsystem* GetCardSubsystem() const;

    UPartyInstance* GetPartyInstance() const;

    TArray<FName> GetRewardPoolByRarity(EJobClass JobClass, ECardRarity CardRarity) const;

    bool TryAddRewardChoice(EJobClass JobClass, TArray<FName>& Choices) const;

    ECardRarity RollRewardCardRarity() const;

    EJobClass ResolveRewardJobClass(int32 PawnIndex) const;

    UPROPERTY(EditAnywhere, Category = "CardReward")
    int32 CardRewardChoiceCount = 3;
};
