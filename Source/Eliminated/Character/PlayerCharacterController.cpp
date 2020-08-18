// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"
#include "Blueprint\UserWidget.h"
#include "Eliminated\UI\PlayerHUD.h"

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetAsset)
	{
		HUDWidget = Cast<UPlayerHUD>(CreateWidget<UUserWidget>(this, HUDWidgetAsset));
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
			HUDWidget->SetVisibility(ESlateVisibility::Visible);
			HUDWidget->HideCrossHair();
		}
	}
}

void APlayerCharacterController::ShowCrossHair()
{
	if (HUDWidget)
	{
		HUDWidget->ShowCrossHair();
	}
}

void APlayerCharacterController::HideCrossHair()
{
	if (HUDWidget)
	{
		HUDWidget->HideCrossHair();
	}
}

void APlayerCharacterController::UpdateHUDAmmoCounter(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateAmmoCounterText(NewCurrentAmmo, NewCurrentClipAmmo);
	}
	
}
