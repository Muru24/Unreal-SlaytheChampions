#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CardDataTypes.h"
#include "PartyMemberPawn.generated.h"

class UHandComponent;

/**
 * APartyMemberPawn
 *
 * 파티원 1명을 나타내는 Pawn 베이스 클래스.
 * HandComponent 를 소유하며 덱/드로우/버린패 상태를 관리한다.
 * BP_Pawn1, BP_Pawn2 의 부모 클래스로 사용.
 *
 * [덱 초기화 우선순위]
 * 1. SaveGame 파일에 저장된 덱 (전투 종료 후 저장된 덱)
 * 2. OverrideDeckIDs (에디터/테스트에서 직접 지정)
 * 3. CardSubsystem 에서 직업 기반 자동 조회
 */
UCLASS()
class SLAYTHECHAMPIONS_API APartyMemberPawn : public APawn
{
    GENERATED_BODY()

public:
    APartyMemberPawn();

protected:
    virtual void BeginPlay() override;

public:
    // 덱/드로우/버린패 상태 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Card")
    TObjectPtr<UHandComponent> HandComponent;

    /**
     * 파티 내 인덱스.
     * SaveGame 에서 덱을 불러올 때 사용.
     * 0 = Pawn1 (Warrior), 1 = Pawn2 (Mage)
     * 에디터에서 각 Pawn 에 맞게 설정 필요.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    int32 PawnIndex = 0;

    // 이 파티원의 직업 (카드 조회 및 덱 초기화에 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    EJobClass JobClass = EJobClass::Warrior;

    /**
     * 직접 지정할 덱 ID 목록. (테스트용)
     * 비어있으면 SaveGame 또는 CardSubsystem 에서 불러옴.
     * 실제 게임에서는 비워둘 것.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    TArray<int32> OverrideDeckIDs;

    // ──────────────────────────────────────────────────────────────────────
    // 덱 관리 함수
    // ──────────────────────────────────────────────────────────────────────

    /**
     * 덱을 초기화하고 HandComponent 에 전달한다.
     * BeginPlay 에서 자동 호출.
     * SaveGame -> OverrideDeck -> CardSubsystem 순서로 우선순위 적용.
     */
    UFUNCTION(BlueprintCallable, Category = "Card")
    void InitializeDeck();

    /**
     * 전투 종료 시 덱 현재 상태를 SaveGame 에 저장한다.
     * DrawPile + Hand + DiscardPile 을 합쳐서 저장 (A방식).
     * 전투 종료 이벤트에서 호출할 것.
     */
    UFUNCTION(BlueprintCallable, Category = "Card")
    void SaveDeckToSaveGame();

    // ──────────────────────────────────────────────────────────────────────
    // 덱 및 카드 조작 함수
    // ──────────────────────────────────────────────────────────────────────

    // 턴 시작 시 카드 드로우 (기본 5장)
    UFUNCTION(BlueprintCallable, Category = "Card")
    void DrawStartOfTurn();

    /**
     * 손패에서 카드를 사용한다.
     * Hand 에서 제거 후 DiscardPile 로 이동.
     * @return 성공 여부 (손패에 없으면 false)
     */
    UFUNCTION(BlueprintCallable, Category = "Card")
    bool PlayCard(int32 CardID);

    // 턴 종료 시 손패 전체를 DiscardPile 로 버린다.
    UFUNCTION(BlueprintCallable, Category = "Card")
    void DiscardHand();

    // ──────────────────────────────────────────────────────────────────────
    // 조회 함수
    // ──────────────────────────────────────────────────────────────────────

    // 현재 손패 카드 ID 목록 반환
    UFUNCTION(BlueprintPure, Category = "Card")
    TArray<int32> GetHand() const;

    // 드로우 더미 남은 장수
    UFUNCTION(BlueprintPure, Category = "Card")
    int32 GetDrawPileCount() const;

    // 버린 카드 더미 장수
    UFUNCTION(BlueprintPure, Category = "Card")
    int32 GetDiscardPileCount() const;
};
