// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SCharacterBaseController.generated.h"

/**
 * 
 */

class UPlayerHUD;
UCLASS()
class ELIMINATED_API ASCharacterBaseController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;


	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets | InGameHUD")
	TSubclassOf< UUserWidget> HUDWidgetAsset;

	/** Variable to hold the widget after creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets | InGameHUD")
	UPlayerHUD* HUDWidget;

public:

	void ShowCrossHair();
	void HideCrossHair();

	void UpdateHUDAmmoCounter(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo);

};
