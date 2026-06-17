// Fill out your copyright notice in the Description page of Project Settings.


#include "VFX/VFXComponent.h"
#include "VFX/SkillVFXDataAsset.h"
#include "VFX/VFXProjectileActor.h"
#include "Unit/Unit.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"


// Sets default values for this component's properties
UVFXComponent::UVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UVFXComponent::BeginPlay()
{
	Super::BeginPlay();
	// AUnit::OnVfxNotified 구독
	// AnimNotify_SpawnVfx 가 Unit 델리게이트를 통해 여기로 신호를 전달
	AUnit* OwnerUnit = Cast<AUnit>(GetOwner());
	if (!OwnerUnit) return;
	OwnerUnit->OnVfxNotified.AddDynamic(this, &UVFXComponent::HandleVfxNotify);
}


void UVFXComponent::SetCurrentCardID(FName CardID)
{
	CurrentCardID = CardID;
}

void UVFXComponent::SetCurrentTarget(AUnit* Target)
{
	CurrentTarget = Target;
	CurrentTargets.Reset();
	if (Target) CurrentTargets.Add(Target);
}

void UVFXComponent::SetCurrentTargets(const TArray<AUnit*>& Targets)
{
	CurrentTargets.Reset();
	CurrentTarget = nullptr;
	for (AUnit* T : Targets)
	{
		if (T) CurrentTargets.Add(T);
	}
	if (CurrentTargets.Num() > 0) CurrentTarget = CurrentTargets[0];
}

void UVFXComponent::ClearCurrentState()
{
	CurrentCardID = NAME_None;
	CurrentTarget = nullptr;
	CurrentTargets.Reset();
}

void UVFXComponent::PlayVfxDirect(FName CardID, AUnit* Target, FName VfxTag)
{
	if (!SkillVfxData) return;
	const TArray<FSkillVfxEntry> Entries = SkillVfxData->GetVfxEntries(CardID, VfxTag);
	if (Entries.IsEmpty()) return;

	TArray<TWeakObjectPtr<AUnit>> TargetArr;
	if (Target) TargetArr.Add(Target);
	ProcessVfxEntries(Entries, TargetArr);
}

void UVFXComponent::PlayVfxDirectMulti(FName CardID, const TArray<AUnit*>& Targets, FName VfxTag)
{
	if (!SkillVfxData) return;
	const TArray<FSkillVfxEntry> Entries = SkillVfxData->GetVfxEntries(CardID, VfxTag);
	if (Entries.IsEmpty()) return;

	TArray<TWeakObjectPtr<AUnit>> WeakTargets;
	for (AUnit* T : Targets) { if (T) WeakTargets.Add(T); }
	ProcessVfxEntries(Entries, WeakTargets);
}




void UVFXComponent::HandleVfxNotify(FName VfxTag)
{
	if (!SkillVfxData || CurrentCardID == NAME_None) return;
	// VfxTag를 키로 이 타이밍에 재생해야 할 Entry만 조회
	const TArray<FSkillVfxEntry> Entries =
		SkillVfxData->GetVfxEntries(CurrentCardID, VfxTag);

	if (Entries.IsEmpty())
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("[VfxComponent] CardID='%s' VfxTag='%s' - 매핑 없음"),
			*CurrentCardID.ToString(), *VfxTag.ToString());
		return;
	}

	ProcessVfxEntries(Entries, CurrentTargets);
}

void UVFXComponent::ProcessVfxEntries(const TArray<FSkillVfxEntry>& Entries, 
	const TArray<TWeakObjectPtr<AUnit>>& Targets)
{
	for (const FSkillVfxEntry& Entry : Entries)
	{
		if (!Entry.ParticleSystem) continue;

		if (Targets.IsEmpty())
		{
			if (Entry.VfxType == ESkillVfxType::CasterSpawn)
				ExecuteEntry(Entry, nullptr);
			continue;
		}

		for (const TWeakObjectPtr<AUnit>& Target : Targets)
			ExecuteEntry(Entry, Target);
	}
}

