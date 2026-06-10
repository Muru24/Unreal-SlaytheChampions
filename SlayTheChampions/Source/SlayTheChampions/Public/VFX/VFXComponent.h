// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Unit/UnitAnimDataAsset.h"
#include "VFX/SkillVFXDataAsset.h"
#include "VFXComponent.generated.h"

class AUnit;
class UParticleSystemComponent;

/**
* AUnit에 붙여 사용하는 스킬 VFX 전담 컴포넌트.
* 
* [타이밍 ? AnimNotify 방식]
*  UAnimNotify_SpawnVfx가 호출한 AUnit::OnVfxNotified 델리게이트를 구독한다.
* 
* [동작흐름]
* CombatManager ->Vfx->SetCurrentCardID("Mage_Fireball")
* CombatManager ->Vfx->SetCurrentTarget(Enemy)
* CombatManager ->Caster->NotifyAttack(true)
* ->AnimComponent가 스킬 몽타주 재생
*/
UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class SLAYTHECHAMPIONS_API UVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVFXComponent();

	//이 유닛이 적용할 스킬 VFX 데이터에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<USkillVFXDataAsset> SkillVfxData;
	

	//런타임 상태 주입
	/** 현재 사용 중인 카드 ID. NotifyAttack() 호출 전에 반드시 세팅. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetCurrentCardID(FName CardID);

	/** 단일 대상 설정. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetCurrentTarget(AUnit* Target);

	/** 다중 대상 설정 (광역 스킬). */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetCurrentTargets(const TArray<AUnit*>& Targets);

	/** 스킬 사용 완료 후 상태 초기화. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void ClearCurrentState();

	//직접 재생
	/** CardID·대상·태그를 직접 지정해 즉시 VFX 재생 (노티파이 없이). */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void PlayVfxDirect(FName CardID, AUnit* Target, FName VfxTag = NAME_None);

	/** 광역 VFX 직접 재생. */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void PlayVfxDirectMulti(FName CardID, const TArray<AUnit*>& Targets,
		FName VfxTag = NAME_None);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FName CurrentCardID = NAME_None;

	UPROPERTY()
	TWeakObjectPtr<AUnit> CurrentTarget;

	UPROPERTY()
	TArray<TWeakObjectPtr<AUnit>> CurrentTargets;

	//이벤트 핸들러
	/**
	 * AUnit::OnVfxNotified 구독 핸들러.
	 * AnimNotify_SpawnVfx 가 브로드캐스트하면 여기서 수신해 파티클 스폰.
	 * @param VfxTag  AnimNotify_SpawnVfx.VfxTag 값
	 */
	UFUNCTION()
	void HandleVfxNotify(FName VfxTag);

	//스폰 내부함수
	void ProcessVfxEntries(const TArray<FSkillVfxEntry>& Entries,
		const TArray<TWeakObjectPtr<AUnit>>& Targets);

	void ExecuteEntry(const FSkillVfxEntry& Entry, TWeakObjectPtr<AUnit> Target);

	UParticleSystemComponent* SpawnAtTarget(const FSkillVfxEntry& Entry, AUnit* Target);
	void                      LaunchProjectile(const FSkillVfxEntry& Entry, AUnit* Target);
	UParticleSystemComponent* SpawnAtCaster(const FSkillVfxEntry& Entry);

	FTransform ResolveSpawnTransform(AActor* Actor, FName SocketName,
		FVector Offset, FRotator RotOffset) const;
	
};
