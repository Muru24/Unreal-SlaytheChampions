// Fill out your copyright notice in the Description page of Project Settings.


#include "VFX/SkillVFXDataAsset.h"

TArray<FSkillVfxEntry> USkillVFXDataAsset::GetVfxEntries(FName CardID, FName VfxTag) const
{
    TArray<FSkillVfxEntry> Result;

    // 1. 우선순위가 높은 MultiSkillVfxMap 먼저 검사
    if (const FSkillVfxEntryList* FoundList = MultiSkillVfxMap.Find(CardID))
    {
        // .Entries를 통해 구조체 내부의 TArray에 접근합니다.
        for (const FSkillVfxEntry& Entry : FoundList->Entries)
        {
            if (VfxTag == NAME_None || Entry.VfxTag == NAME_None || Entry.VfxTag == VfxTag)
            {
                Result.Add(Entry);
            }
        }
    }
    // 2. Multi 맵에 없다면 단일 SkillVfxMap 검사 (Fallback)
    else if (const FSkillVfxEntry* FoundEntry = SkillVfxMap.Find(CardID))
    {
        if (VfxTag == NAME_None || FoundEntry->VfxTag == NAME_None || FoundEntry->VfxTag == VfxTag)
        {
            Result.Add(*FoundEntry);
        }
    }

    return Result;
}

FPrimaryAssetId USkillVFXDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("SkillVfxDataAsset"), GetFName());
}
