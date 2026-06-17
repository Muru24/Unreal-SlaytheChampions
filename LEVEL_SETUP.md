# 레벨 / 전투 셋업 가이드

CombatManager 기반 전투를 **스트리밍 서브레벨** 구조로 굴리기 위한 에디터 설정 정리.
(코드 구조: 퍼시스턴트 MainLevel + 전투 레벨 프리로드 → 하나씩 활성화 → 활성화 시 `BeginCombat`, 전멸 시 `EndCombat`)

---

## 1. 전체 구조

```
MainLevel (퍼시스턴트)
 ├─ NormalMap / EliteMap / BossMap (스트리밍, 프리로드 후 숨김)
 │     └─ 각 레벨에 CombatManager 1개 + PlayerBox/EnemyBox/CameraSlot 배치
 └─ GridMap / ShopLevel / RestLevel ... (CombatManager 없음)

LevelManager(C++)가 레벨을 보이게 함
 → OnLevelShown(BeginPlay 완료) → 그 레벨의 CombatManager.BeginCombat()
 → 전멸 감지 → CombatManager.EndCombat(bWon)
     → BattleMainWidget 제거 + 스폰 유닛 정리 + OnCombatEnded(bWon) 브로드캐스트
 → MainLevel이 OnCombatEnded 구독 → 레벨 숨김 + GridMap 복귀
```

---

## 2. 전투 레벨의 CombatManager 인스턴스 설정 (필수)

각 전투 서브레벨(NormalMap/EliteMap/BossMap 등)에 CombatManager를 1개 배치하고 디테일에서:

| 프로퍼티 | 값 | 비고 |
|---|---|---|
| **bAutoBeginCombat** | **false** | ⚠️ 스트리밍은 반드시 false (LevelManager가 활성화 시 BeginCombat). true면 직접 PIE 테스트용 |
| `EnemyTable` | 적 도감 DataAsset (UEnemyDataTable) | 적 데이터 출처 |
| `StageEncounterTable` | 스테이지 인카운터 DataTable (FStageEncounterRow) | 방 타입별 적 구성 (3-1 참고) |
| `CombatAreaType` | 이 레벨의 방 타입 (Normal/Elite/Boss) | 테이블 행 필터 |
| `EncounterEnemyIDs` | (비움) | 테이블 없을 때만 쓰는 인라인 테스트 목록 |
| `EnemyActorClass` | BP_Enemy | 스폰할 제네릭 적 액터 |
| `PlayerActorClass` | BP_Player | 스폰할 단일 플레이어 액터 |
| `BattleWidgetClass` | WBP_BattleMainWidget | 전투 UI |
| `BattleCameraClass` | BP_BattleCameraActor | 전투 카메라 |
| `PlayerActor_0~2` / `EnemyActor_0~2` | (비움) | 데이터 스폰 시 비움. 채우면 수동 모드 |

**박스/카메라슬롯**: CombatManager의 PlayerBox_0~2 / EnemyBox_0~2 / CameraSlot_* 컴포넌트를 그 레벨 아레나에 맞게 이동·회전.

---

## 3. 적 데이터 (도감)

- **UEnemyDataTable** (도감): `Entries` 배열에 적별 풀스펙(EnemyID, MaxHP, Pattern, Gimmick, StartingEffects, VisualData) 작성.
- 어떤 적이 어느 방에 나오는지는 **StageEncounterTable**에서 결정 (아래 3-1).
- CombatManager가 뽑힌 EnemyID들 → `EnemyTable.FindByID` → BP_Enemy 스폰 후 `InitializeFromDefinition`으로 데이터 주입.

> BP_Enemy의 `EnemyInitializerComponent.Table`은 비워도 됨 (CombatManager가 직접 주입).

---

## 3-1. 스테이지 인카운터 (방 타입별 랜덤 선택)

방 타입(Normal/Elite/Boss)에 따라 인카운터를 **가중치 랜덤**으로 뽑는 방식. DataTable 기반.

**데이터 계층**: `EnemyTable`(도감) → `StageEncounterTable`(등장 규칙 + 적 구성 직접 기재)

**StageEncounterTable** = DataTable, Row 구조체 `FStageEncounterRow`:
| 컬럼 | 의미 |
|---|---|
| `AreaType` | 이 인카운터가 나오는 방 타입 (Normal/Elite/Boss...) |
| `EnemyIDs` | **이 행에서 스폰할 EnemyID 목록** (EnemyTable에 있는 ID, 최대 3, 박스 순서) |
| `Weight` | 가중치 (클수록 자주) — 확률 = 행 Weight ÷ 후보 Weight 합 |
| `MinFloor` / `MaxFloor` | 등장 가능 층 범위 |

