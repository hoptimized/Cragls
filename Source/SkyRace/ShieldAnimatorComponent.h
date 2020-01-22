// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentScaleAnimator.h"
#include "ShieldAnimatorComponent.generated.h"

const int SHIELD_ANIMATOR_STATUS_INACTIVE = 0;
const int SHIELD_ANIMATOR_STATUS_ACTIVATING = 1;
const int SHIELD_ANIMATOR_STATUS_ACTIVE = 2;
const int SHIELD_ANIMATOR_STATUS_DEACTIVATING = 3;

/**
 * 
 */
UCLASS()
class SKYRACE_API UShieldAnimatorComponent : public UComponentScaleAnimator
{
	GENERATED_BODY()
	
protected:
	int ShieldAnimationStatus = SHIELD_ANIMATOR_STATUS_INACTIVE;

	FTimerHandle ShieldTimerHandle;

	UPROPERTY(EditAnywhere)
	float ShieldAbsorbWobbleMagnitude = 0.025f;

	UPROPERTY(EditAnywhere)
	float ShieldAbsorbWobbleFrequency = 4.f;

	UPROPERTY(EditAnywhere)
	float ShieldAbsorbWobbleDamping = 0.5f;

	UPROPERTY(EditAnywhere)
	float ScalingStartSpeed = 1.f;

	UPROPERTY(EditAnywhere)
	float ScalingGravity = 1.f;

	UPROPERTY(EditAnywhere)
	float MaxScalingSpeed = 5.f;

	UPROPERTY(EditAnywhere)
	float ShieldDuration = 10.f;

	UFUNCTION()
	void DeactivateShield();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ActivateShield();
	void Impact();
	void ResetShield();
	bool HasDisappeared();

	float GetShieldTimerRatio();

};
