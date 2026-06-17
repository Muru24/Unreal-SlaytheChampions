// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapStruct.h"
#include "RunSystem.generated.h"

class UArea;
class UMapManager;
class URewardSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomSelectionChanged, const TArray<FAreaInfo>&, RoomInfos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomEntered, const FAreaInfo&, RoomInfo);

UCLASS()
class SLAYTHECHAMPIONS_API URunSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	/* 저장된 맵 구조체 */
	UPROPERTY()
	FSaveMapInfo MapInfo;

	/* 현재 Area */
	UPROPERTY()
	UArea* CurrentArea = nullptr;

	/* Party 스냅샷 저장 */
	UPROPERTY()
	FRunPartySnapshot PartySnapshot;

	/* Deck 스냅샷 저장 */
	UPROPERTY()
	FRunDeckSnapshot DeckSnapshot;

	/* Relic 스냅샷 저장 */
	UPROPERTY()
	FRunRelicSnapshot RelicSnapshot;

	/* MapManager */
	UPROPERTY()
	UMapManager* MapManager = nullptr;

	UPROPERTY()
	URewardSystem* RewardSystem = nullptr;

	UPROPERTY()
	FName MapLevelName = TEXT("RunMap");

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/* 현재 진입가능한 Area 목록 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomSelectionChanged OnEnterableRoomsUpdated;

	/* Area 진입 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnRoomEntered;

	/* 각 Area 타입별 진입 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnBattleRoomEntered;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnEventRoomEntered;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnRestRoomEntered;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnShopRoomEntered;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnRewardRoomEntered;

	UPROPERTY(BlueprintAssignable, Category = "Run")
	FOnRoomEntered OnArtifactEventRoomEntered;

private:
	/* 진입가능한 Area 상태 업데이트 */
	void UpdateEnterableState();
	/* Area 타입별 처리 */
	void BroadcastRoomTypeEvent(const FAreaInfo& RoomInfo);
	/* 데이터 저장 */
	void SaveGameData();
public:
	/* Run 시작 */
	UFUNCTION(BlueprintCallable)
	void StartRun();

	/* Run 시작 후 맵 설정 */
	UFUNCTION(BlueprintCallable)
	void StartRunWithMap();

	/* Run 상태 갱신 */
	UFUNCTION(BlueprintCallable)
	void RefreshRunState();

	/* Area 입장 */
	UFUNCTION(BlueprintCallable)
	bool EnterRoom(UArea* Area);

	/* 현재 Area 클리어 처리 */
	UFUNCTION(BlueprintCallable)
	bool AreaCleared();

	/* Area 클리어 후 맵 복귀 */
	UFUNCTION(BlueprintCallable)
	void ReturnToMapAfterAreaClear();

	/* Area 입장 by GridIndex */
	UFUNCTION(BlueprintCallable)
	bool EnterRoomByGridIndex(int32 FloorIndex, int32 RoomIndex);

	/* Area 입장 가능 여부 확인 */
	UFUNCTION(BlueprintCallable)
	bool CanEnterRoom(UArea* Area) const;

	/* Area 입장 가능 여부 확인 by GridIndex */
	UFUNCTION(BlueprintCallable)
	bool CanEnterRoomByGridIndex(int32 FloorIndex, int32 RoomIndex) const;

	/* 현재 진입가능한 Area 목록 반환 */
	UFUNCTION(BlueprintCallable)
	TArray<FAreaInfo> GetEnterableRooms() const;

	/* 현재 Run 상태 반환 */
	UFUNCTION(BlueprintPure)
	ERunState GetRunState() const { return MapInfo.CurrentRunState; }

	/* 현재 Area 정보 반환 */
	UFUNCTION(BlueprintPure)
	FAreaInfo GetCurrentRoomInfo() const { return MapInfo.CurrentRoomInfo; }

	/* 현재 층 인덱스 반환. 맵 내부 인덱스라 0층부터 시작한다. */
	UFUNCTION(BlueprintPure)
	int32 GetCurrentFloorIndex() const { return MapInfo.CurrentFloorIndex; }

	/* UI 표시용 현재 층 반환. 내부 0층을 1층으로 표시한다. */
	UFUNCTION(BlueprintPure)
	int32 GetCurrentDisplayFloor() const { return MapInfo.CurrentFloorIndex == INDEX_NONE ? 0 : MapInfo.CurrentFloorIndex + 1; }

	/* 보상 시스템 반환 */
	UFUNCTION(BlueprintPure)
	URewardSystem* GetRewardSystem() const { return RewardSystem; }

	/* MapInfo 반환 */
	UFUNCTION(BlueprintPure)
	FSaveMapInfo GetMapInfo() const;

	/* MapInfo 설정 */
	UFUNCTION(BlueprintCallable)
	void SetMapInfo(FSaveMapInfo _info);

	/* 맵 레벨 이름 설정 */
	UFUNCTION(BlueprintCallable)
	void SetMapLevelName(FName InMapLevelName) { MapLevelName = InMapLevelName; }

	/* Party 설정 */
	UFUNCTION(BlueprintCallable)
	void SetPartySnapshot(const FRunPartySnapshot& InSnapshot);

	/* Deck 설정 */
	UFUNCTION(BlueprintCallable)
	void SetDeckSnapshot(const FRunDeckSnapshot& InSnapshot);

	/* Relic 설정 */
	UFUNCTION(BlueprintCallable)
	void SetRelicSnapshot(const FRunRelicSnapshot& InSnapshot);

	UFUNCTION(BlueprintPure)
	FRunPartySnapshot GetPartySnapshot() const { return PartySnapshot; }

	UFUNCTION(BlueprintPure)
	FRunDeckSnapshot GetDeckSnapshot() const { return DeckSnapshot; }

	UFUNCTION(BlueprintPure)
	FRunRelicSnapshot GetRelicSnapshot() const { return RelicSnapshot; }
};
