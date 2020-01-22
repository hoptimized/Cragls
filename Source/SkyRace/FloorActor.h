// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorActor.generated.h"

UCLASS()
class SKYRACE_API AFloorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* SourceMaterial;

	class UMaterialInstanceDynamic* FloorMaterial;

	float ColorFadeStartTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeDelay = 2.f;

	uint16 LastStatus = -1;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
