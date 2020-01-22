// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine.h"
#include "Misc/DefaultValueHelper.h"
#include "TimerManager.h"

#include "ServerRow.h"



UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> ServerRowClassFinder(TEXT("/Game/Menus/MainMenu/WBP_ServerRow"));
	if (ServerRowClassFinder.Class == nullptr) return;
	ServerRowClass = ServerRowClassFinder.Class;
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;
	
	//Main menu buttons
	if (MainMenuPlay == nullptr) return false;
	MainMenuPlay->OnClicked.AddDynamic(this, &UMainMenu::MainMenuPlayOnClick);

	if (MainMenuSettings == nullptr) return false;
	MainMenuSettings->OnClicked.AddDynamic(this, &UMainMenu::MainMenuSettingsOnClick);

	if (MainMenuExit == nullptr) return false;
	MainMenuExit->OnClicked.AddDynamic(this, &UMainMenu::MainMenuExitOnClick);


	//Serverlist buttons
	if (JoinNowButton == nullptr) return false;
	JoinNowButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

	if (HostButton == nullptr) return false;
	HostButton->OnClicked.AddDynamic(this, &UMainMenu::HostButtonOnClick);


	//HostMenu buttons
	if (HostNowButton == nullptr) return false;
	HostNowButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);
	

	//Back button
	if (BackButton == nullptr) return false;
	BackButton->SetVisibility(ESlateVisibility::Hidden);
	BackButton->OnClicked.AddDynamic(this, &UMainMenu::BackButtonOnClick);

	/*if (HostButton == nullptr) return false;
	if (HostNowButton == nullptr) return false;
	if (JoinButton == nullptr) return false;
	if (JoinNowButton == nullptr) return false;

	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);
	HostNowButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);
	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);
	JoinNowButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);*/

	return true;
}

void UMainMenu::Setup()
{
	this->bIsFocusable = true;
	this->AddToViewport();

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr) return;

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = true;
}

void UMainMenu::SetMenuInterface(IMenuInterface* inMenuInterface)
{
	this->MenuInterface = inMenuInterface;
}

void UMainMenu::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (MenuInterface == nullptr) return;
	if (MenuSwitcher == nullptr) return;
	if (MainMenu == nullptr) return;
	if (SteamNotFound == nullptr) return;

	if (MenuInterface->GetIsLoggedIn())
	{
		if (MenuSwitcher->GetActiveWidget() == SteamNotFound) MenuSwitcher->SetActiveWidget(MainMenu);
	}
	else
	{
		MenuSwitcher->SetActiveWidget(SteamNotFound);
	}
}

void UMainMenu::switchToMenu(UWidget* NewMenu)
{
	UWidget* ActiveWidget = MenuSwitcher->GetActiveWidget();
	if (ActiveWidget == NewMenu) return;

	nextMenu = NewMenu;

	if ( 	ActiveWidget == ServerListMenu && nextMenu == HostMenu
		 ||	ActiveWidget == HostMenu && nextMenu == ServerListMenu
		 || ActiveWidget == ServerListMenu && nextMenu == MainMenu
		 || ActiveWidget == SettingsMenu && nextMenu == MainMenu)
	{
		if (MenuFadeInAnimation != nullptr) PlayAnimation(MenuFadeInAnimation, 0.f, 1, EUMGSequencePlayMode::Reverse, 1.f)->OnSequenceFinishedPlaying().AddUObject(this, &UMainMenu::fadeOutCallback);
	}
	else if (ActiveWidget == MainMenu && nextMenu == ServerListMenu
		 || ActiveWidget == MainMenu && nextMenu == SettingsMenu)
	{
		if (MainToServerlistTransition == nullptr) return;
		PlayAnimation(MainToServerlistTransition)->OnSequenceFinishedPlaying().AddUObject(this, &UMainMenu::fadeOutCallback);
	}
}

void UMainMenu::fadeInCallback(UUMGSequencePlayer& Player)
{
	/*UWidget* ActiveWidget = MenuSwitcher->GetActiveWidget();
	if (ActiveWidget == nextMenu) return;

	if (nextMenu == MainMenu)
	{
		PlayAnimation(MenuGrowAnimation,0.f,1,EUMGSequencePlayMode::Reverse,1.f);
	}*/
}

