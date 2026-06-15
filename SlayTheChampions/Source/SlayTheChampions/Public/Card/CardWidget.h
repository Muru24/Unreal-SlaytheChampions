#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Card/CardDataTypes.h"
#include "CardWidget.generated.h"

class UCardStyleDataAsset;
class UHandWidget;
class UImage;
class UTextBlock;
class URichTextBlock;

/**
 * UCardWidget
 *
 * 카드 한 장 UI 의 C++ 베이스 클래스.
 * WBP_Card 의 부모 클래스로 설정.
 *
 * [위젯 구성]
 * 공통
 *   BorderOuter  : 카드 외부 테두리 이미지
 *   BorderInner  : 카드 내부 이미지
 *   GemImage     : 코스트 젬 이미지
 *
 * 카드마다
 *   MainImage    : 카드 그림
 *
 * 희귀도별
 *   RarityBorder    : 카드 그림 테두리
 *   TypeBackground  : 카드 속성 텍스트 배경
 *
 * 텍스트
 *   CardNameText    : 카드 이름
 *   DescriptionText : 카드 설명
 *   CostText        : 코스트 숫자
 *
 * [사용법]
 * 1. WBP_Card 의 UCardWidget 을 부모로 설정
 * 2. WBP_Card 의 아래 BindWidget 변수명으로 위젯 배치
 * 3. SetCardData() 호출하면 자동으로 내용 채워짐
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * 카드 데이터와 스타일을 받아 위젯 전체를 갱신.
     *
     * @param InCardData  FCardDataRow 구조체 (DT_Cards 에서 조회)
     * @param InStyle     직업별 UCardStyleDataAsset (DA_CardStyle_Warrior 등)
     */
    UFUNCTION(BlueprintCallable, Category = "Card|UI")
    void SetCardData(const FCardDataRow& InCardData, UCardStyleDataAsset* InStyle);

    // Blueprint 에서 추가 처리가 필요할 때 오버라이드
    UFUNCTION(BlueprintImplementableEvent, Category = "Card|UI")
    void OnCardDataSet(const FCardDataRow& InCardData);

    // ── 호버 이벤트 ─────────────────────────────────────────────────────────

    // 마우스가 카드 위에 올라왔을 때 — BP에서 위로 이동 + 확대 애니메이션 구현
    UFUNCTION(BlueprintImplementableEvent, Category = "Card|Hover")
    void OnCardHovered();

    // 마우스가 카드를 벗어났을 때 — BP에서 원래 위치/크기 복귀 구현
    UFUNCTION(BlueprintImplementableEvent, Category = "Card|Hover")
    void OnCardUnhovered();

    // ── 대기 상태 ────────────────────────────────────────────────────────────

    // 타겟 대기 상태 진입/해제
    // bInPending = true : 마우스 이탈해도 커진 상태 유지
    // bInPending = false: 즉시 언호버 처리
    UFUNCTION(BlueprintCallable, Category = "Card|Pending")
    void SetPendingState(bool bInPending);

    // 현재 대기 상태 여부 반환
    UFUNCTION(BlueprintPure, Category = "Card|Pending")
    bool IsPendingState() const { return bIsPendingState; }

    // 대기 상태 변경 시 BP에 알림 — WBP_Card에서 노란색 선택 테두리 표시/숨김 구현
    UFUNCTION(BlueprintImplementableEvent, Category = "Card|Pending")
    void OnPendingStateChanged(bool bIsPending);

    // CardID 반환 — HandWidget에서 일치 카드 검색에 사용
    UFUNCTION(BlueprintPure, Category = "Card|UI")
    FName GetCardID() const { return CurrentCardData.CardID; }

    // HandWidget::ClearHand 에서 ClearChildren 직전에 호출
    // — Slate 디퍼드 MouseLeave 이벤트가 언제 발화되든 BP 이벤트 전파를 차단
    void MarkForRemoval();

protected:
    // 마우스 진입 — OnCardHovered 호출
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    // 마우스 이탈 — OnCardUnhovered 호출 (대기 상태면 억제)
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    // 클릭 이벤트 소비 — BattleMainWidget 캔버스로 전파 방지
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
    // ── 공통 이미지 (WBP_Card 에서 변수명 일치 배치 필수) ────────────────────

    // 카드 외부 테두리 이미지
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> BorderOuter;

    // 카드 내부 이미지
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> BorderInner;

    // 코스트 젬 이미지
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> GemImage;

    // ── 카드마다 다른 이미지 ─────────────────────────────────────────────────

    // 카드 그림
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> MainImage;

    // ── 희귀도별 이미지 ──────────────────────────────────────────────────────

    // 카드 그림 테두리 (희귀도별 색상 다름)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> RarityBorder;

    // 카드 속성 텍스트 배경
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UImage> TypeBackground;

    // ── 텍스트 ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> CardNameText;

    // 카드 설명 — RichTextBlock 으로 <debuff>, <buff>, <exhaust> 태그 색상 표시
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<URichTextBlock> DescriptionText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> CostText;

    // HandWidget 참조 — OnShowHand에서 카드 생성 직후 설정, 클릭 시 브로드캐스트에 사용
    UPROPERTY(BlueprintReadWrite, Category = "Card|UI")
    TObjectPtr<UHandWidget> OwningHandWidget;

private:
    // 현재 세팅된 카드 데이터 (Blueprint 에서 읽기 가능)
    UPROPERTY(BlueprintReadOnly, Category = "Card|UI", meta = (AllowPrivateAccess = "true"))
    FCardDataRow CurrentCardData;

    // 타겟 대기 상태 플래그 — true이면 마우스 이탈 시 언호버 억제
    bool bIsPendingState = false;

    // HandWidget::ClearHand 에서 설정. true이면 모든 Slate 입력 이벤트를 무시
    // — ClearChildren 직전에 설정되므로 Slate 디퍼드 MouseLeave 타이밍에 무관하게 동작
    bool bPendingRemoval = false;
};
