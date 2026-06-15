#include "Party/CharacterSelectActor.h"

#include "Animation/AnimationAsset.h"
#include "Components/BoxComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Party/PartyInstance.h"
#include "Party/CharacterSelectVisualDataAsset.h"
#include "Card/CardSubsystem.h"

ACharacterSelectActor::ACharacterSelectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(SceneRoot);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ClickBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ClickBox"));
	ClickBox->SetupAttachment(SceneRoot);
	ClickBox->SetBoxExtent(FVector(80.f, 80.f, 120.f));
	ClickBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClickBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClickBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ACharacterSelectActor::BeginPlay()
{
	Super::BeginPlay();

	LoadDefaultVisualDataAsset();
	LoadUnitDataByID();
}

void ACharacterSelectActor::SetUnitID(FName InUnitID)
{
	CharacterInfo.UnitID = InUnitID;
	LoadUnitDataByID();
}

bool ACharacterSelectActor::ApplyVisualDataByUnitID()
{
	if (!PreviewMesh)
	{
		return false;
	}

	LoadDefaultVisualDataAsset();

	if (!VisualDataAsset || CharacterInfo.UnitID.IsNone())
	{
		return false;
	}

	FCharacterSelectVisualData VisualData;
	if (!VisualDataAsset->FindVisualData(CharacterInfo.UnitID, VisualData))
	{
		return false;
	}

	// UnitID에 연결된 데이터 에셋 값으로 선택창의 스켈레탈 메쉬와 기본 애니메이션을 세팅한다.
	PreviewMesh->SetRelativeTransform(VisualData.RelativeTransform);

	if (VisualData.SkeletalMesh)
	{
		PreviewMesh->SetSkeletalMesh(VisualData.SkeletalMesh);
	}

	for (int32 MaterialIndex = 0; MaterialIndex < VisualData.OverrideMaterials.Num(); ++MaterialIndex)
	{
		if (VisualData.OverrideMaterials[MaterialIndex])
		{
			PreviewMesh->SetMaterial(MaterialIndex, VisualData.OverrideMaterials[MaterialIndex]);
		}
	}

	if (VisualData.AnimClass)
	{
		PreviewMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		PreviewMesh->SetAnimInstanceClass(VisualData.AnimClass);
	}
	else if (VisualData.IdleAnimation)
	{
		PreviewMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		PreviewMesh->SetAnimation(VisualData.IdleAnimation);
		PreviewMesh->Play(VisualData.bLoopIdleAnimation);
	}

	return true;
}

bool ACharacterSelectActor::SelectCharacter()
{
	if (bSelected)
	{
		return true;
	}

	if (CharacterInfo.UnitID.IsNone())
	{
		return false;
	}

	UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	if (!PartyInstance)
	{
		return false;
	}

	// 선택 시 파티 인스턴스에 UnitID와 직업을 저장한다.
	const bool bAdded = PartyInstance->AddPartyMember(CharacterInfo.UnitID, CharacterInfo.JobClass);
	if (!bAdded)
	{
		return false;
	}

	// 스타터 덱 등록 및 시드 — StarterDeckTable이 지정돼 있으면 CardSubsystem에 등록 후 즉시 PartyInstance에 저장
	if (UCardSubsystem* CS = GetGameInstance()->GetSubsystem<UCardSubsystem>())
	{
		if (CharacterInfo.StarterDeckTable)
			CS->RegisterStarterDeck(CharacterInfo.JobClass, CharacterInfo.StarterDeckTable);

		// AddPartyMember가 ChampionJobs에 방금 추가했으므로 마지막 인덱스가 이 캐릭터의 PawnIndex
		const int32 PawnIndex = PartyInstance->GetChampionJobs().Num() - 1;
		if (PawnIndex >= 0)
			PartyInstance->SetDeck(PawnIndex, CS->GetStarterDeckNames(CharacterInfo.JobClass));
	}

	bSelected = true;
	if (bDisableAfterSelected && ClickBox)
	{
		ClickBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnCharacterSelected.Broadcast(this, CharacterInfo);
	OnCharacterSelectionChanged.Broadcast(this, CharacterInfo, bSelected);
	OnSelectionStateChanged(bSelected);
	return true;
}

bool ACharacterSelectActor::DeselectCharacter()
{
	if (!bSelected)
	{
		return true;
	}

	if (CharacterInfo.UnitID.IsNone())
	{
		return false;
	}

	UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	if (!PartyInstance)
	{
		return false;
	}

	// 다시 클릭하면 파티에서 빠지게 해서 선택 토글처럼 동작한다.
	const bool bRemoved = PartyInstance->RemovePartyMember(CharacterInfo.UnitID, CharacterInfo.JobClass);
	if (!bRemoved)
	{
		return false;
	}

	bSelected = false;
	if (ClickBox)
	{
		ClickBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	OnCharacterSelectionChanged.Broadcast(this, CharacterInfo, bSelected);
	OnSelectionStateChanged(bSelected);
	return true;
}

bool ACharacterSelectActor::ToggleCharacterSelection()
{
	return bSelected ? DeselectCharacter() : SelectCharacter();
}

void ACharacterSelectActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	ToggleCharacterSelection();
}

void ACharacterSelectActor::NotifyActorBeginCursorOver()
{
	Super::NotifyActorBeginCursorOver();
	SetHoverOutline(true);
}

void ACharacterSelectActor::NotifyActorEndCursorOver()
{
	Super::NotifyActorEndCursorOver();
	SetHoverOutline(false);
}

void ACharacterSelectActor::LoadUnitDataByID()
{
	if (CharacterInfo.UnitID.IsNone())
	{
		return;
	}

	if (CharacterInfo.DisplayName.IsEmpty())
	{
		CharacterInfo.DisplayName = FText::FromName(CharacterInfo.UnitID);
	}

	ApplyVisualDataByUnitID();
	OnCharacterInfoLoaded(CharacterInfo);
}

void ACharacterSelectActor::LoadDefaultVisualDataAsset()
{
	if (VisualDataAsset || !bLoadDefaultVisualDataAsset)
	{
		return;
	}

	static const TCHAR* VisualDataAssetPath = TEXT("/Game/04_Data/CharacterSelectVisualData.CharacterSelectVisualData");
	VisualDataAsset = LoadObject<UCharacterSelectVisualDataAsset>(nullptr, VisualDataAssetPath);
}

void ACharacterSelectActor::SetHoverOutline(bool bEnabled)
{
	TArray<UMeshComponent*> Meshes;
	GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (!Mesh)
		{
			continue;
		}

		Mesh->SetRenderCustomDepth(bEnabled);
		if (bEnabled)
		{
			Mesh->SetCustomDepthStencilValue(1);
		}
	}
}
