#include "Map/MapAreaActor.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameManagers/LevelManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Map/RunSystem.h"

AMapAreaActor::AMapAreaActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMapAreaActor::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		PlayerController->bEnableClickEvents = true;
		PlayerController->bShowMouseCursor = true;
	}
}

void AMapAreaActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	MoveToTargetLevel();
}

void AMapAreaActor::SetAreaIndex(int32 InFloorIndex, int32 InRoomIndex)
{
	FloorIndex = InFloorIndex;
	RoomIndex = InRoomIndex;
}

void AMapAreaActor::ApplyDebugAreaInfo(const FAreaInfo& InAreaInfo)
{
	FLinearColor DebugColor = FLinearColor::Green;
	if (InAreaInfo.AreaVisit == EAreaVisitState::Cleared || InAreaInfo.AreaVisit == EAreaVisitState::Visited)
	{
		DebugColor = FLinearColor::Blue;
	}
	else if (InAreaInfo.bCanEnter)
	{
		DebugColor = FLinearColor::Red;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		const int32 MaterialCount = PrimitiveComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; MaterialIndex++)
		{
			if (UMaterialInstanceDynamic* DynamicMaterial = PrimitiveComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
			{
				DynamicMaterial->SetVectorParameterValue(DebugColorParameterName, DebugColor);
				DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DebugColor);
				DynamicMaterial->SetVectorParameterValue(TEXT("TintColor"), DebugColor);
				DynamicMaterial->SetVectorParameterValue(TEXT("DebugColor"), DebugColor);
			}
		}
	}
}

void AMapAreaActor::SetTargetLevelName(FName InTargetLevelName)
{
	TargetLevelName = InTargetLevelName;
}

void AMapAreaActor::MoveToTargetLevel()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URunSystem* RunSystem = GameInstance->GetSubsystem<URunSystem>())
		{
			if (!RunSystem->CanEnterRoomByGridIndex(FloorIndex, RoomIndex))
			{
				return;
			}

			if (!RunSystem->EnterRoomByGridIndex(FloorIndex, RoomIndex))
			{
				return;
			}
		}

		if (ULevelManager* LevelManager = GameInstance->GetSubsystem<ULevelManager>())
		{
			if (!TargetLevelName.IsNone())
			{
				LevelManager->MoveToConfiguredLevel(TargetLevelName);
			}
		}
	}
}
