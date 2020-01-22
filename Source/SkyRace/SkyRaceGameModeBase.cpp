// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyRaceGameModeBase.h"

#include "CarPawn.h"
#include "PortalActor.h"
#include "SkyRaceGameStateBase.h"
#include "SkyRacePlayerState.h"
#include "HUD/SkyRaceHUD.h"
#include "SkyRacePlayerController.h"
#include "SkyRaceSpectatorPawn.h"
#include "SkyRaceGameInstance.h"

#include "Blueprint/UserWidget.h"

#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EngineUtils.h"

ASkyRaceGameModeBase::ASkyRaceGameModeBase()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ASkyRaceGameStateBase::StaticClass();
	PlayerStateClass = ASkyRacePlayerState::StaticClass();
	PlayerControllerClass = ASkyRacePlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<ASkyRaceSpectatorPawn> SpectatorClassFinder(TEXT("/Game/Actors/Spectator/BP_SpectatorPawn.BP_SpectatorPawn_C"));
	SpectatorClass = SpectatorClassFinder.Class;

	static ConstructorHelpers::FClassFinder<ACarPawn> PlayerPawnClassFinder(TEXT("/Game/Actors/Car/BP_CarPawn"));
	CarBP = PlayerPawnClassFinder.Class;
	//DefaultPawnClass = CarBP;

	static ConstructorHelpers::FClassFinder<ASkyRaceHUD> HudClassFinder(TEXT("/Game/Menus/HUD/WBP_HUD"));
	CustomHUDClass = HudClassFinder.Class;

	gameDimensions = 7500.f;

	UClass* BPClass = LoadObject<UClass>(nullptr, TEXT("/Game/Actors/Portal/BP_Portal.BP_Portal_C"));
	PortalBlueprint = BPClass;

	

	float SpawnRadius = 6500.f;
	float SpawnY = 87.f;
	float SpawnLocationsCount = 8;
	FTransform BaseTransform = FTransform(FRotator(0.f, 0.f, 0.f), FVector(-SpawnRadius, 0.f, SpawnY), FVector::OneVector);

	TArray<float> SpawnAngles = TArray<float>();
	SpawnAngles.Add(0.f);
	SpawnAngles.Add(180.f);
	SpawnAngles.Add(90.f);
	SpawnAngles.Add(270.f);
	SpawnAngles.Add(45.f);
	SpawnAngles.Add(225.f);
	SpawnAngles.Add(135.f);
	SpawnAngles.Add(315.f);

	SpawnLocations = TArray<FTransform>();
	for (int32 i = 0; i < SpawnAngles.Num(); i++)
	{
		FVector TransformedLocation = BaseTransform.GetLocation().RotateAngleAxis(SpawnAngles[i], FVector(0.f, 0.f, 1.f));
		FRotator TransformedRotator = FRotator(0.f, SpawnAngles[i], 0.f);
		
		SpawnLocations.Add(FTransform(TransformedRotator, TransformedLocation, FVector::OneVector));
	}
}

void ASkyRaceGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerColorsHex.Num() == 0) LoadPlayerColors();

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;
	GS->GameStatus = GAME_STATUS_LOBBY;
}

void ASkyRaceGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(NewPlayer);
	if (!PC) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(NewPlayer->PlayerState);
	if (!PS) return;
	
	if (PlayerColorsHex.Num() == 0) LoadPlayerColors();

	//Get hold of the default pawn that should already have spawned
	APawn* TheDefaultPawn = PC->GetPawn();

	//Basic config
	PS->bIsSpectator = true;
	PC->ChangeState(NAME_Spectating);

	//Delete the default pawn
	if (TheDefaultPawn != nullptr) TheDefaultPawn->Destroy();

	//Randomly pick a color from available colors
	TArray<FString> AvailableColors = GetAvailableColors();
	PS->PlayerColorHex = AvailableColors[FMath::RandRange(0, AvailableColors.Num()-1)];
	
	//Open the lobby
	PC->OpenLobby(PlayerColorsHex);

	++NumberOfPlayers;
}

void ASkyRaceGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumberOfPlayers;
}

void ASkyRaceGameModeBase::StartGameDeferred()
{
	/*ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;

	if (GS->GameStatus != GAME_STATUS_LOBBY) return;

	FTimerHandle GameStartTimer;
	GetWorldTimerManager().SetTimer(GameStartTimer, this, &ASkyRaceGameModeBase::StartGame, 5);

	GS->GameStatus = GAME_STATUS_LEAVING_LOBBY;*/

	StartGame();
}

void ASkyRaceGameModeBase::StartGame()
{
	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;
	GS->GameStatus = GAME_STATUS_STARTING;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	randomGenerator = FRandomStream(FDateTime().Now().ToUnixTimestamp());

	spawnPortal(PORTAL_TYPE_SCORE_PORTAL, false);
	World->GetTimerManager().SetTimer(portalSpawnTimer, this, &ASkyRaceGameModeBase::onTimeToSpawnPortal, 10.0f);

	//For each player in the (authorative) gamestate, update the widget
	uint16 i = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		//TODO: check if spawn location is valid


		ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(*It);
		if (!PC) break;

		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(PC->PlayerState);
		if (!PS) return;

		PS->Score = 0.f;

		//Delete the default pawn
		APawn* DefaultPawn = PC->GetPawn();
		if (DefaultPawn != nullptr) DefaultPawn->Destroy();

		//Close the lobby menu
		PC->CloseLobby();

		//Spawn the pawn
		ACarPawn* NewCar = World->SpawnActorDeferred<ACarPawn>(CarBP, SpawnLocations[i], nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		
		if (NewCar == nullptr) continue;
		
		NewCar->bDoCountdown = true;
		NewCar->SetEngineCutOff(true);
		NewCar->SetPaintMaterial(GetPaintMaterialByHex(PS->PlayerColorHex));
		
		UGameplayStatics::FinishSpawningActor(NewCar, SpawnLocations[i]);

		PC->Possess(NewCar);

		//Reset the timer
		PC->StopClock(GameDuration);

		//Activate the HUD
		if (CustomHUDClass == nullptr) return;
		PC->ClientSetHUD(CustomHUDClass);

		i++;
	}

	//Enable controls after 5 seconds
	FTimerHandle EnableControlsTimer;
	World->GetTimerManager().SetTimer(EnableControlsTimer, this, &ASkyRaceGameModeBase::SetGameStatusGo, 5.f);
}

void ASkyRaceGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;

	if (GS->GameStatus == GAME_STATUS_PLAYING)
	{
		if (World->GetTimeSeconds() > GameEndTimestamp)
		{
			TerminateGame();
		}
		else
		{
			int NumOfPlayingControllers = 0;
			for (APlayerState* ItPS : GS->PlayerArray)
			{
				if (!ItPS->bIsSpectator) ++NumOfPlayingControllers;
			}
			if (NumOfPlayingControllers == 0) TerminateGame();
		}
	}
}

void ASkyRaceGameModeBase::onTimeToSpawnPortal()
{
	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;

	if (GS->GameStatus != GAME_STATUS_PLAYING) return;

	TArray<int> PortalRandomDistribution;

	PortalRandomDistribution.Add(PORTAL_TYPE_SPEED_PORTAL);
	PortalRandomDistribution.Add(PORTAL_TYPE_SPEED_PORTAL);

	PortalRandomDistribution.Add(PORTAL_TYPE_SHIELD_PORTAL);

	PortalRandomDistribution.Add(PORTAL_TYPE_MISSILE_PORTAL);
	PortalRandomDistribution.Add(PORTAL_TYPE_MISSILE_PORTAL);
	PortalRandomDistribution.Add(PORTAL_TYPE_MISSILE_PORTAL);
	PortalRandomDistribution.Add(PORTAL_TYPE_MISSILE_PORTAL);

	PortalRandomDistribution.Add(PORTAL_TYPE_SHOCK_PORTAL);

	spawnPortal(PortalRandomDistribution[randomGenerator.RandRange(0, PortalRandomDistribution.Num() - 1)], true); // RandomGenerator.RandRange(1, 4)
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(portalSpawnTimer, this, &ASkyRaceGameModeBase::onTimeToSpawnPortal, randomGenerator.RandRange(3.f, 10.f));
	}

}

