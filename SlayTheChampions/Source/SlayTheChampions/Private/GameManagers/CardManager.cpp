#include "GameManagers/CardManager.h"
#include "Card/DeckComponent.h"
#include "Card/CardSubsystem.h"
#include "Card/CardSaveGame.h"
#include "Party/PartyInstance.h"

void UCardManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // ?뚰떚 理쒕? 2紐??щ’ ?덉빟
    PartyDeckComponents.SetNum(2);

    // CardSubsystem 罹먯떆
    CardSubsystem = GetGameInstance()->GetSubsystem<UCardSubsystem>();

    UE_LOG(LogTemp, Log, TEXT("[CardManager] Initialized."));
}

// ?? ?뚰떚 ???깅줉 ?????????????????????????????????????????????????????????????

void UCardManager::RegisterDeckComponent(int32 PawnIndex, UDeckComponent* InDeckComp)
{
    if (!IsValidPawnIndex(PawnIndex) || !InDeckComp)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardManager] RegisterDeckComponent: Invalid PawnIndex(%d) or null DeckComponent."),
            PawnIndex);
        return;
    }

    PartyDeckComponents[PawnIndex] = InDeckComp;

    UE_LOG(LogTemp, Log,
        TEXT("[CardManager] Pawn%d DeckComponent registered."), PawnIndex);
}

// ?? 珥덇린?????????????????????????????????????????????????????????????????????

void UCardManager::InitializePartyDecks()
{
    // ?뺤젙??寃쎈줈?먯꽌 StarterDeck DataTable 濡쒕뱶
    static const FSoftObjectPath WarriorPath(
        TEXT("/Game/01_Card/01_Test_DT/DT_StarterDeck_Warrior.DT_StarterDeck_Warrior"));
    static const FSoftObjectPath MagePath(
        TEXT("/Game/01_Card/01_Test_DT/DT_StarterDeck_Mage.DT_StarterDeck_Mage"));

    UDataTable* WarriorTable = Cast<UDataTable>(WarriorPath.TryLoad());
    UDataTable* MageTable    = Cast<UDataTable>(MagePath.TryLoad());

    if (!WarriorTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardManager] DT_StarterDeck_Warrior not found at '%s'."),
            *WarriorPath.ToString());
    }
    if (!MageTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardManager] DT_StarterDeck_Mage not found at '%s'."),
            *MagePath.ToString());
    }

    // SaveGame ?덉쑝硫?濡쒕뱶, ?놁쑝硫?StarterDeck ?쇰줈 ?덈줈 ?앹꽦
    UCardSaveGame* Save = UCardSaveGame::LoadOrCreate(WarriorTable, MageTable);
    if (!Save)
    {
        UE_LOG(LogTemp, Error, TEXT("[CardManager] InitializePartyDecks: Failed to load or create SaveGame."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[CardManager] Party decks initialized."));
}

// ?? ????????????????????????????????????????????????????????????????????????

void UCardManager::SaveAllDecks()
{
    UCardSaveGame* Save = UCardSaveGame::LoadSave();
    if (!Save)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CardManager] SaveAllDecks: No SaveGame found. Call InitializePartyDecks first."));
        return;
    }

    for (int32 i = 0; i < PartyDeckComponents.Num(); ++i)
    {
        UDeckComponent* DC = PartyDeckComponents[i];
        if (!DC) continue;

        // DrawPile + DiscardPile ?⑹궛 ???(Hand???꾪닾 醫낅즺 ???대? DiscardPile濡??대룞???곹깭)
        TArray<FName> Combined;
        Combined.Append(DC->GetDrawPile());
        Combined.Append(DC->GetDiscardPile());

        if (Save->PartyDecks.IsValidIndex(i))
        {
            Save->PartyDecks[i].DeckCards = Combined;
            UE_LOG(LogTemp, Log,
                TEXT("[CardManager] Pawn%d deck saved - %d cards."), i, Combined.Num());
        }
    }

    UCardSaveGame::WriteSave(Save);
    UE_LOG(LogTemp, Log, TEXT("[CardManager] SaveAllDecks completed."));
}

// ?? 蹂댁긽 移대뱶 ?????????????????????????????????????????????????????????????????

