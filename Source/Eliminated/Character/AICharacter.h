// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Eliminated/Character/SCharacterBase.h"
#include "AICharacter.generated.h"

/**
 * 
 */
class ACustomTargetPoint;

UCLASS()
class ELIMINATED_API AAICharacter : public ASCharacterBase
{
	GENERATED_BODY()

public:
	AAICharacter();

protected:
	
	virtual void BeginPlay() override;

public:


	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetMovementSpeed(float NewMovementSpeed);

	TArray<ACustomTargetPoint*> GetPatrolPoints() { return PatrolPoints; }

	bool IsPatrol() { return bIsPatrol; }

protected:

	virtual void StartAimDownSights() override;

	virtual void StopAimDownSights() override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AI")
	void LookAtRotationOverTime(FRotator RotToLookAt);

	virtual void Die() override;

protected:

	////////////////////////////////////////////////////////////////////////////////
	// Patrolling

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float MovementSpeedPatrolling = 180;

	/** Does this guard patrol between 2 or more points */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	bool bIsPatrol = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	TArray<ACustomTargetPoint*> PatrolPoints;

	////////////////////////////////////////////////////////////////////////////////




};
