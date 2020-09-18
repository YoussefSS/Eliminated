// Fill out your copyright notice in the Description page of Project Settings.


#include "SAIController.h"
#include "Eliminated\Character\AICharacter.h"
#include "Perception\AIPerceptionComponent.h"
#include "Perception\AISenseConfig_Sight.h"
#include "Perception\AISenseConfig_Hearing.h"
#include "Perception\AISenseConfig_Damage.h"
#include "Kismet\KismetMathLibrary.h"
#include "Eliminated\Character\SPlayerCharacter.h"
#include "Blueprint\AIBlueprintHelperLibrary.h"
#include "BehaviorTree\BlackboardComponent.h"
#include "Eliminated\AI\CustomTargetPoint.h"
#include "BrainComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ASAIController::ASAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 3200;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->bUseLoSHearing = true;
	HearingConfig->LoSHearingRange = 3700;

	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));


	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*DamageConfig);
	AIPerceptionComp->SetDominantSense(DamageConfig->GetSenseImplementation());

	SetPerceptionComponent(*AIPerceptionComp);

}

void ASAIController::BeginPlay()
{
	Super::BeginPlay();

	SetAIStatus(EAIStatus::EAS_Normal);
	if (AIPerceptionComp)
	{
		//AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ASAIController::OnPerceptionUpdated);
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ASAIController::OnTargetPerceptionUpdated);
	}
}

void ASAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCanSeePlayer && PlayerReference && PlayerReference->IsValidLowLevel())
	{
		LastKnownPlayerLocation = PlayerReference->GetActorLocation();
	}
}

void ASAIController::OnTargetPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	// ELSE ONLY works on sight
	// NOTE that the else parts won't be called as the MaxAge is set to 0, which means never. 

	if (GetAIStatus() == EAIStatus::EAS_Dead) return;
	if (Actor == GetPawn()) return; // Don't do logic if I did triggered this function

	ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(Actor);

	/*************/
	/** HEARING **/
	/*************/
	// We want to hear all sounds, even weapon shots coming from allies
	if (UKismetMathLibrary::ClassIsChildOf(UAISense_Hearing::StaticClass(), UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus)) && (GetAIStatus() != EAIStatus::EAS_Aggroed))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// We want to check if the sound is made by the player, if it is, go to it, if not, don't keep switching back and forth between the player and AI

			if (GetAIStatus() == EAIStatus::EAS_Ivestigating) // If already investigating
			{
				if (PlayerChar) // If the sound was made by the player
				{
					bInvestigatingSightOrSoundMadeByPlayer = true;
					InvestigateLocation(Stimulus.StimulusLocation, 2.5);
				}
				else
				{
					if (!bInvestigatingSightOrSoundMadeByPlayer) // If not investigating a sound made by the player, go to the new sound
					{
						InvestigateLocation(Stimulus.StimulusLocation, 2.5);
					}
					// Do nothing, keep going towards the players last made sound
				}
			}
			else
			{
				InvestigateLocation(Stimulus.StimulusLocation, 2.5);
				if (PlayerChar)
				{
					bInvestigatingSightOrSoundMadeByPlayer = true;
				}
			}

			
		}

		return;
	}


	// No Need to do seeing or damage logic if not playercharacter
	
	if (!PlayerChar) return;
	PlayerReference = PlayerChar;

	GetWorldTimerManager().ClearTimer(StopAggroing_Timer);
	

	/************/
	/** SEEING **/
	/************/
	if (UKismetMathLibrary::ClassIsChildOf(UAISense_Sight::StaticClass(), UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus)))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			bInvestigatingSightOrSoundMadeByPlayer = true;
			bCanSeePlayer = true;

			// Investigate first
			InvestigateLocation(Stimulus.StimulusLocation, 2.5);

			// Then Aggro
			if (!GetWorldTimerManager().IsTimerActive(StartAggroing_Timer)) // If start aggroing timer is active, meaning it has been used by DamageSense, so don't re do it
			{
				// Time to start aggroing, based on the location of the player to AI
				float MaxAggroAfterTime = 3;
				if (SightConfig && GetPawn())
				{
					MaxAggroAfterTime *= FMath::Clamp(FVector::Distance(GetPawn()->GetActorLocation(), PlayerChar->GetActorLocation())  / (SightConfig->SightRadius),
						0.f, MaxAggroAfterTime); // Location of player to AI divided by sight radius
				}

				FTimerDelegate AggroDelegate;
				AggroDelegate.BindUObject(this, &ASAIController::TryStartAggroing, Actor, Stimulus);
				GetWorldTimerManager().SetTimer(StartAggroing_Timer, AggroDelegate, MaxAggroAfterTime, false);
			}


		}
		else
		{
			bCanSeePlayer = false;

			if (GetAIStatus() == EAIStatus::EAS_Aggroed)
			{
				// Stop aggro
				GetWorldTimerManager().SetTimer(StopAggroing_Timer, this, &ASAIController::StopAggroing, TimeToStopAggroing);

				// Then investigate the last seen location
				FTimerDelegate InvestigateDelegate;
				InvestigateDelegate.BindUObject(this, &ASAIController::InvestigateLocation, LastKnownPlayerLocation, 1.f);
				GetWorldTimerManager().SetTimer(StartInvestigating_Timer, InvestigateDelegate, TimeToStopAggroing+0.01, false);
			}
			

		}

		return;
	}


	// If aggroed, return as there is nothing to do ..
	// .. IMPORTANT, if going to handle else in hearing or dmg logic, this needs to be changed
	if (GetAIStatus() == EAIStatus::EAS_Aggroed) return;


	/************/
	/** DAMAGE **/
	/************/
	if (UKismetMathLibrary::ClassIsChildOf(UAISense_Damage::StaticClass(), UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus)))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			bInvestigatingSightOrSoundMadeByPlayer = true;

			// Investigate first
			InvestigateLocation(Stimulus.StimulusLocation, 1);

			// Then Aggro
			FTimerDelegate AggroDelegate;
			AggroDelegate.BindUObject(this, &ASAIController::TryStartAggroing, Actor, Stimulus);
			GetWorldTimerManager().SetTimer(StartAggroing_Timer, AggroDelegate, 0.6, false);
		}

		return;
	}

}


