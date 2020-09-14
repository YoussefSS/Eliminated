// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"

#include "GameFramework\CharacterMovementComponent.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera\CameraComponent.h"
#include "Blueprint\AIBlueprintHelperLibrary.h"
#include "Kismet\GameplayStatics.h"
#include "Eliminated\EliminatedGameModeBase.h"
#include "Eliminated\Character\SAIController.h"


AAICharacter::AAICharacter()
{
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
			FVector2D(1, 1)
		);
	}
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

	ASAIController* AIController = Cast<ASAIController>(GetController());
	if (AIController)
	{
		AIController->OnDeath();
	}
}
