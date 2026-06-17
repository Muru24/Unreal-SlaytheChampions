// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Unit/Enemy/EnemyDataTable.h"
#include "EnemyInitializerComponent.generated.h"

UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class SLAYTHECHAMPIONS_API UEnemyInitializerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyInitializerComponent();

	// 에디터에서 테이블 DA를 지정해두면 BeginPlay에서 EnemyID로 자동 초기화
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TObjectPtr<UEnemyDataTable> Table;

	// BP_Enemy에 적용할 적 ID (Table.FindByID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FName EnemyID;

	// 외부(CombatManager 등)에서 Struct를 직접 넘겨 초기화할 때 사용
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeFromDefinition(const FEnemyDefinition& Def);

	// Table+EnemyID로 찾아서 초기화
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeFromTable();

protected:
	virtual void BeginPlay() override;
};