void ASAIController::TryStartAggroing(AActor* ActorToAggroOn, FAIStimulus Stimulus)
{
	if (bCanSeePlayer)
	{
		GetWorldTimerManager().ClearTimer(StopAggroing_Timer);

		AggroOnActor(ActorToAggroOn);
	}
	/*else
	{
		InvestigateLocation(LastKnownPlayerLocation);
	}*/
	

}

void ASAIController::StopAggroing()
{
	SetAIStatus(EAIStatus::EAS_Normal);

	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsAggroed, false);
	BB->SetValueAsObject(BBKey_TargetActor, nullptr);

	bInvestigatingSightOrSoundMadeByPlayer = false;
}

void ASAIController::AggroOnActor(AActor* ActorToAggroOn)
{
	StopInvestigating(); // Stop investigating anything, and aggro. Without this, when stopping aggroing after a while the AI will investigate something old

	SetAIStatus(EAIStatus::EAS_Aggroed);

	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsAggroed, true);
	BB->SetValueAsObject(BBKey_TargetActor, ActorToAggroOn);


}

void ASAIController::InvestigateLocation(FVector DestinationToInvestigate, float InvestigateAfterTime)
{
	StopAggroing();
	SetAIStatus(EAIStatus::EAS_Ivestigating);

	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsInvestigating, true);
	BB->SetValueAsVector(BBKey_TargetDestination, DestinationToInvestigate);
	BB->SetValueAsFloat(BBKey_TimeToWaitBeforeInvestigating, InvestigateAfterTime);
}

void ASAIController::SetAIStatus(EAIStatus NewAIStatus)
{
	AIStatus = NewAIStatus;

	OnAIStatusChanged.Broadcast(NewAIStatus, this);
}

ACustomTargetPoint* ASAIController::GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime)
{
	CurrentTargetPointIndex++;

	TArray<ACustomTargetPoint*> PatrolPoints;
	AAICharacter* OwnerAI = Cast<AAICharacter>(GetPawn());
	if (OwnerAI)
	{
		PatrolPoints = OwnerAI->GetPatrolPoints();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ASAIController::GetNextTargetPoint: GetPawn is nullptr"));
		return nullptr;
	}

	if (CurrentTargetPointIndex >= PatrolPoints.Num())
	{
		CurrentTargetPointIndex = 0;
	}

	if (PatrolPoints[CurrentTargetPointIndex])
	{
		OutLocation = PatrolPoints[CurrentTargetPointIndex]->GetActorLocation();
		OutWaitTime = PatrolPoints[CurrentTargetPointIndex]->GetSecondsToStay();
	}
	else
	{
		OutLocation = FVector::ZeroVector;
		OutWaitTime = 0.f;
	}

	return PatrolPoints[CurrentTargetPointIndex];
}

void ASAIController::StopInvestigating()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsInvestigating, false);

	if (GetAIStatus() != EAIStatus::EAS_Aggroed)
	{
		SetAIStatus(EAIStatus::EAS_Normal);
	}

	bInvestigatingSightOrSoundMadeByPlayer = false;
}

void ASAIController::SetIsPatrolGuardBBValue()
{
	bool bIsPatrol = false;
	AAICharacter* OwnerAI = Cast<AAICharacter>(GetPawn());
	if (OwnerAI)
	{
		bIsPatrol = OwnerAI->IsPatrol();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ASAIController::SetIsPatrolGuardBBValue: GetPawn is nullptr"));
		return;
	}

	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsPatrolGuard, bIsPatrol);
}

