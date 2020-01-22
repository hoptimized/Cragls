// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyDropdownItem.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API ULobbyDropdownItem : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	FColor HexToColor(FString inString);

};
