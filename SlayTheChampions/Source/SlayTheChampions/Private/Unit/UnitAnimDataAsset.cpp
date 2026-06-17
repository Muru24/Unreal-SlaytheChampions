// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/UnitAnimDataAsset.h"

const FUnitAnimEntry& UUnitAnimDataAsset::GetEntry(EUnitAnimType AnimType) const
{
    switch (AnimType)
    {
    case EUnitAnimType::Attack:  return AttackAnim;
    case EUnitAnimType::Hit:     return HitAnim;
    case EUnitAnimType::Death:   return DeathAnim;
    case EUnitAnimType::Skill:   return SkillAnim;
    case EUnitAnimType::Move:    return MoveAnim;
    case EUnitAnimType::Victory: return VictoryAnim;
    case EUnitAnimType::Idle:    // fall-through
    default:                     return IdleAnim;
    }
}

FPrimaryAssetId UUnitAnimDataAsset::GetPrimaryAssetId() const
{
    // AssetManager 등록 시 "UnitAnim" 타입으로 묶임
    return FPrimaryAssetId(TEXT("UnitAnim"), GetFName());
}
