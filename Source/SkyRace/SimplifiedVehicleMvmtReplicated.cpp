// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplifiedVehicleMvmtReplicated.h"

#include "UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Engine.h"

#include "MissileActor.h"

USimplifiedVehicleMvmtReplicated::USimplifiedVehicleMvmtReplicated()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USimplifiedVehicleMvmtReplicated::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USimplifiedVehicleMvmtReplicated, ServerState);
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// Branches

void USimplifiedVehicleMvmtReplicated::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Get network info
	bool hasAuthority = GetOwner()->HasAuthority();
	bool isPawn = false;
	bool isLocallyControlled = false;
	bool isControlled = false;
	APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
	if (OwnerAsPawn)
	{
		isPawn = true;
		isControlled = OwnerAsPawn->IsPawnControlled();
		isLocallyControlled = OwnerAsPawn->IsLocallyControlled();
	}

	//Retrieve the last move (a set of user input)
	FSimplifiedVehicleMove TempLastMove = GetLastMove();

	//AUTOPROX
	//This pawn is an autonomous proxy, i.e. it is running on a client and controlled by a client
	if (!hasAuthority && isLocallyControlled)
	{
		AutoProxTick(TempLastMove);
	}

	//AUTHORITY
	//This is the pawn of the authority, i.e. it is running on the server and controlled by the server's inputs
	if (hasAuthority && (isLocallyControlled || !isControlled))
	{
		ServerTick(TempLastMove);
	}

	//REMOTE PAWN (SimProx / AutoProx on Server)
	// 1) a pawn on a client that is controlled by the server or another client
	// 2) a pawn on the server that is controlled by a client
	if ((!hasAuthority && !isLocallyControlled) || (hasAuthority && !isLocallyControlled))
	{
		InterpolatedActorTick(DeltaTime);
	}

}

//This method is executed after a client has received an update from the server
void USimplifiedVehicleMvmtReplicated::OnRep_ServerState()
{
	//get network info
	bool hasAuthority = GetOwner()->HasAuthority();
	bool isPawn = false;
	bool isLocallyControlled = false;
	bool isControlled = false;
	APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
	if (OwnerAsPawn)
	{
		isPawn = true;
		isControlled = OwnerAsPawn->IsPawnControlled();
		isLocallyControlled = OwnerAsPawn->IsLocallyControlled();
	}

	//this function should never be called on the server
	if (hasAuthority) return;

	//AUTOPROX
	if (isLocallyControlled)
	{
		AutonomousProxy_OnRep_ServerState();
	}
	//SIMPROX
	else
	{
		SimulatedProxy_OnRep_ServerState();
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// SERVER

void USimplifiedVehicleMvmtReplicated::ServerTick(FSimplifiedVehicleMove Move)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	//Just take the incoming move and update the authorative server state
	UpdateServerState(Move, World->GetRealTimeSeconds());
}

//Update the server state. Basically just pulls the data out of the movement component. Note: this is the movement as simulated on the server, not on the client! Will be synced to the client periodically
void USimplifiedVehicleMvmtReplicated::UpdateServerState(const FSimplifiedVehicleMove& Move, const float Timestamp)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = GetVelocity();
	ServerState.isThrottleBoost = GetBoost();
	ServerState.isEngineCutOff = GetEngineCutOff();
	ServerState.RemoteTimestamp = Timestamp;

	// (synchronization and broadcasting is done automatically by UE4)
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// AUTONOMOUS PROXY

void USimplifiedVehicleMvmtReplicated::AutoProxTick(FSimplifiedVehicleMove Move)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	//save the last move (combination of inputs and deltatime) to the queue
	UnacknowledgedMoves.Add(Move);

	//send the last move to the server for execution
	Server_SendMove(Move, World->GetRealTimeSeconds());
}

