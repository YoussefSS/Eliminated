// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerCharacter.h"
#include "Eliminated\Character\SAIController.h"
#include "Eliminated\Character\AICharacter.h"
#include "EngineUtils.h"
#include "AIModule\Classes\BrainComponent.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Perception\AIPerceptionStimuliSourceComponent.h"
#include "Perception\AISense_Damage.h"

#include "Perception\AISenseConfig_Damage.h"
#include "Perception\AIPerceptionSystem.h"

ASPlayerCharacter::ASPlayerCharacter()
{

}

void ASPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ASPlayerCharacter::Die()
{
	Super::Die();

	for (TActorIterator<AAICharacter> AICharItr(GetWorld()); AICharItr; ++AICharItr)
	{
		AAICharacter* AIChar = *AICharItr;
		AIChar->StopFire();

		ASAIController* AIController = Cast<ASAIController>(AIChar->GetController());
		if (AIController)
		{
			AIController->GetBrainComponent()->StopLogic("Player Is Dead");
		}
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

bool ASPlayerCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor /*= NULL*/, const bool* bWasVisible /*= nullptr*/, int32* UserData /*= nullptr*/) const
{
	static const FName NAME_AILineOfSight = FName(TEXT("TestPawnLineOfSight")); // IDK

	FHitResult HitResult;
	bool bHit;

	///////////////////////////////////////
	// Doing a check to the actor location first
	bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, GetActorLocation(),
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
	// Doing checks for all the socket locations
	for (int i = 0; i< SightSocketNames.Num(); i++)
	{
		FVector SocketLocation = GetMesh()->GetSocketLocation(SightSocketNames[i]);

		// Line trace from ObserverLocation to SocketLocation
		bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, SocketLocation,
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
	// Something between AIChar and the tested locations, so it cannot see the player, so return false
	OutSightStrength = 0;
	return false;
}
