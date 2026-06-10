#include "Card/CardUserComponent.h"
#include "Card/DeckComponent.h"
#include "Card/CardSubsystem.h"
#include "Card/CardSaveGame.h"
#include "Card/CardDataTypes.h"
#include "GameManagers/CardManager.h"
#include "Engine/GameInstance.h"
#include "CombatKernel/CombatManager.h"
#include "Kismet/GameplayStatics.h"

UCardUserComponent::UCardUserComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// ── 라이프사이클 ──────────────────────────────────────────────────────────────

void UCardUserComponent::BeginPlay()
{
    Super::BeginPlay();

    // Owner 에 DeckComponent 를 동적으로 생성하고 컴포넌트로 등록
    DeckComponent = NewObject<UDeckComponent>(GetOwner(), UDeckComponent::StaticClass(), TEXT("DeckComponent"));
    DeckComponent->RegisterComponent();

    // CardManager 에 이 폰의 DeckComponent 를 등록 (외부 접근 창구 역할)
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UCardManager* CM = GI->GetSubsystem<UCardManager>())
        {
            CM->RegisterDeckComponent(PawnIndex, DeckComponent);
        }
    }

    // 덱 초기화 (SaveGame → OverrideDeckNames → CardSubsystem 순)
    InitializeDeck();
}

// ── 덱 초기화 / 저장 ──────────────────────────────────────────────────────────

void UCardUserComponent::InitializeDeck()
{
    if (!DeckComponent) return;

    TArray<FName> DeckNames;

    // 1순위: SaveGame 슬롯에서 이전 전투 덱 불러오기
    TArray<FName> SavedDeck = UCardSaveGame::GetDeckCards(PawnIndex);
    if (SavedDeck.Num() > 0)
    {
        DeckNames = SavedDeck;
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] Pawn%d - Loaded from SaveGame (%d cards)."),
            PawnIndex, DeckNames.Num());
    }

    // 2순위: 에디터에서 직접 지정한 OverrideDeckNames (테스트용)
    if (DeckNames.IsEmpty() && OverrideDeckNames.Num() > 0)
    {
        DeckNames = OverrideDeckNames;
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] Pawn%d - Using OverrideDeck (%d cards)."),
            PawnIndex, DeckNames.Num());
    }

    // 3순위: CardSubsystem 에서 직업(JobClass) 기반으로 자동 조회
    if (DeckNames.IsEmpty())
    {
        if (UGameInstance* GI = GetWorld()->GetGameInstance())
        {
            if (UCardSubsystem* CS = GI->GetSubsystem<UCardSubsystem>())
            {
                DeckNames = CS->GetCardNamesByClass(JobClass);
                UE_LOG(LogTemp, Warning,
                    TEXT("[CardUserComponent] Pawn%d - Loaded from CardSubsystem (%d cards)."),
                    PawnIndex, DeckNames.Num());
            }
        }
    }

    // 세 경로 모두 실패 시 덱 초기화 불가
    if (DeckNames.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] Pawn%d - No cards found. Deck not initialized."),
            PawnIndex);
        return;
    }

    // DeckComponent 초기화 (Fisher-Yates 셔플 포함), Hand 는 빈 상태로 시작
    DeckComponent->InitializeDeck(DeckNames, Hand);
    UE_LOG(LogTemp, Warning,
        TEXT("[CardUserComponent] Pawn%d - Deck initialized (%d cards)."),
        PawnIndex, DeckNames.Num());
}

void UCardUserComponent::SaveDeckToSaveGame()
{
    if (!DeckComponent) return;

    // DrawPile + Hand + DiscardPile 합산 저장 (ExhaustPile 은 전투 내 임시 소멸이므로 제외)
    UCardSaveGame::SaveDeckAfterBattle(
        PawnIndex,
        DeckComponent->GetDrawPile(),
        Hand,
        DeckComponent->GetDiscardPile()
    );

    UE_LOG(LogTemp, Log,
        TEXT("[CardUserComponent] Pawn%d - Deck saved to SaveGame."), PawnIndex);
}

// ── 턴 조작 ──────────────────────────────────────────────────────────────────

void UCardUserComponent::DrawStartOfTurn()
{
    // 턴 시작 시 기본 장 수(DefaultDrawCount) 만큼 드로우
    DrawCards(DefaultDrawCount);
}

void UCardUserComponent::DrawCards(int32 Count)
{
    if (!DeckComponent) return;

    // DeckComponent 에서 Count 장 드로우 후 Hand 에 추가
    TArray<FName> Drawn = DeckComponent->DrawCards(Count, Hand);

    // 한 장이라도 드로우됐으면 UI 갱신용 델리게이트 브로드캐스트
    if (!Drawn.IsEmpty())
    {
        BroadcastHandChanged();
    }
}

