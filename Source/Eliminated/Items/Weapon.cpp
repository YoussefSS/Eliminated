// Fill out your copyright notice in the Description page of Project .


#include "Weapon.h"
#include "Sound\SoundBase.h"
#include "Kismet\GameplayStatics.h"
#include "Sound\SoundCue.h"
#include "Particles\ParticleSystemComponent.h"

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
	SetActorHiddenInGame(true);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(false);
}

void AWeapon::StartFire()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (MuzzleFlashFX)
	{
		MuzzleFlashPSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFlashFX, SkeletalMesh, MuzzleFlashSocketName);
		GetWorldTimerManager().SetTimer(MuzzleFlashTimer, this, &AWeapon::StopMuzzleFlash, MuzzleFlashTime);
	}
}

void AWeapon::StopFire()
{

}

void AWeapon::StopMuzzleFlash()
{
	if (MuzzleFlashPSC)
	{
		MuzzleFlashPSC->DeactivateSystem();
	}
}

