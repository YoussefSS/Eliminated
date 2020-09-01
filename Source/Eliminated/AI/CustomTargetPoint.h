// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "CustomTargetPoint.generated.h"

/**
 * 
 */
UCLASS()
class ELIMINATED_API ACustomTargetPoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	float GetSecondsToStay() { return SecondsToStay; }

protected:

	/** How many seconds should the AI stay at this target point */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "TargetPoint")
	float SecondsToStay;
};
