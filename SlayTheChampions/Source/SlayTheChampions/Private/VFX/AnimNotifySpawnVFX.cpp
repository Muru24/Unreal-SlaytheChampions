// Fill out your copyright notice in the Description page of Project Settings.


#include "VFX/AnimNotifySpawnVFX.h"
#include "Unit/Unit.h"


void UAnimNotifySpawnVFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
											const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	// MeshComp의 Owner가 AUnit이면 델리게이트 브로드캐스트
	AUnit* Unit = MeshComp->GetOwner<AUnit>();
	if (!Unit) return;

	Unit->OnVfxNotified.Broadcast(VfxTag);

	UE_LOG(LogTemp, Verbose,
		TEXT("[AnimNotify_SpawnVfx] '%s' → VfxTag='%s'"),
		*Unit->GetName(),
		*VfxTag.ToString())
}
