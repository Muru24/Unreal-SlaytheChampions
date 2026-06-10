#include "CombatKernel/CombatManager.h"
#include "CombatKernel/EffectManager.h"
#include "CombatKernel/BattleMainWidget.h"
#include "CombatKernel/CardComboEvaluator.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Unit/Unit.h"
#include "Unit/StatComponent.h"
#include "Unit/StatusEffectComponent.h"
#include "Unit/StatusEffect.h"
#include "Unit/Enemy/NPCBrainComponent.h"
#include "Unit/Enemy/IntentComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Card/CardUserComponent.h"  // 스폰된 플레이어에 PawnIndex 주입 및 드로우 호출용
#include "Party/PartyInstance.h"
#include "EngineUtils.h"

// 생성자: 스폰 위치 박스 컴포넌트들을 미리 배치
ACombatManager::ACombatManager()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// 플레이어 박스 (파란색) - 에디터에서 직접 이동하여 스폰 위치 조정
	PlayerBox_0 = SetupBox(TEXT("PlayerBox_0"), FVector(   0.f, -300.f, 0.f), FColor::Blue);
	PlayerBox_1 = SetupBox(TEXT("PlayerBox_1"), FVector(-150.f, -300.f, 0.f), FColor::Blue);
	PlayerBox_2 = SetupBox(TEXT("PlayerBox_2"), FVector( 150.f, -300.f, 0.f), FColor::Blue);

	// 적 박스 (빨간색) - 에디터에서 직접 이동하여 스폰 위치 조정
	EnemyBox_0 = SetupBox(TEXT("EnemyBox_0"), FVector(   0.f, 300.f, 0.f), FColor::Red);
	EnemyBox_1 = SetupBox(TEXT("EnemyBox_1"), FVector(-150.f, 300.f, 0.f), FColor::Red);
	EnemyBox_2 = SetupBox(TEXT("EnemyBox_2"), FVector( 150.f, 300.f, 0.f), FColor::Red);

	// 카메라 슬롯 (화살표 = 카메라 방향) - 에디터에서 이동·회전하여 위치·시선 조정
	// Default: 흰색 — 전투 시작 초기 위치
	CameraSlot_Default = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraSlot_Default"));
	CameraSlot_Default->SetupAttachment(GetRootComponent());
	CameraSlot_Default->SetRelativeLocation(FVector(-500.f, 0.f, 200.f));
	CameraSlot_Default->ArrowColor = FColor::White;
	CameraSlot_Default->SetHiddenInGame(true);

	// Player_0: 파란색 — 0번 플레이어 선택 시 이동 위치
	CameraSlot_Player_0 = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraSlot_Player_0"));
	CameraSlot_Player_0->SetupAttachment(GetRootComponent());
	CameraSlot_Player_0->SetRelativeLocation(FVector(-500.f, -200.f, 200.f));
	CameraSlot_Player_0->ArrowColor = FColor::Blue;
	CameraSlot_Player_0->SetHiddenInGame(true);

	// Player_1: 하늘색 — 1번 플레이어 선택 시 이동 위치
	CameraSlot_Player_1 = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraSlot_Player_1"));
	CameraSlot_Player_1->SetupAttachment(GetRootComponent());
	CameraSlot_Player_1->SetRelativeLocation(FVector(-500.f, -350.f, 200.f));
	CameraSlot_Player_1->ArrowColor = FColor::Cyan;
	CameraSlot_Player_1->SetHiddenInGame(true);

	// Player_2: 초록색 — 2번 플레이어 선택 시 이동 위치
	CameraSlot_Player_2 = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraSlot_Player_2"));
	CameraSlot_Player_2->SetupAttachment(GetRootComponent());
	CameraSlot_Player_2->SetRelativeLocation(FVector(-500.f, -500.f, 200.f));
	CameraSlot_Player_2->ArrowColor = FColor::Green;
	CameraSlot_Player_2->SetHiddenInGame(true);

	// Enemy: 빨간색 — 적 타겟 지정 시 이동 위치 (적 앞)
	CameraSlot_Enemy = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraSlot_Enemy"));
	CameraSlot_Enemy->SetupAttachment(GetRootComponent());
	CameraSlot_Enemy->SetRelativeLocation(FVector(-500.f, 300.f, 200.f));
	CameraSlot_Enemy->ArrowColor = FColor::Red;
	CameraSlot_Enemy->SetHiddenInGame(true);

	// AllPlayers: 보라색 — 아군 타겟 지정 시 이동 위치 (플레이어 전체가 보이는 위치)
	CameraSlot_AllPlayers = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraSlot_AllPlayers"));
	CameraSlot_AllPlayers->SetupAttachment(GetRootComponent());
	CameraSlot_AllPlayers->SetRelativeLocation(FVector(-600.f, -200.f, 250.f));
	CameraSlot_AllPlayers->ArrowColor = FColor(128, 0, 128); // 보라색
	CameraSlot_AllPlayers->SetHiddenInGame(true);
}

