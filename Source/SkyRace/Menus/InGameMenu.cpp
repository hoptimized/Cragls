// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenu.h"

void UInGameMenu::Setup()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	this->bIsFocusable = true;
	this->AddToViewport();

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputModeData);
	PC->bShowMouseCursor = true;
}

void UInGameMenu::DestroyMenu()
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