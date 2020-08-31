// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAmmoChanged, int32, NewCurrentAmmoCounte, int32, NewCurrentClipAmmoCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShotFired);

class USoundCue;
class UParticleSystem;
class UParticleSystemComponent;
class UDamageType;
UCLASS()
class ELIMINATED_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	
	/////////////////////////////////////////////////////////////////
	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* SkeletalMesh;

	////////////////////////////////////////////////////////////////


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Sound")
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Sound")
	USoundCue* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Sound")
	USoundCue* EmptyClipSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	UParticleSystem* MuzzleFlashFX;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon| Visual Effects")
	UParticleSystemComponent* MuzzleFlashPSC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	UParticleSystem* BulletTrailFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	UParticleSystem* BulletImpactWallFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	UMaterialInterface* BulletHoleDecal;



protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	FVector DecalSize = FVector(7,7,7);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	float DecalLifeSpan = 5.f;

	/** The smaller the number the bigger the render distance, 0-0.001 will render from far */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon| Visual Effects")
	float DecalFadeScreenSize = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon| Visual Effects")
	FName MuzzleFlashSocketName = "MuzzleFlash";

	/** Time to play the muzzle flash effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon| Visual Effects")
	float MuzzleFlashTime = 0.1;

	/** Rounds to fire per minute */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float RateOfFire = 0;

	/** How many rounds total does the weapon have */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo = 100;

	/** How many rounds total does a clip have */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 AmmoPerClip = 10;

	/** Amount of recoil to add as controller pitch rotation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float RecoilAmount = 0.1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

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

	int32 GetCurrentAmmoCount() { return CurrentAmmo; }
	int32 GetCurrentClipAmmoCount() { return CurrentClipAmmo; }

	FOnWeaponAmmoChanged OnWeaponAmmoChanged;
	FOnShotFired OnShotFired;

	float GetRecoilAmount() { return RecoilAmount; }
protected:

	void PlayWeaponImpactEffect(FVector TargetPoint);
	void PlayWeaponTrailEffect(FVector TargetPoint);



	////////////////////////////////////////////////////////////////
	// State

	/** Total current ammo remaining */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmo = 0;

	/** Ammo remaining in the current clip */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentClipAmmo = 0;

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

	////////////////////////////////////////////////////////////////
};
