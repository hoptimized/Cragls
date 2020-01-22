// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyRaceGameStateBase.h"

#include "SkyRacePlayerState.h"

#include "UnrealNetwork.h"


void ASkyRaceGameStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASkyRaceGameStateBase, GameStatus);
}

TArray<FRankingListTuple> ASkyRaceGameStateBase::GetPlayersByScore()
{
	TArray<FRankingListTuple> Output;

	TMap<int, TArray<ASkyRacePlayerState*>> PlayersByScore;
	for (APlayerState*& It : PlayerArray)
	{
		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It);
		if (!PS) break;

		//Create the nested array, if it does not exist
		if (!PlayersByScore.Contains((int)PS->Score)) PlayersByScore.Add((int)PS->Score, TArray<ASkyRacePlayerState*>());

		PlayersByScore[(int)PS->Score].Add(PS);
	}

	PlayersByScore.KeySort([](int A, int B) {
		return A > B;
	});

	int CurrentRank = 1;
	int LastScore = -1;
	for (auto& ScoreGroup : PlayersByScore)
	{
		for (auto& ItPS : ScoreGroup.Value)
		{
			FRankingListTuple NewEntry;
			NewEntry.Rank = CurrentRank;
			NewEntry.Score = ScoreGroup.Key;
			NewEntry.PlayerName = ItPS->GetPlayerName();
			NewEntry.PlayerId = ItPS->PlayerId;

			Output.Add(NewEntry);

			ItPS->SetRank(CurrentRank);
		}

		++CurrentRank;
	}

	return Output;
}
