// Fill out your copyright notice in the Description page of Project Settings.


#include "ComponentScaleAnimator.h"

#include "Classes/Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UComponentScaleAnimator::UComponentScaleAnimator()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UComponentScaleAnimator::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UComponentScaleAnimator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//Manage the animations (ScaleToSize and Wobble). Transitions are done by child classes
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//--------------------------------------------------
	//ScaleToSize

	if (CurrentAnimationStatus == ANIMATION_STATUS_SCALING)
	{
		//Calculate scaling acceleration from gravity formula, then convert into delta speed and calculate new ratio
		float scalingAcceleration = scalingGravity / pow(1 - currentRatio, 2);
		scalingSpeed += scalingAcceleration * DeltaTime;
		currentRatio += scalingSpeed * DeltaTime;

		//If we overshot the target, set ratio to target ratio and stop animation
		if (currentRatio >= 1.f)
		{
			currentRatio = 1.f;
			CurrentAnimationStatus = ANIMATION_STATUS_IDLE;
		}

		//Apply the scale
		TheComponent->SetWorldScale3D(StartScale + (TargetScale - StartScale) * currentRatio);
	}

	//--------------------------------------------------
	//Wobble

	if (CurrentAnimationStatus == ANIMATION_STATUS_WOBBLING)
	{
		float elapsedTime = GetWorld()->GetTimeSeconds() - animationStarttime;

		//Swing function w/o damping 
		currentRatio = 1 + FMath::Sin(elapsedTime * swingFrequency * 2 * PI + swingPhaseOffset) * swingMagnitude;

		//Damping of magnitude w/ decay function: A * e^(-b * t), A is initial value, e is Euler's constant, b is decay constant in percent, t is time
		swingMagnitude = swingMagnitude * pow(UKismetMathLibrary::Exp(1), -swingDamping * elapsedTime); 

		//UE_LOG(LogTemp, Warning, TEXT("Wobbling: TargetScale.Z = %f, currentRatio = %f, magnitude = %f"), TargetScale.Z, currentRatio, swingMagnitude);
		
		//If magnitude is kinda small, stop the animation
		if (FMath::Abs(swingMagnitude) <= KINDA_SMALL_NUMBER) CurrentAnimationStatus = ANIMATION_STATUS_IDLE;

		//Apply the scale
		TheComponent->SetWorldScale3D(TargetScale * currentRatio);
	}
}

//Initiates a ScaleToSize animation. Component scales from current scale to TargetScale. Movement can be linear (no gravity) or accelerated (w/ gravity constant)
void UComponentScaleAnimator::ScaleToSize(FVector inTargetScale, float StartSpeed, float inGravity, float inMaxSpeed)
{
	if (TheComponent == nullptr) return;

	StartScale = TheComponent->GetComponentScale();
	TargetScale = inTargetScale;
	currentRatio = 0.f;	//this is actually a ratio for the delta [StartScale + (TargetScale - StartScale) * ratio], so it's 0 in the beginning
	scalingSpeed = StartSpeed;
	scalingGravity = inGravity;
	maxSpeed = inMaxSpeed;

	CurrentAnimationStatus = ANIMATION_STATUS_SCALING;
}

//Initiates a Wobble animation. Component wobbles around its current scale with given magnitude, frequency and damping
void UComponentScaleAnimator::Wobble(float Magnitude, float Frequency, float Damping)
{
	if (TheComponent == nullptr) return;
	WobbleInternal(TheComponent->GetComponentScale(), Magnitude, Frequency, PI, Damping);
}

//Initiates a Wobble animation. Component wobbles around the given target scale, assuming that the component is currently at the highest magnitude. Automatically calculates magnitude from current scale and frequency from current scalingSpeed
void UComponentScaleAnimator::WobbleFromCurrentMovement(FVector inTargetScale, float Damping)
{
	if (TheComponent == nullptr) return;

	float Magnitude = (TheComponent->GetComponentScale() - inTargetScale).Z / inTargetScale.Z; // if currentZ=6 and targetZ=5, Magnitude=0.2 (6 = 1.2 * 5)
	float Frequency = 1 / (4 * Magnitude / scalingSpeed); // Magnitude*4, because sin() completes four magnitudes in one cycle
	swingPhaseOffset = 0.5 * PI; //begin at sin(0.5 * PI) = 1.0 and scale down from there

	WobbleInternal(inTargetScale, Magnitude, Frequency, swingPhaseOffset, Damping);
}

//Initiates a Wobble animation. Called from Wobble() and WobbleFromCurrentMovement(). Sets all variables and starts the animation
void UComponentScaleAnimator::WobbleInternal(FVector inTargetScale, float Magnitude, float Frequency, float PhaseOffset, float Damping)
{
	animationStarttime = GetWorld()->GetTimeSeconds();

	TargetScale = inTargetScale;
	swingMagnitude = Magnitude;
	swingFrequency = Frequency;
	swingPhaseOffset = PhaseOffset;
	swingDamping = Damping;

	CurrentAnimationStatus = ANIMATION_STATUS_WOBBLING;
}

void UComponentScaleAnimator::StopAnimation()
{
	CurrentAnimationStatus = ANIMATION_STATUS_IDLE;
}