//This function is executed on the server when it receives a client's input
void USimplifiedVehicleMvmtReplicated::Server_SendMove_Implementation(FSimplifiedVehicleMove Move, float Timestamp)
{
	//For cheat prevention
	ClientSimulatedTime += Move.DeltaTime;

	//----------------------------------------------------------
	// Auhorative State (Actor Transform, Velocity)

	//save the current velocity to a temp variable and set authorative velocity for the upcoming calculations
	FVector InterpolatedVelocity = GetVelocity();
	SetVelocity(ServerVelocity);

	//Move the actor's root
	ExecuteMove(Move);

	//Update and broadcast the ServerState
	UpdateServerState(Move, Timestamp);

	//----------------------------------------------------------
	// Local interpolated State (MeshOffsetRoot, Velocity)

	//switch back to the local (interpolated) velocity for the upcoming calculations
	ServerVelocity = GetVelocity();
	SetVelocity(InterpolatedVelocity);

	//add new state to the network buffer (state will be put in line, tick routine will pull the state and interpolate later)
	AddRemoteStateToNetworkBuffer(ServerState);
}

//Cheat prevention
bool USimplifiedVehicleMvmtReplicated::Server_SendMove_Validate(FSimplifiedVehicleMove Move, float Timestamp)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;
	bool ClientNotRunningAhead = ProposedTime < GetWorld()->TimeSeconds;
	if (!ClientNotRunningAhead) {
		UE_LOG(LogTemp, Error, TEXT("Client is running too fast."));
		return false;
	}

	if (!Move.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Received invalid move."));
		return false;
	}

	return true;
}

//This method is executed after an autonomous proxy (locally controlled client) has received an update from the server
void USimplifiedVehicleMvmtReplicated::AutonomousProxy_OnRep_ServerState()
{
	//Set our pawn to where the server dictates it to be
	GetOwner()->SetActorTransform(ServerState.Transform);
	SetVelocity(ServerState.Velocity);

	//Remove all moves from the queue that have been acknowledged by the server
	ClearAcknowledgeMoves(ServerState.LastMove);

	//!!!! TOOD: This is not running smoothly
	SetBoost(ServerState.isThrottleBoost);
	SetEngineCutOff(ServerState.isEngineCutOff);

	//Replay all unacknowledged moves so we stay in front of the server
	for (const FSimplifiedVehicleMove& Move : UnacknowledgedMoves)
	{
		ExecuteMove(Move);
	}
}

