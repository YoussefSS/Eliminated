// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Components\Image.h"
#include "Components\TextBlock.h"
#include "Eliminated\Character\PlayerCharacter.h"
#include "Kismet\GameplayStatics.h"

bool UPlayerHUD::Initialize()
{
	Super::Initialize();

	if (!CrossHairImage) return false;
	if (!AmmoCounterText) return false;

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


	AmmoCounterText->SetText(FText::FromString(NewAmmoText));
}
