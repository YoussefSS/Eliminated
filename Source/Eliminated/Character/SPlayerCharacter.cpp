// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerCharacter.h"
#include "Blueprint\AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include "Eliminated\Character\AICharacter.h"
#include "EngineUtils.h"
#include "AIModule\Classes\BrainComponent.h"

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
