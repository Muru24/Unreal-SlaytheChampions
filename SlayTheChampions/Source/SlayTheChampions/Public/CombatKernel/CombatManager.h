#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Card/CardDataTypes.h"
#include "Unit/CombatTypes.h"
#include "Map/MapEnum.h"   // EAreaType (스테이지 인카운터 선택)
#include "CombatManager.generated.h"

class UStatComponent;
class AUnit;
class UBoxComponent;
class UArrowComponent;
class ACameraActor;
class UBattleMainWidget;
class UCardComboEvaluator;
class UDataTable;
class UWidgetComponent;
class UEnemyDataTable;
struct FStageEncounterRow;

/**
 * ETurnPhase
 * 한 턴의 진행 순서. DrawPhase -> PlayerActionPhase -> EnemyPhase.
 * SetPhase()로 전환되며, 각 페이즈 진입 시 CheckCombatEnd()가 먼저 실행된다.
 */
UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
	DrawPhase             UMETA(DisplayName = "Draw Phase"),
	PlayerActionPhase     UMETA(DisplayName = "Player Action Phase"),
	EnemyPhase            UMETA(DisplayName = "Enemy Phase"),
};

/**
 * FQueuedAction
 * PlayerActionPhase에서 큐에 쌓이는 카드 실행 요청 하나.
 * ExecuteNextAction()이 순서대로 꺼내 처리한다.
 */
USTRUCT(BlueprintType)
struct FQueuedAction
{
	GENERATED_BODY()

	// 실행할 카드 데이터
	UPROPERTY()
	FCardDataRow Card;

	// 카드를 사용한 플레이어 인덱스 (SpawnedPlayers 기준)
	UPROPERTY()
	int32 CasterIndex = 0;

	// SingleEnemy 카드의 플레이어 지정 타겟 (nullptr이면 기본 0번 적 사용)
	UPROPERTY()
	AUnit* TargetOverride = nullptr;

	// Hand/DiscardPile 조작에 사용할 DataTable Row Name
	// CardID != Row Name인 DataTable 구성에서 DiscardSpecificCard 오류 방지용
	UPROPERTY()
	FName CardRowName;
};

// ── 델리게이트 ─────────────────────────────────────────────────────────
// 페이즈 전환 시 브로드캐스트 (UI 갱신, 코스트 초기화 등에 바인딩)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, ETurnPhase, NewPhase);
// 카드 하나가 실행될 때마다 브로드캐스트 (히스토리 위젯·애니메이션 트리거용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionExecuted, FCardDataRow, Card, int32, CasterIndex);
// EnemyPhase에서 특정 인덱스의 적이 행동을 시작할 때 브로드캐스트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyTurnStart, int32, EnemyIndex);
// 플레이어 유닛 선택 시 브로드캐스트 (BattleCameraActor BP)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattlePlayerSelected, AUnit*, SelectedPlayer);
// 타겟 대기 진입(true)/해제(false) 시 브로드캐스트
// bIsAlly=false -> CameraSlot_Enemy, bIsAlly=true -> CameraSlot_AllPlayers
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetingStateChanged, bool, bIsTargeting, bool, bIsAlly);
// 뒤로가기로 메인 화면 복귀 시 브로드캐스트 (BattleCameraActor BP)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraReturnToDefault);
// 몬스터 페이즈 시작 시 브로드캐스트 (방어도 사라지기 전 — 연출 트리거용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyPhaseStarted);
// 몬스터 1명 행동 완료 후 브로드캐스트 (다음 행동 전 — 연출 트리거용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyActionFinished, int32, EnemyIndex);
// 카드 히스토리 콤보 발동 시 브로드캐스트 (UI 배너·VFX·사운드 트리거용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboTriggered, FName, ComboID, int32, CasterIndex);
// 전투 종료 시 브로드캐스트 (bWon: 승리 여부) — MainLevel/RunSystem이 레벨 숨김·복귀에 사용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatEnded, bool, bWon);

/**
 * ACombatManager
 * 전투 전체를 관리하는 중앙 Actor.
 * 유닛 스폰, 턴 페이즈 전환, 카드 실행, 적 AI 진행 순서를 담당한다.
 * 레벨에 하나만 배치하고 에디터 Details에서 슬롯별 데이터를 설정한다.
 */
UCLASS()
class SLAYTHECHAMPIONS_API ACombatManager : public AActor
{
	GENERATED_BODY()

public:
	ACombatManager();

