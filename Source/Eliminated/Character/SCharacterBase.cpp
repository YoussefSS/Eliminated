// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacterBase.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera\CameraComponent.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Eliminated\Items\Weapon.h"
#include "Engine\World.h"
#include "Components\SkeletalMeshComponent.h"
#include "Eliminated\Character\SCharacterBaseController.h"
#include "Components\CapsuleComponent.h"
#include "Eliminated\Components\HealthComponent.h"
#include "Eliminated\Eliminated.h"
#include "Components\SphereComponent.h"
#include "Kismet\GameplayStatics.h"
#include "Kismet\KismetMathLibrary.h"
#include "Sound\SoundCue.h"
#include "Eliminated\EliminatedGameModeBase.h"
#include "Perception\AISense_Hearing.h"
#include "MatineeCameraShake.h"

// Sets default values
ASCharacterBase::ASCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	PunchSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PunchSphereComponent"));
	PunchSphereComponent->SetupAttachment(GetRootComponent());
	PunchSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PunchSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PunchSphereComponent->SetSphereRadius(200.f);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// Allowing the player to move freely in all directions without having to look at where the camera is looking
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true; // Player doesn't have to look at camera
		GetCharacterMovement()->AirControl = 0.2f;
		GetCharacterMovement()->JumpZVelocity = 500.f;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;

		GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // Allowing crouching
		GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	}
}

// Called when the game starts or when spawned
void ASCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->RotationRate = FRotator(0.f, CharacterRotationRateWalk, 0.f);
	CamHeightCrouched = 38;

	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = SpringArmDistance_Regular;
	}

	CurrentSelectedWeaponSlot = 1;
	if (bKeepHoldingWeaponWhileNotAiming)
	{
		ChangeCurrentWeaponToSelectedWeapon(false);
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ASCharacterBase::OnHealthChanged);
	}

	AEliminatedGameModeBase* GM = Cast<AEliminatedGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->OnEnemyDied.AddDynamic(this, &ASCharacterBase::OnEnemyDied);
	}

	if (FootstepCue)
	{
		FootstepCue->VolumeMultiplier = FootstepsVolume;
	}

	OnTakePointDamage.AddDynamic(this, &ASCharacterBase::HandleTakePointDamage);
}

void ASCharacterBase::MousePitchInput(float Val)
{
	AddControllerPitchInput(Val);
}

void ASCharacterBase::MouseYawInput(float Val)
{
	AddControllerYawInput(Val);
}

void ASCharacterBase::MoveForward(float Val)
{
	// Setting the axis value to be used on the next frame
	MoveForwardAxisVal = Val;
}

void ASCharacterBase::MoveRight(float Val)
{
	// Setting the axis value to be used on the next frame
	MoveRightAxisVal = Val;
}

void ASCharacterBase::Jump()
{
	StopCrouch();
	Super::Jump();
}

void ASCharacterBase::StartSprint()
{
	if (bIsCrouching)
	{
		StopCrouch();
	}
	bIsSprinting = true;
}

void ASCharacterBase::StopSprint()
{
	bIsSprinting = false;
}

void ASCharacterBase::ToggleCrouch()
{
	if (!bIsCrouching)
	{
		StartCrouch();
	}
	else
	{
		StopCrouch();
	}
}

void ASCharacterBase::StartCrouch()
{
	StopSprint();
	Super::Crouch();
	bIsCrouching = true;

	// Adjusting camera boom height so that the camera doesn't jitter when crouching and the capsule half size decreases
	if (CameraBoom)
	{
		CameraBoom->SetRelativeLocation(FVector(0, 0, CamHeightCrouched));
	}
}

void ASCharacterBase::StopCrouch()
{
	Super::UnCrouch();
	bIsCrouching = false;

	// Adjusting camera boom height so that the camera doesn't jitter when crouching and the capsule half size inscreases
	if (CameraBoom)
	{
		CameraBoom->SetRelativeLocation(FVector(0, 0, 0));
	}
}

void ASCharacterBase::StartAimDownSights() // Implementation for C++ method (can be overriden in BP)
{
	// Start the BP timeline event
	StartAimDownSights_Event(); 
	bIsAimingDownSights = true;

	// Enable weapon and select it and update animations
	if (CurrentSelectedWeaponSlot == 1)
	{
		PlayerStatus = EPlayerStatus::EMS_DownSightsRifle;
	}
	else
	{
		PlayerStatus = EPlayerStatus::EMS_DownSightsPistol;
	}

	if (!bKeepHoldingWeaponWhileNotAiming)
	{
		ChangeCurrentWeaponToSelectedWeapon();
	}
	

	// Setting the player to look at the camera
	bUseControllerRotationYaw = true;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	

	// Showing crosshair
	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		PC->ShowCrossHair();
	}
}

