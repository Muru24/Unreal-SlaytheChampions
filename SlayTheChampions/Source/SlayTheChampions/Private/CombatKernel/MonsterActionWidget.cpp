#include "CombatKernel/MonsterActionWidget.h"
#include "CombatKernel/BuffIconWidget.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void UMonsterActionWidget::SetMonsterName(const FText& Name)
{
	if (Text_MonsterName)
		Text_MonsterName->SetText(Name);
}

void UMonsterActionWidget::UpdateActions(const TArray<FIntent>& Intents)
{
	// 호버 툴팁 등에서 다중 행동을 조회할 수 있도록 보관
	DisplayedIntents = Intents;

	if (!ActionContainer || !IconWidgetClass) return;

	ActionContainer->ClearChildren();

	for (const FIntent& Intent : Intents)
	{
		UBuffIconWidget* Icon = CreateWidget<UBuffIconWidget>(this, IconWidgetClass);
		if (!Icon) continue;

		// EIntentKind → EEffectType 변환
		const EEffectType* Found = IntentIconMap.Find(Intent.Kind);
		const EEffectType  Type  = Found ? *Found : EEffectType::None;

		// Attack이면 데미지 수치 표시, 나머지는 0 (BuffIconWidget에서 자동 숨김)
		const int32 DisplayValue = (Intent.Kind == EIntentKind::Attack) ? Intent.Value : 0;

		Icon->SetBuff(Type, DisplayValue);

		// 아이콘 간 간격 적용 (마지막 항목 제외)
		UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(ActionContainer->AddChild(Icon));
		if (HBoxSlot && &Intent != &Intents.Last())
			HBoxSlot->SetPadding(FMargin(0.f, 0.f, IconSpacing, 0.f));
	}
}
