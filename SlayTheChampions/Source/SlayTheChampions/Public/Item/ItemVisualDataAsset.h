#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/ItemActor.h"
#include "ItemVisualDataAsset.generated.h"

class UMaterialInterface;
class UStaticMesh;

USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FItemVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Visual")
	EItemActorType ItemType = EItemActorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Visual")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Visual")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Visual")
	TObjectPtr<UMaterialInterface> Material = nullptr;
};

UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UItemVisualDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Item Visual")
	bool FindVisualData(EItemActorType ItemType, FName ItemID, FItemVisualData& OutVisualData) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Visual", meta = (AllowPrivateAccess = "true"))
	TArray<FItemVisualData> Visuals;
};
