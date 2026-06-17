#include "CombatKernel/EffectTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

bool UEffectTooltipWidget::SetEntries(const TArray<FEffectInfo>& Entries)
{
	if (Entries.Num() == 0)
		return false;

	// C++ 폴백: Text_Body에 "이름\n설명" 여러 줄로 합쳐 채움
	if (Text_Body)
	{
		FString Body;
		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			const FEffectInfo& E = Entries[i];
			Body += E.DisplayName.ToString();
			if (!E.Description.IsEmpty())
				Body += TEXT("\n") + E.Description.ToString();
			if (i < Entries.Num() - 1)
				Body += TEXT("\n\n");
		}
		Text_Body->SetText(FText::FromString(Body));
	}

	// BP 커스텀 레이아웃 훅
	OnEntriesSet(Entries);
	return true;
}

void UEffectTooltipWidget::ShowAt(const TArray<FEffectInfo>& Entries, FVector2D ViewportPos)
{
	if (!SetEntries(Entries))
	{
		HideTooltip();
		return;
	}

	// 뷰포트 추가(최초 1회)
	if (!IsInViewport())
		AddToViewport(10000);
	// 표시 후 레이아웃을 강제 갱신해 실제 크기(DesiredSize)를 얻는다 (클램프용)
	SetVisibility(ESlateVisibility::HitTestInvisible);
	ForceLayoutPrepass();

	// ── 좌표계 통일: 전부 DPI 독립(slot) 단위로 계산 ────────────────────────
	// ViewportPos(GetMousePositionOnViewport)와 DesiredSize는 이미 slot 단위.
	// 뷰포트 픽셀 크기를 DPI 배율로 나눠 slot 단위 화면 크기를 구한다.
	const float DPI = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), KINDA_SMALL_NUMBER);
	FVector2D ScreenSlot(0.f, 0.f);
	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* VP = World->GetGameViewport())
		{
			FVector2D PixelSize;
			VP->GetViewportSize(PixelSize);
			ScreenSlot = PixelSize / DPI;
		}
	}

	const FVector2D Size = GetDesiredSize();
	FVector2D Pos = ViewportPos + CursorOffset;

	// 화면 비율에 맞춰 오른쪽/아래로 넘치면 커서 반대쪽으로 뒤집거나 끌어올림
	if (ScreenSlot.X > 0.f && Pos.X + Size.X > ScreenSlot.X)
		Pos.X = ViewportPos.X - CursorOffset.X - Size.X;   // 커서 왼쪽으로 플립
	if (ScreenSlot.Y > 0.f && Pos.Y + Size.Y > ScreenSlot.Y)
		Pos.Y = ScreenSlot.Y - Size.Y;                     // 위로 끌어올림
	Pos.X = FMath::Max(0.f, Pos.X);
	Pos.Y = FMath::Max(0.f, Pos.Y);

	// 이미 slot 단위이므로 DPI 재나눗셈 금지 (bRemoveDPIScale=false)
	SetPositionInViewport(Pos, /*bRemoveDPIScale=*/false);
}

void UEffectTooltipWidget::HideTooltip()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