	// ── 유닛 슬롯 (SetPlayerActor/SetEnemyActor로 설정하거나 에디터에서 직접 지정) ──
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	AUnit* PlayerActor_0;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	AUnit* PlayerActor_1;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	AUnit* PlayerActor_2;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	AUnit* EnemyActor_0;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	AUnit* EnemyActor_1;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	AUnit* EnemyActor_2;

	// 전투 화면 메인 위젯 Blueprint 클래스 (InitCombat에서 자동 생성·AddToViewport)
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	TSubclassOf<UBattleMainWidget> BattleWidgetClass;

	// 전투 카메라 Blueprint 클래스 (nullptr이면 카메라 변경 없음)
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	TSubclassOf<ACameraActor> BattleCameraClass;

	// 적 도감(EnemyID별 풀스펙을 담은 데이터 에셋) — 등장 적 선택은 StageEncounterTable/EncounterEnemyIDs가 결정.
	// 뽑힌 EnemyID로 여기서 정의를 찾아 EnemyActorClass를 스폰 후 데이터를 주입한다.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	UEnemyDataTable* EnemyTable = nullptr;

	// 스테이지 테이블 없이 빠르게 테스트할 때 쓰는 인라인 EnemyID 목록 (최대 3)
	// StageEncounterTable이 지정돼 행이 뽑히면 무시됨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Setup")
	TArray<FName> EncounterEnemyIDs;

	// 스테이지 인카운터 테이블 (행: FStageEncounterRow) — 방 타입/층에 맞는 인카운터를 가중치 랜덤 선택.
	// 레벨별로 지정하거나 SetStageEncounterTable로 런타임 주입. 지정 시 EncounterEnemyIDs(인라인)보다 우선.
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	UDataTable* StageEncounterTable = nullptr;

	// 이 전투의 방 타입 — StageEncounterTable 행 필터에 사용 (레벨별 설정 또는 런타임 주입)
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	EAreaType CombatAreaType = EAreaType::Normal;

	// 이 전투의 층 — StageEncounterTable의 Min/MaxFloor 필터 (레벨별 설정 또는 런타임 주입)
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	int32 CombatFloor = 0;

	// 런타임에 스테이지 테이블 주입 (RunSystem/레벨이 BeginCombat 전에 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Combat|Setup")
	void SetStageEncounterTable(UDataTable* InTable) { StageEncounterTable = InTable; }

	// 런타임에 방 타입/층 주입
	UFUNCTION(BlueprintCallable, Category = "Combat|Setup")
	void SetCombatArea(EAreaType InAreaType, int32 InFloor) { CombatAreaType = InAreaType; CombatFloor = InFloor; }

	// EnemyTable 경로로 스폰할 제네릭 적 액터 클래스 (EnemyInitializerComponent를 가진 BP_Enemy)
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	TSubclassOf<AUnit> EnemyActorClass;

	// PartyInstance.ChampionJobs 경로로 스폰할 단일 플레이어 액터 클래스 (BP_Player)
	// 스폰 후 직업(JobComponent·CardUserComponent의 JobClass)만 챔피언별로 주입
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	TSubclassOf<AUnit> PlayerActorClass;

	// true면 BeginPlay에서 자동으로 BeginCombat 호출 (직접 플레이/테스트 레벨용).
	// 스트리밍으로 프리로드되는 전투 레벨은 false로 두고, LevelManager가 활성화(OnLevelShown) 시 BeginCombat 호출.
	UPROPERTY(EditAnywhere, Category = "Combat|Setup")
	bool bAutoBeginCombat = true;

	// ── 스폰 수 (런타임 출력) ─────────────────────────────────────
	// InitCombat에서 실제 스폰된 유닛 수로 갱신됨 — 에디터 입력값이 아니라 결과값(읽기 전용)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	int32 PlayerCount = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Setup")
	int32 EnemyCount = 1;

	// ── 스폰 위치 박스 ────────────────────────────────────────────
	// 에디터에서 직접 이동하여 스폰 위치를 조정하는 BoxComponent 슬롯들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Slots")
	UBoxComponent* PlayerBox_0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Slots")
	UBoxComponent* PlayerBox_1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Slots")
	UBoxComponent* PlayerBox_2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Slots")
	UBoxComponent* EnemyBox_0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Slots")
	UBoxComponent* EnemyBox_1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Slots")
	UBoxComponent* EnemyBox_2;

	// ── 카메라 슬롯 (화살표 방향 = 카메라가 바라보는 방향) ───────
	// 에디터에서 직접 이동·회전하여 각 상황의 카메라 위치·시선을 조정
	// Default: 전투 시작 시 초기 위치 (흰색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|CameraSlots")
	UArrowComponent* CameraSlot_Default;

