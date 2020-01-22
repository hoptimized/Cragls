// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MessageBoard.generated.h"

UCLASS()
class SKYRACE_API AMessageBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMessageBoard();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTextRenderComponent* TextComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* sfxCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* BaseMaterial;

	class UMaterialInstanceDynamic* BoardMaterial;

	FVector TextColor;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetText(FString inText);
	void SetColor(FVector inColor);
	void PlaySfx(USoundCue* inSoundCue = nullptr);
	void PlayAnimation(bool bFast);

};
