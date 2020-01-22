// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Menus/MenuInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "SettingsSaveGame.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Menus/MainMenu.h"
#include "SkyRaceGameInstance.generated.h"

USTRUCT()
struct FHostingData
{
	GENERATED_BODY()

	FString ServerName;
	uint16 MaxPlayers;
	bool isLanMatch;
	bool isPrivate;
	FString Password;
};

UENUM()
enum class EOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};

UCLASS()
class SKYRACE_API USkyRaceGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()
	
public:
	USkyRaceGameInstance(const FObjectInitializer& ObjectInitializer);
	virtual void Init();
	void HostServer(FString ServerName, uint16 MaxPlayers, bool isLanMatch, bool isPrivate, FString Password) override;
	void JoinServer(FString inSessionId) override;
	bool GetIsLoggedIn() override  { return Subsystem != nullptr; };

	void RefreshServerList() override;

	UFUNCTION(BlueprintCallable)
	void LoadMainMenu();

	UFUNCTION(BlueprintCallable)
	void JumpToGame();

	UFUNCTION(Exec)
	void Host();
	UFUNCTION(Exec)
	void Join(const FString& Address);

	FString GetGameName() { return HostingData.ServerName; };
	uint16 GetGameMaxPlayers() { return HostingData.MaxPlayers; };

	UFUNCTION(BlueprintCallable)
	void LeaveServer();

	UFUNCTION(BlueprintCallable)
	void SaveSettings();

	UFUNCTION(BlueprintCallable)
	void LoadSettings(int& ScreenResolution, int& FrameRate, int& GraphicalQuality, int& ShadowsQuality, int& AntiAliasingQuality, int& BloomQuality, int& CarDetails);

	UFUNCTION(BlueprintCallable)
	void SetScreenResolution(int inVal);

	UFUNCTION(BlueprintCallable)
	void SetFrameRate(int inVal);

	UFUNCTION(BlueprintCallable)
	void SetGraphicalQuality(int inVal);

	UFUNCTION(BlueprintCallable)
	void SetShadowsQuality(int inVal);

	UFUNCTION(BlueprintCallable)
	void SetAntialiasingQuality(int inVal);

	UFUNCTION(BlueprintCallable)
	void SetBloomQuality(int inVal);

	UFUNCTION(BlueprintCallable)
	int GetBloomQuality() { return SettingsBloomQuality; };

	UFUNCTION(BlueprintCallable)
	void SetCarDetails(int inVal);

protected:
	TSubclassOf<class UUserWidget> MainMenuClass;

	UPROPERTY()
	class UMainMenu* MainMenu;

	IOnlineSubsystem* Subsystem;
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void CreateSession();
	void onCreateSessionComplete(FName SessionName, bool Success);
	void onDestroySessionComplete(FName SessionName, bool Success);
	void onFindSessionsComplete(bool Success);
	void onJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	FHostingData HostingData;

	FString DesiredPlayerName;

	int SettingsScreenResolution;
	int SettingsFrameRate;
	int SettingsGraphicalQuality;
	int SettingsShadowsQuality;
	int SettingsAntialiasingQuality;
	int SettingsBloomQuality;
	int SettingsCarDetails;

	bool bIsCreatingSession = false;
};