// 스폰 위치 박스 컴포넌트 하나를 생성하고 루트에 부착하는 헬퍼 함수
UBoxComponent* ACombatManager::SetupBox(const FName& BoxName,
                                         const FVector& RelativeLocation,
                                         const FColor& Color)
{
	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(BoxName);
	Box->SetupAttachment(GetRootComponent());
	Box->SetRelativeLocation(RelativeLocation);
	Box->SetBoxExtent(FVector(40.f, 40.f, 80.f));
	Box->ShapeColor = Color;
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetHiddenInGame(true);
	return Box;
}

void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	// 콤보 평가기 생성 (전투마다 새로 생성되어 상태 초기화됨). ComboTable 미지정 시에도 안전
	ComboEvaluator = NewObject<UCardComboEvaluator>(this);
	ComboEvaluator->Initialize(this, ComboTable);

	InitCombat();
}

// Index번 플레이어 슬롯에 액터 설정 — PartyInstance 자동 로드를 무시하고 이 값을 사용
void ACombatManager::SetPlayerActor(int32 Index, AUnit* Actor)
{
	bPlayerManualSet = true;
	switch (Index)
	{
		case 0: PlayerActor_0 = Actor; break;
		case 1: PlayerActor_1 = Actor; break;
		case 2: PlayerActor_2 = Actor; break;
		default: UE_LOG(LogTemp, Warning, TEXT("[CombatManager] SetPlayerActor: 유효하지 않은 인덱스 %d"), Index); break;
	}
}

// Index번 적 슬롯에 액터 설정 — MonsterGroupData 자동 로드를 무시하고 이 값을 사용
void ACombatManager::SetEnemyActor(int32 Index, AUnit* Actor)
{
	bEnemyManualSet = true;
	switch (Index)
	{
		case 0: EnemyActor_0 = Actor; break;
		case 1: EnemyActor_1 = Actor; break;
		case 2: EnemyActor_2 = Actor; break;
		default: UE_LOG(LogTemp, Warning, TEXT("[CombatManager] SetEnemyActor: 유효하지 않은 인덱스 %d"), Index); break;
	}
}

