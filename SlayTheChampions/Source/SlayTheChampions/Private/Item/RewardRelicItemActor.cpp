#include "Item/RewardRelicItemActor.h"

#include "GameFramework/PlayerController.h"
#include "Party/PartyInstance.h"
#include "Relic/RelicSubsystem.h"

ARewardRelicItemActor::ARewardRelicItemActor()
{
	ItemType = EItemActorType::Relic;
	Price = 0;
}

void ARewardRelicItemActor::BeginPlay()
{
	if (ItemID.IsNone())
	{
		RollRewardRelic();
	}

	Super::BeginPlay();
}

void ARewardRelicItemActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	AActor::NotifyActorOnClicked(ButtonPressed);

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (bFocused)
	{
		ClaimRewardRelic(PlayerController);
		return;
	}

	FocusItemCamera(PlayerController);
}

bool ARewardRelicItemActor::RollRewardRelic()
{
	URelicSubsystem* RelicSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URelicSubsystem>() : nullptr;
	if (!RelicSubsystem)
	{
		return false;
	}

	FName RolledRelicID = NAME_None;
	switch (RewardRelicSourceType)
	{
	case ERelicSourceType::Shop:
		RolledRelicID = RelicSubsystem->GetRandomShopRelic();
		break;
	case ERelicSourceType::Event:
	{
		const TArray<FRelic> EventRelics = RelicSubsystem->GetEventRelics();
		if (!EventRelics.IsEmpty())
		{
			RolledRelicID = EventRelics[FMath::RandRange(0, EventRelics.Num() - 1)].RelicID;
		}
		break;
	}
	case ERelicSourceType::Default:
	{
		const TArray<FRelic> DefaultRelics = RelicSubsystem->GetDefaultRelics();
		if (!DefaultRelics.IsEmpty())
		{
			RolledRelicID = DefaultRelics[FMath::RandRange(0, DefaultRelics.Num() - 1)].RelicID;
		}
		break;
	}
	case ERelicSourceType::Common:
	default:
		RolledRelicID = RelicSubsystem->GetRandomCommonRelic();
		break;
	}

	if (RolledRelicID.IsNone())
	{
		return false;
	}

	InitItem(EItemActorType::Relic, RolledRelicID);
	OnRewardRelicRolled(CachedRelicData);
	return bHasValidItemData;
}

bool ARewardRelicItemActor::ClaimRewardRelic(AActor* Interactor)
{
	if (bCollected)
	{
		return false;
	}

	if (!bHasValidItemData && !RefreshItemData())
	{
		return false;
	}

	UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	if (!PartyInstance)
	{
		return false;
	}

	PartyInstance->AddRelic(CachedRelicData);
	OnRewardRelicClaimed(CachedRelicData, Interactor);
	OnFocusedItemConfirmed(Interactor);
	MarkCollected();
	return true;
}
