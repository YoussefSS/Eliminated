// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Eliminated\Character\SCharacterBase.h"
#include "Perception\AISightTargetInterface.h"
#include "SPlayerCharacter.generated.h"


/**
 * 
 */
UCLASS()
class ELIMINATED_API ASPlayerCharacter : public ASCharacterBase, public IAISightTargetInterface
{
	GENERATED_BODY()
	

protected:

	virtual void Die() override;

	virtual void OnEnemyDied(AActor* DeadEnemy, int32 RemainingEnemies) override;

public:

	/** Overriding the CanBeSeeFrom function to specify locations on the characters body that the AI can see */
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = NULL) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ForAI")
	TArray<FName> SightSocketNames;
};