// 등록된 유닛들로 전투를 초기화하고 1턴을 시작
void ACombatManager::InitCombat()
{
	// 이미 초기화됨 — BP BeginPlay 중복 호출 방지
	if (BattleWidget) return;

	SpawnedPlayers.Empty();
	SpawnedEnemies.Empty();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	// ── 1. 배틀 카메라 스폰 ──────────────────────────────────
	// CameraSlot_Default 화살표 위치·회전으로 스폰 — 에디터에서 화살표를 이동·회전해 초기 위치 조정
	if (BattleCameraClass)
	{
		const FTransform SpawnTransform = CameraSlot_Default
			? CameraSlot_Default->GetComponentTransform()
			: GetActorTransform();

		BattleCamera = GetWorld()->SpawnActor<ACameraActor>(BattleCameraClass, SpawnTransform);
		if (BattleCamera && PC)
			PC->SetViewTargetWithBlend(BattleCamera);
		else
			UE_LOG(LogTemp, Error, TEXT("[CombatManager] BattleCamera spawn failed"));
	}

	// ── 3. 플레이어 슬롯 로드 ────────────────────────────────────
	// bPlayerManualSet이면 Set 함수로 지정된 값 그대로 사용 (PartyInstance 무시)
	if (!bPlayerManualSet && !PlayerActor_0)
	{
		// 1순위: PartyInstance.Champions
		UGameInstance* GI = GetWorld()->GetGameInstance();
		UPartyInstance* Party = GI ? GI->GetSubsystem<UPartyInstance>() : nullptr;
		if (Party && Party->GetPartyInfo().Champions.Num() > 0)
		{
			const TArray<AUnit*>& Champions = Party->GetPartyInfo().Champions;
			PlayerCount = FMath::Clamp(Champions.Num(), 1, 3);
			AUnit** PlayerSlots[] = { &PlayerActor_0, &PlayerActor_1, &PlayerActor_2 };
			for (int32 i = 0; i < PlayerCount; i++)
				*PlayerSlots[i] = Champions[i];
			UE_LOG(LogTemp, Log, TEXT("[CombatManager] PartyInstance에서 플레이어 %d명 로드"), PlayerCount);
		}
		else
		{
			// 2순위: 레벨에 배치된 Team==Ally AUnit 자동 탐색
			AUnit** PlayerSlots[] = { &PlayerActor_0, &PlayerActor_1, &PlayerActor_2 };
			int32 Idx = 0;
			for (TActorIterator<AUnit> It(GetWorld()); It && Idx < 3; ++It)
			{
				if (It->Team == ETeam::Ally)
					*PlayerSlots[Idx++] = *It;
			}
			if (Idx > 0)
			{
				PlayerCount = Idx;
				UE_LOG(LogTemp, Log, TEXT("[CombatManager] 레벨에서 플레이어 %d명 자동 탐색"), Idx);
			}
		}
	}

	AUnit* PlayerActorArr[] = { PlayerActor_0, PlayerActor_1, PlayerActor_2 };

	const int32 ClampedPlayerCount = FMath::Clamp(PlayerCount, 1, 3);
	for (int32 i = 0; i < ClampedPlayerCount; i++)
	{
		AUnit* Actor = PlayerActorArr[i];
		if (!Actor) { UE_LOG(LogTemp, Error, TEXT("[CombatManager] PlayerActor_%d 미설정"), i); continue; }

		SpawnedPlayers.Add(Actor);

		UCardUserComponent* CardComp = Actor->FindComponentByClass<UCardUserComponent>();
		if (CardComp)
		{
			CardComp->PawnIndex = i;
			CardComp->InitializeDeck();
		}
		UE_LOG(LogTemp, Log, TEXT("[CombatManager] Player[%d] 등록: %s"), i, *Actor->GetName());
	}

	// ── 4. 적 유닛 등록 ──────────────────────────────────────────
	// bEnemyManualSet이면 MonsterGroupData 무시하고 EnemyActor 슬롯 그대로 사용
	if (!bEnemyManualSet && MonsterGroup && MonsterGroup->Monsters.Num() > 0)
	{
		// 데이터 에셋 기준으로 스폰
		UBoxComponent* EnemyBoxes[] = { EnemyBox_0, EnemyBox_1, EnemyBox_2 };
		const int32 Count = FMath::Min(MonsterGroup->Monsters.Num(), 3);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		for (int32 i = 0; i < Count; i++)
		{
			const FMonsterSlotData& Slot = MonsterGroup->Monsters[i];
			if (!Slot.MonsterClass || !EnemyBoxes[i]) continue;

			AUnit* Actor = GetWorld()->SpawnActor<AUnit>(Slot.MonsterClass, EnemyBoxes[i]->GetComponentTransform(), Params);
			if (!Actor) continue;

			Actor->UnitID = Slot.UnitName;

			if (UStatComponent* Stat = Actor->GetStat())
			{
				Stat->MaxHP     = Slot.MaxHP;
				Stat->CurrentHP = Slot.MaxHP;
			}

			SpawnedEnemies.Add(Actor);
			UE_LOG(LogTemp, Log, TEXT("[CombatManager] Enemy[%d] 스폰: %s (HP:%d)"), i, *Slot.UnitName.ToString(), Slot.MaxHP);
		}
	}
	else if (bEnemyManualSet || EnemyActor_0)
	{
		// EnemyActor 슬롯 직접 사용
		AUnit* EnemyActorArr[] = { EnemyActor_0, EnemyActor_1, EnemyActor_2 };
		const int32 ClampedEnemyCount = FMath::Clamp(EnemyCount, 1, 3);
		for (int32 i = 0; i < ClampedEnemyCount; i++)
		{
			AUnit* Actor = EnemyActorArr[i];
			if (!Actor) continue;
			SpawnedEnemies.Add(Actor);
			UE_LOG(LogTemp, Log, TEXT("[CombatManager] Enemy[%d] 등록: %s"), i, *Actor->GetName());
		}
	}
	else
	{
		// 최후 수단: 레벨에 배치된 Team==Enemy AUnit 자동 탐색
		for (TActorIterator<AUnit> It(GetWorld()); It && SpawnedEnemies.Num() < 3; ++It)
		{
			if (It->Team == ETeam::Enemy)
				SpawnedEnemies.Add(*It);
		}
		EnemyCount = SpawnedEnemies.Num();
		if (EnemyCount > 0)
			UE_LOG(LogTemp, Log, TEXT("[CombatManager] 레벨에서 적 %d명 자동 탐색"), EnemyCount);
	}

	// ── 5. 배틀 메인 위젯 생성 ──────────────────────────────────
	// 유닛 등록 완료 후 생성 — Event Construct에서 GetSpawnedPlayers/Enemies가 유효한 값을 반환하도록
	if (BattleWidgetClass && PC)
	{
		BattleWidget = CreateWidget<UBattleMainWidget>(PC, BattleWidgetClass);
		if (BattleWidget)
			BattleWidget->AddToViewport();
		else
			UE_LOG(LogTemp, Error, TEXT("[CombatManager] BattleWidget creation failed"));
	}

	// 수동 세팅 사용 중이면 화면 좌상단에 표시
	if ((bPlayerManualSet || bEnemyManualSet) && GEngine)
	{
		const FString Msg = FString::Printf(TEXT("[테스트 세팅 사용중] Player:%s  Enemy:%s"),
			bPlayerManualSet ? TEXT("수동") : TEXT("자동"),
			bEnemyManualSet  ? TEXT("수동") : TEXT("자동"));
		GEngine->AddOnScreenDebugMessage(999, 99999.f, FColor::Yellow, Msg);
	}

	// 모든 액터의 BeginPlay가 완료된 후 드로우가 실행되도록 한 프레임 지연
	// (CardUserComponent.DeckComponent는 BeginPlay에서 생성되므로 동일 틱 호출 시 null일 수 있음)
	GetWorldTimerManager().SetTimerForNextTick(this, &ACombatManager::StartTurn);
}


