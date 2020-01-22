// Fill out your copyright notice in the Description page of Project Settings.


#include "NameTagComponent.h"

#include "CarPawn.h"
#include "SkyRacePlayerState.h"

#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Kismet/KismetMathLibrary.h"

UNameTagComponent::UNameTagComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNameTagComponent::BeginPlay()
{
	Super::BeginPlay();

	MaterialInstance = CreateDynamicMaterialInstance(0, BaseMaterial);
	if (MaterialInstance == nullptr) return;

	SetTextMaterial(MaterialInstance);
}

void UNameTagComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	//----------------------------------------------------------
	//Owning controller

	APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
	if (OwnerAsPawn == nullptr) return;

	bool bAnimate = !OwnerAsPawn->IsPawnControlled() || (OwnerAsPawn->IsPawnControlled() && !OwnerAsPawn->IsLocallyControlled());
	SetVisibility(bAnimate);

	if (!bAnimate) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(OwnerAsPawn->GetPlayerState());
	if (PS == nullptr) return;

	//----------------------------------------------------------
	//Watching controller

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC == nullptr) return;

	APawn* Pawn = PC->GetPawnOrSpectator();
	if (Pawn == nullptr) return;

	ACarPawn* CarPawn = Cast<ACarPawn>(Pawn);

	FVector TargetLocation;
	if (CarPawn != nullptr)
	{
		TargetLocation = CarPawn->GetCameraLocation();
	}
	else
	{
		TargetLocation = Pawn->GetActorLocation();
	}

	//----------------------------------------------------------
	//Name Tag

	FVector StartLocation = GetComponentLocation();
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);
	SetWorldRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));

	SetText(FText::FromString("#" + FString::FromInt(PS->GetRank()) + " " + PS->GetPlayerName()));

	if (MaterialInstance == nullptr) return;

	float ColorBoost = 10.f;
	FColor PlayerColor = FColor::FromHex(PS->PlayerColorHex);
	FVector PlayerColorBoosted = FVector(PlayerColor.R / 255.f * ColorBoost, PlayerColor.G / 255.f * ColorBoost, PlayerColor.B / 255.f * ColorBoost);

	MaterialInstance->SetVectorParameterValue("EmissiveColor", PlayerColorBoosted);
}

