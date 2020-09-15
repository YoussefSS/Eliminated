// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIStatus.generated.h"

/**
 * 
 */
UCLASS()
class ELIMINATED_API UAIStatus : public UUserWidget
{
	GENERATED_BODY()
	
public:
	bool Initialize() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UWidgetSwitcher* StatusIconSwitcher;

public:

	void SetStatusInvestigating();

	void SetStatusAggroed();
};
