// Fill out your copyright notice in the Description page of Project Settings.


#include "RankingList.h"

#include "HUD/RankingListEntry.h"
#include "SkyRaceGameStateBase.h"
#include "SkyRacePlayerState.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

#include "UObject/ConstructorHelpers.h"

void URankingList::NativeConstruct()
{
	Super::NativeConstruct();
}

void URankingList::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (NameList == nullptr) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(World->GetGameState());
	if (!GS) return;

	TArray<FRankingListTuple> RankingData = GS->GetPlayersByScore();

	bool bHasChanged = false;

	if (RankingData.Num() == NameList->GetAllChildren().Num())
	{
		int i = 0;
		for (auto& It : NameList->GetAllChildren())
		{
			URankingListEntry* RankingListEntry = Cast<URankingListEntry>(It);
			if (!RankingListEntry) continue;

			if (i < RankingData.Num() && RankingListEntry->PlayerId == RankingData[i].PlayerId)
			{
				if (!RankingListEntry->Ranking->GetText().ToString().Equals(FString::FromInt(RankingData[i].Rank) + ".")) bHasChanged = true;
				if (!RankingListEntry->NameAndScore->GetText().ToString().Equals(RankingData[i].PlayerName + " (" + FString::FromInt(RankingData[i].Score) + ")")) bHasChanged = true;

				if (bHasChanged) break;
			}
			else
			{
				bHasChanged = true;
				break;
			}

			++i;
		}
	}
	else
	{
		bHasChanged = true;
	}

	if (bHasChanged)
	{
		NameList->ClearChildren();

		for (int i=0;i<RankingData.Num();i++)
		{
			URankingListEntry* RowWidget = CreateWidget<URankingListEntry>(this, EntryClass);
			if (RowWidget == nullptr) return;
			RowWidget->PlayerId = RankingData[i].PlayerId;
			RowWidget->Ranking->SetText(FText::FromString(FString(FString::FromInt(RankingData[i].Rank) + ".")));
			RowWidget->NameAndScore->SetText(FText::FromString(RankingData[i].PlayerName + " (" + FString::FromInt(RankingData[i].Score) + ")"));

			NameList->AddChild(RowWidget);
		}
	}




	/*

	//TODO: build a map for the association of ranking list entries with RowWidgets. Don't do linear search all the time
	TMap<int, URankingListEntry*> PlayerIdToRowWidget;
	TMap<URankingListEntry*, int> RowWidgetToPlayerId;
	TMap<URankingListEntry*, int> RowWidgetToRowIndex;

	//Produce a ranking list sorted by score (key = score; value = array of FPlayerRankingData (PlayerId, PlayerName)) --> why a nested TArray? Because keys need to be unique but multiple players may have the same score
	TMap<int16, TArray<FPlayerRankingData>> SortedRankingList = TMap<int16, TArray<FPlayerRankingData>>();
	for (APlayerState*& It : GS->PlayerArray)
	{
		//Gather data
		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It);

		if (!PS->bIsSpectator)
		{
			FPlayerRankingData RankingData;
			RankingData.PlayerId = PS->PlayerId;
			RankingData.PlayerName = PS->GetPlayerName();

			//Create the nested array, if it does not exist
			if (!SortedRankingList.Contains((int16)PS->Score)) SortedRankingList.Add((int16)PS->Score, TArray<FPlayerRankingData>());

			//Push the row entry
			SortedRankingList[(int16)PS->Score].Add(RankingData);
		}
	}
	SortedRankingList.KeySort([](int16 A, int16 B) {
		return A > B;
	});

	//UE_LOG(LogTemp, Warning, TEXT("RANKING LIST:"));
	//for (auto& ScoreGroup : SortedRankingList)
	//{
	//	for (auto& PlayerRow : ScoreGroup.Value)
	//	{
	//		/UE_LOG(LogTemp, Warning, TEXT("Score = %d, ID = %d"), ScoreGroup.Key, PlayerRow.PlayerId);
	//	}
	//}

	//Update score of existing rows and insert new rows for new players
	int currentRank = 1;
	int i = 0;
	for (auto& ScoreGroup : SortedRankingList)
	{
		for (auto& PlayerRow : ScoreGroup.Value)
		{
			URankingListEntry* RowWidget = nullptr;

			//Find the corresponding row widget
			for (auto& ItRaw : NameList->GetAllChildren())
			{
				URankingListEntry* It = Cast<URankingListEntry>(ItRaw);
				if (!It) continue;

				if (It->PlayerId == PlayerRow.PlayerId) RowWidget = It;
			}

			//If row widget does not exist, create it
			if (RowWidget == nullptr)
			{
				RowWidget = CreateWidget<URankingListEntry>(this, EntryClass);
				if (RowWidget == nullptr) return;
				if (RowWidget->NameAndScore == nullptr) continue;
				RowWidget->PlayerId = PlayerRow.PlayerId;

				NameList->AddChild(RowWidget);
			}

			//Update the row widget
			FString CombinedString =  PlayerRow.PlayerName + " (" + FString::FromInt(ScoreGroup.Key) + ")";
			RowWidget->Ranking->SetText(FText::FromString(FString(FString::FromInt(currentRank) + ".")));
			RowWidget->NameAndScore->SetText(FText::FromString(CombinedString));

			//Save association of PlayerId and RowWidget for later lookup
			PlayerIdToRowWidget.Add(PlayerRow.PlayerId, RowWidget);
			RowWidgetToPlayerId.Add(RowWidget, PlayerRow.PlayerId);
			RowWidgetToRowIndex.Add(RowWidget, i);

			i++;
		}

		++currentRank;
	}

	//Remove old rows
	for (auto*& RowWidgetIt : NameList->GetAllChildren())
	{
		URankingListEntry* RowWidget = Cast<URankingListEntry>(RowWidgetIt);
		if (!RowWidget) continue;
		if (!RowWidgetToPlayerId.Contains(RowWidget)) NameList->RemoveChild(RowWidget);
	}

	//Sort the rows
	bool bHasChanged = false;
	i = 0;
	for (auto& ItRaw : NameList->GetAllChildren())
	{
		URankingListEntry* It = Cast<URankingListEntry>(ItRaw);
		if (!It) continue;

		//UE_LOG(LogTemp, Warning, TEXT("RowWidget of Player %d has the index %d, Label = %s"), It->PlayerId, NameList->GetChildIndex(It), *(It->NameAndScore->GetText().ToString()));

		if (RowWidgetToRowIndex[It] != i)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Moving player %d to position %d"), It->PlayerId, i);
			//NameList->ShiftChild(RowWidgetToRowIndex[It], It);
			bHasChanged = true;
			break;
		}

		i++;
	}

	if (bHasChanged)
	{
		NameList->ClearChildren();
		for (auto& ScoreGroup : SortedRankingList)
		{
			for (auto& PlayerRow : ScoreGroup.Value)
			{
				NameList->AddChild(PlayerIdToRowWidget[PlayerRow.PlayerId]);
			}
		}
	}

	*/
}