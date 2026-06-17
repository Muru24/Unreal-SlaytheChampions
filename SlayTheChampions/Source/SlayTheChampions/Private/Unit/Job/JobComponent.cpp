// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Job/JobComponent.h"
#include "Unit/Job/JobDetail.h"
#include "Unit/Job/Job_Healer.h"
#include "Unit/Job/Job_Mage.h"
#include "Unit/Job/Job_Warrior.h"
// Sets default values for this component's properties
UJobComponent::UJobComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UJobComponent::BeginPlay()
{
	Super::BeginPlay();
	SpawnDetail();
}


// 런타임에 직업 변경 — JobClass 갱신 후 Detail 재생성
void UJobComponent::SetJobClass(EJobClass NewJob)
{
	JobClass = NewJob;
	Detail = nullptr;   // 이전 직업 Detail 폐기 (GC 대상) 후 새로 생성
	SpawnDetail();
}

//���� ������ ����
void UJobComponent::SpawnDetail()
{
	TSubclassOf<UJobDetail> DetailClass = nullptr;

	switch (JobClass)
	{
	
	case EJobClass::Warrior: DetailClass = UJob_Warrior::StaticClass(); break;
	case EJobClass::Mage:    DetailClass = UJob_Mage::StaticClass();    break;
	case EJobClass::Healer:  DetailClass = UJob_Healer::StaticClass();  break;
	default:
		// EJobClass::Any �� ���� ���� �����Ͼ��� ����
		UE_LOG(LogTemp, Warning,
			TEXT("[JobComponent] JobClass=Any: no detail spawned (%s)"),
			*GetOwner()->GetName());
		return;
	}

	Detail = NewObject<UJobDetail>(this, DetailClass);
	if (Detail)
	{
		Detail->Initialize(this);
		UE_LOG(LogTemp, Log,
			TEXT("[JobComponent] Spawned %s for %s"),
			*DetailClass->GetName(), *GetOwner()->GetName());
	}
}


void UJobComponent::OnCardPlayed(FGameplayTag CardTag, int32 CardValue)
{
	if (Detail)
	{
		Detail->OnCardPlayed(CardTag, CardValue);
	}
}

float UJobComponent::ModifyCardDamage(FGameplayTag CardTag, float BaseDamage)
{
	if (Detail)
	{
		return Detail->ModifyCardDamage(CardTag, BaseDamage);
	}
	return BaseDamage;
}

void UJobComponent::OnTurnStart()
{
	if (Detail)
	{
		Detail->OnTurnStart();
	}
}
void UJobComponent::OnTurnEnd()
{
	if (Detail)
	{
		Detail->OnTurnEnd();
	}
}



