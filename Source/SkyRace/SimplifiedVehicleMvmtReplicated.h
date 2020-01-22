// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Queue.h"

#include "CoreMinimal.h"
#include "SimplifiedVehicleMovement.h"
#include "SimplifiedVehicleMvmtReplicated.generated.h"

USTRUCT()
struct FSimplifiedMovementState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	bool isThrottleBoost;

	UPROPERTY()
	bool isEngineCutOff;

	UPROPERTY()
	float RemoteTimestamp;

	UPROPERTY()
	FSimplifiedVehicleMove LastMove;
};

USTRUCT()
struct FTimestampedNetStatTuple
{
	GENERATED_USTRUCT_BODY()

	float Lag;
	float LocalTimestamp;
};

struct FHermiteCubicSpline
{
	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float InterpolationRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, InterpolationRatio);
	}
	FVector InterpolateDerivative(float InterpolationRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, InterpolationRatio);
	}
};

/**
 * 
 */

UCLASS()
class SKYRACE_API USimplifiedVehicleMvmtReplicated : public USimplifiedVehicleMovement
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USimplifiedVehicleMvmtReplicated();

protected:

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root; }	


	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	// Server

	//the canonical state of the pawn movement as dictated by the server. Automatically updated on each server tick
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FSimplifiedMovementState ServerState;

	FVector ServerVelocity;

	void ServerTick(FSimplifiedVehicleMove LastMove);
	void UpdateServerState(const FSimplifiedVehicleMove& Move, const float Timestamp); //called on the server: combines all parameters into a new server state and broadcasts it


	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	// Client

	//called on a client when a new canonical state is received
	UFUNCTION()
	void OnRep_ServerState();

	//--------------------------------------------------------------------------------------------
	// Autonomous Proxy
	
	TArray<FSimplifiedVehicleMove> UnacknowledgedMoves; //list of all moves (user input per user's tick) that the server has not yet confirmed
	float ClientSimulatedTime; //that's for cheat protection

	void AutoProxTick(FSimplifiedVehicleMove LastMove);
	void AutonomousProxy_OnRep_ServerState();
	void ClearAcknowledgeMoves(FSimplifiedVehicleMove LastMove); //delete all moves that are newer than the lastMove

	//send a move to the server for processing
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FSimplifiedVehicleMove Move, float Timestamp);

	//--------------------------------------------------------------------------------------------
	// Simulated Proxy

	void SimulatedProxy_OnRep_ServerState();

	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------
	// Buffering / Interpolation System

	//--------------------------------------------------------------------------------------------
	// Attributes

	//state queue
	TMap<float, FSimplifiedMovementState> RemoteStatesByRemoteTime;
	TArray<float> RemoteStateKeys;

	//current interpolation states
	FSimplifiedMovementState InterpolationStartState;
	FSimplifiedMovementState InterpolationTargetState;

	//timestamps and durations for interpolation
	float InterpolationTimeElapsed;
	float InterpolationTimeBetweenStates;

	//the actual interpolation
	FVector InterpolationLastFrameLocation;
	void InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio);
	void InterpolateVelocity(float DeltaTime);
	void InterpolateRotation(float LerpRatio);
	void InterpolateWheels(float SteeringThrow, float DeltaTime);
	float VelocityToDerivative();

	//network buffer metrics
	float const NetworkBufferMinSize = 0.05f;
	float NetworkBufferTargetSize = 0.f;
	float NetworkBufferActualSize = 0.f;

	//network condition metrics (for lag/jitter)
	float const LagDataTargetDuration = 3.f;
	TQueue<FTimestampedNetStatTuple> LagDataQueue;
	TArray<float> LagsBySizeDesc;

	//--------------------------------------------------------------------------------------------
	// Methods/Functions

	//the tick function for interpolated actors
	void InterpolatedActorTick(float DeltaTime);

	//managing the network buffer
	void AddRemoteStateToNetworkBuffer(FSimplifiedMovementState InState);
	void TrimNetworkBuffer();
	void CleanUpNetworkBufferUntil(const float inTimestamp);

	//getters for the buffer state queue
	FSimplifiedMovementState GetNewestState();
	FSimplifiedMovementState GetOldestState(float EarlyLimit = 0.f);
	TArray<FSimplifiedMovementState> GetCandidateStates(float LateLimit, float EarlyLimit = 0.f);
	
	//network statistics
	void CalculateNetStats();

	//connection between network buffer and interpolation system
	void SetInterpolationStates(FSimplifiedMovementState inStartState, FSimplifiedMovementState inTargetState);
	bool TryRestartInterpolation();
	void NextInterpolationStep();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool IsBraking();
};
