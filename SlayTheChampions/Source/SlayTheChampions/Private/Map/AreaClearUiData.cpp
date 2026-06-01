#include "Map/AreaClearUiData.h"

#include "Reward/BattleClearWidget.h"

TSubclassOf<UBattleClearWidget> UAreaClearUiData::GetAreaClearWidgetClass(EAreaType AreaType) const
{
	switch (AreaType)
	{
	case EAreaType::Normal:
	case EAreaType::Elite:
	case EAreaType::Boss:
		return BattleAreaClearWidget;
	case EAreaType::Event:
		return EventAreaClearWidget;
	case EAreaType::Rest:
		return RestAreaClearWidget;
	case EAreaType::Shop:
		return ShopAreaClearWidget;
	case EAreaType::Reword:
		return RewardAreaClearWidget;
	case EAreaType::ArtifactEvent:
		return ArtifactEventAreaClearWidget;
	default:
		return nullptr;
	}
}

