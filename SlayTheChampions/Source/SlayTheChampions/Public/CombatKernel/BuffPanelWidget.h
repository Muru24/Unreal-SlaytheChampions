#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatKernel/EffectTypes.h"
#include "BuffPanelWidget.generated.h"

class UHorizontalBox;
class UBuffIconWidget;
class UStatusEffectComponent;
class AUnit;

/**
 * UBuffPanelWidget
 * 유닛에 활성화된 버프/디버프 목록을 표시하는 패널.
 *
 * 동작 방식:
 *  - InitFromUnit으로 유닛의 StatusEffectComponent에 바인딩
 *  - OnEffectValueChanged 델리게이트 수신 시 자동으로 아이콘 추가/갱신/제거
 *  - 값이 0이 되면 해당 아이콘을 풀에서 제거
 *
 * WBP_BuffPanel이 이 클래스를 부모로 사용한다.
 */
UCLASS()
class SLAYTHECHAMPIONS_API UBuffPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 생성할 버프 아이콘 위젯 클래스 (에디터에서 WBP_BuffIcon 지정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	TSubclassOf<UBuffIconWidget> BuffIconClass;

	// 아이콘 사이 오른쪽 간격 (에디터에서 조절)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff", meta = (ClampMin = "0"))
	float IconSpacing = 4.f;

	// ── 공개 API ─────────────────────────────────────────────────────────────
	// 유닛의 StatusEffectComponent에 바인딩 — 이후 자동으로 버프 UI 갱신
	UFUNCTION(BlueprintCallable, Category = "Buff")
	void InitFromUnit(AUnit* Unit);

	// 현재 바인딩 해제 (유닛 전환 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Buff")
	void Unbind();

protected:
	// 아이콘이 추가될 컨테이너 (WBP에서 'BuffContainer' 이름으로 HorizontalBox 배치)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* BuffContainer;

private:
	// 현재 바인딩된 StatusEffectComponent
	UPROPERTY()
	UStatusEffectComponent* BoundSEC = nullptr;

	// EEffectType → 생성된 UBuffIconWidget 캐시 (빠른 갱신용)
	UPROPERTY()
	TMap<EEffectType, UBuffIconWidget*> ActiveIcons;

	// StatusEffectComponent::OnEffectValueChanged 수신
	// Value > 0 이면 아이콘 추가/갱신, Value == 0 이면 아이콘 제거
	UFUNCTION()
	void OnEffectValueChanged(EEffectType Type, int32 OldValue, int32 NewValue);

	// 새 버프 아이콘 생성 후 BuffContainer에 추가
	UBuffIconWidget* CreateIcon(EEffectType Type, int32 Value);

	// 아이콘 제거 및 ActiveIcons에서 삭제
	void RemoveIcon(EEffectType Type);
};
