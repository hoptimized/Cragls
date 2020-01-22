// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplifiedVehicleMovement.h"

#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"

// Sets default values for this component's properties
USimplifiedVehicleMovement::USimplifiedVehicleMovement()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void USimplifiedVehicleMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//get network status
	bool hasAuthority = GetOwner()->HasAuthority();
	bool isLocallyControlled = false;
	bool isControlled = false;
	APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
	if (OwnerAsPawn)
	{
		isControlled = OwnerAsPawn->IsPawnControlled();
		isLocallyControlled = OwnerAsPawn->IsLocallyControlled();
	}

	// if we are the server or a self-controlled client, execute the move using current input
	if (isLocallyControlled || (hasAuthority && !isControlled))
	{
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr)) return;

		//Gather inputs
		FSimplifiedVehicleMove Move;
		Move.DeltaTime = DeltaTime;
		Move.SteeringThrow = SteeringThrowInput;
		Move.Throttle = ThrottleInput;
		Move.Time = World->TimeSeconds;

		//Save move
		LastMove = Move;

		//Execute the calculations
		ExecuteMove(LastMove);
	}
}

void USimplifiedVehicleMovement::ExecuteMove(const FSimplifiedVehicleMove& Move)
{
	//Get DV ( = Delta Velocity) from friction (air, tires) and user input
	FVector FrictionDV = calculateFrictionDV(Move.DeltaTime);
	FVector UserDV = calculateUserDV(Move.Throttle, Move.DeltaTime);

	//cap the velocity to the defined maximum
	calculateCappedVelocity(FrictionDV, UserDV);

	//apply new transform
	applyRotation(Move.SteeringThrow, Move.DeltaTime);
	updateLocationFromVelocity(Move.DeltaTime);

	//save wheel info
	updateWheels(Move.SteeringThrow, Move.DeltaTime);
}

FVector USimplifiedVehicleMovement::calculateFrictionDV(float DeltaTime)
{
	FVector DeltaVelocity = GetFrictionAcceleration() * DeltaTime;
	return DeltaVelocity;
}

FVector USimplifiedVehicleMovement::GetFrictionAcceleration()
{
	//Drag
	FVector Force = -Velocity.GetSafeNormal() * (Velocity.SizeSquared() / 2) * DragCoefficient;

	//Rolling resistance
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	Force += -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;

	//Convert force to acceleration and return
	return Force / Mass;
}

FVector USimplifiedVehicleMovement::calculateUserDV(float Throttle, float DeltaTime)
{
	FVector Force = GetOwner()->GetActorForwardVector();

	//Apply throttle force or brake force (brake is stronger than engine)
	if (Throttle >= 0)
	{
		//Disable gas pedal (for example after hit by missile)
		if (!isCutOff)
		{
			float booster = 0.f;
			if (isBoosted) booster = ThrottleBoost;
			Force = Force * (MaxThrottleForce + booster) * Throttle;
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("Cut off!"));
			Force = FVector::ZeroVector;
		}
	}
	else
	{
		Force = Force * MaxBrakeForce * Throttle;
	}

	//Convert into acceleration
	FVector Acceleration = Force / Mass;

	//Convert into a change in velocity
	FVector DeltaVelocity = Acceleration * DeltaTime;

	return DeltaVelocity;
}

void USimplifiedVehicleMovement::calculateCappedVelocity(FVector NaturalDV, FVector UserDV)
{
	//calculated proposed speed. DotProduct in combination with the forward vector yields the magnitude of velocity in the direction of travel, i.e. going backwards would show as a negative speed
	float proposedSpeed = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity + NaturalDV + UserDV);

	if (proposedSpeed > maxSpeed && !isBoosted)
	{
		//if exceeding max speed, cap the throttle, but do not actively brake
		float cappedSpeed = FMath::Max(maxSpeed - Velocity.Size() + NaturalDV.Size(), 0.f);
		FVector CappedUserDV = UserDV.GetSafeNormal() * cappedSpeed;
		Velocity += NaturalDV + CappedUserDV;
	}
	else if (proposedSpeed <= 0.f)
	{
		//if going backwards, stop the vehicle
		Velocity = FVector::ZeroVector;
	}
	else
	{
		//if 0 <= speed <= maxSpeed, just apply the user input
		Velocity += NaturalDV + UserDV;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Speed: %f"), Velocity.Size());
}

