// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "PortalActor.h"
#include "MissileActor.h"
#include "SkyRaceGameModeBase.h"
#include "SkyRacePlayerState.h"
#include "SkyRacePlayerController.h"
#include "EnergyRing.h"
#include "NameTagComponent.h"
#include "ShieldActor.h"

#include "Classes/Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Classes/Components/AudioComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Curves/CurveFloat.h"
#include "Particles/ParticleSystem.h"

#include "Kismet/KismetMathLibrary.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ACarPawn::ACarPawn()
{

	//--------------------------------------------------------------------------------------------------
	// Basic pawn configuration

	PrimaryActorTick.bCanEverTick = true;

	//bUseControllerRotationPitch = false;
	//bUseControllerRotationYaw = true;
	//bUseControllerRotationRoll = false;

	bReplicates = true;
	SetReplicateMovement(false);


	//--------------------------------------------------------------------------------------------------
	// Defaults

	MissileInventory = 0;

	//--------------------------------------------------------------------------------------------------
	// Setup physical components

	//collision box
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	BoxCollision->OnComponentHit.AddDynamic(this, &ACarPawn::OnHit);

	//mesh offset root
	MeshOffsetRoot = CreateDefaultSubobject<USceneComponent>("MeshOffsetRoot");
	MeshOffsetRoot->SetIsReplicated(false);

	//the actual skeletal mesh
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");

	//Radial Blur Meshes for the wheels
	RadialBlurMeshFR = CreateDefaultSubobject<UStaticMeshComponent>("RadialBlurMeshFR");
	RadialBlurMeshFL = CreateDefaultSubobject<UStaticMeshComponent>("RadialBlurMeshFL");
	RadialBlurMeshRR = CreateDefaultSubobject<UStaticMeshComponent>("RadialBlurMeshRR");
	RadialBlurMeshRL = CreateDefaultSubobject<UStaticMeshComponent>("RadialBlurMeshRL");

	//portal pointer
	PortalPointer = CreateDefaultSubobject<UStaticMeshComponent>("PortalPointer");
	PortalPointer->SetRelativeLocation(FVector(0.f, 0.f, 200.f));

	//name tag
	NameTag = CreateDefaultSubobject<UNameTagComponent>("NameTag");
	NameTag->SetRelativeLocation(FVector(0.f, 0.f, 180.f));

	//camera arm
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetRelativeRotation(FRotator(-15.f,0.f,0.f));
	CameraArm->TargetArmLength = 700.0f;
	CameraArm->bEnableCameraLag = true;
	CameraArm->CameraLagSpeed = 10.f;
	CameraArm->bEnableCameraRotationLag = true;
	CameraArm->CameraRotationLagSpeed = 10.f;

	//camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetRelativeRotation(FRotator(15.f, 0.f, 0.f));

	//engine sound
	engineAudioComponent = CreateDefaultSubobject<UAudioComponent>("engineAudioComponent");
	engineAudioComponent->bAutoActivate = false;
	engineAudioComponent->SetIsReplicated(true);
	
	//attach components
	SetRootComponent(BoxCollision);
	MeshOffsetRoot->SetupAttachment(BoxCollision);
	PortalPointer->SetupAttachment(MeshOffsetRoot);
	NameTag->SetupAttachment(MeshOffsetRoot);
	SkeletalMesh->SetupAttachment(MeshOffsetRoot);
	RadialBlurMeshFR->SetupAttachment(SkeletalMesh, FName("TyreFR"));
	RadialBlurMeshFL->SetupAttachment(SkeletalMesh, FName("TyreFL"));
	RadialBlurMeshRR->SetupAttachment(SkeletalMesh, FName("TyreRR"));
	RadialBlurMeshRL->SetupAttachment(SkeletalMesh, FName("TyreRL"));
	CameraArm->SetupAttachment(MeshOffsetRoot);
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	engineAudioComponent->SetupAttachment(MeshOffsetRoot);

	//--------------------------------------------------------------------------------------------------
	// Setup logical components

	MovementComponentRplct = CreateDefaultSubobject<USimplifiedVehicleMvmtReplicated>(TEXT("MovementComponentRplct"));
	MissileAlertSystem = CreateDefaultSubobject<UMissileAlertSystem>(TEXT("MissileAlertSystem"));

}

void ACarPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACarPawn, ScorePortalPtr);
	DOREPLIFETIME(ACarPawn, isSpecialProtected);
	DOREPLIFETIME(ACarPawn, MissileInventory);
	DOREPLIFETIME(ACarPawn, CarPaintMaterial);
	DOREPLIFETIME(ACarPawn, bIsBraking);
	DOREPLIFETIME(ACarPawn, bDoCountdown);
	
}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	LoadHUD();

	if (engineAudioCue->IsValidLowLevel()) {
		engineAudioComponent->SetSound(engineAudioCue);
	}
	engineAudioComponent->Play();

	//Initialize brake light
	if (SkeletalMesh != nullptr)
	{
		int32 BrakeLightMatIndex = 18; //5;
		BrakeLightMaterial = SkeletalMesh->CreateDynamicMaterialInstance(BrakeLightMatIndex, BrakeLightBaseMaterial);
		if (BrakeLightMaterial != nullptr) BrakeLightMaterial->SetScalarParameterValue(FName("V_Multiplier"), bIsBraking ? 10.f : 1.f);
	}

	//Initialize blur data
	if (RadialBlurMeshFR != nullptr) RadialBlurData.Add(FRadialBlurData(RadialBlurMeshFR));
	if (RadialBlurMeshFL != nullptr) RadialBlurData.Add(FRadialBlurData(RadialBlurMeshFL));
	if (RadialBlurMeshRR != nullptr) RadialBlurData.Add(FRadialBlurData(RadialBlurMeshRR));
	if (RadialBlurMeshRL != nullptr) RadialBlurData.Add(FRadialBlurData(RadialBlurMeshRL));

	for (int i = 0; i < RadialBlurData.Num(); i++)
	{
		RadialBlurData[i].RadialBlurMaterial = RadialBlurData[i].RadialBlurMesh->CreateDynamicMaterialInstance(0, RadialBlurBaseMaterial);
		if (RadialBlurData[i].RadialBlurMaterial != nullptr)
		{
			RadialBlurData[i].RadialBlurMaterial->SetScalarParameterValue(FName("Angle"), 0.f);
			RadialBlurData[i].RadialBlurMaterial->SetScalarParameterValue(FName("RimRadius"), RimRadius);
		}
	}

	//Deavtivate collision detection for clients. Server takes care of that
	if (!HasAuthority()) BoxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	if (HasAuthority())
	{
		//NetUpdateFrequency = 1; // Replication takes place every 1 / NetUpdateFrequency seconds. NetUpdateFrequency = 10 means 10Hz or 100ms, equals 10FPS. UpdateFrequency should be slower than server FPS, otherwise interpolation may get weird
	}
	
	if (bDoCountdown) InitializeCountdown();
	if (MovementComponentRplct != nullptr) MovementComponentRplct->SetEngineCutOff(bEngineCutOff);

	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	// REMOVE BEFORE FLIGHT!

	// REMOVE BEFORE FLIGHT!
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	if (ensure(PortalPointer != nullptr))
	{
		if (ScorePortalPtr.IsValid())
		{
			FRotator CurrentRotation = PortalPointer->GetComponentRotation();
			FVector CarLocation = GetActorLocation();
			FVector ScorePortalLocation = ScorePortalPtr.Get()->GetActorLocation();
			float TargetYaw = UKismetMathLibrary::FindLookAtRotation(CarLocation, ScorePortalLocation).Yaw;
			FRotator TargetRotation = FRotator(0.f, TargetYaw, 0.f);
			PortalPointer->SetWorldRotation(FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.f)); //Pitch, Yaw, Roll
		}
		else
		{
			//ScorePortalPtr = Cast<ASkyRaceGameStateBase>(GetWorld()->GetGameState())->ScorePortalPtr;
			if (HasAuthority())
			{
				ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
				if (!GM) return;

				ScorePortalPtr = GM->GetScorePortalPtr();
			}
			
		}
		
	}

	//Wheels
	AnimateWheels(DeltaTime);

	//Brake lights
	if ((IsLocallyControlled() || HasAuthority()) && MovementComponentRplct != nullptr) bIsBraking = MovementComponentRplct->IsBraking();	//overwrite replicated serverstate for self-simulated cars
	if (BrakeLightMaterial != nullptr)
	{
		BrakeLightMaterial->SetScalarParameterValue(FName("V_Multiplier"), bIsBraking ? 10.f : 1.f);
	}

	if (isSpecialProtected && SkeletalMesh != nullptr)
	{
		int test = (int)(round(World->GetTimeSeconds() * 2)) % 2;
		//UE_LOG(LogTemp, Warning, TEXT("flag=%d"), test);
		
		if (test == 0)
		{
			SkeletalMesh->SetVisibility(true, true);
		} 
		else
		{
			SkeletalMesh->SetVisibility(false, true);
		}
	}
	else
	{
		if (!SkeletalMesh->IsVisible()) SkeletalMesh->SetVisibility(true, true);
	}


	if (MovementComponentRplct != nullptr)
	{
		if (engineAudioComponent->IsValidLowLevel())
		{
			engineAudioComponent->SetFloatParameter(FName("Pitch"), (MovementComponentRplct->GetVelocity().Size() / EngineMaxPitchVelocity)); //MovementComponent->GetMaxSpeed()
		}
	}
}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Accelerate", this, &ACarPawn::Accelerate);
	PlayerInputComponent->BindAxis("Turn", this, &ACarPawn::TurnRight);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACarPawn::Fire);
	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &ACarPawn::OpenInGameMenu);
}

