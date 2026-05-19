#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CardDataTypes.h"
#include "HandComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHandChanged, const TArray<int32>&, CurrentHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardDrawn, int32, DrawnCard);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardPlayed, int32, PlayedCard);

/**
 * UHandComponent
 *
 * 파티원 1명의 덱/드로우/버린패 상태를 관리하는 컴포넌트.
 * 특정 Actor 에 종속되지 않도록 ActorComponent 로 설계.
 * Enemy 에도 붙여서 재사용 가능.
 *
 * 책임:
 *  - 덱 초기화 및 관리
 *  - 드로우 (DrawPile 소진 시 DiscardPile 자동 재순환)
 *  - 카드 사용 -> 서브시스템에서 효과 -> DiscardPile 이동
 *  - 턴 종료 시 손패 전체 버리기
 *
 * 카드 효과 처리(데미지 계산 등)는 이 컴포넌트 밖에서 처리한다.
 */
UCLASS(ClassGroup = "Card", meta = (BlueprintSpawnableComponent))
class SLAYTHECHAMPIONS_API UHandComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHandComponent();

    // ── 이벤트 ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Card|Hand")
    FOnHandChanged OnHandChanged;

    UPROPERTY(BlueprintAssignable, Category = "Card|Hand")
    FOnCardDrawn OnCardDrawn;

    UPROPERTY(BlueprintAssignable, Category = "Card|Hand")
    FOnCardPlayed OnCardPlayed;

    // ── 설정 값 ───────────────────────────────────────────────────────────
    // 턴 시작 기본 드로우 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Hand",
        meta = (ClampMin = "1"))
    int32 DefaultDrawCount = 5;

    // ── 초기화 ────────────────────────────────────────────────────────────

    // 덱을 전달하고 손패 및 더미 초기 설정
    UFUNCTION(BlueprintCallable, Category = "Card|Hand")
    void InitializeDeck(const TArray<int32>& InDeckIDs);

    // ── 드로우 ────────────────────────────────────────────────────────────

    // DefaultDrawCount 만큼 드로우
    UFUNCTION(BlueprintCallable, Category = "Card|Hand")
    void DrawStartOfTurn();

    // N장 드로우. DrawPile 소진 시 DiscardPile 재순환
    UFUNCTION(BlueprintCallable, Category = "Card|Hand")
    TArray<int32> DrawCards(int32 Count);

    // ── 카드 사용 ─────────────────────────────────────────────────────────

    // 손패에서 카드를 사용 (DiscardPile 이동). 손패에 없으면 false 반환
    UFUNCTION(BlueprintCallable, Category = "Card|Hand")
    bool PlayCard(int32 CardID);

    // ── 턴 종료 ───────────────────────────────────────────────────────────

    // 손패 전체를 DiscardPile 로 이동
    UFUNCTION(BlueprintCallable, Category = "Card|Hand")
    void DiscardHand();

    // ── 조회 ──────────────────────────────────────────────────────────────

    // 현재 손패 반환
    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    const TArray<int32>& GetHand() const { return Hand; }

    // DrawPile 전체 반환 (SaveGame 연동용)
    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    const TArray<int32>& GetDrawPile() const { return DrawPile; }

    // DiscardPile 전체 반환 (SaveGame 연동용)
    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    const TArray<int32>& GetDiscardPile() const { return DiscardPile; }

    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    int32 GetDrawPileCount() const { return DrawPile.Num(); }

    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    int32 GetDiscardPileCount() const { return DiscardPile.Num(); }

    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    int32 GetHandCount() const { return Hand.Num(); }

    // 손패에 해당 카드가 있는지 확인
    UFUNCTION(BlueprintPure, Category = "Card|Hand")
    bool HasCardInHand(int32 CardID) const { return Hand.Contains(CardID); }

private:
    // ── 내부 더미 ─────────────────────────────────────────────────────────
    TArray<int32> DrawPile;
    TArray<int32> Hand;
    TArray<int32> DiscardPile;

    // DiscardPile -> DrawPile 재순환 + 셔플
    void RecycleDiscardIntoDraw();

    void BroadcastHandChanged();
};
