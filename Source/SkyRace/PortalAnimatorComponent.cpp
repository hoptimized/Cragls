// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalAnimatorComponent.h"

//Let the portal appear out of the ground
void UPortalAnimatorComponent::Appear(float inTtl)
{
	ttl = inTtl; //time for the portal to slowly shrink. Does not take into consideration the time it takes for the portal to appear
	LifeCycleStatus = PORTAL_LIFECYCLE_GROWING;
	ScaleToSize(NaturalScale * AppearTargetRatio, AppearStartSpeed, AppearGravity, AppearMaxSpeed); //start appearing, accelerated animation w/ gravity
}

//Animation logic is done in TickComponent() of base class. Only need to manage transitions here
void UPortalAnimatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (!World) return;

	// Growing --> Leveling Out
	if (LifeCycleStatus == PORTAL_LIFECYCLE_GROWING && IsIdle())
	{
		WobbleFromCurrentMovement(NaturalScale, WobbleDamping);
		LifeCycleStatus = PORTAL_LIFECYCLE_LEVELING_OUT;
	}

	//Leveling Out --> Appeared
	if (LifeCycleStatus == PORTAL_LIFECYCLE_LEVELING_OUT && IsIdle())
	{
		LifeCycleStatus = PORTAL_LIFECYCLE_APPEARED;
		AnimationTimestamp = World->GetTimeSeconds();
	}

	//Appeared --> Shrinking (only if ttl>0)
	if (LifeCycleStatus == PORTAL_LIFECYCLE_APPEARED && World->GetTimeSeconds() - AnimationTimestamp > 3.f && ttl > 0.f)
	{
		float shrinkingSpeed = 1.f / (ttl / ShrinkingTargetRatio); //We do not actually shrink from 1 to 0, but from 1 to ShrinkingTargetRatio. If ShrinkingTargetRatio=0.5 and ttl=10, then shrinking speed should be 20
		ScaleToSize(NaturalScale * ShrinkingTargetRatio, shrinkingSpeed, 0.f, shrinkingSpeed); //Linear animation
		LifeCycleStatus = PORTAL_LIFECYCLE_SHRINKING;
	}

	if (LifeCycleStatus == PORTAL_LIFECYCLE_SHRINKING && IsIdle())
	{
		ScaleToSize(FVector::ZeroVector, 1.f / ttl, DisappearGravity, DisappearMaxSpeed); //rapidly disappear
		LifeCycleStatus = PORTAL_LIFECYCLE_DISAPPEARING;
	}

	if (LifeCycleStatus == PORTAL_LIFECYCLE_DISAPPEARING && IsIdle())
	{
		AnimationTimestamp = World->GetTimeSeconds();
		LifeCycleStatus = PORTAL_LIFECYCLE_DESTROYING;
	}

	if (LifeCycleStatus == PORTAL_LIFECYCLE_DESTROYING && World->GetTimeSeconds() - AnimationTimestamp > 3.f)
	{
		GetOwner()->Destroy(); //destroy the portal
	}
}

bool UPortalAnimatorComponent::HasDisappeared()
{
	return LifeCycleStatus == PORTAL_LIFECYCLE_DESTROYING;
}