#include "Shop/CardShopFrameActor.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameManagers/CardManager.h"
#include "InputCoreTypes.h"
#include "Party/PartyInstance.h"

namespace
{
	TWeakObjectPtr<ACardShopFrameActor> FocusedCardShopFrame;
}

ACardShopFrameActor::ACardShopFrameActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FrameInteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FrameInteractionBox"));
	FrameInteractionBox->SetupAttachment(SceneRoot);
	FrameInteractionBox->SetBoxExtent(FVector(180.f, 40.f, 120.f));
	FrameInteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FrameInteractionBox->SetCollisionObjectType(ECC_WorldDynamic);
	FrameInteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	FrameInteractionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	FrameCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FrameCamera"));
	FrameCamera->SetupAttachment(SceneRoot);
	FrameCamera->SetRelativeLocation(FVector(-260.f, 0.f, 95.f));
	FrameCamera->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
	FrameCamera->bAutoActivate = true;
}

void ACardShopFrameActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelFrameFocus();
	Super::EndPlay(EndPlayReason);
}

void ACardShopFrameActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	if (!bFocused)
	{
		FocusFrameCamera(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
	}
}

void ACardShopFrameActor::InitCards(const TArray<FName>& CardIDs, const TArray<int32>& Prices)
{
	SaleCards.Reset();

	const int32 CardCount = FMath::Min(CardIDs.Num(), Prices.Num());
	for (int32 Index = 0; Index < CardCount; ++Index)
	{
		if (CardIDs[Index].IsNone())
		{
			continue;
		}

		FShopCardSaleData SaleData;
		SaleData.CardID = CardIDs[Index];
		SaleData.Price = FMath::Max(0, Prices[Index]);
		SaleData.bPurchased = false;
		SaleCards.Add(SaleData);
	}

	OnCardsInitialized(SaleCards);
}

void ACardShopFrameActor::FocusFrameCamera(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (FocusedCardShopFrame.IsValid() && FocusedCardShopFrame.Get() != this)
	{
		FocusedCardShopFrame->CancelFrameFocus();
	}

	PreviousViewTarget = PlayerController->GetViewTarget();
	bFocused = true;
	FocusedCardShopFrame = this;

	EnableInput(PlayerController);
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACardShopFrameActor::HandleFocusedWorldClick);
	}

	PlayerController->SetViewTargetWithBlend(this, FocusBlendTime);
	OnFrameFocused();
}

void ACardShopFrameActor::CancelFrameFocus()
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

	if (FocusedCardShopFrame.Get() == this)
	{
		FocusedCardShopFrame.Reset();
	}

	OnFrameFocusCanceled();
}

bool ACardShopFrameActor::TryPurchaseCard(int32 CardIndex, int32 PawnIndex)
{
	if (!SaleCards.IsValidIndex(CardIndex))
	{
		return false;
	}

	FShopCardSaleData& SaleData = SaleCards[CardIndex];
	if (SaleData.bPurchased || SaleData.CardID.IsNone() || !CanAfford(SaleData.Price))
	{
		OnCardPurchaseFailed(CardIndex, SaleData);
		return false;
	}

	int32 AddedPawnIndex = INDEX_NONE;
	if (UCardManager* CardManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCardManager>() : nullptr)
	{
		if (!CardManager->TryPurchaseShopCardForParty(SaleData.CardID, SaleData.Price, AddedPawnIndex))
		{
			OnCardPurchaseFailed(CardIndex, SaleData);
			return false;
		}
	}
	else
	{
		OnCardPurchaseFailed(CardIndex, SaleData);
		return false;
	}

	SaleData.bPurchased = true;
	OnCardPurchased(CardIndex, SaleData, AddedPawnIndex);
	return true;
}

bool ACardShopFrameActor::GetSaleCardData(int32 CardIndex, FShopCardSaleData& OutSaleData) const
{
	if (!SaleCards.IsValidIndex(CardIndex))
	{
		return false;
	}

	OutSaleData = SaleCards[CardIndex];
	return true;
}

void ACardShopFrameActor::HandleFocusedWorldClick()
{
	if (!bFocused || FocusedCardShopFrame.Get() != this)
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

	CancelFrameFocus();
}

bool ACardShopFrameActor::CanAfford(int32 Price) const
{
	if (Price <= 0)
	{
		return true;
	}

	const UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
	return PartyInstance && PartyInstance->GetPartyInfo().Gold >= Price;
}

bool ACardShopFrameActor::TryPayPrice(int32 Price)
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