	// Player_0: 0번 플레이어 선택 시 이동할 위치 (파란색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|CameraSlots")
	UArrowComponent* CameraSlot_Player_0;

	// Player_1: 1번 플레이어 선택 시 이동할 위치 (하늘색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|CameraSlots")
	UArrowComponent* CameraSlot_Player_1;

	// Player_2: 2번 플레이어 선택 시 이동할 위치 (초록색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|CameraSlots")
	UArrowComponent* CameraSlot_Player_2;

	// Enemy: 적 타겟 지정 시 이동할 위치 (빨간색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|CameraSlots")
	UArrowComponent* CameraSlot_Enemy;

	// AllPlayers: 아군 타겟 지정 시 이동할 위치 (보라색)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|CameraSlots")
	UArrowComponent* CameraSlot_AllPlayers;

	// ── 딜레이 설정 ──────────────────────────────────────────────
	// 적 한 명 행동 후 다음 적까지 대기 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Combat|Timing", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float EnemyActionDelay = 0.8f;

	// ── 카메라 상태 ──────────────────────────────────────────────
	// 카메라 이동 중 여부 — BP_BattleCameraActor가 Timeline 시작/종료 시 SET
	// HandleBackClicked에서 이동 중 입력 차단에 사용
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	bool bCameraMoving = false;

	// ── 턴 상태 ───────────────────────────────────────────────────
	// 현재 턴 페이즈 (읽기 전용 — SetPhase()로만 변경)
	UPROPERTY(BlueprintReadOnly, Category = "Turn")
	ETurnPhase CurrentPhase = ETurnPhase::DrawPhase;

	// 현재 턴 번호 (1부터 시작)
	UPROPERTY(BlueprintReadOnly, Category = "Turn")
	int32 TurnCount = 0;

	// ── 턴 델리게이트 ─────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "Turn")
	FOnPhaseChanged OnPhaseChanged;

	// 플레이어 액션 하나 실행됐을 때 (애니메이션 트리거용)
	UPROPERTY(BlueprintAssignable, Category = "Turn")
	FOnActionExecuted OnActionExecuted;

	// 적 행동 시작 (EnemyIndex번 적 행동할 차례)
	UPROPERTY(BlueprintAssignable, Category = "Turn")
	FOnEnemyTurnStart OnEnemyTurnStart;

	// 플레이어 유닛 선택 시 (BattleCameraActor BP에서 바인딩)
	UPROPERTY(BlueprintAssignable, Category = "Camera")
	FOnBattlePlayerSelected OnBattlePlayerSelected;

	// 타겟 대기 진입/해제 시 (BattleCameraActor BP에서 바인딩)
	// bIsAlly=false -> CameraSlot_Enemy, bIsAlly=true -> CameraSlot_AllPlayers
	UPROPERTY(BlueprintAssignable, Category = "Camera")
	FOnTargetingStateChanged OnTargetingStateChanged;

	// 뒤로가기 버튼으로 메인 화면 복귀 시 (BattleCameraActor BP에서 바인딩)
	UPROPERTY(BlueprintAssignable, Category = "Camera")
	FOnCameraReturnToDefault OnCameraReturnToDefault;

	// 몬스터 페이즈 시작 시 — 방어도 리셋 전 (연출·UI 트리거용)
	UPROPERTY(BlueprintAssignable, Category = "Turn")
	FOnEnemyPhaseStarted OnEnemyPhaseStarted;

	// 몬스터 1명 행동 완료 후 — 다음 행동 전 (연출·UI 트리거용)
	UPROPERTY(BlueprintAssignable, Category = "Turn")
	FOnEnemyActionFinished OnEnemyActionFinished;

	// 카드 히스토리 콤보 발동 시 (UI 배너·VFX 트리거용)
	UPROPERTY(BlueprintAssignable, Category = "Combo")
	FOnComboTriggered OnComboTriggered;

	// 전투 종료 시 (bWon) — MainLevel/RunSystem이 구독해 레벨 숨김·복귀 처리
	UPROPERTY(BlueprintAssignable, Category = "Turn")
	FOnCombatEnded OnCombatEnded;

	// ── 유닛 슬롯 설정 함수 ──────────────────────────────────────
	// 테스트용: 수동으로 슬롯 지정 시 PartyInstance/EnemyTable 자동 로드를 무시
	UFUNCTION(BlueprintCallable, Category = "Combat|Setup")
	void SetPlayerActor(int32 Index, AUnit* Actor);

