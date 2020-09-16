// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Perception\AIPerceptionTypes.h"

#include "SAIController.generated.h"

UENUM(BlueprintType) 
enum class EAIStatus : uint8 
{
	EAS_Normal				UMETA(DisplayName = "Normal"),
	EAS_Ivestigating		UMETA(DisplayName = "Ivestigating"),
	EAS_Aggroed				UMETA(DisplayName = "Aggroed"),
	EAS_Dead				UMETA(DisplayName = "Dead"),

	EMS_MAX        UMETA(DisplayName = "DefaultMAX") 
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIStatusChanged, EAIStatus, NewStatus, ASAIController*, AffectedController);


/**
 * 
 */
class ASPlayerCharacter;
UCLASS()
class ELIMINATED_API ASAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ASAIController();


protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI| Perception")
	class UAISenseConfig_Damage* DamageConfig;


protected:

	//UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	//void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION(BlueprintNativeEvent, Category = "AI| Perception")
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void TryStartAggroing(AActor* ActorToAggroOn, FAIStimulus Stimulus);

	UFUNCTION()
	void StopAggroing();

	void AggroOnActor(AActor* ActorToAggroOn);

	/** First look at the location, then wait @param: InvestigateAfterTime, then go to the location */
	void InvestigateLocation(FVector DestinationToInvestigate, float InvestigateAfterTime);

	//UFUNCTION()
	//void StopInvestigating();

	void SetAIStatus(EAIStatus NewAIStatus);

	EAIStatus GetAIStatus() { return AIStatus; }

	

public:

	UFUNCTION(BlueprintCallable, Category = "AI")
	class ACustomTargetPoint* GetNextTargetPoint(FVector& OutLocation, float& OutWaitTime);

	/** Immediately stop investigation */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StopInvestigating();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetIsPatrolGuardBBValue();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void GetNextPatrolPointAndSetBBValues();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetOriginalLocationAndRotationBBValues();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void OnDeath();

	FOnAIStatusChanged OnAIStatusChanged;

protected:

	////////////////////////////////////////////////////////////////////////////////
	// Blackboard Keys

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_IsPatrolGuard = "IsPatrolGuard";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_PatrolPointDestination = "PatrolPointDestination";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_TimeToWaitAtPatrolPoint = "TimeToWaitAtPatrolPoint";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_TargetDestination = "TargetDestination";

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI| BlackBoardValues")
	FName BBKey_TimeToWaitBeforeInvestigating = "TimeToWaitBeforeInvestigating";

	// END Blackboard Keys
	////////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float TimeToStartAggroing = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	float TimeToStopAggroing = 3.f;


protected:

	////////////////////////////////////////////////////////////////////////////////
	// State

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI| State")
	EAIStatus AIStatus = EAIStatus::EAS_Normal;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI| State")
	bool bCanSeePlayer = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI| State")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	ASPlayerCharacter* PlayerReference;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI| State")
	FRotator OriginalRotation;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "AI| State")
	int32 CurrentTargetPointIndex = -1;

	FTimerHandle StartAggroing_Timer;

	FTimerHandle StopAggroing_Timer;

	FTimerHandle StartInvestigating_Timer;

	//FTimerHandle StopInvestigating_Timer;
};
