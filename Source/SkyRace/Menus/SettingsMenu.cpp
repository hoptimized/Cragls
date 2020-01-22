// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsMenu.h"

void USettingsMenu::Setup(UUserWidget* inParent)
{
	Parent = inParent;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	this->bIsFocusable = true;
	this->AddToViewport();

	FInputModeGameAndUI InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputModeData);
	PC->bShowMouseCursor = true;
}

void USettingsMenu::DestroyMenu()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	this->RemoveFromViewport();
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	if (Parent == nullptr)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->bShowMouseCursor = false;
	}
	else
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(Parent->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputModeData);
	}
}