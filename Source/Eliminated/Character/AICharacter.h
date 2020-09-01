// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Eliminated/Character/PlayerCharacter.h"
#include "AICharacter.generated.h"

/**
 * 
 */
class ACustomTargetPoint;
UCLASS()
class ELIMINATED_API AAICharacter : public APlayerCharacter
{
	GENERATED_BODY()
	

public:

	/** Does this guard patrol between 2 or more points */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	bool bIsPatrol = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	TArray<ACustomTargetPoint*> PatrolPoints;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI")
	int32 CurrentTargetPointIndex = -1;

	UFUNCTION(BlueprintCallable, Category = "AI")
	ACustomTargetPoint* GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime);
};
