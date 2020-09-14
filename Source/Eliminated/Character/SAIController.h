// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Perception\AIPerceptionTypes.h"

#include "SAIController.generated.h"

/**
 * 
 */
UCLASS()
class ELIMINATED_API ASAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ASAIController();


protected:
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Damage* DamageConfig;


protected:

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void StopAggroing();

public:

	UFUNCTION(BlueprintCallable, Category = "AI")
	class ACustomTargetPoint* GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopInvestigatingSound();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetIsPatrolGuardBBValue();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void GetNextPatrolPointAndSetBBValues();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetOriginalLocationAndRotationBBValues();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void OnDeath();

protected:

	////////////////////////////////////////////////////////////////////////////////
	// Blackboard Keys

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_IsPatrolGuard = "IsPatrolGuard";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_TargetDestination = "TargetDestination";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_TimeToWaitAtPatrolPoint = "TimeToWaitAtPatrolPoint";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_OriginalLocation = "OriginalLocation";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_OriginalRotation = "OriginalRotation";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_IsAggroed = "IsAggroed";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_TargetActor = "TargetActor";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_IsInvestigating = "IsInvestigating";

	// END Blackboard Keys
	////////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float TimeToStopAggroing = 3.f;


protected:

	////////////////////////////////////////////////////////////////////////////////
	// State

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI")
	FRotator OriginalRotation;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI")
	int32 CurrentTargetPointIndex = -1;

	FTimerHandle StopAggroing_Timer;
};