void ASCharacterBase::StopAimDownSights()
{
	StopAimDownSights_Event();

	// If not reloading then return to normal animation state, if reloading it will be handled by OnEndReload
	if (!bIsReloading)
	{
		PlayerStatus = EPlayerStatus::EMS_NoWeapon;
		if (!bKeepHoldingWeaponWhileNotAiming)
		{
			DisableCurrentWeapon();
		}
	}

	// Setting the player to run freely without looking at the camera
	bUseControllerRotationYaw = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	// Hiding crosshair
	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		PC->HideCrossHair();
	}
	bIsAimingDownSights = false;
}

void ASCharacterBase::StartFire()
{
	if (CurrentWeapon && bIsAimingDownSights)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacterBase::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacterBase::PlayFootstepSound()
{
	if (FootstepCue)
	{
		float Volume =
			UKismetMathLibrary::MapRangeClamped(GetVelocity().Size(), 10.f, 600.f, 0.f, 1.f)
			* IsCrouched() ? 0.1f : 1.f
			* bIsSprinting ? 1.f : 0.3f;

		UGameplayStatics::PlaySoundAtLocation(this, FootstepCue, GetActorLocation(), Volume);

		if (bReportFootstepNoiseEvent)
		{
			float NoiseRange;

			if (bIsSprinting)
			{
				NoiseRange = SprintFootstepNoise;
			}
			else if (bIsCrouching)
			{
				NoiseRange = CrouchFootstepNoise;
			}
			else // Normal
			{
				NoiseRange = WalkFootstepNoise;
			}
			UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1, this, NoiseRange, NAME_None);
		}
	}
	
}

void ASCharacterBase::TryReload()
{
	if (!CurrentWeapon) return;

	
	if (CurrentWeapon->CanReload())
	{
		DoReload();
	}
}

void ASCharacterBase::SelectWeaponOne()
{
	CurrentSelectedWeaponSlot = 1;

	// Use if bIsAimingDownSights if you don't want to hold a weapon while not aiming
	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void ASCharacterBase::SelectWeaponTwo()
{
	CurrentSelectedWeaponSlot = 2;

	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void ASCharacterBase::SelectNextWeapon()
{
	CurrentSelectedWeaponSlot++;
	if (CurrentSelectedWeaponSlot >= MaxNumberOfWeaponSlots + 1)
	{
		CurrentSelectedWeaponSlot = 1;
	}

	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void ASCharacterBase::SelectPreviousWeapon()
{
	CurrentSelectedWeaponSlot--;
	if (CurrentSelectedWeaponSlot <= 0)
	{
		CurrentSelectedWeaponSlot = MaxNumberOfWeaponSlots;
	}

	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void ASCharacterBase::StartPunch()
{
	// Play animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (PunchMontage)
		{
			AnimInstance->Montage_Play(PunchMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Punch"), PunchMontage);
		}
	}

	// Stop Movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
	if (GetController())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		DisableInput(PC);
	}

	// Look at AI logic
	TArray<AActor*> OverlappingActors;
	PunchSphereComponent->GetOverlappingActors(OverlappingActors);

	ASCharacterBase* ClosestAI = nullptr;
	float MinDistanceToAI = INFINITY;
	for (AActor* OverlappedActor : OverlappingActors)
	{
		ASCharacterBase* AIChar = Cast<ASCharacterBase>(OverlappedActor);
		if (AIChar)
		{
			if (AIChar == this)
			{
				continue;
			}

			if (ClosestAI == nullptr)
			{
				ClosestAI = AIChar;
			}
			else
			{
				float DistanceToAI = (AIChar->GetActorLocation() - GetActorLocation()).Size();
				if (DistanceToAI < MinDistanceToAI)
				{
					MinDistanceToAI = DistanceToAI;
					ClosestAI = AIChar;
				}
			}

		}
	}

	AIInCloseProximity = nullptr;
	if (ClosestAI)
	{
		FRotator LookAtAIRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ClosestAI->GetActorLocation());
		LookAtAIRotation.Pitch = 0;
		LookAtAIRotation.Roll = 0;
		SetActorRotation(LookAtAIRotation);
		AIInCloseProximity = ClosestAI;
	}
}

void ASCharacterBase::PauseKeyPressed()
{
	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		PC->TogglePauseMenu();
	}
}

void ASCharacterBase::DoPunch()
{
	// Punch damage logic
	if (AIInCloseProximity)
	{
		UGameplayStatics::ApplyDamage(AIInCloseProximity, PunchDamage, GetInstigatorController(), this, PunchDamageType);

		if (PunchSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PunchSound, GetActorLocation());
		}
	}
}

