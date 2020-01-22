// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreBoardRow.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UScoreBoardRow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerRanking;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerScore;

};
