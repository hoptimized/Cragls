// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentScaleAnimator.h"
#include "PortalAnimatorComponent.generated.h"

const int PORTAL_LIFECYCLE_SPAWNED = 1;
const int PORTAL_LIFECYCLE_GROWING = 2;
const int PORTAL_LIFECYCLE_LEVELING_OUT = 3;
const int PORTAL_LIFECYCLE_APPEARED = 4;
const int PORTAL_LIFECYCLE_SHRINKING = 5;
const int PORTAL_LIFECYCLE_DISAPPEARING = 6;
const int PORTAL_LIFECYCLE_DESTROYING = 7;

/**
 * 
 */
UCLASS()
class SKYRACE_API UPortalAnimatorComponent : public UComponentScaleAnimator
{
	GENERATED_BODY()
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	int LifeCycleStatus;
	float AnimationTimestamp;
	float ttl;

	UPROPERTY(EditAnywhere, Category = "Appear")
	float AppearTargetRatio = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Appear")
	float AppearStartSpeed = 0.f;

	UPROPERTY(EditAnywhere, Category = "Appear")
	float AppearGravity = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Appear")
	float AppearMaxSpeed = 1.f;

	UPROPERTY(EditAnywhere, Category = "Wobble")
	float WobbleDamping = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Shrink")
	float ShrinkingTargetRatio = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Disappear")
	float DisappearGravity = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Disappear")
	float DisappearMaxSpeed = 5.f;

public:
	void Appear(float inTtl);
	bool HasDisappeared();

};
