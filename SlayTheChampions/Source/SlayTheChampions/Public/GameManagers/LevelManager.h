#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelManager.generated.h"

UCLASS()
class SLAYTHECHAMPIONS_API ULevelManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

private:
   /*이동할 레벨의 이름*/
    FName MoveLevelName;

   /*현재 스트리밍되어 있는 레벨 이름*/
    FName CurrentStreamedLevelName;

   /*다음에 스트리밍할 레벨 이름*/
    FName PendingStreamedLevelName;

    TObjectPtr<UWorld> ActiveWorld = nullptr;

    bool bIsStreamingTransitionInProgress = false;

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /*
    다른 레벨로 이동
    @param LevelName 이동할 레벨의 이름
    */
    void GoToLevel(FName LevelName);

    UFUNCTION(BlueprintCallable, Category = "Level")
    void MoveToConfiguredLevel(FName LevelName);

    /*
    Persistent Level 위에서 서브레벨 전환
    @param LevelName 스트리밍할 서브레벨 이름
    */
    void GoToStreamedLevel(FName LevelName);

    UFUNCTION(BlueprintCallable, Category = "Level")
    void SetUseStreamedLevelTransition(bool bUseStreaming) { bUseStreamedLevelTransition = bUseStreaming; }

    UFUNCTION(BlueprintPure, Category = "Level")
    bool IsUsingStreamedLevelTransition() const { return bUseStreamedLevelTransition; }

    /*비동기 레벨 로드 호출*/
    void AsynchronousLoadLevel();

    /*비동기 레벨 호출 완료시 시작될 이벤트*/
    UFUNCTION()
    void OnLevelLoadCompleted();

    UFUNCTION()
    void OnStreamedLevelLoaded();

    UFUNCTION()
    void OnCurrentStreamedLevelUnloaded();

    /*Level열릴시 호출되는 이벤트*/
    UFUNCTION()
    void OnLevelStarted(UWorld* World);

    void OnPostWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS);

private:
    UPROPERTY(EditAnywhere, Category = "Level")
    bool bUseStreamedLevelTransition = true;

    UPROPERTY(EditAnywhere, Category = "Level")
    bool bKeepStreamedLevelsLoaded = true;

    void LoadStreamedLevel(FName LevelName);

    void UnloadCurrentStreamedLevel();

    FName GetConfiguredInitialStreamedLevelName() const;

    bool ShouldBootstrapMapSystem(const FString& MapName) const;

    void EnsureInitialStreamedLevelLoaded();

    void HandleMapWorldEntered();

    class UWorld* GetStreamingWorld() const;

    class UMapManager* GetMapManager() const;

    class URunSystem* GetRunSystem() const;

    class ULevelStreaming* FindStreamingLevel(FName LevelName) const;

    bool IsStreamedLevelLoaded(FName LevelName) const;

    void SetStreamedLevelVisibility(FName LevelName, bool bVisible) const;

    /*디버그용*/
    void SpawnDebugInputActor(UWorld* World);
};

