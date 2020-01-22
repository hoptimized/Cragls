// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingsSaveGame.generated.h"

const FString SAVE_SLOT_NAME = TEXT("SettingsSaveSlot");
const int SAVE_USER_INDEX = 0;

/**
 * 
 */
UCLASS()
class SKYRACE_API USettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	USettingsSaveGame();

protected:
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclScreenResolution;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclFrameRate;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclGraphicalQuality;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclShadowsQuality;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclAntialiasingQuality;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclBloomQuality;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	int LclCarDetails;

public:

	UFUNCTION(BlueprintCallable)
	void Save(int ScreenResolution, int FrameRate, int GraphicalQuality, int ShadowsQuality, int AntiAliasingQuality, int BloomQuality, int CarDetails);

	UFUNCTION(BlueprintCallable)
	void GetSettings(int& ScreenResolution, int& FrameRate, int& GraphicalQuality, int& ShadowsQuality, int& AntiAliasingQuality, int& BloomQuality, int& CarDetails);
};
