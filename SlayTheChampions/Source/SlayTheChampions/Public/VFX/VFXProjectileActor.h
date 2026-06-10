// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXProjectileActor.generated.h"

class UParticleSystemComponent;
class UParticleSystem;
class AUnit;

/**
* Fireball 처럼 시전자 에서 대상으로 날아가는 발사체 액터
* 물리기반이 아닌 선형보간이동으로 대상 추적
* 
* [동작]
* Launch() ->파티를 활성화, 대상 위치를 목표로 이동시작
* TickComponent에서 MoveSpeed로 이동
* 대상 반경 도달시 
* ParticleSystem 비활성화
* ImpactParticle 이 있으면 대상 위치에 스폰
* 자기자신 Destroy
* 
* [커스터마이징]
* Blueprint에서 이 클래스를 상속받아
* BeforeImpact() 이벤트 등을 오버라이드해서
* 사운드나 화면흔들림 추가가능
* 
* [FSkillVfxEntry.ProjectileClass 에 등록]
* 기본으로 사용하고 싶으면 null
* 커스텀으로 하고싶으면 생성하여 지정
*/
UCLASS(BlueprintType, Blueprintable)
class SLAYTHECHAMPIONS_API AVFXProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVFXProjectileActor();

	/*
	* 발사 초기화 VFXComponent가 스폰 직후 호출
	* @param InParticle    재생할 Cascade 파티클 (발사체 비주얼)
	* @param InTarget      날아갈 대상 유닛
	* @param InSpeed       이동 속도 (cm/s)
	* @param InImpactParticle 도착 시 스폰할 충돌 파티클 (null 이면 없음)
	*/
	UFUNCTION(BlueprintCallable, Category = "VFX|Projectile")
	void Launch(UParticleSystem* InParticle,
		AUnit* InTarget,
		float  InSpeed,
		UParticleSystem* InImpactParticle);

	//에디터 설정
	/** 대상 도달 판정 반경 (cm). 이 거리 이내면 도착으로 처리. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Projectile",
		meta = (ClampMin = "10"))
	float ArrivalThreshold = 50.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/**
	 * 대상에 도달하기 직전 호출되는 Blueprint 이벤트.
	 * 사운드, 카메라 셰이크 등의 연출을 Blueprint에서 추가할 때 오버라이드.
	 */
	
	virtual void BeforeImpact();
	

private:
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ParticleComp;

	UPROPERTY()
	TWeakObjectPtr<AUnit> TargetUnit;

	UPROPERTY()
	TObjectPtr<UParticleSystem> ImpactParticle;

	float MoveSpeed = 800.f;
	bool  bLaunched = false;

	void OnArrived();

};
