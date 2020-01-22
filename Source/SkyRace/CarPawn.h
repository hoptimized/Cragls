// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SimplifiedVehicleMvmtReplicated.h"
#include "MissileAlertSystem.h"
#include "ShieldAnimatorComponent.h"
#include "MessageBoard.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CarPawn.generated.h"

USTRUCT()
struct FRadialBlurData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	class UStaticMeshComponent* RadialBlurMesh;

	UPROPERTY()
	class UMaterialInstanceDynamic* RadialBlurMaterial;

	FRadialBlurData(UStaticMeshComponent* inRadialBlurMesh) : RadialBlurMesh(inRadialBlurMesh), RadialBlurMaterial(nullptr) {}
	FRadialBlurData() : RadialBlurMesh(nullptr), RadialBlurMaterial(nullptr) {}
};

UCLASS()
class SKYRACE_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACarPawn();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* RadialBlurMeshFR;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* RadialBlurMeshFL;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* RadialBlurMeshRR;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* RadialBlurMeshRL;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* RadialBlurBaseMaterial;

	UPROPERTY(EditAnywhere, Category = "Components")
	float RimRadius = 42.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* PortalPointer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UAudioComponent* engineAudioComponent;

	UPROPERTY(EditAnywhere, Category = "Missile System")
	TSubclassOf<class AMissileActor> MissileClass;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UMaterialInterface* BrakeLightBaseMaterial;
	class UMaterialInstanceDynamic* BrakeLightMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
	class UNameTagComponent* NameTag;

	UPROPERTY(EditAnywhere, Category = "References")
	TSubclassOf<class AShieldActor> ShieldActorClass;

	TWeakObjectPtr<class AShieldActor> ShieldActor;

	UPROPERTY(EditAnywhere, Category = "References")
	TSubclassOf<class AEnergyRing> EnergyRingClass;

	UPROPERTY(EditAnywhere, Category = "References")
	TSubclassOf<AActor> ExplosionActorClass;



	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// SFX

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* engineAudioCue;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* engineAudioCueBoosted;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* scoredCue;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* countdownCue;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* speedUpCue;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* ShockSoundCue;

	UPROPERTY(EditAnywhere, Category = "SFX")
	class USoundCue* NoMissilesCue;

	UPROPERTY(EditAnywhere, Category = "SFX")
	float EngineMaxPitchVelocity = 35.f;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Logical Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USimplifiedVehicleMvmtReplicated* MovementComponentRplct;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMissileAlertSystem* MissileAlertSystem;

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Fields and Functions

	void Accelerate(float Value);
	void TurnRight(float Value);
	void Fire();

	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------
	// Red Code

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile System")
	float MissileSpawnOffset = 3.f;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_FireMissile();



	UPROPERTY(replicated)
	TWeakObjectPtr<class APortalActor> ScorePortalPtr;

	UFUNCTION()
	void DeactivateBoost();

	UFUNCTION()
	void ActivateEngine();

	FTimerHandle BoostTimerHandle;
	FTimerHandle CutOffTimerHandle;
	FTimerHandle SpecialProtectionTimerHandle;

	UPROPERTY(replicated)
	bool isSpecialProtected;

	UFUNCTION()
	void deactiveateSpecialProtection();

	

	FVector ShieldTargetScale;

	UPROPERTY(ReplicatedUsing = OnRep_MissileInventory)
	uint32 MissileInventory;

	UFUNCTION()
	void OnRep_MissileInventory();


	TSubclassOf<class UUserWidget> HUDClass;
	class USkyRaceHUD* SkyRaceHUD;
	void LoadHUD();

	UPROPERTY(ReplicatedUsing = OnRep_CarPaintMaterial)
	class UMaterialInterface* CarPaintMaterial;

	UFUNCTION()
	void OnRep_CarPaintMaterial();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AMessageBoard> MessageBoardClass;

	void SetCountdownThree();
	void SetCountdownTwo();
	void SetCountdownOne();
	void SetCountdownGo();
	void CancelCountdown();

	FTimerHandle CountdownTimerHandle;

	bool ReceivedDamage();

	UPROPERTY(ReplicatedUsing = OnRep_IsBraking)
	bool bIsBraking = false;

	UFUNCTION()
	void OnRep_IsBraking();

	TArray<FRadialBlurData> RadialBlurData;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Client, Reliable)
	void Client_HasScored();
	void HasScored();

	UFUNCTION(Client, Reliable)
	void Client_CollectMissile();
	void CollectMissile();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShieldImpact();

	/*UFUNCTION(Client, Reliable)
	void Client_GotHit();
	*/
	void GotHit();

	UFUNCTION(Client, Reliable)
	void Client_ActivateBoost();
	void ActivateBoost();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateShield();
	void ActivateShield();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_GotShocked();
	void GotShocked();

	UFUNCTION(BlueprintCallable)
	bool IsBraking() { return bIsBraking; };

	UFUNCTION(BlueprintCallable)
	float GetSpeedPercentage();

	UFUNCTION(BlueprintCallable)
	int32 GetSpeedInt();

	UFUNCTION(BlueprintCallable)
	float GetBoostTimerRatio();

	UFUNCTION(BlueprintCallable)
	float GetShieldTimerRatio();

	UFUNCTION(BlueprintCallable)
	int32 GetAlertLevel();


	UFUNCTION(BlueprintCallable)
	int32 GetNumNormalMissiles();

	UFUNCTION(BlueprintCallable)
	int32 GetNumHomingMissiles();

	void SetPaintMaterial(class UMaterialInterface* Material);

	USpringArmComponent* GetCameraArm() { return CameraArm; }

	void ShowMessage(FString inMessage, FVector inColor, bool bPlaySfx = false, class USoundCue* inSoundCue = nullptr);

	UFUNCTION(BlueprintCallable)
	float GetWheelsRotation() { return WheelsRotation; };

	UFUNCTION(BlueprintCallable)
	float GetWheelsTurnAngle();

	UPROPERTY(EditAnywhere)
	class UCurveFloat* WheelsRotationCurve;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* WheelsBlurCurve;

	void AnimateWheels(float DeltaTime);
	float WheelsRotation;
	float RadialBlurAlpha;

	FVector GetCameraLocation();
	FRotator GetCameraRotation();
	FVector GetMeshOffsetRootLocation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExplodeAndDie();
	void ExplodeAndDie();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	void activateSpecialProtection(float inDuration = 7.f);

	void InitializeCountdown();

	UPROPERTY(Replicated)
	bool bDoCountdown = false;

	UPROPERTY(Replicated)
	bool bEngineCutOff = true;

	void SetEngineCutOff(bool inVal);

	UFUNCTION()
	void OpenInGameMenu();
};
