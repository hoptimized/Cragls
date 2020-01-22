// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRow.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UServerRow : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ServerName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HostName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NumOfPlayers;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TimeRemaining;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Ping;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Network;

	UPROPERTY(meta = (BindWidget))
	class UImage* IsPrivate;

	void Setup(class UMainMenu* Parent, uint32 Index, FString inSessionId);

	FString GetSessionId() { return SessionId; };

	void Select();

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* ServerRowButton;

	UFUNCTION()
	void onClicked();

	UPROPERTY()
	class UMainMenu* Parent;

	uint32 Index;
	FString SessionId;

};