void UCardManager::AddRewardCard(int32 PawnIndex, FName CardName)
{
    if (!IsValidPawnIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardManager] AddRewardCard: Invalid PawnIndex(%d)."), PawnIndex);
        return;
    }

    UDeckComponent* DC = PartyDeckComponents[PawnIndex];
    if (!DC)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardManager] AddRewardCard: Pawn%d DeckComponent not registered."), PawnIndex);
        return;
    }

    // 硫붾え由?DeckComponent) DiscardPile??異붽? (?ㅼ쓬 RecycleDiscardIntoDraw ??DrawPile濡??⑸쪟)
    DC->DiscardCard(CardName);

    // SaveGame ??諛섏쁺
    UCardSaveGame::AddCard(PawnIndex, CardName);

    UE_LOG(LogTemp, Log,
        TEXT("[CardManager] Pawn%d reward card added - %s."), PawnIndex, *CardName.ToString());
}

// ?? 議고쉶 ?????????????????????????????????????????????????????????????????????

void UCardManager::AddCardToPartyDeck(int32 PawnIndex, FName CardName)
{
    if (!IsValidPawnIndex(PawnIndex) || CardName.IsNone())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardManager] AddCardToPartyDeck: Invalid PawnIndex(%d) or CardName."), PawnIndex);
        return;
    }

    if (UDeckComponent* DC = PartyDeckComponents[PawnIndex])
    {
        DC->DiscardCard(CardName);
    }

    UCardSaveGame::AddCard(PawnIndex, CardName);

    UE_LOG(LogTemp, Log,
        TEXT("[CardManager] Pawn%d card added to party deck - %s."), PawnIndex, *CardName.ToString());
}

bool UCardManager::TryPurchaseShopCardForParty(FName CardName, int32 Price, int32& OutPawnIndex)
{
    OutPawnIndex = INDEX_NONE;

    if (CardName.IsNone() || Price < 0)
    {
        return false;
    }

    const int32 PawnIndex = FindPartyDeckIndexForCard(CardName);
    if (!IsValidPawnIndex(PawnIndex))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardManager] TryPurchaseShopCardForParty: No matching party deck for %s."), *CardName.ToString());
        return false;
    }

    UPartyInstance* PartyInstance = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPartyInstance>() : nullptr;
    if (!PartyInstance || PartyInstance->GetGold() < Price)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardManager] TryPurchaseShopCardForParty: Not enough gold for %s. Price=%d"),
            *CardName.ToString(), Price);
        return false;
    }

    PartyInstance->UseGold(Price);
    AddCardToPartyDeck(PawnIndex, CardName);
    OutPawnIndex = PawnIndex;
    return true;
}

UDeckComponent* UCardManager::GetDeckComponent(int32 PawnIndex) const
{
    if (!IsValidPawnIndex(PawnIndex)) return nullptr;
    return PartyDeckComponents[PawnIndex];
}

const FCardDataRow* UCardManager::GetCardData(FName CardName) const
{
    if (!CardSubsystem) return nullptr;
    return CardSubsystem->GetCard(CardName);
}

// ?? Private ??????????????????????????????????????????????????????????????????

bool UCardManager::IsValidPawnIndex(int32 PawnIndex) const
{
    return PartyDeckComponents.IsValidIndex(PawnIndex);
}

int32 UCardManager::FindPartyDeckIndexForCard(FName CardName) const
{
    const FCardDataRow* CardData = GetCardData(CardName);
    if (!CardData)
    {
        return INDEX_NONE;
    }

    UCardSaveGame* Save = UCardSaveGame::LoadSave();
    if (!Save)
    {
        return CardData->RequiredClass == EJobClass::Any && IsValidPawnIndex(0) ? 0 : INDEX_NONE;
    }

    if (CardData->RequiredClass == EJobClass::Any)
    {
        return Save->PartyDecks.IsValidIndex(0) ? 0 : INDEX_NONE;
    }

    for (int32 Index = 0; Index < Save->PartyDecks.Num(); ++Index)
    {
        if (Save->PartyDecks[Index].JobClass == CardData->RequiredClass && IsValidPawnIndex(Index))
        {
            return Index;
        }
    }

    return INDEX_NONE;
}

