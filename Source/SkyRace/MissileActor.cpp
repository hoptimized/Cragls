// Fill out your copyright notice in the Description page of Project Settings.


#include "MissileActor.h"
#include "CarPawn.h"
#include "ShieldActor.h"

#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "Classes/Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"
#include "UnrealNetwork.h"

#include <limits>

AMissileActor::AMissileActor()
{

	//--------------------------------------------------------------------------------------------------
	// Actor pawn configuration

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	//--------------------------------------------------------------------------------------------------
	//Default values
	detectionDistance = 50000.f;
	detectionAngle = 60.f;
	LifeTime = 10.f;

	//--------------------------------------------------------------------------------------------------
	// Setup physical components

	//collision box
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	//BoxCollision->BodyInstance.SetCollisionProfileName(TEXT("Missile"));
	BoxCollision->OnComponentHit.AddDynamic(this, &AMissileActor::OnHit);

	//mesh offset root
	MeshOffsetRoot = CreateDefaultSubobject<USceneComponent>("MeshOffsetRoot");

	//the static mesh
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	
	//attach components
	SetRootComponent(BoxCollision);
	MeshOffsetRoot->SetupAttachment(BoxCollision);
	StaticMesh->SetupAttachment(MeshOffsetRoot);


	//--------------------------------------------------------------------------------------------------
	// Setup logical components

	MovementReplicated = CreateDefaultSubobject<USimplifiedVehicleMvmtReplicated>(TEXT("MovementReplicated"));

}

void AMissileActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMissileActor, TargetPawn);
}

// Called when the game starts or when spawned
void AMissileActor::BeginPlay()
{
	Super::BeginPlay();

	BoxCollision->IgnoreActorWhenMoving(GetOwner(), true);

	UWorld* World = GetWorld();
	if (!World) return;

	SpawnTimeSeconds = World->GetTimeSeconds();
}

// Called every frame
void AMissileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	if (HasAuthority()) {

		//Acquire and lock target, if this is a homing missile
		if (!TargetPawn.IsValid() && bCanAim) {
			float closestDistance = std::numeric_limits<float>::max();

			ACarPawn* tentativeTarget = nullptr;
			for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				APlayerController* PlayerController = Iterator->Get();

				ACarPawn* Candidate = Cast<ACarPawn>(PlayerController->GetPawn());
				if (!Candidate) return;

				bool CanTargetThisPawn = true;
				if (OwnerPawn.IsValid())
				{
					if (OwnerPawn.Get() == Candidate) CanTargetThisPawn = false;
				}

				if (CanTargetThisPawn)
				{
					//UE_LOG(LogTemp, Warning, TEXT("NOT the owner!"));

					FRotator currentRotation = GetActorRotation();
					FRotator lookAtRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Candidate->GetActorLocation());
					float turnAngle = UKismetMathLibrary::NormalizedDeltaRotator(lookAtRotator, currentRotation).Yaw;


					//UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), turnAngle);
					if (FMath::Abs(turnAngle) <= 45)
					{
						float distance = FVector(GetActorLocation() - Candidate->GetActorLocation()).Size();
						//UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), distance);
						if (distance <= detectionDistance)
						{
							if (distance < closestDistance)
							{
								tentativeTarget = Candidate;
								closestDistance = distance;
							}
						}
					}
				}
			}

			if (tentativeTarget != nullptr) {
				TargetPawn = TWeakObjectPtr<ACarPawn>(tentativeTarget);
				//UE_LOG(LogTemp, Warning, TEXT("Target acquired"));

				//tentativeTarget->Client_AddIncomingMissile(this);
			}
		}

		//Steer the missile towards its target
		if (TargetPawn.IsValid()) {
			FRotator currentRotation = GetActorRotation();
			FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPawn.Get()->GetActorLocation());
			float turnAngle = UKismetMathLibrary::NormalizedDeltaRotator(lookAtRotation, currentRotation).Yaw;
			//UE_LOG(LogTemp, Warning, TEXT("TurnAngle: %f"), turnAngle);

			if (FMath::Abs(turnAngle) > KINDA_SMALL_NUMBER) {
				float angleToSteering = FMath::DegreesToRadians(turnAngle) / (MovementReplicated->GetMaxAngularVelocity() * DeltaTime);
				MovementReplicated->SetSteeringThrow(FMath::Max(FMath::Min(angleToSteering,1.f),-1.f));
			}
		}
	}

	if (World->GetTimeSeconds() - SpawnTimeSeconds > LifeTime)  Multicast_ExplodeAndDie();
}

void AMissileActor::FireInDirection(ACarPawn* inOwnerPawn, const FVector& StartVelocity, bool inCanAim)
{
	OwnerPawn = TWeakObjectPtr<class ACarPawn>(inOwnerPawn);
	bCanAim = inCanAim;
	
	MovementReplicated->SetVelocity(StartVelocity);
	MovementReplicated->SetThrottle(1.f);
}

void AMissileActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	ACarPawn* OtherCar = Cast<ACarPawn>(OtherActor);
	if (OtherCar != nullptr)
	{
		OtherCar->GotHit();
	}

	AShieldActor* OtherShield = Cast<AShieldActor>(OtherActor);
	if (OtherShield != nullptr)
	{
		OtherShield->GotHit(Hit.ImpactPoint);
	}

	Multicast_ExplodeAndDie();
}

void AMissileActor::ExplodeAndDie()
{
	if (!HasAuthority()) return;
	Multicast_ExplodeAndDie();
}

void AMissileActor::Multicast_ExplodeAndDie_Implementation()
{
	SetActorScale3D(FVector::ZeroVector);

	UWorld* World = GetWorld();
	if (ExplosionActorClass != nullptr && World != nullptr)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		World->SpawnActor<AActor>(ExplosionActorClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
	}

	Destroy();
}