// 카드 데이터를 기반으로 타겟을 결정하고 데미지/회복/방어도 효과를 적용
// TargetOverride가 있으면 SingleEnemy 타입에서 해당 유닛을 타겟으로 우선 사용
void ACombatManager::ExecuteCard(const FCardDataRow& Card, int32 CasterIndex, AUnit* TargetOverride)
{
	if (!SpawnedPlayers.IsValidIndex(CasterIndex)) return;
	AUnit* Caster = SpawnedPlayers[CasterIndex];

	// 타겟 목록 결정
	TArray<AUnit*> Targets;
	switch (Card.TargetType)
	{
		case ETargetType::SingleEnemy:
			// 플레이어가 선택한 타겟 우선, 없으면 0번 적 고정
			if (TargetOverride) Targets.Add(TargetOverride);
			else if (SpawnedEnemies.IsValidIndex(0)) Targets.Add(SpawnedEnemies[0]);
			break;
		case ETargetType::AllEnemies:
			Targets = SpawnedEnemies;
			break;
		case ETargetType::Self:
			Targets.Add(Caster);
			break;
		case ETargetType::SingleAlly:
			if (TargetOverride) Targets.Add(TargetOverride);
			else if (SpawnedPlayers.IsValidIndex(0)) Targets.Add(SpawnedPlayers[0]);
			break;
		case ETargetType::AllAllies:
		case ETargetType::Single_Team:
			Targets = SpawnedPlayers;
			break;
	}

	UE_LOG(LogTemp, Log, TEXT("[ExecuteCard] Targets=%d Damage=%d"), Targets.Num(), Card.Damage);

	// ── 1. 기본 효과: Damage / HealAmount / Block — 카드의 TargetType 대상에게 적용 ──
	for (AUnit* Target : Targets)
	{
		if (!Target || !Target->IsAlive()) continue;

		UStatComponent* Stat = Target->GetStat();
		if (!Stat) continue;

		if (Card.Damage > 0)
		{
			for (int32 i = 0; i < Card.UsingCount; i++)
				UEffectManager::ProcessDamage(Target, Card.Damage, Caster);
		}

		if (Card.HealAmount > 0)
			Stat->Heal(Card.HealAmount);

		if (Card.Block > 0)
			UEffectManager::ApplyEffect(Target, EEffectType::Shield, Card.Block);
	}

	// ── 2. Effects 배열 — 각 항목의 TargetType으로 독립적으로 대상 결정 ──────────
	// EEffectTargetType::UseCardDefault이면 카드 기본 Targets 그대로 사용
	auto ResolveEffectTargets = [&](EEffectTargetType EffTargetType) -> TArray<AUnit*>
	{
		switch (EffTargetType)
		{
			case EEffectTargetType::UseCardDefault: return Targets;
			case EEffectTargetType::SingleEnemy:
				if (TargetOverride) return { TargetOverride };
				if (SpawnedEnemies.IsValidIndex(0)) return { SpawnedEnemies[0] };
				return {};
			case EEffectTargetType::AllEnemies:   return SpawnedEnemies;
			case EEffectTargetType::Self:          return { Caster };
			case EEffectTargetType::SingleAlly:
				if (TargetOverride) return { TargetOverride };
				if (SpawnedPlayers.IsValidIndex(0)) return { SpawnedPlayers[0] };
				return {};
			case EEffectTargetType::AllAllies:     return SpawnedPlayers;
			default:                               return Targets;
		}
	};

	for (const FCardEffect& Effect : Card.Effects)
	{
		if (Effect.EffectType == EEffectType::None || Effect.Value <= 0) continue;

		// DrawCard: 시전자 드로우 — 타겟 수와 무관하게 1회만 실행
		if (Effect.EffectType == EEffectType::DrawCard)
		{
			if (UCardUserComponent* CardComp = Caster->FindComponentByClass<UCardUserComponent>())
				CardComp->DrawCards(Effect.Value);
			continue;
		}

		const TArray<AUnit*> EffectTargets = ResolveEffectTargets(Effect.TargetType);
		const uint8 TypeVal = static_cast<uint8>(Effect.EffectType);

		for (AUnit* Target : EffectTargets)
		{
			if (!Target || !Target->IsAlive()) continue;

			// Heal: StatComponent로 직접 HP 회복
			if (Effect.EffectType == EEffectType::Heal)
			{
				if (UStatComponent* Stat = Target->GetStat())
					Stat->Heal(Effect.Value);
				continue;
			}

			if (TypeVal >= 200)
				UEffectManager::ApplyDebuff(Target, Effect.EffectType, Effect.Value);
			else if (TypeVal >= 100)
				UEffectManager::ApplyBuff(Target, Effect.EffectType, Effect.Value);
			else
				UEffectManager::ApplyEffect(Target, Effect.EffectType, Effect.Value);
		}
	}

	// 카드 실행 완료 브로드캐스트 (히스토리 위젯·애니메이션 트리거용)
	OnActionExecuted.Broadcast(Card, CasterIndex);

	// 카드 사용 후 즉시 사망 여부 확인 (적 전멸 시 전투 종료 조건 체크)
	CheckCombatEnd();
}