void UMainMenu::fadeOutCallback(UUMGSequencePlayer& Player)
{
	if (MenuSwitcher == nullptr) return;
	if (nextMenu == nullptr) return;

	UWidget* ActiveWidget = MenuSwitcher->GetActiveWidget();
	if (ActiveWidget == nextMenu) return;

	//Configure back button
	if (BackButton != nullptr)
	{
		if (nextMenu == MainMenu)
		{
			BackButton->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			BackButton->SetVisibility(ESlateVisibility::Visible);
		}
	}

	//Switch menu page
	MenuSwitcher->SetActiveWidget(nextMenu);

	//Call init routines
	if (nextMenu == ServerListMenu)
	{
		OpenServerlistMenu();
	}

	//Fade in content / shrink
	if (nextMenu == MainMenu)
	{
		if (MainToServerlistTransition == nullptr) return;
		PlayAnimation(MainToServerlistTransition, 0.f, 1, EUMGSequencePlayMode::Reverse, 1.f)->OnSequenceFinishedPlaying().AddUObject(this, &UMainMenu::shrinkCallback);
	}
	else
	{
		if (MenuFadeInAnimation == nullptr) return;
		PlayAnimation(MenuFadeInAnimation)->OnSequenceFinishedPlaying().AddUObject(this, &UMainMenu::fadeInCallback);
	}


}

void UMainMenu::growCallback(UUMGSequencePlayer& Player)
{

}

void UMainMenu::shrinkCallback(UUMGSequencePlayer& Player)
{
	if (nextMenu == MainMenu)
	{
		if (MainToServerlistTransition == nullptr) return;
		//PlayAnimation(MainToServerlistTransition, 0.f, 1, EUMGSequencePlayMode::Reverse)->OnSequenceFinishedPlaying().AddUObject(this, &UMainMenu::fadeOutCallback);
	}
}

void UMainMenu::BackButtonOnClick()
{
	
	if (MenuSwitcher == nullptr) return;

	UWidget* ActiveWidget = MenuSwitcher->GetActiveWidget();

	if (ActiveWidget == ServerListMenu)
	{
		if (MainMenu == nullptr) return;
		switchToMenu(MainMenu);
	}
	else if (ActiveWidget == HostMenu)
	{
		if (ServerListMenu == nullptr) return;
		switchToMenu(ServerListMenu);
	}
	else if (ActiveWidget == SettingsMenu)
	{
		if (MainMenu == nullptr) return;
		switchToMenu(MainMenu);
	}
}

/* --------------------------------------------------------------------------------------------------------- */
/* START PAGE */

void UMainMenu::MainMenuPlayOnClick()
{
	switchToMenu(ServerListMenu);
}

void UMainMenu::OpenServerlistMenu()
{
	RefreshServerList();
}

void UMainMenu::MainMenuSettingsOnClick()
{
	switchToMenu(SettingsMenu);
}

void UMainMenu::MainMenuExitOnClick()
{
	FGenericPlatformMisc::RequestExit(false);
}

/* --------------------------------------------------------------------------------------------------------- */
/* SERVER LIST */

void UMainMenu::SetServerList(TArray<FServerData> ServerNames)
{
	if (destroying) return;

	UWorld* World = this->GetWorld();
	if (World == nullptr) return;

	if (ServerList == nullptr) return;
	if (!ServerList->IsValidLowLevel()) return;

	bool bFoundOldSession = false;
	ServerList->ClearChildren();

	uint32 i = 0;
	for (const FServerData& ServerData : ServerNames)
	{
		UServerRow* ServerRow = CreateWidget<UServerRow>(this, ServerRowClass);
		if (ServerRow == nullptr) return;

		//UE_LOG(LogTemp, Warning, TEXT("%d"), SteamUser()->GetSteamID().ConvertToUint64());

		ServerRow->ServerName->SetText(FText::FromString(ServerData.ServerName));
		ServerRow->HostName->SetText(FText::FromString(ServerData.HostName));
		ServerRow->NumOfPlayers->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers)));
		ServerRow->TimeRemaining->SetText(FText::FromString(FString::FromInt(ServerData.SecondsRemaining)));
		ServerRow->Ping->SetText(FText::FromString(FString::FromInt(ServerData.PingInMs)));

		if (ServerData.isLanMatch)
		{
			ServerRow->Network->SetText(FText::FromString("LAN"));
		}
		else
		{
			ServerRow->Network->SetText(FText::FromString("Internet"));
		}

		if (ServerData.isPrivate)
		{
			ServerRow->IsPrivate->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ServerRow->IsPrivate->SetVisibility(ESlateVisibility::Hidden);
		}
	
		ServerRow->Setup(this, i, ServerData.SessionId);

		ServerList->AddChild(ServerRow);

		if (SelectedSession.IsSet() && SelectedSession.GetValue() == ServerData.SessionId)
		{
			bFoundOldSession = true;
			ServerRow->Select();
		}

		++i;
	}

	if (!bFoundOldSession) SelectedSession.Reset();

	RefreshServerList();
}

void UMainMenu::RefreshServerList()
{
	UWorld* World = this->GetWorld();
	if (World == nullptr) return;

	World->GetTimerManager().SetTimer(RefreshTimer, this, &UMainMenu::RefreshServerListCallback, 2);
}

