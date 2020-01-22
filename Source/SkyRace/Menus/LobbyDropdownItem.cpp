// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyDropdownItem.h"

FColor ULobbyDropdownItem::HexToColor(FString inString)
{
	return FColor::FromHex(inString);
}