// 모든 적 또는 플레이어의 생존 여부를 확인하고 결과를 로그로 출력
// TODO: 승리/패배 화면 구현 시 여기서 페이즈 진행 중단 처리 추가
void ACombatManager::CheckCombatEnd()
{
	// 살아있는 적이 한 명도 없으면 전투 승리
	const bool bAllEnemiesDead = SpawnedEnemies.Num() > 0 &&
		!SpawnedEnemies.ContainsByPredicate([](AUnit* U){ return U && U->IsAlive(); });

	if (bAllEnemiesDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatManager] 모든 적 사망 — 전투 승리"));
		// TODO: 전투 종료 처리 (승리 화면, 보상 등) 구현 시 여기서 페이즈 진행 중단
		return;
	}

	// 살아있는 플레이어가 한 명도 없으면 게임 오버
	const bool bAllPlayersDead = SpawnedPlayers.Num() > 0 &&
		!SpawnedPlayers.ContainsByPredicate([](AUnit* U){ return U && U->IsAlive(); });

	if (bAllPlayersDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatManager] 모든 플레이어 사망 — 게임 오버"));
		// TODO: 게임 오버 처리 (패배 화면 등) 구현 시 여기서 페이즈 진행 중단
	}
}

// 현재 페이즈를 변경하고 델리게이트를 브로드캐스트. 진입 전 전투 종료 여부를 먼저 확인
void ACombatManager::SetPhase(ETurnPhase NewPhase)
{
	CheckCombatEnd();

	CurrentPhase = NewPhase;
	OnPhaseChanged.Broadcast(NewPhase);

	switch (NewPhase)
	{
		case ETurnPhase::DrawPhase:            UE_LOG(LogTemp, Log, TEXT("[Turn %d] 드로우턴"), TurnCount);      break;
		case ETurnPhase::PlayerActionPhase:    UE_LOG(LogTemp, Log, TEXT("[Turn %d] 플레이어 행동턴"), TurnCount); break;
		case ETurnPhase::EnemyPhase:           UE_LOG(LogTemp, Log, TEXT("[Turn %d] 몬스터턴"), TurnCount);      break;
	}
}