void USimplifiedVehicleMovement::applyRotation(float SteeringThrow, float DeltaTime)
{
	//Do nothing if input factors are kinda small
	if (Velocity.Size() < KINDA_SMALL_NUMBER) return;
	if (FMath::Abs(SteeringThrow) < KINDA_SMALL_NUMBER) return;

	//calculate actualAngularVelocity from user input (-1 <= input <= 1)
	float ActualAngularVelocity = calculateAngularVelocity(SteeringThrow);

	//save current curve radius. Will be useful for animation
	CurveRadius = calculateCurveRadius(ActualAngularVelocity);

	//calculate the actual rotationDelta from angularVelocity and DeltaTime, same as for speeds: d = w * t
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), ActualAngularVelocity * DeltaTime);						

	//rotate the velocity vector and the owner
	Velocity = RotationDelta.RotateVector(Velocity);															
	GetOwner()->AddActorWorldRotation(RotationDelta);															

}

float USimplifiedVehicleMovement::calculateAngularVelocity(float SteeringThrow)
{
	if (Velocity.Size() < KINDA_SMALL_NUMBER) return 0.f;

	float MaxAngularVelocity = GetMaxAngularVelocity();
	return MaxAngularVelocity * SteeringThrow;
}

float USimplifiedVehicleMovement::calculateCurveRadius(float AngularVelocity)
{
	if (Velocity.Size() < KINDA_SMALL_NUMBER) return 0.f;

	return Velocity.Size() / AngularVelocity;
}

float USimplifiedVehicleMovement::GetMaxAngularVelocity()
{
	//Do nothing if input factors are kinda small
	if (Velocity.Size() < KINDA_SMALL_NUMBER) return 0.f;

	//calculate min curve radius: curveRadius = (Mass * VelocitySquared) / MaxCentripetalForce. Makes the curve bigger for higher speeds, simulating that the driver turns more carefully to avoid losing grip. If going too slow, set curve radius to min curve radius.
	float minCurveRadiusForVelocity = FMath::Max((Mass * Velocity.SizeSquared()) / maxLateralForce, minCurveRadius);

	//get maxAngularVelocity from minCurveRadius, w = v / r
	float MaxAngularVelocity = Velocity.Size() / minCurveRadiusForVelocity;

	return MaxAngularVelocity;
}

void USimplifiedVehicleMovement::updateLocationFromVelocity(float DeltaTime)
{
	//Do nothing if velocity is kinda small
	if (Velocity.Size() < KINDA_SMALL_NUMBER) return;

	//Calculate and apply translation. Remember, Unity calculates in cm, we calculate in m. 1m = 100cm
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult Hit;
	GetOwner()->AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void USimplifiedVehicleMovement::updateWheels(float SteeringThrow, float DeltaTime)
{
	//we calculate in meters here, not in cm

	//abort if wheels radius is not set (avoid division by zero)
	if (FMath::Abs(WheelRadius) < KINDA_SMALL_NUMBER) return;

	//get the distance the car has moved within the current frame
	float MovedDistance = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;

	//get the rotations the wheels need to make to drive the MovedDistance
	float WheelsDeltaRotation = - MovedDistance / (2 * PI * (WheelRadius / 100));
	
	WheelsRotationFrequency = WheelsDeltaRotation / DeltaTime;

	//abort if wheeles have not moved
	//if (FMath::Abs(WheelsDeltaRotation) < KINDA_SMALL_NUMBER) return;

	//normalize DeltaRotation and calculate new rotation
	//WheelsDeltaRotation = (FMath::Abs(WheelsDeltaRotation) - FMath::Abs(floor(WheelsDeltaRotation))) * (WheelsDeltaRotation / FMath::Abs(WheelsDeltaRotation));
	//WheelsRotation = WheelsRotation + WheelsDeltaRotation + 1;
	//WheelsRotation = WheelsRotation - floor(WheelsRotation);

	WheelsTurnAngle = calculateWheelsTurnAngle(SteeringThrow);
}

float USimplifiedVehicleMovement::calculateWheelsTurnAngle(float SteeringThrow)
{
	if (FMath::Abs(WheelsCenterToCenter) < SMALL_NUMBER) return 0.f;
	if (FMath::Abs(SteeringThrow) < SMALL_NUMBER) return 0.f;

	float CurrentCurveRadius = 0.f;
	if (Velocity.Size() > KINDA_SMALL_NUMBER)
	{
		CurrentCurveRadius = CurveRadius;
	}
	else
	{
		CurrentCurveRadius = minCurveRadius * (SteeringThrow < SMALL_NUMBER ? -1.f : 1.f);
	}

	float AbsAngleRad = UKismetMathLibrary::Atan2(FMath::Abs(CurrentCurveRadius), WheelsCenterToCenter);
	float AbsAngleDeg = FMath::RadiansToDegrees(AbsAngleRad);
	float CalculatedAngle = (CurrentCurveRadius < 0 ? -1.f : 1.f) * (90.f - FMath::Clamp(AbsAngleDeg, 0.f, 90.f));

	if (Velocity.Size() > KINDA_SMALL_NUMBER)
	{
		return CalculatedAngle;
	}
	else
	{
		return CalculatedAngle * FMath::Abs(SteeringThrow);
	}
}