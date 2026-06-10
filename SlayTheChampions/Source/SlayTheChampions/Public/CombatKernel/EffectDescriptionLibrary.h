#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatKernel/EffectTypes.h"
#include "EffectDescriptionLibrary.generated.h"

/**
 * FEffectInfo
 * 버프/디버프 1종의 표시용 정보 (이름 + 설명).
 * 툴팁 위젯(WBP_EffectTooltip)이 이 구조체로 한 줄을 그린다.
 */
USTRUCT(BlueprintType)
struct FEffectInfo
{
	GENERATED_BODY()

	// 효과 한글 이름 — 예: "약화", "화상"
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FText DisplayName;

	// 효과 설명 — 예: "주는 데미지가 25% 감소합니다."
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	FText Description;
};

/**
 * UEffectDescriptionLibrary
 * 버프/디버프 설명 텍스트를 코드에서 직접 생성하는 BlueprintFunctionLibrary.
 * 설명이 EffectManager의 실제 효과 로직(약화 -25%, 취약 +50% 등)과 한 곳에서 묶이므로
 * 수치를 바꿔도 설명이 어긋나지 않는다. (CardComboLibrary와 동일한 코드 기반 패턴)
 */
UCLASS()
class SLAYTHECHAMPIONS_API UEffectDescriptionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 이름 + 설명을 묶어 반환 (툴팁 한 줄 구성용)
	UFUNCTION(BlueprintPure, Category = "Effect")
	static FEffectInfo GetEffectInfo(EEffectType Type);

	// 효과 한글 이름. 매핑 없으면 enum DisplayName(영어) 폴백
	UFUNCTION(BlueprintPure, Category = "Effect")
	static FText GetEffectName(EEffectType Type);

	// 효과 설명. 매핑 없으면 빈 텍스트
	UFUNCTION(BlueprintPure, Category = "Effect")
	static FText GetEffectDescription(EEffectType Type);
};
