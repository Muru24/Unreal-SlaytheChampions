#include "Item/ItemVisualDataAsset.h"

bool UItemVisualDataAsset::FindVisualData(EItemActorType ItemType, FName ItemID, FItemVisualData& OutVisualData) const
{
	const FItemVisualData* FoundVisualData = Visuals.FindByPredicate(
		[ItemType, ItemID](const FItemVisualData& VisualData)
		{
			return VisualData.ItemType == ItemType && VisualData.ItemID == ItemID;
		});

	if (!FoundVisualData)
	{
		return false;
	}

	OutVisualData = *FoundVisualData;
	return true;
}
