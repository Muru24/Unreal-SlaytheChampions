#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Unit/CombatTypes.h"
#include "CombatKernel/EffectTypes.h"
#include "MonsterActionWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class UBuffIconWidget;

/**
 * UMonsterActionWidget
 * 몬스터 머리 위에 붙는 위젯.
 * 몬스터 이름 + 이번 턴 예고 행동 1~3개를 BuffIconWidget으로 표시한다.
 *
 * C++ 역할:
 *   UpdateActions(TArray<FIntent>) → ActionContainer를 비우고
 *   행동 수만큼 BuffIconWidget 동적 생성 → SetBuff(MappedType, Value)
 *   Attack이면 데미지 수치 표시, 나머지는 0 (숫자 숨김)
 *
 * BP 작업:
 *   1. WBP 부모 클래스를 MonsterActionWidget으로 설정
 *   2. Text_MonsterName, ActionContainer(HorizontalBox) 배치
 *   3. IconWidgetClass에 WBP_BuffIcon 설정
 *   4. IntentIconMap에 EIntentKind → Intent_* EEffectType 매핑 설정
 *   5. WBP_BuffIcon의 EffectSystemMap에 Intent_* → 나이아가라 에셋 연결
 *   6. 아이콘 위젯 크기를 3:2 비율로 설정 (수치 텍스트와 겹침 방지)
 */
UCLASS()
class SLAYTHECHAMPIONS_API UMonsterActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 생성할 아이콘 위젯 클래스 (BP에서 WBP_BuffIcon 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
	TSubclassOf<UBuffIconWidget> IconWidgetClass;

	// EIntentKind → EEffectType 매핑 (BP 클래스 디폴트에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent")
	TMap<EIntentKind, EEffectType> IntentIconMap;

	// 아이콘 간 간격 (px)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intent", meta = (ClampMin = "0"))
	float IconSpacing = 6.f;

	// 몬스터 이름 설정 — 초기화 시 1회 호출
	UFUNCTION(BlueprintCallable, Category = "Intent")
	void SetMonsterName(const FText& Name);

	// 행동 목록으로 컨테이너 갱신 — 기존 항목 제거 후 재생성
	UFUNCTION(BlueprintCallable, Category = "Intent")
	void UpdateActions(const TArray<FIntent>& Intents);

	// 현재 표시 중인 의도들 — 호버 툴팁 등에서 다중 행동을 그대로 조회 (UpdateActions가 갱신)
	UPROPERTY(BlueprintReadOnly, Category = "Intent")
	TArray<FIntent> DisplayedIntents;

protected:
	// 몬스터 이름 텍스트 (WBP에서 'Text_MonsterName' 이름으로 배치)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_MonsterName;

	// 행동 아이콘 컨테이너 (WBP에서 'ActionContainer' 이름의 HorizontalBox 배치)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* ActionContainer;
};
