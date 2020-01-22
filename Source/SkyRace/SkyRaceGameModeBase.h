// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SkyRaceGameModeBase.generated.h"

const float PORTAL_DISTANCE_TO_BORDER = 1500.f;

USTRUCT()
struct FCarPaint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* CarPaintMaterialInstance;

	UPROPERTY(EditAnywhere)
	FString GuiColor;
};

/**
 * 
 */
UCLASS()
class SKYRACE_API ASkyRaceGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ASkyRaceGameModeBase();

	void PostLogin(APlayerController* NewPlayer) override;

	void Logout(AController* Exiting) override;

	TWeakObjectPtr<class APortalActor> GetScorePortalPtr() { return ScorePortalPtr; };
	
	void RespawnScorePortal();

	TArray<FString> GetAvailableColors();

	bool CheckPlayersReady();
	bool CheckPlayersComplete();
	int GetMaxNumOfPlayers();
	int GetCurrentNumOfPlayers();

	class ACarPawn* RespawnCar(FString PlayerColorHex);

	void StartGameDeferred();

protected:
	virtual void BeginPlay() override;
	void spawnPortal(int portalType, bool isRandLocation);
	void onTimeToSpawnPortal();

	float gameDimensions;

	FTimerHandle portalSpawnTimer;
	FRandomStream randomGenerator;
	TSubclassOf<class APortalActor> PortalBlueprint;

	TWeakObjectPtr<class APortalActor> ScorePortalPtr;

	FVector GetRandomPortalLocation();
	
	void StartGame();
	

	uint32 NumberOfPlayers = 0;

	TSubclassOf<class ACarPawn> CarBP;

	//move this to a better place
	TArray<FString> PlayerColorsHex;

	TArray<FTransform> SpawnLocations;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	TArray<FCarPaint> PlayerColors;

	void LoadPlayerColors();

	class UMaterialInterface* GetPaintMaterialByHex(FString inHex);

	TSubclassOf<class AHUD> CustomHUDClass;

	void SetGameStatusGo();

	float GameEndTimestamp;

	float GameDuration = 300.f;

	void TerminateGame();

};
