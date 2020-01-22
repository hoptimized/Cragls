// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Types/SlateEnums.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomComboBox.generated.h"

/**
 * 
 */
UCLASS()
class SKYRACE_API UCustomComboBox : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(class ULobbyPlayerRow* inParent, bool inEnabled);
	
	void SetIsEnabledCustom(bool inIsEnabled);
	bool GetIsEnabledCustom() { return bIsEnabledCustom; };

	void SetColor(FString inColorHex);
	FString GetSelectedColor();

	UFUNCTION(BlueprintCallable)
	void OnPlayerColorChange(FString newColorHex, ESelectInfo::Type SelectType);

	void SetOptions(TArray<FString>);

protected:

	bool Initialize() override;
	
	class ULobbyPlayerRow* Parent;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* TheComboBox;

	UPROPERTY(meta = (BindWidget))
	class UImage* ImageOverlay;

	bool bIsEnabledCustom;

};