void ASkyRaceGameModeBase::spawnPortal(int portalType, bool isRandLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("Trying to spawn portal of type: %d"), portalType);

	UWorld* const World = GetWorld();
	if (World)
	{
		FVector spawnLocation;

		if (isRandLocation)
		{
			spawnLocation = GetRandomPortalLocation();
		}
		else
		{
			spawnLocation = FVector(0.f, 0.f, 0.f);
		}

		FRotator rotator = FRotator(0.f, 0.f, 0.f);
		FActorSpawnParameters spawnParameters;
		spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		APortalActor* newPortal = World->SpawnActor<APortalActor>(PortalBlueprint, spawnLocation, rotator, spawnParameters);
		if (newPortal == nullptr) return;
		newPortal->InitializePortal(portalType, 15.f);

		if (portalType == PORTAL_TYPE_SCORE_PORTAL) ScorePortalPtr = TWeakObjectPtr<APortalActor>(newPortal);
	}
}

FVector ASkyRaceGameModeBase::GetRandomPortalLocation()
{
	return FVector(randomGenerator.FRandRange(-(gameDimensions - PORTAL_DISTANCE_TO_BORDER), gameDimensions - PORTAL_DISTANCE_TO_BORDER), randomGenerator.FRandRange(-(gameDimensions - PORTAL_DISTANCE_TO_BORDER), gameDimensions - PORTAL_DISTANCE_TO_BORDER), 0.f);
}

void ASkyRaceGameModeBase::RespawnScorePortal()
{
	spawnPortal(PORTAL_TYPE_SCORE_PORTAL, true);
}

TArray<FString> ASkyRaceGameModeBase::GetAvailableColors()
{
	TArray<FString> AvailableColors = PlayerColorsHex;

	if (GameState == nullptr) return AvailableColors;

	for (APlayerState*& It : GameState->PlayerArray)
	{
		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It);
		uint16 i = 0;
		int16 foundIndex = -1;
		do
		{
			if (AvailableColors[i].Equals(PS->PlayerColorHex, ESearchCase::IgnoreCase)) foundIndex = i;
			++i;
		} while (i < AvailableColors.Num() && foundIndex < 0);

		if (foundIndex >= 0)
		{
			AvailableColors.RemoveAt(foundIndex);
		}
	}

	return AvailableColors;
}

void ASkyRaceGameModeBase::LoadPlayerColors()
{
	if (PlayerColors.Num() == 0) return;


	PlayerColorsHex = TArray<FString>();
	for (auto It = PlayerColors.CreateConstIterator(); It; ++It)
	{
		FCarPaint ThisPaint = *It;
		PlayerColorsHex.Add(ThisPaint.GuiColor);
	}

	/*PlayerColors = TArray<FString>();
	PlayerColors.Add("#FF0000"); //Red
	PlayerColors.Add("#0031E2"); //Blue
	PlayerColors.Add("#3FD800"); //Green
	PlayerColors.Add("#FFEF2F"); //Yellow
	PlayerColors.Add("#C800EC"); //Purple
	PlayerColors.Add("#969696"); //Silver
	PlayerColors.Add("#ECB900"); //Gold
	PlayerColors.Add("#EA7700"); //Orange
	PlayerColors.Add("#FF5FBF"); //Pink
	PlayerColors.Add("#FFFFFF"); //White*/
}

