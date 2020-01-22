// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyRacePlayerState.h"

#include "UnrealNetwork.h"


void ASkyRacePlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASkyRacePlayerState, PlayerColorHex);
	DOREPLIFETIME(ASkyRacePlayerState, bIsReady);
}

void ASkyRacePlayerState::OnRep_PlayerColorHex()
{
	UE_LOG(LogTemp, Warning, TEXT("Replicated color hex"));
	bColorPending = false;
}