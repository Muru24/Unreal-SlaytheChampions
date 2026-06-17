// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"

class AUnit;

// UI와 연동되어 사용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, int32, OldValue, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlockChanged, int32, OldValue, int32, NewValue);

UCLASS( ClassGroup=(Unit), meta=(BlueprintSpawnableComponent) )
class SLAYTHECHAMPIONS_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatComponent();

	UPROPERTY(BlueprintReadOnly, Category = "Stat")
	int32 MaxHP = 100;

	UPROPERTY(BlueprintReadOnly, Category = "Stat")
	int32 CurrentHP = 100;

	UFUNCTION(BlueprintCallable, Category  = "Stat")
	void TakeDamage(int32 Damage, AUnit* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void Heal(int32 Amount);

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnHPChanged OnHPChanged;
		

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
		
};
