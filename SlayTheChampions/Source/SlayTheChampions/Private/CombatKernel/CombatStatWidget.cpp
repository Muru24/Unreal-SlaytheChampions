#include "CombatKernel/CombatStatWidget.h"
#include "Unit/Unit.h"
#include "Unit/StatComponent.h"
#include "Unit/StatusEffectComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "NiagaraSystemWidget.h"
#include "CombatKernel/BuffPanelWidget.h"
#include "CombatKernel/EffectTooltipWidget.h"
#include "CombatKernel/EffectDescriptionLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"

// 유닛의 StatComponent와 StatusEffectComponent를 연결해 HP·Shield 변경 이벤트를 구독
void UCombatStatWidget::InitFromUnit(AUnit* InUnit)
{
	if (!InUnit) return;

	// StatComponent 연결 → HP 표시
	UnitStatComp = InUnit->GetStat();
	if (UnitStatComp)
	{
		UnitStatComp->OnHPChanged.AddDynamic(this, &UCombatStatWidget::OnUnitHPChanged);
		// 초기 HP 값을 즉시 반영
		OnUnitHPChanged(0, UnitStatComp->CurrentHP);
	}

	// StatusEffectComponent 연결 → Shield 표시 + 호버 툴팁용 캐시
	UStatusEffectComponent* SEC = InUnit->FindComponentByClass<UStatusEffectComponent>();
	if (SEC)
	{
		SEC->OnEffectValueChanged.AddDynamic(this, &UCombatStatWidget::OnShieldValueChanged);
		BoundSEC = SEC;
	}

	// 유닛 이름 표시
	if (Text_Name)
		Text_Name->SetText(FText::FromName(InUnit->UnitID));

	// Shield 초기값 (0 → 숨김)
	UpdateShieldVisibility(0);

	// 버프/디버프 패널 연결
	if (BuffPanel)
		BuffPanel->InitFromUnit(InUnit);
}

// StatManager가 만든 공용 툴팁 주입 — 모든 스탯 위젯이 공유
void UCombatStatWidget::SetSharedTooltip(UEffectTooltipWidget* InTooltip)
{
	SharedTooltip = InTooltip;
}

// HP가 변경될 때 호출. "현재HP / 최대HP" 형식으로 텍스트 갱신
void UCombatStatWidget::OnUnitHPChanged(int32 OldValue, int32 NewValue)
{
	if (Text_HP && UnitStatComp)
		Text_HP->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), NewValue, UnitStatComp->MaxHP)));
}

// 이펙트 값이 변경될 때 호출. Shield 타입일 때만 방어도 UI를 갱신
void UCombatStatWidget::OnShieldValueChanged(EEffectType Type, int32 OldValue, int32 NewValue)
{
	if (Type != EEffectType::Shield) return;

	if (Text_Defence)
		Text_Defence->SetText(FText::AsNumber(NewValue));

	UpdateShieldVisibility(NewValue);
}

// Shield 수치에 따라 방어도 텍스트와 아이콘의 표시 여부를 전환
void UCombatStatWidget::UpdateShieldVisibility(int32 ShieldValue)
{
	// Shield가 0이면 UI 요소를 숨기고, 1 이상이면 표시
	const ESlateVisibility ShieldVisibility = ShieldValue > 0
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed;

	if (Text_Defence)   Text_Defence->SetVisibility(ShieldVisibility);
	if (Image_Block)    Image_Block->SetVisibility(ShieldVisibility);
	if (NiagaraShield)  NiagaraShield->SetVisibility(ShieldVisibility);
}

// 마우스 진입 → HoverDelay 후 툴팁 표시 타이머 시작
void UCombatStatWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (UWorld* World = GetWorld())
		World->GetTimerManager().SetTimer(HoverTimerHandle, this, &UCombatStatWidget::ShowBuffTooltip, HoverDelay, false);
}

// 마우스 이탈 → 대기 타이머 취소 + 표시 중이면 숨김
void UCombatStatWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(HoverTimerHandle);
	if (SharedTooltip)
		SharedTooltip->HideTooltip();
	if (TooltipInstance)
		TooltipInstance->HideTooltip();
}

// 파괴 시 타이머/툴팁 정리
void UCombatStatWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(HoverTimerHandle);
	if (TooltipInstance)
	{
		TooltipInstance->RemoveFromParent();
		TooltipInstance = nullptr;
	}
	Super::NativeDestruct();
}

// 현재 활성 버프/디버프를 모아 툴팁에 전달
void UCombatStatWidget::ShowBuffTooltip()
{
	if (!BoundSEC) return;

	// 활성 수치형 효과 → FEffectInfo (None·Shield 제외, Shield는 별도 표시되므로)
	TArray<FEffectInfo> Entries;
	for (const TPair<EEffectType, int32>& Pair : BoundSEC->EffectValues)
	{
		if (Pair.Value <= 0) continue;
		if (Pair.Key == EEffectType::None || Pair.Key == EEffectType::Shield) continue;

		FEffectInfo Info = UEffectDescriptionLibrary::GetEffectInfo(Pair.Key);
		// 이름 뒤에 스택 수 표기 — 예: "약화 (2)"
		Info.DisplayName = FText::FromString(FString::Printf(TEXT("%s (%d)"), *Info.DisplayName.ToString(), Pair.Value));
		Entries.Add(Info);
	}

	if (Entries.Num() == 0) return;

	const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

	// 1순위: StatManager가 주입한 공용 툴팁 — 1개를 모든 스탯 위젯이 공유하며 마우스 위치에 표시
	if (SharedTooltip)
	{
		SharedTooltip->ShowAt(Entries, MousePos);
		return;
	}

	// 2순위: 폴백 — TooltipClass로 지연 생성해 커서 근처에 표시
	if (!TooltipClass) return;
	if (!TooltipInstance)
		TooltipInstance = CreateWidget<UEffectTooltipWidget>(GetOwningPlayer(), TooltipClass);
	if (!TooltipInstance) return;

	TooltipInstance->ShowAt(Entries, MousePos);
}
