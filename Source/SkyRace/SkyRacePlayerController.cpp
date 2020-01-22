// Fill out your copyright notice in the Description page of Project Settings.

#include "SkyRacePlayerController.h"

#include "Menus/LobbyMenu.h"
#include "Menus/ScoreBoard.h"
#include "Menus/InGameMenu.h"
#include "MissileActor.h"
#include "CarPawn.h"
#include "PortalActor.h"
#include "MessageBoard.h"

#include "UObject/ConstructorHelpers.h"
#include "UnrealNetwork.h"
#include "TimerManager.h"

#include "SkyRaceGameStateBase.h"
#include "SkyRaceGameInstance.h"
#include "SkyRacePlayerState.h"
#include "SkyRaceGameModeBase.h"
#include "HUD/SkyRaceHUD.h"

#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectIterator.h"


ASkyRacePlayerController::ASkyRacePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> LobbyMenuClassFinder(TEXT("/Game/Menus/Lobby/WBP_LobbyMenu"));
	if (LobbyMenuClassFinder.Class == nullptr) return;
	LobbyMenuClass = LobbyMenuClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> ScoreBoardClassFinder(TEXT("/Game/Menus/ScoreBoard/WBP_ScoreBoard"));
	if (ScoreBoardClassFinder.Class == nullptr) return;
	ScoreBoardClass = ScoreBoardClassFinder.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuClassFinder(TEXT("/Game/Menus/InGame/WBP_InGameMenu"));
	if (InGameMenuClassFinder.Class == nullptr) return;
	InGameMenuClass = InGameMenuClassFinder.Class;
}

void ASkyRacePlayerController::BeginPlay()
{
	Super::BeginPlay();
}


void ASkyRacePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*if (PlayerState == nullptr) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	if (PS->bColorPending)
	{
		UE_LOG(LogTemp, Warning, TEXT("Color pending: YES"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Color pending: NO"));
	}*/
}

void ASkyRacePlayerController::OpenLobby()
{
	Client_OpenLobby(AllColorsHex);
}

void ASkyRacePlayerController::OpenLobby(const TArray<FString>& PlayerColors)
{
	Client_OpenLobby(PlayerColors);
}

void ASkyRacePlayerController::Client_OpenLobby_Implementation(const TArray<FString>& PlayerColors)
{
	if (LobbyMenuClass == nullptr) return;

	if (LobbyMenu == nullptr)
	{
		LobbyMenu = CreateWidget<ULobbyMenu>(this, LobbyMenuClass);
		if (LobbyMenu == nullptr) return;
	}
	
	AllColorsHex = PlayerColors;

	USkyRaceGameInstance* GI = Cast<USkyRaceGameInstance>(GetGameInstance());
	if (GI == nullptr) return;

	LobbyMenu->Setup(PlayerColors, GI->GetGameName());
	LobbyMenu->RefreshPlayerList();
}

void ASkyRacePlayerController::CloseLobby()
{
	Client_CloseLobby();
}

void ASkyRacePlayerController::Client_CloseLobby_Implementation()
{
	if (LobbyMenu == nullptr) return;
	LobbyMenu->DestroyMenu();
	LobbyMenu = nullptr;
}

void ASkyRacePlayerController::ChangePlayerColor(FString inColor)
{
	if (!HasAuthority())
	{
		if (PlayerState == nullptr) return;

		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
		if (!PS) return;

		UE_LOG(LogTemp, Warning, TEXT("Setting color to pending"));

		PS->bColorPending = true;

		UWorld* World = GetWorld();
		if (World == nullptr) return;

		if (ColorPendingTimer.IsValid()) World->GetTimerManager().ClearTimer(ColorPendingTimer);
		World->GetTimerManager().SetTimer(ColorPendingTimer, this, &ASkyRacePlayerController::ResetColorPendingStatus, 3.f);
	}
	Server_ChangePlayerColor(inColor);
}

void ASkyRacePlayerController::Server_ChangePlayerColor_Implementation(const FString& inColor)
{
	/*
		As this is a server rpc for the replicated playercontroller, 
		the local variables regarding the playercontroller contain 
		the values of the client, NOT THE SERVER 
		(especially the PlayerId will refer to the client ID)
	*/

	if (!HasAuthority()) return;

	if (PlayerState == nullptr) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	UE_LOG(LogTemp, Warning, TEXT("Server received new color %s from player %d"), *inColor, PS->PlayerId);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
	if (!GM) return;

	if (GM->GetAvailableColors().Contains(inColor))
	{
		//Grant new color
		PS->PlayerColorHex = inColor;
	}
	else
	{
		//Notify client about denial
		Client_DenyNewColor(inColor);
	}
}

bool ASkyRacePlayerController::Server_ChangePlayerColor_Validate(const FString& inColor)
{
	return true;
}

void ASkyRacePlayerController::Client_DenyNewColor_Implementation(const FString& inColor)
{
	UE_LOG(LogTemp, Warning, TEXT("Server denied color"));
	ResetColorPendingStatus();
}

void ASkyRacePlayerController::ResetColorPendingStatus()
{
	if (PlayerState == nullptr) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	PS->bColorPending = false;
}

void ASkyRacePlayerController::SetReadyStatus(bool bIsReady)
{
	Server_SetReadyStatus(bIsReady);
}

void ASkyRacePlayerController::Server_SetReadyStatus_Implementation(bool bIsReady)
{
	if (!HasAuthority()) return;

	if (PlayerState == nullptr) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	PS->bIsReady = bIsReady;

	/*if (bIsReady)
	{
		UWorld* World = GetWorld();
		if (World == nullptr) return;

		ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
		if (!GM) return;

		GM->CheckStartGame();
	}*/
}

bool ASkyRacePlayerController::Server_SetReadyStatus_Validate(bool bIsReady)
{
	return true;
}

void ASkyRacePlayerController::OpenScoreBoard()
{
	Client_OpenScoreBoard();
}

void ASkyRacePlayerController::Client_OpenScoreBoard_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Opening ScoreBoard"));

	if (ScoreBoardClass == nullptr) return;

	if (ScoreBoard == nullptr)
	{
		ScoreBoard = CreateWidget<UScoreBoard>(this, ScoreBoardClass);
		if (ScoreBoard == nullptr) return;
	}

	CloseLobby();
	CloseInGameMenu();

	ScoreBoard->Setup();
	//ScoreBoard->RefreshPlayerList();
}

void ASkyRacePlayerController::CloseScoreBoard()
{
	Client_CloseScoreBoard();
}

void ASkyRacePlayerController::Client_CloseScoreBoard_Implementation()
{
	if (ScoreBoard == nullptr) return;
	ScoreBoard->DestroyMenu();
	ScoreBoard = nullptr;
}

void ASkyRacePlayerController::CloseHUD()
{
	ASkyRaceHUD* SkyRaceHUD = Cast<ASkyRaceHUD>(GetHUD());
	if (SkyRaceHUD == nullptr) return;

	SkyRaceHUD->Hide();
	SkyRaceHUD->Destroy();
}

void ASkyRacePlayerController::TerminateGame()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (RespawnTimer.IsValid()) World->GetTimerManager().ClearTimer(RespawnTimer);

	for (TObjectIterator<AActor> It; It; ++It)
	{
		ACarPawn* CarPawn = Cast<ACarPawn>(*It);
		if (CarPawn != nullptr) CarPawn->ExplodeAndDie();

		AMissileActor* MissileActor = Cast<AMissileActor>(*It);
		if (MissileActor != nullptr) MissileActor->ExplodeAndDie();

		APortalActor* PortalActor = Cast<APortalActor>(*It);
		if (PortalActor != nullptr) PortalActor->ExplodeAndDie();
	}

	Client_TerminateGame();
}

