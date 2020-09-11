// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Components\Image.h"
#include "Components\TextBlock.h"
#include "Eliminated\Character\SCharacterBase.h"
#include "Kismet\GameplayStatics.h"
#include "Eliminated\EliminatedGameModeBase.h"

bool UPlayerHUD::Initialize()
{
	Super::Initialize();

	if (!CrossHairImage) return false;
	if (!AmmoCounterText) return false;
	if (!EnemiesRemainingText) return false;

	return true;
}

void UPlayerHUD::ShowCrossHair()
{
	if (CrossHairImage)
	{
		CrossHairImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlayerHUD::HideCrossHair()
{
	if (CrossHairImage)
	{
		CrossHairImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerHUD::UpdateAmmoCounterText(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo)
{
	FString NewAmmoText = FString::FromInt(NewCurrentClipAmmo)
		.Append(" / ")
		.Append(FString::FromInt(NewCurrentAmmo));


	if (AmmoCounterText)
	{
		AmmoCounterText->SetText(FText::FromString(NewAmmoText));
	}
	
}

void UPlayerHUD::UpdateEnemiesRemainingTextFromGM()
{
	// Let the game mode know you were spawned
	AEliminatedGameModeBase* GM = Cast<AEliminatedGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		FString NewEnemyRemainingText = FString::FromInt(GM->GetCurrentNumberOfEnemies())
			.Append(" / ")
			.Append(FString::FromInt(GM->GetMaxNumOfEnemies()));

		if (EnemiesRemainingText)
		{
			EnemiesRemainingText->SetText(FText::FromString(NewEnemyRemainingText));
		}
	}
}
