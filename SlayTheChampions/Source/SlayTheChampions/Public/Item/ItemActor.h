#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Relic/RelicStruct.h"
#include "Potion/PotionStruct.h"
#include "ItemActor.generated.h"

class UBoxComponent;
class UCameraComponent;
class UItemVisualDataAsset;
class UStaticMeshComponent;
class APlayerController;

UENUM(BlueprintType)
/* 월드에 놓이는 아이템 종류 */
enum class EItemActorType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Relic = 1 UMETA(DisplayName = "Relic"),
	Potion = 2 UMETA(DisplayName = "Potion"),
};

UCLASS()
class SLAYTHECHAMPIONS_API AItemActor : public AActor
{
	GENERATED_BODY()

public:
	AItemActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component")
	TObjectPtr<UCameraComponent> ItemCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemActorType ItemType = EItemActorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Item|Shop")
	int32 Price = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FRelic CachedRelicData;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FPotionData CachedPotionData;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bHasValidItemData = false;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bCollected = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
	TObjectPtr<UItemVisualDataAsset> ItemVisualDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Focus")
	float FocusBlendTime = 0.35f;

	UPROPERTY(BlueprintReadOnly, Category = "Item|Focus")
	bool bFocused = false;

	UPROPERTY()
	TObjectPtr<AActor> PreviousViewTarget;

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitItem(EItemActorType InItemType, FName InItemID);

	UFUNCTION(BlueprintCallable, Category = "Item|Shop")
	void SetPrice(int32 InPrice);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetItemVisualDataAsset(UItemVisualDataAsset* InItemVisualDataAsset);

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool RefreshItemData();

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool ApplyItemVisual();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintPure, Category = "Item|Shop")
	bool CanAfford() const;

	UFUNCTION(BlueprintCallable, Category = "Item|Shop")
	bool TryPayPrice();

	UFUNCTION(BlueprintCallable, Category = "Item|Focus")
	void FocusItemCamera(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Item|Focus")
	void CancelItemFocus();

	UFUNCTION(BlueprintCallable, Category = "Item|Focus")
	void ConfirmFocusedItem(AActor* Interactor);

	UFUNCTION(BlueprintPure, Category = "Item")
	EItemActorType GetItemType() const { return ItemType; }

	UFUNCTION(BlueprintPure, Category = "Item")
	FName GetItemID() const { return ItemID; }

	UFUNCTION(BlueprintPure, Category = "Item|Shop")
	int32 GetPrice() const { return Price; }

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsCollected() const { return bCollected; }

	UFUNCTION(BlueprintPure, Category = "Item|Focus")
	bool IsFocused() const { return bFocused; }

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetDescription() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetItemName() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetItemDescription() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void OnItemDataRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void OnItemInteracted(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Shop")
	void OnItemPurchaseFailed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void OnItemCollected();

	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Focus")
	void OnItemFocused();

	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Focus")
	void OnItemFocusCanceled();

	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Focus")
	void OnFocusedItemConfirmed(AActor* Interactor);

protected:
	void HandleFocusedWorldClick();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void MarkCollected();
};
