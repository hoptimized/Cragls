// Fill out your copyright notice in the Description page of Project Settings.


#include "MessageBoard.h"

#include "Kismet/GameplayStatics.h"

#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"

// Sets default values
AMessageBoard::AMessageBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("TextComponent");
	TextComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMessageBoard::BeginPlay()
{
	Super::BeginPlay();
	
	BoardMaterial = TextComponent->CreateDynamicMaterialInstance(0, BaseMaterial);
	if (BoardMaterial == nullptr) return;

	TextComponent->SetTextMaterial(BoardMaterial);

	TextColor = FVector(250.f, 250.f, 250.f);

	BoardMaterial->SetVectorParameterValue("EmissiveColor", TextColor);
}

// Called every frame
void AMessageBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMessageBoard::SetText(FString inText)
{
	if (TextComponent == nullptr) return;

	TextComponent->SetText(FText::FromString(inText));
}

void AMessageBoard::SetColor(FVector inColor)
{
	TextColor = inColor;
	BoardMaterial->SetVectorParameterValue("EmissiveColor", TextColor);
}

void AMessageBoard::PlaySfx(USoundCue* inSoundCue)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	//Collect the SoundCue / replace with default if no soundcue is passed as parameter
	USoundCue* TheSoundcue = inSoundCue;
	if (TheSoundcue == nullptr) TheSoundcue = sfxCue;

	if (TheSoundcue == nullptr) return;

	if (TheSoundcue->IsValidLowLevel()) UGameplayStatics::PlaySound2D(World, TheSoundcue, 1.f, 1.f, 0.f);
}

void AMessageBoard::PlayAnimation(bool bFast)
{

}