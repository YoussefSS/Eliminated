// Fill out your copyright notice in the Description page of Project .


#include "Weapon.h"
#include "Sound\SoundBase.h"
#include "Kismet\GameplayStatics.h"
#include "Sound\SoundCue.h"
#include "Particles\ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

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
	SetActorHiddenInGame(true);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);
}

void AWeapon::StartFire()
{
	bStartedFiring = true;
	Fire();
}

void AWeapon::Fire()
{
	if (!bStartedFiring) return;

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
		}



		// Play weapon trail effect
		PlayWeaponTrailEffect(LineTraceHitPoint);
	

	}

	


	// Repeat the shot based on the rate of fire
	GetWorldTimerManager().SetTimer(FireShot_TimerHandle, this, &AWeapon::Fire, FireAfterTime);
}

void AWeapon::StopFire()
{
	bStartedFiring = false;
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

