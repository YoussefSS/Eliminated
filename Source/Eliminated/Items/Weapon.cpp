// Fill out your copyright notice in the Description page of Project .


#include "Weapon.h"
#include "Sound\SoundBase.h"
#include "Kismet\GameplayStatics.h"
#include "Sound\SoundCue.h"
#include "Particles\ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Components\DecalComponent.h"
#include "PhysicalMaterials\PhysicalMaterial.h"
#include "Eliminated/Eliminated.h"
#include "Perception\AISense_Hearing.h"
#include "Eliminated\Character\SPlayerCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	FireAfterTime = 1 / (RateOfFire / 60);
	CurrentAmmo = MaxAmmo;
	CurrentClipAmmo = AmmoPerClip;
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::EnableWeapon()
{
	SetActorHiddenInGame(false);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(true);
}

void AWeapon::DisableWeapon()
{
	bStartedFiring = false;
	bIsReloading = false; // End reload in case the weapon is disabled before the reload animation finishes
	SetActorHiddenInGame(true);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);
}

bool AWeapon::CanFire()
{
	if (!bStartedFiring) return false;
	if (CurrentClipAmmo <= 0) return false;
	if (bIsReloading) return false;

	return true;
}

void AWeapon::StartFire()
{
	bStartedFiring = true;
	Fire();
}

void AWeapon::Fire()
{
	if (!CanFire())
	{
		if (CurrentClipAmmo <= 0 && !bIsReloading) // If out of ammo and not reloading, play empty clip sound
		{
			if (EmptyClipSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, EmptyClipSound, GetActorLocation());
			}
		}
		else // else try and fire again (useful for when reloading, and the player keeps pressing, it will resume firing after reload
		{
			GetWorldTimerManager().SetTimer(FireShot_TimerHandle, this, &AWeapon::Fire, FireAfterTime);
		}

		return;
	}

	// Play Sound
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	if (GetOwner()->IsA(ASPlayerCharacter::StaticClass()))
	{
		UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), WeaponSoundLoudness, GetOwner(), WeaponSoundMaxRange, NAME_None);
	}
	

	// Play Muzzle particle effect
	if (MuzzleFlashFX)
	{
		StopMuzzleFlash();
		MuzzleFlashPSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFlashFX, SkeletalMesh, MuzzleFlashSocketName);
		GetWorldTimerManager().SetTimer(MuzzleFlash_TimerHandle, this, &AWeapon::StopMuzzleFlash, MuzzleFlashTime);
	}


	// Fire Shot using LineTraceSingleByChannel
	if (GetOwner())
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet spread, does not add controller rotation
		if(bAdditionalBulletSpread)
		{
			float HalfRad = FMath::DegreesToRadians(BulletSpreadDegrees);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		}

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000); // The end is the direction we are looking + a big number

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true; // Will trace off each individial triangle of the mesh we are hitting (more expensive & more accurate)
		QueryParams.bReturnPhysicalMaterial = true;

		FVector LineTraceHitPoint = TraceEnd;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams)) // We used Visibility which means anything that is visible will block our trace
		{
			// TODO: Do another line trace to check if there is something in between the weapon and the hit location, NO NEED
			
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			// Hit logic here
			AActor* HitActor = Hit.GetActor();
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= HeadshotDamageMultiplier;
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, GetOwner()->GetInstigatorController(), this, DamageType);
			LineTraceHitPoint = Hit.ImpactPoint;

			

			PlayWeaponImpactEffect(LineTraceHitPoint, SurfaceType);

			// Bullet hole decal
			if (BulletHoleDecal)
			{
				
				UDecalComponent* DecalComp = UGameplayStatics::SpawnDecalAtLocation(this, BulletHoleDecal, DecalSize, Hit.ImpactPoint, Hit.Normal.Rotation(), DecalLifeSpan);
				DecalComp->SetFadeScreenSize(DecalFadeScreenSize);
			}
		}



		// Play weapon trail effect
		PlayWeaponTrailEffect(LineTraceHitPoint);

	}

	OnShotFired.Broadcast();
	
	// Reduce ammo
	CurrentClipAmmo--;
	OnWeaponAmmoChanged.Broadcast(CurrentAmmo, CurrentClipAmmo);

	// Repeat the shot based on the rate of fire
	GetWorldTimerManager().SetTimer(FireShot_TimerHandle, this, &AWeapon::Fire, FireAfterTime);
}

void AWeapon::StopFire()
{
	bStartedFiring = false;
}

bool AWeapon::CanReload()
{
	if (bIsReloading) return false;
	if (CurrentAmmo <= 0) return false;
	if (CurrentClipAmmo >= AmmoPerClip) return false;

	return true;
}

void AWeapon::StartReload()
{
	bIsReloading = true;

	// Play Sound
	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}
}

void AWeapon::EndReload()
{
	bIsReloading = false;

	// Calculate ammo
	float AmmoNeededForReload = AmmoPerClip - CurrentClipAmmo; // The number of bullets needed for the reload
	float ConsumedAmmo;
	if (CurrentAmmo >= AmmoNeededForReload) // Current ammo is enough for reload, so just take the ammo needed for reload
	{
		ConsumedAmmo = AmmoNeededForReload;
	}
	else // Current ammo is NOT enough for reload, so take the current ammo
	{
		ConsumedAmmo = CurrentAmmo;
	}

	CurrentAmmo -= ConsumedAmmo;
	CurrentClipAmmo += ConsumedAmmo;

	OnWeaponAmmoChanged.Broadcast(CurrentAmmo, CurrentClipAmmo);
}

void AWeapon::PlayWeaponImpactEffect(FVector TargetPoint, EPhysicalSurface SurfaceType)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactFX;
		break;

	default:
		SelectedEffect = DefaultImpactFX;
		break;
	}

	if (SelectedEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, TargetPoint);
	}
}

void AWeapon::PlayWeaponTrailEffect(FVector TargetPoint)
{
	FVector MuzzleLocation = SkeletalMesh->GetSocketLocation(MuzzleFlashSocketName);
	UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletTrailFX, MuzzleLocation);
	if (TracerComp)
	{
		TracerComp->SetVectorParameter("ShockBeamEnd", TargetPoint);
	}
}

void AWeapon::StopMuzzleFlash()
{
	if (MuzzleFlashPSC)
	{
		MuzzleFlashPSC->DeactivateSystem();
	}
}

