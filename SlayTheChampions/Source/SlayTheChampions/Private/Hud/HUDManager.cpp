// Fill out your copyright notice in the Description page of Project Settings.


#include "Hud/HUDManager.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void UHUDManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

UUserWidget* UHUDManager::ShowMainHUD()
{
    UUserWidget* TargetHUD = EnsureMainHUD();
    if (!TargetHUD)
    {
        return nullptr;
    }

    if (!TargetHUD->IsInViewport())
    {
        TargetHUD->AddToViewport();
    }

    TargetHUD->SetVisibility(ESlateVisibility::Visible);
    return TargetHUD;
}

UUserWidget* UHUDManager::ShowWidgetByName(FName UIName)
{
    ShowMainHUD();

    if (!WidgetPool.Contains(UIName))
    {
        return nullptr;
    }

    UUserWidget* TargetWidget = WidgetPool[UIName];
    if (TargetWidget)
    {
        TargetWidget->SetVisibility(ESlateVisibility::Visible);
        return TargetWidget;
    }

    return nullptr;
}

void UHUDManager::HideWidgetByName(FName UIName)
{
    if (WidgetPool.Contains(UIName))
    {
        if (UUserWidget* TargetWidget = WidgetPool[UIName])
        {
            TargetWidget->SetVisibility(ESlateVisibility::Collapsed);
            return;
        }
    }
}

void UHUDManager::RegisterWidget(FName UIName, UUserWidget* Widget)
{
    if (UIName.IsNone())
    {
        return;
    }

    if (!Widget)
    {
        return;
    }

    Widget->SetVisibility(ESlateVisibility::Collapsed);
    WidgetPool.Add(UIName, Widget);
}

UUserWidget* UHUDManager::EnsureMainHUD()
{
    if (MainHUD)
    {
        return MainHUD;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return nullptr;
    }

    static const TCHAR* MainHUDPath = TEXT("/Game/01_WBP/AreaRewardUI/WBP_MainHUD.WBP_MainHUD_C");
    TSubclassOf<UUserWidget> MainHUDClass = LoadClass<UUserWidget>(nullptr, MainHUDPath);
    if (!MainHUDClass)
    {
        return nullptr;
    }

    MainHUD = CreateWidget<UUserWidget>(PlayerController, MainHUDClass);
    return MainHUD;
}

