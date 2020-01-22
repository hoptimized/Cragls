// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Menus/MenuInterface.h"

#include "Animation/UMGSequencePlayer.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"


USTRUCT()
struct FServerData
{
	GENERATED_BODY()

	FString SessionId;
	FString ServerName;
	FString HostName;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	uint16 SecondsRemaining;
	int32 PingInMs;
	bool isLanMatch;
	bool isPrivate;
};


UCLASS()
class SKYRACE_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer& ObjectInitializer);
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	void SetMenuInterface(IMenuInterface* theInterface);
	void Setup();
	void SetServerList(TArray<FServerData> ServerNames);
	void SelectSession(FString inSessionId);
	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
	void DestroyMenu(UWorld* InWorld);

protected:
	bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UButton* BackButton;

	UFUNCTION()
	void BackButtonOnClick();

	/* --------------------------------------------------------------------------------------------------------- */
	/* START PAGE */

	UPROPERTY(meta = (BindWidget))
	class UWidget* MainMenu;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainMenuPlay;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainMenuSettings;

	UPROPERTY(meta = (BindWidget))
	class UButton* MainMenuExit;

	UFUNCTION()
	void MainMenuPlayOnClick();

	UFUNCTION()
	void MainMenuSettingsOnClick();

	UFUNCTION(BlueprintCallable)
	void MainMenuExitOnClick();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UWidgetAnimation* MainToServerlistTransition;

	void OpenServerlistMenu();

	/* --------------------------------------------------------------------------------------------------------- */
	/* SETTINGS */

	UPROPERTY(meta = (BindWidget))
	class UWidget* SettingsMenu;

	/* --------------------------------------------------------------------------------------------------------- */
	/* SERVER LIST */

	UPROPERTY(meta = (BindWidget))
	class UWidget* ServerListMenu;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* ServerList;

	TSubclassOf<class UUserWidget> ServerRowClass;
	TOptional<FString> SelectedSession;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinNowButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UFUNCTION()
	void HostButtonOnClick();

	UFUNCTION()
	void JoinServer();

	UFUNCTION()
	void RefreshServerListCallback();
	void RefreshServerList();

	FTimerHandle RefreshTimer;

	/* --------------------------------------------------------------------------------------------------------- */
	/* HOST MENU */

	UPROPERTY(meta = (BindWidget))
	class UWidget* HostMenu;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostNowButton;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* GameNameTextbox;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* MaxPlayersComboBox;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* NetworkComboBox;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* IsPrivateComboBox;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* PasswordTextBox;

	UFUNCTION()
	void HostServer();

	void OpenHostMenu();

	bool destroying = false;


	UWidget* nextMenu;
	void switchToMenu(UWidget* NewMenu);
	void fadeInCallback(UUMGSequencePlayer& Player);
	void fadeOutCallback(UUMGSequencePlayer& Player);
	void growCallback(UUMGSequencePlayer& Player);
	void shrinkCallback(UUMGSequencePlayer& Player);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UWidgetAnimation* MenuFadeInAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UWidgetAnimation* MenuGrowAnimation;

	UPROPERTY(meta = (BindWidget))
	class UWidget* SteamNotFound;
	

private:
	IMenuInterface* MenuInterface;

};
