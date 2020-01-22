// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyRaceGameInstance.h"

#include "UObject/ConstructorHelpers.h"
#include "Classes/Kismet/GameplayStatics.h"

#include "OnlineSubsystem.h"
#include "Online.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterface.h"

#include "Blueprint/UserWidget.h"

#include "SkyRacePlayerController.h"

const static FName SESSION_NAME = NAME_GameSession;

USkyRaceGameInstance::USkyRaceGameInstance(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuClassFinder(TEXT("/Game/Menus/MainMenu/WBP_MainMenu"));
	if (!ensure(MainMenuClassFinder.Class != nullptr)) return;
	MainMenuClass = MainMenuClassFinder.Class;

	LoadSettings(SettingsScreenResolution, SettingsFrameRate, SettingsGraphicalQuality, SettingsShadowsQuality, SettingsAntialiasingQuality, SettingsBloomQuality, SettingsCarDetails);
}

void USkyRaceGameInstance::Init()
{
	//UEngine* Engine = GetEngine();
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Test"));

	Subsystem = IOnlineSubsystem::Get(FName("Steam"));
	if (Subsystem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Online Subsystem: %s"), *Subsystem->GetSubsystemName().ToString());
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Found Session Interface"));
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USkyRaceGameInstance::onCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &USkyRaceGameInstance::onDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USkyRaceGameInstance::onFindSessionsComplete);		
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USkyRaceGameInstance::onJoinSessionComplete);
		}

		ELoginStatus::Type LogInStatus = Subsystem->GetIdentityInterface()->GetLoginStatus(0);
		bool bIsLoggedIn = LogInStatus == ELoginStatus::Type::LoggedIn;
		UE_LOG(LogTemp, Warning, TEXT("Steam logged in: %s"), *FString(bIsLoggedIn ? "Yes" : "No"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find Online Subsystem!"));
	}
}

void USkyRaceGameInstance::LoadMainMenu()
{
	if (!ensure(MainMenuClass != nullptr)) return;

	MainMenu = CreateWidget<UMainMenu>(this, MainMenuClass);
	if (!ensure(MainMenu != nullptr)) return;

	MainMenu->Setup();
	MainMenu->SetMenuInterface(this);
}

void USkyRaceGameInstance::HostServer(FString ServerName, uint16 MaxPlayers, bool isLanMatch, bool isPrivate, FString Password)
{
	if (bIsCreatingSession) return;
	bIsCreatingSession = true;

	FHostingData newHostingData;
	newHostingData.ServerName = ServerName;
	newHostingData.MaxPlayers = MaxPlayers;
	newHostingData.isLanMatch = isLanMatch;
	newHostingData.isPrivate = isPrivate;
	newHostingData.Password = Password;
	HostingData = newHostingData;

	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
}

void USkyRaceGameInstance::onCreateSessionComplete(FName SessionName, bool Success)
{
	bIsCreatingSession = false;

	if (!Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create session"));
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr) return;
		
	MainMenu->DestroyMenu(World);
	//World->GetAuthGameMode()->bUseSeamlessTravel = true;

	FString MapName("/Game/Maps/DefaultMap");
	EOnlineMode OnlineMode = HostingData.isLanMatch ? EOnlineMode::LAN : EOnlineMode::Online;
	FString StartURL = FString::Printf(TEXT("%s%s%s"), *MapName, OnlineMode != EOnlineMode::Offline ? TEXT("?listen") : TEXT(""), OnlineMode == EOnlineMode::LAN ? TEXT("?bIsLanMatch") : TEXT(""));

	World->ServerTravel(StartURL);
}

void USkyRaceGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		//SessionSearch->bIsLanQuery = true;

		UE_LOG(LogTemp, Warning, TEXT("Searching for sessions..."));

		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void USkyRaceGameInstance::onFindSessionsComplete(bool Success) {

	if (MainMenu == nullptr) return;
	if (!MainMenu->IsValidLowLevel()) return;

	if (Success && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Finished find Session"));

		TArray <FServerData> ServerData;
		for (FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found Session: %s"), *SearchResult.GetSessionIdStr());

			FServerData Data;

			Data.SessionId = SearchResult.GetSessionIdStr();

			if (!SearchResult.Session.SessionSettings.Get(TEXT("ServerName"), Data.ServerName)) Data.ServerName = FString("<unnamed server>");
			
			Data.HostName = SearchResult.Session.OwningUserName;
			Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;

			//if (!SearchResult.Session.SessionSettings.Get(TEXT("SecondsRemaining"), Data.SecondsRemaining)) 
			Data.SecondsRemaining = 0;

			Data.PingInMs = SearchResult.PingInMs;

			Data.isLanMatch = SearchResult.Session.SessionSettings.bIsLANMatch;

			//if (!SearchResult.Session.SessionSettings.Get(TEXT("IsPrivate"), Data.isPrivate)) 
			Data.isPrivate = false;

			ServerData.Add(Data);
		}

		/*//Dummy 1
		FServerData Dummy1;
		Dummy1.ServerName = FString("Dummy 1");
		Dummy1.MaxPlayers = 4;
		Dummy1.CurrentPlayers = 2;
		Dummy1.SecondsRemaining = 120;
		Dummy1.PingInMs = 1;
		Dummy1.isLanMatch = true;
		Dummy1.isPrivate = false;
		ServerData.Add(Dummy1);

		//Dummy 2
		FServerData Dummy2;
		Dummy2.ServerName = FString("Dummy 2");
		Dummy2.MaxPlayers = 8;
		Dummy2.CurrentPlayers = 0;
		Dummy1.SecondsRemaining = 347;
		Dummy2.PingInMs = 1;
		Dummy2.isLanMatch = false;
		Dummy2.isPrivate = true;
		ServerData.Add(Dummy2);*/

		MainMenu->SetServerList(ServerData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find sessions"));
	}
}

void USkyRaceGameInstance::JoinServer(FString inSessionId)
{
	UE_LOG(LogTemp, Warning, TEXT("Game instance wants to join server!"));

	//UEngine* Engine = GetEngine();
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Game instance wants to join server!"));

	if (!SessionInterface.IsValid()) return;
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SessionInterface is valid!"));

	if (!SessionSearch.IsValid()) return;
	UE_LOG(LogTemp, Warning, TEXT("SessionSearch is valid!"));
	
	TOptional<FOnlineSessionSearchResult> SearchResult;
	for (FOnlineSessionSearchResult& ItSearchResult : SessionSearch->SearchResults)
	{
		if (ItSearchResult.GetSessionIdStr().Equals(inSessionId))
		{
			SearchResult = ItSearchResult;
		}
	}

	if (!SearchResult.IsSet()) return;

	if (!SearchResult.GetValue().Session.SessionSettings.Get(TEXT("ServerName"), HostingData.ServerName)) HostingData.ServerName = FString("<unnamed server>");

	UE_LOG(LogTemp, Warning, TEXT("Joining Session with ID %s"), *SearchResult.GetValue().GetSessionIdStr());
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Joining Session with ID %s"), *SessionSearch->SearchResults[Index].GetSessionIdStr()));
	SessionInterface->JoinSession(0, SESSION_NAME, SearchResult.GetValue());
}

void USkyRaceGameInstance::onDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success && bIsCreatingSession)
	{
		CreateSession();
	}
}

