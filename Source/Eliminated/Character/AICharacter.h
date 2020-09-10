// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Eliminated/Character/SCharacterBase.h"

#include "Perception\AIPerceptionTypes.h"

#include "AICharacter.generated.h"

/**
 * 
 */
class ACustomTargetPoint;
class UAIPerceptionComponent;
UCLASS()
class ELIMINATED_API AAICharacter : public ASCharacterBase
{
	GENERATED_BODY()

public:
	AAICharacter();

protected:
	
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Damage* DamageConfig;


public:


	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetMovementSpeed(float NewMovementSpeed);

	UFUNCTION(BlueprintCallable, Category = "AI")
	ACustomTargetPoint* GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime);


	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopInvestigatingSound();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetIsPatrolGuardBBValue();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void GetNextPatrolPointAndSetBBValues();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetOriginalLocationAndRotationBBValues();

protected:

	virtual void StartAimDownSights() override;

	virtual void StopAimDownSights() override;

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AI")
	void LookAtRotationOverTime(FRotator RotToLookAt);

	UFUNCTION()
	void StopAggroing();

	virtual void Die() override;

protected:

	////////////////////////////////////////////////////////////////////////////////
	// Patrolling

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float MovementSpeedPatrolling = 180;

	/** Does this guard patrol between 2 or more points */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	bool bIsPatrol = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	TArray<ACustomTargetPoint*> PatrolPoints;

	////////////////////////////////////////////////////////////////////////////////
	// Other

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float TimeToStopAggroing = 3.f;


	////////////////////////////////////////////////////////////////////////////////
	// Blackboard Keys

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_IsPatrolGuard = "IsPatrolGuard";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_TargetDestination = "TargetDestination";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_TimeToWaitAtPatrolPoint = "TimeToWaitAtPatrolPoint";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_OriginalLocation = "OriginalLocation";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_OriginalRotation = "OriginalRotation";


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_IsAggroed = "IsAggroed";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_TargetActor = "TargetActor";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlackBoardValues")
	FName BBKey_IsInvestigating = "IsInvestigating";

	////////////////////////////////////////////////////////////////////////////////


protected:

	////////////////////////////////////////////////////////////////////////////////
	// State

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI")
	FRotator OriginalRotation;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI")
	int32 CurrentTargetPointIndex = -1;

	FTimerHandle StopAggroing_Timer;
};
