
// Fill out your copyright notice in the Description page of Project Settings.


#include "MissileAlertSystem.h"
#include "MissileActor.h"
#include "CarPawn.h"

#include "Sound/SoundCue.h"
#include "Classes/Components/AudioComponent.h"

#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include <limits>

// Sets default values for this component's properties
UMissileAlertSystem::UMissileAlertSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UMissileAlertSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMissileAlertSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	ACarPawn* OwnerPawn = Cast<ACarPawn>(GetOwner());
	if (!OwnerPawn) return;
	if (!OwnerPawn->IsLocallyControlled()) return;

	//if (OwnerPawn->Role == ROLE_AutonomousProxy) UE_LOG(LogTemp, Warning, TEXT("Autoprox"));
	//if (OwnerPawn->GetRemoteRole() == ROLE_SimulatedProxy) UE_LOG(LogTemp, Warning, TEXT("Server"));

	if (AlertLevels.Num() == 0) return;

	//UE_LOG(LogTemp, Warning, TEXT("Looping missiles"));
	float closestDistance = std::numeric_limits<float>::max();
	for (TActorIterator<AMissileActor> It(World); It; ++It)
	{
		//UE_LOG(LogTemp, Warning, TEXT("loop"));
		TWeakObjectPtr<AMissileActor> MissilePtr = TWeakObjectPtr<AMissileActor>(*It); //Cast<AMissileActor>(*It);
		if (!MissilePtr.IsValid()) continue;

		AMissileActor* Missile = MissilePtr.Get();

		TWeakObjectPtr<ACarPawn> TargetPawnPtr = Missile->GetTargetPawn();

		//UE_LOG(LogTemp, Warning, TEXT("Checking target pawn"));

		if (!TargetPawnPtr.IsValid()) continue;

		//UE_LOG(LogTemp, Warning, TEXT("Target Pawn valid"));

		ACarPawn* TargetPawn = TargetPawnPtr.Get();

		//UE_LOG(LogTemp, Warning, TEXT("Target Pawn valid"));

		if (TargetPawn == OwnerPawn)
		{
			float distance = FVector(OwnerPawn->GetActorLocation() - Missile->GetActorLocation()).Size() / 100.f;
			UE_LOG(LogTemp,Warning,TEXT("%f"), distance);
			if (distance < closestDistance) closestDistance = distance;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Done looping missiles"));

	//UE_LOG(LogTemp, Warning, TEXT("Closest distance = %f"), closestDistance);

	USoundCue* NewAlarmSoundCue = nullptr;
	currentAlertLevel = -1;
	int32 i = 0;
	do {
		if (closestDistance <= AlertLevels[i].AlertDistance)
		{
			currentAlertLevel = i;
			NewAlarmSoundCue = AlertLevels[i].AlarmSoundCue;
		}
		++i;
	} while (i < AlertLevels.Num() && NewAlarmSoundCue == nullptr);

	if (NewAlarmSoundCue != nullptr)
	{
		if (!NewAlarmSoundCue->IsValidLowLevel()) return;


		if (ActiveAlarmSoundCue != NewAlarmSoundCue) {
			//UE_LOG(LogTemp, Warning, TEXT("Stopping current alarm"));
			stopSoundSafe(AudioComponent);
			AudioComponent = nullptr;
		}

		//UE_LOG(LogTemp, Warning, TEXT("Playing alarm"));
		playSoundSafe(AudioComponent, World, NewAlarmSoundCue, 1.f, 1.f);
	}
	else
	{
		stopSoundSafe(AudioComponent);
	}

	ActiveAlarmSoundCue = NewAlarmSoundCue;
}

void UMissileAlertSystem::TurnOff()
{
	stopSoundSafe(AudioComponent);
	AudioComponent = nullptr;
}

void UMissileAlertSystem::playSoundSafe(UAudioComponent*& inAudioComponent, UWorld* World, USoundCue* SoundCue, float pitch, float volume)
{
	if (!ensure(World != nullptr)) return;

	if (!ensure(SoundCue != nullptr)) return;
	if (!SoundCue->IsValidLowLevel()) return;

	//UE_LOG(LogTemp, Warning, TEXT("World and SoundCue: OK"));

	if (inAudioComponent == nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("nullptr, spawn"));

		inAudioComponent = UGameplayStatics::SpawnSound2D(World, SoundCue, 1.f, 1.f, 0.f);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("NOT nullptr"));

		if (inAudioComponent->IsValidLowLevel())
		{
			//UE_LOG(LogTemp, Warning, TEXT("Valid low level, play"));
			if (!inAudioComponent->IsPlaying()) inAudioComponent->Play();
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("NOT valid low level, spawn"));
			inAudioComponent = UGameplayStatics::SpawnSound2D(World, SoundCue, 1.f, 1.f, 0.f);
		}
	}
}

void UMissileAlertSystem::stopSoundSafe(UAudioComponent*& inAudioComponent)
{
	if (inAudioComponent != nullptr)
	{
		if (inAudioComponent->IsValidLowLevel())
		{
			if (inAudioComponent->IsPlaying()) inAudioComponent->Stop();
		}
	}
}
