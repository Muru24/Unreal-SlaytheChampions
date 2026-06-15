#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Card/CardDataTypes.h"
#include "CardSubsystem.generated.h"

/**
 * UCardSubsystem
 *
 * GameInstance Subsystem.
 * - DataTable 에서 카드 데이터를 로드/캐시한다.
 * - 직업별 덱 풀, 보상 카드 풀 조회를 제공한다.
 * - 게임 어디서든 GetGameInstance()->GetSubsystem 으로 접근 가능.
 *
 * 사용법 (C++):
 *   UCardSubsystem* CS = GetGameInstance()->GetSubsystem<UCardSubsystem>();
 *   const FCardDataRow* Row = CS->GetCard(FName("Warrior_Attack"));
 *
 * 사용법 (Blueprint):
 *   GetGameInstance -> GetSubsystem(CardSubsystem) -> GetCard / GetCardsByClass
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ── Subsystem 라이프사이클 ───────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ── 데이터 테이블 로드 ──────────────────────────────────────────────────
    /**
     * 런타임에 DataTable 을 교체하거나 다시 로드할 때 호출.
     * Initialize 에서 자동으로 호출되므로 일반적으로 직접 호출 불필요.
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Data")
    void LoadCardDataTable(UDataTable* InTable);

    // ── 카드 조회 ────────────────────────────────────────────────────────────

    /**
     * RowName 기반 단일 카드 데이터를 반환한다. 없으면 nullptr.
     * (const 포인터 반환은 UFUNCTION 불가 -> C++ 전용)
     */
    const FCardDataRow* GetCard(FName RowName) const;

    /** 직업에 해당하는 카드 RowName 목록 반환 (Any 직업 카드 포함). */
    UFUNCTION(BlueprintCallable, Category = "Card|Query")
    TArray<FName> GetCardNamesByClass(EJobClass JobClass) const;

    /**
     * 직업에 해당하는 FCardDataRow 포인터 목록 반환.
     * (const 포인터 배열은 UFUNCTION 불가 -> C++ 전용)
     */
    TArray<const FCardDataRow*> GetCardsByClass(EJobClass JobClass) const;

    /**
     * 보상 카드 풀: 직업 + 최소 희귀도 이상 카드 반환.
     * (희귀도 필터링 시 Normal 이하는 Any 로 처리)
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Reward")
    TArray<FName> GetRewardPool(EJobClass JobClass, ECardRarity MinRarity) const;

    /** 전체 카드 RowName 목록 반환 */
    UFUNCTION(BlueprintCallable, Category = "Card|Query")
    TArray<FName> GetAllCardNames() const;

    // ── 스타터 덱 ────────────────────────────────────────────────────────────

    /**
     * 직업별 스타터 덱 DataTable 등록.
     * BP GameInstance Init 또는 CharacterSelect 레벨 BeginPlay에서 직업마다 호출.
     * Table 행 구조: FStarterDeckRow (CardID = DT_Cards RowName).
     */
    UFUNCTION(BlueprintCallable, Category = "Card|StarterDeck")
    void RegisterStarterDeck(EJobClass JobClass, UDataTable* Table);

    /**
     * 직업의 스타터 덱 RowName 목록 반환.
     * RegisterStarterDeck으로 테이블이 등록돼 있으면 그 테이블 기준,
     * 없으면 DT_Cards 필터 결과(GetCardNamesByClass) 반환.
     */
    UFUNCTION(BlueprintCallable, Category = "Card|StarterDeck")
    TArray<FName> GetStarterDeckNames(EJobClass JobClass) const;

    /**
     * CardID 필드값으로 DataTable Row Name을 역조회한다.
     * Row Name == CardID인 경우 동일 값을 반환하므로 오버헤드 없음.
     * 없으면 NAME_None 반환.
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Query")
    FName GetRowNameByCardID(FName CardID) const;

private:
    // ── 내부 데이터 ──────────────────────────────────────────────────────────

    /** 에디터에서 할당: Content/Data/DT_Cards */
    UPROPERTY()
    TObjectPtr<UDataTable> CardDataTable;

    /** CardID 필드값 → Row Name 역조회 캐시 (LoadCardDataTable 시 구축) */
    TMap<FName, FName> CardIDToRowName;

    /** 직업별 스타터 덱 DataTable (RegisterStarterDeck으로 등록) */
    UPROPERTY()
    TMap<EJobClass, TObjectPtr<UDataTable>> StarterDeckTables;

    /** Rarity 수치화 헬퍼 (보상 풀 필터링용) */
    static int32 RarityToInt(ECardRarity Rarity);
};
