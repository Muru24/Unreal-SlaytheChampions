#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatKernel/CardComboTypes.h"
#include "CardComboLibrary.generated.h"

class UDataTable;
class ACombatManager;

/**
 * UCardComboLibrary
 * 콤보 족보(WBP_ComboBook) UI용 BlueprintFunctionLibrary.
 * DataTable 조회 + 조건/보상의 사람이 읽을 수 있는 텍스트 자동 생성.
 * 텍스트가 로직(조건 구조체)에서 직접 생성되므로 항상 동기화된다.
 */
UCLASS()
class SLAYTHECHAMPIONS_API UCardComboLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// DT_CardCombos의 모든 행을 배열로 반환 (족보 채우기용). bEnabled=false도 포함
	UFUNCTION(BlueprintCallable, Category = "Combo")
	static void GetComboRules(UDataTable* Table, TArray<FCardComboRuleRow>& OutRules);

	// 조건을 사람이 읽는 텍스트로 — 예: "공격 3연속", "타격 → 수비 순서" (카드 ID는 이름으로 변환)
	// WorldContextObject로 CardSubsystem을 찾아 카드 ID → 카드 이름 변환. BP에선 self가 자동 연결됨.
	UFUNCTION(BlueprintPure, Category = "Combo", meta = (WorldContext = "WorldContextObject"))
	static FText DescribeComboCondition(const FCardComboCondition& Condition, UObject* WorldContextObject);

	// 보상을 사람이 읽는 텍스트로 — 예: "All Enemies 20 데미지 + Burn 3"
	UFUNCTION(BlueprintPure, Category = "Combo")
	static FText DescribeComboReward(const FCardComboReward& Reward);

	// 라이브 진행도 — 현재 큐 꼬리 기준으로 '진행 중(matched≥1)' 또는 '이번 카드에 막 완성된' 콤보만 반환.
	// 이미 지난 카드에 완성된 콤보는 제외(다음 카드 사용 시 자동으로 사라지는 효과).
	// 슬롯 라벨은 카드 이름으로 채워진다.
	UFUNCTION(BlueprintCallable, Category = "Combo")
	static void GetComboProgress(ACombatManager* CombatManager, UDataTable* Table, TArray<FCardComboProgress>& OutProgress);
};
