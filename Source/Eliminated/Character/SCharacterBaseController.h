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

	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets | PauseMenu")
	TSubclassOf< UUserWidget> PauseMenuWidgetAsset;

	/** Variable to hold the widget after creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets | PauseMenu")
	UUserWidget* PauseMenuWidget;

public:

	void ShowCrossHair();
	void HideCrossHair();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

	void UpdateHUDAmmoCounter(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo);

protected:

	bool bPauseMenuVisible = false;
};
