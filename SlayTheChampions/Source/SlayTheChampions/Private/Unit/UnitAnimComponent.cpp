// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/UnitAnimComponent.h"
#include "Unit/Unit.h"
#include "Unit/StatComponent.h"
#include "Unit/UnitAnimDataAsset.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
UUnitAnimComponent::UUnitAnimComponent()
{
	// 이동 인터폴레이션을 Tick에서 처리하므로 활성화
	PrimaryComponentTick.bCanEverTick = true;
	// 기본은 비활성화, 이동 시작 시 활성화
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void UUnitAnimComponent::BeginPlay()
{
	Super::BeginPlay();
	TryCacheSkeletalMesh();
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	if (!OwnerUnit) return;

	//StatComponent::OnHPChanged ->피격애니메이션
	if (bAutoPlayHitOnDamage)
	{
		if (UStatComponent* Stat = OwnerUnit->FindComponentByClass<UStatComponent>())
		{
			Stat->OnHPChanged.AddUniqueDynamic(this, &UUnitAnimComponent::HandleHPChanged);
		}
	}

	//StatComponent::OnUnitDied ->사망 애니메이션
	if (bAutoPlayDeath)
	{
		OwnerUnit->OnUnitDied.AddUniqueDynamic(this, &UUnitAnimComponent::HandleUnitDied);
	}

	// Unit::OnUnitAttackNotified -> 공격/스킬 애니메이션
	OwnerUnit->OnUnitAttackNotified.AddUniqueDynamic(this, &UUnitAnimComponent::HandleAttackNotified);

	// Unit::OnUnitMoveNotified -> 이동 애니메이션
	OwnerUnit->OnUnitMoveNotified.AddUniqueDynamic(this, &UUnitAnimComponent::HandleMoveNotified);
	
}


// Called every frame
void UUnitAnimComponent::TickComponent(float DeltaTime, ELevelTick TickType,
										FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsMoving || !GetOwner()) return;

	const float Speed = AnimData ? AnimData->MoveSpeed : 300.f;
	const float Step = Speed * DeltaTime;
	MoveTraveledDistance += Step;

	if (MoveTotalDistance <= 0.f || MoveTraveledDistance >= MoveTotalDistance)
	{
		// 이동 완료
		GetOwner()->SetActorLocation(MoveDestination);
		bIsMoving = false;
		PrimaryComponentTick.SetTickFunctionEnable(false);
		OnMoveFinished.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("[UnitAnimComponent] %s Move 완료"), *GetOwner()->GetName());
	}
	else
	{
		const float Alpha = MoveTotalDistance > 0.f ? MoveTraveledDistance / MoveTotalDistance : 1.f;
		const FVector NewLoc = FMath::Lerp(MoveStartLocation, MoveDestination, Alpha);
		GetOwner()->SetActorLocation(NewLoc);
	}
}


float UUnitAnimComponent::Play(EUnitAnimType AnimType)
{
	if (!AnimData) return 0.f;
	const FUnitAnimEntry& Entry = AnimData->GetEntry(AnimType);
	return PlayMontageInternal(Entry, AnimType);
}

void UUnitAnimComponent::Stop(float BlendOutTime)
{
	if (!TargetMesh) return;
	UAnimInstance* AnimInst = TargetMesh->GetAnimInstance();
	if (!AnimInst || !CurrentMontage) return;

	AnimInst->Montage_Stop(BlendOutTime < 0.f ? 0.25f : BlendOutTime, CurrentMontage);
}

void UUnitAnimComponent::PlayMove(FVector WorldDestination)
{
	if (!GetOwner()) return;

	MoveStartLocation = GetOwner()->GetActorLocation();
	MoveDestination = WorldDestination;
	MoveTotalDistance = FVector::Dist(MoveStartLocation, MoveDestination);
	MoveTraveledDistance = 0.f;
	bIsMoving = true;

	PrimaryComponentTick.SetTickFunctionEnable(true);

	// 이동 애니메이션 재생
	Play(EUnitAnimType::Move);

	UE_LOG(LogTemp, Log, TEXT("[UnitAnimComponent] %s Move 시작 → %s (%.0f cm)"),
		*GetOwner()->GetName(), *WorldDestination.ToString(), MoveTotalDistance);
}

