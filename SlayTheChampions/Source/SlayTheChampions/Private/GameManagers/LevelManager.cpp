#include "GameManagers/LevelManager.h"

#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"
#include "Map/AreaLevelData.h"
#include "Map/DebugRunInputActor.h"
#include "Map/MapConfigData.h"
#include "Map/MapManager.h"
#include "Map/RunSystem.h"

void ULevelManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULevelManager::OnLevelStarted);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ULevelManager::OnPostWorldInitialized);
}

void ULevelManager::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	ActiveWorld = nullptr;

	Super::Deinitialize();
}

void ULevelManager::GoToLevel(FName LevelName)
{
	MoveLevelName = LevelName;

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("TestLoading"));
}

void ULevelManager::MoveToConfiguredLevel(FName LevelName)
{
	if (bUseStreamedLevelTransition)
	{
		GoToStreamedLevel(LevelName);
		return;
	}

	GoToLevel(LevelName);
}

void ULevelManager::GoToStreamedLevel(FName LevelName)
{
	if (LevelName.IsNone() || bIsStreamingTransitionInProgress)
	{
		return;
	}

	if (CurrentStreamedLevelName == LevelName)
	{
		return;
	}

	PendingStreamedLevelName = LevelName;
	bIsStreamingTransitionInProgress = true;

	if (bKeepStreamedLevelsLoaded)
	{
		if (!CurrentStreamedLevelName.IsNone())
		{
			SetStreamedLevelVisibility(CurrentStreamedLevelName, false);
		}

		if (IsStreamedLevelLoaded(PendingStreamedLevelName))
		{
			SetStreamedLevelVisibility(PendingStreamedLevelName, true);
			OnStreamedLevelLoaded();
			return;
		}

		LoadStreamedLevel(PendingStreamedLevelName);
		return;
	}

	if (CurrentStreamedLevelName.IsNone())
	{
		LoadStreamedLevel(PendingStreamedLevelName);
		return;
	}

	UnloadCurrentStreamedLevel();
}

void ULevelManager::AsynchronousLoadLevel()
{
	if (MoveLevelName.IsNone())
	{
		return;
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(ULevelManager, OnLevelLoadCompleted);
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__;

	UGameplayStatics::LoadStreamLevel(GetWorld(), MoveLevelName, true, false, LatentInfo);
}

void ULevelManager::OnLevelStarted(UWorld* World)
{
	if (!World)
	{
		return;
	}

	ActiveWorld = World;
	const FString MapName = World->RemovePIEPrefix(World->GetMapName());

	if (MapName == "TestLoading")
	{
		AsynchronousLoadLevel();
		return;
	}

	//?붾쾭洹몄슜
	SpawnDebugInputActor(World);

	if (ShouldBootstrapMapSystem(MapName))
	{
		EnsureInitialStreamedLevelLoaded();
		HandleMapWorldEntered();
	}
}

void ULevelManager::OnPostWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World)
	{
		return;
	}

	if (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)
	{
		OnLevelStarted(World);
	}
}

void ULevelManager::OnLevelLoadCompleted()
{
	UGameplayStatics::OpenLevel(GetWorld(), MoveLevelName);
}

void ULevelManager::OnStreamedLevelLoaded()
{
	SetStreamedLevelVisibility(PendingStreamedLevelName, true);
	CurrentStreamedLevelName = PendingStreamedLevelName;
	PendingStreamedLevelName = NAME_None;
	bIsStreamingTransitionInProgress = false;
}

void ULevelManager::OnCurrentStreamedLevelUnloaded()
{
	CurrentStreamedLevelName = NAME_None;
	LoadStreamedLevel(PendingStreamedLevelName);
}

void ULevelManager::SpawnDebugInputActor(UWorld* World)
{
	//?붾쾭洹몄슜
	if (!World)
	{
		return;
	}

	World->SpawnActor<ADebugRunInputActor>();
}

