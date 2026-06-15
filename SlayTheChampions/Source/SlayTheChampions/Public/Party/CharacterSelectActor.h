#pragma once

#include "CoreMinimal.h"
#include "Card/CardDataTypes.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "CharacterSelectActor.generated.h"

class UBoxComponent;
class UCharacterSelectVisualDataAsset;
class USkeletalMeshComponent;
class UTexture2D;

USTRUCT(BlueprintType)
struct FSelectableCharacterInfo
{
	GENERATED_BODY()

	/* 파티/시각 데이터에서 같은 캐릭터를 찾기 위한 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FName UnitID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	EJobClass JobClass = EJobClass::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UTexture2D> Portrait = nullptr;

	/* 이 캐릭터의 초기 덱 DataTable (FStarterDeckRow 구조체) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UDataTable> StarterDeckTable = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterSelected, ACharacterSelectActor*, SelectActor, const FSelectableCharacterInfo&, CharacterInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCharacterSelectionChanged, ACharacterSelectActor*, SelectActor, const FSelectableCharacterInfo&, CharacterInfo, bool, bInSelected);

UCLASS(BlueprintType, Blueprintable)
class SLAYTHECHAMPIONS_API ACharacterSelectActor : public AActor
{
	GENERATED_BODY()

public:
	ACharacterSelectActor();

	/* UnitID를 바꾸면 기본 정보와 시각 데이터를 다시 읽는다. */
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	void SetUnitID(FName InUnitID);

	/* UnitID에 맞는 스켈레탈 메쉬/애니메이션 데이터를 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	bool ApplyVisualDataByUnitID();

	/* 파티에 캐릭터를 추가한다. */
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	bool SelectCharacter();

	/* 파티에서 캐릭터를 제거한다. */
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	bool DeselectCharacter();

	/* 클릭할 때마다 선택/해제를 번갈아 처리한다. */
	UFUNCTION(BlueprintCallable, Category = "CharacterSelect")
	bool ToggleCharacterSelection();

	UFUNCTION(BlueprintPure, Category = "CharacterSelect")
	const FSelectableCharacterInfo& GetCharacterInfo() const { return CharacterInfo; }

	UFUNCTION(BlueprintPure, Category = "CharacterSelect")
	bool IsSelected() const { return bSelected; }

	UPROPERTY(BlueprintAssignable, Category = "CharacterSelect")
	FOnCharacterSelected OnCharacterSelected;

	UPROPERTY(BlueprintAssignable, Category = "CharacterSelect")
	FOnCharacterSelectionChanged OnCharacterSelectionChanged;

protected:
	virtual void BeginPlay() override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect")
	void OnCharacterInfoLoaded(const FSelectableCharacterInfo& LoadedInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "CharacterSelect")
	void OnSelectionStateChanged(bool bInSelected);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	TObjectPtr<USkeletalMeshComponent> PreviewMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	TObjectPtr<UBoxComponent> ClickBox;

	/* 에디터에서 넣는 캐릭터 기본 정보 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	FSelectableCharacterInfo CharacterInfo;

	/* UnitID별 선택창 메쉬/애니메이션 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect|Visual")
	TObjectPtr<UCharacterSelectVisualDataAsset> VisualDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect|Visual")
	bool bLoadDefaultVisualDataAsset = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CharacterSelect")
	bool bDisableAfterSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterSelect")
	bool bSelected = false;

private:
	void LoadUnitDataByID();
	void LoadDefaultVisualDataAsset();
	void SetHoverOutline(bool bEnabled);
};
