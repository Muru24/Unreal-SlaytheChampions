#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Card/CardDataTypes.h"
#include "CombatKernel/EffectTypes.h"
#include "Effect/EffectStruct.h"
#include "RelicStruct.generated.h"

UENUM(BlueprintType)
/*유물 적용 타이밍*/
enum class EEffectiveDate : uint8
{
	OnAcquire = 0 UMETA(DisplayName = "On Acquire"),
	OnMap = 1 UMETA(DisplayName = "On Map"),
	InShop = 2 UMETA(DisplayName = "In Shop"),
	OnBattleStart = 3 UMETA(DisplayName = "On Battle Start"),
	OnBattleEnd = 4 UMETA(DisplayName = "On Battle End"),
	DuringBattle = 5 UMETA(DisplayName = "During Battle"),
	None = 99 UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
/*유물 획득 경로*/
enum class ERelicSourceType : uint8
{
	Default = 0 UMETA(DisplayName = "Default"),
	Shop = 1 UMETA(DisplayName = "Shop"),
	Event = 2 UMETA(DisplayName = "Event"),
	Common = 3 UMETA(DisplayName = "Common"),
	None = 99 UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
/* 유물 희귀도 */
enum class ERelicRarity : uint8
{
	Normal = 0 UMETA(DisplayName = "Normal"),
	Rare = 1 UMETA(DisplayName = "Rare"),
	Legendary = 2 UMETA(DisplayName = "Legendary"),
	None = 99 UMETA(DisplayName = "None"),
};

USTRUCT(BlueprintType)
/* 유물 기본 정보 CSV 행 */
struct FRelicDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RelicID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RelicName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERelicRarity Rarity = ERelicRarity::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERelicSourceType RelicSourceType = ERelicSourceType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETargetType TargetScope = ETargetType::Self;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplyTiming ApplyTiming = EEffectApplyTiming::None;
};

USTRUCT(BlueprintType)
/* 런타임에서 사용하는 유물 데이터 */
struct FRelic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RelicID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RelicName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERelicRarity Rarity = ERelicRarity::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERelicSourceType RelicSourceType = ERelicSourceType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETargetType TargetScope = ETargetType::Self;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplyTiming ApplyTiming = EEffectApplyTiming::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSourceEffectData> Effects;
};