void UMainMenu::RefreshServerListCallback()
{
	if (MenuInterface == nullptr) return;
	MenuInterface->RefreshServerList();
}

void UMainMenu::SelectSession(FString inSessionId)
{
	SelectedSession = inSessionId;

	if (ServerList == nullptr) return;
	
	for (UWidget*& It : ServerList->GetAllChildren())
	{
		UServerRow* ServerRow = Cast<UServerRow>(It);
		if (ServerRow == nullptr) continue;

		ServerRow->SetColorAndOpacity(ServerRow->GetSessionId().Equals(inSessionId) ? FColor::FromHex("#FFFFFF") : FColor::FromHex("#00FFB6"));
	}
}

void UMainMenu::HostButtonOnClick()
{
	UE_LOG(LogTemp, Warning, TEXT("HostButtonClick"));

	if (destroying) return;

	if (HostMenu == nullptr) return;
	switchToMenu(HostMenu);
	//if (MenuFadeInAnimation == nullptr) return;
	//PlayAnimation(MenuFadeInAnimation,0.f,1,EUMGSequencePlayMode::Reverse,1.f)->OnSequenceFinishedPlaying().AddUObject(this, &UMainMenu::OpenHostMenu);
}

void UMainMenu::JoinServer()
{
	UE_LOG(LogTemp, Warning, TEXT("JoinButtonClick"));

	if (destroying) return;

	if (!SelectedSession.IsSet()) return;
	if (MenuInterface == nullptr) return;
	//if (PlayerNameTextbox == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("Selected Session %s"), *SelectedSession.GetValue());

	MenuInterface->JoinServer(SelectedSession.GetValue());
}

/* --------------------------------------------------------------------------------------------------------- */
/* HOST MENU */

void UMainMenu::OpenHostMenu()
{
	//if (MenuSwitcher == nullptr) return;
	//if (HostMenu == nullptr) return;

	//MenuSwitcher->SetActiveWidget(HostMenu);
	//PlayAnimation(MenuFadeInAnimation);
}

void UMainMenu::HostServer()
{
	if (destroying) return;

	if (MenuInterface == nullptr) return;
	if (GameNameTextbox == nullptr) return;
	if (MaxPlayersComboBox == nullptr) return;
	if (NetworkComboBox == nullptr) return;
	if (IsPrivateComboBox == nullptr) return;
	if (PasswordTextBox == nullptr) return;

	FString ServerName;
	uint16 MaxPlayers;
	bool isLanMatch;
	bool isPrivate;
	FString Password;

	ServerName = GameNameTextbox->GetText().ToString();
	if (ServerName.IsEmpty()) ServerName = FString("<unnamed server>");


	FString SelectedMaxPlayers = MaxPlayersComboBox->GetSelectedOption();
	if (SelectedMaxPlayers.Equals("2"))
	{
		MaxPlayers = 2;
	}
	else if (SelectedMaxPlayers.Equals("3"))
	{
		MaxPlayers = 3;
	}
	else if (SelectedMaxPlayers.Equals("5"))
	{
		MaxPlayers = 5;
	}
	else if (SelectedMaxPlayers.Equals("6"))
	{
		MaxPlayers = 6;
	}
	else if (SelectedMaxPlayers.Equals("7"))
	{
		MaxPlayers = 7;
	}
	else if (SelectedMaxPlayers.Equals("8"))
	{
		MaxPlayers = 8;
	}
	else
	{
		MaxPlayers = 4;
	}

	isLanMatch = NetworkComboBox->GetSelectedOption().Equals("LAN");

	isPrivate = IsPrivateComboBox->GetSelectedOption().Equals("Yes");

	Password = FMD5::HashAnsiString(*(PasswordTextBox->GetText().ToString()));

	MenuInterface->HostServer(ServerName, MaxPlayers, isLanMatch, isPrivate, Password);
}

/* --------------------------------------------------------------------------------------------------------- */
/* OTHER */

void UMainMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (InLevel == nullptr) return;
	UE_LOG(LogTemp, Warning, TEXT("Removing level from world %s"), *(InLevel->GetFName().ToString()));
	DestroyMenu(InWorld);
}

void UMainMenu::DestroyMenu(UWorld* InWorld)
{
	destroying = true;

	UE_LOG(LogTemp, Warning, TEXT("DestroyingMenu"));
	if (InWorld == nullptr) return;
	UE_LOG(LogTemp, Warning, TEXT("Removing Menu from viewport"));
	this->RemoveFromViewport();

	UE_LOG(LogTemp, Warning, TEXT("Getting player controller"));
	APlayerController* PlayerController = InWorld->GetFirstPlayerController();
	if (PlayerController == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("Setting input mode"));
	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = false;
}