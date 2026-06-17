#include "Map/DebugRunInputActor.h"

#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Map/RunSystem.h"

ADebugRunInputActor::ADebugRunInputActor()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoReceiveInput = EAutoReceiveInput::Player0;
}

void ADebugRunInputActor::BeginPlay()
{
	Super::BeginPlay();

	// 디버그 입력을 받기 위해 플레이어 입력을 연결한다.
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		EnableInput(PlayerController);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ADebugRunInputActor::DebugClearCurrentStage);
		}
	}
}

void ADebugRunInputActor::DebugClearCurrentStage()
{
	if (!GetWorld())
	{
		return;
	}

	const FString CurrentMapName = GetWorld()->RemovePIEPrefix(GetWorld()->GetMapName());
	if (CurrentMapName == ReturnLevelName.ToString())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URunSystem* RunSystem = GameInstance->GetSubsystem<URunSystem>())
		{
			RunSystem->AreaCleared();
		}
	}
}
