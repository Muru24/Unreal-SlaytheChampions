// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Unit.h"
#include "Unit/StatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"

// Sets default values
AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;
	// NotifyActorOnClicked 수신 조건: PlayerController BP에서 bEnableClickEvents = true 설정 필요

	// 클릭 감지 전용 캡슐을 루트로 생성
	// Visibility 채널만 Block — 물리/이동에는 영향 없이 마우스 클릭 반경만 확장
	// 클릭 감지 전용 캡슐 — 크기는 BP에서 유닛마다 조정
	ClickCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ClickCapsule"));
	SetRootComponent(ClickCapsule);
	ClickCapsule->InitCapsuleSize(60.f, 100.f); // BP에서 유닛마다 덮어쓸 수 있는 기본값
	ClickCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClickCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClickCapsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();

	// StatComponent 1회 캐싱 (없는 유닛이면 nullptr 유지)
	CachedStat = FindComponentByClass<UStatComponent>();
}

void AUnit::HandleDeath()
{
	OnUnitDied.Broadcast(this);
}

void AUnit::NotifyAttack(bool bIsSkill)
{
	OnUnitAttackNotified.Broadcast(bIsSkill);
}

void AUnit::NotifyMove(FVector WorldDestination)
{
	OnUnitMoveNotified.Broadcast(WorldDestination);
}



UStatComponent* AUnit::GetStat() const
{
	// 캐시 우선, BeginPlay 전이거나 미캐싱이면 Find로 폴백
	return CachedStat ? CachedStat : FindComponentByClass<UStatComponent>();
}

bool AUnit::IsAlive() const
{
	if (const UStatComponent* Stat = GetStat())
	{
		return Stat->CurrentHP > 0;
	}

	// StatComponent가 없으면 죽은 것으로 간주 (전투 대상에서 제외)
	return false;
}

// 마우스 클릭 시 OnUnitClicked 브로드캐스트
void AUnit::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	OnUnitClicked.Broadcast(this);
}

// 마우스 커서 진입 — 외곽선 켜기 + 호버 알림
void AUnit::NotifyActorBeginCursorOver()
{
	Super::NotifyActorBeginCursorOver();
	SetHoverOutline(true);
	OnUnitHovered.Broadcast(this, true);
}

// 마우스 커서 이탈 — 외곽선 끄기 + 호버 해제 알림
void AUnit::NotifyActorEndCursorOver()
{
	Super::NotifyActorEndCursorOver();
	SetHoverOutline(false);
	OnUnitHovered.Broadcast(this, false);
}

// 유닛의 모든 MeshComponent(Static/Skeletal/Paper)에 Custom Depth 렌더링 토글
// 포스트프로세스 외곽선 머티리얼이 Custom Depth Stencil을 읽어 윤곽선을 그린다
void AUnit::SetHoverOutline(bool bEnabled)
{
	TArray<UMeshComponent*> Meshes;
	GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;
		Mesh->SetRenderCustomDepth(bEnabled);
		if (bEnabled)
			Mesh->SetCustomDepthStencilValue(HoverStencilValue);
	}
}