// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "Eliminated\AI\CustomTargetPoint.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera\CameraComponent.h"
#include "Perception\AIPerceptionComponent.h"
#include "Perception\AISenseConfig_Sight.h"
#include "Perception\AISenseConfig_Hearing.h"
#include "Perception\AISenseConfig_Damage.h"
#include "BehaviorTree\BehaviorTreeComponent.h"
#include "BehaviorTree\BlackboardComponent.h"
#include "Blueprint\AIBlueprintHelperLibrary.h"
#include "Eliminated\Character\SPlayerCharacter.h"
#include "Kismet\GameplayStatics.h"
#include "Eliminated\EliminatedGameModeBase.h"

AAICharacter::AAICharacter()
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
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

	/////////////////////////////////////////////
	CameraBoom->TargetArmLength = 0.f;

	SpringArmDistance_Regular = 0;
	SpringArmDistance_AimDownSight = 0;
	
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeedPatrolling;
	}

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AAICharacter::OnPerceptionUpdated);
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAICharacter::OnTargetPerceptionUpdated);
	}

	// Let the game mode know you were spawned
	AEliminatedGameModeBase* GM = Cast<AEliminatedGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->AddEnemyNPC(this);
	}
}


void AAICharacter::SetMovementSpeed(float NewMovementSpeed)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NewMovementSpeed;
	}
}

ACustomTargetPoint* AAICharacter::GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime)
{
	CurrentTargetPointIndex++;

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

void AAICharacter::StopInvestigatingSound()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsInvestigating, false);
}

void AAICharacter::SetIsPatrolGuardBBValue()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsPatrolGuard, bIsPatrol);
}

void AAICharacter::GetNextPatrolPointAndSetBBValues()
{
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

void AAICharacter::SetOriginalLocationAndRotationBBValues()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsVector(BBKey_OriginalLocation, GetActorLocation());
	BB->SetValueAsRotator(BBKey_OriginalRotation, GetActorRotation());
}

void AAICharacter::StartAimDownSights()
{
	Super::StartAimDownSights();

	bUseControllerRotationYaw = false;
	
	CameraBoom->bUsePawnControlRotation = false;
	Camera->bUsePawnControlRotation = true;
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		//GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0, 100, 0);
	}
}

void AAICharacter::StopAimDownSights()
{
	Super::StopAimDownSights();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,
			2, //Duration
			FColor::Cyan,
			FString::Printf(TEXT("                     Stopped aimind down sights")),
			true,
			FVector2D(3, 3)
		);
	}
}

void AAICharacter::StopAggroing()
{
	UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
	BB->SetValueAsBool(BBKey_IsAggroed, false);
	BB->SetValueAsObject(BBKey_TargetActor, nullptr);
}



void AAICharacter::Die()
{
	Super::Die();

	StopFire();

	// Let the game mode know you died
	AEliminatedGameModeBase* GM = Cast<AEliminatedGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->EnemyNPCDied(this);
	}
}

void AAICharacter::OnTargetPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{

}

void AAICharacter::OnPerceptionUpdated_Implementation(const TArray<AActor*>& UpdatedActors)
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
				ASPlayerCharacter* PlayerChar = Cast<ASPlayerCharacter>(TargetActor);
				if (PlayerChar)
				{
					if (LastSensedStimuli[StimIndex].WasSuccessfullySensed()) // Just started sensing
					{
						UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
						BB->SetValueAsBool(BBKey_IsAggroed, LastSensedStimuli[StimIndex].WasSuccessfullySensed());
						BB->SetValueAsObject(BBKey_TargetActor, PlayerChar);

						GetWorldTimerManager().ClearTimer(StopAggroing_Timer);
					}
					else // Just stopped sensing NOTE THAT THIS IS CALLED EVEN IF ANOTHER SENSE HAS SENSED
					{
						GetWorldTimerManager().SetTimer(StopAggroing_Timer, this, &AAICharacter::StopAggroing, TimeToStopAggroing);
						UE_LOG(LogTemp, Warning, TEXT("NOT SENESED"));
					}
					
				}

			}
			else if (StimIndex == 1)
			{
				// Hearing sense //
				if (LastSensedStimuli[StimIndex].WasSuccessfullySensed()) // Just started sensing
				{
					UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
					BB->SetValueAsBool(BBKey_IsInvestigating, LastSensedStimuli[StimIndex].WasSuccessfullySensed());
					BB->SetValueAsVector(BBKey_TargetDestination, LastSensedStimuli[StimIndex].StimulusLocation);
				}

			}
			else if (StimIndex == 2)
			{
				// Damage sense //


			}
		}
	}
}
