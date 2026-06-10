// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RunSystem.h"

#include "GameManagers/LevelManager.h"
#include "Map/Area.h"
#include "Map/MapManager.h"
#include "Reward/RewardSystem.h"
#include "Save/STCGameInstance.h"

void URunSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MapManager = GetGameInstance()->GetSubsystem<UMapManager>();
	RewardSystem = NewObject<URewardSystem>(this, URewardSystem::StaticClass());
	if (RewardSystem)
	{
		RewardSystem->Initialize(this);
	}
	MapInfo.CurrentRunState = ERunState::Ready;
	MapInfo.CurrentRoomInfo = FAreaInfo();
	MapInfo.CurrentFloorIndex = INDEX_NONE;
	MapInfo.CurrentRoomIndex = INDEX_NONE;
	MapInfo.bHasCurrentRoom = false;
	CurrentArea = nullptr;
	UpdateEnterableState();
}

void URunSystem::StartRun()
{
	MapInfo.CurrentRunState = ERunState::RunInit;
	MapInfo.CurrentRoomInfo = FAreaInfo();
	MapInfo.CurrentFloorIndex = INDEX_NONE;
	MapInfo.CurrentRoomIndex = INDEX_NONE;
	MapInfo.bHasCurrentRoom = false;
	CurrentArea = nullptr;
	UpdateEnterableState();
	OnEnterableRoomsUpdated.Broadcast(GetEnterableRooms());
}

void URunSystem::StartRunWithMap()
{
	if (!MapManager)
	{
		MapManager = GetGameInstance()->GetSubsystem<UMapManager>();
	}

	if (MapManager)
	{
		MapManager->MapCreate();
	}

	if (MapInfo.bHasSavedMap && MapInfo.bHasCurrentRoom)
	{
		RefreshRunState();
		return;
	}

	StartRun();
}

void URunSystem::RefreshRunState()
{
	if (!CurrentArea && MapManager && MapInfo.bHasCurrentRoom)
	{
		CurrentArea = MapManager->GetAreaAt(MapInfo.CurrentFloorIndex, MapInfo.CurrentRoomIndex);
	}

	UpdateEnterableState();
	OnEnterableRoomsUpdated.Broadcast(GetEnterableRooms());
}

bool URunSystem::EnterRoom(UArea* Area)
{
	if (!CanEnterRoom(Area))
	{
		return false;
	}

	if (CurrentArea)
	{
		CurrentArea->SetCurrentArea(false);
	}

	CurrentArea = Area;
	CurrentArea->SetVisitState(EAreaVisitState::Visited);
	CurrentArea->SetState(EAreaState::Running);
	CurrentArea->SetCurrentArea(true);
	MapInfo.CurrentRoomInfo = CurrentArea->GetAreaInfo();
	MapInfo.CurrentFloorIndex = static_cast<int32>(MapInfo.CurrentRoomInfo.AreaPos.X);
	MapInfo.CurrentRoomIndex = static_cast<int32>(MapInfo.CurrentRoomInfo.AreaPos.Y);
	MapInfo.bHasCurrentRoom = true;
	MapInfo.CurrentRunState = ERunState::RoomEntered;
	UpdateEnterableState();
	OnRoomEntered.Broadcast(MapInfo.CurrentRoomInfo);
	BroadcastRoomTypeEvent(MapInfo.CurrentRoomInfo);
	OnEnterableRoomsUpdated.Broadcast(GetEnterableRooms());
	SaveGameData();
	return true;
}

bool URunSystem::AreaCleared()
{
	if (!CurrentArea)
	{
		return false;
	}

	CurrentArea->SetVisitState(EAreaVisitState::Cleared);
	CurrentArea->SetState(EAreaState::End);
	MapInfo.CurrentRoomInfo = CurrentArea->GetAreaInfo();
	MapInfo.CurrentRunState = ERunState::StageClear;
	UpdateEnterableState();
	SaveGameData();
	if (RewardSystem)
	{
		RewardSystem->OpenAreaClearReward(MapInfo.CurrentRoomInfo);
	}
	return true;
}

void URunSystem::ReturnToMapAfterAreaClear()
{
	MapInfo.CurrentRunState = ERunState::StageSelect;
	RefreshRunState();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULevelManager* LevelManager = GameInstance->GetSubsystem<ULevelManager>())
		{
			LevelManager->MoveToConfiguredLevel(MapLevelName);
		}
	}
}

bool URunSystem::EnterRoomByGridIndex(int32 FloorIndex, int32 RoomIndex)
{
	if (!MapManager)
	{
		return false;
	}

	return EnterRoom(MapManager->GetAreaAt(FloorIndex, RoomIndex));
}