void USimplifiedVehicleMvmtReplicated::ClearAcknowledgeMoves(FSimplifiedVehicleMove Move)
{
	TArray<FSimplifiedVehicleMove> NewMoves;

	//only copy newer moves to the new queue
	for (const FSimplifiedVehicleMove& It : UnacknowledgedMoves)
	{
		if (It.Time > Move.Time)
		{
			NewMoves.Add(It);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// SIMULATED PROXY

//simulated proxy receives new server state
void USimplifiedVehicleMvmtReplicated::SimulatedProxy_OnRep_ServerState()
{
	AddRemoteStateToNetworkBuffer(ServerState);
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// NETWORK BUFFER / INTERPOLATION

//---------------------------------------------------------------------------------------------------------
//Tick for interpolated actors

void USimplifiedVehicleMvmtReplicated::InterpolatedActorTick(float DeltaTime)
{
	InterpolationTimeElapsed += DeltaTime;

	//UE_LOG(LogTemp, Warning, TEXT("%f - %f"), ClientStartState.RemoteTimestamp, ClientTargetState.RemoteTimestamp);

	//If we do not have valid start and target states, there's nothing we can do
	if (InterpolationStartState.RemoteTimestamp < SMALL_NUMBER || InterpolationTargetState.RemoteTimestamp < SMALL_NUMBER) return;

	float InterpolationRatio = InterpolationTimeElapsed / InterpolationTimeBetweenStates;

	//UE_LOG(LogTemp, Warning, TEXT("a=%f"), InterpolationRatio);

	if (InterpolationRatio > 1.f)
	{
		NextInterpolationStep();
		InterpolationRatio = InterpolationTimeElapsed / InterpolationTimeBetweenStates;
	}

	if (InterpolationRatio <= 1.f)
	{
		FHermiteCubicSpline Spline;

		Spline.TargetLocation = InterpolationTargetState.Transform.GetLocation();
		Spline.StartLocation = InterpolationStartState.Transform.GetLocation();
		Spline.StartDerivative = InterpolationStartState.Velocity * VelocityToDerivative();
		Spline.TargetDerivative = InterpolationTargetState.Velocity * VelocityToDerivative();

		InterpolateLocation(Spline, InterpolationRatio);
		InterpolateVelocity(DeltaTime);
		InterpolateRotation(InterpolationRatio);
		InterpolateWheels(InterpolationStartState.LastMove.SteeringThrow, DeltaTime);
	}

	if (NetworkBufferActualSize + InterpolationTimeBetweenStates - InterpolationTimeElapsed > 3 * NetworkBufferTargetSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Buffer FULL, stopping interpolation"));

		InterpolationStartState.RemoteTimestamp = 0.f;
		InterpolationTargetState.RemoteTimestamp = 0.f;
		TryRestartInterpolation();
	}
}

//---------------------------------------------------------------------------------------------------------
//Connection between buffer and interpolation

void USimplifiedVehicleMvmtReplicated::SetInterpolationStates(FSimplifiedMovementState inStartState, FSimplifiedMovementState inTargetState)
{
	bool hasError = false;
	if (inStartState.RemoteTimestamp < SMALL_NUMBER) hasError = true;
	if (inTargetState.RemoteTimestamp < SMALL_NUMBER) hasError = true;
	if (inTargetState.RemoteTimestamp - inStartState.RemoteTimestamp < SMALL_NUMBER) hasError = true;

	if (hasError)
	{
		InterpolationStartState.RemoteTimestamp = 0.f;
		InterpolationTargetState.RemoteTimestamp = 0.f;
		return;
	}

	InterpolationStartState = inStartState;
	InterpolationTargetState = inTargetState;

	InterpolationTimeBetweenStates = InterpolationTargetState.RemoteTimestamp - InterpolationStartState.RemoteTimestamp;
	InterpolationTimeElapsed = 0.f;
}

bool USimplifiedVehicleMvmtReplicated::TryRestartInterpolation() {

	//Check validity of ClientStates
	uint8 NumOfValidClientStates = 0;
	if (InterpolationStartState.RemoteTimestamp > SMALL_NUMBER) ++NumOfValidClientStates;
	if (InterpolationTargetState.RemoteTimestamp > SMALL_NUMBER) ++NumOfValidClientStates;

	if (NumOfValidClientStates >= 2) return false;

	//If we are restarting the interpolation anyways, wait until we have enough buffer before we start
	FSimplifiedMovementState NewestState = GetNewestState();
	if (NewestState.RemoteTimestamp < SMALL_NUMBER) return false;

	TArray<FSimplifiedMovementState> CandidateStates = GetCandidateStates(NewestState.RemoteTimestamp - NetworkBufferTargetSize);

	//If interpolation has not yet started or has been canceled, and if we have two valid states, set up the new states
	if (CandidateStates.Num() < 2) return false;

	//UE_LOG(LogTemp, Warning, TEXT("Restarting interpolation"));

	SetInterpolationStates(CandidateStates[CandidateStates.Num() - 2], CandidateStates[CandidateStates.Num() - 1]);

	InterpolationLastFrameLocation = InterpolationStartState.Transform.GetLocation();

	//Remove old states from the queue
	//UE_LOG(LogTemp, Warning, TEXT("Restarter calling CleanUp with timestamp=%f"), ClientTargetState.RemoteTimestamp);
	CleanUpNetworkBufferUntil(InterpolationTargetState.RemoteTimestamp);

	UE_LOG(LogTemp, Warning, TEXT("Restarting interpolation"));

	return true;
}

void USimplifiedVehicleMvmtReplicated::NextInterpolationStep()
{
	float Overshoot = InterpolationTimeElapsed - InterpolationTimeBetweenStates;
	FSimplifiedMovementState OldTargetState = InterpolationTargetState;
	FSimplifiedMovementState NewTargetState = GetOldestState(OldTargetState.RemoteTimestamp + Overshoot);

	if (NewTargetState.RemoteTimestamp < SMALL_NUMBER)
	{
		UE_LOG(LogTemp, Warning, TEXT("Buffer EMPTY, stopping interpolation"));
		InterpolationStartState.RemoteTimestamp = 0.f;
		InterpolationTargetState.RemoteTimestamp = 0.f;
		return;
	}

	SetInterpolationStates(OldTargetState, NewTargetState);

	if (InterpolationTargetState.RemoteTimestamp < SMALL_NUMBER) return;

	InterpolationTimeElapsed = FMath::Max(0.f, Overshoot - (InterpolationStartState.RemoteTimestamp - OldTargetState.RemoteTimestamp));

	CleanUpNetworkBufferUntil(InterpolationTargetState.RemoteTimestamp);

	//UE_LOG(LogTemp, Warning, TEXT("Next frame calling cleanup with timestamp=%f"), ClientTargetState.RemoteTimestamp);
}

//---------------------------------------------------------------------------------------------------------
//Managing the Network Buffer

void USimplifiedVehicleMvmtReplicated::AddRemoteStateToNetworkBuffer(FSimplifiedMovementState InState)
{
	//Add the new state to the network buffer
	RemoteStatesByRemoteTime.Add(InState.RemoteTimestamp, InState);
	RemoteStatesByRemoteTime.KeySort([](float A, float B) {
		return A < B;
		});
	RemoteStatesByRemoteTime.GenerateKeyArray(RemoteStateKeys);

	//Immediately set the actor's transform. This will only move the BoxCollision, NOT the visible mesh. The visible mesh will be moved by interpolation
	GetOwner()->SetActorTransform(InState.Transform, true);

	//Manage the network buffer
	CalculateNetStats();
	TrimNetworkBuffer();
	TryRestartInterpolation();
}

void USimplifiedVehicleMvmtReplicated::TrimNetworkBuffer()
{
	//Clean up the buffer if we are lagging too far behind
	if (NetworkBufferActualSize > 2 * NetworkBufferTargetSize)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Lag decreaser calling cleanup with timestamp=%f"), GetNewestState().RemoteTimestamp - SimProxBufferTargetSize);
		CleanUpNetworkBufferUntil(GetNewestState().RemoteTimestamp - NetworkBufferTargetSize);
	}
}

void USimplifiedVehicleMvmtReplicated::CleanUpNetworkBufferUntil(const float inTimestamp)
{
	if (inTimestamp > SMALL_NUMBER)
	{
		//UE_LOG(LogTemp, Warning, TEXT("inTimestamp=%f"), inTimestamp);

		//int RemovedEntries = 0;

		//TODO: just make this a for loop with break statement

		int InitialCount = RemoteStatesByRemoteTime.Num();

		int i = 0;
		while (i < InitialCount)
		{
			float LookupTimestamp = RemoteStatesByRemoteTime[RemoteStateKeys[i]].RemoteTimestamp;
			//UE_LOG(LogTemp,Warning,TEXT("Lookup=%f, inTimestamp=%f"), LookupTimestamp, inTimestamp);
			if (LookupTimestamp > inTimestamp)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Breaking, i=%d, ClientStateKey=%f, LookupTimestamp=%f, inTimestamp=%f"), i, RemoteStateKeys[i], LookupTimestamp, inTimestamp);
				break;
			}

			//UE_LOG(LogTemp, Warning, TEXT("Attempting to remove state"));
			RemoteStatesByRemoteTime.Remove(RemoteStateKeys[i]);

			//RemovedEntries++;

			//UE_LOG(LogTemp, Warning, TEXT("Removed entries=%d, Count=%d"), RemovedEntries, SimProxStateQueue.Num());

			++i;
		};
		RemoteStatesByRemoteTime.GenerateKeyArray(RemoteStateKeys);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Cleaned up. New Queue=%d"), SimProxStateQueue.Num());
}

//---------------------------------------------------------------------------------------------------------
//Getters for the Network Buffer

TArray<FSimplifiedMovementState> USimplifiedVehicleMvmtReplicated::GetCandidateStates(float LateLimit, float EarlyLimit)
{
	//Get all states that are ready to be simulated
	TArray<FSimplifiedMovementState> CandidateStates;
	for (int i = 0; i < RemoteStatesByRemoteTime.Num(); i++)
	{
		//if the state is ready for simulation, we might have a candidate
		if (RemoteStatesByRemoteTime[RemoteStateKeys[i]].RemoteTimestamp < LateLimit)
		{
			//Check whether the current state conflicts with the last state (or the function parameter, if any). We cannot add two states with the same timestamp, this would lead to division by zero in the interpolation
			if (RemoteStatesByRemoteTime[RemoteStateKeys[i]].RemoteTimestamp > EarlyLimit + SMALL_NUMBER)
			{
				CandidateStates.Add(RemoteStatesByRemoteTime[RemoteStateKeys[i]]);
				EarlyLimit = RemoteStatesByRemoteTime[RemoteStateKeys[i]].RemoteTimestamp;
			}
		}
		else
		{
			//the StateQueue is ordered by timestamp, so if we find the first state that is not yet due for simulation, we can stop checking the rest
			break;
		}
	};

	return CandidateStates;
}

FSimplifiedMovementState USimplifiedVehicleMvmtReplicated::GetOldestState(float EarlyLimit)
{
	FSimplifiedMovementState Output;

	if (RemoteStatesByRemoteTime.Num() == 0) return Output;

	for (int i = 0; i < RemoteStatesByRemoteTime.Num(); i++)
	{
		if (RemoteStatesByRemoteTime[RemoteStateKeys[i]].RemoteTimestamp > EarlyLimit)
		{
			Output = RemoteStatesByRemoteTime[RemoteStateKeys[i]];
			break;
		}
	}

	return Output;
}

FSimplifiedMovementState USimplifiedVehicleMvmtReplicated::GetNewestState()
{
	FSimplifiedMovementState Output;

	if (RemoteStatesByRemoteTime.Num() == 0) return Output;

	return RemoteStatesByRemoteTime[RemoteStateKeys[RemoteStateKeys.Num() - 1]];
}

//---------------------------------------------------------------------------------------------------------
//Network Statistics / Diagnostics

void USimplifiedVehicleMvmtReplicated::CalculateNetStats()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	//Get current local timestamp and lag
	float CurrentTimestamp = World->GetRealTimeSeconds();
	float CurrentLag = CurrentTimestamp - ServerState.RemoteTimestamp;

	FTimestampedNetStatTuple NewTuple;
	NewTuple.Lag = CurrentLag;
	NewTuple.LocalTimestamp = CurrentTimestamp;

	//Add new lag to the data pool
	LagDataQueue.Enqueue(NewTuple);
	LagsBySizeDesc.Add(CurrentLag);

	//Sort the lags by size
	LagsBySizeDesc.Sort([](const float A, float B) {
		return A > B;
	});

	//Delete timed-out lags (result should be the lags of all transmissions within the last x seconds)
	while (LagsBySizeDesc.Num() > 0)
	{
		FTimestampedNetStatTuple HeadTuple;
		LagDataQueue.Peek(HeadTuple);
		if (HeadTuple.LocalTimestamp < CurrentTimestamp - LagDataTargetDuration)
		{
			LagDataQueue.Pop();
			LagsBySizeDesc.RemoveSingle(HeadTuple.Lag);
		}
		else
		{
			break;
		}
	}

	//Get the smallest and biggest lag within the observed timeframe
	float LowestAdjustment = 0;
	float HighestAdjustment = 0;
	if (LagsBySizeDesc.Num() > 0)
	{
		LowestAdjustment = LagsBySizeDesc[LagsBySizeDesc.Num() - 1];
		HighestAdjustment = LagsBySizeDesc[0];
	}

	//Calculate jitter: difference between biggest and smallest lag, divided by 2
	float Jitter = (HighestAdjustment - LowestAdjustment) / 2.f;

	//Set Target Buffer Size
	NetworkBufferTargetSize = FMath::Max(Jitter * 1.5f, NetworkBufferMinSize);

	//Calculate actual buffer size
	NetworkBufferActualSize = GetNewestState().RemoteTimestamp - GetOldestState().RemoteTimestamp;

	//UE_LOG(LogTemp, Warning, TEXT("Buffer stats: frames=%d-%d, TargetSize=%fms, ActualSize=%fms, Jitter=%fms, 2*Jitter=%fms"), RemoteStateKeys.Num(), LagsBySizeDesc.Num(), NetworkBufferTargetSize*2000.f, NetworkBufferActualSize*1000.f, Jitter*1000.f, 2.f*Jitter*1000.f);
}

//---------------------------------------------------------------------------------------------------------
//Interpolation

void USimplifiedVehicleMvmtReplicated::InterpolateLocation(const FHermiteCubicSpline& Spline, float InterpolationRatio)
{
	FVector NewLocation = Spline.InterpolateLocation(InterpolationRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}
}

void USimplifiedVehicleMvmtReplicated::InterpolateVelocity(float DeltaTime)
{
	/*FVector NewDerivative = Spline.InterpolateDerivative(InterpolationRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	SetVelocity(NewVelocity.GetClampedToMaxSize(GetMaxSpeed() * 3.f));*/

	if (MeshOffsetRoot == nullptr) return;
	
	FVector MovedDistance = FVector(MeshOffsetRoot->GetComponentLocation() - InterpolationLastFrameLocation);
	SetVelocity(MovedDistance / DeltaTime / 100.f);

	InterpolationLastFrameLocation = MeshOffsetRoot->GetComponentLocation();
}

void USimplifiedVehicleMvmtReplicated::InterpolateRotation(float InterpolationRatio)
{
	FQuat TargetRotation = InterpolationTargetState.Transform.GetRotation();
	FQuat StartRotation = InterpolationStartState.Transform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, InterpolationRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}
}

void USimplifiedVehicleMvmtReplicated::InterpolateWheels(float SteeringThrow, float DeltaTime)
{
	if (MeshOffsetRoot == nullptr) return;
	if (FMath::Abs(WheelRadius) < KINDA_SMALL_NUMBER) return;

	float ActualAngularVelocity = calculateAngularVelocity(SteeringThrow);
	CurveRadius = calculateCurveRadius(ActualAngularVelocity);

	//get the distance the car has moved within the current frame
	//float MovedDistance = FVector(MeshOffsetRoot->GetComponentLocation() - InterpolationLastFrameLocation).Size() / 100.f;
	float MovedDistance = FVector(GetVelocity() * DeltaTime).Size();

	//get the rotations the wheels need to make to drive the MovedDistance
	float WheelsDeltaRotation = -MovedDistance / (2 * PI * (WheelRadius / 100.f));

	WheelsRotationFrequency = WheelsDeltaRotation / DeltaTime;

	//UE_LOG(LogTemp, Warning, TEXT("Velocity=%f"), GetVelocity().Size());

	WheelsTurnAngle = calculateWheelsTurnAngle(SteeringThrow);

	InterpolationLastFrameLocation = MeshOffsetRoot->GetComponentLocation();
}

float USimplifiedVehicleMvmtReplicated::VelocityToDerivative()
{
	return InterpolationTimeBetweenStates * 100;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------
// STATE INFO

bool USimplifiedVehicleMvmtReplicated::IsBraking()
{
	//Get network info
	bool hasAuthority = GetOwner()->HasAuthority();
	bool isLocallyControlled = false;
	bool isControlled = false;
	APawn* OwnerAsPawn = Cast<APawn>(GetOwner());
	if (OwnerAsPawn)
	{
		isControlled = OwnerAsPawn->IsPawnControlled();
		isLocallyControlled = OwnerAsPawn->IsLocallyControlled();
	}

	if (isLocallyControlled)
	{
		return Super::IsBraking();
	}
	else
	{
		if (hasAuthority)
		{
			return ServerState.LastMove.Throttle < -KINDA_SMALL_NUMBER;
		}
		else
		{
			if (InterpolationStartState.RemoteTimestamp > SMALL_NUMBER) return InterpolationStartState.LastMove.Throttle < -KINDA_SMALL_NUMBER;
		}
	}

	return false;
}