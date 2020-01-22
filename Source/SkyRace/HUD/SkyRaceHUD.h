// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SkyRaceHUD.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API ASkyRaceHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASkyRaceHUD();
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void Hide();

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> RankingListClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> TimerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> SpeedometerClass;

protected:
	class UHUDTimer* TimerWidget;
	class URankingList* RankingListWidget;
	class USpeedometer* SpeedometerWidget;

};
