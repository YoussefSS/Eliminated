// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"



class USoundCue;
class UParticleSystem;
class UParticleSystemComponent;
UCLASS()
class ELIMINATED_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* SkeletalMesh;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USoundCue* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USoundCue* EmptyClipSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleFlashFX;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	UParticleSystemComponent* MuzzleFlashPSC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* BulletTrailFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* BulletImpactWallFX;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName MuzzleFlashSocketName = "MuzzleFlash";

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

	void EnableWeapon();
	void DisableWeapon();

	bool CanFire();
	void StartFire();
	void Fire();
	void StopFire();

	bool CanReload();
	void StartReload();
	bool IsReloading() { return bIsReloading; }
	void EndReload();

protected:

	void PlayWeaponImpactEffect(FVector TargetPoint);
	void PlayWeaponTrailEffect(FVector TargetPoint);

	//////////////////////////////////////////////////////////////////////////////////
	// Customizable variables

	/** Rounds to fire per minute */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float RateOfFire = 0;

	/** How many rounds total does the weapon have */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float MaxAmmo = 100;

	/** How many rounds total does a clip have */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float AmmoPerClip = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float MuzzleFlashTime = 0.1;

	////////////////////////////////////////////////////////////////////////////////////
	// Other

	/** Total current ammo remaining */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	float CurrentAmmo = 0;

	/** Ammo remaining in the current clip */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	float CurrentClipAmmo = 0;

	/** Should the weapon currently be firing */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	bool bStartedFiring = false;

	/** How many seconds it will take to fire the next round, calculated on beginplay */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireAfterTime;

	FTimerHandle FireShot_TimerHandle;

	void StopMuzzleFlash();
	FTimerHandle MuzzleFlash_TimerHandle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	bool bIsReloading = false;

};
