// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Enemy/Gimmick/Gimmick_Summoner.h"
#include "Unit/Unit.h"
#include "Unit/StatComponent.h"

bool UGimmick_Summoner::HasAliveMinion() const
{
	return GetAliveMinionCount() > 0;
}

int32 UGimmick_Summoner::GetAliveMinionCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AUnit>& Ref : SpawnedMinions)
	{
		if (Ref.IsValid() && Ref->IsAlive())
		{
			++Count;
		}
	}
	return Count;
}

void UGimmick_Summoner::RegisterMinion(AUnit* Minion)
{
	if (Minion)
	{
		SpawnedMinions.Add(Minion);
	}
}

void UGimmick_Summoner::BeginPlay()
{
	Super::BeginPlay();
}

void UGimmick_Summoner::OnGimmickTurnStart()
{
	++TurnsSinceLastSummon;

	// 하수인이 없고 쿨타임 충족 시 재소환
	const bool bNeedSummon = !bHasSummoned
		|| (!HasAliveMinion() && TurnsSinceLastSummon >= ResummonCooldown);

	if (bNeedSummon)
	{
		OnMinionSpawnRequested.Broadcast(MinionCount);
		OnGimmickAnnounce.Broadcast(SummonAnnounce);
		TurnsSinceLastSummon = 0;
		bHasSummoned = true;
		return; // 소환 턴에는 힐 안 함
	}

	// 하수인이 살아있으면 보스 회복
	if (HasAliveMinion() && HealPerTurn > 0)
	{
		if (UStatComponent* Stat = GetOwner()->FindComponentByClass<UStatComponent>())
		{
			Stat->Heal(HealPerTurn);
		}
	}
}
