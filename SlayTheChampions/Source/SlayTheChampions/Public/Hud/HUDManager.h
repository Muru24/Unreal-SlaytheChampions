// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HUDManager.generated.h"

/**
 * 
 */
UCLASS()
class SLAYTHECHAMPIONS_API UHUDManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()


public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "GlobalUI")
    UUserWidget* ShowMainHUD();

    UFUNCTION(BlueprintCallable, Category = "GlobalUI")
    UUserWidget* ShowWidgetByName(FName UIName);

    UFUNCTION(BlueprintCallable, Category = "GlobalUI")
    void HideWidgetByName(FName UIName);

    UFUNCTION(BlueprintCallable, Category = "GlobalUI")
    void RegisterWidget(FName UIName, UUserWidget* Widget);

private:
    UUserWidget* EnsureMainHUD();

    UPROPERTY()
    UUserWidget* MainHUD = nullptr;

    UPROPERTY()
    TMap<FName, UUserWidget*> WidgetPool;
	
};

