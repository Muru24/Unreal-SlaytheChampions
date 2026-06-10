// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Job/Job_Healer.h"
#include "GameplayTagsmanager.h"

void UJob_Healer::Initialize_Implementation(UActorComponent* OwnerComponent)
{
	TagHeal = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Healer.Heal"), false);
	TagShield = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Healer.Shield"), false);
	TagBuff = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Healer.Buff"), false);
}

void UJob_Healer::OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue)
{
	const bool bIsHealCard =
		(TagHeal.IsValid() && CardTag.MatchesTagExact(TagHeal)) ||
		(TagShield.IsValid() && CardTag.MatchesTagExact(TagShield)) ||
		(TagBuff.IsValid() && CardTag.MatchesTagExact(TagBuff));

	if (!bIsHealCard) return;

	HealStack++;
	UE_LOG(LogTemp, Log, TEXT("[Healer] HealStack=%d / %d"),
		HealStack, HealStackThreshold);

	if (HealStack >= HealStackThreshold)
	{
		bHealReady = true;
		UE_LOG(LogTemp, Log, TEXT("[Healer] BurstHeal READY!"));
	}
}

int32 UJob_Healer::ModifyHealAmount(FGameplayTag CardTag, int32 BaseHeal)
{
	if (bHealReady)
	{
		const int32 FinalHeal = BaseHeal * HealMultiplier;
		HealStack = 0;
		bHealReady = false;

		return FinalHeal;
	}
    return BaseHeal;
}





float UJob_Healer::ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage)
{
	return BaseDamage;
}

void UJob_Healer::OnTurnStart_Implementation()
{
	//매턴 효과처리
}

void UJob_Healer::OnTurnEnd_Implementation()
{
	//보호막 만료 처리
}