void ASCharacterBase::EndPunch()
{
	// Start Movement again
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	if (GetController())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		EnableInput(PC);
	}

	AIInCloseProximity = nullptr;
}

void ASCharacterBase::DoReload()
{
	ensure(CurrentWeapon);

	// Reloading while not aiming down sights
	bool bTempWasntAimingDownSights = false;
	if (!bIsAimingDownSights)
	{
		StartAimDownSights();
		bTempWasntAimingDownSights = true;
	}
	//

	bIsReloading = true;
	CurrentWeapon->StartReload();

	// Playing reload animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (PlayerStatus == EPlayerStatus::EMS_DownSightsPistol && PistolMontage)
		{
			AnimInstance->Montage_Play(PistolMontage, .8f);
			AnimInstance->Montage_JumpToSection(FName("Reload"), PistolMontage);
		}
		else if (PlayerStatus == EPlayerStatus::EMS_DownSightsRifle && RifleMontage)
		{
			AnimInstance->Montage_Play(RifleMontage, 1.2f);
			AnimInstance->Montage_JumpToSection(FName("Reload"), RifleMontage);
		}
	}


	if (bTempWasntAimingDownSights)
	{
		StopAimDownSights();
	}
}

void ASCharacterBase::OnHealthChanged(UHealthComponent* HealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsDead) return;

	//UE_LOG(LogTemp, Warning, TEXT("CurrentHealth: %f, Random %f"), CurrentHealth, 5.f);

	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		if (HealthComp)
		{
			PC->UpdateHealthBar(HealthComp->GetMaxHealth(), CurrentHealth);
		}
	}

	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void ASCharacterBase::HandleTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	LastShotHitLocation = HitLocation;
	LastShotDirection = ShotFromDirection;
}

void ASCharacterBase::Die()
{
	bIsDead = true;

	StopAimDownSights();
	StopFire();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Turning on ragdoll
	if (GetMesh())
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->AddImpulseAtLocation(LastShotDirection * 40000, LastShotHitLocation);

		GetWorldTimerManager().SetTimer(StopRagdoll_TimerHandle, this, &ASCharacterBase::StopRagdoll, StopRagdollTime);
	}

	
	// Show lose menu
	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		PC->ShowLoseMenu();
	}

	OnDeath();
}

void ASCharacterBase::OnEnemyDied(AActor* DeadEnemy, int32 RemainingEnemies)
{
	
}

void ASCharacterBase::StopRagdoll()
{
	if (GetMesh())
	{
		// Using these plays the idle animation
		//GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//GetMesh()->SetAllBodiesSimulatePhysics(false);
		//GetMesh()->SetSimulatePhysics(false);
		GetMesh()->PutAllRigidBodiesToSleep();
	}
}

void ASCharacterBase::OnEndReload() /** Called when the reload animation ends from animinstance */
{
	bIsReloading = false;
	if (!CurrentWeapon) return;

	CurrentWeapon->EndReload();

	// If not aiming down sights, return to no weapon animations
	if (!bIsAimingDownSights)
	{
		PlayerStatus = EPlayerStatus::EMS_NoWeapon;
		if (!bKeepHoldingWeaponWhileNotAiming)
		{
			DisableCurrentWeapon();
		}
	}
}

void ASCharacterBase::OnWeaponAmmoChanged(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo)
{
	// Reload if the current weapon clip is empty
	if (bAutoReloadIfClipIsEmpty && NewCurrentClipAmmo <= 0)
	{
		TryReload();

		// Update ammo numbers after reload
		if (CurrentWeapon)
		{
			NewCurrentAmmo = CurrentWeapon->GetCurrentAmmoCount();
			NewCurrentClipAmmo = CurrentWeapon->GetCurrentClipAmmoCount();
		}
		
	}

	// Update the ammo counter in the HUD
	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		PC->UpdateHUDAmmoCounter(NewCurrentAmmo, NewCurrentClipAmmo);
	}
	
}

