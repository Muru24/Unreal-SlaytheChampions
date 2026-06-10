#include "CombatKernel/BuffIconWidget.h"
#include "Components/TextBlock.h"
#include "NiagaraSystem.h"

// 버프 타입과 스택 수를 설정
// 1. 현재 상태 갱신
// 2. Text_Value 텍스트 갱신
// 3. EffectSystemMap에서 해당 타입의 나이아가라 시스템 조회 후 BP에 전달
void UBuffIconWidget::SetBuff(EEffectType Type, int32 Value)
{
	CurrentEffectType = Type;
	CurrentValue      = Value;

	// 스택 수 텍스트 갱신 (0 이하면 숨김)
	if (Text_Value)
	{
		if (Value > 0)
		{
			Text_Value->SetText(FText::AsNumber(Value));
			Text_Value->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Value->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 매핑된 나이아가라 시스템 조회 — 없으면 nullptr 전달 (BP에서 기본 처리)
	UNiagaraSystem* NewSystem = nullptr;
	if (UNiagaraSystem** Found = EffectSystemMap.Find(Type))
		NewSystem = *Found;

	UE_LOG(LogTemp, Log, TEXT("[BuffIcon] SetBuff: Type=%d Value=%d | Text_Value=%s | System=%s | InViewport=%s"),
		(int32)Type, Value,
		Text_Value ? TEXT("OK") : TEXT("NULL"),
		NewSystem ? *NewSystem->GetName() : TEXT("NULL"),
		IsInViewport() ? TEXT("YES") : TEXT("NO"));

	// BP에서 NiagaraSystemWidget 스왑 및 PopulateRemapList 처리
	OnBuffTypeChanged(NewSystem);
}
