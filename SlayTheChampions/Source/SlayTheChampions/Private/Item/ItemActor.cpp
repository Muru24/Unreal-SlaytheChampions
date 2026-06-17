#include "Item/ItemActor.h"

#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Item/ItemVisualDataAsset.h"
#include "Party/PartyInstance.h"
#include "Potion/PotionSubsystem.h"
#include "Relic/RelicSubsystem.h"

namespace
{
	TWeakObjectPtr<AItemActor> FocusedItemActor;
}

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

	ItemCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ItemCamera"));
	ItemCamera->SetupAttachment(SceneRoot);
	ItemCamera->SetRelativeLocation(FVector(-180.f, 0.f, 80.f));
	ItemCamera->SetRelativeRotation(FRotator(-12.f, 0.f, 0.f));
	ItemCamera->bAutoActivate = true;
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshItemData();
	ApplyItemVisual();
}

void AItemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelItemFocus();
	Super::EndPlay(EndPlayReason);
}

void AItemActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (bFocused)
	{
		ConfirmFocusedItem(PlayerController);
		return;
	}

	FocusItemCamera(PlayerController);
}

void AItemActor::InitItem(EItemActorType InItemType, FName InItemID)
{
	ItemType = InItemType;
	ItemID = InItemID;
	bCollected = false;
	RefreshItemData();
	ApplyItemVisual();
}

void AItemActor::SetPrice(int32 InPrice)
{
	Price = FMath::Max(0, InPrice);
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

	if (!TryPayPrice())
	{
		OnItemPurchaseFailed();
		return;
	}

	OnItemInteracted(Interactor);
}

bool AItemActor::CanAfford() const
{
	if (Price <= 0)
	{
		return true;
	}

	const UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	return PartyInstance && PartyInstance->GetPartyInfo().Gold >= Price;
}

bool AItemActor::TryPayPrice()
{
	if (Price <= 0)
	{
		return true;
	}

	UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	if (!PartyInstance || PartyInstance->GetPartyInfo().Gold < Price)
	{
		return false;
	}

	PartyInstance->UseGold(Price);
	return true;
}

void AItemActor::FocusItemCamera(APlayerController* PlayerController)
{
	if (bCollected || !PlayerController)
	{
		return;
	}

	if (FocusedItemActor.IsValid() && FocusedItemActor.Get() != this)
	{
		FocusedItemActor->CancelItemFocus();
	}

	PreviousViewTarget = PlayerController->GetViewTarget();
	bFocused = true;
	FocusedItemActor = this;

	EnableInput(PlayerController);
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AItemActor::HandleFocusedWorldClick);
	}

	PlayerController->SetViewTargetWithBlend(this, FocusBlendTime);
	OnItemFocused();
}

void AItemActor::CancelItemFocus()
{
	if (!bFocused)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PlayerController)
	{
		AActor* RestoreTarget = PreviousViewTarget ? PreviousViewTarget.Get() : Cast<AActor>(PlayerController->GetPawn());
		if (RestoreTarget)
		{
			PlayerController->SetViewTargetWithBlend(RestoreTarget, FocusBlendTime);
		}

		DisableInput(PlayerController);
	}

	PreviousViewTarget = nullptr;
	bFocused = false;

	if (FocusedItemActor.Get() == this)
	{
		FocusedItemActor.Reset();
	}

	OnItemFocusCanceled();
}

void AItemActor::ConfirmFocusedItem(AActor* Interactor)
{
	if (bCollected)
	{
		return;
	}

	Interact(Interactor);
	OnFocusedItemConfirmed(Interactor);
}

void AItemActor::HandleFocusedWorldClick()
{
	if (!bFocused || bCollected || FocusedItemActor.Get() != this)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FHitResult HitResult;
	PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if (HitResult.GetActor() == this)
	{
		return;
	}

	CancelItemFocus();
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

FText AItemActor::GetItemName() const
{
	return GetDisplayName();
}

FText AItemActor::GetItemDescription() const
{
	return GetDescription();
}

void AItemActor::MarkCollected()
{
	if (bCollected)
	{
		return;
	}

	CancelItemFocus();
	bCollected = true;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	OnItemCollected();
}
