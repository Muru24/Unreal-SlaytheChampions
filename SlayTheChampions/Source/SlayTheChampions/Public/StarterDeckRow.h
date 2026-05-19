#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StarterDeckRow.generated.h"

/**
 * FStarterDeckRow
 *
 * 캐릭터별 시작 덱 DataTable Row Struct.
 * DT_StarterDeck_Warrior, DT_StarterDeck_Mage 등에서 사용.
 * CardID 는 DT_Cards 의 _CardID 와 일치해야 한다.
 */
USTRUCT(BlueprintType)
struct FStarterDeckRow : public FTableRowBase
{
    GENERATED_BODY()

    // DT_Cards 의 _CardID (101, 201 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deck")
    int32 CardID = 0;
};
