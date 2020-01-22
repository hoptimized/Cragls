// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Speedometer.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API USpeedometer : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* SpeedometerImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* BoostIndicatorImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* ShieldIndicatorImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* WarningImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* NormalMissileImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* HomingMissileImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SpeedText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NormalMissileText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HomingMissileText;

	FTimerHandle WarningTimer;

	virtual void NativeConstruct();
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	UFUNCTION()
	void flipWarningImage();
};
