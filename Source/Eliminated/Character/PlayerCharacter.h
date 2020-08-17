// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	EMS_NoWeapon			UMETA(DisplayName = "NoWeapon"),
	EMS_CrouchedNoWeapon	UMETA(DisplayName = "CrouchedNoWeapon"),
	EMS_Pistol				UMETA(DisplayName = "Pistol"),

	EMS_MAX					UMETA(DisplayName = "DefaultMAX")
};


class AWeapon;
UCLASS()
class ELIMINATED_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class UCameraComponent* Camera;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	/** Input Functions */
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

	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	bool bIsCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,  Category = "Camera")
	float CamHeightNormal = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CamHeightCrouched;

	void StartAimDownSights();
	void StopAimDownSights();

	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	void StartAimDownSights_Event();
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	void StopAimDownSights_Event();

	void StartFire();
	void StopFire();
	/** End Input Functions */



	void UpdateRotationRate();
	void UpdateMovementAxisInput();

	void ResetMovementToWalk();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE EPlayerStatus GetMovementStatus() { return PlayerStatus; }

	virtual FVector GetPawnViewLocation() const override;

protected:

	void DisableCurrentWeapon();
	void EnablePistol();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	AWeapon* CurrentWeapon;

	/** PISTOL */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf <AWeapon> PistolClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName PistolAttachSocketName = "PistolSocket";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AWeapon* Pistol;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "540.0", UIMin = "0.0", UIMax = "540.0"), Category = "Movement")
	float CharacterRotationRateWalk = 540.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "540.0", UIMin = "0.0", UIMax = "540.0"), Category = "Movement")
	float CharacterRotationRateFalling = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "100.0", ClampMax = "700.0", UIMin = "100.0", UIMax = "700.0"), Category = "Movement")
	float SpringArmDistance_Regular = 600.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "100.0", ClampMax = "700.0", UIMin = "100.0", UIMax = "700.0"), Category = "Movement")
	float SpringArmDistance_AimDownSight = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | No Weapon")
	float WalkMultiplier_NoWeapon = 0.33f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | No Weapon")
	float SprintMultiplier_NoWeapon = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | AimDownSight")
	float WalkMultiplier_AimDownSight = 0.15f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | AimDownSight")
	float SprintMultiplier_AimDownSight = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement | Crouch")
	float WalkMultiplier_Crouched = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AimDownSight")
	FVector SpringArmCameraOffset_AimDownSight;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	EPlayerStatus PlayerStatus = EPlayerStatus::EMS_NoWeapon;




};