void ACarPawn::Accelerate(float Value)
{
	if (MovementComponentRplct == nullptr) return;
	MovementComponentRplct->SetThrottle(Value);
}


void ACarPawn::TurnRight(float Value)
{
	if (MovementComponentRplct == nullptr) return;
	MovementComponentRplct->SetSteeringThrow(Value);
}

void ACarPawn::Fire()
{
	if (MissileInventory > 0)
	{
		Server_FireMissile();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No missiles to fire"));
		
		UWorld* World = GetWorld();
		if (!ensure(World != nullptr)) return;

		if (NoMissilesCue != nullptr)
		{
			if (NoMissilesCue->IsValidLowLevel()) UGameplayStatics::PlaySound2D(World, NoMissilesCue, 1.f, 1.f, 0.f);
		}
	}
}

void ACarPawn::Server_FireMissile_Implementation()
{

	// Attempt to fire a projectile.
	if (MissileClass && MissileInventory > 0)
	{
		// Calculate spawning location in front of the car
		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * MissileSpawnOffset * 100;
		FRotator SpawnRotation = GetActorRotation();

		UWorld* World = GetWorld();
		if (!ensure(World != nullptr)) return;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Setting instigator to %s"), *(Instigator->GetName())));

		//Spawn missile
		AMissileActor* Missile = World->SpawnActor<AMissileActor>(MissileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (Missile)
		{
			BoxCollision->IgnoreActorWhenMoving(Missile, true);

			bool spawnedMissileCanHome = false;
			if (MissileInventory >= 3)
			{
				spawnedMissileCanHome = true;
				MissileInventory -= 3;
			}
			else
			{
				spawnedMissileCanHome = false;
				MissileInventory -= 1;
			}

			// Set the projectile's initial trajectory: same direction and speed as the car
			Missile->FireInDirection(this, MovementComponentRplct->GetVelocity(), spawnedMissileCanHome);
		}
	}
}

bool ACarPawn::Server_FireMissile_Validate()
{
	//TODO: cheat protection

	return true;
}

void ACarPawn::HasScored()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(GetPlayerState());
	if (!PS) return;

	UE_LOG(LogTemp, Warning, TEXT("Scored! (on server)"));
	PS->Score++;

	Client_HasScored();
}
void ACarPawn::Client_HasScored_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Scored!"));

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (scoredCue != nullptr)
	{
		if (scoredCue->IsValidLowLevel())
		{
			UGameplayStatics::PlaySound2D(World, scoredCue, 1.f, 1.f, 0.f);
		}
	}
	
	if (MessageBoardClass == nullptr) return;
	ShowMessage("Scored!", FVector(0.f, 0.f, 250.f), true);
}

