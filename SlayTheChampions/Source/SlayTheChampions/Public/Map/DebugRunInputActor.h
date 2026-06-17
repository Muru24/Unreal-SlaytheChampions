#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugRunInputActor.generated.h"

UCLASS()
class SLAYTHECHAMPIONS_API ADebugRunInputActor : public AActor
{
	GENERATED_BODY()

public:
	ADebugRunInputActor();

protected:
	virtual void BeginPlay() override;

private:
	// 디버그용: 현재 스테이지를 클리어 처리한다.
	UFUNCTION()
	void DebugClearCurrentStage();

private:
	// 맵 화면에서는 디버그 클리어 입력을 무시한다.
	UPROPERTY(EditAnywhere, Category = "Debug")
	FName ReturnLevelName = TEXT("RunMap");
};
