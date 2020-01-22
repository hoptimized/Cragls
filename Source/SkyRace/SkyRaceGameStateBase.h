// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SkyRaceGameStateBase.generated.h"

const uint16 GAME_STATUS_LOBBY = 1;
const uint16 GAME_STATUS_LEAVING_LOBBY = 2;
const uint16 GAME_STATUS_STARTING = 3;
const uint16 GAME_STATUS_PLAYING = 4;

USTRUCT()
struct FRankingListTuple
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	int Rank;

	UPROPERTY()
	int Score;

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int PlayerId;
};

/**
 * 
 */
UCLASS()
class SKYRACE_API ASkyRaceGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

protected:

public:

	UPROPERTY(replicated)
	uint16 GameStatus;

	UFUNCTION(BlueprintCallable)
	bool IsPlaying() { return GameStatus == GAME_STATUS_PLAYING; }

	UFUNCTION(BlueprintCallable)
	bool IsInLobby() { return GameStatus == GAME_STATUS_LOBBY || GameStatus == GAME_STATUS_LEAVING_LOBBY; }

	TArray<FRankingListTuple> GetPlayersByScore();

};
