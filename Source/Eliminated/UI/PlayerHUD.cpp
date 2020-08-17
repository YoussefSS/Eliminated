// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUD.h"
#include "Components\Image.h"

bool UPlayerHUD::Initialize()
{
	Super::Initialize();

	if (!CrossHairImage) return false;

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
