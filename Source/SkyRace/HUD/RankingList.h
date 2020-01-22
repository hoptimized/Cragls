// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RankingList.generated.h"

USTRUCT()
struct FPlayerRankingData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int PlayerId;

};

/**
 * 
 */
UCLASS()
class SKYRACE_API URankingList : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual void NativeConstruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UUserWidget> EntryClass;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* NameList;

};