void USkyRaceGameInstance::CreateSession()
{
	if (!SessionInterface.IsValid()) return;

	//UEngine* Engine = GetEngine();
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Now creating new session..."));

	UE_LOG(LogTemp, Warning, TEXT("ServerName: %s"), *HostingData.ServerName);
	UE_LOG(LogTemp, Warning, TEXT("MaxPlayers: %d"), HostingData.MaxPlayers);
	if (HostingData.isLanMatch)
	{
		UE_LOG(LogTemp, Warning, TEXT("isLanMatch: true"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("isLanMatch: false"));
	}
	if (HostingData.isPrivate)
	{
		UE_LOG(LogTemp, Warning, TEXT("isPrivate: true"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("isPrivate: false"));
	}
	UE_LOG(LogTemp, Warning, TEXT("Password: %s"), *HostingData.Password);

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = HostingData.isLanMatch;
	SessionSettings.NumPublicConnections = HostingData.MaxPlayers;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = !HostingData.isLanMatch;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.Set(FName("ServerName"), HostingData.ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(FName("SecondsRemaining"), 0, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(FName("IsPrivate"), HostingData.isPrivate, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
}

void USkyRaceGameInstance::onJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//UEngine* Engine = GetEngine();
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Join session complete!"));

	UE_LOG(LogTemp, Warning, TEXT("Join Session complete!"));

	if (!SessionInterface.IsValid()) return;

	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Join session valid!"));
	UE_LOG(LogTemp, Warning, TEXT("Join Session valid!"));

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not get connect String"));
		return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	//FString ConnectionURL(TEXT("192.168.0.101:7777"));
	FString ConnectionURL = Address;//FString::Printf(TEXT("%s?Name=%s"), *Address, *DesiredPlayerName);
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Initializing Client Travel to: %s"), *ConnectionURL));
	
	MainMenu->DestroyMenu(GetWorld());

	UE_LOG(LogTemp, Warning, TEXT("Initializing Client Travel to: %s"), *ConnectionURL);
	PlayerController->ClientTravel(ConnectionURL, ETravelType::TRAVEL_Absolute);

}

void USkyRaceGameInstance::JumpToGame()
{
	FString MapName("/Game/Maps/DefaultMap");
	//FString GameName("/Game/Maps/MySkyRaceGameModeBase.MySkyRaceGameModeBase_C");
	EOnlineMode OnlineMode = EOnlineMode::LAN;
	//"%s?game=%s%s%s"
	FString StartURL = FString::Printf(TEXT("%s%s%s"), *MapName, OnlineMode != EOnlineMode::Offline ? TEXT("?listen") : TEXT(""), OnlineMode == EOnlineMode::LAN ? TEXT("?bIsLanMatch") : TEXT(""));
	// StartURL = FString::Printf(TEXT("/Game/Maps/%s?game=%s%s%s?%s=%d%s"), *MapName, *GameType, GameInstance->GetOnlineMode() != EOnlineMode::Offline ? TEXT("?listen") : TEXT(""), GameInstance->GetOnlineMode() == EOnlineMode::LAN ? TEXT("?bIsLanMatch") : TEXT(""), *AShooterGameMode::GetBotsCountOptionName(), BotsCountOpt, bIsRecordingDemo ? TEXT("?DemoRec") : TEXT(""));

	//UEngine* Engine = GetEngine();
	//Engine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("ServerTravel to: %s"), *StartURL));

	UWorld* World = GetWorld();
	World->SeamlessTravel(StartURL);
}

void USkyRaceGameInstance::LeaveServer()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(World->GetFirstPlayerController());
	if (PC == nullptr) return;

	if (SessionInterface.IsValid()) //PC->HasAuthority() && 
	{
		SessionInterface->DestroySession(SESSION_NAME);
	}

	PC->ClientTravel(FString("/Game/Menus/MainMenu/MainMenuLevel"), ETravelType::TRAVEL_Absolute);
}

void USkyRaceGameInstance::Host()
{
	//UEngine* Engine = GetEngine();
	//if (!ensure(Engine != nullptr)) return;

	//Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	World->ServerTravel("/Game/Maps/DefaultMap?listen");
}
void USkyRaceGameInstance::Join(const FString& Address)
{
	//UEngine* Engine = GetEngine();
	//if (!ensure(Engine != nullptr)) return;

	//Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void USkyRaceGameInstance::SetScreenResolution(int inVal)
{
	SettingsScreenResolution = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
		case 0:
			PC->ConsoleCommand(TEXT("r.ScreenPercentage 25"), true);
			break;
		case 1:
			PC->ConsoleCommand(TEXT("r.ScreenPercentage 50"), true);
			break;
		case 2:
			PC->ConsoleCommand(TEXT("r.ScreenPercentage 75"), true);
			break;
		default:
			PC->ConsoleCommand(TEXT("r.ScreenPercentage 100"), true);
	}
}

void USkyRaceGameInstance::SetFrameRate(int inVal)
{
	SettingsFrameRate = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
	case 0:
		PC->ConsoleCommand(TEXT("t.MaxFPS 15"), true);
		break;
	case 1:
		PC->ConsoleCommand(TEXT("t.MaxFPS 30"), true);
		break;
	case 2:
		PC->ConsoleCommand(TEXT("t.MaxFPS 60"), true);
		break;
	default:
		PC->ConsoleCommand(TEXT("t.MaxFPS 144"), true);
	}
}

void USkyRaceGameInstance::SetGraphicalQuality(int inVal)
{
	SettingsGraphicalQuality = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
	case 0:
		PC->ConsoleCommand(TEXT("sg.PostProcessQuality 0"), true);
		break;
	case 1:
		PC->ConsoleCommand(TEXT("sg.PostProcessQuality 1"), true);
		break;
	case 2:
		PC->ConsoleCommand(TEXT("sg.PostProcessQuality 2"), true);
		break;
	default:
		PC->ConsoleCommand(TEXT("sg.PostProcessQuality 3"), true);
	}
}

void USkyRaceGameInstance::SetShadowsQuality(int inVal)
{
	SettingsShadowsQuality = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
	case 0:
		PC->ConsoleCommand(TEXT("sg.ShadowQuality 0"), true);
		break;
	case 1:
		PC->ConsoleCommand(TEXT("sg.ShadowQuality 1"), true);
		break;
	case 2:
		PC->ConsoleCommand(TEXT("sg.ShadowQuality 2"), true);
		break;
	default:
		PC->ConsoleCommand(TEXT("sg.ShadowQuality 3"), true);
	}
}

