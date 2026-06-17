#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor.h"
#include "Relic/RelicStruct.h"
#include "RewardRelicItemActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SLAYTHECHAMPIONS_API ARewardRelicItemActor : public AItemActor
{
	GENERATED_BODY()

public:
	ARewardRelicItemActor();

protected:
	virtual void BeginPlay() override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Reward Relic")
	bool RollRewardRelic();

	UFUNCTION(BlueprintCallable, Category = "Reward Relic")
	bool ClaimRewardRelic(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Reward Relic")
	void OnRewardRelicRolled(const FRelic& RelicData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Reward Relic")
	void OnRewardRelicClaimed(const FRelic& RelicData, AActor* Interactor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward Relic")
	ERelicSourceType RewardRelicSourceType = ERelicSourceType::Common;
};
