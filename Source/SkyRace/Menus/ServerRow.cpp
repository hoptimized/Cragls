// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRow.h"
#include "MainMenu.h"

#include "Components/Button.h"


void UServerRow::Setup(class UMainMenu* inParent, uint32 inIndex, FString inSessionId)
{
	this->Parent = inParent;
	this->Index = inIndex;
	this->SessionId = inSessionId;
	
	if (ServerRowButton != nullptr) ServerRowButton->OnClicked.AddDynamic(this, &UServerRow::onClicked);
}

void UServerRow::onClicked()
{
	Parent->SelectSession(SessionId);
}

void UServerRow::Select()
{
	onClicked();
}