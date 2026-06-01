#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Map/MapEnum.h"
#include "AreaClearUiData.generated.h"

class UBattleClearWidget;

UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UAreaClearUiData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaClearUi")
	TSubclassOf<UBattleClearWidget> BattleAreaClearWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaClearUi")
	TSubclassOf<UBattleClearWidget> EventAreaClearWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaClearUi")
	TSubclassOf<UBattleClearWidget> RestAreaClearWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaClearUi")
	TSubclassOf<UBattleClearWidget> ShopAreaClearWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaClearUi")
	TSubclassOf<UBattleClearWidget> RewardAreaClearWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaClearUi")
	TSubclassOf<UBattleClearWidget> ArtifactEventAreaClearWidget;

	/* Area 타입에 따른 클리어 위젯 클래스 반환 */
	TSubclassOf<UBattleClearWidget> GetAreaClearWidgetClass(EAreaType AreaType) const;
};

