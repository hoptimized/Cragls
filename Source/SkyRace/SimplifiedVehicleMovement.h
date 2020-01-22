// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimplifiedVehicleMovement.generated.h"

USTRUCT()
struct FSimplifiedVehicleMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1 && FMath::Abs(SteeringThrow) <= 1;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYRACE_API USimplifiedVehicleMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	USimplifiedVehicleMovement();

protected:

	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	// externally controllable parameters for the driving behavior

	// mass of the vehicle in kg
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// throttle force in Newtons
	UPROPERTY(EditAnywhere)
	float MaxThrottleForce = 7500;

	// brake force in Newtons
	UPROPERTY(EditAnywhere)
	float MaxBrakeForce = 11000;

	// unitless. Higher number leads to more drag
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 20;

	// unitless. Higher number leads to more rolling resistance
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.01;

	// minimum curve radius in meters
	UPROPERTY(EditAnywhere)
	float minCurveRadius = 10;

	// maximal lateral force, makes curve radius bigger for higher speeds.  in Newtons
	UPROPERTY(EditAnywhere)
	float maxLateralForce = 20000;

	// max velocity in meters per second
	UPROPERTY(EditAnywhere)
	float maxSpeed = 35;

	// max velocity in meters per second
	UPROPERTY(EditAnywhere)
	float ThrottleBoost = 4500;

	// radius of a wheel (in cm)
	UPROPERTY(EditAnywhere)
	float WheelRadius = 40;

	// radius of a wheel (in cm)
	UPROPERTY(EditAnywhere)
	float WheelsCenterToCenter = 3.45f;

	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	// internal physics variables and functions

private:
	FVector Velocity;
	float ThrottleInput;
	float SteeringThrowInput;
	
	FSimplifiedVehicleMove LastMove;
	bool isBoosted;
	bool isCutOff;
	//float WheelsRotation = 0.f; //0-1, represents 0-360deg rotation
	//float WheelsAngle = 0.f; //left/right turning of the front wheels

	FVector calculateFrictionDV(float DeltaTime);
	FVector calculateUserDV(float Throttle, float DeltaTime);
	void calculateCappedVelocity(FVector NaturalDV, FVector UserDV);
	void applyRotation(float SteeringThrow, float DeltaTime);
	void updateLocationFromVelocity(float DeltaTime);

protected:
	void updateWheels(float SteeringThrow, float DeltaTime);
	float calculateWheelsTurnAngle(float SteeringThrow);
	float calculateAngularVelocity(float SteeringThrow);
	float calculateCurveRadius(float AngularVelocity);

	float WheelsRotationFrequency;
	float WheelsTurnAngle;
	float CurveRadius = 0.f;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ExecuteMove(const FSimplifiedVehicleMove& Move);

	void SetMass(float Val) { Mass = Val; }
	void SetMaxThrottleForce(float Val) { maxSpeed = Val; }
	void SetMaxBrakeForce(float Val) { MaxBrakeForce = Val; }
	void SetDragCoefficient(float Val) { DragCoefficient = Val; }
	void SetminCurveRadius(float Val) { minCurveRadius = Val; }
	void SetMaxLateralForce(float Val) { maxLateralForce = Val; }
	void SetMaxSpeed(float Val) { maxSpeed = Val; }
	void SetVelocity(FVector Val) { Velocity = Val; };
	void SetThrottle(float Val) { ThrottleInput = Val; };
	void SetSteeringThrow(float Val) { SteeringThrowInput = Val; }
	void SetEngineCutOff(bool Val) { isCutOff = Val; }
	void SetBoost(bool Val) { isBoosted = Val; }

	float GetMaxSpeed() { return maxSpeed; };
	FVector GetVelocity() { return Velocity; };
	float GetThrottle() { return ThrottleInput; };
	bool GetBoost() { return isBoosted; }
	bool GetEngineCutOff() { return isCutOff; }
	float GetMaxAngularVelocity();
	FSimplifiedVehicleMove GetLastMove() { return LastMove; } //returns the last move (user input) that has been recorded
	FVector GetFrictionAcceleration();
	float GetWheelsRotationFrequency() { return WheelsRotationFrequency; };
	float GetWheelsTurnAngle() { return WheelsTurnAngle; };

	bool IsBraking() { return LastMove.Throttle < -KINDA_SMALL_NUMBER; };
};