void ACarPawn::ActivateBoost()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (!MovementComponentRplct) return;

	if (BoostTimerHandle.IsValid()) World->GetTimerManager().ClearTimer(BoostTimerHandle);
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("DeactivateBoost"));
	World->GetTimerManager().SetTimer(BoostTimerHandle, TimerDel, 12.f, false);

	MovementComponentRplct->SetBoost(true);

	Client_ActivateBoost();
}
void ACarPawn::Client_ActivateBoost_Implementation()
{
	ShowMessage("Speed Boost!", FVector(0.f, 50.f, 0.f), true);

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (speedUpCue == nullptr) return;
	if (!speedUpCue->IsValidLowLevel()) return;

	UGameplayStatics::PlaySound2D(World, speedUpCue, 1.f, 1.f, 0.f);

	//Quick and dirty solution: the client needs to have a timer for the speed boost indicator.
	if (!HasAuthority())
	{
		if (BoostTimerHandle.IsValid()) World->GetTimerManager().ClearTimer(BoostTimerHandle);
		FTimerDelegate TimerDel;
		World->GetTimerManager().SetTimer(BoostTimerHandle, TimerDel, 12.f, false);
	}
}

void ACarPawn::DeactivateBoost()
{
	MovementComponentRplct->SetBoost(false);
}

void ACarPawn::GotHit()
{
	if (!HasAuthority()) return;

	ReceivedDamage();
	
}

bool ACarPawn::ReceivedDamage()
{
	if (!HasAuthority()) return false;

	//Cancel if currently invincible
	if (isSpecialProtected) return false;

	UE_LOG(LogTemp, Warning, TEXT("The car got hit!"));

	UWorld* World = GetWorld();
	if (World == nullptr) return false;

	//Decrement score
	ASkyRacePlayerState* PS = Cast<ASkyRacePlayerState>(GetPlayerState());
	if (!PS) return false;
	PS->Score = FMath::Max((int)(PS->Score) - 1, 0);

	//Enable special protection
	activateSpecialProtection(7.f);

	//Cut off engine
	if (!MovementComponentRplct) return false;
	if (CutOffTimerHandle.IsValid()) World->GetTimerManager().ClearTimer(CutOffTimerHandle);
	FTimerDelegate CutOffTimerDel;
	CutOffTimerDel.BindUFunction(this, FName("ActivateEngine"));
	World->GetTimerManager().SetTimer(CutOffTimerHandle, CutOffTimerDel, 5.f, false);
	MovementComponentRplct->SetEngineCutOff(true);

	return true;
}


void ACarPawn::activateSpecialProtection(float inDuration)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (SpecialProtectionTimerHandle.IsValid()) World->GetTimerManager().ClearTimer(SpecialProtectionTimerHandle);
	FTimerDelegate ProtectionTimerDel;
	ProtectionTimerDel.BindUFunction(this, FName("deactiveateSpecialProtection"));
	World->GetTimerManager().SetTimer(SpecialProtectionTimerHandle, ProtectionTimerDel, inDuration, false);
	isSpecialProtected = true;
}

void ACarPawn::ActivateEngine()
{
	if (!HasAuthority()) return;

	MovementComponentRplct->SetEngineCutOff(false);
}