UMaterialInterface* ASkyRaceGameModeBase::GetPaintMaterialByHex(FString inHex)
{
	if (PlayerColors.Num() == 0) return nullptr;

	for (auto It = PlayerColors.CreateConstIterator(); It; ++It)
	{
		FCarPaint ThisPaint = *It;
		if (ThisPaint.GuiColor.Equals(inHex, ESearchCase::IgnoreCase)) return ThisPaint.CarPaintMaterialInstance;
	}

	return nullptr;
}

bool ASkyRaceGameModeBase::CheckPlayersReady()
{
	bool bAllClear = true;
	for (APlayerState*& It : GameState->PlayerArray)
	{
		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It);
		if (!PS) continue;

		if (!PS->bIsReady)
		{
			bAllClear = false;
			break;
		}
	}

	return bAllClear;

	/*if (bAllClear)
	{
		UWorld* World = GetWorld();
		if (World == nullptr) return;

		USkyRaceGameInstance* GI = Cast<USkyRaceGameInstance>(GetGameInstance());
		if (GI == nullptr) return;

		if (World->GetNumPlayerControllers() == (uint32)GI->GetGameMaxPlayers())
		{
			StartGameDeferred();
		}
		else
		{
			ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(World->GetFirstPlayerController());
			if (PC == nullptr) return;

			PC->ShowStartAlertBox(true);
		}

	}*/
}

bool ASkyRaceGameModeBase::CheckPlayersComplete()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return false;

	USkyRaceGameInstance* GI = Cast<USkyRaceGameInstance>(GetGameInstance());
	if (GI == nullptr) return false;

	return (World->GetNumPlayerControllers() == (uint32)GI->GetGameMaxPlayers());
}

int ASkyRaceGameModeBase::GetMaxNumOfPlayers()
{
	USkyRaceGameInstance* GI = Cast<USkyRaceGameInstance>(GetGameInstance());
	if (GI == nullptr) return 0;

	return GI->GetGameMaxPlayers();
}

int ASkyRaceGameModeBase::GetCurrentNumOfPlayers()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return 0;

	return World->GetNumPlayerControllers();
}

void ASkyRaceGameModeBase::SetGameStatusGo()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(*It);
		if (!PC) continue;

		PC->ResetClock(GameDuration);

		ACarPawn* CarPawn = Cast<ACarPawn>(PC->GetPawn());
		if (CarPawn == nullptr) continue;

		CarPawn->SetEngineCutOff(false);
	}

	GameEndTimestamp = World->GetTimeSeconds() + GameDuration;

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;
	GS->GameStatus = GAME_STATUS_PLAYING;
}

void ASkyRaceGameModeBase::TerminateGame()
{
	UE_LOG(LogTemp, Warning, TEXT("Terminating Game"));

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->GetTimerManager().ClearTimer(portalSpawnTimer);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(*It);
		if (!PC) continue;
		
		PC->TerminateGame();

		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It->Get()->PlayerState);
		if (!PS) continue;

		PS->bIsReady = false;
		PS->bIsSpectator = true;
		PC->ChangeState(NAME_Spectating);

		PC->OpenScoreBoard();
	}

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(GameState);
	if (!GS) return;
	GS->GameStatus = GAME_STATUS_LOBBY;
}

ACarPawn* ASkyRaceGameModeBase::RespawnCar(FString PlayerColorHex)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return nullptr;

	if (SpawnLocations.Num() == 0) return nullptr;

	//Spawn the pawn
	FTransform SpawnTransform = SpawnLocations[FMath::RandRange(0, SpawnLocations.Num() - 1)];
	ACarPawn* NewCar = World->SpawnActorDeferred<ACarPawn>(CarBP, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	if (NewCar == nullptr) return nullptr;

	NewCar->bDoCountdown = false;
	NewCar->SetEngineCutOff(false);
	NewCar->SetPaintMaterial(GetPaintMaterialByHex(PlayerColorHex));

	UGameplayStatics::FinishSpawningActor(NewCar, SpawnTransform);

	return NewCar;
}