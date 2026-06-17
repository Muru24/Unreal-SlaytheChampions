// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RunSystem.h"

#include "GameManagers/LevelManager.h"
#include "Map/Area.h"
#include "Map/MapManager.h"
#include "Map/MapEnum.h"
#include "Reward/RewardSystem.h"
#include "Save/STCGameInstance.h"

namespace
{
FString AreaTypeToString(EAreaType AreaType)
{
	const UEnum* AreaTypeEnum = StaticEnum<EAreaType>();
	return AreaTypeEnum ? AreaTypeEnum->GetNameStringByValue(static_cast<int64>(AreaType)) : TEXT("Unknown");
}

FString VisitStateToString(EAreaVisitState VisitState)
{
	const UEnum* VisitStateEnum = StaticEnum<EAreaVisitState>();
	return VisitStateEnum ? VisitStateEnum->GetNameStringByValue(static_cast<int64>(VisitState)) : TEXT("Unknown");
}
}

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
	if (!MapManager)
	{
		MapManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[RunSystem] RefreshRunState Begin. HasMapManager=%s bHasCurrentRoom=%s SavedCurrent=(Floor=%d Room=%d) CurrentArea=%s"),
		MapManager ? TEXT("true") : TEXT("false"),
		MapInfo.bHasCurrentRoom ? TEXT("true") : TEXT("false"),
		MapInfo.CurrentFloorIndex,
		MapInfo.CurrentRoomIndex,
		CurrentArea ? TEXT("Valid") : TEXT("None"));

	if (!CurrentArea && MapManager && MapInfo.bHasCurrentRoom)
	{
		CurrentArea = MapManager->GetAreaAt(MapInfo.CurrentFloorIndex, MapInfo.CurrentRoomIndex);
		UE_LOG(LogTemp, Warning, TEXT("[RunSystem] RefreshRunState Restore CurrentArea. Result=%s"),
			CurrentArea ? TEXT("Valid") : TEXT("None"));
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
		UE_LOG(LogTemp, Warning, TEXT("[RunSystem] AreaCleared failed. CurrentArea is null. SavedCurrent=(Floor=%d Room=%d) bHasCurrentRoom=%s"),
			MapInfo.CurrentFloorIndex,
			MapInfo.CurrentRoomIndex,
			MapInfo.bHasCurrentRoom ? TEXT("true") : TEXT("false"));
		return false;
	}

	const FAreaInfo BeforeClearInfo = CurrentArea->GetAreaInfo();
	UE_LOG(LogTemp, Warning, TEXT("[RunSystem] AreaCleared. Current=(Floor=%d Room=%d Type=%s Visit=%s) NextAreas=%d"),
		static_cast<int32>(BeforeClearInfo.AreaPos.X),
		static_cast<int32>(BeforeClearInfo.AreaPos.Y),
		*AreaTypeToString(BeforeClearInfo.AreaType),
		*VisitStateToString(BeforeClearInfo.AreaVisit),
		CurrentArea->GetNextAreas().Num());

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
	UE_LOG(LogTemp, Warning, TEXT("[RunSystem] ReturnToMapAfterAreaClear. ReturnLevel=%s Current=(Floor=%d Room=%d) CurrentArea=%s"),
		*MapLevelName.ToString(),
		MapInfo.CurrentFloorIndex,
		MapInfo.CurrentRoomIndex,
		CurrentArea ? TEXT("Valid") : TEXT("None"));

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
		MapManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
	}

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
	const UMapManager* ActiveMapManager = MapManager;
	if (!ActiveMapManager)
	{
		ActiveMapManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
	}

	if (!ActiveMapManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RunSystem] GetEnterableRooms failed. MapManager is null."));
		return EnterableRooms;
	}

	const int32 MapHeight = ActiveMapManager->GetMapHeight();
	const int32 MapWidth = ActiveMapManager->GetMapWidth();
	for (int32 FloorIndex = 0; FloorIndex < ActiveMapManager->GetMapHeight(); FloorIndex++)
	{
		for (int32 RoomIndex = 0; RoomIndex < ActiveMapManager->GetMapWidth(); RoomIndex++)
		{
			UArea* Area = ActiveMapManager->GetAreaAt(FloorIndex, RoomIndex);
			if (Area && Area->GetAreaInfo().bCanEnter)
			{
				EnterableRooms.Add(Area->GetAreaInfo());
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[RunSystem] GetEnterableRooms Result=%d MapSize=(%d x %d) bHasCurrentRoom=%s Current=(Floor=%d Room=%d) CurrentArea=%s"),
		EnterableRooms.Num(),
		MapWidth,
		MapHeight,
		MapInfo.bHasCurrentRoom ? TEXT("true") : TEXT("false"),
		MapInfo.CurrentFloorIndex,
		MapInfo.CurrentRoomIndex,
		CurrentArea ? TEXT("Valid") : TEXT("None"));

	return EnterableRooms;
}

FSaveMapInfo URunSystem::GetMapInfo() const
{
	FSaveMapInfo SaveMapInfo = MapInfo;
	const UMapManager* ActiveMapManager = MapManager;
	if (!ActiveMapManager)
	{
		ActiveMapManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
	}

	if (ActiveMapManager && ActiveMapManager->HasMapData())
	{
		ActiveMapManager->WriteMapInfo(SaveMapInfo);
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

	if (!MapManager)
	{
		MapManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
	}

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
		MapManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
	}

	if (!MapManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RunSystem] UpdateEnterableState failed. MapManager is null."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[RunSystem] UpdateEnterableState Begin. MapSize=(%d x %d) bHasCurrentRoom=%s Current=(Floor=%d Room=%d) CurrentArea=%s"),
		MapManager->GetMapWidth(),
		MapManager->GetMapHeight(),
		MapInfo.bHasCurrentRoom ? TEXT("true") : TEXT("false"),
		MapInfo.CurrentFloorIndex,
		MapInfo.CurrentRoomIndex,
		CurrentArea ? TEXT("Valid") : TEXT("None"));

	if (CurrentArea)
	{
		const FAreaInfo& CurrentInfo = CurrentArea->GetAreaInfo();
		UE_LOG(LogTemp, Warning, TEXT("[RunSystem] CurrentArea Info. Pos=(%d,%d) Type=%s Visit=%s NextAreas=%d"),
			static_cast<int32>(CurrentInfo.AreaPos.X),
			static_cast<int32>(CurrentInfo.AreaPos.Y),
			*AreaTypeToString(CurrentInfo.AreaType),
			*VisitStateToString(CurrentInfo.AreaVisit),
			CurrentArea->GetNextAreas().Num());

		for (UArea* NextArea : CurrentArea->GetNextAreas())
		{
			if (!NextArea)
			{
				UE_LOG(LogTemp, Warning, TEXT("[RunSystem] CurrentArea NextArea=None"));
				continue;
			}

			const FAreaInfo& NextInfo = NextArea->GetAreaInfo();
			UE_LOG(LogTemp, Warning, TEXT("[RunSystem] CurrentArea NextArea Pos=(%d,%d) Type=%s Visit=%s"),
				static_cast<int32>(NextInfo.AreaPos.X),
				static_cast<int32>(NextInfo.AreaPos.Y),
				*AreaTypeToString(NextInfo.AreaType),
				*VisitStateToString(NextInfo.AreaVisit));
		}
	}

	int32 EnterableCount = 0;
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

			if (Area->GetAreaInfo().bCanEnter)
			{
				EnterableCount++;
				const FAreaInfo& EnterableInfo = Area->GetAreaInfo();
				UE_LOG(LogTemp, Warning, TEXT("[RunSystem] EnterableRoom Pos=(%d,%d) Type=%s Visit=%s"),
					static_cast<int32>(EnterableInfo.AreaPos.X),
					static_cast<int32>(EnterableInfo.AreaPos.Y),
					*AreaTypeToString(EnterableInfo.AreaType),
					*VisitStateToString(EnterableInfo.AreaVisit));
			}
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

	UE_LOG(LogTemp, Warning, TEXT("[RunSystem] UpdateEnterableState End. EnterableCount=%d Current=(Floor=%d Room=%d) CurrentArea=%s"),
		EnterableCount,
		MapInfo.CurrentFloorIndex,
		MapInfo.CurrentRoomIndex,
		CurrentArea ? TEXT("Valid") : TEXT("None"));
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

