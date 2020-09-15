// Fill out your copyright notice in the Description page of Project Settings.


#include "AIStatus.h"
#include "Components\WidgetSwitcher.h"

bool UAIStatus::Initialize()
{
	Super::Initialize();

	if (!StatusIconSwitcher) return false;

	return true;
}

void UAIStatus::SetStatusInvestigating()
{
	if (StatusIconSwitcher)
	{
		StatusIconSwitcher->SetActiveWidgetIndex(0);
	}
}

void UAIStatus::SetStatusAggroed()
{
	if (StatusIconSwitcher)
	{
		StatusIconSwitcher->SetActiveWidgetIndex(1);
	}
}

