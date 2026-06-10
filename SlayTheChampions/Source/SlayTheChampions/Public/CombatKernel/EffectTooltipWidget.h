#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatKernel/EffectDescriptionLibrary.h"
#include "EffectTooltipWidget.generated.h"

class UTextBlock;

/**
 * UEffectTooltipWidget
 * 버프/디버프(또는 적 행동) 설명을 마우스 근처에 띄우는 공용 툴팁.
 *
 * 표시 방식 (둘 다 지원):
 *  - C++ 폴백: Text_Body(BindWidgetOptional)가 있으면 "이름 + 설명" 여러 줄을 자동 채움
 *  - BP 커스텀: OnEntriesSet 이벤트로 배열을 받아 직접 레이아웃 가능
 *
 * 호버 소스(스탯 위젯 등)가 인스턴스를 1개 만들어 ShowAt/HideTooltip만 호출하면 된다.
 */
UCLASS()
class SLAYTHECHAMPIONS_API UEffectTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 엔트리를 채우고 뷰포트의 ViewportPos(마우스) 근처에 화면 비율을 고려해 표시 (처음 호출 시 뷰포트에 추가)
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowAt(const TArray<FEffectInfo>& Entries, FVector2D ViewportPos);

	// 툴팁 숨김 (Collapsed)
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void HideTooltip();

	// BP 커스텀 레이아웃용 — ShowAt 시 엔트리 배열 전달 (선택)
	UFUNCTION(BlueprintImplementableEvent, Category = "Tooltip")
	void OnEntriesSet(const TArray<FEffectInfo>& Entries);

protected:
	// 엔트리를 Text_Body/OnEntriesSet에 반영. 표시할 게 있으면 true
	bool SetEntries(const TArray<FEffectInfo>& Entries);

	// 설명 본문 텍스트 — 있으면 C++가 "이름\n설명" 여러 줄로 자동 채움
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Body;

	// 커서 기준 오프셋 (px)
	UPROPERTY(EditAnywhere, Category = "Tooltip")
	FVector2D CursorOffset = FVector2D(16.f, 16.f);
};
