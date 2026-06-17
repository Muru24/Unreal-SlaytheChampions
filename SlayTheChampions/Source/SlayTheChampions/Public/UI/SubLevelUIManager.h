#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SubLevelUIManager.generated.h"

class UHUDManager;
class ULevelManager;

USTRUCT(BlueprintType)
struct FSubLevelUIRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubLevelUI")
	FName LevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubLevelUI")
	FName UIName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubLevelUI")
	bool bShowMainHUD = true;
};

UCLASS(BlueprintType, Blueprintable)
class SLAYTHECHAMPIONS_API ASubLevelUIManager : public AActor
{
	GENERATED_BODY()

public:
	ASubLevelUIManager();

	UFUNCTION(BlueprintCallable, Category = "SubLevelUI")
	void ApplyUIForLevel(FName LevelName);

	UFUNCTION(BlueprintCallable, Category = "SubLevelUI")
	void HideActiveLevelUI();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleStreamedLevelChanged(FName PreviousLevelName, FName NewLevelName);

	UFUNCTION(BlueprintImplementableEvent, Category = "SubLevelUI")
	void OnSubLevelChanged(FName PreviousLevelName, FName NewLevelName);

	UFUNCTION(BlueprintImplementableEvent, Category = "SubLevelUI")
	void OnSubLevelUIRuleApplied(const FSubLevelUIRule& Rule);

	UFUNCTION(BlueprintImplementableEvent, Category = "SubLevelUI")
	void OnSubLevelUIRuleMissing(FName LevelName);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SubLevelUI")
	bool bApplyCurrentLevelOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SubLevelUI")
	bool bHidePreviousUI = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SubLevelUI")
	TArray<FSubLevelUIRule> LevelUIRules;

	UPROPERTY(BlueprintReadOnly, Category = "SubLevelUI")
	FName ActiveLevelName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "SubLevelUI")
	FName ActiveUIName = NAME_None;

private:
	ULevelManager* GetLevelManager() const;
	UHUDManager* GetHUDManager() const;
	const FSubLevelUIRule* FindRule(FName LevelName) const;
};
