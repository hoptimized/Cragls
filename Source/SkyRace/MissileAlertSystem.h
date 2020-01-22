// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MissileAlertSystem.generated.h"

USTRUCT()
struct FAlertLevel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	class USoundCue* AlarmSoundCue;

	UPROPERTY(EditAnywhere)
	float AlertDistance;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYRACE_API UMissileAlertSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMissileAlertSystem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		TArray<FAlertLevel> AlertLevels;

	class UAudioComponent* AudioComponent;

	class USoundCue* ActiveAlarmSoundCue;

	void playSoundSafe(UAudioComponent*& AudioComponent, UWorld* World, USoundCue* SoundCue, float pitch, float volume);
	void stopSoundSafe(UAudioComponent*& AudioComponent);

	int32 currentAlertLevel = -1;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	int32 GetCurrentAlertLevel() { return currentAlertLevel; }
	
	void TurnOff();

};
