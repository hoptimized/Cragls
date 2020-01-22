// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "SkyRaceSpectatorPawn.generated.h"

const int CAMERA_MODE_MENU = 0;
const int CAMERA_MODE_ORBIT = 1;
const int CAMERA_MODE_FOLLOW = 2;
const int CAMERA_MODE_FREE = 3;
const int NUM_CAMERA_MODES = 4;

/**
 * 
 */
UCLASS()
class SKYRACE_API ASkyRaceSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	ASkyRaceSpectatorPawn();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere)
	float OrbitDuration = 35.f;

	UPROPERTY(EditAnywhere)
	float OrbitPitch = -10.f;

	UPROPERTY(EditAnywhere)
	float OrbitRadius = 5500.f;

	UPROPERTY(EditAnywhere)
	float OrbitAltitude = 1200.f;

	int CameraMode = CAMERA_MODE_MENU;
	int CameraFollowPlayerIndex = 0;
	TWeakObjectPtr<class ACarPawn> CameraFollowPlayerPawn;

	void NextCameraMode();
	bool FollowNextPlayer();
	bool FollowPlayer(int PawnIndex);

	float VInterpSpeed = 10.f;
	float RInterpSpeed = 3.f;

	bool bIsStartup = true;

	TSubclassOf<class AHUD> CustomHUDClass;

	UFUNCTION()
	void OpenInGameMenu();

public:
	void SwitchCameraMode();

};
