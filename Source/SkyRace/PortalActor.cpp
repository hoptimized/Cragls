// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalActor.h"
#include "CarPawn.h"
#include "SkyRaceGameModeBase.h"
#include "ShieldActor.h"

#include "Components/SceneComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"

#include "Classes/Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealNetwork.h"

// Sets default values
APortalActor::APortalActor()
{

	//--------------------------------------------------------------------------------------------------
	// Basic actor configuration

	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	SetReplicateMovement(true);


	//--------------------------------------------------------------------------------------------------
	// Setup physical components

	//mesh root
	MeshRoot = CreateDefaultSubobject<USceneComponent>("MeshOffsetRoot");
	

	//the actual mesh
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMesh->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnOverlapBegin);
	StaticMesh->SetIsReplicated(false);
	

	//attach components
	SetRootComponent(MeshRoot);
	StaticMesh->SetupAttachment(MeshRoot);


	//--------------------------------------------------------------------------------------------------
	// Setup logical components

	PortalAnimatorComponent = CreateDefaultSubobject<UPortalAnimatorComponent>(TEXT("PortalAnimatorComponent"));

}


void APortalActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APortalActor, PortalInitInfo);
}


// Called when the game starts or when spawned
void APortalActor::BeginPlay()
{
	Super::BeginPlay();

	//Retreive portal materials from blueprint
	PortalMaterials = TArray<UMaterialInterface*>();
	PortalMaterials.Insert(ScorePortalMaterial, PORTAL_TYPE_SCORE_PORTAL);
	PortalMaterials.Insert(SpeedPortalMaterial, PORTAL_TYPE_SPEED_PORTAL);
	PortalMaterials.Insert(ShieldPortalMaterial, PORTAL_TYPE_SHIELD_PORTAL);
	PortalMaterials.Insert(MissilePortalMaterial, PORTAL_TYPE_MISSILE_PORTAL);
	PortalMaterials.Insert(ShockPortalMaterial, PORTAL_TYPE_SHOCK_PORTAL);

	//Set actor scale to 1 and mesh scale to 0
	SetActorScale3D(FVector::OneVector);
	StaticMesh->SetWorldScale3D(FVector::ZeroVector);
	
	//If we are a client and the server has already sent init info, do the initialization
	if (PortalInitInfo.isInitialized) InitializePortal(PortalInitInfo.portalType, PortalInitInfo.ttl);
}


//On replication of init info (executed by client)
void APortalActor::OnRep_Initialization()
{
	//UE_LOG(LogTemp, Warning, TEXT("Client received init info"));
	//if (PortalInitInfo.isInitialized) UE_LOG(LogTemp, Warning, TEXT("Information complete"));

	//portalType and ttl should be replicated from server at the same time as isInitialized is
	if (PortalInitInfo.isInitialized) InitializePortal(PortalInitInfo.portalType, PortalInitInfo.ttl);
}


//Actual initialization function
void APortalActor::InitializePortal(uint32 inPortalType, float inTtl)
{

	//If the portal materials have not been loaded, this function is called on a client, before BeginPlay has even been called. Return and wait for BeginPlay to call again
	if (PortalMaterials.Num() == 0) return;

	//Check, if the required material is available
	if (PortalMaterials[inPortalType] == nullptr) return;
	StaticMesh->SetMaterial(0, PortalMaterials[inPortalType]);

	if (inPortalType == PORTAL_TYPE_SHOCK_PORTAL) StaticMesh->SetCollisionProfileName(FName("ShockPortal"));

	//Server only: set the portal init info, will be replicated to clients
	if (HasAuthority())
	{
		PortalInitInfo.portalType = inPortalType;
		PortalInitInfo.ttl = inTtl;
		if (inPortalType == PORTAL_TYPE_SCORE_PORTAL) PortalInitInfo.ttl = 0.f;	//Score Portals do not disappear!
		PortalInitInfo.isInitialized = true;
	}

	//Make the portal appear, start the animation
	PortalAnimatorComponent->Appear(PortalInitInfo.ttl);
}


void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//nothing to do here, all done in animator component
}


//Collision function
void APortalActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	//Collision can only be calculated by authority
	if (!HasAuthority()) return;

	//Do not execute calculations for portals being destroyed
	if (PortalAnimatorComponent != nullptr)
	{
		if (PortalAnimatorComponent->HasDisappeared()) return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Overlap!"));

	UWorld* World = GetWorld();
	if (World == nullptr) return;

	//Get the overlapping car
	ACarPawn* Car = Cast<ACarPawn>(OtherActor);
	if (Car != nullptr)
	{
		//Depending on the portal type, interact with the car pawn
		if (PortalInitInfo.portalType == PORTAL_TYPE_SCORE_PORTAL)
		{
			Car->HasScored();

			ASkyRaceGameModeBase* GM = Cast<ASkyRaceGameModeBase>(World->GetAuthGameMode());
			if (GM)
			{
				GM->RespawnScorePortal();
			}
		}
		else if (PortalInitInfo.portalType == PORTAL_TYPE_SPEED_PORTAL)
		{
			Car->ActivateBoost();
		}
		else if (PortalInitInfo.portalType == PORTAL_TYPE_SHIELD_PORTAL)
		{
			Car->ActivateShield();
		}
		else if (PortalInitInfo.portalType == PORTAL_TYPE_MISSILE_PORTAL)
		{
			Car->CollectMissile();
		}
		else if (PortalInitInfo.portalType == PORTAL_TYPE_SHOCK_PORTAL)
		{
			Car->GotShocked();
		}

		Destroy();
	}

	AShieldActor* Shield = Cast<AShieldActor>(OtherActor);
	if (Shield != nullptr)
	{
		Shield->GotHit(SweepResult.ImpactPoint);

		Destroy();
	}

	
}

int32 APortalActor::GetPortalType()
{
	return PortalInitInfo.portalType;
}

void APortalActor::ExplodeAndDie()
{
	if (!HasAuthority()) return;
	Multicast_ExplodeAndDie();
}

void APortalActor::Multicast_ExplodeAndDie_Implementation()
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