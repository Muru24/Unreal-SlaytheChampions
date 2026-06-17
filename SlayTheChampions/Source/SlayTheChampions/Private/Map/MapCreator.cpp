// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/MapCreator.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/AreaLevelData.h"
#include "Map/Area.h"
#include "Map/MapAreaActor.h"
#include "Map/MapConfigData.h"
#include "Map/MapStruct.h"
#include "Map/RunSystem.h"
#include "Save/STCGameInstance.h"

UMapCreator::UMapCreator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UMapCreator::~UMapCreator()
{
}

UWorld* UMapCreator::GetWorld() const
{
	if (const UObject* OuterObject = GetOuter())
	{
		return OuterObject->GetWorld();
	}

	return nullptr;
}

void UMapCreator::InitWorldMap()
{
	for (AActor* Actor : WorldMap)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	WorldMap.Empty();
}

bool UMapCreator::LoadDefaultConfig()
{
	if (!MapConfig)
	{
		MapConfig = LoadObject<UMapConfigData>(nullptr, TEXT("/Game/04_Data/MapCreatorData.MapCreatorData"));
	}

	if (!MapConfig)
	{
		return false;
	}

	CurrentWidth = MapConfig->MapWidth;
	CurrentHeight = MapConfig->MapHeight;
	return true;
}

void UMapCreator::CreateMap()
{
	if (UWorld* World = GetWorld())
	{
		if (USTCGameInstance* STCGameInstance = Cast<USTCGameInstance>(World->GetGameInstance()))
		{
			STCGameInstance->LoadGameData();

			if (URunSystem* RunSystem = STCGameInstance->GetSubsystem<URunSystem>())
			{
				const FSaveMapInfo SaveMapInfo = RunSystem->GetMapInfo();
				if (RestoreMapFromSaveInfo(SaveMapInfo))
				{
					RunSystem->SetMapInfo(SaveMapInfo);
					return;
				}
			}
		}
	}

	if (!LoadDefaultConfig()) return;

	//맵 생성 작업전 정보 담을 배열들 초기화
	InitMap();
	InitGridMap();
	InitWorldMap();

	// 그리드 맵 생성
	GridMapCreate(CurrentWidth, CurrentHeight, MapConfig->AreaSpawnProbability);

	//Map에 데이터 입력
	SetMapData(CurrentWidth, CurrentHeight);

	//Area 연결
	ConnectAreas(CurrentWidth, CurrentHeight);

	//디버그용
	for (int i = 0; i < Map.Num(); i++)
	{
		if (Map[i] == nullptr) continue;

		Map[i]->DebugShowInfo();
	}

	// 실제 월드 액터 생성은 PortalSpawner가 현재 진입 가능한 방만 처리한다.
}

bool UMapCreator::HasMapData() const
{
	for (UArea* Area : Map)
	{
		if (Area)
		{
			return true;
		}
	}

	return false;
}

void UMapCreator::RestoreWorldMap()
{
	if (!HasMapData() || !MapConfig)
	{
		return;
	}

	InitWorldMap();
}

void UMapCreator::WriteMapInfo(FSaveMapInfo& OutMapInfo) const
{
	OutMapInfo.SavedMapWidth = CurrentWidth;
	OutMapInfo.SavedMapHeight = CurrentHeight;
	OutMapInfo.bHasSavedMap = HasMapData();
	OutMapInfo.SavedMap.Empty();
	OutMapInfo.SavedMap.SetNum(Map.Num());

	for (int32 Index = 0; Index < Map.Num(); Index++)
	{
		UArea* Area = Map[Index];
		if (!Area)
		{
			continue;
		}

		FSaveAreaData& SaveAreaData = OutMapInfo.SavedMap[Index];
		SaveAreaData.bHasArea = true;
		SaveAreaData.AreaInfo = Area->GetAreaInfo();
		SaveAreaData.NextAreaIndices.Empty();

		for (UArea* NextArea : Area->GetNextAreas())
		{
			if (!NextArea)
			{
				continue;
			}

			const FVector2D NextAreaPos = NextArea->GetArrPos();
			const int32 NextAreaIndex = (static_cast<int32>(NextAreaPos.X) * CurrentWidth) + static_cast<int32>(NextAreaPos.Y);
			if (Map.IsValidIndex(NextAreaIndex))
			{
				SaveAreaData.NextAreaIndices.Add(NextAreaIndex);
			}
		}
	}
}

