// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnergyRing.generated.h"

UCLASS()
class SKYRACE_API AEnergyRing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnergyRing();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* EnergyRingPS;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* HighVoltageCue;

	UAudioComponent* HighVoltageAudioComponent;

	float SpawnTime;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
