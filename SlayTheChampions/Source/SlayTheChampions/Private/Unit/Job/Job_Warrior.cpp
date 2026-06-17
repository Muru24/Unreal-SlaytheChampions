// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Job/Job_Warrior.h"
#include "GameplayTagsManager.h"

void UJob_Warrior::Initialize_Implementation(UActorComponent* OwnerComponent)
{
	// GameplayTag 캐시: 문자열 룩업을 BeginPlay 한 번으로 제한
	TagEnhance = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Warrior.Enhance"), /*ErrorIfNotFound=*/false);
	TagAttack = UGameplayTagsManager::Get().RequestGameplayTag(
		FName("Card.Warrior.Attack"), false);

}

void UJob_Warrior::OnCardPlayed_Implementation(FGameplayTag CardTag, int32 CardValue)
{
	// 강화 카드 사용 시 스택 증가
	if (TagEnhance.IsValid() && CardTag.MatchesTagExact(TagEnhance))
	{
		EnhanceStack++;
		UE_LOG(LogTemp, Log, TEXT("[Warrior] EnhanceStack=%d / %d"),
			EnhanceStack, EnhanceStackThreshold);

		if (EnhanceStack >= EnhanceStackThreshold)
		{
			bEnhanceAttackReady = true;
			UE_LOG(LogTemp, Log, TEXT("[Warrior] PromotedAttack READY!"));
		}
	}
}

float UJob_Warrior::ModifyCardDamage_Implementation(FGameplayTag CardTag, float BaseDamage)
{
	// 승격 공격 조건: 공격 카드 & bPromotedAttackReady
	if (TagAttack.IsValid() && CardTag.MatchesTagExact(TagAttack) && bEnhanceAttackReady)
	{
		const float FinalDamage = BaseDamage * DamageMultiplier; 
		

		// 스택 소모 & 플래그 리셋
		EnhanceStack = 0;
		bEnhanceAttackReady = false;

		return FinalDamage;
	}
	return BaseDamage;
}

void UJob_Warrior::OnTurnStart_Implementation()
{
}

void UJob_Warrior::OnTurnEnd_Implementation()
{
}