bool UMapCreator::RestoreMapFromSaveInfo(const FSaveMapInfo& InMapInfo)
{
	if (!InMapInfo.bHasSavedMap || InMapInfo.SavedMapWidth <= 0 || InMapInfo.SavedMapHeight <= 0 || InMapInfo.SavedMap.Num() == 0)
	{
		return false;
	}

	if (!LoadDefaultConfig()) return false;

	CurrentWidth = InMapInfo.SavedMapWidth;
	CurrentHeight = InMapInfo.SavedMapHeight;

	InitMap();
	InitGridMap();
	InitWorldMap();

	const int32 MapSize = CurrentWidth * CurrentHeight;
	const int32 RestoreCount = FMath::Min(MapSize, InMapInfo.SavedMap.Num());

	for (int32 Index = 0; Index < RestoreCount; Index++)
	{
		const FSaveAreaData& SaveAreaData = InMapInfo.SavedMap[Index];
		if (!SaveAreaData.bHasArea)
		{
			continue;
		}

		UArea* Area = NewObject<UArea>(this);
		Area->InitArea(SaveAreaData.AreaInfo);
		Map[Index] = Area;
		GridMap[Index] = true;
	}

	for (int32 Index = 0; Index < RestoreCount; Index++)
	{
		UArea* Area = Map[Index];
		if (!Area)
		{
			continue;
		}

		const FSaveAreaData& SaveAreaData = InMapInfo.SavedMap[Index];
		for (int32 NextAreaIndex : SaveAreaData.NextAreaIndices)
		{
			if (Map.IsValidIndex(NextAreaIndex) && Map[NextAreaIndex])
			{
				Area->SetNextAreas(Map[NextAreaIndex]);
			}
		}
	}

	return true;
}
void UMapCreator::RefreshDebugWorldMapState()
{
	//디버그용
	for (AActor* Actor : WorldMap)
	{
		AMapAreaActor* MapAreaActor = Cast<AMapAreaActor>(Actor);
		if (!MapAreaActor)
		{
			continue;
		}

		UArea* Area = GetAreaAt(MapAreaActor->GetFloorIndex(), MapAreaActor->GetRoomIndex());
		if (!Area)
		{
			continue;
		}

		MapAreaActor->ApplyDebugAreaInfo(Area->GetAreaInfo());
	}
}

#pragma region GridMapGeneration
void UMapCreator::GridMapCreate(int32 MapWidth, int32 MapHeight, float AreaSpawnProbability)
{

	//1층 방 세팅
	GridMap[MapWidth / 2] = true;

	//중간방들 세팅
	for (int height = 1; height < MapHeight - 1; height++)
	{
		int32 FloorStart = height * MapWidth;
		int32 MustHavePos = FloorStart + FMath::RandRange(0, MapWidth - 1);
		GridMap[MustHavePos] = true;

		for (int32 w = 0; w < MapWidth; w++)
		{
			if (FMath::RandRange(0, 100) < AreaSpawnProbability)
			{
				GridMap[FloorStart + w] = true;
			}
		}
	}

	//마지막 층 방 세팅

	int32 LastStart = (MapHeight - 1) * MapWidth;
	int32 LastCenter = LastStart + (MapWidth / 2);
	GridMap[LastCenter] = true;

	//디버그용
	/*FString DebugMapString = TEXT("\n--- Grid Map Debug ---\n");
	for (int32 y = 0; y < MapHeight; y++)
	{
		FString Line = TEXT("");
		for (int32 x = 0; x < MapWidth; x++)
		{
			int32 Index = (y * MapWidth) + x;
			Line += GridMap[Index] ? TEXT(" 1 ") : TEXT(" 0 ");
		}
		DebugMapString += Line + TEXT("\n");
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugMapString);*/
}
#pragma endregion

#pragma region MapDataSetting
void UMapCreator::SetMapData(int32 MapWidth, int32 MapHeight)
{
	for (int height = 0; height < MapHeight; height++)
	{
		for (int width = 0; width < MapWidth; width++)
		{
			int32 pos = (height * MapWidth) + width;

			if (GridMap[pos])
			{
				Map[pos] = AreaCreate(height, width);
			}
		}
	}
}

UArea* UMapCreator::AreaCreate(int32 height, int32 width)
{
	UArea* Area = NewObject<UArea>(this);

	EAreaType type = EAreaType::None;
	const FAreaFixedPlacement& FixedPlacement = MapConfig->FixedPlacement;
	int32 floor = height + 1;

	if ((floor == FixedPlacement.ArtifactEvent)) type = EAreaType::ArtifactEvent;
	else if ((floor == FixedPlacement.Reword)) type = EAreaType::Reword;
	else if ((floor == FixedPlacement.Rest)) type = EAreaType::Rest;
	else if ((floor == FixedPlacement.Boss)) type = EAreaType::Boss;

	if (type == EAreaType::None)type = GetRandAreaType();

	FAreaInfo AreaInfo(
		EAreaState::Ready,
		type,
		FVector2D(height, width),
		EAreaVisitState::None);

	Area->InitArea(AreaInfo);

	return Area;
}

EAreaType UMapCreator::GetRandAreaType()
{
	const FAreaSpawnProbability& Prob = MapConfig->AreaTypeSpawnProbability;

	const float TotalProbability = Prob.Normal + Prob.Elite + Prob.Event + Prob.Rest + Prob.Shop;
	if (TotalProbability <= 0.f) return EAreaType::Shop;

	float RandVal = FMath::FRandRange(0.f, TotalProbability);

	if ((RandVal -= Prob.Normal) <= 0.f) return EAreaType::Normal;
	if ((RandVal -= Prob.Elite) <= 0.f) return EAreaType::Elite;
	if ((RandVal -= Prob.Event) <= 0.f) return EAreaType::Event;
	if ((RandVal -= Prob.Rest) <= 0.f) return EAreaType::Rest;

	return EAreaType::Shop;
}
#pragma endregion

