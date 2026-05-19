#include "CardSubsystem.h"
#include "Engine/DataTable.h"

// ── Lifecycle ─────────────────────────────────────────────────────────────

void UCardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 에디터 없이도 DataTable 자동 로드
    // 실제 프로젝트에 맞게 경로 수정 필요
    static const FSoftObjectPath CardTablePath(
        TEXT("01/DT_Cards"));

    if (UDataTable* Loaded = Cast<UDataTable>(CardTablePath.TryLoad()))
    {
        LoadCardDataTable(Loaded);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UCardSubsystem: DT_Cards 를 '%s' 경로에서 찾을 수 없음. "
                "LoadCardDataTable() 을 직접 호출하세요."),
            *CardTablePath.ToString());
    }
}

// ── Public API ────────────────────────────────────────────────────────────

void UCardSubsystem::LoadCardDataTable(UDataTable* InTable)
{
    CardDataTable = InTable;

    if (!CardDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("UCardSubsystem: LoadCardDataTable 에 Null DataTable 이 전달됨."));
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("UCardSubsystem: DataTable '%s' 로드 완료 (%d rows)."),
        *CardDataTable->GetName(),
        CardDataTable->GetRowNames().Num());
}

const FCardDataRow* UCardSubsystem::GetCard(int32 CardID) const
{
    if (!CardDataTable) return nullptr;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        const FCardDataRow* Row =
            CardDataTable->FindRow<FCardDataRow>(RowName, TEXT("UCardSubsystem::GetCard"));
        if (Row && Row->_CardID == CardID)
            return Row;
    }
    return nullptr;
}

TArray<int32> UCardSubsystem::GetCardIDsByClass(EJobClass JobClass) const
{
    TArray<int32> Result;
    if (!CardDataTable) return Result;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        const FCardDataRow* Row =
            CardDataTable->FindRow<FCardDataRow>(RowName, TEXT("GetCardIDsByClass"));

        if (Row && (Row->RequiredClass == JobClass || Row->RequiredClass == EJobClass::Any))
        {
            Result.Add(Row->_CardID);
        }
    }
    return Result;
}

TArray<const FCardDataRow*> UCardSubsystem::GetCardsByClass(EJobClass JobClass) const
{
    TArray<const FCardDataRow*> Result;
    if (!CardDataTable) return Result;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        const FCardDataRow* Row =
            CardDataTable->FindRow<FCardDataRow>(RowName, TEXT("GetCardsByClass"));

        if (Row && (Row->RequiredClass == JobClass || Row->RequiredClass == EJobClass::Any))
        {
            Result.Add(Row);
        }
    }
    return Result;
}

TArray<int32> UCardSubsystem::GetRewardPool(EJobClass JobClass, ECardRarity MinRarity) const
{
    TArray<int32> Result;
    if (!CardDataTable) return Result;

    const int32 MinRarityInt = RarityToInt(MinRarity);

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        const FCardDataRow* Row =
            CardDataTable->FindRow<FCardDataRow>(RowName, TEXT("GetRewardPool"));

        if (!Row) continue;

        const bool bClassOk =
            (Row->RequiredClass == JobClass || Row->RequiredClass == EJobClass::Any);
        const bool bRarityOk =
            (RarityToInt(Row->Rarity) >= MinRarityInt);

        if (bClassOk && bRarityOk)
        {
            Result.Add(Row->_CardID);
        }
    }
    return Result;
}

TArray<int32> UCardSubsystem::GetAllCardIDs() const
{
    TArray<int32> Result;
    if (!CardDataTable) return Result;

    for (const FName& RowName : CardDataTable->GetRowNames())
    {
        const FCardDataRow* Row =
            CardDataTable->FindRow<FCardDataRow>(RowName, TEXT("GetAllCardIDs"));
        if (Row)
            Result.Add(Row->_CardID);
    }
    return Result;
}

// ── Private Helpers ───────────────────────────────────────────────────────

int32 UCardSubsystem::RarityToInt(ECardRarity Rarity)
{
    switch (Rarity)
    {
    case ECardRarity::Status:    return 0;
    case ECardRarity::Normal:    return 1;
    case ECardRarity::Rare:      return 2;
    case ECardRarity::Legendary: return 3;
    default:                     return 0;
    }
}
