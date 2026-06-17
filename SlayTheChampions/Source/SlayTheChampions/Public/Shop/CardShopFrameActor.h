#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardShopFrameActor.generated.h"

class UBoxComponent;
class UCameraComponent;
class USceneComponent;
class APlayerController;

USTRUCT(BlueprintType)
struct FShopCardSaleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Card")
	FName CardID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Card")
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Card")
	bool bPurchased = false;
};

UCLASS()
class SLAYTHECHAMPIONS_API ACardShopFrameActor : public AActor
{
	GENERATED_BODY()

public:
	ACardShopFrameActor();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	TObjectPtr<UBoxComponent> FrameInteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	TObjectPtr<UCameraComponent> FrameCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Focus")
	float FocusBlendTime = 0.35f;

	UPROPERTY(BlueprintReadOnly, Category = "Shop|Focus")
	bool bFocused = false;

	UPROPERTY(BlueprintReadOnly, Category = "Shop|Card")
	TArray<FShopCardSaleData> SaleCards;

	UPROPERTY()
	TObjectPtr<AActor> PreviousViewTarget;

public:
	UFUNCTION(BlueprintCallable, Category = "Shop|Card")
	void InitCards(const TArray<FName>& CardIDs, const TArray<int32>& Prices);

	UFUNCTION(BlueprintCallable, Category = "Shop|Focus")
	void FocusFrameCamera(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Shop|Focus")
	void CancelFrameFocus();

	UFUNCTION(BlueprintCallable, Category = "Shop|Card")
	bool TryPurchaseCard(int32 CardIndex, int32 PawnIndex);

	UFUNCTION(BlueprintPure, Category = "Shop|Card")
	bool GetSaleCardData(int32 CardIndex, FShopCardSaleData& OutSaleData) const;

	UFUNCTION(BlueprintPure, Category = "Shop|Card")
	const TArray<FShopCardSaleData>& GetSaleCards() const { return SaleCards; }

	UFUNCTION(BlueprintPure, Category = "Shop|Focus")
	bool IsFocused() const { return bFocused; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Card")
	void OnCardsInitialized(const TArray<FShopCardSaleData>& Cards);

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Focus")
	void OnFrameFocused();

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Focus")
	void OnFrameFocusCanceled();

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Card")
	void OnCardPurchased(int32 CardIndex, const FShopCardSaleData& CardData, int32 PawnIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Card")
	void OnCardPurchaseFailed(int32 CardIndex, const FShopCardSaleData& CardData);

protected:
	void HandleFocusedWorldClick();

	bool CanAfford(int32 Price) const;
	bool TryPayPrice(int32 Price);
};