// 지정 유닛 배열의 Shield만 0으로 리셋
void ACombatManager::ApplyShieldReset(const TArray<AUnit*>& Units)
{
	for (AUnit* Unit : Units)
	{
		if (!Unit) continue;
		UStatusEffectComponent* SEC = Unit->FindComponentByClass<UStatusEffectComponent>();
		if (!SEC) continue;

		if (SEC->GetEffectValue(EEffectType::Shield) > 0)
			SEC->SetEffectValue(EEffectType::Shield, 0);
	}
}

// Shield 제외 버프/디버프 tick (Regen 회복, Burn 데미지, 시간제 스택 감소)
void ACombatManager::TickBuffsAndDebuffs(const TArray<AUnit*>& Units)
{
	for (AUnit* Unit : Units)
	{
		if (!Unit) continue;
		UStatusEffectComponent* SEC = Unit->FindComponentByClass<UStatusEffectComponent>();
		if (!SEC) continue;

		// 수치형 버프/디버프 틱
		UEffectManager::TickEffects(Unit);

		// 오브젝트형 효과 틱 (UStatusEffect 파생 클래스)
		for (UStatusEffect* Effect : SEC->Active)
			if (Effect) Effect->OnTurnEnd();
	}
}

// 턴 카운트를 올리고 DrawPhase → PlayerActionPhase 순서로 진행
void ACombatManager::StartTurn()
{
	TurnCount++;
	UE_LOG(LogTemp, Log, TEXT("[CombatManager] Turn %d 시작"), TurnCount);

	SetPhase(ETurnPhase::DrawPhase);

	// 플레이어: Shield 리셋 + 버프/디버프 tick
	ApplyShieldReset(SpawnedPlayers);
	TickBuffsAndDebuffs(SpawnedPlayers);

	// 몬스터: 버프/디버프 tick만 (Shield는 EnemyPhase 시작 시 따로 리셋)
	TickBuffsAndDebuffs(SpawnedEnemies);

	PlanAllEnemyActions(); // DrawPhase: 모든 적의 이번 턴 행동을 미리 결정

	// 살아있는 플레이어 유닛에게만 턴 시작 드로우 요청
	for (AUnit* Player : SpawnedPlayers)
	{
		if (!Player || !Player->IsAlive()) continue;
		UCardUserComponent* CardComp = Player->FindComponentByClass<UCardUserComponent>();
		if (CardComp)
			CardComp->DrawStartOfTurn();
	}

	SetPhase(ETurnPhase::PlayerActionPhase);
}

// PlayerActionPhase에서만 동작. 사용한 카드를 ActionQueue에 추가
void ACombatManager::QueuePlayerAction(const FCardDataRow& Card, int32 CasterIndex, FName CardRowName, AUnit* TargetOverride)
{
	if (CurrentPhase != ETurnPhase::PlayerActionPhase) return;

	FQueuedAction Action;
	Action.Card           = Card;
	Action.CasterIndex    = CasterIndex;
	Action.CardRowName    = CardRowName;
	Action.TargetOverride = TargetOverride;
	if (ActionQueue.Num() >= 10)
		ActionQueue.RemoveAt(0);
	ActionQueue.Add(Action);

	UE_LOG(LogTemp, Log, TEXT("[CombatManager] 큐 추가: %s Target=%s (큐 크기=%d)"),
		*Card.CardID.ToString(),
		TargetOverride ? *TargetOverride->GetName() : TEXT("default"),
		ActionQueue.Num());

	// 콤보 판정은 여기서 하지 않는다 — 카드 효과(ExecuteCard) 적용 이후에 호출되어야 하므로
	// BattleMainWidget::QueueCardAction이 ExecuteCard 다음에 EvaluatePlayedCardCombos()를 호출한다
}

