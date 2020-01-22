// Fill out your copyright notice in the Description page of Project Settings.

#include "Speedometer.h"
#include "CarPawn.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

void USpeedometer::NativeConstruct()
{
	Super::NativeConstruct();

	if (WarningImage == nullptr) return;
	WarningImage->SetVisibility(ESlateVisibility::Hidden);
}

void USpeedometer::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	ACarPawn* Car = Cast<ACarPawn>(GetPlayerContext().GetPawn());
	if (!Car) return;

	//Draw speedometer markings
	if (SpeedometerImage != nullptr)
	{
		UMaterialInstanceDynamic* SpeedometerMaterial = SpeedometerImage->GetDynamicMaterial();
		if (SpeedometerMaterial != nullptr)
		{
			//the 36 in the formula is related to the angles of the indications on the speedometer, it is NOT related to the conversion between m/s and km/h
			SpeedometerMaterial->SetScalarParameterValue(FName("Progress"), FMath::RoundHalfFromZero(Car->GetSpeedPercentage() * 36) / 36.f);
		}
	}

	//Draw boost indicator
	if (BoostIndicatorImage != nullptr)
	{
		UMaterialInstanceDynamic* BoostIndicatorMaterial = BoostIndicatorImage->GetDynamicMaterial();
		if (BoostIndicatorMaterial != nullptr)
		{
			BoostIndicatorMaterial->SetScalarParameterValue(FName("Progress"), Car->GetBoostTimerRatio());
		}
	}

	//Draw shield indicator
	if (ShieldIndicatorImage != nullptr)
	{
		UMaterialInstanceDynamic* ShieldIndicatorMaterial = ShieldIndicatorImage->GetDynamicMaterial();
		if (ShieldIndicatorMaterial != nullptr)
		{
			ShieldIndicatorMaterial->SetScalarParameterValue(FName("Progress"), Car->GetShieldTimerRatio());
		}
	}

	//Draw written speed
	if (SpeedText != nullptr)
	{
		SpeedText->SetText(FText::FromString(FString::FromInt(Car->GetSpeedInt())));
	}

	//Draw number of normal missiles
	if (NormalMissileText != nullptr)
	{
		NormalMissileText->SetText(FText::FromString(FString::FromInt(Car->GetNumNormalMissiles())));
	}

	//Draw number of homing missiles
	if (HomingMissileText != nullptr)
	{
		HomingMissileText->SetText(FText::FromString(FString::FromInt(Car->GetNumHomingMissiles())));
	}

	//Draw missile alert triangle
	if (WarningImage != nullptr)
	{
		int32 AlertLevel = Car->GetAlertLevel();

		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			//flash faster if missile is in proximity
			float interval = 0.f;
			if (AlertLevel == 0)
			{
				interval = 0.25f;
			}
			if (AlertLevel > 0)
			{
				interval = 0.5f;
			}

			if (interval > KINDA_SMALL_NUMBER)
			{
				FTimerDelegate TimerDel;
				TimerDel.BindUFunction(this, FName("flipWarningImage"));

				if (WarningTimer.IsValid())
				{
					if (World->GetTimerManager().IsTimerActive(WarningTimer))
					{
						float currentRate = World->GetTimerManager().GetTimerRate(WarningTimer);
						if (interval < currentRate - KINDA_SMALL_NUMBER || interval > currentRate + KINDA_SMALL_NUMBER)
						{
							World->GetTimerManager().ClearTimer(WarningTimer);
							World->GetTimerManager().SetTimer(WarningTimer, TimerDel, interval, true);
						}
					}
				}
				else
				{
					World->GetTimerManager().SetTimer(WarningTimer, TimerDel, interval, true);
				}
				
			}
			else
			{
				//No alert. Reset timer and warning image
				if (WarningTimer.IsValid()) World->GetTimerManager().ClearTimer(WarningTimer);

				if (WarningImage == nullptr) return;
				WarningImage->SetVisibility(ESlateVisibility::Hidden);
			}
			
		}
	}
}

void USpeedometer::flipWarningImage()
{
	if (WarningImage == nullptr) return;

	ESlateVisibility ThisVisibility = WarningImage->GetVisibility();

	if (ThisVisibility == ESlateVisibility::Visible)
	{
		WarningImage->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		WarningImage->SetVisibility(ESlateVisibility::Visible);
	}
}