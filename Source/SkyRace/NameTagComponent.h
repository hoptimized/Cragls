// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "NameTagComponent.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UNameTagComponent : public UTextRenderComponent
{
	GENERATED_BODY()
	
public:
	UNameTagComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInterface* BaseMaterial;

	class UMaterialInstanceDynamic* MaterialInstance;

};
