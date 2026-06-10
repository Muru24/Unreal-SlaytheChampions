# 카드 히스토리 콤보 시스템 — 구조 설계 (제안)

전투 중 사용한 카드 히스토리(`ACombatManager::ActionQueue`)를 감시해, 디자이너가 DataTable로 정의한
**조건(같은 카드 N회 / 같은 타입 N회 등)이 충족되면 보상 효과(데미지·버프·디버프 등)를 발동**하는 시스템.

> 상태: **설계 제안** — 구현 전 결정 사항(맨 아래 7절) 확인 필요.

---

## 1. 기존 구조와의 연결점

| 재사용 대상 | 위치 | 용도 |
|------------|------|------|
| `ActionQueue : TArray<FQueuedAction>` | CombatManager | 전투 내내 누적되는 카드 히스토리 (현재 턴마다 안 비움) |
| `FQueuedAction { Card, CasterIndex, TargetOverride, CardRowName }` | CombatManager.h | 히스토리 1건 — 콤보 판정 입력 |
| `OnActionExecuted(Card, CasterIndex)` | CombatManager | 카드 1장 실행 직후 브로드캐스트 — **콤보 평가 훅 지점** |
| `FCardEffect { EffectType, Value, TargetType }` | EffectTypes.h | 콤보 **보상 효과** 정의에 그대로 재사용 |
| `ECardType / ECardRarity / EJobClass / CardID` | CardDataTypes.h | 콤보 **조건 매칭 키**로 재사용 |
| `UEffectManager::ApplyBuff/ApplyDebuff/...` | EffectManager | 콤보 보상 효과 적용 |

핵심: **새 효과 시스템을 만들지 않는다.** 조건 판정 + 기존 효과 적용을 잇는 얇은 레이어만 추가.

---

## 2. 신규 열거형

```cpp
// 무엇을 기준으로 "같다"고 볼지
UENUM(BlueprintType)
enum class ECardComboMatchKey : uint8
{
    CardID    UMETA(DisplayName="Same Card (CardID)"),   // 완전히 같은 카드
    CardType  UMETA(DisplayName="Same Type"),            // Attack/Defense/Skill...
    Rarity    UMETA(DisplayName="Same Rarity"),
    JobClass  UMETA(DisplayName="Same Job"),
};

// 카운트 집계 범위
UENUM(BlueprintType)
enum class ECardComboWindow : uint8
{
    ThisTurn     UMETA(DisplayName="This Turn"),      // 이번 플레이어 행동턴 동안
    WholeCombat  UMETA(DisplayName="Whole Combat"),   // 전투 전체 누적
    Consecutive  UMETA(DisplayName="Consecutive"),    // 연속 (중간에 다른 키 끼면 리셋)
};

// 발동 반복 정책
UENUM(BlueprintType)
enum class ECardComboRepeat : uint8
{
    OncePerCombat   UMETA(DisplayName="Once / Combat"),
    OncePerTurn     UMETA(DisplayName="Once / Turn"),
    EveryThreshold  UMETA(DisplayName="Every N (3,6,9...)"),  // 임계값 도달마다 반복
};
```

---

## 3. 조건 / 보상 구조체

```cpp
// 콤보 발동 조건
USTRUCT(BlueprintType)
struct FCardComboCondition
{
    GENERATED_BODY()

    // 그룹화/비교 기준
    UPROPERTY(EditAnywhere) ECardComboMatchKey MatchKey = ECardComboMatchKey::CardType;

    // false: "같은 것 아무거나 N개" (예: 같은 타입 아무거나 3개)
    // true : 아래 지정값과 일치하는 카드 N개 (예: 'Attack' 타입 정확히 3개)
    UPROPERTY(EditAnywhere) bool bRequireSpecificValue = false;

    // bRequireSpecificValue=true일 때, MatchKey에 해당하는 칸만 사용
    UPROPERTY(EditAnywhere, meta=(EditCondition="bRequireSpecificValue"))
    FName        SpecificCardID;                          // MatchKey==CardID
    UPROPERTY(EditAnywhere, meta=(EditCondition="bRequireSpecificValue"))
    ECardType    SpecificCardType = ECardType::Attack;    // MatchKey==CardType
    UPROPERTY(EditAnywhere, meta=(EditCondition="bRequireSpecificValue"))
    ECardRarity  SpecificRarity   = ECardRarity::Normal;  // MatchKey==Rarity
    UPROPERTY(EditAnywhere, meta=(EditCondition="bRequireSpecificValue"))
    EJobClass    SpecificJob      = EJobClass::Any;        // MatchKey==JobClass

    // 발동 필요 개수
    UPROPERTY(EditAnywhere, meta=(ClampMin="2")) int32 RequiredCount = 3;

    // 집계 범위
    UPROPERTY(EditAnywhere) ECardComboWindow Window = ECardComboWindow::ThisTurn;
};

// 콤보 발동 시 보상
USTRUCT(BlueprintType)
struct FCardComboReward
{
    GENERATED_BODY()

    // 추가 데미지 (데미지는 EEffectType이 아니라 별도 경로라 분리)
    UPROPERTY(EditAnywhere, meta=(ClampMin="0")) int32 BonusDamage = 0;

    // 버프/디버프/회복/실드 등 — 기존 FCardEffect 재사용 (에디터 드롭다운 그대로)
    UPROPERTY(EditAnywhere) TArray<FCardEffect> Effects;

    // 보상 대상 (데미지/효과 공통)
    UPROPERTY(EditAnywhere) EEffectTargetType Target = EEffectTargetType::AllEnemies;
};
```

