#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PaperSprite.h"
#include "CardDataTypes.generated.h"

/**
 * ECardRarity
 * 카드 희귀도 열거형.
 * DA_CardStyle 에서 희귀도별 이미지를 결정하는 데 사용.
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
 * 공격/방어/스킬 등 카드의 행동을 분류.
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
 * 카드가 어느 직업 전용인지, DA_CardStyle 프레임에 사용.
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
 * 카드 대상 타입.
 * 단일 적/전체 적/자신/아군 등 카드 효과 적용 범위 결정.
 */
UENUM(BlueprintType)
enum class ETargetType : uint8
{
    SingleEnemy UMETA(DisplayName = "SingleEnemy"),  // 단일 적 대상
    AllEnemies  UMETA(DisplayName = "AllEnemies"),   // 전체 적
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
 * - CardSubsystem 에서 _CardID 로 조회
 * - HandComponent 에서 int32(카드 ID) 로 관리
 * - CardWidget 의 SetCardData() 에 전달해서 UI 갱신
 */
USTRUCT(BlueprintType)
struct FCardDataRow : public FTableRowBase
{
    GENERATED_BODY()

    // ── 카드 식별 ──────────────────────────────────────────────────────────

    // 카드 고유 번호 (101, 201 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    int32 _CardID;

    // 카드 이름 (타이틀, 표시 용) - UI 표시용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    FText CardName;

    // 카드 설명 텍스트 - UI 표시용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    FText Description;

    /**
     * 카드 대표 이미지 (PaperSprite).
     * 카드 1장마다 다른 스프라이트 사용.
     * DT_Cards 에서 pixelCardAssest_Sprite_XX 지정.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Identity")
    TSoftObjectPtr<UPaperSprite> MainImage;

    // ── 카드 분류 ──────────────────────────────────────────────────────────

    // 희귀도 (DA_CardStyle 에서 희귀도별 이미지 표시에 사용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Category")
    ECardRarity Rarity = ECardRarity::Normal;

    // 카드 타입 (공격/스킬 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Category")
    ECardType CardType = ECardType::Attack;

    // 사용 가능 직업 제한 (Any 면 모든 직업 사용 가능)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Category")
    EJobClass RequiredClass = EJobClass::Any;

    // ── 카드 코스트 ────────────────────────────────────────────────────────

    // 카드 사용 코스트 (에너지 소모 코스트로 사용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Cost",
        meta = (ClampMin = "0", ClampMax = "10"))
    int32 Cost = 1;

    // ── 효과 수치 ──────────────────────────────────────────────────────────

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

    // ── 카드 특수 효과 ─────────────────────────────────────────────────────

    // 특수 효과 태그 (버프/디버프 ID 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect")
    FName EffectTag;

    // 특수 효과 수치
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Effect",
        meta = (ClampMin = "0"))
    int32 EffectValue = 0;

    // ── 대상 범위 ──────────────────────────────────────────────────────────

    // 카드 효과 적용 범위
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Target")
    ETargetType TargetType = ETargetType::SingleEnemy;

    // ── 카드 밸런스 ────────────────────────────────────────────────────────

    // 카드 밸런스 점수 (기획 참고용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Balance",
        meta = (ClampMin = "0"))
    int32 ScoreValue = 0;

    // ── 기획 메타 ──────────────────────────────────────────────────────────

    // 기획 메모 (디자이너 이상용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card|Meta")
    FString Notes;
};