void ASAIController::GetNextPatrolPointAndSetBBValues()
{
	bool bIsPatrol = false;
	AAICharacter* OwnerAI = Cast<AAICharacter>(GetPawn());
	if (OwnerAI)
	{
		bIsPatrol = OwnerAI->IsPatrol();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ASAIController::GetNextPatrolPointAndSetBBValues: GetOwner is nullptr"));
		return;
	}

	if (bIsPatrol)
	{
		FVector OutLocation;
		float OutWaitTime;
		GetNextTargetPoint(OutLocation, OutWaitTime);

		UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
		BB->SetValueAsVector(BBKey_PatrolPointDestination, OutLocation);
		BB->SetValueAsFloat(BBKey_TimeToWaitAtPatrolPoint, OutWaitTime);
	}
}

void ASAIController::SetOriginalLocationAndRotationBBValues()
{
	if (!GetPawn())
	{
		UE_LOG(LogTemp, Error, TEXT("ASAIController::SetOriginalLocationAndRotationBBValues: GetOwner is nullptr"));
		return;
	}

	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsVector(BBKey_OriginalLocation, GetPawn()->GetActorLocation());
	BB->SetValueAsRotator(BBKey_OriginalRotation, GetPawn()->GetActorRotation());
}

void ASAIController::OnDeath()
{
	SetAIStatus(EAIStatus::EAS_Dead);
	GetBrainComponent()->StopLogic("AI Died");
	if (AIPerceptionComp)
	{
		AIPerceptionComp->UnregisterComponent();
		AIPerceptionComp->DestroyComponent();
	}

	GetWorldTimerManager().ClearAllTimersForObject(this);

	OnUnPossess();
}

/*
void ASAIController::OnPerceptionUpdated_Implementation(const TArray<AActor*>& UpdatedActors)
{
	for (int i = 0; i < UpdatedActors.Num(); i++)
	{
		AActor* UpdatedActor = UpdatedActors[i]; // Getting the actor to check
		FActorPerceptionBlueprintInfo PerceptionInfo;
		AIPerceptionComp->GetActorsPerception(UpdatedActor, PerceptionInfo); // Getting details about the PerceptionInfo

		AActor* TargetActor = PerceptionInfo.Target;
		TArray<FAIStimulus> LastSensedStimuli = PerceptionInfo.LastSensedStimuli;
		bool bIsHostile = PerceptionInfo.bIsHostile;





		for (int StimIndex = 0; StimIndex < LastSensedStimuli.Num(); StimIndex++)
		{

			if (StimIndex == 0)
			{
				// Sight sense //
				if (LastSensedStimuli[0].WasSuccessfullySensed())
				{
					ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(TargetActor);
					if (PlayerChar)
					{
						UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
						BB->SetValueAsBool(BBKey_IsAggroed, LastSensedStimuli[StimIndex].WasSuccessfullySensed());
						BB->SetValueAsObject(BBKey_TargetActor, PlayerChar);

						SetAIStatus(EAIStatus::EAS_Aggroed);
						GetWorldTimerManager().ClearTimer(StopAggroing_Timer);

					}
				}
				else // Just stopped sensing NOTE THAT THIS IS CALLED EVEN IF ANOTHER SENSE HAS SENSED
				{
					GetWorldTimerManager().SetTimer(StopAggroing_Timer, this, &ASAIController::StopAggroing, TimeToStopAggroing);
					//UE_LOG(LogTemp, Warning, TEXT("NOT SENESED"));
				}


			} // END if sight sense

			else if (StimIndex == 1)
			{
				// Hearing sense //
				if (LastSensedStimuli[1].WasSuccessfullySensed()) // Just started sensing
				{
					UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
					BB->SetValueAsBool(BBKey_IsInvestigating, LastSensedStimuli[StimIndex].WasSuccessfullySensed());
					BB->SetValueAsVector(BBKey_TargetDestination, LastSensedStimuli[StimIndex].StimulusLocation);

					SetAIStatus(EAIStatus::EAS_Ivestigating);
				}

			} // END if hearing sense

			else if (StimIndex == 2)
			{
				// Dmg sense sense //
				if (LastSensedStimuli[2].WasSuccessfullySensed())
				{
					ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(TargetActor);
					if (PlayerChar)
					{
						UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
						BB->SetValueAsBool(BBKey_IsAggroed, LastSensedStimuli[StimIndex].WasSuccessfullySensed());
						BB->SetValueAsObject(BBKey_TargetActor, PlayerChar);


						SetAIStatus(EAIStatus::EAS_Aggroed);
						GetWorldTimerManager().ClearTimer(StopAggroing_Timer);
					}
				}
				else // Just stopped sensing NOTE THAT THIS IS CALLED EVEN IF ANOTHER SENSE HAS SENSED
				{
					GetWorldTimerManager().SetTimer(StopAggroing_Timer, this, &ASAIController::StopAggroing, TimeToStopAggroing);
					//UE_LOG(LogTemp, Warning, TEXT("NOT SENESED"));
				}

			} // END if damage sense

		}

	}
}
*/