void ULevelManager::LoadStreamedLevel(FName LevelName)
{
	if (LevelName.IsNone())
	{
		bIsStreamingTransitionInProgress = false;
		return;
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(ULevelManager, OnStreamedLevelLoaded);
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__;

	if (UWorld* World = ActiveWorld.Get())
	{
		UGameplayStatics::LoadStreamLevel(World, LevelName, true, false, LatentInfo);
	}
	else
	{
		bIsStreamingTransitionInProgress = false;
	}
}

void ULevelManager::UnloadCurrentStreamedLevel()
{
	if (CurrentStreamedLevelName.IsNone())
	{
		LoadStreamedLevel(PendingStreamedLevelName);
		return;
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(ULevelManager, OnCurrentStreamedLevelUnloaded);
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__;

	if (UWorld* World = ActiveWorld.Get())
	{
		UGameplayStatics::UnloadStreamLevel(World, CurrentStreamedLevelName, LatentInfo, false);
	}
	else
	{
		bIsStreamingTransitionInProgress = false;
	}
}

FName ULevelManager::GetConfiguredInitialStreamedLevelName() const
{
	static const TCHAR* MapConfigPath = TEXT("/Game/04_Data/MapCreatorData.MapCreatorData");

	const UMapConfigData* MapConfig = LoadObject<UMapConfigData>(nullptr, MapConfigPath);
	if (!MapConfig || !MapConfig->AreaLevelData)
	{
		return NAME_None;
	}

	return MapConfig->AreaLevelData->InitialStreamedLevelName;
}

bool ULevelManager::ShouldBootstrapMapSystem(const FString& MapName) const
{
	const FName ConfiguredInitialLevelName = GetConfiguredInitialStreamedLevelName();
	return MapName == TEXT("NormalMap") || MapName == ConfiguredInitialLevelName.ToString();
}

void ULevelManager::EnsureInitialStreamedLevelLoaded()
{
	if (!bUseStreamedLevelTransition || !CurrentStreamedLevelName.IsNone())
	{
		return;
	}

	const FName ConfiguredInitialLevelName = GetConfiguredInitialStreamedLevelName();
	if (ConfiguredInitialLevelName.IsNone())
	{
		return;
	}

	if (IsStreamedLevelLoaded(ConfiguredInitialLevelName))
	{
		CurrentStreamedLevelName = ConfiguredInitialLevelName;
		SetStreamedLevelVisibility(CurrentStreamedLevelName, true);
		return;
	}

	PendingStreamedLevelName = ConfiguredInitialLevelName;
	bIsStreamingTransitionInProgress = true;
	LoadStreamedLevel(ConfiguredInitialLevelName);
}

void ULevelManager::HandleMapWorldEntered()
{
	if (UMapManager* MapManager = GetMapManager())
	{
		if (MapManager->HasMapData())
		{
			MapManager->RestoreMapWorld();
			if (URunSystem* RunSystem = GetRunSystem())
			{
				RunSystem->RefreshRunState();
			}
			return;
		}

		MapManager->StartRunSystem();
	}
}

UWorld* ULevelManager::GetStreamingWorld() const
{
	return ActiveWorld.Get() ? ActiveWorld.Get() : GetWorld();
}

UMapManager* ULevelManager::GetMapManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UMapManager>() : nullptr;
}

URunSystem* ULevelManager::GetRunSystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<URunSystem>() : nullptr;
}

ULevelStreaming* ULevelManager::FindStreamingLevel(FName LevelName) const
{
	if (LevelName.IsNone())
	{
		return nullptr;
	}

	if (UWorld* World = GetStreamingWorld())
	{
		if (ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(World, LevelName))
		{
			return StreamingLevel;
		}
	}

	return nullptr;
}

bool ULevelManager::IsStreamedLevelLoaded(FName LevelName) const
{
	if (ULevelStreaming* StreamingLevel = FindStreamingLevel(LevelName))
	{
		return StreamingLevel->GetLoadedLevel() != nullptr;
	}

	return false;
}

void ULevelManager::SetStreamedLevelVisibility(FName LevelName, bool bVisible) const
{
	if (ULevelStreaming* StreamingLevel = FindStreamingLevel(LevelName))
	{
		StreamingLevel->SetShouldBeVisible(bVisible);
	}
}
