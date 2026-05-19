#include "HandComponentTest.h"
#include "HandComponent.h"
#include "PartyMemberPawn.h"

AHandComponentTester::AHandComponentTester()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AHandComponentTester::BeginPlay()
{
    Super::BeginPlay();

    // Pawn BeginPlay 이후 실행 보장을 위해 타이머로 1프레임 지연
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AHandComponentTester::RunAllTests, 0.1f, false);
}

void AHandComponentTester::RunAllTests()
{
    UE_LOG(LogTemp, Log, TEXT("=============================="));
    UE_LOG(LogTemp, Log, TEXT("[HandTest] 테스트 시작"));
    UE_LOG(LogTemp, Log, TEXT("=============================="));

    if (Pawn1Ref) TestPawn(Pawn1Ref, TEXT("Pawn1"));
    else          UE_LOG(LogTemp, Warning, TEXT("[HandTest] Pawn1Ref 미할당 - 건너뜀"));

    if (Pawn2Ref) TestPawn(Pawn2Ref, TEXT("Pawn2"));
    else          UE_LOG(LogTemp, Warning, TEXT("[HandTest] Pawn2Ref 미할당 - 건너뜀"));

    UE_LOG(LogTemp, Log, TEXT("=============================="));
    UE_LOG(LogTemp, Log, TEXT("[HandTest] 테스트 종료"));
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}

void AHandComponentTester::TestPawn(APawn* Pawn, const FString& Label)
{
    Info(Label, FString::Printf(TEXT("Pawn 클래스: %s"), *Pawn->GetClass()->GetName()));

    UHandComponent* HC = Pawn->FindComponentByClass<UHandComponent>();
    if (!HC)
    {
        Fail(Label, TEXT("HandComponent 없음"));
        return;
    }

    Info(Label, FString::Printf(TEXT("초기 상태 - DrawPile:%d Hand:%d Discard:%d"),
        HC->GetDrawPileCount(), HC->GetHandCount(), HC->GetDiscardPileCount()));

    Test_InitialState(HC, Label);
    Test_DrawStartOfTurn(HC, Label);
    Test_PlayCard(HC, Label);
    Test_DiscardHand(HC, Label);
    Test_RecycleDiscard(HC, Label);
}

void AHandComponentTester::Test_InitialState(UHandComponent* HC, const FString& Label)
{
    const FString TC = Label + TEXT(" TC-01 [초기화 상태]");

    if (HC->GetDrawPileCount() == 0 && HC->GetHandCount() == 0 && HC->GetDiscardPileCount() == 0)
    {
        Fail(TC, TEXT("DrawPile 이 비어있음 - CardSubsystem / DataTable 연결 확인 필요"));
        return;
    }

    Pass(TC, FString::Printf(TEXT("DrawPile=%d장 정상 로드"), HC->GetDrawPileCount()));
}

void AHandComponentTester::Test_DrawStartOfTurn(UHandComponent* HC, const FString& Label)
{
    const FString TC = Label + TEXT(" TC-02 [턴 시작 드로우]");

    const int32 DrawBefore = HC->GetDrawPileCount();
    const int32 HandBefore = HC->GetHandCount();

    HC->DrawStartOfTurn();

    const int32 HandAfter = HC->GetHandCount();
    const int32 DrawnCount = HandAfter - HandBefore;
    const int32 Expected = HC->DefaultDrawCount;

    // 드로우된 카드 목록 출력
    const TArray<int32>& Hand = HC->GetHand();
    Info(TC, FString::Printf(TEXT("드로우 후 손패 (%d장):"), Hand.Num()));
    for (const int32 CardID : Hand)
    {
        Info(TC, FString::Printf(TEXT("  Hand: %d"), CardID));
    }

    if (DrawBefore >= Expected)
    {
        if (DrawnCount == Expected)
            Pass(TC, FString::Printf(TEXT("%d장 드로우 성공"), DrawnCount));
        else
            Fail(TC, FString::Printf(TEXT("기대:%d 실제:%d"), Expected, DrawnCount));
    }
    else
    {
        if (DrawnCount == DrawBefore)
            Pass(TC, FString::Printf(TEXT("DrawPile 부족 - 가능한 %d장 드로우"), DrawnCount));
        else
            Fail(TC, FString::Printf(TEXT("부족 상황 드로우 오류 기대:%d 실제:%d"), DrawBefore, DrawnCount));
    }
}

