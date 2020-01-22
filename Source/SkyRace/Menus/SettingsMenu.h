// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenu.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API USettingsMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	UPROPERTY()
	class UUserWidget* Parent;

public:
	void Setup(UUserWidget* inParent);

	UFUNCTION(BlueprintCallable)
	void DestroyMenu();

};
