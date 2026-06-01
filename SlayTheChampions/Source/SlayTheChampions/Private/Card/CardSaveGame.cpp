#include "Card/CardSaveGame.h"
#include "Card/StarterDeckRow.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

const FString UCardSaveGame::SlotName = TEXT("PlayerDeckSave");
const int32 UCardSaveGame::UserIndex = 0;

UCardSaveGame* UCardSaveGame::LoadSave()
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
        return nullptr;

    return Cast<UCardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
}

void UCardSaveGame::WriteSave(UCardSaveGame* SaveGame)
{
    if (!SaveGame) return;
    UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] WriteSave completed"));
}

UCardSaveGame* UCardSaveGame::LoadOrCreate(UDataTable* StarterDeckWarrior, UDataTable* StarterDeckMage)
{
    // 기존 SaveGame 파일이 있으면 로드
    UCardSaveGame* Save = LoadSave();
    if (Save)
    {
        UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Loaded existing SaveGame"));
        return Save;
    }

    // SaveGame 없으면 DT 에서 초기 덱 생성
    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] No SaveGame found - Creating from DataTable"));

    Save = Cast<UCardSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UCardSaveGame::StaticClass()));

    // Pawn1(Warrior), Pawn2(Mage) 2명 슬롯 초기화
    Save->PartyDecks.SetNum(2);

    // Pawn1: Warrior 스타터 덱
    Save->PartyDecks[0].JobClass = EJobClass::Warrior;
    if (StarterDeckWarrior)
    {
        for (const FName& RowName : StarterDeckWarrior->GetRowNames())
        {
            const FStarterDeckRow* Row = StarterDeckWarrior->FindRow<FStarterDeckRow>(
                RowName, TEXT("LoadOrCreate_Warrior"));
            if (Row)
                Save->PartyDecks[0].DeckCards.Add(Row->CardID);
        }
        UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Warrior deck: %d cards"),
            Save->PartyDecks[0].DeckCards.Num());
    }

    // Pawn2: Mage 스타터 덱
    Save->PartyDecks[1].JobClass = EJobClass::Mage;
    if (StarterDeckMage)
    {
        for (const FName& RowName : StarterDeckMage->GetRowNames())
        {
            const FStarterDeckRow* Row = StarterDeckMage->FindRow<FStarterDeckRow>(
                RowName, TEXT("LoadOrCreate_Mage"));
            if (Row)
                Save->PartyDecks[1].DeckCards.Add(Row->CardID);
        }
        UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Mage deck: %d cards"),
            Save->PartyDecks[1].DeckCards.Num());
    }

    WriteSave(Save);
    return Save;
}

void UCardSaveGame::SaveDeckAfterBattle(int32 PawnIndex,
    const TArray<FName>& DrawPile,
    const TArray<FName>& Hand,
    const TArray<FName>& DiscardPile)
{
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardSaveGame] SaveDeckAfterBattle: Invalid PawnIndex %d"), PawnIndex);
        return;
    }

    // A방식: DrawPile + Hand + DiscardPile 합산 저장 (ExhaustPile 제외)
    TArray<FName> Combined;
    Combined.Append(DrawPile);
    Combined.Append(Hand);
    Combined.Append(DiscardPile);

    Save->PartyDecks[PawnIndex].DeckCards = Combined;
    WriteSave(Save);

    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Pawn%d battle end saved - %d cards"),
        PawnIndex, Combined.Num());
}

void UCardSaveGame::AddCard(int32 PawnIndex, FName CardName)
{
    // 보상/상점에서 카드를 덱에 추가할 때 호출
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardSaveGame] AddCard: Invalid PawnIndex %d"), PawnIndex);
        return;
    }

    Save->PartyDecks[PawnIndex].DeckCards.Add(CardName);
    WriteSave(Save);

    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Pawn%d card added - %s (total %d)"),
        PawnIndex, *CardName.ToString(),
        Save->PartyDecks[PawnIndex].DeckCards.Num());
}

void UCardSaveGame::RemoveCard(int32 PawnIndex, FName CardName)
{
    // 상점/특수 이벤트에서 덱에서 카드를 영구 제거할 때 호출
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardSaveGame] RemoveCard: Invalid PawnIndex %d"), PawnIndex);
        return;
    }

    Save->PartyDecks[PawnIndex].DeckCards.Remove(CardName);
    WriteSave(Save);

    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Pawn%d card removed - %s"),
        PawnIndex, *CardName.ToString());
}

TArray<FName> UCardSaveGame::GetDeckCards(int32 PawnIndex)
{
    // 저장된 덱 카드 목록 반환. 없으면 빈 배열.
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
        return {};

    return Save->PartyDecks[PawnIndex].DeckCards;
}

EJobClass UCardSaveGame::GetJobClass(int32 PawnIndex)
{
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        return EJobClass::Any;
    }

    return Save->PartyDecks[PawnIndex].JobClass;
}
