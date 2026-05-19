#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CardDataTypes.h"
#include "CardSaveGame.generated.h"

/**
 * FPlayerDeckSaveData
 *
 * 파티원 1명의 덱 데이터를 저장하는 구조체.
 * UCardSaveGame 안에 파티원 수만큼 배열로 저장된다.
 * Index 0 = Pawn1, Index 1 = Pawn2
 */
USTRUCT(BlueprintType)
struct FPlayerDeckSaveData
{
    GENERATED_BODY()

    // 덱 소유 직업 분류 (Warrior, Mage, Healer 등)
    UPROPERTY(BlueprintReadWrite)
    EJobClass JobClass = EJobClass::Warrior;

    /**
     * 카드 ID 배열.
     * DT_Cards 의 _CardID 값 (예: 101, 201)
     * 전투 종료 시 DrawPile + Hand + DiscardPile 을 합쳐서 저장.
     * 전투 시작 시 이 배열을 꺼내서 DrawPile 에 적재.
     */
    UPROPERTY(BlueprintReadWrite)
    TArray<int32> DeckCards;
};

/**
 * UCardSaveGame
 *
 * 파티원의 덱 데이터를 파일로 저장/불러오기 위한 클래스.
 *
 * [사용 흐름]
 * 1. 게임 시작 시 LoadOrCreate() 호출
 *    -> SaveGame 파일이 있으면 불러옴
 *    -> 없으면 DT_StarterDeck 에서 읽어 초기 SaveGame 생성
 *
 * 2. 전투 종료 시 SaveDeckAfterBattle() 호출
 *    -> DrawPile + Hand + DiscardPile 합쳐서 저장 (A방식)
 *
 * 3. 카드 획득 시 AddCard() 호출
 *    -> 카드 추가 후 즉시 저장
 *
 * [슬롯 정보]
 * SlotName : "PlayerDeckSave"
 * UserIndex : 0
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // 파티원별 덱 데이터 배열 (Index 0 = Pawn1, Index 1 = Pawn2)
    UPROPERTY(BlueprintReadWrite, Category = "Save")
    TArray<FPlayerDeckSaveData> PartyDecks;

    // 저장 슬롯 이름 (고정값)
    static const FString SlotName;

    // 저장 슬롯 유저 인덱스 (싱글플레이어 기준 0 고정)
    static const int32 UserIndex;

    // ──────────────────────────────────────────────────────────────────────
    // 정적 유틸 함수
    // 어디서든 UCardSaveGame:: 으로 바로 호출 가능
    // ──────────────────────────────────────────────────────────────────────

    /**
     * SaveGame 파일을 디스크에서 불러온다.
     * 파일이 없으면 nullptr 반환.
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static UCardSaveGame* LoadSave();

    /**
     * SaveGame 객체를 디스크에 저장한다.
     * nullptr 이면 무시.
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void WriteSave(UCardSaveGame* SaveGame);

    /**
     * SaveGame 파일이 있으면 불러오고, 없으면 DT에서 초기 덱을 생성한다.
     * 게임 시작 시 1회 호출.
     *
     * @param StarterDeckWarrior  DT_StarterDeck_Warrior (Pawn1 초기 덱)
     * @param StarterDeckMage     DT_StarterDeck_Mage    (Pawn2 초기 덱)
     * @return 로드/생성된 UCardSaveGame 객체
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static UCardSaveGame* LoadOrCreate(UDataTable* StarterDeckWarrior, UDataTable* StarterDeckMage);

    /**
     * 전투 종료 시 덱 상태를 저장한다. (A방식)
     * DrawPile + Hand + DiscardPile 을 합쳐서 DeckCards 에 저장.
     * APartyMemberPawn::SaveDeckToSaveGame() 에서 호출.
     *
     * @param PawnIndex    파티원 인덱스 (0 = Pawn1, 1 = Pawn2)
     * @param DrawPile     현재 남은 드로우 더미
     * @param Hand         현재 손패
     * @param DiscardPile  현재 버린 카드 더미
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void SaveDeckAfterBattle(int32 PawnIndex,
        const TArray<int32>& DrawPile,
        const TArray<int32>& Hand,
        const TArray<int32>& DiscardPile);

    /**
     * 보상으로 카드를 덱에 추가하고 즉시 저장한다.
     *
     * @param PawnIndex  파티원 인덱스
     * @param CardID     추가할 카드 ID (DT_Cards _CardID)
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void AddCard(int32 PawnIndex, int32 CardID);

    /**
     * 특정 카드를 덱에서 삭제하고 즉시 저장한다.
     *
     * @param PawnIndex  파티원 인덱스
     * @param CardID     삭제할 카드 ID
     */
    UFUNCTION(BlueprintCallable, Category = "Save")
    static void RemoveCard(int32 PawnIndex, int32 CardID);

    /**
     * 파티원의 현재 덱 카드 ID 목록을 반환한다.
     *
     * @param PawnIndex  파티원 인덱스
     * @return 카드 ID 배열
     */
    UFUNCTION(BlueprintPure, Category = "Save")
    static TArray<int32> GetDeckCards(int32 PawnIndex);
};
