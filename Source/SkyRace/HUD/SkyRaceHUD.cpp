// Fill out your copyright notice in the Description page of Project Settings.


#include "SkyRaceHUD.h"

#include "RankingList.h"
#include "HUDTimer.h"
#include "Speedometer.h"

ASkyRaceHUD::ASkyRaceHUD()
{
}

void ASkyRaceHUD::DrawHUD()
{
}

void ASkyRaceHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	UE_LOG(LogTemp, Warning, TEXT("HUD activated"));

	if (TimerClass != nullptr)
	{
		TimerWidget = CreateWidget<UHUDTimer>(World, TimerClass);
		if (TimerWidget != nullptr) TimerWidget->AddToViewport();
	}

	if (RankingListClass != nullptr)
	{
		RankingListWidget = CreateWidget<URankingList>(World, RankingListClass);
		if (RankingListWidget != nullptr) RankingListWidget->AddToViewport();
	}

	if (SpeedometerClass != nullptr)
	{
		SpeedometerWidget = CreateWidget<USpeedometer>(World, SpeedometerClass);
		if (SpeedometerWidget != nullptr) SpeedometerWidget->AddToViewport();
	}
}

void ASkyRaceHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASkyRaceHUD::Hide()
{
	if (TimerWidget != nullptr) TimerWidget->RemoveFromViewport();
	if (RankingListWidget != nullptr) RankingListWidget->RemoveFromViewport();
	if (SpeedometerWidget != nullptr) SpeedometerWidget->RemoveFromViewport();
}