#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Card/CardDataTypes.h"
#include "Effect/EffectStruct.h"
#include "PotionStruct.generated.h"

UENUM(BlueprintType)
/* 포션 희귀도 */
enum class EPotionRarity : uint8
{
	Common = 0 UMETA(DisplayName = "Common"),
	Uncommon = 1 UMETA(DisplayName = "Uncommon"),
	Rare = 2 UMETA(DisplayName = "Rare"),
	None = 99 UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
/* 포션 획득 경로 */
enum class EPotionSourceType : uint8
{
	Common = 0 UMETA(DisplayName = "Common"),
	Event = 1 UMETA(DisplayName = "Event"),
	None = 99 UMETA(DisplayName = "None"),
};

USTRUCT(BlueprintType)
/* 포션 기본 정보 CSV 행 */
struct FPotionDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PotionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText PotionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPotionRarity Rarity = EPotionRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPotionSourceType PotionSourceType = EPotionSourceType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplyTiming UseTiming = EEffectApplyTiming::OnUse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETargetType TargetScope = ETargetType::Self;
};

USTRUCT(BlueprintType)
/* 런타임에서 사용하는 포션 데이터 */
struct FPotionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PotionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText PotionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPotionRarity Rarity = EPotionRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPotionSourceType PotionSourceType = EPotionSourceType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplyTiming UseTiming = EEffectApplyTiming::OnUse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETargetType TargetScope = ETargetType::Self;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSourceEffectData> Effects;
};
