// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDTimer.h"

FString UHUDTimer::GetTimeText(float inSeconds)
{
	int TotalMillisecs = (int)(inSeconds * 1000.f);
	int Millisecs = TotalMillisecs % 1000;
	int TotalSeconds = (TotalMillisecs - Millisecs) / 1000;
	int Seconds = TotalSeconds % 60;
	int TotalMinutes = (TotalSeconds - Seconds) / 60;
	int Minutes = TotalMinutes;

	return IntToStringWithZeros(Minutes, 2) + FString(":") + IntToStringWithZeros(Seconds, 2) + FString(".") + IntToStringWithZeros(Millisecs, 3);
}

FString UHUDTimer::IntToStringWithZeros(int inVal, int numDigits)
{
	FString TempString = FString::FromInt(inVal);
	while (TempString.Len() < numDigits)
	{
		TempString = FString("0") + TempString;
	}
	return TempString;
}