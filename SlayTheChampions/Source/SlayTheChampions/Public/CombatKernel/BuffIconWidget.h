#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatKernel/EffectTypes.h"
#include "BuffIconWidget.generated.h"

class UTextBlock;
class UNiagaraSystem;

/**
 * UBuffIconWidget
 * 버프/디버프 아이콘 1개를 표시하는 위젯.
 *
 * C++ 역할: SetBuff 호출 시 텍스트 갱신 + BP에 시스템 교체 이벤트 전달
 * BP 역할: NiagaraSystemWidget 스왑 + PopulateRemapList 구현
 *
 * WBP_BuffIcon이 이 클래스를 부모로 사용한다.
 */
UCLASS()
class SLAYTHECHAMPIONS_API UBuffIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── 에디터 설정 ─────────────────────────────────────────────────────────
	// EEffectType → 나이아가라 시스템 매핑
	// WBP_BuffIcon 클래스 디폴트에서 각 버프/디버프에 해당하는 나이아가라 에셋 연결
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff|Visual")
	TMap<EEffectType, UNiagaraSystem*> EffectSystemMap;

	// ── 현재 상태 ────────────────────────────────────────────────────────────
	// 현재 표시 중인 효과 타입
	UPROPERTY(BlueprintReadOnly, Category = "Buff")
	EEffectType CurrentEffectType = EEffectType::None;

	// 현재 스택 수
	UPROPERTY(BlueprintReadOnly, Category = "Buff")
	int32 CurrentValue = 0;

	// ── 공개 API ─────────────────────────────────────────────────────────────
	// 버프/디버프 타입과 스택 수를 설정 — 텍스트 갱신 후 BP에 시스템 교체 요청
	UFUNCTION(BlueprintCallable, Category = "Buff")
	void SetBuff(EEffectType Type, int32 Value);

	// ── BP 구현부 ─────────────────────────────────────────────────────────────
	// SetBuff 호출 시 발동 — WBP에서 NiagaraSystemWidget의 시스템을 교체하고 PopulateRemapList 처리
	UFUNCTION(BlueprintImplementableEvent, Category = "Buff")
	void OnBuffTypeChanged(UNiagaraSystem* NewSystem);

protected:
	// 스택 수 텍스트 (WBP에서 'Text_Value' 이름으로 배치)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text_Value;
};