void UVFXComponent::ExecuteEntry(const FSkillVfxEntry& Entry, TWeakObjectPtr<AUnit> Target)
{
	switch (Entry.VfxType)
	{
	case ESkillVfxType::Spawn:       SpawnAtTarget(Entry, Target.Get());   break;
	case ESkillVfxType::Projectile:  LaunchProjectile(Entry, Target.Get()); break;
	case ESkillVfxType::CasterSpawn: SpawnAtCaster(Entry);                  break;
	}
}

UParticleSystemComponent* UVFXComponent::SpawnAtTarget(const FSkillVfxEntry& Entry, AUnit* Target)
{
	if (!Entry.ParticleSystem) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	AActor* LocationActor = Target ? static_cast<AActor*>(Target) : GetOwner();
	if (!LocationActor) return nullptr;

	const FTransform T = ResolveSpawnTransform(
		LocationActor, Entry.SocketName, Entry.SpawnOffset, Entry.SpawnRotationOffset);

	return UGameplayStatics::SpawnEmitterAtLocation(
		World, Entry.ParticleSystem, T, true);
}

void UVFXComponent::LaunchProjectile(const FSkillVfxEntry& Entry, AUnit* Target)
{
	if (!Entry.ParticleSystem || !Target) return;

	UWorld* World = GetWorld();
	AActor* Caster = GetOwner();
	if (!World || !Caster) return;

	const FTransform LaunchTransform = ResolveSpawnTransform(
		Caster, Entry.SocketName, Entry.SpawnOffset, Entry.SpawnRotationOffset);

	TSubclassOf<AActor> ProjClass;
	if (Entry.ProjectileClass)
		ProjClass = Entry.ProjectileClass;
	else
		ProjClass = AVFXProjectileActor::StaticClass();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Proj = World->SpawnActor<AActor>(ProjClass, LaunchTransform, Params);
	if (!Proj)
	{
		return;
	}

	AVFXProjectileActor* VfxProj = Cast<AVFXProjectileActor>(Proj);
	if (!VfxProj)
	{
		Proj->Destroy();
		return;
	}

	VfxProj->Launch(Entry.ParticleSystem, Target, Entry.ProjectileSpeed, Entry.ImpactParticle);

	UE_LOG(LogTemp, Log, TEXT("[VFXComponent] Projectile '%s' -> '%s' (Speed: %.0f)"), *Entry.ParticleSystem->GetName(), *Target->GetName(), Entry.ProjectileSpeed);
}

UParticleSystemComponent* UVFXComponent::SpawnAtCaster(const FSkillVfxEntry& Entry)
{
	if (!Entry.ParticleSystem) return nullptr;
	UWorld* World = GetWorld();
	AActor* Caster = GetOwner();
	if (!World || !Caster) return nullptr;

	const FTransform T = ResolveSpawnTransform(
		Caster, Entry.SocketName, Entry.SpawnOffset, Entry.SpawnRotationOffset);

	return UGameplayStatics::SpawnEmitterAtLocation(
		World, Entry.ParticleSystem, T, true);
}

FTransform UVFXComponent::ResolveSpawnTransform(AActor* Actor, FName SocketName, FVector Offset, FRotator RotOffset) const
{
	if (!Actor) return FTransform::Identity;

	FVector  Location = Actor->GetActorLocation();
	FRotator Rotation = Actor->GetActorRotation();

	if (SocketName != NAME_None)
	{
		if (USkeletalMeshComponent* Mesh =
			Actor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(SocketName))
			{
				Location = Mesh->GetSocketLocation(SocketName);
				Rotation = Mesh->GetSocketRotation(SocketName);
			}
		}
	}

	Location += Rotation.RotateVector(Offset);
	Rotation = (Rotation.Quaternion() * RotOffset.Quaternion()).Rotator();

	return FTransform(Rotation, Location);
}
