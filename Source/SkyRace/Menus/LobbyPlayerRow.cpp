// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyPlayerRow.h"

#include "SkyRaceGameStateBase.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Menus/CustomComboBox.h"
#include "Menus/LobbyMenu.h"

bool ULobbyPlayerRow::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (PlayerColor == nullptr) return false;
	PlayerColor->Init(this, false);

	return true;
}

void ULobbyPlayerRow::Init(class ULobbyMenu* inMenu, int32 inPlayerId, TArray<FString> PlayerColors)
{
	ParentMenu = inMenu;
	PlayerId = inPlayerId;

	if (PlayerColor == nullptr) return;
	PlayerColor->SetOptions(PlayerColors);
}

void ULobbyPlayerRow::NativeConstruct()
{
	//if (PlayerColor == nullptr) return;
	//PlayerColor->SetColor(FString("#FF0000"));
}

void ULobbyPlayerRow::SetColor(FString inColorHex)
{
	if (PlayerColor == nullptr) return;
	PlayerColor->SetColor(inColorHex);
}

void ULobbyPlayerRow::SetIsEnabledCustom(bool inIsEnabled)
{
	if (PlayerColor == nullptr) return;
	PlayerColor->SetIsEnabledCustom(inIsEnabled);
}

void ULobbyPlayerRow::OnPlayerColorChange(FString newColorHex)
{
	if (ParentMenu == nullptr) return;
	ParentMenu->onColorPickerInput(PlayerId, newColorHex);
}

void ULobbyPlayerRow::Update(FString inPlayerName, FString inPlayerColor, bool bIsReady, bool bIsSpectator, bool inIsEnabled)
{
	if (PlayerName == nullptr) return;
	if (!PlayerName->GetText().ToString().Equals(inPlayerName, ESearchCase::CaseSensitive)) PlayerName->SetText(FText::FromString(inPlayerName));

	if (PlayerColor == nullptr) return;
	if (!PlayerColor->GetSelectedColor().Equals(inPlayerColor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Changing player color to %s"), *inPlayerColor);
		PlayerColor->SetColor(inPlayerColor);
	}

	if (PlayerColor->GetIsEnabledCustom() != inIsEnabled)
	{
		PlayerColor->SetIsEnabledCustom(inIsEnabled);
	}

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast <ASkyRaceGameStateBase>(World->GetGameState());
	if (!GS) return;

	if (PlayerStatus == nullptr) return;

	FString StatusString;
	if (GS->IsPlaying())
	{
		StatusString = bIsSpectator ? FString("spectating") : FString("playing");
	}
	else if (GS->IsInLobby())
	{
		StatusString = bIsReady ? FString("go!") : FString("not ready");
	}
	else
	{
		StatusString = FString("STARTING!");
	}

	PlayerStatus->SetText(FText::FromString(StatusString));
}