---

## 4. DataTable Row — 디자이너 편집 단위

```cpp
// DT_CardCombos 의 Row Struct (DT_Cards와 동일한 편집 경험)
USTRUCT(BlueprintType)
struct FCardComboRuleRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Combo|Identity") FName  ComboID;
    UPROPERTY(EditAnywhere, Category="Combo|Identity") FText  DisplayName;
    UPROPERTY(EditAnywhere, Category="Combo|Identity") FText  Description;
    UPROPERTY(EditAnywhere, Category="Combo|Identity") TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, Category="Combo|Rule")  FCardComboCondition Condition;
    UPROPERTY(EditAnywhere, Category="Combo|Rule")  FCardComboReward    Reward;
    UPROPERTY(EditAnywhere, Category="Combo|Rule")  ECardComboRepeat    Repeat = ECardComboRepeat::OncePerTurn;

    // 같은 카드로 여러 콤보가 동시 충족될 때 평가 우선순위 (높을수록 먼저)
    UPROPERTY(EditAnywhere, Category="Combo|Rule")  int32 Priority = 0;

    // 비활성 토글 (밸런싱 중 임시 차단)
    UPROPERTY(EditAnywhere, Category="Combo|Rule")  bool bEnabled = true;
};
```

→ 디자이너는 `DT_CardCombos` 에 행을 추가하고, 드롭다운으로 조건·보상을 채우기만 하면 됨. **코드 수정 불필요.**

---

## 5. 평가기 — `UCardComboEvaluator`

CombatManager가 소유하는 얇은 UObject (CombatManager 비대화 방지). 단일 책임: 히스토리 → 콤보 발동.

```cpp
UCLASS()
class UCardComboEvaluator : public UObject
{
    // 초기화 시 DT_CardCombos 로드, CombatManager 참조 보관
    void Initialize(ACombatManager* InOwner, UDataTable* ComboTable);

    // OnActionExecuted 에 바인딩 — 카드 1장 실행될 때마다 호출
    void OnCardPlayed(const FCardDataRow& Card, int32 CasterIndex);

    // 턴 경계에서 ThisTurn 윈도우 / OncePerTurn 정책 리셋
    void OnPlayerTurnStart();

private:
    // 룰별 누적 카운트와 발동 이력
    // (Window별 집계는 ActionQueue를 스캔하거나 증분 카운터로 관리)
    TMap<FName /*ComboID*/, int32> FiredCountThisCombat;
    TMap<FName /*ComboID*/, int32> FiredCountThisTurn;

    // OnComboTriggered 브로드캐스트 (UI 배너/VFX/사운드 트리거)
    // CombatManager에 FOnComboTriggered(ComboID, CasterIndex) 델리게이트 추가
};
```

### 평가 흐름 (OnCardPlayed)
```
1. for each (활성) 룰 R in DT_CardCombos (Priority 내림차순):
2.    Count = 윈도우(R.Window) 안에서 R.Condition 매칭되는 카드 수 집계
         - MatchKey로 키 추출(CardID/CardType/Rarity/JobClass)
         - bRequireSpecificValue면 지정값과 일치하는 것만
         - Consecutive면 "방금 카드 포함 연속 동일 키" 길이
3.    if Count >= R.Condition.RequiredCount AND Repeat 정책 허용:
         - 발동: ApplyReward(R.Reward, CasterIndex)
         - FiredCount 갱신
         - OnComboTriggered.Broadcast(R.ComboID, CasterIndex)
```

### 보상 적용 (ApplyReward)
```
- 대상 해석: Reward.Target(EEffectTargetType) → 실제 유닛 목록
    (ExecuteCard의 타겟 해석 로직 재사용)
- BonusDamage > 0 → 각 대상에 데미지 (ExecuteCard 데미지 경로 재사용)
- Effects[] → EffectType 범위로 ApplyBuff/ApplyDebuff/ApplyEffect 분기 (기존 규칙 동일)
```

### Window 구현 메모
- **WholeCombat**: `ActionQueue` 전체를 스캔(또는 전투 시작 시부터 증분 카운터).
- **ThisTurn**: PlayerActionPhase 진입 시점의 `ActionQueue.Num()`을 마커로 저장 → 그 이후 슬라이스만 집계.
- **Consecutive**: 마지막 키와 현재 키가 같으면 streak++, 다르면 1로 리셋.