void ASCharacterBase::OnShotFired()
{
	ensure(CurrentWeapon);

	// Recoil
	AddControllerPitchInput(-CurrentWeapon->GetRecoilAmount());
	AddControllerYawInput(FMath::FRandRange(-CurrentWeapon->GetRecoilAmount(), CurrentWeapon->GetRecoilAmount()));
	//AddControllerPitchInput(0);

	// Play animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (PlayerStatus == EPlayerStatus::EMS_DownSightsPistol && PistolMontage)
		{
			AnimInstance->Montage_Play(PistolMontage, 1.2f);
			AnimInstance->Montage_JumpToSection(FName("Fire"), PistolMontage);
		}
		else if (PlayerStatus == EPlayerStatus::EMS_DownSightsRifle && RifleMontage)
		{
			AnimInstance->Montage_Play(RifleMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Fire"), RifleMontage);
		}
	}


	// Cam shake
	ASCharacterBaseController* PC = Cast<ASCharacterBaseController>(GetController());
	if (PC)
	{
		if (PistolFireCamShake) PC->ClientPlayCameraShake(PistolFireCamShake);
	}
}

void ASCharacterBase::UpdateRotationRate()
{
	// Limit rotation while falling/jumping
	if (GetCharacterMovement())
	{
		if (GetCharacterMovement()->IsFalling())
		{
			GetCharacterMovement()->RotationRate = FRotator(0.f, CharacterRotationRateFalling, 0.f);
		}
		else
		{
			GetCharacterMovement()->RotationRate = FRotator(0.f, CharacterRotationRateWalk, 0.f);
		}
	}
}

void ASCharacterBase::UpdateMovementAxisInput()
{
	// If both axis values are zero
	if (FMath::IsNearlyZero(MoveForwardAxisVal) && FMath::IsNearlyZero(MoveRightAxisVal)) return;

	// Getting movement direction
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // Find out which way is right

	// Getting movement speed
	float WalkMultiplier= 0;
	float SprintMultiplier = 0;
	switch (PlayerStatus)
	{
	case EPlayerStatus::EMS_NoWeapon: 
		WalkMultiplier = WalkMultiplier_NoWeapon;
		SprintMultiplier = SprintMultiplier_NoWeapon;
		break;
	case EPlayerStatus::EMS_DownSightsPistol:
	case EPlayerStatus::EMS_DownSightsRifle:
		WalkMultiplier = WalkMultiplier_AimDownSight;
		SprintMultiplier = SprintMultiplier_AimDownSight;
		break;

	case EPlayerStatus::EMS_MAX:
	default:
		WalkMultiplier = 0;
		SprintMultiplier = 0;
		UE_LOG(LogTemp, Warning, TEXT("ASCharacterBase::UpdateMovementAxisInput: Reached default case"));
		break;

	}

	if (bIsCrouching)
	{
		WalkMultiplier = WalkMultiplier_Crouched;
	}

	float MoveSpeedForward = bIsSprinting ? (MoveForwardAxisVal * SprintMultiplier) : (MoveForwardAxisVal * WalkMultiplier);
	float MoveSpeedRight = bIsSprinting ? (MoveRightAxisVal * SprintMultiplier) : (MoveRightAxisVal * WalkMultiplier);


	// Combining both movement inputs so that diagonal movement doesn't become faster than horizontal/vertical movement
	if (FMath::IsNearlyEqual(FMath::Abs(MoveSpeedForward), FMath::Abs(MoveSpeedRight))) // If both axis are nearly equal, won't work well with analog sticks, as the axis will never be equal
	{
		MoveSpeedForward *= 0.71f;
		MoveSpeedRight *= 0.71f;
	}
	AddMovementInput(ForwardDirection, MoveSpeedForward);
	AddMovementInput(RightDirection, MoveSpeedRight);
}



// Called every frame
void ASCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotationRate();
	UpdateMovementAxisInput();
}

