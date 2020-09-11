// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Eliminated\Character\SCharacterBase.h"
#include "SPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ELIMINATED_API ASPlayerCharacter : public ASCharacterBase
{
	GENERATED_BODY()
	

protected:

	virtual void Die() override;

	virtual void OnEnemyDied(AActor* DeadEnemy, int32 RemainingEnemies) override;
};
