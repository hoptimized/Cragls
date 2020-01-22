// Fill out your copyright notice in the Description page of Project Settings.

#include "SettingsSaveGame.h"

#include "Classes/Kismet/GameplayStatics.h"

USettingsSaveGame::USettingsSaveGame()
{
	SaveSlotName = SAVE_SLOT_NAME;
	UserIndex = SAVE_USER_INDEX;

	LclScreenResolution = 3;
	LclFrameRate = 2;
	LclGraphicalQuality = 3;
	LclShadowsQuality = 3;
	LclAntialiasingQuality = 3;
	LclBloomQuality = 3;
	LclCarDetails = 2;
}

void USettingsSaveGame::Save(int ScreenResolution, int FrameRate, int GraphicalQuality, int ShadowsQuality, int AntiAliasingQuality, int BloomQuality, int CarDetails)
{
	LclScreenResolution = ScreenResolution;
	LclFrameRate = FrameRate;
	LclGraphicalQuality = GraphicalQuality;
	LclShadowsQuality = ShadowsQuality;
	LclAntialiasingQuality = AntiAliasingQuality;
	LclBloomQuality = BloomQuality;
	LclCarDetails = CarDetails;

	UE_LOG(LogTemp, Warning, TEXT("SAVING"));
	if (UGameplayStatics::SaveGameToSlot(this, TEXT("SettingsSaveSlot"), UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SAVED"));
	}
}

void USettingsSaveGame::GetSettings(int& ScreenResolution, int& FrameRate, int& GraphicalQuality, int& ShadowsQuality, int& AntiAliasingQuality, int& BloomQuality, int& CarDetails)
{
	ScreenResolution = LclScreenResolution;
	FrameRate = LclFrameRate;
	GraphicalQuality = LclGraphicalQuality;
	ShadowsQuality = LclShadowsQuality;
	AntiAliasingQuality = LclAntialiasingQuality;
	BloomQuality = LclBloomQuality;
	CarDetails = LclCarDetails;
}