// Called to bind functionality to input
void ASCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacterBase::MousePitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacterBase::MouseYawInput);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacterBase::MoveRight);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ASCharacterBase::Jump);

	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &ASCharacterBase::StartSprint);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &ASCharacterBase::StopSprint);

	PlayerInputComponent->BindAction("AimDownSights", EInputEvent::IE_Pressed, this, &ASCharacterBase::StartAimDownSights);
	PlayerInputComponent->BindAction("AimDownSights", EInputEvent::IE_Released, this, &ASCharacterBase::StopAimDownSights);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ASCharacterBase::StartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ASCharacterBase::StopFire);

	PlayerInputComponent->BindAction("HoldCrouch", EInputEvent::IE_Pressed, this, &ASCharacterBase::StartCrouch);
	PlayerInputComponent->BindAction("HoldCrouch", EInputEvent::IE_Released, this, &ASCharacterBase::StopCrouch);

	PlayerInputComponent->BindAction("ToggleCrouch", EInputEvent::IE_Pressed, this, &ASCharacterBase::ToggleCrouch);

	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ASCharacterBase::TryReload);

	PlayerInputComponent->BindAction("SelectWeapon1", EInputEvent::IE_Pressed, this, &ASCharacterBase::SelectWeaponOne);
	PlayerInputComponent->BindAction("SelectWeapon2", EInputEvent::IE_Pressed, this, &ASCharacterBase::SelectWeaponTwo);

	PlayerInputComponent->BindAction("NextWeapon", EInputEvent::IE_Pressed, this, &ASCharacterBase::SelectNextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", EInputEvent::IE_Pressed, this, &ASCharacterBase::SelectPreviousWeapon);

	PlayerInputComponent->BindAction("Punch", EInputEvent::IE_Pressed, this, &ASCharacterBase::StartPunch);

	PlayerInputComponent->BindAction("Pause", EInputEvent::IE_Pressed, this, &ASCharacterBase::PauseKeyPressed);
}

FVector ASCharacterBase::GetPawnViewLocation() const
{
	if (Camera)
	{
		return Camera->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacterBase::ChangeCurrentWeaponToSelectedWeapon(bool bSetPlayerStatus)
{
	DisableCurrentWeapon();

	if (CurrentSelectedWeaponSlot == 1)
	{
		EnableRifle();

		if(bSetPlayerStatus) PlayerStatus = EPlayerStatus::EMS_DownSightsRifle;
	}

	if (CurrentSelectedWeaponSlot == 2)
	{
		EnablePistol();
		
		if(bSetPlayerStatus) PlayerStatus = EPlayerStatus::EMS_DownSightsPistol;
	}

	// Update HUD
	if (CurrentWeapon)
	{
		OnWeaponAmmoChanged(CurrentWeapon->GetCurrentAmmoCount(), CurrentWeapon->GetCurrentClipAmmoCount());
	}
}

void ASCharacterBase::DisableCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DisableWeapon();
	}
	PlayerStatus = EPlayerStatus::EMS_NoWeapon;
	bIsReloading = false;
	CurrentWeapon = nullptr;
}

void ASCharacterBase::EnablePistol()
{
	if (Pistol)
	{
		CurrentWeapon = Pistol;
		Pistol->EnableWeapon();
	}
	else
	{
		Pistol = GetWorld()->SpawnActor<AWeapon>(PistolClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (Pistol)
		{
			Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, PistolAttachSocketName);
			Pistol->SetOwner(this);
			Pistol->OnWeaponAmmoChanged.AddDynamic(this, &ASCharacterBase::OnWeaponAmmoChanged);
			OnWeaponAmmoChanged(Pistol->GetCurrentAmmoCount(), Pistol->GetCurrentClipAmmoCount()); // Calling it now to update the ammo hud counter

			Pistol->OnShotFired.AddDynamic(this, &ASCharacterBase::OnShotFired);
		}
		else
		{
			return;
		}

		EnablePistol();
	}
}

void ASCharacterBase::EnableRifle()
{
	if (Rifle)
	{
		CurrentWeapon = Rifle;
		Rifle->EnableWeapon();
	}
	else
	{
		Rifle = GetWorld()->SpawnActor<AWeapon>(RifleClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (Rifle)
		{
			Rifle->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, RifleAttachSocketName);
			Rifle->SetOwner(this);
			Rifle->OnWeaponAmmoChanged.AddDynamic(this, &ASCharacterBase::OnWeaponAmmoChanged);
			OnWeaponAmmoChanged(Rifle->GetCurrentAmmoCount(), Rifle->GetCurrentClipAmmoCount()); // Calling it now to update the ammo hud counter

			Rifle->OnShotFired.AddDynamic(this, &ASCharacterBase::OnShotFired);
		}
		else
		{
			return;
		}

		EnableRifle();
	}
}
