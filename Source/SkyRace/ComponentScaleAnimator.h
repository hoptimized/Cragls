// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentScaleAnimator.generated.h"

const int ANIMATION_STATUS_IDLE = 0;
const int ANIMATION_STATUS_SCALING = 1;
const int ANIMATION_STATUS_WOBBLING = 2;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYRACE_API UComponentScaleAnimator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UComponentScaleAnimator();

protected:
	virtual void BeginPlay() override;

	//The component to be scaled
	UPROPERTY()
	class UStaticMeshComponent* TheComponent;

	//The scale that the component has at ratio 1.0
	UPROPERTY(EditAnywhere)
	FVector NaturalScale;

	//General animation fields
	FVector StartScale;
	FVector TargetScale;
	float currentRatio;

	//For Scaling
	float scalingSpeed;
	float scalingGravity;
	float maxSpeed;

	//For Wobbling
	float animationStarttime;
	float swingMagnitude;
	float swingFrequency;
	float swingPhaseOffset;
	float swingDamping;

private:
	int CurrentAnimationStatus = ANIMATION_STATUS_IDLE;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//Reference to the mesh component needs to be set in blueprint!
	UFUNCTION(BlueprintCallable)
	void SetMesh(class UStaticMeshComponent* inMesh) { TheComponent = inMesh; }

	void ScaleToSize(FVector inTargetScale, float StartSpeed, float inGravity, float inMaxSpeed);
	void Wobble(float Magnitude, float Frequency, float Damping);
	void WobbleFromCurrentMovement(FVector inTargetScale, float Damping);
	void WobbleInternal(FVector inTargetScale, float Magnitude, float Frequency, float PhaseOffset, float Damping);
	void StopAnimation();

	bool IsIdle() { return CurrentAnimationStatus== ANIMATION_STATUS_IDLE; }
};
