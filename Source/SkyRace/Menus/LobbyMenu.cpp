// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyMenu.h"

#include "Components/PanelWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

#include "SkyRaceGameInstance.h"
#include "SkyRaceGameModeBase.h"
#include "SkyRaceGameStateBase.h"
#include "SkyRacePlayerState.h"
#include "SkyRacePlayerController.h"
#include "SkyRaceSpectatorPawn.h"

#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"


ULobbyMenu::ULobbyMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerRowClassFinder(TEXT("/Game/Menus/Lobby/WBP_LobbyRow"));
	if (PlayerRowClassFinder.Class == nullptr) return;
	PlayerRowClass = PlayerRowClassFinder.Class;
}

bool ULobbyMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (ButtonExit == nullptr) return false;
	ButtonExit->OnClicked.AddDynamic(this, &ULobbyMenu::OnButtonExitClick);

	//--------------------------------------------------------------------------
	// Controls: client, ready?

	if (ButtonWait == nullptr) return false;
	ButtonWait->OnClicked.AddDynamic(this, &ULobbyMenu::OnButtonWaitClick);

	if (ButtonGo == nullptr) return false;
	ButtonGo->OnClicked.AddDynamic(this, &ULobbyMenu::OnButtonGoClick);

	//--------------------------------------------------------------------------
	// Controls: client, spectate

	if (ButtonSpectate == nullptr) return false;
	ButtonSpectate->OnClicked.AddDynamic(this, &ULobbyMenu::OnButtonSpectateClick);

	//--------------------------------------------------------------------------
	// Controls: server, start now

	if (ButtonSpectate == nullptr) return false;
	ButtonStartNow->OnClicked.AddDynamic(this, &ULobbyMenu::OnButtonStartNowClick);

	//--------------------------------------------------------------------------
	// PopUp: clients not ready

	if (ButtonNotReadyClose == nullptr) return false;
	ButtonNotReadyClose->OnClicked.AddDynamic(this, &ULobbyMenu::ClosePopUp);

	//--------------------------------------------------------------------------
	// PopUp: not reached max num of players

	if (ButtonDoNotStart == nullptr) return false;
	ButtonDoNotStart->OnClicked.AddDynamic(this, &ULobbyMenu::ClosePopUp);

	if (ButtonStartAnyways == nullptr) return false;
	ButtonStartAnyways->OnClicked.AddDynamic(this, &ULobbyMenu::OnButtonStartAnywaysClick);
	
	return true;
}

void ULobbyMenu::Setup(TArray<FString> inPlayerColors, FString GameName)
{
	PlayerColors = inPlayerColors;

	//---------------------------------------
	// Get references

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	//---------------------------------------
	// Configure the lobby umg

	if (PopUpBoxWindow != nullptr) PopUpBoxWindow->SetVisibility(ESlateVisibility::Collapsed);
	if (ServerNameText != nullptr) ServerNameText->SetText(FText::FromString(GameName));

	//---------------------------------------
	// Download player info

	RefreshPlayerList();
	bIsListReady = true;
	World->GetTimerManager().SetTimer(RefreshTimer, this, &ULobbyMenu::doRefresh, 1.0f);

	//---------------------------------------
	// Add to viewport

	this->bIsFocusable = true;
	this->AddToViewport();

	FInputModeGameAndUI InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputModeData);
	PC->bShowMouseCursor = true;

}

void ULobbyMenu::doRefresh()
{
	RefreshPlayerList();
	UWorld* World = GetWorld();
	if (World == nullptr) return;
	World->GetTimerManager().SetTimer(RefreshTimer, this, &ULobbyMenu::doRefresh, 1.0f);
}

void ULobbyMenu::RefreshPlayerList()
{
	/*	
		Outdated algorithm!
		1. Make a copy of local player array
		2. For each player in the gamestate
			a. if does not exist locally, create rowWidget and create record in TMap
			b. set playername, waiting status
			c. set/change color ONLY IF CHANGED
			d. delete player set from original array
		3. Delete all row widgets of the players that are still in the original array
		4. copy the temp array to the permanent array
	*/

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	AGameStateBase* tempGS = World->GetGameState();
	if (tempGS == nullptr) return;
	ASkyRaceGameStateBase* GS = Cast <ASkyRaceGameStateBase>(tempGS);
	if (!GS) return;

	APlayerController* LocalPlayerController = World->GetFirstPlayerController();
	if (LocalPlayerController == nullptr) return;

	APlayerState* LocalPlayerStateTemp = LocalPlayerController->PlayerState;
	if (LocalPlayerStateTemp == nullptr) return;
	ASkyRacePlayerState* LocalPlayerState = Cast<ASkyRacePlayerState>(LocalPlayerStateTemp);
	if (!LocalPlayerState) return;

	//UE_LOG(LogTemp, Warning, TEXT("PlayerStates:"));

	//For each player in the (authorative) gamestate, update the widget
	for (APlayerState*& It : GS->PlayerArray)
	{
		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It);
		if (!PS) break;

		//UE_LOG(LogTemp, Warning, TEXT("%s = %s"), *(FString::FromInt(PS->PlayerId)), *(PS->PlayerColorHex));

		ULobbyPlayerRow* ThisPlayerRow = nullptr;
		for (auto*& RowIt : PlayerListWidget->GetAllChildren())
		{
			ULobbyPlayerRow* RowItCasted = Cast<ULobbyPlayerRow>(RowIt);
			if (RowItCasted->GetPlayerId() == PS->PlayerId) ThisPlayerRow = RowItCasted;
		}

		//New player / damaged dataset, create widget
		if (ThisPlayerRow == nullptr)
		{
			ThisPlayerRow = CreateWidget<ULobbyPlayerRow>(this, PlayerRowClass);
			if (ThisPlayerRow == nullptr) break;

			if (PlayerColors.Num()==0) break;

			ThisPlayerRow->Init(this, PS->PlayerId, PlayerColors);
			ThisPlayerRow->Update(PS->GetPlayerName(), PS->PlayerColorHex, PS->bIsReady, PS->bIsSpectator, PS->PlayerId == LocalPlayerState->PlayerId);
			PlayerListWidget->AddChild(ThisPlayerRow);
		}

		//Update (if not local player (local player is updated via RPCs))
		if (PS->PlayerId != LocalPlayerState->PlayerId || !LocalPlayerState->bColorPending) ThisPlayerRow->Update(PS->GetPlayerName(), PS->PlayerColorHex, PS->bIsReady, PS->bIsSpectator, PS->PlayerId == LocalPlayerState->PlayerId);
	}

	//Remove all widgets that represent old players
	for (auto*& RowIt : PlayerListWidget->GetAllChildren())
	{
		ULobbyPlayerRow* RowItCasted = Cast<ULobbyPlayerRow>(RowIt);
		bool found = false;
		for (auto*& PlayerIt : GS->PlayerArray)
		{
			if (PlayerIt->PlayerId == RowItCasted->GetPlayerId()) found = true;
		}
		if (!found) {
			PlayerListWidget->RemoveChild(RowIt);
		}
	}

}

