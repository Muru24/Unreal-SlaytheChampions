#include "HandComponent.h"
#include "Algo/RandomShuffle.h"

UHandComponent::UHandComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// ── 초기화 ────────────────────────────────────────────────────────────────

void UHandComponent::InitializeDeck(const TArray<int32>& InDeckIDs)
{
    DrawPile = InDeckIDs;
    Hand.Reset();
    DiscardPile.Reset();

    // 초기 셔플
    Algo::RandomShuffle(DrawPile);

    BroadcastHandChanged();
}

// ── 드로우 ────────────────────────────────────────────────────────────────

void UHandComponent::DrawStartOfTurn()
{
    DrawCards(DefaultDrawCount);
}

TArray<int32> UHandComponent::DrawCards(int32 Count)
{
    TArray<int32> Drawn;

    for (int32 i = 0; i < Count; ++i)
    {
        // DrawPile 이 비면 DiscardPile 재순환
        if (DrawPile.IsEmpty())
        {
            if (DiscardPile.IsEmpty()) break;  // 둘 다 비어있으면 드로우 불가
            RecycleDiscardIntoDraw();
        }

        int32 Card = DrawPile.Pop();
        Hand.Add(Card);
        Drawn.Add(Card);

        OnCardDrawn.Broadcast(Card);
    }

    if (!Drawn.IsEmpty())
    {
        BroadcastHandChanged();
    }

    return Drawn;
}

// ── 카드 사용 ─────────────────────────────────────────────────────────────

bool UHandComponent::PlayCard(int32 CardID)
{
    const int32 Idx = Hand.IndexOfByKey(CardID);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UHandComponent::PlayCard - CardID %d not found in hand."),
            CardID);
        return false;
    }

    Hand.RemoveAt(Idx);
    DiscardPile.Add(CardID);

    OnCardPlayed.Broadcast(CardID);
    BroadcastHandChanged();
    return true;
}

// ── 턴 종료 ───────────────────────────────────────────────────────────────

void UHandComponent::DiscardHand()
{
    DiscardPile.Append(Hand);
    Hand.Reset();
    BroadcastHandChanged();
}

// ── Private ───────────────────────────────────────────────────────────────

void UHandComponent::RecycleDiscardIntoDraw()
{
    DrawPile = MoveTemp(DiscardPile);
    DiscardPile.Reset();
    Algo::RandomShuffle(DrawPile);

    UE_LOG(LogTemp, Log,
        TEXT("UHandComponent: DiscardPile 재순환 → DrawPile (%d장, 셔플 완료)."),
        DrawPile.Num());
}

void UHandComponent::BroadcastHandChanged()
{
    OnHandChanged.Broadcast(Hand);
}
