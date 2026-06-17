#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelCameraSetter.generated.h"

class ACameraActor;
class UArrowComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelCameraSwitched, int32, SlotIndex, ACameraActor*, CameraActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLevelCameraMoveRequested, int32, SlotIndex, const FTransform&, TargetTransform, ACameraActor*, CameraActor);

UCLASS()
class SLAYTHECHAMPIONS_API ALevelCameraSetter : public AActor
{
	GENERATED_BODY()

public:
	ALevelCameraSetter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Camera|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Camera|Component")
	TObjectPtr<UArrowComponent> DefaultCameraSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Camera")
	TSubclassOf<ACameraActor> CameraClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Camera")
	bool bSetCameraOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Camera", meta = (ClampMin = "0.0"))
	float DefaultBlendTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Camera")
	bool bSetCameraTransformDirectly = true;

	// BP에 추가한 ArrowComponent들을 BeginPlay 때 자동으로 슬롯에 등록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Camera|Slots")
	bool bAutoRegisterChildArrowSlots = true;

	// 자동 등록을 끄고 직접 등록하고 싶을 때 사용하는 카메라 위치 목록
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Level Camera|Slots")
	TArray<TObjectPtr<UArrowComponent>> AdditionalCameraSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Level Camera|Runtime")
	TObjectPtr<ACameraActor> ActiveCamera;

public:
	UPROPERTY(BlueprintAssignable, Category = "Level Camera")
	FOnLevelCameraSwitched OnLevelCameraSwitched;

	UPROPERTY(BlueprintAssignable, Category = "Level Camera")
	FOnLevelCameraMoveRequested OnLevelCameraMoveRequested;

	UFUNCTION(BlueprintCallable, Category = "Level Camera")
	void SpawnDefaultCamera();

	UFUNCTION(BlueprintCallable, Category = "Level Camera")
	void SwitchCameraToDefault(float OverrideBlendTime = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Level Camera")
	void SwitchCameraToSlotIndex(int32 SlotIndex, float OverrideBlendTime = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Level Camera")
	void SwitchCameraToSlot(USceneComponent* CameraSlot, float OverrideBlendTime = -1.f);

	UFUNCTION(BlueprintCallable, Category = "Level Camera|Slots")
	void RegisterAdditionalCameraSlot(UArrowComponent* CameraSlot);

	UFUNCTION(BlueprintCallable, Category = "Level Camera|Slots")
	void AutoRegisterChildArrowSlots();

	UFUNCTION(BlueprintCallable, Category = "Level Camera|Slots")
	void ClearAdditionalCameraSlots();

	UFUNCTION(BlueprintPure, Category = "Level Camera|Slots")
	UArrowComponent* GetDefaultCameraSlot() const { return DefaultCameraSlot; }

	UFUNCTION(BlueprintPure, Category = "Level Camera|Slots")
	UArrowComponent* GetCameraSlotByIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Level Camera|Runtime")
	ACameraActor* GetActiveCamera() const { return ActiveCamera; }

private:
	void MoveCameraToSlot(USceneComponent* CameraSlot, int32 SlotIndex, float OverrideBlendTime);
	float ResolveBlendTime(float OverrideBlendTime) const;
};
