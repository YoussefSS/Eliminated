// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class ELIMINATED_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	class USpringArmComponent* CameraArm;
	class UCameraComponent* Camera;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	void MousePitchInput(float Val);
	void MouseYawInput(float Val);

	void MoveForward(float Val);
	float MoveForwardAxisVal = 0;
	void MoveRight(float Val);
	float MoveRightAxisVal = 0;


	virtual void Jump() override;

	void StartSprint();
	void StopSprint();
	bool bIsSprinting;

	void UpdateRotationRate();
	void UpdateMovementAxisInput();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | No Weapon")
	float WalkMultiplier = 0.33f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | No Weapon")
	float SprintMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "540.0", UIMin = "0.0", UIMax = "540.0"), Category = "Movement | No Weapon")
	float CharacterRotationRateWalk = 540.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "540.0", UIMin = "0.0", UIMax = "540.0"), Category = "Movement | No Weapon")
	float CharacterRotationRateFalling = 100.0f;

};