void ASkyRacePlayerController::Client_TerminateGame_Implementation()
{
	CloseHUD();
}

void ASkyRacePlayerController::DisableControls()
{
	//GetPawn()->DisableInput(this);
	Client_DisableControls();
}
void ASkyRacePlayerController::Client_DisableControls_Implementation()
{
	APawn* TempPawn = GetPawn();
	if (TempPawn == nullptr) return;
	
	TempPawn->DisableInput(this);
}

void ASkyRacePlayerController::EnableControls()
{
	Client_EnableControls();
}
void ASkyRacePlayerController::Client_EnableControls_Implementation()
{
	APawn* ThePawn = GetPawn();
	if (ThePawn == nullptr) return;

	ThePawn->EnableInput(this);

	//DEVELOP
	//UWorld* World = GetWorld();
}

void ASkyRacePlayerController::ResetClock(float inDuration)
{
	Client_ResetClock(inDuration);
}

void ASkyRacePlayerController::Client_ResetClock_Implementation(float inDuration)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	GameEndTimestamp = World->GetTimeSeconds() + inDuration;
	ClockOverride = -1.f;
}

void ASkyRacePlayerController::StopClock(float inValue)
{
	Client_StopClock(inValue);
}

void ASkyRacePlayerController::Client_StopClock_Implementation(float inValue)
{
	ClockOverride = inValue;
}

float ASkyRacePlayerController::GetTimeRemaining()
{
	if (ClockOverride < -SMALL_NUMBER)
	{
		if (GameEndTimestamp < SMALL_NUMBER) return 0.0f;

		UWorld* World = GetWorld();
		if (World == nullptr) return 0.f;

		return GameEndTimestamp - World->GetTimeSeconds();
	}
	else
	{
		return ClockOverride;
	}
}

void ASkyRacePlayerController::LostPawn()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ACarPawn* CarPawn = Cast<ACarPawn>(GetPawn());
	if (CarPawn == nullptr) return;

	FRotator CameraRotation = CarPawn->GetCameraRotation();

	CarPawn->GotHit();
	CarPawn->ExplodeAndDie();

	ClientSetRotation(CameraRotation);

	if (RespawnTimer.IsValid()) World->GetTimerManager().ClearTimer(RespawnTimer);
	World->GetTimerManager().SetTimer(RespawnTimer, this, &ASkyRacePlayerController::Respawn, 2.f);
}

void ASkyRacePlayerController::Respawn()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
	if (!GM) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	ACarPawn* NewCar = GM->RespawnCar(PS->PlayerColorHex);
	if (NewCar == nullptr) return;

	NewCar->activateSpecialProtection(5.f);
	Possess(NewCar);
}

void ASkyRacePlayerController::OpenInGameMenu()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(World->GetGameState());
	if (!GS) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	if (!GS->IsPlaying() || PS->bIsSpectator) return;

	if (InGameMenuClass == nullptr) return;

	if (InGameMenu == nullptr)
	{
		InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
		if (InGameMenu == nullptr) return;
		InGameMenu->Setup();
	}
}

void ASkyRacePlayerController::CloseInGameMenu()
{
	if (InGameMenu == nullptr) return;
	InGameMenu->DestroyMenu();
	InGameMenu = nullptr;
}

void ASkyRacePlayerController::BackToLobby()
{
	Server_BackToLobby();

	CloseInGameMenu();
	CloseHUD();
	OpenLobby();
}
void ASkyRacePlayerController::Server_BackToLobby_Implementation()
{
	ACarPawn* CarPawn = Cast<ACarPawn>(GetPawn());
	if (CarPawn != nullptr) CarPawn->ExplodeAndDie();

	ChangeState(NAME_Spectating);

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PlayerState);
	if (!PS) return;

	PS->bIsReady = false;
	PS->bIsSpectator = true;

	
}
bool ASkyRacePlayerController::Server_BackToLobby_Validate()
{
	return true;
}