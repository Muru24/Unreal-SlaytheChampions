// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Particles/ParticleSystem.h"
#include "SkillVFXDataAsset.generated.h"

/*
* ESkillVFXType
* 스킬VFX타입
* Spawn :대상 위치에 즉시 스폰(FireStorm 폭발)
* Projectile : 시전자에서 대상으로 날아가는 발사체
* CasterSpawn :시전자 위치 또는 소켓에 생성
*/
UENUM(BlueprintTYpe)
enum class ESkillVfxType : uint8
{
	Spawn		UMETA(DisplayName = "Spawn on Target"),
	Projectile	UMETA(DisplayName = "Projectile to Target"),
	CasterSpawn	UMETA(DisplayName = "Spawn on Caster")
};

/*
* FSkillVfxEntry
* 스킬 하나에 대응하는 VFX 데이터
* USkillVfxDataAsset의 매핑 배열 원소로 사용
* 
* [Cascase 파티클 사용]
* ParticleSystem :재생할 Cascase 파티클 에셋
* 
* [소켓]
* SocketName이 NAME_None 이 아니면 해당 소켓 위치에 부착
* SpawnOffset은 소켓(또는 액터루트)에서의 로컬 오프셋
* 
* [발사체 전용]
* ProjectileSpeed : 날아가는 속도
* ProjectileClass : 커스텀발사체 클래스
* ImpactParticle : 대상에 도달했을때 추가로 스폰할 파티클
*/
USTRUCT(BlueprintType)
struct SLAYTHECHAMPIONS_API FSkillVfxEntry
{
	GENERATED_BODY()

	//공통
	//재생할 Cascase 파티클 시스템
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UParticleSystem> ParticleSystem = nullptr;

	//VFX 재생방식
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	ESkillVfxType VfxType = ESkillVfxType::Spawn;

	/*
	* 이 Entry를 트리거할 AnimNotify태그
	* AnimNotify_SpawnVfx 의 VfxTag와 일치
	* NAME_None이면 VfxTag 노티파이가 와도 재생
	*/
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "VFX|Timing")
	FName VfxTag = NAME_None;

	/*
	* 스폰 소켓이름
	* NAME_None이면 액터 루트 위치 사용
	* "Caster_Hand_R","Target_Root"
	*/
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "VFX|Socket")
	FName SocketName = NAME_None;

	//소켓 또는 루트 위치에서의 추가 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category ="VFX|Socket")
	FVector SpawnOffset = FVector::ZeroVector;

	//스폰 회전 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Socket")
	FRotator SpawnRotationOffset = FRotator::ZeroRotator;

	//발사체 전용 (VfxType == Proejctile)

	//발사체 이동속도 Projectile 타입에서만 유효
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "VFX|Projectile",
		meta = (ClampMin = "100",EditCondition = "VfxType == ESkillVfxType::Projectile"))
	float ProjectileSpeed = 800.f;

	/*
	* 커스텀 발사체 액터 클래스
	* null 이면 기본 AVfxProjectileActor 사용
	* 별도 시각 효과나 충돌 처리가 필요할 때 지정
	*/
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "VFX|Projectile",
		meta = (EditCondition = "VfxType == ESkillVfxType::Projectile"))
	TSubclassOf<AActor> ProjectileClass;

	/*
	* 대상 도달시 스폰할 충돌 파티클
	* Projectile 타입에서 도착 지점에 추가로 재생
	* null이면 충돌 이펙트 없음
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Projectile",
		meta = (EditCondition = "VfxType == ESkillVfxType::Projectile"))
	TObjectPtr<UParticleSystem> ImpactParticle = nullptr;
};

/*
* FSkillVfxEntryList
* TMap에서 TArray를 값(Value)으로 사용하기 위해 감싸주는 래퍼 구조체
*/
USTRUCT(BlueprintType)
struct FSkillVfxEntryList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TArray<FSkillVfxEntry> Entries;
};


/**
 * USkillVfxDataAsset
 * 스킬 카드 ID(FName)와 VFX 항목을 매핑하는 Primary DataAsset.
 * UVfxComponent가 AnimNotify_SpawnVfx 수신 시 이 에셋을 참조해 파티클을 스폰한다.
 * 
 * [설정법]
 * 1. Content Browser -> 우클릭 -> Data Asset -> SkillVfxDataAsset
 * 2. SkillVfxMap 또는 MultiSkillVfxMap에 카드 ID와 Entry 추가
 * 3. AUnit Blueprint의 UVfxComponent.SkillVfxData에 드래그&드롭
 */
UCLASS(BlueprintType)
class SLAYTHECHAMPIONS_API USkillVFXDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/*
	* 카드 ID ->VFX 항목 단일 매핑
	* 스킬 하나에 Entry 하나만 필요할 때
	* MultiSKillVfxMap보다 낮은 원순위
	* 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Mapping")
	TMap<FName, FSkillVfxEntry> SkillVfxMap;

	/*
	* 카드 ID → VFX 항목 배열 매핑.
	 * 하나의 스킬에 여러 타이밍/파티클이 필요할 때.
	 * SkillVfxMap보다 높은 우선순위.
	* 예) CardID = "Mage_Fireball"
	* [0] VfxTag= "Cast", CasterSpawn, P_Mage_CastEffect
	* [1] VfxTag="Impact", Projectile,  P_Fireball, ImpactParticle=P_Explosion
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Mapping")
	TMap<FName, FSkillVfxEntryList> MultiSkillVfxMap;

	/*
	* CardID + VfxTag로 일치하는 Entry 목록 반환.
	* VfxTag == NAME_None인 Entry는 태그 무관하게 항상 포함.
	* 
	* @param CardID  DT_Cards의 RowName
	* @param VfxTag  AnimNotify_SpawnVfx에서 넘어온 태그 (NAME_None이면 전체)
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX")
	TArray<FSkillVfxEntry> GetVfxEntries(FName CardID, FName VfxTag) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
