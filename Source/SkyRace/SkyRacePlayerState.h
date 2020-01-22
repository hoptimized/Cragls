// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SkyRacePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API ASkyRacePlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerColorHex)
	FString PlayerColorHex;

	UPROPERTY(replicated)
	bool bIsReady;

	bool bColorPending;

	UFUNCTION(BlueprintCallable)
	int GetRank() { return Rank; };

	void SetRank(int inRank) { Rank = inRank; };

protected:

	UFUNCTION()
	void OnRep_PlayerColorHex();

	int Rank;
};
