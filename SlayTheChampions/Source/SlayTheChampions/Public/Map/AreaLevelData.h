#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Map/MapEnum.h"
#include "AreaLevelData.generated.h"

UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UAreaLevelData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName InitialStreamedLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName NormalLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName EliteLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName BossLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName EventLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName RestLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName ShopLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName RewardLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AreaLevel")
	FName ArtifactEventLevelName;

	/* Area 타입에 따른 레벨 이름 반환 */
	FName GetLevelName(EAreaType AreaType) const;
};