---

## 6. 사용 예시 (디자이너 입력)

**예시 A — 같은 카드 3번이면 그 적에게 추가 데미지**
```
ComboID = Triple_Same
Condition: MatchKey=CardID, bRequireSpecificValue=false, RequiredCount=3, Window=ThisTurn
Reward:    BonusDamage=10, Target=SingleEnemy
Repeat:    OncePerTurn
```

**예시 B — 공격 타입 카드 3장이면 전체 적에게 화상 디버프**
```
ComboID = Triple_Attack_Burn
Condition: MatchKey=CardType, bRequireSpecificValue=true, SpecificCardType=Attack,
           RequiredCount=3, Window=ThisTurn
Reward:    Effects=[{ Debuff_Burn, Value=3 }], Target=AllEnemies
Repeat:    OncePerTurn
```

**예시 C — 스킬 카드 누적(전투 내내, 큐 한정) 시 파티 전체 방어막**
```
ComboID = Skill_Stacker
Condition: MatchKey=CardType, bRequireSpecificValue=true, SpecificCardType=Skill, RequiredCount=6
Reward:    Effects=[{ Shield, Value=5, TargetType=AllAllies }]
```

**예시 D — 특정 카드 3종 조합(101+102+103)이면 전체 적 강타** ★카드 ID 지정
```
ComboID = Recipe_Triple
Condition: MatchKey=SpecificCardSet, RequiredCardIDs=[101,102,103], bRequireOrder=false
Reward:    BonusDamage=20, DamageTarget=AllEnemies
```
- `bRequireOrder=true`로 두면 101→102→103을 **연속 순서**로 사용했을 때만 발동.
- `RequiredCardIDs=[101,101]`처럼 같은 ID 중복 기재 시 그만큼(101 두 장) 필요.

---

## 7. 결정 사항 (확정)

| # | 항목 | 결정 |
|---|------|------|
| 1 | 집계 범위 | **전투 전체 누적이되 큐 최근 10장 한정**(슬라이딩 윈도우). → `ECardComboWindow` enum 제거, 고정 윈도우 `ComboHistoryWindow=10` |
| 2 | 파티 공용 vs 시전자별 | **파티 공용** (ActionQueue 전체 합산). 시전자별 옵션 미도입 (나중 필요 시 재논의) |
| 3 | 효과 대상 | **전체 적용** — 데미지/디버프=적 전체, 버프=아군 전체 (`Reward.DamageTarget` 기본 AllEnemies, 효과는 FCardEffect.TargetType) |
| 4 | 발동 연출 | `OnComboTriggered(ComboID, CasterIndex)` 델리게이트만 **지금 추가**. UI 위젯/VFX는 나중에 |
| 5 | 중복 발동 | **허용** — 한 카드로 여러 룰 충족 시 **전부 발동**. → `Priority` 제거 |

### 구조 단순화 결과 (CardComboTypes.h가 구현 소스)
- 제거: `ECardComboWindow`, `ECardComboRepeat`, `Priority`, 시전자별 옵션
- `FCardComboCondition`: `Window` 필드 없음 (항상 최근 10장)
- `FCardComboReward`: `DamageTarget`(기본 AllEnemies) + `Effects[]`(각 효과 TargetType로 대상)
- 같은 룰의 **재발동 정책**: 최근 10장 윈도우에서 조건이 *새로 충족되는 순간*(rising edge)에 1회 발동.
  윈도우에서 카드가 밀려나 조건이 깨졌다가 다시 충족되면 재발동. (매 카드마다 스팸 방지)
- **연속(인접) 조건**: 모든 모드에서 조건 카드가 큐에서 **붙어있어야** 발동한다 (사이에 다른 카드가 끼면 끊김).
  - 같은 카드/타입/희귀도/직업: 같은 키가 *연속으로* RequiredCount장 이어져야 함.
  - SpecificCardSet: 지정 카드들이 *연속 구간*으로 모두 등장해야 함 (bRequireOrder면 순서까지 일치).

---

## 8. 추가 파일 계획 (구현 시)

| 파일 | 내용 |
|------|------|
| `Public/CombatKernel/CardComboTypes.h` | 2·3·4절 열거형/구조체 (Row Struct 포함) |
| `Public/CombatKernel/CardComboEvaluator.h` / `.cpp` | 5절 평가기 |
| CombatManager.h/.cpp | `UCardComboEvaluator` 소유, `OnActionExecuted`/턴 시작에 연결, `OnComboTriggered` 델리게이트 |
| `DT_CardCombos` (에디터) | 디자이너 룰 데이터 |

> 모든 신규 코드는 **CombatKernel 폴더 내** 배치 (기존 작업 규칙 준수).
