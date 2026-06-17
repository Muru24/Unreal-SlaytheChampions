// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/MapManager.h"

#include "Map/MapCreator.h"
#include "Map/RunSystem.h"

void UMapManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MapCreator = NewObject<UMapCreator>(this, UMapCreator::StaticClass());
	RunSystem = GetGameInstance()->GetSubsystem<URunSystem>();
}

void UMapManager::StartRunSystem()
{
	if (!RunSystem)
	{
		RunSystem = GetGameInstance()->GetSubsystem<URunSystem>();
	}

	if (RunSystem)
	{
		RunSystem->StartRunWithMap();
	}
}

void UMapManager::MapCreate()
{
	if (MapCreator)
	{
		MapCreator->CreateMap();
	}
}

void UMapManager::RestoreMapWorld()
{
	if (MapCreator)
	{
		MapCreator->RestoreWorldMap();
	}
}

bool UMapManager::HasMapData() const
{
	return MapCreator ? MapCreator->HasMapData() : false;
}

void UMapManager::WriteMapInfo(FSaveMapInfo& OutMapInfo) const
{
	if (MapCreator)
	{
		MapCreator->WriteMapInfo(OutMapInfo);
	}
}

bool UMapManager::RestoreMapFromSaveInfo(const FSaveMapInfo& InMapInfo)
{
	return MapCreator ? MapCreator->RestoreMapFromSaveInfo(InMapInfo) : false;
}
void UMapManager::RefreshDebugMapState()
{
	if (MapCreator)
	{
		MapCreator->RefreshDebugWorldMapState();
	}
}

UArea* UMapManager::GetAreaAt(int32 height, int32 width) const
{
	return MapCreator ? MapCreator->GetAreaAt(height, width) : nullptr;
}

int32 UMapManager::GetMapWidth() const
{
	return MapCreator ? MapCreator->GetMapWidth() : 0;
}

int32 UMapManager::GetMapHeight() const
{
	return MapCreator ? MapCreator->GetMapHeight() : 0;
}

int32 UMapManager::GetCurrentFloorIndex() const
{
	return RunSystem ? RunSystem->GetCurrentFloorIndex() : INDEX_NONE;
}

int32 UMapManager::GetCurrentDisplayFloor() const
{
	return RunSystem ? RunSystem->GetCurrentDisplayFloor() : 0;
}
