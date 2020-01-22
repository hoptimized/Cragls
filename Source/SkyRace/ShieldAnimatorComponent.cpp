// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldAnimatorComponent.h"

#include "Classes/Components/StaticMeshComponent.h"

#include "TimerManager.h"

void UShieldAnimatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Update the lifecycle status when animations are completed
	if (IsIdle())
	{
		if (ShieldAnimationStatus == SHIELD_ANIMATOR_STATUS_ACTIVATING) ShieldAnimationStatus = SHIELD_ANIMATOR_STATUS_ACTIVE;
		if (ShieldAnimationStatus == SHIELD_ANIMATOR_STATUS_DEACTIVATING) ShieldAnimationStatus = SHIELD_ANIMATOR_STATUS_INACTIVE;
	}
}

//Activate shield. Initiates the shield animation and sets a timer to deactivate the shield
void UShieldAnimatorComponent::ActivateShield()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (ShieldTimerHandle.IsValid()) World->GetTimerManager().ClearTimer(ShieldTimerHandle);
	FTimerDelegate ShieldTimerDel;
	ShieldTimerDel.BindUFunction(this, FName("DeactivateShield"));
	World->GetTimerManager().SetTimer(ShieldTimerHandle, ShieldTimerDel, ShieldDuration, false);

	ShieldAnimationStatus = SHIELD_ANIMATOR_STATUS_ACTIVATING;
	ScaleToSize(NaturalScale, ScalingStartSpeed, ScalingGravity, MaxScalingSpeed);
}

//Deactivate shield
void UShieldAnimatorComponent::DeactivateShield()
{
	ShieldAnimationStatus = SHIELD_ANIMATOR_STATUS_DEACTIVATING;
	ScaleToSize(FVector::ZeroVector, ScalingStartSpeed, ScalingGravity, MaxScalingSpeed);
}

//Wobble on impact
void UShieldAnimatorComponent::Impact()
{
	if (ShieldAnimationStatus == SHIELD_ANIMATOR_STATUS_ACTIVE) Wobble(ShieldAbsorbWobbleMagnitude, ShieldAbsorbWobbleFrequency, ShieldAbsorbWobbleDamping);
}

//Reset shield to 0. Useful of respawn
void UShieldAnimatorComponent::ResetShield()
{
	StopAnimation();
	TheComponent->SetWorldScale3D(FVector::ZeroVector);
}

bool UShieldAnimatorComponent::HasDisappeared()
{
	return ShieldAnimationStatus == SHIELD_ANIMATOR_STATUS_INACTIVE;
}

float UShieldAnimatorComponent::GetShieldTimerRatio() {
	UWorld* World = GetWorld();
	if (!World) return 0.f;

	if (!ShieldTimerHandle.IsValid()) return 0.f;
	if (World->GetTimerManager().GetTimerRate(ShieldTimerHandle) < KINDA_SMALL_NUMBER) return 0.f;

	return 1 - (World->GetTimerManager().GetTimerElapsed(ShieldTimerHandle) / World->GetTimerManager().GetTimerRate(ShieldTimerHandle));
}
