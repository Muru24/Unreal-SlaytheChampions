// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CombatTypes.h"
#include "Unit.generated.h"

class UStatComponent;
class UCapsuleComponent;

/**
 * 전투에 참여하는 모든 유닛의 베이스 Pawn.
 * 유닛에는 어떤 컴포넌트도 강제 포함하지 않으며,
 * 서브클래스 또는 외부 코드에서 필요한 컴포넌트를 붙일 능력을 부여한다.
 *-- 애니메이션 연동 -----
 *  공격·이동 타이밍은 UUnitAnimComponent가 아래 델리게이트를 구독해 처리한다.
 *  CombatManager / EffectManager 등 외부 시스템은
 *  NotifyAttack() / NotifyMove()만 호출하면 애니메이션이 자동으로 재생된다.
 */


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDied, AUnit*, Unit);



// 유닛 클릭 시 브로드캐스트 — BattleMainWidget이 구독해 선택 처리
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitClicked, AUnit*, Unit);

// 마우스 호버 시작(true)/종료(false) — 선택 가능 표시(외곽선 등)에 사용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitHovered, AUnit*, Unit, bool, bHovered);

UCLASS()
class SLAYTHECHAMPIONS_API AUnit : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AUnit();

	// 유닛 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit")
	FName UnitID;

	// 유닛 소속
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unit")
	ETeam Team = ETeam::Enemy;

	//사망 이벤트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitDied, AUnit*, Unit);
	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitDied OnUnitDied;

	UFUNCTION(BlueprintCallable, Category = "Unit")
	void HandleDeath();

	// ── 공격 알림 델리게이트 ─────────────────────────────────────────────────
   //
   //  bIsSkill = false → 일반 공격 (Attack 몽타주)
   //  bIsSkill = true  → 스킬/광역 카드 (Skill 몽타주)
   //
   //  [사용 측 예시 — CombatManager.cpp]
   //    if (AUnit* Caster = SpawnedPlayers[CasterIndex])
   //        Caster->NotifyAttack(Card.TargetType == ETargetType::AllEnemies);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitAttackNotified, bool, bIsSkill);
	UPROPERTY(BlueprintAssignable, Category = "Unit|Anim")
	FOnUnitAttackNotified OnUnitAttackNotified;

	/**
	 * 공격 애니메이션 재생 트리거.
	 * CombatManager::ExecuteCard() 직전에 호출한다.
	 * @param bIsSkill true면 Skill 몽타주, false면 Attack 몽타주 재생.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unit|Anim")
	void NotifyAttack(bool bIsSkill = false);

	// ── 이동 알림 델리게이트 ─────────────────────────────────────────────────
	//
	//  [사용 측 예시 — MapManager.cpp or LevelManager.cpp]
	//    if (AUnit* Player = GetPlayerUnit())
	//        Player->NotifyMove(NextAreaWorldLocation);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitMoveNotified, FVector, Destination);
	UPROPERTY(BlueprintAssignable, Category = "Unit|Anim")
	FOnUnitMoveNotified OnUnitMoveNotified;

	// AnimNotify_SpawnVfx -> VfxComponent 연결용 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVfxNotified, FName, VfxTag);
	UPROPERTY(BlueprintAssignable, Category = "Unit|VFX");
	FOnVfxNotified OnVfxNotified;

	/**
	 * 이동 애니메이션 + 위치 인터폴레이션 트리거.
	 * MapManager / LevelManager 등에서 유닛이 새 위치로 이동할 때 호출한다.
	 * @param WorldDestination 이동 목표 월드 좌표.
	 */
	UFUNCTION(BlueprintCallable, Category = "Unit|Anim")
	void NotifyMove(FVector WorldDestination);

	


	// 이 유닛이 클릭될 때 브로드캐스트 (bGenerateClickEvents 활성화 필요)
	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitClicked OnUnitClicked;

	// 마우스 호버 시작(true)/종료(false) 시 브로드캐스트 (bGenerateMouseOverEvents 활성화 필요)
	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitHovered OnUnitHovered;

	// 호버 외곽선 스텐실 값 — 포스트프로세스 외곽선 머티리얼이 이 값으로 대상을 구분 (팀별 색 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Hover")
	int32 HoverStencilValue = 1;

	// 마우스 클릭 감지 전용 캡슐 — Visibility 채널만 Block해 클릭 반경을 넓힘
	// BP에서 캡슐 크기(HalfHeight, Radius)를 유닛마다 조정 가능
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
	UCapsuleComponent* ClickCapsule;

	

	// StatComponent 반환. BeginPlay에서 1회 캐싱하고, 없으면 nullptr (매 호출 Find 비용 절감)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Unit")
	UStatComponent* GetStat() const;

	// StatComponent에서 생존 여부 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Unit")
	bool IsAlive() const;

	// 외곽선 강조 수동 토글 — 타겟팅 중 MainCanvas가 커서 오버를 막을 때 위젯이 직접 호출
	UFUNCTION(BlueprintCallable, Category = "Unit|Hover")
	void SetHoverHighlight(bool bEnabled) { SetHoverOutline(bEnabled); }

	// HP가 필요 없는 유닛도 Unit을 베이스로 쓸 수 있으므로, StatComponent가 없으면 nullptr을 유지한다.
	// (캐싱하더라도 미보유 유닛은 계속 nullptr — 호출 측에서 null 체크로 분기)


protected:

	virtual void BeginPlay() override;

	// 마우스 클릭 시 OnUnitClicked 브로드캐스트
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

	// 마우스 커서 진입/이탈 시 외곽선 토글 + OnUnitHovered 브로드캐스트
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;

private:

	// 모든 MeshComponent의 Custom Depth 렌더링을 켜고/꺼서 외곽선 표시
	void SetHoverOutline(bool bEnabled);

	// GetStat() 캐싱용 — BeginPlay에서 설정. 미보유 유닛이면 nullptr 유지
	UPROPERTY()
	UStatComponent* CachedStat = nullptr;

};
