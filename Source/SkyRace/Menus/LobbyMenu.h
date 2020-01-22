// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Menus/LobbyPlayerRow.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenu.generated.h"

/**
 * 
 */

UCLASS()
class SKYRACE_API ULobbyMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	ULobbyMenu(const FObjectInitializer& ObjectInitializer);
	void Setup(TArray<FString> inPlayerColors, FString GameName);
	void RefreshPlayerList();
	void onColorPickerInput(int32 PlayerId, FString NewColorHex);

	UFUNCTION()
	void OnButtonWaitClick();

	UFUNCTION()
	void OnButtonGoClick();

	UFUNCTION()
	void OnButtonSpectateClick();

	UFUNCTION()
	void OnButtonStartAnywaysClick();

	UFUNCTION()
	void OnButtonStartNowClick();

	UFUNCTION()
	void ClosePopUp();

	UFUNCTION()
	void OnButtonExitClick();

	void DestroyMenu();

protected:
	virtual void NativeTick(const FGeometry & MyGeometry, float InDeltaTime) override;

	bool Initialize() override;

	//--------------------------------------------------------------------------
	// HEADER

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ServerNameText;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonExit;

	//--------------------------------------------------------------------------
	// PLAYER LIST

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* PlayerListWidget;

	//--------------------------------------------------------------------------
	// CONTROLS

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* LobbyControlsSwitcher;

	// Client: ready?
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* ClientReadyBox;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonWait;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonGo;

	// Client: spectate
	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonSpectate;

	// Server: start game?
	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonStartNow;

	//--------------------------------------------------------------------------
	// POP UPS

	UPROPERTY(meta = (BindWidget))
	class UUserWidget* PopUpBoxWindow;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* PopUpBoxSwitcher;

	// Clients not ready
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* ClientsNotReadyBox;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonNotReadyClose;

	// Not reached max amount of clients
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* FewClientsBox;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonStartAnyways;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonDoNotStart;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextCurrentPlayers;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextMaxPlayers;

	//--------------------------------------------------------------------------
	// OTHER

	TSubclassOf<class UUserWidget> PlayerRowClass;

	FTimerHandle RefreshTimer;

	void doRefresh();

	TArray<FString> PlayerColors;

	void SetReadyStatus(bool bIsReady);

	bool bIsListReady = false;

	
};
