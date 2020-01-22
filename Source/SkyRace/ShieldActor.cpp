// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldActor.h"

#include "CarPawn.h"

#include "Components/SphereComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneComponent.h"

#include "Curves/CurveFloat.h"
#include "UnrealNetwork.h"

AShieldActor::AShieldActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>("ShieldMesh");

	SetRootComponent(SphereCollision);
	ShieldMesh->SetupAttachment(SphereCollision);
}

void AShieldActor::BeginPlay()
{
	Super::BeginPlay();
	
	//Initialize dynamic material
	if (ShieldMesh != nullptr && BaseMaterial != nullptr) MaterialInstance = ShieldMesh->CreateDynamicMaterialInstance(0, BaseMaterial);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	LifecycleStatus = SHIELD_STATUS_GROWING;
	ScaleTimerStart = World->GetTimeSeconds();
}

void AShieldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (GrowCurve == nullptr) return;
	if (ShrinkCurve == nullptr) return;
	
	float MinTime;
	float CurrentTime = World->GetTimeSeconds();
	ElapsedTime = CurrentTime - ScaleTimerStart;

	if (LifecycleStatus == SHIELD_STATUS_GROWING)
	{
		SetActorScale3D(TargetScale * GrowCurve->GetFloatValue(ElapsedTime));
		GrowCurve->GetTimeRange(MinTime, MaxTime);
		if (ElapsedTime >= MaxTime)
		{
			LifecycleStatus = SHIELD_STATUS_FULL_SIZE;
			ScaleTimerStart = CurrentTime;
		}
	}
	else if (LifecycleStatus == SHIELD_STATUS_FULL_SIZE)
	{
		if (ElapsedTime > ShieldDuration)
		{
			LifecycleStatus = SHIELD_STATUS_SHRINKING;
			ScaleTimerStart = CurrentTime;
		}
	}
	else if (LifecycleStatus == SHIELD_STATUS_SHRINKING)
	{
		SetActorScale3D(TargetScale * ShrinkCurve->GetFloatValue(ElapsedTime));
		ShrinkCurve->GetTimeRange(MinTime, MaxTime);
		if (ElapsedTime >= MaxTime)
		{
			Destroy();
		}
	}

	AActor* ParentActor = GetAttachParentActor();
	if (ParentActor != nullptr && ShieldMesh != nullptr)
	{
		FVector CurrentLocation = ShieldMesh->GetComponentLocation();
		if (MeshLastLocation == FVector::ZeroVector) MeshLastLocation = CurrentLocation;

		ACarPawn* CarPawn = Cast<ACarPawn>(ParentActor);
		if (CarPawn != nullptr)
		{
			FVector TargetLocation = CarPawn->GetMeshOffsetRootLocation();
			
			CurrentLocation = FMath::VInterpTo(MeshLastLocation, TargetLocation, DeltaTime, 40.f);
			ShieldMesh->SetWorldLocation(CurrentLocation);
		}

		MeshLastLocation = CurrentLocation;
	}

	if (ImpactCurve == nullptr) return;
	if (MaterialInstance == nullptr) return;
	MaterialInstance->SetScalarParameterValue(FName("Radius"), ImpactCurve->GetFloatValue(World->GetTimeSeconds() - ImpactCurveStart) * 200.f);

	
}

void AShieldActor::GotHit(FVector ImpactPoint)
{
	if (!HasAuthority()) return;
	Multicast_Impact(ImpactPoint - ShieldMesh->GetComponentLocation());
}

void AShieldActor::Multicast_Impact_Implementation(FVector RelativeLocation)
{
	if (MaterialInstance == nullptr) return;
	if (ShieldMesh == nullptr) return;

	MaterialInstance->SetVectorParameterValue(FName("ImpactPoint"), ShieldMesh->GetComponentLocation() + RelativeLocation);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	ImpactCurveStart = World->GetTimeSeconds();
}

float AShieldActor::GetLifetimePercentage()
{
	if (LifecycleStatus == SHIELD_STATUS_GROWING) return ElapsedTime / MaxTime;
	if (LifecycleStatus == SHIELD_STATUS_FULL_SIZE) return 1 - (ElapsedTime / ShieldDuration);
	if (LifecycleStatus == SHIELD_STATUS_SHRINKING) return 0.f;

	return 0.0f;
}

void AShieldActor::ResetTimer()
{
	if (LifecycleStatus == SHIELD_STATUS_GROWING) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	SetActorScale3D(TargetScale);

	LifecycleStatus = SHIELD_STATUS_FULL_SIZE;
	ScaleTimerStart = World->GetTimeSeconds();
}