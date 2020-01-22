// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SimplifiedVehicleMvmtReplicated.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissileActor.generated.h"


UCLASS()
class SKYRACE_API AMissileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMissileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Physical Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* MeshOffsetRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "References")
	TSubclassOf<AActor> ExplosionActorClass;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Logical Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USimplifiedVehicleMvmtReplicated* MovementReplicated;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Fields and Functions

	UPROPERTY(EditAnywhere, Category = Movement)
	float LifeTime;

	UPROPERTY(EditAnywhere, Category = Movement)
	float detectionDistance;

	UPROPERTY(EditAnywhere, Category = Movement)
	float detectionAngle;

	TWeakObjectPtr<class ACarPawn> OwnerPawn;

	UPROPERTY(replicated)
	TWeakObjectPtr<class ACarPawn> TargetPawn;

	float SpawnTimeSeconds;

	bool bCanAim;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void FireInDirection(ACarPawn* inOwnerPawn, const FVector& ShootDirection, bool inCanAim);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	TWeakObjectPtr<class ACarPawn> GetTargetPawn() { return TargetPawn; };

	void SetCanAim(bool Val) { bCanAim = Val; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExplodeAndDie();
	void ExplodeAndDie();
};
