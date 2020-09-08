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

AAICharacter::AAICharacter()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

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

void AAICharacter::OnTargetPerceptionUpdated_Implementation(AActor* Actor, FAIStimulus Stimulus)
{

}

void AAICharacter::OnPerceptionUpdated_Implementation(const TArray<AActor*>& UpdatedActors)
{
	for (int i = 0; i < UpdatedActors.Num(); i++)
	{
		AActor* UpdatedActor = UpdatedActors[i];
		FActorPerceptionBlueprintInfo PerceptionInfo;
		AIPerceptionComp->GetActorsPerception(UpdatedActor, PerceptionInfo);

		AActor* TargetActor = PerceptionInfo.Target;
		TArray<FAIStimulus> LastSensedStimuli = PerceptionInfo.LastSensedStimuli;
		bool bIsHostile = PerceptionInfo.bIsHostile;

		for (int StimIndex = 0; StimIndex < LastSensedStimuli.Num(); StimIndex++)
		{
			if (StimIndex == 0)
			{
				// Sight sense
				ASCharacterBase* PlayerChar = Cast<ASCharacterBase>(TargetActor);
				if (PlayerChar)
				{
					UBlackboardComponent* BB = UAIBlueprintHelperLibrary::GetBlackboard(this);
					BB->SetValueAsBool(BBKey_IsAggroed, LastSensedStimuli[StimIndex].WasSuccessfullySensed());
					BB->SetValueAsObject(BBKey_TargetActor, PlayerChar);
				}

			}
			else if (StimIndex == 1)
			{
				// Hearing sense


			}
			else if (StimIndex == 2)
			{
				// Damage sense


			}
		}
	}
}
