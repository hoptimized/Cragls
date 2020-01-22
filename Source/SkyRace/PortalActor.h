// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PortalAnimatorComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

const int PORTAL_TYPE_SCORE_PORTAL = 0;
const int PORTAL_TYPE_SPEED_PORTAL = 1;
const int PORTAL_TYPE_SHIELD_PORTAL = 2;
const int PORTAL_TYPE_MISSILE_PORTAL = 3;
const int PORTAL_TYPE_SHOCK_PORTAL = 4;

USTRUCT()
struct FPortalInitInfo
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	bool isInitialized = false;

	UPROPERTY()
	int32 portalType;

	UPROPERTY()
	float ttl;
};

UCLASS()
class SKYRACE_API APortalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Physical Components

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USceneComponent* MeshRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticMesh;

	//Materials
	TArray<class UMaterialInterface*> PortalMaterials;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* ScorePortalMaterial;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* SpeedPortalMaterial;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* ShieldPortalMaterial;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* MissilePortalMaterial;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* ShockPortalMaterial;

	UPROPERTY(EditAnywhere, Category = "References")
	TSubclassOf<AActor> ExplosionActorClass;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Logical Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPortalAnimatorComponent* PortalAnimatorComponent;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Other Fields

	//Initialization info replicated and sent to the clients in a batch
	UPROPERTY(ReplicatedUsing = OnRep_Initialization)
	FPortalInitInfo PortalInitInfo;

	UFUNCTION()
	void OnRep_Initialization();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int32 GetPortalType();

	void InitializePortal(uint32 inPortalType, float inTtl);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExplodeAndDie();
	void ExplodeAndDie();

};
