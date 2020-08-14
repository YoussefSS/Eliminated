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
	void MoveRight(float Val);


	virtual void Jump() override;

	void StartSprint();
	void StopSprint();
	bool bIsSprinting;

	void UpdateRotationRate();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement | No Weapon")
	float WalkMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement | No Weapon")
	float SprintMultiplier = 1.f;

};
