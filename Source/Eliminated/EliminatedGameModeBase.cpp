// Copyright Epic Games, Inc. All Rights Reserved.


#include "EliminatedGameModeBase.h"

AEliminatedGameModeBase::AEliminatedGameModeBase()
{

}

int32 AEliminatedGameModeBase::GetNumberOfEnemies()
{
	return CurrentNumberOfEnemies;
}

void AEliminatedGameModeBase::AddEnemyNPC(AActor* AddedNPC)
{
	MaxNumOfEnemies++;
	CurrentNumberOfEnemies++;
}

void AEliminatedGameModeBase::EnemyNPCDied(AActor* DeadEnemy)
{
	CurrentNumberOfEnemies--;
	OnEnemyDied.Broadcast(DeadEnemy, CurrentNumberOfEnemies);
}
