// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacterBase.generated.h"

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	EMS_NoWeapon						UMETA(DisplayName = "NoWeapon"),
	EMS_DownSightsPistol				UMETA(DisplayName = "DownSightsPistol"),
	EMS_DownSightsRifle					UMETA(DisplayName = "DownSightsRifle"),

	EMS_MAX								UMETA(DisplayName = "DefaultMAX")
};



class AWeapon;
class UDamageType;
class USoundCue;

UCLASS()
class ELIMINATED_API ASCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class USphereComponent* PunchSphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	class UHealthComponent* HealthComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	////////////////////////////////////////////////////////////////
	// Input Functions

	void MousePitchInput(float Val);
	void MouseYawInput(float Val);

	void MoveForward(float Val);
	float MoveForwardAxisVal = 0;
	void MoveRight(float Val);
	float MoveRightAxisVal = 0;


	virtual void Jump() override;

	void StartSprint();
	void StopSprint();
	

	void ToggleCrouch();
	void StartCrouch();
	void StopCrouch();
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,  Category = "Camera")
	float CamHeightNormal = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CamHeightCrouched;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	virtual void StartAimDownSights();
	UFUNCTION(BlueprintCallable, Category = "Camera")
	virtual void StopAimDownSights();
	

	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	void StartAimDownSights_Event();
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	void StopAimDownSights_Event();

	void StartFire();
	void StopFire();

	/** Try to reload when pressing the reload button */
	void TryReload();


	void SelectWeaponOne();
	void SelectWeaponTwo();
	void SelectNextWeapon();
	void SelectPreviousWeapon();
	
	void StartPunch();

	////////////////////////////////////////////////////////////////



	void UpdateRotationRate();
	void UpdateMovementAxisInput();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE EPlayerStatus GetMovementStatus() { return PlayerStatus; }

	virtual FVector GetPawnViewLocation() const override;

	bool IsReloading() { return bIsReloading; }

	bool IsCrouched() { return bIsCrouching; }

	bool IsDead() { return bIsDead; }

	/** Called when the reload animation ends (from animinstance)  */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnEndReload();

	UFUNCTION()
	void OnWeaponAmmoChanged(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo);

	/** Shot firing animation */
	UFUNCTION()
	void OnShotFired();

	/** Do the punching logic */
	UFUNCTION(BlueprintCallable, Category = "Punching")
	void DoPunch();

	/** End the punching (enable movement again) */
	UFUNCTION(BlueprintCallable, Category = "Punching")
	void EndPunch();

protected:

	void ChangeCurrentWeaponToSelectedWeapon(bool bSetPlayerStatus = true);
	void DisableCurrentWeapon();
	void EnablePistol();
	void EnableRifle();

	/** Do the reload action, only call this from TryReload */
	void DoReload();

	/** When health is changed from the HealthComponent */
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* HealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void HandleTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punching")
	class UAnimMontage* PunchMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punching")
	float PunchDamage = 1000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punching")
	TSubclassOf<class UDamageType> PunchDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching")
	USoundCue* PunchSound;

	/** The maximum number of weapon slots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 MaxNumberOfWeaponSlots = 2;

	/** Should the weapon automatically reload if the clip is empty */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	bool bAutoReloadIfClipIsEmpty = true;

	/** Should the character keep holding the weapon in the hand even if they are not aiming down sights  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	bool bKeepHoldingWeaponWhileNotAiming = true;


	////////////////////////////////////////////////////////
	// PISTOL

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Pistol")
	TSubclassOf <AWeapon> PistolClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Pistol")
	FName PistolAttachSocketName = "PistolSocket";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon| Pistol")
	AWeapon* Pistol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon| Pistol")
	class UAnimMontage* PistolMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Pistol")
	TSubclassOf<UCameraShake> PistolFireCamShake;

	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// RIFLE

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Rifle")
	TSubclassOf <AWeapon> RifleClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Rifle")
	FName RifleAttachSocketName = "RifleSocket";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon| Rifle")
	AWeapon* Rifle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon| Rifle")
	class UAnimMontage* RifleMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Rifle")
	TSubclassOf<UCameraShake> RifleFireCamShake;

	////////////////////////////////////////////////////////

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "540.0", UIMin = "0.0", UIMax = "540.0"), Category = "Movement")
	float CharacterRotationRateWalk = 540.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "540.0", UIMin = "0.0", UIMax = "540.0"), Category = "Movement")
	float CharacterRotationRateFalling = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "700.0", UIMin = "0.0", UIMax = "700.0"), Category = "Movement")
	float SpringArmDistance_Regular = 600.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "700.0", UIMin = "0.0", UIMax = "700.0"), Category = "Movement")
	float SpringArmDistance_AimDownSight = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement| No Weapon")
	float WalkMultiplier_NoWeapon = 0.33f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement| No Weapon")
	float SprintMultiplier_NoWeapon = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement| AimDownSight")
	float WalkMultiplier_AimDownSight = 0.15f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement| AimDownSight")
	float SprintMultiplier_AimDownSight = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement| AimDownSight")
	FVector SpringArmCameraOffset_AimDownSight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = "Movement| Crouch")
	float WalkMultiplier_Crouched = 0.1f;



	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	EPlayerStatus PlayerStatus = EPlayerStatus::EMS_NoWeapon;

protected:

	////////////////////////////////////////////////////////
	// STATE

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Movement")
	bool bIsAimingDownSights;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Movement")
	bool bIsCrouching;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Movement")
	bool bIsDead = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	bool bIsReloading = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentSelectedWeaponSlot = 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	AWeapon* CurrentWeapon;

	FVector LastShotHitLocation;
	FVector LastShotDirection;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Punching")
	ASCharacterBase* AIInCloseProximity;

};
