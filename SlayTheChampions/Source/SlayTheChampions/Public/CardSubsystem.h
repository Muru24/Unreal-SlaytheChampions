#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CardDataTypes.h"
#include "CardSubsystem.generated.h"

/**
 * UCardSubsystem
 *
 * GameInstance Subsystem.
 * - DataTable 에서 카드 데이터를 로드·캐싱한다.
 * - 직업별 덱 풀, 보상 카드 풀 조회를 담당한다.
 * - 게임 어디서든 GEngine->GetEngineSubsystem 없이 바로 접근.
 *
 * 사용 예 (C++):
 *   UCardSubsystem* CS = GetGameInstance()->GetSubsystem<UCardSubsystem>();
 *   const FCardDataRow* Row = CS->GetCard(101);
 *
 * 사용 예 (Blueprint):
 *   GetGameInstance → GetSubsystem(CardSubsystem) -> GetCard / GetCardIDsByClass
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ── Subsystem Lifecycle ────────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ── 데이터 테이블 로드 ─────────────────────────────────────────────────
    /**
     * 런타임에 DataTable 을 교체하거나 다시 로드할 때 호출.
     * Initialize 에서 자동으로 호출되므로 별도 호출은 불필요.
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Data")
    void LoadCardDataTable(UDataTable* InTable);

    // ── 카드 조회 ──────────────────────────────────────────────────────────

    /**
     * CardID 로 단일 카드 데이터를 가져온다. 없으면 nullptr.
     * (const 포인터 반환은 UFUNCTION 불가 -> C++ 전용)
     */
    const FCardDataRow* GetCard(int32 CardID) const;

    /** 직업에 해당하는 카드 ID 목록 반환 (Any 포함 카드 포함). */
    UFUNCTION(BlueprintCallable, Category = "Card|Query")
    TArray<int32> GetCardIDsByClass(EJobClass JobClass) const;

    /**
     * 직업에 해당하는 FCardDataRow 포인터 목록 반환.
     * (const 포인터 배열은 UFUNCTION 불가 -> C++ 전용)
     */
    TArray<const FCardDataRow*> GetCardsByClass(EJobClass JobClass) const;

    /**
     * 보상 카드 풀: 직업 + 희귀도 조건으로 카드 ID 목록 반환.
     * (희귀도 필터를 Normal 미만으로 걸면 Rarity = Normal 이상 Any 도 처리)
     */
    UFUNCTION(BlueprintCallable, Category = "Card|Reward")
    TArray<int32> GetRewardPool(EJobClass JobClass, ECardRarity MinRarity) const;

    /** 전체 카드 ID 목록 */
    UFUNCTION(BlueprintCallable, Category = "Card|Query")
    TArray<int32> GetAllCardIDs() const;

private:
    // ── 내부 멤버 ──────────────────────────────────────────────────────────
    /** 에디터에서 할당: Content/Data/DT_Cards */
    UPROPERTY()
    TObjectPtr<UDataTable> CardDataTable;

    /** Rarity 수치화 헬퍼 (보상 풀 필터링용) */
    static int32 RarityToInt(ECardRarity Rarity);
};
