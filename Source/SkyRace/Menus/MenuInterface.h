// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMenuInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SKYRACE_API IMenuInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void HostServer(FString ServerName, uint16 MaxPlayers, bool isLanMatch, bool isPrivate, FString Password) = 0;
	virtual void RefreshServerList() = 0;
	virtual void JoinServer(FString inSessionId) = 0;
	virtual bool GetIsLoggedIn() = 0;
};