void ACarPawn::deactiveateSpecialProtection()
{
	if (!HasAuthority()) return;

	isSpecialProtected = false;
}

void ACarPawn::ActivateShield()
{
	if (!HasAuthority()) return;
	Multicast_ActivateShield();
}
void ACarPawn::Multicast_ActivateShield_Implementation()
{
	if (IsPawnControlled() && IsLocallyControlled()) ShowMessage("Shields activated", FVector(70.f, 0.f, 70.f), true);

	if (!ShieldActor.IsValid())
	{
		UWorld* World = GetWorld();
		if (World == nullptr) return;

		if (ShieldActorClass == nullptr) return;

		FActorSpawnParameters spawnParameters;
		spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ShieldActor = World->SpawnActor<AShieldActor>(ShieldActorClass, GetActorLocation(), GetActorRotation(), spawnParameters);
	
		if (!ShieldActor.IsValid()) return;
		
		FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
		ShieldActor->AttachToComponent(RootComponent, AttachmentTransformRules);
		
	}
	else
	{
		ShieldActor->ResetTimer();
	}
}

void ACarPawn::Multicast_ShieldImpact_Implementation()
{
	//ShieldAnimatorComponent->Impact();
}

void ACarPawn::GotShocked()
{
	if (!HasAuthority()) return;

	//Broadcast the damage, if damage applies
	if (ReceivedDamage()) Multicast_GotShocked();
}
void ACarPawn::Multicast_GotShocked_Implementation()
{
	//Show message only for affected player
	if (IsLocallyControlled()) ShowMessage("Shocked!", FVector(70.f, 30.f, 0.f), false);

	if (EnergyRingClass == nullptr) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (MeshOffsetRoot == nullptr) return;

	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AEnergyRing* ER = World->SpawnActor<AEnergyRing>(EnergyRingClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnParameters);
	if (ER == nullptr) return;

	FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
	ER->AttachToComponent(MeshOffsetRoot, AttachmentTransformRules);

	if (ShockSoundCue != nullptr)
	{
		if (ShockSoundCue->IsValidLowLevel()) UGameplayStatics::PlaySoundAtLocation(World, ShockSoundCue, GetActorLocation(), 1.f, 1.f, 0.f);
	}
}
void ACarPawn::CollectMissile()
{
	if (!HasAuthority()) return;

	++MissileInventory;

	Client_CollectMissile();
}
void ACarPawn::Client_CollectMissile_Implementation()
{
	ShowMessage("Collected Blaster", FVector(100.f, 0.f, 0.f), true);

	//TODO: Play sound, animation or something else, so that the player knows that he has successfully collected a missile

	/*if (scoredCue == nullptr) return;
	if (!scoredCue->IsValidLowLevel()) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	UGameplayStatics::PlaySound2D(World, scoredCue, 1.f, 1.f, 0.f);*/
}

void ACarPawn::LoadHUD()
{
	/*if (HUDClass == nullptr) return;

	SkyRaceHUD = CreateWidget<USkyRaceHUD>(GetWorld(), HUDClass);
	if (SkyRaceHUD == nullptr) return;

	SkyRaceHUD->Setup();
	//MainMenu->SetMenuInterface(this);*/
}

void ACarPawn::OnRep_MissileInventory()
{
	if (SkyRaceHUD == nullptr) return;
	//SkyRaceHUD->SetMissiles(MissileInventory);
}

/*bool ACarPawn::IsBraking()
{
	if (MovementReplicator != nullptr)
	{
		return MovementReplicator->IsBraking();
	}
	else
	{
		if (MovementComponent == nullptr) return false;
		return (MovementComponent->IsBraking());
	}
	
}*/

void ACarPawn::OnRep_IsBraking()
{

}

float ACarPawn::GetSpeedPercentage()
{
	if (MovementComponentRplct == nullptr) return false;
	return MovementComponentRplct->GetVelocity().Size() / EngineMaxPitchVelocity;
}

