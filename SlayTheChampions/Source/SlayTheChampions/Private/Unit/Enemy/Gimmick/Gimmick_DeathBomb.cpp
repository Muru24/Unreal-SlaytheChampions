// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Enemy/Gimmick/Gimmick_DeathBomb.h"

void UGimmick_DeathBomb::HandleHPChanged(int32 OldHP, int32 NewHP)
{
    if (NewHP < OldHP)
    {
        AccumulatedDamage += (OldHP - NewHP);
    }
}

void UGimmick_DeathBomb::HandleOwnerDied(AUnit* Unit)
{
    if (AccumulatedDamage > 0)
    {
        OnGimmickDamageRequest.Broadcast(ETargetType::AllEnemies, AccumulatedDamage);
        OnGimmickAnnounce.Broadcast(DeathAnnounce);
    }
}