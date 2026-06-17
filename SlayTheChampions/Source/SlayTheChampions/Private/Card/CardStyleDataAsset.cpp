#include "Card/CardStyleDataAsset.h"

UPaperSprite* UCardStyleDataAsset::GetRarityBorder(ECardRarity Rarity) const
{
    switch (Rarity)
    {
    case ECardRarity::Rare:      return RarityBorder_Rare.LoadSynchronous();
    case ECardRarity::Legendary: return RarityBorder_Legendary.LoadSynchronous();
    default:                     return RarityBorder_Normal.LoadSynchronous();
    }
}

UPaperSprite* UCardStyleDataAsset::GetTypeBackground(ECardRarity Rarity) const
{
    switch (Rarity)
    {
    case ECardRarity::Rare:      return TypeBackground_Rare.LoadSynchronous();
    case ECardRarity::Legendary: return TypeBackground_Legendary.LoadSynchronous();
    default:                     return TypeBackground_Normal.LoadSynchronous();
    }
}//test