bool UCardUserComponent::PlayCard(FName CardName)
{
    // Hand 에서 해당 카드 위치 탐색
    const int32 Idx = Hand.IndexOfByKey(CardName);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] PlayCard - '%s' not found in hand."),
            *CardName.ToString());
        return false;
    }

    // CardSubsystem 에서 카드 데이터 조회 (효과 실행에 필요)
    const FCardDataRow* Row = nullptr;
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UCardSubsystem* CS = GI->GetSubsystem<UCardSubsystem>())
            Row = CS->GetCard(CardName);
    }

    if (!Row)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] PlayCard - '%s' card data not found in CardSubsystem."),
            *CardName.ToString());
        return false;
    }

    // Hand 에서 제거
    Hand.RemoveAt(Idx);

    if (DeckComponent)
    {
        // bExhaust 카드면 ExhaustPile(소멸), 일반 카드면 DiscardPile(버리기)
        if (Row->bExhaust)
            DeckComponent->ExhaustCard(CardName);
        else
            DeckComponent->DiscardCard(CardName);
    }

    // CombatManager 의 ActionQueue(히스토리)에 카드 등록 — 효과 실행은 포함되지 않음
    // (실제 효과 실행은 BattleMainWidget::QueueCardAction → ExecuteCard 에서 즉시 처리)
    ACombatManager* CombatMgr = Cast<ACombatManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ACombatManager::StaticClass()));
    if (CombatMgr)
    {
        CombatMgr->QueuePlayerAction(*Row, PawnIndex);
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] Pawn%d - '%s' queued to CombatManager."),
            PawnIndex, *CardName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] PlayCard - CombatManager not found in world."));
    }

    // 카드 사용 이벤트 및 Hand 변경 이벤트 브로드캐스트
    OnCardPlayed.Broadcast(CardName);
    BroadcastHandChanged();
    return true;
}

void UCardUserComponent::DiscardHand()
{
    if (!DeckComponent) return;

    // Hand 전체를 DiscardPile 로 이동 (턴 종료 시 호출)
    DeckComponent->DiscardAll(Hand);

    BroadcastHandChanged();
}

bool UCardUserComponent::RemoveFromHand(FName CardName)
{
    // Hand 에서 카드 위치 탐색
    const int32 Idx = Hand.IndexOfByKey(CardName);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[CardUserComponent] RemoveFromHand - '%s' not found in hand."),
            *CardName.ToString());
        return false;
    }

    // DiscardPile 이동 없이 Hand 에서만 제거
    Hand.RemoveAt(Idx);
    BroadcastHandChanged();
    return true;
}

void UCardUserComponent::AddToHand(FName CardName)
{
    // 큐 취소 시 카드를 손패로 되돌림 — DiscardPile/DrawPile 조작 없이 Hand 에만 추가
    Hand.Add(CardName);
    BroadcastHandChanged();
}

void UCardUserComponent::DiscardSpecificCard(FName CardName)
{
    if (!DeckComponent) return;

    // bExhaust 카드면 ExhaustPile, 일반 카드면 DiscardPile 로 이동
    if (IsExhaustCard(CardName))
        DeckComponent->ExhaustCard(CardName);
    else
        DeckComponent->DiscardCard(CardName);

    // 카드 사용 완료 이벤트 브로드캐스트
    OnCardPlayed.Broadcast(CardName);
}

// ── 조회 ─────────────────────────────────────────────────────────────────────

int32 UCardUserComponent::GetDrawPileCount() const
{
    return DeckComponent ? DeckComponent->GetDrawPileCount() : 0;
}

int32 UCardUserComponent::GetDiscardPileCount() const
{
    return DeckComponent ? DeckComponent->GetDiscardPileCount() : 0;
}

int32 UCardUserComponent::GetExhaustPileCount() const
{
    return DeckComponent ? DeckComponent->GetExhaustPileCount() : 0;
}

// ── Private ──────────────────────────────────────────────────────────────────

void UCardUserComponent::BroadcastHandChanged()
{
    // Hand 배열 전체를 UI 등 외부에 전달
    OnHandChanged.Broadcast(Hand);
}

bool UCardUserComponent::IsExhaustCard(FName CardName) const
{
    // CardSubsystem 에서 카드 데이터를 조회해 bExhaust 플래그 반환
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (UCardSubsystem* CS = GI->GetSubsystem<UCardSubsystem>())
        {
            const FCardDataRow* Row = CS->GetCard(CardName);
            return Row && Row->bExhaust;
        }
    }
    return false;
}
