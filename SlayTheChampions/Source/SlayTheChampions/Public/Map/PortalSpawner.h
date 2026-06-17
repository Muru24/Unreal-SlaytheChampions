#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapStruct.h"
#include "PortalSpawner.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SLAYTHECHAMPIONS_API APortalSpawner : public AActor
{
	GENERATED_BODY()

public:
	APortalSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "PortalSpawner")
	void SpawnPortals();

	UFUNCTION(BlueprintCallable, Category = "PortalSpawner")
	void SpawnEnterableRoomPortals();

	UFUNCTION(BlueprintCallable, Category = "PortalSpawner")
	void SpawnPortalsForRooms(const TArray<FAreaInfo>& RoomInfos);

	UFUNCTION(BlueprintCallable, Category = "PortalSpawner")
	void ClearSpawnedPortals();

	UFUNCTION(BlueprintPure, Category = "PortalSpawner")
	const TArray<AActor*>& GetSpawnedPortals() const { return SpawnedPortals; }

protected:
	UFUNCTION()
	void HandleEnterableRoomsUpdated(const TArray<FAreaInfo>& RoomInfos);

	void RetrySpawnEnterableRoomPortals();

	AActor* SpawnPortalAtIndex(int32 Index, int32 Count);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	TSubclassOf<AActor> PortalActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner", meta = (ClampMin = "1"))
	int32 PortalCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner", meta = (ClampMin = "0.0"))
	float Radius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float ArcAngleDegrees = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner", meta = (ClampMin = "0.0"))
	float PortalSpacing = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	float HeightOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	bool bUseEnterableRooms = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner", meta = (ClampMin = "0"))
	int32 EmptyRoomRetryCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner", meta = (ClampMin = "0.0"))
	float EmptyRoomRetryDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	bool bClearBeforeSpawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	bool bUseSpawnerForwardDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PortalSpawner")
	bool bPortalLookAtSpawner = true;

	UPROPERTY(BlueprintReadOnly, Category = "PortalSpawner")
	TArray<AActor*> SpawnedPortals;

	int32 CurrentEmptyRoomRetryCount = 0;
};
