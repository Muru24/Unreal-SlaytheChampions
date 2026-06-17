// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/MapStruct.h"
#include "MapManager.generated.h"

class UArea;
class UMapCreator;
class URunSystem;

UCLASS(BlueprintType, Blueprintable)
class SLAYTHECHAMPIONS_API UMapManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UMapCreator* MapCreator = nullptr;

	UPROPERTY()
	URunSystem* RunSystem = nullptr;

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	UFUNCTION(BlueprintCallable)
	void StartRunSystem();

	UFUNCTION(BlueprintCallable)
	void MapCreate();

	void RestoreMapWorld();

	bool HasMapData() const;

	void WriteMapInfo(FSaveMapInfo& OutMapInfo) const;

	bool RestoreMapFromSaveInfo(const FSaveMapInfo& InMapInfo);

	void RefreshDebugMapState();

	UArea* GetAreaAt(int32 height, int32 width) const;

	int32 GetMapWidth() const;

	int32 GetMapHeight() const;

	UFUNCTION(BlueprintCallable)
	URunSystem* GetRunSystem() const { return RunSystem; }

	UFUNCTION(BlueprintPure)
	int32 GetCurrentFloorIndex() const;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentDisplayFloor() const;
};
