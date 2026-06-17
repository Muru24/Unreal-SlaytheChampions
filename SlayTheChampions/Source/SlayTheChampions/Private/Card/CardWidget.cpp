#include "Card/CardWidget.h"
#include "Card/CardStyleDataAsset.h"
#include "CombatKernel/HandWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PaperSprite.h"
#include "Engine/Texture2D.h"

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

// ClearHand 직전에 HandWidget이 호출 — 이후 발화되는 모든 Slate 입력 이벤트를 차단
// GetParent() 기반 검사는 UE5의 ClearChildren 내부 실행 순서에 따라
// Slot 포인터가 null이 되기 전에 MouseLeave가 발화될 수 있어 신뢰할 수 없음
void UCardWidget::MarkForRemoval()
{
    bPendingRemoval = true;
}

// 마우스가 카드 위에 올라오면 BP 호버 이벤트 호출
void UCardWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bPendingRemoval) return;
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    OnCardHovered();
}

// 카드 클릭 이벤트를 여기서 소비
// OwningHandWidget에 직접 브로드캐스트 — BP 이벤트 디스패처 없이 즉시 전달
// Handled() 반환으로 상위 패널(BattleMainWidget 캔버스)로의 전파를 차단
FReply UCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bPendingRemoval) return FReply::Handled();
    Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    if (OwningHandWidget)
    {
        // 브로드캐스트 전에 자기 자신을 직접 등록 — CardID 검색 시 인덱스 0번이 잘못 선택되는 문제 방지
        OwningHandWidget->SetCardPendingDirect(this);
        OwningHandWidget->OnCardSelected.Broadcast(CurrentCardData.CardID, this);
    }
    return FReply::Handled();
}

// 마우스가 카드를 벗어나면 BP 언호버 이벤트 호출
// 대기 상태(bIsPendingState)이면 언호버를 억제해 커진 상태를 유지
// bPendingRemoval이 true이면 ClearHand 이후 발화된 디퍼드 MouseLeave이므로 완전 무시
void UCardWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    // ClearChildren 직전에 설정된 플래그 — Slate 내부 순서와 무관하게 이벤트 차단
    if (bPendingRemoval) return;
    Super::NativeOnMouseLeave(InMouseEvent);
    if (!bIsPendingState)
        OnCardUnhovered();
}

// 타겟 대기 상태 진입/해제
// 진입 시: OnCardHovered 호출로 마우스 위치 무관하게 확대 상태 유지
// 해제 시: OnCardUnhovered 호출로 원래 크기로 복귀
void UCardWidget::SetPendingState(bool bInPending)
{
    bIsPendingState = bInPending;
    OnPendingStateChanged(bInPending);
    if (bIsPendingState)
        OnCardHovered();
    else
        OnCardUnhovered();
}

void UCardWidget::SetCardData(const FCardDataRow& InCardData, UCardStyleDataAsset* InStyle)
{
    // 현재 카드 데이터 저장 (Blueprint 에서 읽기 가능)
    CurrentCardData = InCardData;

    if (InStyle)
    {
        // 공통 이미지 적용 (희귀도 무관) ────────────────────────────────────
        // 카드 외부 테두리, 내부 이미지, 코스트 젬 이미지 적용
        ApplySpriteToImage(BorderOuter, InStyle->GetBorderOuter());
        ApplySpriteToImage(BorderInner, InStyle->GetBorderInner());
        ApplySpriteToImage(GemImage, InStyle->GetGemImage());

        // 희귀도별 이미지 적용 ────────────────────────────────────────────────
        // 카드 그림 테두리, 속성 텍스트 배경을 희귀도에 맞게 적용
        ApplySpriteToImage(RarityBorder, InStyle->GetRarityBorder(InCardData.Rarity));
        ApplySpriteToImage(TypeBackground, InStyle->GetTypeBackground(InCardData.Rarity));
    }

    // 카드마다 다른 그림 적용 ─────────────────────────────────────────────────
    // DT_Cards 의 MainImage(Texture2D) 동기 로드 후 UImage 에 적용
    if (MainImage)
    {
        if (UTexture2D* Tex = InCardData.MainImage.LoadSynchronous())
            MainImage->SetBrushFromTexture(Tex);
    }

    // 텍스트 적용 ─────────────────────────────────────────────────────────────
    if (CardNameText)
        CardNameText->SetText(InCardData.CardName);

    if (DescriptionText)
        DescriptionText->SetText(InCardData.Description);

    if (CostText)
        CostText->SetText(FText::AsNumber(InCardData.Cost));

    // Blueprint 에서 추가 처리가 필요할 때 호출
    OnCardDataSet(InCardData);
}
