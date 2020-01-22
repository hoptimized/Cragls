// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComboBox.h"

#include "Components/Image.h"
#include "Menus/LobbyPlayerRow.h"
#include "Components/ComboBoxString.h"

bool UCustomComboBox::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	bIsEnabledCustom = true;

	return true;
}

void UCustomComboBox::Init(class ULobbyPlayerRow* inParent, bool inEnabled)
{
	Parent = inParent;

	if (TheComboBox == nullptr) return;
	TheComboBox->OnSelectionChanged.AddDynamic(this, &UCustomComboBox::OnPlayerColorChange);
}

void UCustomComboBox::OnPlayerColorChange(FString newColorHex, ESelectInfo::Type SelectType)
{
	Parent->OnPlayerColorChange(newColorHex);
}

void UCustomComboBox::SetColor(FString inColorHex)
{
	if (TheComboBox == nullptr) return;
	TheComboBox->SetSelectedOption(inColorHex);

	if (ImageOverlay == nullptr) return;
	ImageOverlay->SetColorAndOpacity(FLinearColor(FColor::FromHex(inColorHex)));
}

void UCustomComboBox::SetIsEnabledCustom(bool inIsEnabled)
{
	bIsEnabledCustom = inIsEnabled;

	if (TheComboBox == nullptr) return;
	if (ImageOverlay == nullptr) return;

	if (bIsEnabledCustom)
	{
		TheComboBox->SetVisibility(ESlateVisibility::Visible);
		ImageOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		TheComboBox->SetVisibility(ESlateVisibility::Hidden);
		ImageOverlay->SetVisibility(ESlateVisibility::Visible);
	}
}

FString UCustomComboBox::GetSelectedColor()
{
	if (TheComboBox == nullptr) return FString();
	return TheComboBox->GetSelectedOption();
}

void UCustomComboBox::SetOptions(TArray<FString> inOptions)
{ 
	if (TheComboBox == nullptr) return;
	if (inOptions.Num() == 0) return;

	TheComboBox->ClearOptions();

	for (auto It = inOptions.CreateConstIterator(); It; ++It)
	{
		FString ThisColorHex = *It;
		TheComboBox->AddOption(ThisColorHex);
	}

	SetColor(inOptions[0]);
}