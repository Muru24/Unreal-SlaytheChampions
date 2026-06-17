// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UnitAnimDataAsset.generated.h"

class UAnimMontage;
/**
 * EUnitAnimType
 * 유닛이 재생할수 있는 애니메이션 종류
 * UUnitAnimComponent::Play()의 인자
 * Blueprint 이벤트 OnAnimStarted/ OnAnimFinished의 파라미터
 */

UENUM(BlueprintType)
enum class EUnitAnimType : uint8
{
	Idle	UMETA(DisplayName = "Idle"),	//기본대기
	Attack	UMETA(DisplayName = "Attack"),	//공격
	Hit		UMETA(DisplayName = "Hit"),		//피격
	Death	UMETA(DisplayName = "Death"),	//사망
	Move	UMETA(DisplayName = "Move"),	//이동
	Skill	UMETA(DisplayName = "Skill"),	//스킬
	Victory	UMETA(DisplayName = "Victory")	//승리
};

/**
 * FUnitAnimEntry
 * 몽타주 하나에 대응하는 데이터 묶음.
 * 재생 속도(PlayRate)와 시작 섹션 이름을 함께 저장해
 * DataAsset에서 세부 조정이 가능하다.
 */
USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FUnitAnimEntry
{
    GENERATED_BODY()

    // 재생할 AnimMontage. null이면 해당 타입은 재생 건너뜀
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    // 재생 속도 배율 (1.0 = 원본 속도)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.1"))
    float PlayRate = 1.0f;

    // 시작할 섹션 이름. 비어 있으면 처음부터 재생
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    FName StartSection = NAME_None;
};

/**
 * UUnitAnimDataAsset
 * 유닛 종류(캐릭터/적)마다 만드는 Primary DataAsset.
 * EUnitAnimType 별 몽타주 정보를 한 곳에 모아 에디터에서 설정한다.
 *
 * 사용법:
 *   1. Content Browser → 우클릭 → Miscellaneous → Data Asset → UnitAnimDataAsset 선택
 *   2. 각 슬롯에 AnimMontage 지정
 *   3. AUnit Blueprint의 UUnitAnimComponent.AnimData에 드래그&드롭
 */

UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API UUnitAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    //상태별 몽타주맵

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry AttackAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry HitAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry DeathAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry SkillAnim;


    //이동
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry MoveAnim;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Move")
    float MoveSpeed = 300.f;    // 이동 인터폴레이션 속도 (cm/s). UUnitAnimComponent가 참조

    //연출
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry IdleAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anim|Combat")
    FUnitAnimEntry VictoryAnim;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation")
    const FUnitAnimEntry& GetEntry(EUnitAnimType AnimType) const;

    // UPrimaryDataAsset 식별자 (AssetManager 사용 시)
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
