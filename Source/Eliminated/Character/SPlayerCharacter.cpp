// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerCharacter.h"
#include "Blueprint\AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include "Eliminated\Character\AICharacter.h"
#include "EngineUtils.h"
#include "AIModule\Classes\BrainComponent.h"
#include "GameFramework\CharacterMovementComponent.h"


void ASPlayerCharacter::Die()
{
	Super::Die();

	for (TActorIterator<AAICharacter> AICharItr(GetWorld()); AICharItr; ++AICharItr)
	{
		AAICharacter* AIChar = *AICharItr;
		AIChar->StopFire();

		AAIController* AIController = UAIBlueprintHelperLibrary::GetAIController(AIChar);
		AIController->GetBrainComponent()->StopLogic("Player Is Dead");
	}


}

void ASPlayerCharacter::OnEnemyDied(AActor* DeadEnemy, int32 RemainingEnemies)
{
	Super::OnEnemyDied(DeadEnemy, RemainingEnemies);

	if (RemainingEnemies <= 0)
	{
		//StopAimDownSights();
		StopFire();

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}
	}
}

bool ASPlayerCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor /*= NULL*/) const
{
	static const FName NAME_AILineOfSight = FName(TEXT("TestPawnLineOfSight")); // IDK

	FHitResult HitResult;

	// Doing checks for all the socket locations
	for (int i = 0; i< SightSocketNames.Num(); i++)
	{
		FVector SocketLocation = GetMesh()->GetSocketLocation(SightSocketNames[i]);

		// Line trace from ObserverLocation to SocketLocation
		bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, SocketLocation,
			FCollisionObjectQueryParams(ECC_WorldStatic | ECC_WorldDynamic),
			FCollisionQueryParams(NAME_AILineOfSight, true, IgnoreActor));

		NumberOfLoSChecksPerformed++;

		// Nothing between the AIChar and tested location, or this actor is me
		if (bHit == false || (HitResult.Actor.IsValid() && HitResult.Actor->IsOwnedBy(this)))
		{
			OutSightStrength = 1;
			OutSeenLocation = SocketLocation;

			return true;

		}
	}

	///////////////////////////////////////
	// Doing a check to the actor location
	bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, GetActorLocation(),
		FCollisionObjectQueryParams(ECC_WorldStatic | ECC_WorldDynamic),
		FCollisionQueryParams(NAME_AILineOfSight, true, IgnoreActor));

	NumberOfLoSChecksPerformed++;

	// Nothing between the AIChar and tested location, or this actor is me
	if (bHit == false || (HitResult.Actor.IsValid() && HitResult.Actor->IsOwnedBy(this)))
	{
		OutSightStrength = 1;
		OutSeenLocation = GetActorLocation();

		return true;

	}
	///////////////////////////////////////


	// Something between AIChar and the tested locations, so it cannot see the player, so return false
	OutSightStrength = 0;
	return false;
}