// 카드 효과 적용 후 콤보 조건 평가 — BattleMainWidget::QueueCardAction에서 ExecuteCard 다음에 호출
void ACombatManager::EvaluatePlayedCardCombos(int32 CasterIndex)
{
	if (ComboEvaluator)
		ComboEvaluator->EvaluateAfterCardPlayed(CasterIndex);
}

// 플레이어 행동 입력을 종료하고 적 턴으로 직접 전환 (카드 효과는 이미 즉시 실행됨)
void ACombatManager::EndPlayerActionPhase()
{
	if (CurrentPhase != ETurnPhase::PlayerActionPhase) return;
	StartEnemyPhase();
}

// 적 턴 페이즈를 시작하고 첫 번째 적부터 행동을 진행
void ACombatManager::StartEnemyPhase()
{
	CurrentEnemyIndex = 0;
	SetPhase(ETurnPhase::EnemyPhase);

	// 몬스터 페이즈 시작 트리거 — 방어도 사라지기 전 (연출·UI용)
	OnEnemyPhaseStarted.Broadcast();

	// 몬스터 Shield만 리셋 (버프/디버프는 DrawPhase에서 이미 처리됨)
	ApplyShieldReset(SpawnedEnemies);

	ExecuteNextEnemyAction();
}

// 죽은 적을 건너뛰며 현재 인덱스의 적이 행동하도록 처리.
// 모든 적이 행동 완료되면 다음 턴을 시작
void ACombatManager::ExecuteNextEnemyAction()
{
	// 죽은 적 건너뜀
	while (SpawnedEnemies.IsValidIndex(CurrentEnemyIndex) &&
		   (!SpawnedEnemies[CurrentEnemyIndex] || !SpawnedEnemies[CurrentEnemyIndex]->IsAlive()))
	{
		CurrentEnemyIndex++;
	}

	if (!SpawnedEnemies.IsValidIndex(CurrentEnemyIndex))
	{
		// 모든 적 행동 완료 → 다음 턴
		StartTurn();
		return;
	}

	AUnit* Enemy = SpawnedEnemies[CurrentEnemyIndex];
	OnEnemyTurnStart.Broadcast(CurrentEnemyIndex);

	// PlanNextAction에서 정해둔 행동을 실행
	UNPCBrainComponent* Brain = Enemy->FindComponentByClass<UNPCBrainComponent>();
	if (Brain)
		ExecuteEnemyAction(Enemy, Brain->PendingAction);

	// EnemyActionDelay 후 다음 적 자동 진행 (에디터 Combat|Timing에서 조정)
	GetWorldTimerManager().SetTimer(
		EnemyTimerHandle,
		this, &ACombatManager::OnEnemyActionComplete,
		EnemyActionDelay, false);
}

// 현재 적의 행동이 끝났을 때 호출. 다음 적으로 인덱스를 넘김
void ACombatManager::OnEnemyActionComplete()
{
	// 몬스터 행동 완료 트리거 (연출·UI용)
	OnEnemyActionFinished.Broadcast(CurrentEnemyIndex);

	CurrentEnemyIndex++;
	ExecuteNextEnemyAction();
}

// Index번 플레이어의 StatComponent를 반환. 유효하지 않으면 nullptr
UStatComponent* ACombatManager::GetPlayerStat(int32 Index) const
{
	if (!SpawnedPlayers.IsValidIndex(Index) || !SpawnedPlayers[Index]) return nullptr;
	return SpawnedPlayers[Index]->GetStat();
}

// Index번 적의 StatComponent를 반환. 유효하지 않으면 nullptr
UStatComponent* ACombatManager::GetEnemyStat(int32 Index) const
{
	if (!SpawnedEnemies.IsValidIndex(Index) || !SpawnedEnemies[Index]) return nullptr;
	return SpawnedEnemies[Index]->GetStat();
}

// DrawPhase에 살아있는 모든 적의 행동을 미리 결정
// 적 시점: Allies=SpawnedEnemies(아군), Enemies=SpawnedPlayers(적)
void ACombatManager::PlanAllEnemyActions()
{
	for (AUnit* Enemy : SpawnedEnemies)
	{
		if (!Enemy || !Enemy->IsAlive()) continue;
		UNPCBrainComponent* Brain = Enemy->FindComponentByClass<UNPCBrainComponent>();
		if (Brain)
			Brain->PlanNextAction(SpawnedEnemies, SpawnedPlayers);
	}
}

