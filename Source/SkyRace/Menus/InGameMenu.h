// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameMenu.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UInGameMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup();
	void DestroyMenu();

};