#pragma region MapConnect
void UMapCreator::ConnectAreas(int32 MapWidth, int32 MapHeight)
{
	for (int height = 0; height < MapHeight - 1; height++)
	{
		for (int width = 0; width < MapWidth; width++)
		{
			int32 CurrnetPos = (height * MapWidth) + width;
			int32 NextHeight = height + 1;
			if (Map[CurrnetPos] == nullptr) continue;

			//1층일경우 2층 모든방과 연결
			if (height == 0)
			{
				for (int32 nextW = 0; nextW < MapWidth; nextW++)
				{
					int32 NextIdx = (NextHeight * MapWidth) + nextW;
					if (Map[NextIdx])
					{
						Map[CurrnetPos]->SetNextAreas(Map[NextIdx]);
					}
				}
				continue;
			}
			//마지막 직전 층일경우 전부 보스방과 연결
			if (NextHeight == MapHeight - 1)
			{
				int32 BossIdx = (NextHeight * MapWidth) + (MapWidth / 2);
				if (Map[BossIdx])
				{
					Map[CurrnetPos]->SetNextAreas(Map[BossIdx]);
				}
				continue;
			}

			//나머지방
			bool isConnect = false;
			for (int32 dw = -1; dw <= 1; dw++)
			{
				int32 NextWidth = width + dw;
				if (ConnectIfValid(CurrnetPos, NextHeight, NextWidth, MapWidth))
				{
					isConnect = true;
				}
			}

			if (!isConnect)
			{
				for (int32 dist = 2; dist < MapWidth; dist++)
				{
					if (ConnectIfValid(CurrnetPos, NextHeight, width - dist, MapWidth)) break;
					if (ConnectIfValid(CurrnetPos, NextHeight, width + dist, MapWidth)) break;
				}
			}
		}
	}
}

bool UMapCreator::ConnectIfValid(int32 CurrnetPos, int32 NextHeight, int32 NextWidth, int32 MapWidth)
{
	if (NextWidth >= 0 && NextWidth < MapWidth)
	{
		int32 NextIdx = (NextHeight * MapWidth) + NextWidth;
		if (Map[NextIdx])
		{
			Map[CurrnetPos]->SetNextAreas(Map[NextIdx]);
			return true;
		}
	}
	return false;
}
#pragma endregion

#pragma region WorldMapGeneration
void UMapCreator::WorldMapCreate(int32 MapWidth, int32 MapHeight)
{
	UWorld* World = GetWorld();
	if (!World || !MapConfig) return;

	for (int32 height = 0; height < MapHeight; height++)
	{
		for (int32 width = 0; width < MapWidth; width++)
		{
			int32 CurrentPos = (height * MapWidth) + width;

			if (!Map.IsValidIndex(CurrentPos) || Map[CurrentPos] == nullptr) continue;

			FVector SpawnLocation = FVector(width * MapConfig->TileDistance, height * MapConfig->TileDistance, 0.0f);
			FRotator SpawnRotation = FRotator::ZeroRotator;

			TSubclassOf<AActor> SelectedClass = GetAreaClass(Map[CurrentPos]->GetType());

			if (SelectedClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				const FString AreaTypeName = StaticEnum<EAreaType>()->GetNameStringByValue(static_cast<int64>(Map[CurrentPos]->GetType()));
				const FString AreaName = FString::Printf(TEXT("%d층 %d번 %s"), height + 1, width + 1, *AreaTypeName);
				AActor* SpawnedActor = World->SpawnActor<AActor>(SelectedClass, SpawnLocation, SpawnRotation, SpawnParams);
				if (SpawnedActor)
				{
					//디버그용
					if (AMapAreaActor* MapAreaActor = Cast<AMapAreaActor>(SpawnedActor))
					{
						MapAreaActor->SetAreaIndex(height, width);
						MapAreaActor->ApplyDebugAreaInfo(Map[CurrentPos]->GetAreaInfo());
						if (MapConfig->AreaLevelData)
						{
							MapAreaActor->SetTargetLevelName(MapConfig->AreaLevelData->GetLevelName(Map[CurrentPos]->GetType()));
						}
					}
#if WITH_EDITOR
					SpawnedActor->SetActorLabel(AreaName);
#endif
					WorldMap.Add(SpawnedActor);
				}
			}
		}
	}
}

TSubclassOf<AActor> UMapCreator::GetAreaClass(EAreaType type)
{
	switch (type)
	{
	case EAreaType::Normal: return MapConfig->NormalMap;
	case EAreaType::Elite:return MapConfig->EliteMap;
	case EAreaType::Boss:return MapConfig->BossMap;
	case EAreaType::Event:return MapConfig->EventMap;
	case EAreaType::Rest:return MapConfig->RestMap;
	case EAreaType::Shop:return MapConfig->ShopMap;
	case EAreaType::Reword:return MapConfig->RewordMap;
	case EAreaType::ArtifactEvent:return MapConfig->ArtifactEventMap;
	}
	return MapConfig->NormalMap;
}
#pragma endregion