	UFUNCTION(BlueprintCallable, Category = "Combat|Setup")
	void SetEnemyActor(int32 Index, AUnit* Actor);

	// ── 전투 생명주기 ─────────────────────────────────────────────
	// 레벨 활성화 시 호출 — 상태 리셋 후 전투 초기화(스폰·위젯·턴 시작).
	// LevelManager가 OnLevelShown에서, 또는 bAutoBeginCombat=true면 BeginPlay에서 호출.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginCombat();

	// 전투 종료 — BattleMainWidget 제거 + 이 매니저가 스폰한 유닛 정리 + OnCombatEnded 브로드캐스트.
	// CheckCombatEnd가 전멸을 감지하면 자동 호출. 외부에서 강제 종료에도 사용 가능.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndCombat(bool bWon);

	// 내부 전투 초기화 (BeginCombat이 리셋 후 호출). 직접 호출 비권장.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void InitCombat();

	// ── 카드 효과 실행 ────────────────────────────────────────────
	// 카드 데이터로 타겟을 결정하고 데미지·회복·Shield 효과를 적용
	// TargetOverride가 설정된 경우 SingleEnemy 타입에서 해당 유닛을 타겟으로 사용
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteCard(const FCardDataRow& Card, int32 CasterIndex, AUnit* TargetOverride = nullptr);

	// ── 턴 함수 ───────────────────────────────────────────────────
	// DrawPhase 시작: Shield 리셋 -> 버프/디버프 tick -> PlayerActionPhase
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void StartTurn();

	// 이번 턴 사용 카드를 ActionHistory에 기록 — 효과 실행은 BattleMainWidget::QueueCardAction에서 즉시 처리됨
	// CardRowName — 기록 식별용 Row Name (NAME_None이면 Card.CardID 사용)
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void QueuePlayerAction(const FCardDataRow& Card, int32 CasterIndex, FName CardRowName = NAME_None, AUnit* TargetOverride = nullptr);

	// 카드 효과(ExecuteCard) 적용 후 콤보 조건 평가 — QueueCardAction이 ExecuteCard 다음에 호출
	void EvaluatePlayedCardCombos(int32 CasterIndex);

	// PlayerActionPhase 종료 -> EnemyPhase로 직접 전환 (카드 효과는 이미 즉시 실행됨)
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void EndPlayerActionPhase();

	// 이번 턴 사용한 카드 기록 반환 (사용 순서대로)
	// 카드 사용 히스토리 반환 (최대 10개, 턴 초기화 없음)
	UFUNCTION(BlueprintPure, Category = "Turn")
	const TArray<FQueuedAction>& GetActionHistory() const { return ActionQueue; }

	// 적 행동 완료 후 호출 -> 다음 적으로 인덱스 전진
	UFUNCTION(BlueprintCallable, Category = "Turn")
	void OnEnemyActionComplete();

	// ── 스탯 조회 ─────────────────────────────────────────────────
	// Index번 플레이어의 StatComponent 반환 (없으면 nullptr)
	UFUNCTION(BlueprintPure, Category = "Combat")
	UStatComponent* GetPlayerStat(int32 Index) const;

	// Index번 적의 StatComponent 반환 (없으면 nullptr)
	UFUNCTION(BlueprintPure, Category = "Combat")
	UStatComponent* GetEnemyStat(int32 Index) const;

	// 스폰된 플레이어 유닛 목록 반환 (BattleMainWidget 등 외부 시스템에서 참조용)
	UFUNCTION(BlueprintPure, Category = "Combat")
	const TArray<AUnit*>& GetSpawnedPlayers() const { return SpawnedPlayers; }

	// 스폰된 적 유닛 목록 반환 (타겟 선택 등 외부 시스템에서 참조용)
	UFUNCTION(BlueprintPure, Category = "Combat")
	const TArray<AUnit*>& GetSpawnedEnemies() const { return SpawnedEnemies; }

	// InitCombat에서 생성된 BattleMainWidget 반환 (BattleCameraActor BP 등 외부에서 참조용)
	UFUNCTION(BlueprintPure, Category = "Combat")
	UBattleMainWidget* GetBattleWidget() const { return BattleWidget; }

	// InitCombat에서 스폰된 BattleCamera 반환 (BattleCameraActor BP 등 외부에서 참조용)
	UFUNCTION(BlueprintPure, Category = "Combat")
	ACameraActor* GetBattleCamera() const { return BattleCamera; }

protected:
	virtual void BeginPlay() override;

