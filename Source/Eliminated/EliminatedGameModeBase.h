// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EliminatedGameModeBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyDied, AActor*, DeadEnemy, int32, RemainingEnemies);
/**
 * 
 */
UCLASS()
class ELIMINATED_API AEliminatedGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AEliminatedGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	int32 GetNumberOfEnemies();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void AddEnemyNPC(AActor* AddedNPC);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void EnemyNPCDied(AActor* DeadEnemy);

	UPROPERTY(BlueprintAssignable)
	FOnEnemyDied OnEnemyDied;

	int32 GetMaxNumOfEnemies() { return MaxNumOfEnemies; }
	int32 GetCurrentNumberOfEnemies() { return CurrentNumberOfEnemies; }

protected:

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "GameMode")
	int32 MaxNumOfEnemies;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "GameMode")
	int32 CurrentNumberOfEnemies;


};
