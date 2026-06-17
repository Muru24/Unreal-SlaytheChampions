#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CombatKernel/EffectTypes.h"
#include "CardDataTypes.generated.h"

/**
 * ECardRarity
 * 카드 희귀도 열거형.
 * DA_CardStyle 에서 희귀도별 이미지를 참조하는 데 사용.
 * 카드 희귀도 열거형.
 * DA_CardStyle 에서 희귀도별 이미지를 참조하는 데 사용.
 */
UENUM(BlueprintType)
enum class ECardRarity : uint8
{
    Normal    UMETA(DisplayName = "Normal"),
    Rare      UMETA(DisplayName = "Rare"),
    Legendary UMETA(DisplayName = "Legendary"),
    Status    UMETA(DisplayName = "Status"),
};

/**
 * ECardType
 * 카드 종류.
 * 공격/방어/스킬 등 카드의 분류를 결정.
 */
UENUM(BlueprintType)
enum class ECardType : uint8
{
    Attack  UMETA(DisplayName = "Attack"),
    Defense UMETA(DisplayName = "Defense"),
    Skill   UMETA(DisplayName = "Skill"),
    Buff    UMETA(DisplayName = "Buff"),
    AOE     UMETA(DisplayName = "AOE"),
    Status  UMETA(DisplayName = "Status"),
};

/**
 * EJobClass
 * 직업 분류.
 * 카드가 어느 직업 전용인지, DA_CardStyle 에서 테마에 사용.
 */
UENUM(BlueprintType)
enum class EJobClass : uint8
{
    Any     UMETA(DisplayName = "Any"),
    Warrior UMETA(DisplayName = "Warrior"),
    Mage    UMETA(DisplayName = "Mage"),
    Healer  UMETA(DisplayName = "Healer"),
};

/**
 * ETargetType
 * 카드 효과 타겟 타입.
 * 단일 적/전체 적/자신/아군 등 카드 효과 적용 대상 결정.
 */
UENUM(BlueprintType)
enum class ETargetType : uint8
{
    SingleEnemy UMETA(DisplayName = "SingleEnemy"),  // 단일 적 대상
    AllEnemies  UMETA(DisplayName = "AllEnemies"),   // 모든 적 대상
    Self        UMETA(DisplayName = "Self"),          // 자기 자신
    SingleAlly  UMETA(DisplayName = "SingleAlly"),   // 아군 단일 대상
    AllAllies   UMETA(DisplayName = "AllAllies"),     // 아군 전체
    Single_Team UMETA(DisplayName = "Single_Team"),  // 파티원 선택
};

/**
 * FCardDataRow
 *
 * DT_Cards DataTable 의 Row Struct.
 * 카드 1장의 모든 데이터를 담는다.
 *
 * [참고]
 * - MainImage  : PaperSprite (pixelCardAssest_Sprite_XX)
 * - CardSubsystem 에서 RowName(카드 ID) 기반 조회
 * - HandComponent 에서 FName(카드 ID) 기반 사용
 * - CardWidget 의 SetCardData() 에 전달해서 UI 갱신
 * - bExhaust   : true 이면 사용 시 ExhaustPile 으로 이동 (소멸),
 *                전투 종료 후 SaveAllDecks 에 포함되지 않음
 *                (제거와 다름 - 다음 전투에서 덱에 복귀)
 */
USTRUCT(BlueprintType)
struct FCardDataRow : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 식별 ──────────────────────────────────────────────────────────────────

    // 카드 고유 번호 (101, 201 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    FName CardID;

    // 카드 이름 (타이틀, 한글 등) - UI 표시용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    FText CardName;

    // 카드 설명 텍스트 - UI 표시용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    FText Description;

    /**
     * 카드 메인 이미지 (Texture2D).
     * 카드 1장마다 다른 텍스처 참조.
     * DT_Cards 에서 Content/01_Card/CardMainImage/ 경로 지정.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    TSoftObjectPtr<UTexture2D> MainImage;

    // 희귀도 및 분류 ──────────────────────────────────────────────────────────────

    // 희귀도 (DA_CardStyle 에서 희귀도별 이미지 결정에 사용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Category")
    ECardRarity Rarity = ECardRarity::Normal;

    // 카드 타입 (공격/스킬 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Category")
    ECardType CardType = ECardType::Attack;

    // 필요 직업 분류 (Any 면 모든 직업 사용 가능)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Category")
    EJobClass RequiredClass = EJobClass::Any;

    // 카드 코스트 ──────────────────────────────────────────────────────────────────

    // 카드 사용 코스트 (파티 공유 코스트에서 차감)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Cost",
        meta = (ClampMin = "0", ClampMax = "10"))
    int32 Cost = 1;

    // 기본 효과 수치 ──────────────────────────────────────────────────────────────

    // 공격력
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect",
        meta = (ClampMin = "0"))
    int32 Damage = 0;

    // 방어
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect",
        meta = (ClampMin = "0"))
    int32 Block = 0;

    // 드로우 수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect",
        meta = (ClampMin = "0"))
    int32 DrawCount = 0;

    // 사용 횟수 (1 = 1회, 2 = 2회 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect",
        meta = (ClampMin = "1"))
    int32 UsingCount = 1;

    // 회복량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect",
        meta = (ClampMin = "0"))
    int32 HealAmount = 0;

    // 버프/디버프 효과 목록 ────────────────────────────────────────────────────────
    // 한 카드에 여러 효과 지정 가능 (예: Damage + Debuff_Weak, Buff_Regen + Buff_AttackUp)
    // EffectType 범위에 따라 ExecuteCard에서 ApplyBuff / ApplyDebuff / ApplyEffect 자동 분기
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect")
    TArray<FCardEffect> Effects;

    // 타겟 설정 ──────────────────────────────────────────────────────────────────

    // 카드 효과 대상 타입
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Target")
    ETargetType TargetType = ETargetType::SingleEnemy;

    // 소멸 여부 ──────────────────────────────────────────────────────────────────

    /**
     * 소멸 카드 여부.
     * true  : 사용 시 DiscardPile 대신 ExhaustPile 로 이동.
     *         전투 종료 후 SaveAllDecks 에 포함되지 않음 (전투 한정 소멸).
     *         다음 전투 시작 시 덱에 자동 복귀 (A방식).
     * false : 일반 카드 - 사용 후 DiscardPile 로 이동.
     *
     * [소멸 vs 제거]
     * 소멸(Exhaust) = 전투 내 임시 제거, 다음 전투 복귀.
     * 제거(Remove)  = CardManager 에서 영구 삭제 (상점/특수 이벤트 전용).
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect")
    bool bExhaust = false;

    // 업적 전용 여부 ──────────────────────────────────────────────────────────────

    /**
     * 업적 전용 카드 여부.
     * true  : 특정 업적 달성 시에만 도감에 등록됨.
     *         미획득 시 도감에서 완전 검정으로 표시 (이름/정보 숨김).
     * false : 일반 카드 - 보상/상점으로 획득 가능.
     *         미획득 시 도감에서 실루엣(어둡게)으로 표시.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Meta")
    bool bAchievementOnly = false;

    // 밸런스 정보 ──────────────────────────────────────────────────────────────────

    // 카드 밸런스 점수 (기획 참고용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Balance",
        meta = (ClampMin = "0"))
    int32 ScoreValue = 0;

    // 기획 메타 ──────────────────────────────────────────────────────────────────

    // 기획 메모 (기타나 이상형)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Meta")
    FString Notes;
};
