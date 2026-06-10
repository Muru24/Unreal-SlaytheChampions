#include "CombatKernel/BuffPanelWidget.h"
#include "CombatKernel/BuffIconWidget.h"
#include "Unit/Unit.h"
#include "Unit/StatusEffectComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

// 유닛의 StatusEffectComponent에 바인딩
// 이전 바인딩이 있으면 먼저 해제하고 새로 연결
void UBuffPanelWidget::InitFromUnit(AUnit* Unit)
{
	Unbind();

	if (!Unit) return;

	BoundSEC = Unit->FindComponentByClass<UStatusEffectComponent>();
	if (!BoundSEC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuffPanel] StatusEffectComponent not found on %s"), *Unit->GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[BuffPanel] InitFromUnit: %s | BuffIconClass=%s | BuffContainer=%s"),
		*Unit->GetName(),
		BuffIconClass ? *BuffIconClass->GetName() : TEXT("NULL"),
		BuffContainer ? TEXT("OK") : TEXT("NULL"));

	BoundSEC->OnEffectValueChanged.AddDynamic(this, &UBuffPanelWidget::OnEffectValueChanged);
}

// 바인딩 해제 및 아이콘 전체 제거
void UBuffPanelWidget::Unbind()
{
	if (BoundSEC)
	{
		BoundSEC->OnEffectValueChanged.RemoveDynamic(this, &UBuffPanelWidget::OnEffectValueChanged);
		BoundSEC = nullptr;
	}

	if (BuffContainer)
		BuffContainer->ClearChildren();

	ActiveIcons.Empty();
}

// 효과 수치 변경 수신 — 아이콘 추가/갱신/제거
void UBuffPanelWidget::OnEffectValueChanged(EEffectType Type, int32 OldValue, int32 NewValue)
{
	UE_LOG(LogTemp, Log, TEXT("[BuffPanel] OnEffectValueChanged: Type=%d Old=%d New=%d"), (int32)Type, OldValue, NewValue);

	// Shield는 CombatStatWidget에서 별도 처리
	if (Type == EEffectType::Shield) return;

	if (NewValue <= 0)
	{
		// 스택이 0이 되면 아이콘 제거
		RemoveIcon(Type);
	}
	else if (UBuffIconWidget** Existing = ActiveIcons.Find(Type))
	{
		// 이미 있는 아이콘이면 값만 갱신
		(*Existing)->SetBuff(Type, NewValue);
	}
	else
	{
		// 새 효과이면 아이콘 생성
		CreateIcon(Type, NewValue);
	}
}

// 아이콘 위젯 생성 후 BuffContainer에 추가
UBuffIconWidget* UBuffPanelWidget::CreateIcon(EEffectType Type, int32 Value)
{
	if (!BuffContainer || !BuffIconClass) return nullptr;

	// WidgetComponent 소속 위젯은 GetOwningPlayer()가 null — World의 첫 번째 PC 사용
	APlayerController* PC = GetOwningPlayer();
	if (!PC && GetWorld())
		PC = GetWorld()->GetFirstPlayerController();

	UE_LOG(LogTemp, Log, TEXT("[BuffPanel] CreateIcon: Type=%d Value=%d | PC=%s"),
		(int32)Type, Value, PC ? TEXT("OK") : TEXT("NULL"));

	if (!PC) return nullptr;

	UBuffIconWidget* Icon = CreateWidget<UBuffIconWidget>(PC, BuffIconClass);
	if (!Icon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuffPanel] CreateWidget failed for BuffIconClass"));
		return nullptr;
	}

	// 계층구조에 먼저 추가한 뒤 SetBuff 호출 — 나이아가라가 위젯 트리 안에서 초기화돼야 렌더링됨
	UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(BuffContainer->AddChild(Icon));
	if (HBoxSlot)
		HBoxSlot->SetPadding(FMargin(0.f, 0.f, IconSpacing, 0.f));
	ActiveIcons.Add(Type, Icon);
	Icon->SetBuff(Type, Value);

	UE_LOG(LogTemp, Log, TEXT("[BuffPanel] Icon added. BuffContainer children: %d"), BuffContainer->GetChildrenCount());

	return Icon;
}

// 아이콘 위젯 제거
void UBuffPanelWidget::RemoveIcon(EEffectType Type)
{
	UBuffIconWidget** IconPtr = ActiveIcons.Find(Type);
	if (!IconPtr) return;

	if (*IconPtr)
		(*IconPtr)->RemoveFromParent();

	ActiveIcons.Remove(Type);
}
