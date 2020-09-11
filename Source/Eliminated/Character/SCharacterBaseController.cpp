// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacterBaseController.h"
#include "Blueprint\UserWidget.h"
#include "Eliminated\UI\PlayerHUD.h"
#include "Kismet\GameplayStatics.h"
#include "Eliminated\EliminatedGameModeBase.h"

void ASCharacterBaseController::BeginPlay()
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

			HUDWidget->UpdateEnemiesRemainingTextFromGM();
		}
	}

	if (PauseMenuWidgetAsset)
	{
		PauseMenuWidget = Cast<UUserWidget>(CreateWidget<UUserWidget>(this, PauseMenuWidgetAsset));
		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToViewport(0);
			PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (LoseMenuWidgetAsset)
	{
		LoseMenuWidget = Cast<UUserWidget>(CreateWidget<UUserWidget>(this, LoseMenuWidgetAsset));
		if (LoseMenuWidget)
		{
			LoseMenuWidget->AddToViewport(0);
			LoseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (VictoryMenuWidgetAsset)
	{
		VictoryMenuWidget = Cast<UUserWidget>(CreateWidget<UUserWidget>(this, VictoryMenuWidgetAsset));
		if (VictoryMenuWidget)
		{
			VictoryMenuWidget->AddToViewport(0);
			VictoryMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	AEliminatedGameModeBase* GM = Cast<AEliminatedGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->OnEnemyDied.AddDynamic(this, &ASCharacterBaseController::OnEnemyDied);
	}
}

void ASCharacterBaseController::ShowCrossHair()
{
	if (HUDWidget)
	{
		HUDWidget->ShowCrossHair();
	}
}

void ASCharacterBaseController::HideCrossHair()
{
	if (HUDWidget)
	{
		HUDWidget->HideCrossHair();
	}
}

void ASCharacterBaseController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}

	bPauseMenuVisible = !bPauseMenuVisible;
}

void ASCharacterBaseController::ShowPauseMenu()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 0);

	if (PauseMenuWidget)
	{
		PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;

		FInputModeUIOnly InputModeUIOnly;
		SetInputMode(InputModeUIOnly);
	}
}

void ASCharacterBaseController::HidePauseMenu()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1);

	if (PauseMenuWidget)
	{
		PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		bShowMouseCursor = false;

		FInputModeGameOnly InputModeGameOnly;
		SetInputMode(InputModeGameOnly);
	}
}

void ASCharacterBaseController::ShowLoseMenu()
{
	if (LoseMenuWidget)
	{
		LoseMenuWidget->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;

		FInputModeUIOnly InputModeUIOnly;
		SetInputMode(InputModeUIOnly);
	}
}

void ASCharacterBaseController::ShowVictoryMenu()
{
	if (VictoryMenuWidget)
	{
		VictoryMenuWidget->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;

		FInputModeUIOnly InputModeUIOnly;
		SetInputMode(InputModeUIOnly);
	}
}

void ASCharacterBaseController::UpdateHUDAmmoCounter(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateAmmoCounterText(NewCurrentAmmo, NewCurrentClipAmmo);
	}
	
}

void ASCharacterBaseController::UpdateHealthBar(float MaxHealth, float CurrentHealth)
{
	if (HUDWidget)
	{
		HUDWidget->UpdateHealthBar(MaxHealth, CurrentHealth);
	}
}

void ASCharacterBaseController::OnEnemyDied(AActor* DeadEnemy, int32 RemainingEnemies)
{
	// No more enemies remaining
	if (RemainingEnemies <= 0)
	{
		ShowVictoryMenu();
	}

	if (HUDWidget)
	{
		HUDWidget->UpdateEnemiesRemainingTextFromGM();
	}
}
