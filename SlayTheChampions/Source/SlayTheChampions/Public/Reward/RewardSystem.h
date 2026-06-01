#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Map/MapStruct.h"
#include "RewardSystem.generated.h"

class UAreaClearUiData;
class UBattleClearWidget;
class URunSystem;

UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API URewardSystem : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	URunSystem* RunSystem = nullptr;

	/* 현재 활성화된 Area 클리어 위젯 */
	UPROPERTY()
	UBattleClearWidget* ActiveAreaClearWidget = nullptr;

	/* Area 클리어 보상 UI 데이터 에셋 */
	UPROPERTY()
	UAreaClearUiData* AreaClearUiData = nullptr;

	/* 클리어한 Area 정보 */
	UPROPERTY()
	FAreaInfo ClearedAreaInfo;

public:
	void Initialize(URunSystem* InRunSystem);

	/* Area 클리어 보상 창 처리 */
	UFUNCTION(BlueprintCallable)
	void OpenAreaClearReward(const FAreaInfo& InClearedAreaInfo);

	/* Area 클리어 보상 수령 후 지도로 복귀 처리 */
	UFUNCTION(BlueprintCallable)
	void ReturnToMapAfterReward();

	/* Area 클리어 보상 UI 데이터 설정 */
	UFUNCTION(BlueprintCallable)
	void SetAreaClearUiData(UAreaClearUiData* InAreaClearUiData);

private:
	/* Area 클리어 기본 데이터 로드 */
	bool LoadDefaultAreaClearUiData();

	/* Area 클리어 보상 UI 열기 */
	void OpenAreaClearWidget();
};
