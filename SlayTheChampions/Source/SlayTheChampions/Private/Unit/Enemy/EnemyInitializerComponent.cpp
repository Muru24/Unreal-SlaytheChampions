// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Enemy/EnemyInitializerComponent.h"
#include "Unit/StatComponent.h"
#include "Unit/Enemy/NPCBrainComponent.h"
#include "Unit/UnitAnimComponent.h"
#include "Unit/StatusEffectComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
UEnemyInitializerComponent::UEnemyInitializerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}



void UEnemyInitializerComponent::BeginPlay()
{
	Super::BeginPlay();
	//Table과 EnemyID가 설정되어있으면 자동초기화
	InitializeFromTable();
}


void UEnemyInitializerComponent::InitializeFromTable()
{
	// Table 미지정(예: CombatManager가 스폰 후 InitializeFromDefinition으로 직접 주입하는 경우)이면
	// 여기서 역참조하지 않고 종료 — null Table->FindByID 접근 위반 방지
	if (!Table) return;

	const FEnemyDefinition* Def = Table->FindByID(EnemyID);
	if (!Def) return;


	InitializeFromDefinition(*Def);
}


void UEnemyInitializerComponent::InitializeFromDefinition(const FEnemyDefinition& Def)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	//스탯
	if (UStatComponent* Stat = Owner->FindComponentByClass<UStatComponent>())
	{
		Stat->MaxHP = Def.MaxHP;
		Stat->CurrentHP = Def.MaxHP;
	}

	//행동패턴
	if (UNPCBrainComponent* Brain = Owner->FindComponentByClass<UNPCBrainComponent>())
	{
		Brain->Pattern = Def.Pattern;
		Brain->SequenceIndex = 0;
	}

	//기믹 동적부착
	if (Def.GimmickClass)
	{
		UGimmickComponent* G = NewObject<UGimmickComponent>(Owner, Def.GimmickClass);
		G->Data = Def.GimmickData;
		G->RegisterComponent();
	}

	//시작 상태이상
	if (UStatusEffectComponent* SE = Owner->FindComponentByClass<UStatusEffectComponent>())
	{
		for (const FStartingEffect& E : Def.StartingEffects)
			SE->ApplyEffect(E.EffectClass, E.Stacks, E.Duration);
	}

	//비주얼 주입
	if (Def.VisualData)
	{
		// 메시 교체
		if (USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Def.VisualData->SkeletalMesh)
				Mesh->SetSkeletalMesh(Def.VisualData->SkeletalMesh);

			if (Def.VisualData->AnimBPClass)
				Mesh->SetAnimInstanceClass(Def.VisualData->AnimBPClass);

			Mesh->SetRelativeLocation(Def.VisualData->MeshOffset);
			Mesh->SetRelativeScale3D(Def.VisualData->MeshScale);
			Mesh->SetRelativeRotation(Def.VisualData->MeshRotation);
		}

		// 몽타주 세트 주입
		if (Def.VisualData->AnimData)
		{
			if (UUnitAnimComponent* Anim = Owner->FindComponentByClass<UUnitAnimComponent>())
				Anim->AnimData = Def.VisualData->AnimData;
		}
	}
}


