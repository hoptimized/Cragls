// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RankingListEntry.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API URankingListEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameAndScore;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Ranking;

	int PlayerId;
	
};
