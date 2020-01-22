// Fill out your copyright notice in the Description page of Project Settings.


#include "EnergyRing.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Classes/Components/AudioComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AEnergyRing::AEnergyRing()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnergyRing::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	SpawnTime = World->GetTimeSeconds();

	UParticleSystemComponent* EnergyRingComponent = UGameplayStatics::SpawnEmitterAttached(EnergyRingPS, RootComponent);
	if (EnergyRingComponent == nullptr) return;

	EnergyRingComponent->SetRelativeLocation(FVector::ZeroVector);

	if (HighVoltageCue == nullptr) return;
	if (!HighVoltageCue->IsValidLowLevel()) return;

	HighVoltageAudioComponent = UGameplayStatics::SpawnSoundAttached(HighVoltageCue, RootComponent);
}

void AEnergyRing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (World->GetTimeSeconds() > SpawnTime + 5.f)
	{
		if (HighVoltageAudioComponent != nullptr)
		{
			if (HighVoltageAudioComponent->IsValidLowLevel())
			{
				if (HighVoltageAudioComponent->IsPlaying()) HighVoltageAudioComponent->Stop();
			}
		}

		Destroy();
	}
}