bool URunSystem::CanEnterRoom(UArea* Area) const
{
	if (!Area)
	{
		return false;
	}

	const FAreaInfo& AreaInfo = Area->GetAreaInfo();
	if (AreaInfo.AreaVisit == EAreaVisitState::Visited || AreaInfo.AreaVisit == EAreaVisitState::Cleared)
	{
		return false;
	}

	if (!MapInfo.bHasCurrentRoom)
	{
		return static_cast<int32>(AreaInfo.AreaPos.X) == 0;
	}

	if (!CurrentArea)
	{
		return false;
	}

	for (UArea* NextArea : CurrentArea->GetNextAreas())
	{
		if (NextArea == Area)
		{
			return true;
		}
	}

	return false;
}

bool URunSystem::CanEnterRoomByGridIndex(int32 FloorIndex, int32 RoomIndex) const
{
	if (!MapManager)
	{
		return false;
	}

	return CanEnterRoom(MapManager->GetAreaAt(FloorIndex, RoomIndex));
}

TArray<FAreaInfo> URunSystem::GetEnterableRooms() const
{
	TArray<FAreaInfo> EnterableRooms;
	if (!MapManager)
	{
		return EnterableRooms;
	}

	for (int32 FloorIndex = 0; FloorIndex < MapManager->GetMapHeight(); FloorIndex++)
	{
		for (int32 RoomIndex = 0; RoomIndex < MapManager->GetMapWidth(); RoomIndex++)
		{
			UArea* Area = MapManager->GetAreaAt(FloorIndex, RoomIndex);
			if (Area && Area->GetAreaInfo().bCanEnter)
			{
				EnterableRooms.Add(Area->GetAreaInfo());
			}
		}
	}

	return EnterableRooms;
}

FSaveMapInfo URunSystem::GetMapInfo() const
{
	FSaveMapInfo SaveMapInfo = MapInfo;
	if (MapManager && MapManager->HasMapData())
	{
		MapManager->WriteMapInfo(SaveMapInfo);
	}
	return SaveMapInfo;
}

void URunSystem::SetPartySnapshot(const FRunPartySnapshot& InSnapshot)
{
	PartySnapshot = InSnapshot;
}

void URunSystem::SetDeckSnapshot(const FRunDeckSnapshot& InSnapshot)
{
	DeckSnapshot = InSnapshot;
}

void URunSystem::SetMapInfo(FSaveMapInfo _info)
{
	MapInfo = _info;
	CurrentArea = nullptr;

	if (MapManager && MapInfo.bHasCurrentRoom)
	{
		CurrentArea = MapManager->GetAreaAt(MapInfo.CurrentFloorIndex, MapInfo.CurrentRoomIndex);
	}

	UpdateEnterableState();
}

void URunSystem::SetRelicSnapshot(const FRunRelicSnapshot& InSnapshot)
{
	RelicSnapshot = InSnapshot;
}

void URunSystem::UpdateEnterableState()
{
	if (!MapManager)
	{
		return;
	}

	for (int32 FloorIndex = 0; FloorIndex < MapManager->GetMapHeight(); FloorIndex++)
	{
		for (int32 RoomIndex = 0; RoomIndex < MapManager->GetMapWidth(); RoomIndex++)
		{
			UArea* Area = MapManager->GetAreaAt(FloorIndex, RoomIndex);
			if (!Area)
			{
				continue;
			}

			Area->SetCanEnter(CanEnterRoom(Area));
			Area->SetCurrentArea(Area == CurrentArea);
		}
	}

	if (CurrentArea)
	{
		MapInfo.CurrentRoomInfo = CurrentArea->GetAreaInfo();
		MapInfo.CurrentFloorIndex = static_cast<int32>(MapInfo.CurrentRoomInfo.AreaPos.X);
		MapInfo.CurrentRoomIndex = static_cast<int32>(MapInfo.CurrentRoomInfo.AreaPos.Y);
	}
	else if (!MapInfo.bHasCurrentRoom)
	{
		MapInfo.CurrentRoomInfo = FAreaInfo();
		MapInfo.CurrentFloorIndex = INDEX_NONE;
		MapInfo.CurrentRoomIndex = INDEX_NONE;
	}

	MapManager->RefreshDebugMapState();
}

void URunSystem::SaveGameData()
{
	if (USTCGameInstance* STCGameInstance = Cast<USTCGameInstance>(GetGameInstance()))
	{
		STCGameInstance->SaveGameData();
	}
}

void URunSystem::BroadcastRoomTypeEvent(const FAreaInfo& RoomInfo){
	switch (RoomInfo.AreaType)
	{
	case EAreaType::Normal:
	case EAreaType::Elite:
	case EAreaType::Boss:
		OnBattleRoomEntered.Broadcast(RoomInfo);
		break;
	case EAreaType::Event:
		OnEventRoomEntered.Broadcast(RoomInfo);
		break;
	case EAreaType::Rest:
		OnRestRoomEntered.Broadcast(RoomInfo);
		break;
	case EAreaType::Shop:
		OnShopRoomEntered.Broadcast(RoomInfo);
		break;
	case EAreaType::Reword:
		OnRewardRoomEntered.Broadcast(RoomInfo);
		break;
	case EAreaType::ArtifactEvent:
		OnArtifactEventRoomEntered.Broadcast(RoomInfo);
		break;
	}
}

