// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SkyRacePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API ASkyRacePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ASkyRacePlayerController();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void Client_OpenLobby(const TArray<FString>& PlayerColors);
	void OpenLobby(const TArray<FString>& PlayerColors);
	void OpenLobby();

	UFUNCTION(Client, Reliable)
	void Client_CloseLobby();
	void CloseLobby();

	UFUNCTION(Client, Reliable)
	void Client_TerminateGame();
	void TerminateGame();

	void CloseHUD();

	UFUNCTION(Client, Reliable)
	void Client_OpenScoreBoard();
	void OpenScoreBoard();

	UFUNCTION(Client, Reliable)
	void Client_CloseScoreBoard();
	void CloseScoreBoard();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ChangePlayerColor(const FString& inColor);
	void ChangePlayerColor(FString inColor);

	UFUNCTION(Client, Reliable)
	void Client_DenyNewColor(const FString& inColor);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetReadyStatus(bool bIsReady);
	void SetReadyStatus(bool bIsReady);

	UFUNCTION(Client, Reliable)
	void Client_DisableControls();
	void DisableControls();

	UFUNCTION(Client, Reliable)
	void Client_EnableControls();
	void EnableControls();

	UFUNCTION(Client, Reliable)
	void Client_ResetClock(float inDuration);
	void ResetClock(float inDuration);

	UFUNCTION(Client, Reliable)
	void Client_StopClock(float inValue);
	void StopClock(float inValue);

	UFUNCTION(BlueprintCallable)
	float GetTimeRemaining();

	void LostPawn();

	void OpenInGameMenu();

	UFUNCTION(BlueprintCallable)
	void CloseInGameMenu();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_BackToLobby();

	UFUNCTION(BlueprintCallable)
	void BackToLobby();

protected:
	TSubclassOf<class UUserWidget> LobbyMenuClass;
	TSubclassOf<class UUserWidget> ScoreBoardClass;
	TSubclassOf<class UUserWidget> InGameMenuClass;

	UPROPERTY()
	class ULobbyMenu* LobbyMenu;

	UPROPERTY()
	class UScoreBoard* ScoreBoard;

	UPROPERTY()
	class UInGameMenu* InGameMenu;

	void ResetColorPendingStatus();
	FTimerHandle ColorPendingTimer;

	float GameEndTimestamp = -1.f;
	float ClockOverride = -1.f;

	FTimerHandle RespawnTimer;
	void Respawn();

	TArray<FString> AllColorsHex;
};