bool UUnitAnimComponent::IsPlayingAnim(EUnitAnimType AnimType) const
{
	if (!TargetMesh || !CurrentMontage) return false;
	const UAnimInstance* AnimInst = TargetMesh->GetAnimInstance();
	return AnimInst && CurrentAnimType == AnimType && AnimInst->Montage_IsPlaying(CurrentMontage);
}


void UUnitAnimComponent::HandleHPChanged(int32 OldHP, int32 NewHP)
{
	// HP가 줄었을 때만 Hit 재생 (회복은 무시)
	if (NewHP < OldHP)
	{
		Play(EUnitAnimType::Hit);
	}
}

void UUnitAnimComponent::HandleUnitDied(AUnit* Unit)
{
	Play(EUnitAnimType::Death);
}

void UUnitAnimComponent::HandleAttackNotified(bool bIsSkill)
{
	Play(bIsSkill ? EUnitAnimType::Skill : EUnitAnimType::Attack);
}

void UUnitAnimComponent::HandleMoveNotified(FVector Destination)
{
	PlayMove(Destination);
}

void UUnitAnimComponent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentMontage) return;

	const EUnitAnimType FinishedType = CurrentAnimType;
	CurrentMontage = nullptr;
	CurrentAnimType = EUnitAnimType::Idle;

	OnAnimFinished.Broadcast(FinishedType);
}

float UUnitAnimComponent::PlayMontageInternal(const FUnitAnimEntry& Entry, EUnitAnimType AnimType)
{
	if (!Entry.Montage)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[UnitAnimComponent] %s — AnimType %d 몽타주 없음, 건너뜀"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("?"), (int32)AnimType);
		return 0.f;
	}

	if (!TargetMesh) TryCacheSkeletalMesh();
	if (!TargetMesh) return 0.f;

	UAnimInstance* AnimInst = TargetMesh->GetAnimInstance();
	if (!AnimInst) return 0.f;

	const float Length = AnimInst->Montage_Play(Entry.Montage, Entry.PlayRate,
		EMontagePlayReturnType::MontageLength,
		0.f, true);
	if (Length <= 0.f) return 0.f;

	// 시작 섹션 점프
	if (Entry.StartSection != NAME_None)
	{
		AnimInst->Montage_JumpToSection(Entry.StartSection, Entry.Montage);
	}

	CurrentMontage = Entry.Montage;
	CurrentAnimType = AnimType;

	// 종료 감지 등록 — 멀티캐스트(OnMontageEnded) 대신 이 몽타주 인스턴스 전용 단일 델리게이트 사용
	// 멀티캐스트 목록을 매 재생마다 Add/Remove 하지 않으므로, 몽타주 중단(interrupt) 시
	// OnMontageEnded 브로드캐스트 도중 목록이 수정되어 ensure가 터지던 문제를 회피한다.
	FOnMontageEnded EndDel;
	EndDel.BindUObject(this, &UUnitAnimComponent::HandleMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDel, Entry.Montage);

	OnAnimStarted.Broadcast(AnimType, Length);
	UE_LOG(LogTemp, Log, TEXT("[UnitAnimComponent] %s — %s 재생 (%.2fs)"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("?"),
		*UEnum::GetValueAsString(AnimType), Length);

	return Length;
}

void UUnitAnimComponent::TryCacheSkeletalMesh()
{
	if (TargetMesh) return;
	if (AActor* Owner = GetOwner())
	{
		TargetMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (!TargetMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UnitAnimComponent] %s — SkeletalMeshComponent 없음. "
				"에디터에서 TargetMesh를 직접 지정하거나 SkeletalMeshComponent를 추가하세요."),
				*Owner->GetName());
		}
	}
}
