// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifySpawnVFX.generated.h"

/**
 * UAnimNotify_SpawnVfx
 *
 * 몽타주의 특정 프레임(히트 프레임 등)에 배치해
 * VFX 스폰 타이밍을 제어하는 AnimNotify.
 *
 * [동작 흐름]
 *   몽타주 히트 프레임 → Notify() 호출
 *     -> Owner(AUnit)의 OnVfxNotified 델리게이트 브로드캐스트
 *       -> UVfxComponent::HandleVfxNotify() 수신
 *         -> 현재 CardID에 등록된 파티클 즉시 스폰
 *
 * [에디터 설정]
 *   1. 몽타주 에디터에서 Notifies 트랙에 'Spawn Vfx' 추가
 *   2. 히트 프레임(칼이 닿는 순간, 마법이 발동되는 순간 등)에 드래그
 *   3. 노티파이 인스턴스 선택 -> VfxTag로 어떤 VFX 슬롯인지 구분 가능
 *      (같은 몽타주에서 여러 타이밍에 다른 파티클을 쓸 때 활용)
 *
 * [여러 노티파이 배치]
 *   하나의 몽타주에 여러 개 배치 가능.
 *   VfxTag가 같으면 같은 파티클, 다르면 다른 파티클로 분기.
 *   예) 첫 번째: VfxTag=Cast (캐스팅 파티클) / 두 번째: VfxTag=Impact (충돌 파티클)
 *
 * [수정하지 않는 파일]
 *   UUnitAnimComponent에 아래 두 줄 추가만 요청:
 *     - Unit.h  : OnVfxNotified 델리게이트 선언 (예시 주석 참고)
 *     - UUnitAnimComponent.cpp : HandleMontageNotifyBegin() 에서 캐스트 후 브로드캐스트
 */
UCLASS()
class SLAYTHECHAMPIONS_API UAnimNotifySpawnVFX : public UAnimNotify
{
	GENERATED_BODY()
public:
	/**
	 * VFX 구분 태그.
	 * UVfxComponent가 여러 Entry 중 어느 것을 재생할지 식별할 때 사용.
	 * 비워두면 현재 CardID에 등록된 모든 Entry 재생.
	 * 예) "Cast", "Impact", "Trail"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FName VfxTag = NAME_None;

	/**
	 * 몽타주 히트 프레임에 도달했을 때 UE5가 자동 호출.
	 * Owner(AUnit)의 OnVfxNotified 델리게이트를 브로드캐스트.
	 */
	virtual void Notify(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return VfxTag != NAME_None
			? FString::Printf(TEXT("SpawnVfx [%s]"), *VfxTag.ToString())
			: TEXT("SpawnVfx");
	}
};
