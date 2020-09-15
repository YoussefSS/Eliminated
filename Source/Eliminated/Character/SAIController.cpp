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
		AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ASAIController::OnPerceptionUpdated);
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ASAIController::OnTargetPerceptionUpdated);
	}
}


void ASAIController::OnTargetPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{
	// ELSE ONLY works on sight
	// NOTE that the else parts won't be called as the MaxAge is set to 0, which means never. 

	// SIGHT
	if (UKismetMathLibrary::ClassIsChildOf(UAISense_Sight::StaticClass(), UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus)))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(Actor);
			if (PlayerChar)
			{
				UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
				BB->SetValueAsBool(BBKey_IsAggroed, true);
				BB->SetValueAsObject(BBKey_TargetActor, PlayerChar);

				SetAIStatus(EAIStatus::EAS_Aggroed);
				GetWorldTimerManager().ClearTimer(StopAggroing_Timer);
			}
		}
		else
		{
			// TODO StopAggro
			//GetWorldTimerManager().SetTimer(StopAggroing_Timer, this, &ASAIController::StopAggroing, TimeToStopAggroing);
			ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(Actor);
			if (PlayerChar)
			{
				UE_LOG(LogTemp, Warning, TEXT("stopped seeing lol how"));
			}
			
		}
	}

	// IF AGGROED, RETURN
	if (GetAIStatus() == EAIStatus::EAS_Aggroed) return;

	// HEARING
	if (UKismetMathLibrary::ClassIsChildOf(UAISense_Hearing::StaticClass(), UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus)))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
			BB->SetValueAsBool(BBKey_IsInvestigating, true);
			BB->SetValueAsVector(BBKey_TargetDestination, Stimulus.StimulusLocation);

			SetAIStatus(EAIStatus::EAS_Ivestigating);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("stopped hearing lol how"));
		}
	}


	// DAMAGE
	if (UKismetMathLibrary::ClassIsChildOf(UAISense_Damage::StaticClass(), UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus)))
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(Actor);
			if (PlayerChar)
			{
				UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
				BB->SetValueAsBool(BBKey_IsAggroed, true);
				BB->SetValueAsObject(BBKey_TargetActor, PlayerChar);


				SetAIStatus(EAIStatus::EAS_Aggroed);
				GetWorldTimerManager().ClearTimer(StopAggroing_Timer);
			}
		}
	}
}

void ASAIController::OnPerceptionUpdated_Implementation(const TArray<AActor*>& UpdatedActors)
{
	/* for (int i = 0; i < UpdatedActors.Num(); i++)
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
	*/
}

void ASAIController::StopAggroing()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsAggroed, false);
	BB->SetValueAsObject(BBKey_TargetActor, nullptr);

	SetAIStatus(EAIStatus::EAS_Normal);
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

void ASAIController::StopInvestigatingSound()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsInvestigating, false);
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
		UE_LOG(LogTemp, Error, TEXT("ASAIController::SetIsPatrolGuardBBValue: GetOwner is nullptr"));
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
		BB->SetValueAsVector(BBKey_TargetDestination, OutLocation);
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

	OnUnPossess();
}