	// 매 프레임 적 행동 위젯(머리 위 WidgetComponent)을 카메라를 향해 회전(빌보드)
	virtual void Tick(float DeltaSeconds) override;

private:
	// 스폰된 플레이어 유닛 목록
	UPROPERTY()
	TArray<AUnit*> SpawnedPlayers;

	// 스폰된 적 유닛 목록
	UPROPERTY()
	TArray<AUnit*> SpawnedEnemies;

	// 카드 사용 히스토리 (최대 10개, 전투 중 초기화 없음)
	UPROPERTY()
	TArray<FQueuedAction> ActionQueue;

	// 카드 히스토리 콤보 룰 DataTable (DT_CardCombos) — 에디터에서 인스턴스에 지정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Combo", meta = (AllowPrivateAccess = "true"))
	UDataTable* ComboTable = nullptr;

	// 콤보 평가기 — BeginPlay에서 생성, QueuePlayerAction마다 평가
	UPROPERTY()
	UCardComboEvaluator* ComboEvaluator = nullptr;

	// EnemyPhase에서 현재 행동 중인 적 인덱스
	int32 CurrentEnemyIndex = 0;

	// SetPlayerActor/SetEnemyActor 호출 시 true — PartyInstance·EnemyTable 자동 로드를 무시
	bool bPlayerManualSet = false;
	bool bEnemyManualSet = false;

	// InitCombat 재진입 차단 — 스폰된 적(BP_Enemy)의 BeginPlay가 InitCombat을 재호출해
	// 무한 재귀로 적을 계속 스폰하는 문제를 막는다 (가드는 스폰 전에 세움)
	bool bCombatInitialized = false;

	// 전투 종료 1회 가드 — CheckCombatEnd가 여러 번 불려도 EndCombat은 한 번만, 종료 후 턴 진행 차단
	bool bCombatEnded = false;

	// 이 CombatManager가 스폰한 유닛 (EndCombat에서 Destroy) — 레벨에 직접 배치한 유닛은 포함하지 않음
	UPROPERTY()
	TArray<AUnit*> ManagerSpawnedUnits;

	// 적 행동 딜레이 타이머
	FTimerHandle EnemyTimerHandle;

	// InitCombat에서 생성한 배틀 메인 위젯 인스턴스
	UPROPERTY()
	UBattleMainWidget* BattleWidget = nullptr;

	// InitCombat에서 스폰한 배틀 카메라 액터 인스턴스
	UPROPERTY()
	ACameraActor* BattleCamera = nullptr;

	// 카메라를 향해 빌보드할 적 행동 위젯 컴포넌트 캐시 (InitCombat에서 수집, Tick에서 회전)
	UPROPERTY()
	TArray<UWidgetComponent*> EnemyActionWidgetComps;

	// 페이즈를 전환하고 CheckCombatEnd -> OnPhaseChanged 브로드캐스트
	void SetPhase(ETurnPhase NewPhase);
	// 전투 종료 조건(전멸) 확인 — 전멸 감지 시 EndCombat 호출
	void CheckCombatEnd();
	// 지정 유닛의 Shield만 0으로 리셋
	void ApplyShieldReset(const TArray<AUnit*>& Units);
	// Shield 제외 버프/디버프 tick (Regen 회복, Burn 데미지, 시간제 스택 감소)
	void TickBuffsAndDebuffs(const TArray<AUnit*>& Units);
	// EnemyPhase를 시작하고 첫 번째 살아있는 적부터 행동을 진행
	void StartEnemyPhase();
	// 죽은 적을 건너뛰며 CurrentEnemyIndex 적이 행동하도록 처리
	void ExecuteNextEnemyAction();

	// DrawPhase에 살아있는 모든 적의 NPCBrainComponent::PlanNextAction을 호출
	void PlanAllEnemyActions();
	// 적 한 명의 FEnemyAction을 실행 (Attack -> ProcessDamage, Defend -> Shield 부여)
	void ExecuteEnemyAction(AUnit* Caster, const FEnemyAction& Action);

	// 스폰 위치 BoxComponent를 생성하고 루트에 부착하는 헬퍼
	UBoxComponent* SetupBox(const FName& BoxName, const FVector& RelativeLocation, const FColor& Color);

	// StageEncounterTable에서 CombatAreaType/CombatFloor에 맞는 행을 가중치 랜덤으로 선택 (없으면 nullptr)
	// 반환된 행의 EnemyIDs로 적을 스폰한다.
	// TODO: Boss 타입은 런 시작 시 1개 확정 옵션을 추후 추가 (지금은 보스도 랜덤)
	const FStageEncounterRow* PickEncounterFromTable() const;
};