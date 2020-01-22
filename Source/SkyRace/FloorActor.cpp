// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorActor.h"

#include "SkyRaceGameStateBase.h"

#include "Classes/Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFloorActor::AFloorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFloorActor::BeginPlay()
{
	Super::BeginPlay();

	if (StaticMesh == nullptr) return;

	FloorMaterial = StaticMesh->CreateDynamicMaterialInstance(0, SourceMaterial);
	if (FloorMaterial == nullptr) return;

	ColorFadeStartTime = 0.f;
}

// Called every frame
void AFloorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FloorMaterial == nullptr) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ASkyRaceGameStateBase* GS = Cast<ASkyRaceGameStateBase>(World->GetGameState());
	if (!GS) return;

	//This factor makes the green light fade out after the game has started (the floor material will then move between blue and green-ish colors)
	float FadingFactor = 1 - FMath::Clamp<float>((World->GetTimeSeconds() - ColorFadeStartTime - FadeDelay) / FadeDuration, 0.f, 1.f);

	//Set override factors: (faded) green when the game is running, red otherwise
	FloorMaterial->SetScalarParameterValue("RedOverride", (GS->GameStatus == GAME_STATUS_PLAYING) ? 0.f : 1.f);
	FloorMaterial->SetScalarParameterValue("GreenOverride", (GS->GameStatus == GAME_STATUS_PLAYING) ? FadingFactor : 0.f);

	//If the game status has just switched to PLAYING, activate the timer for fading out the green light
	if (GAME_STATUS_PLAYING && LastStatus != GAME_STATUS_PLAYING) ColorFadeStartTime = World->GetTimeSeconds();

	//keep the last game status for later comparison
	LastStatus = GS->GameStatus;
}

