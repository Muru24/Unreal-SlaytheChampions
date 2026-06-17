// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Enemy/Gimmick/Gimmick_SlowStarter.h"

void UGimmick_SlowStarter::OnGimmickTurnStart()
{
	if (bSleeping && TurnCounter >= SleepDuration)
	{
		Awaken();
	}
}

void UGimmick_SlowStarter::HandleHPChanged(int32 OldHP, int32 NewHP)
{
	// 피격 시 조기 각성
	if (bWakeOnDamage && bSleeping && NewHP < OldHP)
	{
		Awaken();
	}
}

void UGimmick_SlowStarter::Awaken()
{
	if (!bSleeping) return;

	bSleeping = false;
	OnAwakened.Broadcast();
	OnGimmickAnnounce.Broadcast(AwakenAnnounce);
}