void AHandComponentTester::Test_PlayCard(UHandComponent* HC, const FString& Label)
{
    const FString TC = Label + TEXT(" TC-03 [카드 사용]");

    if (HC->GetHandCount() == 0)
    {
        Info(TC, TEXT("손패가 비어있어 건너뜀"));
        return;
    }

    const int32 TargetCard = HC->GetHand()[0];
    const int32 HandBefore = HC->GetHandCount();
    const int32 DiscardBefore = HC->GetDiscardPileCount();

    const bool bSuccess = HC->PlayCard(TargetCard);

    if (!bSuccess)
    {
        Fail(TC, FString::Printf(TEXT("PlayCard 실패 - CardID: %d"), TargetCard));
        return;
    }

    const int32 HandAfter = HC->GetHandCount();
    const int32 DiscardAfter = HC->GetDiscardPileCount();

    Info(TC, FString::Printf(TEXT("사용 카드: %d | Hand:%d->%d | Discard:%d->%d"),
        TargetCard, HandBefore, HandAfter, DiscardBefore, DiscardAfter));

    if (HandAfter == HandBefore - 1 && DiscardAfter == DiscardBefore + 1)
        Pass(TC, TEXT("Hand -1, Discard +1 정상"));
    else
        Fail(TC, TEXT("카드 이동 수치 불일치"));

    const bool bFail = HC->PlayCard(-1);
    if (!bFail)
        Pass(TC, TEXT("없는 카드 PlayCard -> false 반환 정상"));
    else
        Fail(TC, TEXT("없는 카드 PlayCard 가 true 를 반환함"));
}

void AHandComponentTester::Test_DiscardHand(UHandComponent* HC, const FString& Label)
{
    const FString TC = Label + TEXT(" TC-04 [손패 버리기]");

    const int32 HandBefore = HC->GetHandCount();
    const int32 DiscardBefore = HC->GetDiscardPileCount();

    HC->DiscardHand();

    const int32 HandAfter = HC->GetHandCount();
    const int32 DiscardAfter = HC->GetDiscardPileCount();

    Info(TC, FString::Printf(TEXT("Hand:%d->%d | Discard:%d->%d"),
        HandBefore, HandAfter, DiscardBefore, DiscardAfter));

    if (HandAfter == 0 && DiscardAfter == DiscardBefore + HandBefore)
        Pass(TC, TEXT("손패 전량 Discard 이동 정상"));
    else
        Fail(TC, TEXT("수치 불일치"));
}

void AHandComponentTester::Test_RecycleDiscard(UHandComponent* HC, const FString& Label)
{
    const FString TC = Label + TEXT(" TC-05 [Discard 재순환]");

    const int32 TotalInDraw = HC->GetDrawPileCount();
    if (TotalInDraw > 0)
    {
        HC->DrawCards(TotalInDraw);
    }

    const int32 DiscardBeforeRecycle = HC->GetDiscardPileCount();
    Info(TC, FString::Printf(TEXT("DrawPile 소진 후 - DrawPile:0 Hand:%d Discard:%d"),
        HC->GetHandCount(), DiscardBeforeRecycle));

    if (DiscardBeforeRecycle == 0)
    {
        Info(TC, TEXT("Discard 도 비어있어 재순환 유발 불가"));
        return;
    }

    HC->DrawCards(1);

    const int32 DrawAfterRecycle = HC->GetDrawPileCount();
    Info(TC, FString::Printf(TEXT("재순환 후 - DrawPile:%d Hand:%d Discard:%d"),
        DrawAfterRecycle, HC->GetHandCount(), HC->GetDiscardPileCount()));

    if (HC->GetHandCount() > 0)
        Pass(TC, FString::Printf(TEXT("재순환 성공 - DrawPile 복구됨 (%d장)"), DrawAfterRecycle));
    else
        Fail(TC, TEXT("재순환 후 DrawPile 이 여전히 비어있음"));
}

void AHandComponentTester::Pass(const FString& Label, const FString& Msg)
{
    UE_LOG(LogTemp, Log, TEXT("[HandTest] PASS | %s | %s"), *Label, *Msg);
}

void AHandComponentTester::Fail(const FString& Label, const FString& Msg)
{
    UE_LOG(LogTemp, Error, TEXT("[HandTest] FAIL | %s | %s"), *Label, *Msg);
}

void AHandComponentTester::Info(const FString& Label, const FString& Msg)
{
    UE_LOG(LogTemp, Log, TEXT("[HandTest] INFO | %s | %s"), *Label, *Msg);
}