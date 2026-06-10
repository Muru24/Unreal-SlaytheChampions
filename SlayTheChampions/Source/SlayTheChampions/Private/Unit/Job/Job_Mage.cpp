// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Job/Job_Mage.h"
#include "GameplayTagsManager.h"

void UJob_Mage::Initialize_Implementation(UActorComponent* OwnerComponent)
{
	TagFire = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Mage.Element.Fire"), false);
	TagIce = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Mage.Element.Ice"), false);
	TagThunder = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Mage.Element.Thunder"), false);

}

ECardElementType UJob_Mage::ResolveElement(const FGameplayTag& CardTag) const
{
	if (TagFire.IsValid() && CardTag.MatchesTagExact(TagFire))    return ECardElementType::Fire;
	if (TagIce.IsValid() && CardTag.MatchesTagExact(TagIce))     return ECardElementType::Ice;
	if (TagThunder.IsValid() && CardTag.MatchesTagExact(TagThunder)) return ECardElementType::Thunder;
	return ECardElementType::None;
}

void UJob_Mage::OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue)
{
	const ECardElementType NewElement = ResolveElement(CardTag);
	if (NewElement == ECardElementType::None) return;

	if (FieldElement != ECardElementType::None && FieldElement != NewElement)
	{
		//다른 원소 ->반응 준비
		bReactionReady = true;
	}
	FieldElement = NewElement;
}

float UJob_Mage::ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage)
{
	if (bReactionReady)
	{
		const float FinalDamage = BaseDamage * ReactionDamageMultiplier;
		
		FieldElement = ECardElementType::None;
		bReactionReady = false;

		return FinalDamage;
	}
	return BaseDamage;
}

void UJob_Mage::OnTurnStart_Implementation()
{
	//확장 필드원소를 매턴 유지할지  만료시킬지
}

void UJob_Mage::OnTurnEnd_Implementation()
{
	//필드 지속턴 카운트
}


