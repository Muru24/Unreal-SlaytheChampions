#include "CardSaveGame.h"
#include "StarterDeckRow.h"
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
    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] 저장 완료"));
}

UCardSaveGame* UCardSaveGame::LoadOrCreate(UDataTable* StarterDeckWarrior, UDataTable* StarterDeckMage)
{
    // 기존 SaveGame 파일이 있으면 불러옴
    UCardSaveGame* Save = LoadSave();
    if (Save)
    {
        UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] 기존 SaveGame 로드 성공"));
        return Save;
    }

    // SaveGame 없으면 DT에서 초기 덱 생성
    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] SaveGame 없음 - DataTable 에서 초기 덱 생성"));

    Save = Cast<UCardSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UCardSaveGame::StaticClass()));

    // Pawn1(Warrior), Pawn2(Mage) 2명 초기화
    Save->PartyDecks.SetNum(2);

    // Pawn1: Warrior 시작 덱
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
        UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Warrior 덱: %d장"),
            Save->PartyDecks[0].DeckCards.Num());
    }

    // Pawn2: Mage 시작 덱
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
        UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Mage 덱: %d장"),
            Save->PartyDecks[1].DeckCards.Num());
    }

    WriteSave(Save);
    return Save;
}

void UCardSaveGame::SaveDeckAfterBattle(int32 PawnIndex,
    const TArray<int32>& DrawPile,
    const TArray<int32>& Hand,
    const TArray<int32>& DiscardPile)
{
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardSaveGame] SaveDeckAfterBattle: 잘못된 PawnIndex %d"), PawnIndex);
        return;
    }

    // A방식: DrawPile + Hand + DiscardPile 합쳐서 저장
    TArray<int32> Combined;
    Combined.Append(DrawPile);
    Combined.Append(Hand);
    Combined.Append(DiscardPile);

    Save->PartyDecks[PawnIndex].DeckCards = Combined;
    WriteSave(Save);

    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Pawn%d 전투 종료 저장 - %d장"),
        PawnIndex, Combined.Num());
}

void UCardSaveGame::AddCard(int32 PawnIndex, int32 CardID)
{
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardSaveGame] AddCard: 잘못된 PawnIndex %d"), PawnIndex);
        return;
    }

    Save->PartyDecks[PawnIndex].DeckCards.Add(CardID);
    WriteSave(Save);

    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Pawn%d 카드 추가 - ID:%d (총 %d장)"),
        PawnIndex, CardID,
        Save->PartyDecks[PawnIndex].DeckCards.Num());
}

void UCardSaveGame::RemoveCard(int32 PawnIndex, int32 CardID)
{
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardSaveGame] RemoveCard: 잘못된 PawnIndex %d"), PawnIndex);
        return;
    }

    Save->PartyDecks[PawnIndex].DeckCards.Remove(CardID);
    WriteSave(Save);

    UE_LOG(LogTemp, Log, TEXT("[CardSaveGame] Pawn%d 카드 삭제 - ID:%d"),
        PawnIndex, CardID);
}

TArray<int32> UCardSaveGame::GetDeckCards(int32 PawnIndex)
{
    UCardSaveGame* Save = LoadSave();
    if (!Save || !Save->PartyDecks.IsValidIndex(PawnIndex))
        return {};

    return Save->PartyDecks[PawnIndex].DeckCards;
}