void USkyRaceGameInstance::SetAntialiasingQuality(int inVal)
{
	SettingsAntialiasingQuality = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
	case 0:
		PC->ConsoleCommand(TEXT("r.PostProcessAAQuality 0"), true);
		break;
	case 1:
		PC->ConsoleCommand(TEXT("r.PostProcessAAQuality 1"), true);
		break;
	case 2:
		PC->ConsoleCommand(TEXT("r.PostProcessAAQuality 2"), true);
		break;
	default:
		PC->ConsoleCommand(TEXT("r.PostProcessAAQuality 3"), true);
	}

	SetBloomQuality(SettingsBloomQuality);
}

void USkyRaceGameInstance::SetBloomQuality(int inVal)
{
	SettingsBloomQuality = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
	case 0:
		PC->ConsoleCommand(TEXT("r.BloomQuality 2"), true);
		break;
	case 1:
		PC->ConsoleCommand(TEXT("r.BloomQuality 4"), true);
		break;
	case 2:
		PC->ConsoleCommand(TEXT("r.BloomQuality 5"), true);
		break;
	default:
		PC->ConsoleCommand(TEXT("r.BloomQuality 5"), true);
	}
}

void USkyRaceGameInstance::SetCarDetails(int inVal)
{
	SettingsCarDetails = inVal;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC == nullptr) return;

	switch (inVal) {
	case 0:
		PC->ConsoleCommand(TEXT("r.SkeletalMeshLODBias 3"), true);
		break;
	case 1:
		PC->ConsoleCommand(TEXT("r.SkeletalMeshLODBias 2"), true);
		break;
	case 2:
		PC->ConsoleCommand(TEXT("r.SkeletalMeshLODBias 1"), true);
		break;
	default:
		PC->ConsoleCommand(TEXT("r.SkeletalMeshLODBias 0"), true);
	}
}

void USkyRaceGameInstance::SaveSettings()
{
	USettingsSaveGame* SaveGameInstance = Cast<USettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass()));
	if (SaveGameInstance == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("Created new savegame"));
	SaveGameInstance->Save(SettingsScreenResolution, SettingsFrameRate, SettingsGraphicalQuality, SettingsShadowsQuality, SettingsAntialiasingQuality, SettingsBloomQuality, SettingsCarDetails);
}

void USkyRaceGameInstance::LoadSettings(int& ScreenResolution, int& FrameRate, int& GraphicalQuality, int& ShadowsQuality, int& AntiAliasingQuality, int& BloomQuality, int& CarDetails)
{
	USettingsSaveGame* SaveGameInstance;

	SaveGameInstance = Cast<USettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(*SAVE_SLOT_NAME, SAVE_USER_INDEX));
	if (SaveGameInstance == nullptr)
	{
			SaveGameInstance = Cast<USettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass()));	
	}

	if (SaveGameInstance == nullptr) return;

	SaveGameInstance->GetSettings(SettingsScreenResolution, SettingsFrameRate, SettingsGraphicalQuality, SettingsShadowsQuality, SettingsAntialiasingQuality, SettingsBloomQuality, SettingsCarDetails);

	SetScreenResolution(SettingsScreenResolution);
	SetFrameRate(SettingsFrameRate);
	SetGraphicalQuality(SettingsGraphicalQuality);
	SetShadowsQuality(SettingsShadowsQuality);
	SetAntialiasingQuality(SettingsAntialiasingQuality);
	SetBloomQuality(SettingsBloomQuality);
	SetCarDetails(SettingsCarDetails);

	ScreenResolution = SettingsScreenResolution;
	FrameRate = SettingsFrameRate;
	GraphicalQuality = SettingsGraphicalQuality;
	ShadowsQuality = SettingsShadowsQuality;
	AntiAliasingQuality = SettingsAntialiasingQuality;
	BloomQuality = SettingsBloomQuality;
	CarDetails = SettingsCarDetails;
	
}