// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Enemy/GimmickComponent.h"
#include "Unit/Enemy/GimmickData.h"
#include "Unit/StatComponent.h"
#include "Unit/Unit.h"

// Sets default values for this component's properties
UGimmickComponent::UGimmickComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGimmickComponent::BeginPlay()
{
	Super::BeginPlay();

	// Fired 배열 초기화
	if (Data)
	{
		Fired.SetNum(Data->Phases.Num());
		for (bool& b : Fired) b = false;
	}

	// StatComponent.OnHPChanged 자동 바인딩
	if (UStatComponent* Stat = GetOwner()->FindComponentByClass<UStatComponent>())
	{
		Stat->OnHPChanged.AddUniqueDynamic(this, &UGimmickComponent::HandleHPChanged);
	}

	// Unit.OnUnitDied 자동 바인딩
	if (AUnit* Unit = Cast<AUnit>(GetOwner()))
	{
		Unit->OnUnitDied.AddUniqueDynamic(this, &UGimmickComponent::HandleOwnerDied);
	}

}

void UGimmickComponent::OnTurnStart()
{
	++TurnCounter;
	CheckTriggers();
	OnGimmickTurnStart();
}



void UGimmickComponent::OnTurnEnd()
{
	OnGimmickTurnEnd();
}




// 모든 페이즈의 모든 트리거 검사
void UGimmickComponent::CheckTriggers()
{
	if (!Data) return;
	for (int32 i = 0; i < Data->Phases.Num(); ++i)
	{
		if (Fired[i] && Data->Phases[i].bOneShot) continue;

		if (EvaluateTrigger(Data->Phases[i]))
		{
			EnterPhase(i);
		}
	}
}

bool UGimmickComponent::EvaluateTrigger(const FGimmickPhase& Phase) const
{
	switch (Phase.Trigger)
	{
	case EGimmickTrigger::TurnCountReached:
		return TurnCounter >= static_cast<int32>(Phase.TriggerValue);

	case EGimmickTrigger::HPThresholdBelow:
	{
		const UStatComponent* Stat = GetOwner()->FindComponentByClass<UStatComponent>();
		if (!Stat || Stat->MaxHP <= 0) return false;
		const float Ratio = static_cast<float>(Stat->CurrentHP) / Stat->MaxHP;
		return Ratio <= Phase.TriggerValue;
	}

	case EGimmickTrigger::OnDamaged:
		// OnDamaged는 CheckTriggers가 아니라 HandleHPChanged에서 직접 처리
		return false;

	default:
		return false;
	}
}

void UGimmickComponent::EnterPhase(int32 Index)
{
	Fired[Index] = true;
	OnPhaseEntered.Broadcast(Data->Phases[Index]);

	if (!Data->Phases[Index].AnnounceText.IsEmpty())
	{
		OnGimmickAnnounce.Broadcast(Data->Phases[Index].AnnounceText);
	}
}



