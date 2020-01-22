// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Types/SlateEnums.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerRow.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API ULobbyPlayerRow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
	class UCustomComboBox* PlayerColor;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerStatus;

	UFUNCTION(BlueprintCallable)
	void OnPlayerColorChange(FString newColorHex);

	UFUNCTION(BlueprintCallable)
	void SetColor(FString inColorHex);

	void SetIsEnabledCustom(bool inIsEnabled);

	void Update(FString inPlayerName, FString inPlayerColor, bool bIsReady, bool bIsSpectator, bool inIsEnabled);

	void Init(class ULobbyMenu* inMenu, int32 inPlayerId, TArray<FString> PlayerColors);
	int32 GetPlayerId() { return PlayerId; }

protected:
	bool Initialize() override;

	int32 PlayerId;

	UPROPERTY()
	class ULobbyMenu* ParentMenu;

};
