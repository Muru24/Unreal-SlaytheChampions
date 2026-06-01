#include "Party/PartyInstance.h"

void UPartyInstance::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (PartyInfo.Champions.IsEmpty())
	{
		InitParty();
	}
}

void UPartyInstance::InitParty()
{
	PartyInfo.InitSavePartyInfo();
	PartyInfo.Champions.SetNum(2);
	PartyInfo.Deck.SetNum(2);
}


void UPartyInstance::AddGold(int32 _Gold)
{
	UE_LOG(LogTemp, Warning, TEXT("골드획득 : %d"), _Gold);
	PartyInfo.Gold += _Gold;
	if (PartyInfo.Gold >= PartyInfo.MaxGold)
		PartyInfo.Gold = PartyInfo.MaxGold;

	UE_LOG(LogTemp, Warning, TEXT("현재 골드 : %d"), PartyInfo.Gold);
}

void UPartyInstance::UseGold(int32 _Price)
{
	UE_LOG(LogTemp, Warning, TEXT("골드사용 : %d"), _Price);
	if (PartyInfo.Gold >= _Price)
	{
		PartyInfo.Gold -= _Price;
	}

	if (PartyInfo.Gold < 0) PartyInfo.Gold = 0;
	UE_LOG(LogTemp, Warning, TEXT("현재 골드 : %d"), PartyInfo.Gold);
}

void UPartyInstance::AddRelic(FRelic _Relic)
{
	UE_LOG(LogTemp, Warning, TEXT("유물추가 : %s"), *_Relic.RelicID.ToString());
	PartyInfo.Relics.Add(_Relic);
	if (PartyInfo.Relics.ContainsByPredicate([_Relic](const FRelic& Item)
	{
		return Item.RelicID == _Relic.RelicID;
	}))
	{
		UE_LOG(LogTemp, Warning, TEXT("유물획득성공 : %s"), *_Relic.RelicID.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("유물획득실패"));
	}
	
}

void UPartyInstance::LostRelic(FName _RelicName) 
{
	PartyInfo.Relics.RemoveAll([_RelicName](const FRelic& Item)
	{
		return Item.RelicID == _RelicName;
	});
}

void UPartyInstance::AddPotion(FPotionData _Potion)
{
	PartyInfo.Potions.Add(_Potion);
}

void UPartyInstance::LostPotion(FName _PotionName)
{
	PartyInfo.Potions.RemoveAll([_PotionName](const FPotionData& Item)
	{
		return Item.PotionID == _PotionName;
	});
}
