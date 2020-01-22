// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDTimer.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UHUDTimer : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	FString IntToStringWithZeros(int inVal, int numDigits);

public:

	UFUNCTION(BlueprintCallable)
	FString GetTimeText(float inSeconds);

};
