// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShieldActor.generated.h"

const int SHIELD_STATUS_GROWING = 0;
const int SHIELD_STATUS_FULL_SIZE = 1;
const int SHIELD_STATUS_SHRINKING = 2;

UCLASS()
class SKYRACE_API AShieldActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AShieldActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* ShieldMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* BaseMaterial;

	class UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UCurveFloat* ImpactCurve;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UCurveFloat* GrowCurve;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UCurveFloat* ShrinkCurve;

	UPROPERTY(EditAnywhere, Category = "Components")
	FVector TargetScale;

	UPROPERTY(EditAnywhere)
	float ShieldDuration = 10.f;

	FTimerHandle ShieldTimerHandle;

	float ImpactCurveStart = 0.f;
	float ScaleTimerStart = 0.f;

	int LifecycleStatus = SHIELD_STATUS_GROWING;

	float ElapsedTime;
	float MaxTime;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Impact(FVector RelativeLocation);

	FVector MeshLastLocation;

public:	
	virtual void Tick(float DeltaTime) override;

	void GotHit(FVector ImpactPoint);
	float GetLifetimePercentage();
	void ResetTimer();

};
