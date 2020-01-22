// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreBoard.h"

#include "Menus/ScoreBoardRow.h"

#include "SkyRaceGameModeBase.h"
#include "SkyRaceGameStateBase.h"
#include "SkyRacePlayerController.h"
#include "SkyRacePlayerState.h"
#include "SkyRaceGameInstance.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "UObject/ConstructorHelpers.h"

UScoreBoard::UScoreBoard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerRowClassFinder(TEXT("/Game/Menus/ScoreBoard/WBP_ScoreBoardRow"));
	if (PlayerRowClassFinder.Class == nullptr) return;
	PlayerRowClass = PlayerRowClassFinder.Class;
}

bool UScoreBoard::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (ButtonNextRound == nullptr) return false;
	ButtonNextRound->OnClicked.AddDynamic(this, &UScoreBoard::OnButtonNextRoundClick);

	if (ButtonExit == nullptr) return false;
	ButtonExit->OnClicked.AddDynamic(this, &UScoreBoard::OnButtonExitClick);

	return true;
}

void UScoreBoard::Setup()
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

void UScoreBoard::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (PlayerListWidget == nullptr) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(World->GetGameState());
	if (!GS) return;

	TArray<FRankingListTuple> RankingData = GS->GetPlayersByScore();

	bool bHasChanged = false;

	if (RankingData.Num() == PlayerListWidget->GetAllChildren().Num())
	{
		int i = 0;
		for (auto& It : PlayerListWidget->GetAllChildren())
		{
			UScoreBoardRow* ScoreBoardRow = Cast<UScoreBoardRow>(It);
			if (!ScoreBoardRow) continue;

			if (i < RankingData.Num() && ScoreBoardRow->PlayerName->GetText().ToString().Equals(RankingData[i].PlayerName))
			{
				if (!ScoreBoardRow->PlayerRanking->GetText().ToString().Equals(FString::FromInt(RankingData[i].Rank) + ".")) bHasChanged = true;
				if (!ScoreBoardRow->PlayerScore->GetText().ToString().Equals(FString::FromInt(RankingData[i].Score))) bHasChanged = true;

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
		PlayerListWidget->ClearChildren();

		for (int i = 0; i < RankingData.Num(); i++)
		{
			if (i == 0) WinnerText->SetText(FText::FromString(RankingData[i].PlayerName));

			UScoreBoardRow* ScoreBoardRow = CreateWidget<UScoreBoardRow>(this, PlayerRowClass);
			if (ScoreBoardRow == nullptr) return;

			ScoreBoardRow->PlayerRanking->SetText(FText::FromString(FString(FString::FromInt(RankingData[i].Rank) + ".")));
			ScoreBoardRow->PlayerName->SetText(FText::FromString(RankingData[i].PlayerName));
			ScoreBoardRow->PlayerScore->SetText(FText::FromString(FString::FromInt(RankingData[i].Score)));

			PlayerListWidget->AddChild(ScoreBoardRow);
		}
	}
}

void UScoreBoard::RefreshPlayerList()
{
	/*UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (PlayerListWidget == nullptr) return;

	AGameStateBase* tempGS = World->GetGameState();
	if (tempGS == nullptr) return;
	ASkyRaceGameStateBase* GS = Cast <ASkyRaceGameStateBase>(tempGS);
	if (!GS) return;
	
	PlayerListWidget->ClearChildren();

	for (auto& It : GS->GetPlayersByScore())
	{
		UScoreBoardRow* ThisPlayerRow = CreateWidget<UScoreBoardRow>(this, PlayerRowClass);
		if (ThisPlayerRow == nullptr) break;

		ThisPlayerRow->PlayerRanking->SetText(FText::FromString(FString::FromInt(It.Rank) + FString(".")));
		ThisPlayerRow->PlayerName->SetText(FText::FromString(It.PlayerName));
		ThisPlayerRow->PlayerScore->SetText(FText::FromString(FString::FromInt(It.Score)));
		PlayerListWidget->AddChild(ThisPlayerRow);
	}*/

	/*TMap<int, TArray<APlayerState*>> PlayersByScore;
	for (APlayerState*& It : GS->PlayerArray)
	{
		ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(It);
		if (!PS) break;

		//Create the nested array, if it does not exist
		if (!PlayersByScore.Contains((int)PS->Score)) PlayersByScore.Add((int)PS->Score, TArray<APlayerState*>());

		PlayersByScore[(int)PS->Score].Add(PS);
	}

	PlayersByScore.KeySort([](int A, int B) {
		return A > B;
	});
	UE_LOG(LogTemp, Warning, TEXT("ScoreRows=%d"), PlayersByScore.Num());

	PlayerListWidget->ClearChildren();

	int CurrentRank = 1;
	int LastScore = -1;
	for (auto& ScoreGroup : PlayersByScore)
	{
		for (auto& ItPS : ScoreGroup.Value)
		{
			//if this is the winner, set winner text
			if (CurrentRank == 1 && WinnerText != nullptr) WinnerText->SetText(FText::FromString(ItPS->GetPlayerName()));

			//New player / damaged dataset, create widget
			UScoreBoardRow* ThisPlayerRow = CreateWidget<UScoreBoardRow>(this, PlayerRowClass);
			if (ThisPlayerRow == nullptr) break;

			ThisPlayerRow->PlayerRanking->SetText(FText::FromString(FString::FromInt(CurrentRank) + FString(".")));
			ThisPlayerRow->PlayerName->SetText(FText::FromString(ItPS->GetPlayerName()));
			ThisPlayerRow->PlayerScore->SetText(FText::FromString(FString::FromInt(ScoreGroup.Key)));
			PlayerListWidget->AddChild(ThisPlayerRow);
		}

		++CurrentRank;
	}*/
}

void UScoreBoard::OnButtonNextRoundClick()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return; 
	
	APlayerController* PcRaw = World->GetFirstPlayerController();
	if (PcRaw == nullptr) return;

	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(PcRaw);
	if (PC == nullptr) return;

	PC->CloseScoreBoard();
	PC->OpenLobby();
}

void UScoreBoard::OnButtonExitClick()
{
	USkyRaceGameInstance* GI = Cast<USkyRaceGameInstance>(GetGameInstance());
	if (GI == nullptr) return;

	GI->LeaveServer();
}

void UScoreBoard::DestroyMenu()
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