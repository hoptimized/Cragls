// Fill out your copyright notice in the Description page of Project Settings.

#include "SkyRaceSpectatorPawn.h"

#include "CarPawn.h"
#include "SkyRacePlayerController.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Controller.h"
#include "Classes/Components/InputComponent.h"
#include "UObject/UObjectIterator.h"
#include "GameFramework/HUD.h"

ASkyRaceSpectatorPawn::ASkyRaceSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<AHUD> HudClassFinder(TEXT("/Game/Menus/SpectatorHUD/WBP_SpectatorHUD"));
	CustomHUDClass = HudClassFinder.Class;
}

void ASkyRaceSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SwitchCameraMode", IE_Pressed, this, &ASkyRaceSpectatorPawn::SwitchCameraMode);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &ASkyRaceSpectatorPawn::OpenInGameMenu);
}

void ASkyRaceSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	if (CustomHUDClass == nullptr) return;
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr) return;

	PC->ClientSetHUD(CustomHUDClass);
}

void ASkyRaceSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	AController* PC = GetController();
	if (PC == nullptr) return;
	
	if (CameraMode == CAMERA_MODE_MENU || CameraMode == CAMERA_MODE_ORBIT)
	{
		float TimeSeconds = World->GetTimeSeconds();
		float NormalizedTime = TimeSeconds - floor(TimeSeconds / OrbitDuration) * OrbitDuration;
		float CurrentAngle = NormalizedTime / OrbitDuration * 360;
		FVector TransformedLocation = FVector(-OrbitRadius, 0.f, OrbitAltitude).RotateAngleAxis(CurrentAngle, FVector(0.f, 0.f, 1.f));
		FRotator TransformedRotator = FRotator(OrbitPitch, CurrentAngle, 0.f);

		if (!bIsStartup)
		{
			SetActorLocation(FMath::VInterpTo(GetActorLocation(), TransformedLocation, DeltaTime, VInterpSpeed));
			PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), TransformedRotator, DeltaTime, RInterpSpeed));
		}
		else
		{
			SetActorLocation(TransformedLocation);
			PC->SetControlRotation(TransformedRotator);

			bIsStartup = false;
		}
		
	}
	if (CameraMode == CAMERA_MODE_FOLLOW)
	{
		if (CameraFollowPlayerPawn.IsValid())
		{
			SetActorLocation(FMath::VInterpTo(GetActorLocation(), CameraFollowPlayerPawn->GetCameraLocation(), DeltaTime, VInterpSpeed));
			PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), CameraFollowPlayerPawn->GetCameraRotation(), DeltaTime, RInterpSpeed));
		}
	}
}

void ASkyRaceSpectatorPawn::SwitchCameraMode()
{
	if (CameraMode == CAMERA_MODE_FOLLOW)
	{
		if (!FollowNextPlayer()) NextCameraMode();
	}
	else
	{
		NextCameraMode();
	}

	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(GetController());
	if (PC == nullptr) return;

	if (CameraMode == CAMERA_MODE_MENU)
	{
		PC->OpenLobby();
	}
	if (CameraMode == CAMERA_MODE_ORBIT)
	{
		PC->CloseLobby();
	}
	if (CameraMode == CAMERA_MODE_FOLLOW)
	{
		if (!FollowPlayer(0)) NextCameraMode();
	}
}

void ASkyRaceSpectatorPawn::NextCameraMode()
{
	CameraMode = (CameraMode + 1) % NUM_CAMERA_MODES;
}

bool ASkyRaceSpectatorPawn::FollowNextPlayer()
{
	return FollowPlayer(++CameraFollowPlayerIndex);
}

bool ASkyRaceSpectatorPawn::FollowPlayer(int PawnIndex)
{
	CameraFollowPlayerIndex = PawnIndex;

	UWorld* World = GetWorld();
	if (World == nullptr) return false;	

	bool bIsSuccessful = false;
	int i = 0;
	for (TObjectIterator<ACarPawn> It; It; ++It)
	{
		if (i == PawnIndex)
		{
			CameraFollowPlayerPawn = *It;
			bIsSuccessful = true;
			//AttachToActor(*It, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		++i;
	}

	return bIsSuccessful;
}

void ASkyRaceSpectatorPawn::OpenInGameMenu()
{
	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(GetController());
	if (PC == nullptr) return;

	PC->OpenInGameMenu();
}