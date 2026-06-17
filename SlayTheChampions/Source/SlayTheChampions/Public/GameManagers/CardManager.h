#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Card/CardDataTypes.h"
#include "CardManager.generated.h"

class UDeckComponent;
class UCardSubsystem;

/**
 * UCardManager
 *
 * 카드 덱 관련 모든 흐름의 단일 창구 (GameInstanceSubsystem).
 * 레벨/전투와 무관하게 게임 전체 수명 동안 유지된다.
 *
 * 역할:
 *  - 게임 시작 시 파티 덱 초기화 (SaveGame 또는 StarterDeck)
 *  - 전투 종료 시 전체 파티 덱 저장
 *  - 보상 카드 추가 및 저장
 *  - 외부에서 DeckComponent 접근 창구 제공
 *  - 카드 데이터 조회 (CardSubsystem 위임)
 *
 * 사용법 (C++):
 *   UCardManager* CM = GetGameInstance()->GetSubsystem<UCardManager>();
 *
 * 사용법 (Blueprint):
 *   GetGameInstance -> GetSubsystem(CardManager)
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ── 파티 덱 등록 ─────────────────────────────────────────────────────────

    /**
     * Pawn의 DeckComponent를 CardManager에 등록한다.
     * UCardUserComponent::BeginPlay 에서 호출할 것.
     * @param PawnIndex     파티 내 인덱스 (0 = Pawn1, 1 = Pawn2)
     * @param InDeckComp    등록할 DeckComponent
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Manager")
    void RegisterDeckComponent(int32 PawnIndex, UDeckComponent* InDeckComp);

    // ── 초기화 ───────────────────────────────────────────────────────────────

    /**
     * 게임 시작 시 1회 호출. 파티 전체 덱을 초기화한다.
     * SaveGame 있으면 로드, 없으면 확정된 경로의 StarterDeck DataTable 로 새로 생성.
     * 경로: /Game/01_Card/01_Test_DT/DT_StarterDeck_Warrior, DT_StarterDeck_Mage
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Manager")
    void InitializePartyDecks();

    // ── 저장 ─────────────────────────────────────────────────────────────────

    /**
     * 파티 전체 덱을 SaveGame에 저장한다.
     * 전투 종료 시 외부(레벨 전환 등)에서 호출할 것.
     * 함수 구현만 제공 — 호출 타이밍은 외부에서 결정.
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Manager")
    void SaveAllDecks();

    // ── 보상 카드 ─────────────────────────────────────────────────────────────

    /**
     * 보상 카드를 파티원의 덱에 추가하고 SaveGame에 저장한다.
     * 함수 구현만 제공 — 호출 타이밍은 외부에서 결정.
     * @param PawnIndex  파티 인덱스 (0 = Pawn1, 1 = Pawn2)
     * @param CardName   추가할 카드 ID (DT_Cards RowName)
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Manager")
    void AddRewardCard(int32 PawnIndex, FName CardName);

    UFUNCTION(BlueprintCallable, Category = "Card|Manager")
    void AddCardToPartyDeck(int32 PawnIndex, FName CardName);

    UFUNCTION(BlueprintCallable, Category = "Card|Shop")
    bool TryPurchaseShopCardForParty(FName CardName, int32 Price, int32& OutPawnIndex);

    // ── 조회 ─────────────────────────────────────────────────────────────────

    /**
     * 파티원의 DeckComponent를 반환한다.
     * 등록되지 않았거나 유효하지 않으면 nullptr 반환.
     * @param PawnIndex  파티 인덱스
     */
    UFUNCTION(BlueprintPure, Category = "Card|Manager")
    UDeckComponent* GetDeckComponent(int32 PawnIndex) const;

    /**
     * 카드 ID로 카드 데이터를 조회한다. (CardSubsystem 위임)
     * 없으면 nullptr 반환.
     * @param CardName  카드 ID (DT_Cards RowName)
     */
    const FCardDataRow* GetCardData(FName CardName) const;

private:
    // 파티원 DeckComponent 배열 (Index = PawnIndex)
    UPROPERTY()
    TArray<TObjectPtr<UDeckComponent>> PartyDeckComponents;

    // CardSubsystem 캐시 참조
    UPROPERTY()
    TObjectPtr<UCardSubsystem> CardSubsystem;

    // PawnIndex 유효성 검사
    bool IsValidPawnIndex(int32 PawnIndex) const;
    int32 FindPartyDeckIndexForCard(FName CardName) const;
};