int32 ACarPawn::GetSpeedInt()
{
	return FMath::RoundHalfFromZero(MovementComponentRplct->GetVelocity().Size() * 3.6);
	/*int32 speedInt = FMath::RoundHalfFromZero(MovementComponent->GetVelocity().Size() * 3.6);
	FString speedString = FString::FromInt(speedInt);
	FString combinedString = speedString;// +FString(TEXT(" km/h"));
	FText theText = FText::FromString(combinedString);
	return theText;*/
}

float ACarPawn::GetBoostTimerRatio()
{
	UWorld* World = GetWorld();
	if (!World) return 0.f;

	if (!BoostTimerHandle.IsValid()) return 0.f;
	if (World->GetTimerManager().GetTimerRate(BoostTimerHandle) < KINDA_SMALL_NUMBER) return 0.f;

	return 1 - (World->GetTimerManager().GetTimerElapsed(BoostTimerHandle) / World->GetTimerManager().GetTimerRate(BoostTimerHandle));
}

float ACarPawn::GetShieldTimerRatio()
{
	if (!ShieldActor.IsValid()) return 0.f;
	return ShieldActor->GetLifetimePercentage();
}

int32 ACarPawn::GetAlertLevel()
{
	if (MissileAlertSystem == nullptr) return -1;
	return MissileAlertSystem->GetCurrentAlertLevel();
}

int32 ACarPawn::GetNumNormalMissiles()
{
	return MissileInventory % 3;
}

int32 ACarPawn::GetNumHomingMissiles()
{
	return FMath::FloorToInt(MissileInventory / 3);
};

void ACarPawn::SetPaintMaterial(class UMaterialInterface* Material)
{
	CarPaintMaterial = Material;
	SkeletalMesh->SetMaterial(6, CarPaintMaterial);
	SkeletalMesh->SetMaterial(26, CarPaintMaterial);
}
void ACarPawn::OnRep_CarPaintMaterial()
{
	SetPaintMaterial(CarPaintMaterial);
}

void ACarPawn::ShowMessage(FString inMessage, FVector inColor, bool bPlaySfx, USoundCue* inSoundCue)
{
	if (MessageBoardClass == nullptr) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	USpringArmComponent* SpringArm = GetCameraArm();
	if (SpringArm == nullptr) return;

	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMessageBoard* MB = World->SpawnActor<AMessageBoard>(MessageBoardClass, FVector::ZeroVector, FRotator::ZeroRotator, spawnParameters);

	if (MB == nullptr) return;

	MB->SetText(inMessage);
	MB->SetColor(inColor);
	if (bPlaySfx) MB->PlaySfx(inSoundCue);

	FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true);
	MB->AttachToComponent(SpringArm, AttachmentTransformRules, FName("SpringEndpoint"));

	//MB->PlayAnimation(bFast);
}

void ACarPawn::InitializeCountdown()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	CountdownTimerHandle;
	World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ACarPawn::SetCountdownThree, 2.f);
}

void ACarPawn::SetCountdownThree()
{
	ShowMessage("3", FVector(25.f, 25.f, 25.f));

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	CountdownTimerHandle;
	World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ACarPawn::SetCountdownTwo, 1.f);

	if (countdownCue == nullptr) return;
	if (!countdownCue->IsValidLowLevel()) return;

	UGameplayStatics::PlaySound2D(World, countdownCue, 1.f, 1.f, 2.f);
}

void ACarPawn::SetCountdownTwo()
{
	ShowMessage("2", FVector(25.f, 25.f, 25.f));

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	CountdownTimerHandle;
	World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ACarPawn::SetCountdownOne, 1.f);
}

void ACarPawn::SetCountdownOne()
{
	ShowMessage("1", FVector(25.f, 25.f, 25.f));

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	CountdownTimerHandle;
	World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ACarPawn::SetCountdownGo, 1.f);
}

