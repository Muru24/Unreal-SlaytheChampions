#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Map/MapStruct.h"
#include "Reward/RewardStruct.h"
#include "BattleClearWidget.generated.h"

class URewardSystem;
class UCardRewardSystem;

UCLASS(Blueprintable, BlueprintType)
class SLAYTHECHAMPIONS_API UBattleClearWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/* 리워드 데이터 만들어줄 함수 */
	UFUNCTION(BlueprintCallable, Category = "AreaClear")
	TArray<FRewardData> GenerateRewardData(EAreaType _type);

	/* 위젯이 실제로 보일 때 보상 목록 갱신 */
	UFUNCTION(BlueprintCallable, Category = "AreaClear")
	void RefreshRewards();

	UFUNCTION(BlueprintCallable, Category = "AreaClear")
	TArray<FName> GetCardRewardChoices(int32 PawnIndex) const;

	UFUNCTION(BlueprintPure, Category = "AreaClear")
	int32 GetPartyRewardTargetCount() const;

	UFUNCTION(BlueprintCallable, Category = "AreaClear")
	void SetCardRewardChoiceCount(int32 InCount);

	UFUNCTION(BlueprintPure, Category = "AreaClear")
	int32 GetCardRewardChoiceCount() const;

	UFUNCTION(BlueprintPure, Category = "AreaClear")
	const TArray<FRewardData>& GetCurrentRewardData() const { return CurrentRewardData; }

	/* 보상 목록이 갱신된 뒤 Blueprint에서 UI를 채우는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "AreaClear")
	void OnRewardDataRefreshed(const TArray<FRewardData>& Rewards);
	
private:
	UFUNCTION()
	void HandleVisibilityChanged(ESlateVisibility InVisibility);

	UCardRewardSystem* GetCardRewardSystem() const;

	EAreaType GetCurrentAreaType() const;

	/* 확률형 보상 생성 여부 */
	bool RollChance(float _Probability);

	/*골드 보상 수치*/
	int32 RollGold(EAreaType _type);

	UPROPERTY(BlueprintReadOnly, Category = "AreaClear", meta = (AllowPrivateAccess = "true"))
	TArray<FRewardData> CurrentRewardData;
};