// 적 한 명의 FEnemyAction을 실제로 실행
// - Attack: IntentComponent에 저장된 타겟 or 광역 대상에게 Hits 횟수만큼 ProcessDamage
// - Defend: 자신에게 Shield 부여
// - Buff/Debuff: 미구현 (StatusEffect 오브젝트 연결 후 추가 예정)
// - NoAttack: 행동 없음
void ACombatManager::ExecuteEnemyAction(AUnit* Caster, const FEnemyAction& Action)
{
	if (!Caster || !Caster->IsAlive()) return;

	// 타겟 목록 결정 (적 시점에서 "Enemy" = 플레이어, "Ally" = 다른 적)
	TArray<AUnit*> Targets;
	switch (Action.TargetType)
	{
	case ETargetType::SingleEnemy:
	{
		// 1순위: IntentComponent에 PlanNextAction이 저장한 타겟
		AUnit* Resolved = nullptr;
		UIntentComponent* Intent = Caster->FindComponentByClass<UIntentComponent>();
		if (Intent && Intent->Current.Target.IsValid())
			Resolved = Intent->Current.Target.Get();

		// 2순위: IntentComponent 없거나 타겟 무효 → 살아있는 랜덤 플레이어 직접 선택
		if (!Resolved)
		{
			TArray<AUnit*> Alive;
			for (AUnit* P : SpawnedPlayers) { if (P && P->IsAlive()) Alive.Add(P); }
			if (!Alive.IsEmpty())
				Resolved = Alive[FMath::RandRange(0, Alive.Num() - 1)];
		}

		if (Resolved) Targets.Add(Resolved);
		break;
	}
	case ETargetType::SingleAlly:
	{
		// 1순위: IntentComponent 타겟
		AUnit* Resolved = nullptr;
		UIntentComponent* Intent = Caster->FindComponentByClass<UIntentComponent>();
		if (Intent && Intent->Current.Target.IsValid())
			Resolved = Intent->Current.Target.Get();

		// 2순위: 살아있는 랜덤 적 직접 선택
		if (!Resolved)
		{
			TArray<AUnit*> Alive;
			for (AUnit* E : SpawnedEnemies) { if (E && E->IsAlive() && E != Caster) Alive.Add(E); }
			if (!Alive.IsEmpty())
				Resolved = Alive[FMath::RandRange(0, Alive.Num() - 1)];
		}

		if (Resolved) Targets.Add(Resolved);
		break;
	}
	case ETargetType::AllEnemies: // 적 시점: 모든 플레이어
		for (AUnit* P : SpawnedPlayers) { if (P && P->IsAlive()) Targets.Add(P); }
		break;
	case ETargetType::AllAllies: // 적 시점: 모든 적
		for (AUnit* E : SpawnedEnemies) { if (E && E->IsAlive()) Targets.Add(E); }
		break;
	case ETargetType::Self:
		Targets.Add(Caster);
		break;
	default:
		break;
	}

	// 행동 종류별 실행
	switch (Action.IntentKind)
	{
	case EIntentKind::Attack:
		for (AUnit* Target : Targets)
		{
			// Hits 횟수만큼 반복 공격
			for (int32 i = 0; i < FMath::Max(1, Action.Hits); i++)
				UEffectManager::ProcessDamage(Target, Action.Value, Caster);
		}
		UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s 공격 → %d 타겟, %d 데미지 x%d"),
			*Caster->GetName(), Targets.Num(), Action.Value, Action.Hits);
		break;

	case EIntentKind::Defend:
		// Targets 배열 기준으로 Shield 부여 (Self → 자신, AllAllies → 아군 전체 등)
		// Targets가 비어있으면 자신에게 폴백
		if (Targets.IsEmpty()) Targets.Add(Caster);
		for (AUnit* Target : Targets)
			UEffectManager::ApplyEffect(Target, EEffectType::Shield, Action.Value);
		UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s 방어 → %d 타겟에 Shield +%d"),
			*Caster->GetName(), Targets.Num(), Action.Value);
		break;

	case EIntentKind::Buff:
	case EIntentKind::Debuff:
		// TODO: StatusEffect 오브젝트형 연결 시 구현
		UE_LOG(LogTemp, Warning, TEXT("[CombatManager] %s Buff/Debuff — 미구현"),
			*Caster->GetName());
		break;

	case EIntentKind::NoAttack:
	default:
		UE_LOG(LogTemp, Log, TEXT("[CombatManager] %s 행동 없음"), *Caster->GetName());
		break;
	}
}
