#include "UI/SubLevelUIManager.h"

#include "Blueprint/UserWidget.h"
#include "GameManagers/LevelManager.h"
#include "Hud/HUDManager.h"

ASubLevelUIManager::ASubLevelUIManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASubLevelUIManager::BeginPlay()
{
	Super::BeginPlay();

	if (ULevelManager* LevelManager = GetLevelManager())
	{
		LevelManager->OnStreamedLevelChanged.AddUniqueDynamic(this, &ASubLevelUIManager::HandleStreamedLevelChanged);
		LevelManager->OnStreamedLevelEntered.AddUniqueDynamic(this, &ASubLevelUIManager::ApplyUIForLevel);

		if (bApplyCurrentLevelOnBeginPlay)
		{
			const FName CurrentLevelName = LevelManager->GetCurrentStreamedLevelName();
			if (!CurrentLevelName.IsNone())
			{
				ApplyUIForLevel(CurrentLevelName);
			}
		}
	}
}

void ASubLevelUIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULevelManager* LevelManager = GetLevelManager())
	{
		LevelManager->OnStreamedLevelChanged.RemoveDynamic(this, &ASubLevelUIManager::HandleStreamedLevelChanged);
		LevelManager->OnStreamedLevelEntered.RemoveDynamic(this, &ASubLevelUIManager::ApplyUIForLevel);
	}

	Super::EndPlay(EndPlayReason);
}

void ASubLevelUIManager::HandleStreamedLevelChanged(FName PreviousLevelName, FName NewLevelName)
{
	OnSubLevelChanged(PreviousLevelName, NewLevelName);
}

void ASubLevelUIManager::ApplyUIForLevel(FName LevelName)
{
	if (LevelName.IsNone())
	{
		return;
	}

	const FSubLevelUIRule* Rule = FindRule(LevelName);
	if (!Rule)
	{
		if (bHidePreviousUI)
		{
			HideActiveLevelUI();
		}

		ActiveLevelName = LevelName;
		ActiveUIName = NAME_None;
		OnSubLevelUIRuleMissing(LevelName);
		return;
	}

	if (bHidePreviousUI && ActiveUIName != Rule->UIName)
	{
		HideActiveLevelUI();
	}

	if (UHUDManager* HUDManager = GetHUDManager())
	{
		if (Rule->bShowMainHUD)
		{
			HUDManager->ShowMainHUD();
		}

		if (!Rule->UIName.IsNone())
		{
			HUDManager->ShowWidgetByName(Rule->UIName);
		}
	}

	ActiveLevelName = Rule->LevelName;
	ActiveUIName = Rule->UIName;
	OnSubLevelUIRuleApplied(*Rule);
}

void ASubLevelUIManager::HideActiveLevelUI()
{
	if (ActiveUIName.IsNone())
	{
		return;
	}

	if (UHUDManager* HUDManager = GetHUDManager())
	{
		HUDManager->HideWidgetByName(ActiveUIName);
	}

	ActiveUIName = NAME_None;
}

ULevelManager* ASubLevelUIManager::GetLevelManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<ULevelManager>() : nullptr;
}

UHUDManager* ASubLevelUIManager::GetHUDManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UHUDManager>() : nullptr;
}

const FSubLevelUIRule* ASubLevelUIManager::FindRule(FName LevelName) const
{
	return LevelUIRules.FindByPredicate([LevelName](const FSubLevelUIRule& Rule)
	{
		return Rule.LevelName == LevelName;
	});
}
