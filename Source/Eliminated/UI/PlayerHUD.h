// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */
UCLASS()
class ELIMINATED_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UImage* CrossHairImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* AmmoCounterText;

public:
	void ShowCrossHair();
	void HideCrossHair();

	void UpdateAmmoCounterText(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo);
};
