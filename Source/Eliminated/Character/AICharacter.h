// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Eliminated/Character/PlayerCharacter.h"

#include "Perception\AIPerceptionTypes.h"

#include "AICharacter.generated.h"

/**
 * 
 */
class ACustomTargetPoint;
class UAIPerceptionComponent;
UCLASS()
class ELIMINATED_API AAICharacter : public APlayerCharacter
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float MovementSpeedPatrolling = 180;

	/** Does this guard patrol between 2 or more points */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	bool bIsPatrol = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	TArray<ACustomTargetPoint*> PatrolPoints;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI")
	int32 CurrentTargetPointIndex = -1;

	UFUNCTION(BlueprintCallable, Category = "AI")
	ACustomTargetPoint* GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime);

protected:

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);


};