void ACarPawn::SetCountdownGo()
{
	ShowMessage("GO!", FVector(25.f, 25.f, 25.f));
}

void ACarPawn::CancelCountdown()
{
	if (!CountdownTimerHandle.IsValid()) return;

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	World->GetTimerManager().ClearTimer(CountdownTimerHandle);
}

void ACarPawn::AnimateWheels(float DeltaTime)
{
	if (MovementComponentRplct == nullptr) return;
	if (WheelsRotationCurve == nullptr) return;
	
	float WheelsRotationFrequencyRaw = MovementComponentRplct->GetWheelsRotationFrequency();
	float WheelsRotationFrequency = WheelsRotationCurve->GetFloatValue(FMath::Abs(WheelsRotationFrequencyRaw));
	float WheelsDeltaRotation = WheelsRotationFrequency * DeltaTime;
	WheelsDeltaRotation = (WheelsRotationFrequencyRaw < 0) ? -WheelsDeltaRotation : WheelsDeltaRotation;

	if (FMath::Abs(WheelsDeltaRotation) > SMALL_NUMBER)
	{
		WheelsDeltaRotation = (FMath::Abs(WheelsDeltaRotation) - FMath::Abs(floor(WheelsDeltaRotation))) * (WheelsDeltaRotation / FMath::Abs(WheelsDeltaRotation));
		WheelsRotation = WheelsRotation + WheelsDeltaRotation + 1;
		WheelsRotation = WheelsRotation - floor(WheelsRotation);
	}

	if (WheelsBlurCurve == nullptr) return;

	RadialBlurAlpha = WheelsBlurCurve->GetFloatValue(FMath::Abs(WheelsRotationFrequencyRaw));

	for (int i = 0; i < RadialBlurData.Num(); i++)
	{
		if (RadialBlurData[i].RadialBlurMaterial != nullptr) RadialBlurData[i].RadialBlurMaterial->SetScalarParameterValue(FName("Angle"), RadialBlurAlpha);
	}
}

float ACarPawn::GetWheelsTurnAngle()
{
	if (MovementComponentRplct == nullptr) return 0.f;

	return MovementComponentRplct->GetWheelsTurnAngle();
}

FVector ACarPawn::GetCameraLocation()
{
	if (Camera == nullptr) return FVector();

	return Camera->GetComponentLocation();
}
FRotator ACarPawn::GetCameraRotation()
{
	if (Camera == nullptr) return FRotator();

	return Camera->GetComponentRotation();
}

FVector ACarPawn::GetMeshOffsetRootLocation()
{
	if (MeshOffsetRoot == nullptr) return FVector();

	return MeshOffsetRoot->GetComponentLocation();
}


void ACarPawn::ExplodeAndDie()
{
	if (!HasAuthority()) return;

	Multicast_ExplodeAndDie();
}

void ACarPawn::Multicast_ExplodeAndDie_Implementation()
{
	if (MissileAlertSystem != nullptr) MissileAlertSystem->TurnOff();

	UWorld* World = GetWorld();
	if (ExplosionActorClass != nullptr && World != nullptr)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		World->SpawnActor<AActor>(ExplosionActorClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
	}

	Destroy();
}

void ACarPawn::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	if (OtherComponent->GetCollisionObjectType() == ECollisionChannel::ECC_WorldStatic || OtherComponent->GetCollisionProfileName().IsEqual(FName("BlockAllDynamic")))
	{
		ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(GetController());
		if (PC == nullptr) return;

		PC->LostPawn();
	}
}

void ACarPawn::SetEngineCutOff(bool inVal)
{
	if (HasAuthority()) bEngineCutOff = inVal;

	if (MovementComponentRplct == nullptr) return;
	MovementComponentRplct->SetEngineCutOff(inVal);
}

void ACarPawn::OpenInGameMenu()
{
	ASkyRacePlayerController* PC = Cast<ASkyRacePlayerController>(GetController());
	if (PC == nullptr) return;

	PC->OpenInGameMenu();
}
