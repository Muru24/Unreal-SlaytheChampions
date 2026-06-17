// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Unit/UnitAnimDataAsset.h"
#include "UnitAnimComponent.generated.h"


class UAnimMontage;
class USkeletalMeshComponent;
class UUnitAnimDataAsset;
class UStatComponent;
class AUnit;

//이벤트 시작 델리게이트
// 재생 시작/종료시 브로드캐스트
// CombatManager 또는 연출시스템
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitAnimStarted, EUnitAnimType, AnimType, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitAnimFinished, EUnitAnimType, AnimType);


/**
 * UUnitAnimComponent
 *
 * AUnit에 붙여 사용하는 애니메이션 재생 전담 컴포넌트.
 * UUnitAnimDataAsset에 선언된 몽타주를 USkeletalMeshComponent를 통해 재생한다.
 *
 * 자동 이벤트 바인딩
 *  • StatComponent::OnHPChanged     → 데미지 감지 → PlayHit()
 *  • Unit::OnUnitDied               → PlayDeath()
 *  • Unit::OnUnitAttackNotified     → PlayAttack() or PlaySkill()
 *  • Unit::OnUnitMoveNotified       → PlayMove() + 위치 인터폴레이션
 *
 * 외부직접호출
 *  CombatManager가 타이밍을 완전히 제어해야 할 때 Play(EUnitAnimType) 직접 호출.
 *
 * Blueprint 확장법
 *  OnAnimStarted / OnAnimFinished 델리게이트를 Blueprint에서 바인딩해
 *  VFX, 카메라 셰이크, 사운드 등을 추가할 수 있다.
 */

UCLASS(ClassGroup=(Unit), meta=(BlueprintSpawnableComponent))
class SLAYTHECHAMPIONS_API UUnitAnimComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUnitAnimComponent();

	//설정
	/*
	* 이 유닛에 적용할 애니메이션 데이터에셋
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Animation")
	TObjectPtr<UUnitAnimDataAsset> AnimData;

	//몽타주를 재생할 SkeletalMeshComponent
	//null이면 Beginplay에서 Owner의 첫번째 Skeletalmeshcomponent를 자동으로 찾기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

	//True일때: StatComponent::OnHPChanged 수신시 HP가 줄었을 때 자동으로 HitAnim호출
	//False일때: 외부에서 직접 play(EUnitAnimType::Hit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bAutoPlayHitOnDamage = true;

	//True일 때 OnUnitDied수신시 자동으로 DeathAnim 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bAutoPlayDeath = true;

	//재생 인터페이스
	/**
	 * 지정한 AnimType의 몽타주를 즉시 재생한다.
	 * AnimData가 없거나 해당 몽타주가 null이면 아무것도 하지 않는다.
	 * @return 재생된 몽타주의 길이(초). 재생 실패 시 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	float Play(EUnitAnimType AnimType);

	/**
	 * 현재 재생 중인 몽타주를 중단한다.
	 * @param BlendOutTime 블렌드 아웃 시간(초). -1이면 기본값 사용.
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void Stop(float BlendOutTime = -1.f);

	/**
	 * 이동 애니메이션과 함께 WorldDestination으로 인터폴레이션 이동을 시작한다.
	 * MoveSpeed는 AnimData::MoveSpeed를 따른다.
	 * 이동 완료 시 OnMoveFinished 델리게이트가 브로드캐스트된다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayMove(FVector WorldDestination);

	//현재 상태 조회용

	UFUNCTION(BlueprintCallable, BlueprintPure,Category = "Animation")
	bool IsPlayingAnim(EUnitAnimType AnimType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure,Category = "Animation")
	EUnitAnimType GetCurrentAnimType() const { return CurrentAnimType; }

	//Delegate

	//몽타주 재생이 시작될때 (AnimType, 재생길이)
	UPROPERTY(BlueprintAssignable, Category = "Animation")
	FOnUnitAnimStarted OnAnimStarted;

	//몽타주 재생이 완전히 끝났을때 (BlendOut포함)
	UPROPERTY(BlueprintAssignable, Category = "Animtaion")
	FOnUnitAnimFinished OnAnimFinished;

	/** PlayMove() 이동이 완료됐을 때 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveFinished);
	UPROPERTY(BlueprintAssignable, Category = "Animation")
	FOnMoveFinished OnMoveFinished;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
								FActorComponentTickFunction* ThisTickFunction) override;


private:
	//내부
	UPROPERTY()
	EUnitAnimType CurrentAnimType = EUnitAnimType::Idle;

	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentMontage = nullptr;

	//이동 인터플레이션 상태
	bool		bIsMoving				= false;
	FVector		MoveDestination			= FVector::ZeroVector;
	FVector		MoveStartLocation		= FVector::ZeroVector;
	float		MoveTotalDistance		= 0.f;
	float		MoveTraveledDistance	= 0.f;
		
	//이벤트 핸들러(델리게이트 바인딩)
	UFUNCTION()
	void HandleHPChanged(int32 OldHP, int32 NewHP);

	UFUNCTION()
	void HandleUnitDied(AUnit* Unit);

	//Unit::OnUnitAttackNotified 수신->Attack or skill 재생
	UFUNCTION()
	void HandleAttackNotified(bool bIsSkill);

	//Unit::OnUnitMoveNotified 수신 ->PlayMove()호출
	UFUNCTION()
	void HandleMoveNotified(FVector Destination);

	//AnimInstance의 OnMontageEnded에 바인딩
	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	//Utility

	//TargetMesh의 AnimInstance를 통해 몽타주 재생. 길이 반환
	float PlayMontageInternal(const FUnitAnimEntry& Entry, EUnitAnimType AnimType);

	//Owner에서 SkeletalMeshComponent를 찾아 TargetMesh에 캐시
	void TryCacheSkeletalMesh();

};
