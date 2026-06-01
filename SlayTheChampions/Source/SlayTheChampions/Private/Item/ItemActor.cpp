#include "Item/ItemActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/ItemVisualDataAsset.h"
#include "Potion/PotionSubsystem.h"
#include "Relic/RelicSubsystem.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(SceneRoot);
	InteractionBox->SetBoxExtent(FVector(60.f, 60.f, 60.f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshItemData();
	ApplyItemVisual();
}

void AItemActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	Interact(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
}

void AItemActor::InitItem(EItemActorType InItemType, FName InItemID)
{
	ItemType = InItemType;
	ItemID = InItemID;
	bCollected = false;
	RefreshItemData();
	ApplyItemVisual();
}

void AItemActor::SetItemVisualDataAsset(UItemVisualDataAsset* InItemVisualDataAsset)
{
	ItemVisualDataAsset = InItemVisualDataAsset;
	ApplyItemVisual();
}

bool AItemActor::RefreshItemData()
{
	bHasValidItemData = false;
	CachedRelicData = FRelic();
	CachedPotionData = FPotionData();

	if (ItemID.IsNone())
	{
		OnItemDataRefreshed();
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		OnItemDataRefreshed();
		return false;
	}

	switch (ItemType)
	{
	case EItemActorType::Relic:
		if (URelicSubsystem* RelicSubsystem = GameInstance->GetSubsystem<URelicSubsystem>())
		{
			bHasValidItemData = RelicSubsystem->GetCachedRelicData(ItemID, CachedRelicData);
		}
		break;

	case EItemActorType::Potion:
		if (UPotionSubsystem* PotionSubsystem = GameInstance->GetSubsystem<UPotionSubsystem>())
		{
			bHasValidItemData = PotionSubsystem->GetCachedPotionData(ItemID, CachedPotionData);
		}
		break;

	default:
		break;
	}

	OnItemDataRefreshed();
	return bHasValidItemData;
}

bool AItemActor::ApplyItemVisual()
{
	if (!ItemVisualDataAsset || !MeshComponent || ItemID.IsNone())
	{
		return false;
	}

	FItemVisualData VisualData;
	if (!ItemVisualDataAsset->FindVisualData(ItemType, ItemID, VisualData))
	{
		return false;
	}

	if (VisualData.Mesh)
	{
		MeshComponent->SetStaticMesh(VisualData.Mesh);
	}

	if (VisualData.Material)
	{
		MeshComponent->SetMaterial(0, VisualData.Material);
	}

	return true;
}

void AItemActor::Interact(AActor* Interactor)
{
	if (bCollected)
	{
		return;
	}

	OnItemInteracted(Interactor);
}

FText AItemActor::GetDisplayName() const
{
	switch (ItemType)
	{
	case EItemActorType::Relic:
		return CachedRelicData.RelicName;
	case EItemActorType::Potion:
		return CachedPotionData.PotionName;
	default:
		return FText::GetEmpty();
	}
}

FText AItemActor::GetDescription() const
{
	switch (ItemType)
	{
	case EItemActorType::Relic:
		return CachedRelicData.Description;
	case EItemActorType::Potion:
		return CachedPotionData.Description;
	default:
		return FText::GetEmpty();
	}
}

void AItemActor::MarkCollected()
{
	if (bCollected)
	{
		return;
	}

	bCollected = true;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	OnItemCollected();
}
