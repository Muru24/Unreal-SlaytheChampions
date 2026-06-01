#include "Reward/RewardSystem.h"

#include "Map/AreaClearUiData.h"
#include "Reward/BattleClearWidget.h"
#include "Map/RunSystem.h"
#include "Hud/HUDManager.h"
#include "Hud/HudDefine.h"

void URewardSystem::Initialize(URunSystem* InRunSystem)
{
	RunSystem = InRunSystem;
	ActiveAreaClearWidget = nullptr;
	LoadDefaultAreaClearUiData();
}

void URewardSystem::OpenAreaClearReward(const FAreaInfo& InClearedAreaInfo)
{
	ClearedAreaInfo = InClearedAreaInfo;
	OpenAreaClearWidget();
}

void URewardSystem::ReturnToMapAfterReward()
{
	if (ActiveAreaClearWidget)
	{
		ActiveAreaClearWidget->RemoveFromParent();
		ActiveAreaClearWidget = nullptr;
	}

	if (!RunSystem)
	{
		return;
	}

	RunSystem->ReturnToMapAfterAreaClear();
}

void URewardSystem::SetAreaClearUiData(UAreaClearUiData* InAreaClearUiData)
{
	AreaClearUiData = InAreaClearUiData;
}

bool URewardSystem::LoadDefaultAreaClearUiData()
{
	if (!AreaClearUiData)
	{
		AreaClearUiData = LoadObject<UAreaClearUiData>(nullptr, TEXT("/Game/04_Data/AreaClearUiData.AreaClearUiData"));
	}

	return AreaClearUiData != nullptr;
}

void URewardSystem::OpenAreaClearWidget()
{
	if (!RunSystem || ActiveAreaClearWidget)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
	{
		return;
	}

	UHUDManager* Hud = World->GetGameInstance()->GetSubsystem<UHUDManager>();
	if (Hud)
	{
		Hud->ShowWidgetByName(HudDefine::BattleHUD);
	}
}