void ULobbyMenu::onColorPickerInput(int32 PlayerId, FString NewColorHex)
{
	if (!bIsListReady) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) return;

	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(PlayerController);
	if (!PC) return;

	APlayerState* PS = PC->PlayerState;
	if (PS == nullptr) return;

	if (PS->PlayerId == PlayerId) PC->ChangePlayerColor(NewColorHex);
}


//TEMP TEMP TEMP
void ULobbyMenu::NativeTick(const FGeometry & MyGeometry, float InDeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast <ASkyRaceGameStateBase>(World->GetGameState());
	if (!GS) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	if (LobbyControlsSwitcher == nullptr) return;

	if (ButtonStartNow != nullptr)
	{
		ButtonStartNow->SetVisibility(PC->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (GS->IsPlaying())
	{
		if (ButtonSpectate != nullptr) LobbyControlsSwitcher->SetActiveWidget(ButtonSpectate);
	}
	else
	{
		if (ClientReadyBox != nullptr) LobbyControlsSwitcher->SetActiveWidget(ClientReadyBox);
	}
}

void ULobbyMenu::OnButtonGoClick()
{
	SetReadyStatus(true);
}

void ULobbyMenu::OnButtonWaitClick()
{
	SetReadyStatus(false);
}

void ULobbyMenu::OnButtonSpectateClick()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PC = Cast<APlayerController>(World->GetFirstPlayerController());
	if (!PC) return;

	ASkyRaceSpectatorPawn* SpectatorPawn = Cast<ASkyRaceSpectatorPawn>(PC->GetSpectatorPawn());
	if (SpectatorPawn == nullptr) return;

	SpectatorPawn->SwitchCameraMode();
}

void ULobbyMenu::SetReadyStatus(bool bIsReady)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) return;

	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(PlayerController);
	if (!PC) return;

	PC->SetReadyStatus(bIsReady);
}

void ULobbyMenu::OnButtonStartAnywaysClick()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
	if (GM == nullptr) return;

	GM->StartGameDeferred();

	ClosePopUp();
}

void ULobbyMenu::OnButtonStartNowClick()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
	if (GM == nullptr) return;

	if (GM->CheckPlayersReady())
	{
		if (GM->CheckPlayersComplete())
		{
			GM->StartGameDeferred();
		}
		else
		{
			if (PopUpBoxSwitcher != nullptr)
			{
				if (TextCurrentPlayers != nullptr) TextCurrentPlayers->SetText(FText::FromString(FString::FromInt(GM->GetCurrentNumOfPlayers())));
				if (TextMaxPlayers != nullptr) TextMaxPlayers->SetText(FText::FromString(FString::FromInt(GM->GetMaxNumOfPlayers())));
				PopUpBoxSwitcher->SetActiveWidget(FewClientsBox);
			}
			if (PopUpBoxWindow != nullptr) PopUpBoxWindow->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (PopUpBoxWindow != nullptr) PopUpBoxWindow->SetVisibility(ESlateVisibility::Visible);
		if(PopUpBoxSwitcher != nullptr && ClientsNotReadyBox != nullptr) PopUpBoxSwitcher->SetActiveWidget(ClientsNotReadyBox);
	}
}

void ULobbyMenu::ClosePopUp()
{
	if (PopUpBoxWindow == nullptr) return;
	PopUpBoxWindow->SetVisibility(ESlateVisibility::Collapsed);
}

void ULobbyMenu::OnButtonExitClick()
{
	USkyRaceGameInstance* GI = Cast<USkyRaceGameInstance>(GetGameInstance());
	if (GI == nullptr) return;

	GI->LeaveServer();
}

void ULobbyMenu::DestroyMenu()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	this->RemoveFromViewport();
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	FInputModeGameOnly InputModeData;
	PC->SetInputMode(InputModeData);
	PC->bShowMouseCursor = false;
}