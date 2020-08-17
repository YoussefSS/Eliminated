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

	void StartFire();
	void Fire();
	void StopFire();

protected:

	void PlayWeaponImpactEffect(FVector TargetPoint);
	void PlayWeaponTrailEffect(FVector TargetPoint);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bStartedFiring = false;

	// Rounds to fire per minute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float RateOfFire = 0;

	// How many seconds it will take to fire the next round, calculated on beginplay
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireAfterTime;

	FTimerHandle FireShot_TimerHandle;

	void StopMuzzleFlash();

	FTimerHandle MuzzleFlash_TimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float MuzzleFlashTime = 0.1;

};
