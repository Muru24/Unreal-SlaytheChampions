#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/TimerHandle.h"
#include "Unit/StatusEffectComponent.h"
#include "CombatStatWidget.generated.h"

class UTextBlock;
class UImage;
class AUnit;
class UStatComponent;
class UStatusEffectComponent;
class UNiagaraSystemWidget;
class UBuffPanelWidget;
class UEffectTooltipWidget;

/**
 * UCombatStatWidget
 * 유닛 한 명의 HP·Shield 수치를 실시간으로 표시하는 위젯.
 * InitFromUnit()으로 AUnit에 연결하면 델리게이트를 통해 자동 갱신된다.
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCombatStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// AUnit 하나로 HP + Shield 모두 연결. BeginPlay 이후에 호출해야 한다
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void InitFromUnit(AUnit* InUnit);

	// StatManager가 만든 공용 툴팁 1개를 주입. 모든 스탯 위젯이 이걸 공유해 내용만 갈아끼운다.
	// 주입되면 자체 Tooltip/TooltipClass보다 우선 사용된다.
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void SetSharedTooltip(UEffectTooltipWidget* InTooltip);

protected:
	// HP 텍스트 (BindWidget — BP 디자이너에서 같은 이름으로 배치 필수)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_HP;

	// 방어도(Shield) 수치 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Defence;

	// 유닛 이름 텍스트 — WBP에서 'Text_Name' 이름으로 배치
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Text_Name;

	// 방패 아이콘 — Shield=0이면 숨김 (BindWidgetOptional: 없어도 크래시 없음)
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Image_Block;

	// 방어도 나이아가라 위젯 — Shield=0이면 숨김, WBP에서 'NiagaraShield' 이름으로 배치
	UPROPERTY(meta = (BindWidgetOptional))
	UNiagaraSystemWidget* NiagaraShield;

	// 버프/디버프 패널 — WBP에서 'BuffPanel' 이름으로 배치
	UPROPERTY(meta = (BindWidgetOptional))
	UBuffPanelWidget* BuffPanel;

	// SharedTooltip이 주입되지 않았을 때 지연 생성할 폴백 클래스 (커서를 따라다님)
	UPROPERTY(EditAnywhere, Category = "Tooltip")
	TSubclassOf<UEffectTooltipWidget> TooltipClass;

	// 툴팁이 뜨기까지의 호버 유지 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Tooltip", meta = (ClampMin = "0"))
	float HoverDelay = 1.f;

	// 1초 호버 시작 → 타이머, 종료 → 취소 + 숨김
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeDestruct() override;

private:
	// 연결된 유닛의 StatComponent 캐시
	UPROPERTY()
	UStatComponent* UnitStatComp;

	// 연결된 유닛의 StatusEffectComponent 캐시 (호버 툴팁에서 활성 효과 열거용)
	UPROPERTY()
	UStatusEffectComponent* BoundSEC = nullptr;

	// 지연 생성되는 툴팁 인스턴스 (자체 폴백용)
	UPROPERTY()
	UEffectTooltipWidget* TooltipInstance = nullptr;

	// StatManager가 주입한 공용 툴팁 (있으면 최우선 사용)
	UPROPERTY()
	UEffectTooltipWidget* SharedTooltip = nullptr;

	// 호버 지연 타이머
	FTimerHandle HoverTimerHandle;

	// 타이머 만료 시 호출 — 현재 활성 버프/디버프를 모아 툴팁 표시
	UFUNCTION()
	void ShowBuffTooltip();

	// StatComponent::OnHPChanged 수신 → Text_HP 갱신
	UFUNCTION()
	void OnUnitHPChanged(int32 OldValue, int32 NewValue);

	// StatusEffectComponent::OnEffectValueChanged 수신 → Shield UI 갱신
	UFUNCTION()
	void OnShieldValueChanged(EEffectType Type, int32 OldValue, int32 NewValue);

	// Shield 수치에 따라 Text_Defence·Image_Block 표시 여부를 전환
	void UpdateShieldVisibility(int32 ShieldValue);
};
