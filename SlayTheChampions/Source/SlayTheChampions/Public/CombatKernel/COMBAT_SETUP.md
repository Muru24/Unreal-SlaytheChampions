# CombatKernel 전투 세팅 튜토리얼

> 마지막 업데이트: 2026-06-02

---

## 목차
1. [전체 흐름](#전체-흐름)
2. [플레이어 세팅](#플레이어-세팅)
3. [몬스터 세팅](#몬스터-세팅)
4. [레벨 블루프린트 세팅](#레벨-블루프린트-세팅)
5. [StatManager 위젯 세팅](#statmanager-위젯-세팅)
6. [테스트 모드](#테스트-모드)

---

## 전체 흐름

```
레벨 로드
├── BP_TestPlayer BeginPlay → RegisterChampion(Self)   ← 자동
└── 레벨 블루프린트 BeginPlay
      → CombatManager::SetMonsterGroup(DataAsset)
      → CombatManager::InitCombat()
            ├── PartyInstance.Champions에서 플레이어 자동 로드
            ├── MonsterGroupData 기반 몬스터 스폰
            └── BattleMainWidget 생성 (StatManager 포함)
```

---

## 플레이어 세팅

### 1. BP_TestPlayer — WidgetComponent 삭제
기존에 붙어 있던 CombatStatWidget WidgetComponent를 삭제한다.
StatWidget은 이제 BattleMainWidget의 StatManager에서 관리한다.

### 2. BP_TestPlayer — BeginPlay 설정
```
Event BeginPlay
  → Get Game Instance → Get Subsystem (PartyInstance)
  → Register Champion (Target: PartyInstance, Unit: Self)
```

- 레벨에 배치된 플레이어 액터가 BeginPlay에서 자신을 PartyInstance에 등록한다
- InitCombat 시점에 PartyInstance.Champions를 읽어 자동으로 슬롯에 채운다
- 플레이어 최대 3명 지원 (PlayerActor_0 / 1 / 2)

### 3. 플레이어 스탯 설정
레벨에 배치된 BP_TestPlayer 선택 → Details → StatComponent
- `Max HP` 직접 입력
- CombatManager가 덮어쓰지 않으므로 각 액터에서 개별 설정

---

## 몬스터 세팅

### 1. MonsterGroupData 에셋 생성
```
Content Browser → 우클릭 → Miscellaneous → Data Asset → MonsterGroupData
```

### 2. MonsterGroupData 설정

| 항목 | 설명 |
|------|------|
| `MonsterClass` | 스폰할 몬스터 Blueprint 클래스 |
| `UnitName` | 이름 (CombatStatWidget Text_Name에 표시) |
| `MaxHP` | 최대 HP |

**예시**
```
Monsters[0]
  MonsterClass → BP_TestEnemy
  UnitName     → Slime
  MaxHP        → 80

Monsters[1]
  MonsterClass → BP_TestEnemy2
  UnitName     → Goblin
  MaxHP        → 60
```

- `Monsters` 배열 크기가 자동으로 EnemyCount를 결정 (최대 3개)
- 스폰 위치는 CombatManager의 EnemyBox_0 / 1 / 2 기준

### 3. 스폰 위치 조정
레벨의 CombatManager 액터 선택 → 뷰포트에서 빨간 박스(EnemyBox_0/1/2) 위치 이동

---

## 레벨 블루프린트 세팅

```
Event BeginPlay
  → Get Actor Of Class (BP_CombatManager)
  → Set Monster Group (DA_MonsterGroup_방이름)
  → Init Combat
```

- `SetMonsterGroup`으로 이번 전투의 몬스터 구성을 지정
- `InitCombat` 호출 전에 SetMonsterGroup이 먼저 실행되어야 함
- 플레이어는 각자 BeginPlay에서 자동 등록되므로 별도 설정 불필요

---

## StatManager 위젯 세팅

### WBP_CombatantStat 디자이너
- TextBlock `Text_HP` (BindWidget)
- TextBlock `Text_Defence` (BindWidget)
- TextBlock `Text_Name` (BindWidgetOptional) ← 유닛 이름 표시

### WBP_StatManager 디자이너
```
Canvas Panel (좌측 앵커)
└── Vertical Box
    ├── Vertical Box "EnemyStatContainer"   (Is Variable 체크)
    └── Vertical Box "PlayerStatContainer"  (Is Variable 체크)
```

### WBP_StatManager — Event Construct
```
[적]
Get Actor Of Class (CombatManager) → Get Spawned Enemies
→ For Each Loop
  → Create WBP_CombatantStat (Owning Player 연결)
  → EnemyStatContainer → Add Child
  → Init From Unit (In Unit = Array Element)

[플레이어]
Get Actor Of Class (CombatManager) → Get Spawned Players
→ For Each Loop
  → Create WBP_CombatantStat (Owning Player 연결)
  → PlayerStatContainer → Add Child
  → Init From Unit (In Unit = Array Element)
```

### WBP_BattleMainWidget 디자이너
- `WBP_StatManager` 좌측에 배치

---

## 테스트 모드

SetPlayerActor 또는 SetEnemyActor를 호출하면 자동 로드를 무시하고 수동 값을 사용한다.
활성화 시 화면 좌상단에 노란색으로 표시된다.

```
[테스트 세팅 사용중] Player:수동  Enemy:자동
```

### SetPlayerActor (테스트용)
```
CombatManager → Set Player Actor (Index=0, Actor=SomePlayer)
CombatManager → Set Player Count (1)
CombatManager → Init Combat
```
- PartyInstance.Champions 자동 로드를 무시
- Index: 0~2

### SetEnemyActor (테스트용)
```
CombatManager → Set Enemy Actor (Index=0, Actor=SomeEnemy)
CombatManager → Set Enemy Count (1)
CombatManager → Init Combat
```
- MonsterGroupData 스폰을 무시하고 레벨에 미리 배치된 액터 사용
- Index: 0~2

---

## CombatManager Details 항목 요약

| 항목 | 카테고리 | 설명 |
|------|----------|------|
| `MonsterGroup` | Combat\|Setup | 이번 전투 몬스터 데이터 에셋 |
| `PlayerActor_0/1/2` | Combat\|Setup | 테스트용 플레이어 슬롯 |
| `EnemyActor_0/1/2` | Combat\|Setup | 테스트용 적 슬롯 |
| `PlayerCount` | Combat\|Setup | 참여 플레이어 수 (1~3) |
| `EnemyCount` | Combat\|Setup | 참여 적 수 (1~3, MonsterGroup 사용 시 무시됨) |
| `BattleWidgetClass` | Combat\|Setup | 배틀 메인 위젯 BP 클래스 |
| `BattleCameraClass` | Combat\|Setup | 배틀 카메라 BP 클래스 |
| `EnemyActionDelay` | Combat\|Timing | 적 행동 간격 (초) |
| `EnemyBox_0/1/2` | Combat\|Slots | 몬스터 스폰 위치 박스 |
| `PlayerBox_0/1/2` | Combat\|Slots | 플레이어 위치 박스 (참고용) |
| `CameraSlot_*` | Combat\|CameraSlots | 상황별 카메라 위치 화살표 |
