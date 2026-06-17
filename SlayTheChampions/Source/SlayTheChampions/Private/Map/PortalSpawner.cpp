#include "Map/PortalSpawner.h"

#include "Engine/World.h"
#include "Map/AreaLevelData.h"
#include "Map/MapAreaActor.h"
#include "Map/MapConfigData.h"
#include "Map/RunSystem.h"
#include "TimerManager.h"

APortalSpawner::APortalSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APortalSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		if (bUseEnterableRooms)
		{
			if (URunSystem* RunSystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URunSystem>() : nullptr)
			{
				RunSystem->OnEnterableRoomsUpdated.AddUniqueDynamic(this, &APortalSpawner::HandleEnterableRoomsUpdated);
			}

			SpawnEnterableRoomPortals();
		}
		else
		{
			SpawnPortals();
		}
	}
}

void APortalSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (URunSystem* RunSystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URunSystem>() : nullptr)
	{
		RunSystem->OnEnterableRoomsUpdated.RemoveDynamic(this, &APortalSpawner::HandleEnterableRoomsUpdated);
	}

	Super::EndPlay(EndPlayReason);
}

void APortalSpawner::SpawnPortals()
{
	if (!GetWorld() || !PortalActorClass || PortalCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PortalSpawner] SpawnPortals failed. PortalActorClass=%s PortalCount=%d"),
			PortalActorClass ? TEXT("Valid") : TEXT("None"),
			PortalCount);
		return;
	}

	if (bClearBeforeSpawn)
	{
		ClearSpawnedPortals();
	}

	for (int32 Index = 0; Index < PortalCount; ++Index)
	{
		SpawnPortalAtIndex(Index, PortalCount);
	}
}

void APortalSpawner::SpawnEnterableRoomPortals()
{
	URunSystem* RunSystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<URunSystem>() : nullptr;
	if (!RunSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PortalSpawner] SpawnEnterableRoomPortals failed. RunSystem is null."));
		return;
	}

	RunSystem->RefreshRunState();
	SpawnPortalsForRooms(RunSystem->GetEnterableRooms());
}

void APortalSpawner::SpawnPortalsForRooms(const TArray<FAreaInfo>& RoomInfos)
{
	if (!GetWorld() || !PortalActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PortalSpawner] SpawnPortalsForRooms failed. PortalActorClass=%s"),
			PortalActorClass ? TEXT("Valid") : TEXT("None"));
		return;
	}

	if (RoomInfos.IsEmpty())
	{
		if (bClearBeforeSpawn)
		{
			ClearSpawnedPortals();
		}

		if (CurrentEmptyRoomRetryCount < EmptyRoomRetryCount)
		{
			CurrentEmptyRoomRetryCount++;
			FTimerHandle RetryTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				RetryTimerHandle,
				this,
				&APortalSpawner::RetrySpawnEnterableRoomPortals,
				EmptyRoomRetryDelay,
				false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PortalSpawner] Enterable rooms are empty after retry. PortalCount=0"));
		}
		return;
	}

	CurrentEmptyRoomRetryCount = 0;

	if (bClearBeforeSpawn)
	{
		ClearSpawnedPortals();
	}

	UAreaLevelData* AreaLevelData = nullptr;
	if (UMapConfigData* MapConfig = LoadObject<UMapConfigData>(nullptr, TEXT("/Game/04_Data/MapCreatorData.MapCreatorData")))
	{
		AreaLevelData = MapConfig->AreaLevelData;
	}

	for (int32 Index = 0; Index < RoomInfos.Num(); ++Index)
	{
		AActor* SpawnedPortal = SpawnPortalAtIndex(Index, RoomInfos.Num());
		if (AMapAreaActor* MapAreaActor = Cast<AMapAreaActor>(SpawnedPortal))
		{
			const FAreaInfo& RoomInfo = RoomInfos[Index];
			MapAreaActor->SetAreaIndex(static_cast<int32>(RoomInfo.AreaPos.X), static_cast<int32>(RoomInfo.AreaPos.Y));
			MapAreaActor->ApplyDebugAreaInfo(RoomInfo);
			if (AreaLevelData)
			{
				MapAreaActor->SetTargetLevelName(AreaLevelData->GetLevelName(RoomInfo.AreaType));
			}
		}
	}
}

void APortalSpawner::HandleEnterableRoomsUpdated(const TArray<FAreaInfo>& RoomInfos)
{
	SpawnPortalsForRooms(RoomInfos);
}

void APortalSpawner::RetrySpawnEnterableRoomPortals()
{
	SpawnEnterableRoomPortals();
}

AActor* APortalSpawner::SpawnPortalAtIndex(int32 Index, int32 Count)
{
	if (!GetWorld() || !PortalActorClass || Count <= 0)
	{
		return nullptr;
	}

	const FVector CenterLocation = GetActorLocation();
	const FVector UpVector = FVector::UpVector;
	const FVector ForwardVector = bUseSpawnerForwardDirection ? GetActorForwardVector() : FVector::ForwardVector;
	const FVector RightVector = bUseSpawnerForwardDirection ? GetActorRightVector() : FVector::RightVector;

	float TotalArcAngleDegrees = ArcAngleDegrees;
	float AngleStepDegrees = Count > 1 ? ArcAngleDegrees / static_cast<float>(Count - 1) : 0.f;
	if (PortalSpacing > 0.f && Radius > 0.f && Count > 1)
	{
		AngleStepDegrees = FMath::RadiansToDegrees(PortalSpacing / Radius);
		TotalArcAngleDegrees = AngleStepDegrees * static_cast<float>(Count - 1);
	}

	const float StartAngleDegrees = -TotalArcAngleDegrees * 0.5f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const float AngleDegrees = Count > 1 ? StartAngleDegrees + (AngleStepDegrees * Index) : 0.f;
	const float AngleRadians = FMath::DegreesToRadians(AngleDegrees);

	const FVector ArcOffset =
		(ForwardVector * FMath::Cos(AngleRadians) * Radius) +
		(RightVector * FMath::Sin(AngleRadians) * Radius) +
		(UpVector * HeightOffset);

	const FVector SpawnLocation = CenterLocation + ArcOffset;
	const FRotator SpawnRotation = bPortalLookAtSpawner
		? (CenterLocation - SpawnLocation).Rotation()
		: GetActorRotation();

	if (AActor* SpawnedPortal = GetWorld()->SpawnActor<AActor>(PortalActorClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		SpawnedPortals.Add(SpawnedPortal);
		return SpawnedPortal;
	}

	UE_LOG(LogTemp, Warning, TEXT("[PortalSpawner] SpawnActor failed. Index=%d Count=%d Location=%s"),
		Index,
		Count,
		*SpawnLocation.ToString());
	return nullptr;
}

void APortalSpawner::ClearSpawnedPortals()
{
	for (AActor* SpawnedPortal : SpawnedPortals)
	{
		if (IsValid(SpawnedPortal))
		{
			SpawnedPortal->Destroy();
		}
	}

	SpawnedPortals.Empty();
}
