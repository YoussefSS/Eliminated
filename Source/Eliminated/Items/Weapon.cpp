// Fill out your copyright notice in the Description page of Project .


#include "Weapon.h"
#include "Sound\SoundBase.h"
#include "Kismet\GameplayStatics.h"
#include "Sound\SoundCue.h"
#include "Particles\ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Components\DecalComponent.h"

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
	EndReload();
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
		if (CurrentClipAmmo <= 0) // If out of ammo, play empty clip sound
		{
			if (EmptyClipSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, EmptyClipSound, GetActorLocation());
			}
		}

		return;
	}

	// Play Sound
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// Play Muzzle particle effect
	if (MuzzleFlashFX)
	{
		MuzzleFlashPSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFlashFX, SkeletalMesh, MuzzleFlashSocketName);
		GetWorldTimerManager().SetTimer(MuzzleFlash_TimerHandle, this, &AWeapon::StopMuzzleFlash, MuzzleFlashTime);
	}


	// Fire Shot using LineTraceSingleByChannel
	if (GetOwner())
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation); // These are out parameters
		FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000); // The end is the direction we are looking + a big number

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true; // Will trace off each individial triangle of the mesh we are hitting (more expensive & more accurate)

		FVector LineTraceHitPoint = TraceEnd;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams)) // We used Visibility which means anything that is visible will block our trace
		{
			// Hit logic here
			LineTraceHitPoint = Hit.ImpactPoint;
			PlayWeaponImpactEffect(LineTraceHitPoint);

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
	bool bCanReloadA = true;
	CurrentAmmo <= 0 ? bCanReloadA = false : bCanReloadA = true;

	bool bCanReloadB = true;
	CurrentClipAmmo >= AmmoPerClip ? bCanReloadB = false : bCanReloadB = true;
	return bCanReloadA & bCanReloadB;
}

void AWeapon::StartReload()
{
	bIsReloading = true;

	// Play Sound
	if (ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
	}

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

void AWeapon::EndReload()
{
	bIsReloading = false;
}

void AWeapon::PlayWeaponImpactEffect(FVector TargetPoint)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpactWallFX, TargetPoint);
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