> EncounterData 에셋을 거치지 않고 **행에 EnemyID를 직접** 적습니다.

**CombatManager 설정** (레벨별 또는 런타임 주입):
| 프로퍼티 | 의미 |
|---|---|
| `StageEncounterTable` | 위 DataTable 지정 (지정 시 EncounterEnemyIDs 인라인보다 우선) |
| `CombatAreaType` | 이 레벨의 방 타입 (NormalMap=Normal, BossMap=Boss 등) |
| `CombatFloor` | 이 전투의 층 (Min/MaxFloor 필터) |
| `SetStageEncounterTable()` / `SetCombatArea()` | 런타임 주입용 함수 |

**선택 우선순위 (BeginCombat)**:
```
StageEncounterTable(방 타입+층 가중치 랜덤) > EncounterEnemyIDs(인라인 테스트)
```
- 테이블 지정 시 → 방 타입에 맞는 행 중 가중치 랜덤 → 그 행의 EnemyIDs로 스폰
- 미지정 시 → 인라인 EncounterEnemyIDs 폴백 (테스트용)

> 보스 고정(런 시작 시 1개 확정)은 옵션으로 추후 추가 예정 — 현재는 Boss도 풀에서 랜덤.

## 4. 플레이어 데이터 (PartyInstance)

플레이어는 **단일 BP_Player를 직업별로 스폰**. 인스턴스에 직업 목록을 채워두면 됨.

- **데이터 주입** (메뉴/GameMode BeginPlay 등 전투 진입 전):
  ```
  Get Game Instance Subsystem (PartyInstance)
   → Add Champion (Job: Warrior)   // PawnIndex 0
   → Add Champion (Job: Mage)      // PawnIndex 1
  ```
  배열 순서 = PlayerBox 순서 = PawnIndex.
- CombatManager가 `ChampionJobs`마다 BP_Player 스폰 후:
  - `CardUserComponent.JobClass` = 직업 (덱 로드)
  - `JobComponent.SetJobClass(직업)` (직업 로직 Detail 재생성)

> 타이밍: CombatManager의 InitCombat은 한 틱 미뤄 실행되므로, GameMode BeginPlay에서 `Add Champion` 하면 됨.

---

## 5. BP 컴포넌트 요구사항

**BP_Player** (단일, 직업만 바뀜): `CardUserComponent`, `JobComponent`, `StatComponent`, (필요시 UnitAnimComponent 등). 직업 기본값은 무관 — CombatManager가 덮어씀.

**BP_Enemy** (단일, 데이터 주입): `EnemyInitializerComponent`, `StatComponent`, `NPCBrainComponent`, `IntentComponent`, `UnitAnimComponent`, 머리 위 `WidgetComponent`(MonsterActionWidget). Table/EnemyID는 비워둠.

---

## 6. MainLevel (퍼시스턴트) — 전투 종료 처리

각 전투 레벨 CombatManager의 **`OnCombatEnded(bWon)` 델리게이트를 구독**해서:
- 승리 → RunSystem.AreaCleared → 레벨 숨김 + GridMap 복귀
- 패배 → RunFail 처리

> BattleMainWidget 제거·스폰 유닛 정리는 CombatManager.EndCombat이 이미 처리하므로, MainLevel은 **레벨 숨김/복귀**만 담당.

---

## 7. 흐름 요약

1. MainLevel에서 전투 레벨들 프리로드(숨김) — 이 시점엔 BeginPlay 미실행
2. 방 선택 → 전투 레벨 보이게 → `OnLevelShown` → `BeginCombat()`
3. 스폰(플레이어/적) → 위젯 생성 → 턴 시작
4. 전멸 → `CheckCombatEnd` → `EndCombat(bWon)` → 위젯 삭제 + 유닛 정리 + `OnCombatEnded`
5. MainLevel이 받아서 레벨 숨김 + GridMap 복귀
6. 재방문 시 다시 `OnLevelShown` → `BeginCombat` → 깨끗한 새 전투

---

## 8. 빠른 체크리스트

- [ ] 전투 레벨 CombatManager: `bAutoBeginCombat = false`
- [ ] `EnemyTable` / `StageEncounterTable` + `CombatAreaType`(또는 `EncounterEnemyIDs`) / `EnemyActorClass` / `PlayerActorClass` 지정
- [ ] PlayerActor/EnemyActor 슬롯 비움
- [ ] PartyInstance에 전투 진입 전 `Add Champion` 호출
- [ ] BP_Player / BP_Enemy 필수 컴포넌트 확인
- [ ] MainLevel이 `OnCombatEnded` 구독 → 레벨 숨김/복귀
- [ ] 직접 PIE 테스트 시에만 `bAutoBeginCombat = true`
