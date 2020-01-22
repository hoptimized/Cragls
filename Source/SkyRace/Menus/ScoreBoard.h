// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreBoard.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UScoreBoard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UScoreBoard(const FObjectInitializer& ObjectInitializer);
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	void Setup();
	void RefreshPlayerList();

	UFUNCTION()
	void OnButtonNextRoundClick();

	UFUNCTION()
	void OnButtonExitClick();

	void DestroyMenu();

protected:
	bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WinnerText;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* PlayerListWidget;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonNextRound;

	UPROPERTY(meta = (BindWidget))
	class UButton* ButtonExit;

	TSubclassOf<class UUserWidget> PlayerRowClass;

};
