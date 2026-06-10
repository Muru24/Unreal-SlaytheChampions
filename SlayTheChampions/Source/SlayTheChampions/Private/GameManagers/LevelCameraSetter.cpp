#include "GameManagers/LevelCameraSetter.h"

#include "Camera/CameraActor.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/PlayerController.h"

ALevelCameraSetter::ALevelCameraSetter()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DefaultCameraSlot = CreateDefaultSubobject<UArrowComponent>(TEXT("DefaultCameraSlot"));
	DefaultCameraSlot->SetupAttachment(SceneRoot);
	DefaultCameraSlot->SetRelativeLocation(FVector(-500.f, 0.f, 200.f));
	DefaultCameraSlot->ArrowColor = FColor::White;
	DefaultCameraSlot->SetHiddenInGame(true);
}

void ALevelCameraSetter::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegisterChildArrowSlots)
	{
		AutoRegisterChildArrowSlots();
	}

	if (bSetCameraOnBeginPlay)
	{
		SpawnDefaultCamera();
		SwitchCameraToDefault(DefaultBlendTime);
	}
}

void ALevelCameraSetter::SpawnDefaultCamera()
{
	if (ActiveCamera || !CameraClass || !DefaultCameraSlot)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveCamera = World->SpawnActor<ACameraActor>(CameraClass, DefaultCameraSlot->GetComponentTransform(), SpawnParams);
	if (!ActiveCamera)
	{
		return;
	}

	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		PlayerController->SetViewTargetWithBlend(ActiveCamera, 0.f);
	}
}

void ALevelCameraSetter::SwitchCameraToDefault(float OverrideBlendTime)
{
	MoveCameraToSlot(DefaultCameraSlot, 0, OverrideBlendTime);
}

void ALevelCameraSetter::SwitchCameraToSlotIndex(int32 SlotIndex, float OverrideBlendTime)
{
	UArrowComponent* CameraSlot = GetCameraSlotByIndex(SlotIndex);
	if (!CameraSlot)
	{
		return;
	}

	MoveCameraToSlot(CameraSlot, SlotIndex, OverrideBlendTime);
}

void ALevelCameraSetter::SwitchCameraToSlot(USceneComponent* CameraSlot, float OverrideBlendTime)
{
	if (!CameraSlot)
	{
		return;
	}

	int32 SlotIndex = INDEX_NONE;
	if (CameraSlot == DefaultCameraSlot)
	{
		SlotIndex = 0;
	}
	else
	{
		for (int32 Index = 0; Index < AdditionalCameraSlots.Num(); ++Index)
		{
			if (AdditionalCameraSlots[Index] == CameraSlot)
			{
				SlotIndex = Index + 1;
				break;
			}
		}
	}

	MoveCameraToSlot(CameraSlot, SlotIndex, OverrideBlendTime);
}

void ALevelCameraSetter::RegisterAdditionalCameraSlot(UArrowComponent* CameraSlot)
{
	if (!CameraSlot || CameraSlot == DefaultCameraSlot)
	{
		return;
	}

	AdditionalCameraSlots.AddUnique(CameraSlot);
}

void ALevelCameraSetter::AutoRegisterChildArrowSlots()
{
	AdditionalCameraSlots.Reset();

	TArray<UArrowComponent*> ArrowComponents;
	GetComponents<UArrowComponent>(ArrowComponents);

	for (UArrowComponent* ArrowComponent : ArrowComponents)
	{
		if (!ArrowComponent || ArrowComponent == DefaultCameraSlot)
		{
			continue;
		}

		AdditionalCameraSlots.Add(ArrowComponent);
	}
}

void ALevelCameraSetter::ClearAdditionalCameraSlots()
{
	AdditionalCameraSlots.Reset();
}

UArrowComponent* ALevelCameraSetter::GetCameraSlotByIndex(int32 SlotIndex) const
{
	if (SlotIndex == 0)
	{
		return DefaultCameraSlot;
	}

	const int32 AdditionalIndex = SlotIndex - 1;
	return AdditionalCameraSlots.IsValidIndex(AdditionalIndex) ? AdditionalCameraSlots[AdditionalIndex] : nullptr;
}

void ALevelCameraSetter::MoveCameraToSlot(USceneComponent* CameraSlot, int32 SlotIndex, float OverrideBlendTime)
{
	if (!CameraSlot)
	{
		return;
	}

	if (!ActiveCamera)
	{
		SpawnDefaultCamera();
	}

	if (!ActiveCamera)
	{
		return;
	}

	const FTransform TargetTransform = CameraSlot->GetComponentTransform();
	const float BlendTime = ResolveBlendTime(OverrideBlendTime);

	if (bSetCameraTransformDirectly)
	{
		ActiveCamera->SetActorTransform(TargetTransform);
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetViewTargetWithBlend(ActiveCamera, BlendTime);
		}
	}

	OnLevelCameraMoveRequested.Broadcast(SlotIndex, TargetTransform, ActiveCamera);
	OnLevelCameraSwitched.Broadcast(SlotIndex, ActiveCamera);
}

float ALevelCameraSetter::ResolveBlendTime(float OverrideBlendTime) const
{
	return OverrideBlendTime >= 0.f ? OverrideBlendTime : DefaultBlendTime;
}
