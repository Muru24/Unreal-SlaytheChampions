#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Card/CardDataTypes.h"
#include "Item/ItemActor.h"
#include "ShopSystem.generated.h"

class USceneComponent;
class UItemVisualDataAsset;
class ACardShopFrameActor;

UENUM(BlueprintType)
/* 상점 판매 아이템 종류 */
enum class EShopSaleItemType : uint8
{
	Card = 0 UMETA(DisplayName = "Card"),
	Relic = 1 UMETA(DisplayName = "Relic"),
	Potion = 2 UMETA(DisplayName = "Potion"),
};

UCLASS()
class SLAYTHECHAMPIONS_API AShopSystem : public AActor
{
	GENERATED_BODY()

public:
	AShopSystem();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Class")
	TSubclassOf<AActor> CardSaleActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Class")
	TSubclassOf<ACardShopFrameActor> CardShopFrameActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Class")
	TSubclassOf<AItemActor> RelicItemActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Class")
	TSubclassOf<AItemActor> PotionItemActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Visual")
	TObjectPtr<UItemVisualDataAsset> ItemVisualDataAsset;

	UPROPERTY(BlueprintReadOnly, Category = "Shop|SpawnPoint")
	TArray<TObjectPtr<USceneComponent>> CardSpawnPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Shop|SpawnPoint")
	TArray<TObjectPtr<USceneComponent>> RelicSpawnPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Shop|SpawnPoint")
	TArray<TObjectPtr<USceneComponent>> PotionSpawnPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Shop|Runtime")
	TArray<TObjectPtr<AActor>> SpawnedSaleActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Card")
	EJobClass ShopCardClass = EJobClass::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Card")
	ECardRarity ShopCardMinRarity = ECardRarity::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Card")
	int32 CardsPerFrame = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Price")
	int32 CardMinPrice = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Price")
	int32 CardMaxPrice = 120;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Price")
	int32 RelicMinPrice = 120;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Price")
	int32 RelicMaxPrice = 220;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Price")
	int32 PotionMinPrice = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Price")
	int32 PotionMaxPrice = 90;

public:
	UFUNCTION(BlueprintCallable, Category = "Shop|SpawnPoint")
	void RegisterSpawnPoint(EShopSaleItemType SaleItemType, USceneComponent* SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Shop|SpawnPoint")
	void ClearRegisteredSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SpawnShopItems();

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ClearShopItems();

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Card")
	void OnCardSaleItemSpawned(AActor* SpawnedActor, FName CardID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop|Card")
	void OnCardShopFrameSpawned(ACardShopFrameActor* SpawnedFrame);

	UFUNCTION(BlueprintImplementableEvent, Category = "Shop")
	void OnShopItemsSpawned();

protected:
	FName GetRandomShopCardID() const;

	FName GetRandomShopRelicID() const;

	FName GetRandomShopPotionID() const;

	int32 RollPrice(int32 MinPrice, int32 MaxPrice) const;

	void SpawnCardSaleItem(USceneComponent* SpawnPoint);

	void SpawnRelicSaleItem(USceneComponent* SpawnPoint);

	void SpawnPotionSaleItem(USceneComponent* SpawnPoint);

	AActor* SpawnActorAtPoint(TSubclassOf<AActor> ActorClass, USceneComponent* SpawnPoint);
};
