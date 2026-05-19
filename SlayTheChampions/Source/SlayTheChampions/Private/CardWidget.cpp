#include "CardWidget.h"
#include "CardStyleDataAsset.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PaperSprite.h"

/**
 * PaperSprite 를 UImage 위젯에 적용하는 내부 헬퍼 함수.
 * Sprite 가 nullptr 이면 무시.
 *
 * @param ImageWidget  적용할 UImage 위젯
 * @param Sprite       적용할 UPaperSprite
 */
static void ApplySpriteToImage(UImage* ImageWidget, UPaperSprite* Sprite)
{
    if (!ImageWidget || !Sprite) return;

    FSlateBrush Brush;
    Brush.SetResourceObject(Sprite);
    ImageWidget->SetBrush(Brush);
}

/**
 * Description 텍스트의 태그를 실제 수치로 치환하는 내부 헬퍼 함수.
 * DT_Cards 의 Description 에 {Damage}, {Block} 등 태그를 사용하면
 * 카드 수치가 바뀔 때 자동으로 텍스트도 갱신된다.
 *
 * 사용 가능한 태그:
 *   {Damage}     피해량
 *   {Block}      방어도
 *   {DrawCount}  드로우 수
 *   {UsingCount} 사용 횟수
 *   {HealAmount} 회복량
 *   {EffectValue} 특수 효과 수치
 *
 * @param InCardData  태그 치환에 사용할 카드 데이터
 * @return 태그가 수치로 치환된 최종 텍스트
 */
static FText BuildDescription(const FCardDataRow& InCardData)
{
    FString Desc = InCardData.Description.ToString();

    Desc = Desc.Replace(TEXT("{Damage}"), *FString::FromInt(InCardData.Damage));
    Desc = Desc.Replace(TEXT("{Block}"), *FString::FromInt(InCardData.Block));
    Desc = Desc.Replace(TEXT("{DrawCount}"), *FString::FromInt(InCardData.DrawCount));
    Desc = Desc.Replace(TEXT("{UsingCount}"), *FString::FromInt(InCardData.UsingCount));
    Desc = Desc.Replace(TEXT("{HealAmount}"), *FString::FromInt(InCardData.HealAmount));
    Desc = Desc.Replace(TEXT("{EffectValue}"), *FString::FromInt(InCardData.EffectValue));

    return FText::FromString(Desc);
}

void UCardWidget::SetCardData(const FCardDataRow& InCardData, UCardStyleDataAsset* InStyle)
{
    // 현재 카드 데이터 저장 (Blueprint 에서 조회 가능)
    CurrentCardData = InCardData;

    if (InStyle)
    {
        // ── 직업별 이미지 적용 (희귀도 무관) ─────────────────────────────
        // 카드 외부 테두리, 내부 배경, 보석은 직업마다 고정
        ApplySpriteToImage(BorderOuter, InStyle->GetBorderOuter());
        ApplySpriteToImage(BorderInner, InStyle->GetBorderInner());
        ApplySpriteToImage(GemImage, InStyle->GetGemImage());

        // ── 희귀도별 이미지 적용 ─────────────────────────────────────────
        // 카드 그림 테두리, 속성 텍스트 배경은 희귀도에 따라 변경
        ApplySpriteToImage(RarityBorder, InStyle->GetRarityBorder(InCardData.Rarity));
        ApplySpriteToImage(TypeBackground, InStyle->GetTypeBackground(InCardData.Rarity));
    }

    // ── 카드 그림 적용 (카드마다 다름) ───────────────────────────────────
    // DT_Cards 의 MainImage 에서 해당 카드 스프라이트 로드
    ApplySpriteToImage(MainImage, InCardData.MainImage.LoadSynchronous());

    // ── 텍스트 적용 ──────────────────────────────────────────────────────
    if (CardNameText)
        CardNameText->SetText(InCardData.CardName);

    // Description 의 {Damage}, {Block} 등 태그를 실제 수치로 치환 후 적용
    if (DescriptionText)
        DescriptionText->SetText(BuildDescription(InCardData));

    if (CostText)
        CostText->SetText(FText::AsNumber(InCardData.Cost));

    // Blueprint 에서 추가 처리가 필요할 때 호출
    OnCardDataSet(InCardData);
}