// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera\CameraComponent.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Eliminated\Items\Weapon.h"
#include "Engine\World.h"
#include "Components\SkeletalMeshComponent.h"
#include "Eliminated\Character\PlayerCharacterController.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Allowing the player to move freely in all directions without having to look at where the camera is looking
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	

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
void APlayerCharacter::BeginPlay()
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
}

void APlayerCharacter::MousePitchInput(float Val)
{
	AddControllerPitchInput(Val);
}

void APlayerCharacter::MouseYawInput(float Val)
{
	AddControllerYawInput(Val);
}

void APlayerCharacter::MoveForward(float Val)
{
	// Setting the axis value to be used on the next frame
	MoveForwardAxisVal = Val;
}

void APlayerCharacter::MoveRight(float Val)
{
	// Setting the axis value to be used on the next frame
	MoveRightAxisVal = Val;
}

void APlayerCharacter::Jump()
{
	StopCrouch();
	Super::Jump();
}

void APlayerCharacter::StartSprint()
{
	if (bIsCrouching)
	{
		StopCrouch();
	}
	bIsSprinting = true;
}

void APlayerCharacter::StopSprint()
{
	bIsSprinting = false;
}

void APlayerCharacter::ToggleCrouch()
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

void APlayerCharacter::StartCrouch()
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

void APlayerCharacter::StopCrouch()
{
	Super::UnCrouch();
	bIsCrouching = false;

	// Adjusting camera boom height so that the camera doesn't jitter when crouching and the capsule half size inscreases
	if (CameraBoom)
	{
		CameraBoom->SetRelativeLocation(FVector(0, 0, 0));
	}
}

void APlayerCharacter::StartAimDownSights() // Implementation for C++ method (can be overriden in BP)
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
	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());
	if (PC)
	{
		PC->ShowCrossHair();
	}
}

void APlayerCharacter::StopAimDownSights()
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
	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());
	if (PC)
	{
		PC->HideCrossHair();
	}
	bIsAimingDownSights = false;
}

void APlayerCharacter::StartFire()
{
	if (CurrentWeapon && bIsAimingDownSights)
	{
		CurrentWeapon->StartFire();
	}
}

void APlayerCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void APlayerCharacter::TryReload()
{
	if (!CurrentWeapon) return;
	if (!bIsAimingDownSights) return;

	if (CurrentWeapon->CanReload())
	{
		DoReload();
	}
}

void APlayerCharacter::SelectWeaponOne()
{
	CurrentSelectedWeaponSlot = 1;

	// Use if bIsAimingDownSights if you don't want to hold a weapon while not aiming
	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void APlayerCharacter::SelectWeaponTwo()
{
	CurrentSelectedWeaponSlot = 2;

	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void APlayerCharacter::SelectNextWeapon()
{
	CurrentSelectedWeaponSlot++;
	if (CurrentSelectedWeaponSlot >= MaxNumberOfWeaponSlots + 1)
	{
		CurrentSelectedWeaponSlot = 1;
	}

	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void APlayerCharacter::SelectPreviousWeapon()
{
	CurrentSelectedWeaponSlot--;
	if (CurrentSelectedWeaponSlot <= 0)
	{
		CurrentSelectedWeaponSlot = MaxNumberOfWeaponSlots;
	}

	ChangeCurrentWeaponToSelectedWeapon(bIsAimingDownSights);
}

void APlayerCharacter::DoReload()
{
	ensure(CurrentWeapon);

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
}

void APlayerCharacter::OnEndReload() /** Called when the reload animation ends from animinstance */
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

void APlayerCharacter::OnWeaponAmmoChanged(int32 NewCurrentAmmo, int32 NewCurrentClipAmmo)
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
	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());
	if (PC)
	{
		PC->UpdateHUDAmmoCounter(NewCurrentAmmo, NewCurrentClipAmmo);
	}
	
}

void APlayerCharacter::OnShotFired()
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
	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());
	if (PC)
	{
		if (PistolFireCamShake) PC->ClientPlayCameraShake(PistolFireCamShake);
	}
}

void APlayerCharacter::UpdateRotationRate()
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

void APlayerCharacter::UpdateMovementAxisInput()
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
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::UpdateMovementAxisInput: Reached default case"));
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
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotationRate();
	UpdateMovementAxisInput();
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::MousePitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &APlayerCharacter::MouseYawInput);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &APlayerCharacter::Jump);

	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &APlayerCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &APlayerCharacter::StopSprint);

	PlayerInputComponent->BindAction("AimDownSights", EInputEvent::IE_Pressed, this, &APlayerCharacter::StartAimDownSights);
	PlayerInputComponent->BindAction("AimDownSights", EInputEvent::IE_Released, this, &APlayerCharacter::StopAimDownSights);

	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &APlayerCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &APlayerCharacter::StopFire);

	PlayerInputComponent->BindAction("HoldCrouch", EInputEvent::IE_Pressed, this, &APlayerCharacter::StartCrouch);
	PlayerInputComponent->BindAction("HoldCrouch", EInputEvent::IE_Released, this, &APlayerCharacter::StopCrouch);

	PlayerInputComponent->BindAction("ToggleCrouch", EInputEvent::IE_Pressed, this, &APlayerCharacter::ToggleCrouch);

	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &APlayerCharacter::TryReload);

	PlayerInputComponent->BindAction("SelectWeapon1", EInputEvent::IE_Pressed, this, &APlayerCharacter::SelectWeaponOne);
	PlayerInputComponent->BindAction("SelectWeapon2", EInputEvent::IE_Pressed, this, &APlayerCharacter::SelectWeaponTwo);

	PlayerInputComponent->BindAction("NextWeapon", EInputEvent::IE_Pressed, this, &APlayerCharacter::SelectNextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", EInputEvent::IE_Pressed, this, &APlayerCharacter::SelectPreviousWeapon);
}

FVector APlayerCharacter::GetPawnViewLocation() const
{
	if (Camera)
	{
		return Camera->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void APlayerCharacter::ChangeCurrentWeaponToSelectedWeapon(bool bSetPlayerStatus)
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

void APlayerCharacter::DisableCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DisableWeapon();
	}
	PlayerStatus = EPlayerStatus::EMS_NoWeapon;
	bIsReloading = false;
	CurrentWeapon = nullptr;
}

void APlayerCharacter::EnablePistol()
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
			Pistol->OnWeaponAmmoChanged.AddDynamic(this, &APlayerCharacter::OnWeaponAmmoChanged);
			OnWeaponAmmoChanged(Pistol->GetCurrentAmmoCount(), Pistol->GetCurrentClipAmmoCount()); // Calling it now to update the ammo hud counter

			Pistol->OnShotFired.AddDynamic(this, &APlayerCharacter::OnShotFired);
		}
		else
		{
			return;
		}

		EnablePistol();
	}
}

void APlayerCharacter::EnableRifle()
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
			Rifle->OnWeaponAmmoChanged.AddDynamic(this, &APlayerCharacter::OnWeaponAmmoChanged);
			OnWeaponAmmoChanged(Rifle->GetCurrentAmmoCount(), Rifle->GetCurrentClipAmmoCount()); // Calling it now to update the ammo hud counter

			Rifle->OnShotFired.AddDynamic(this, &APlayerCharacter::OnShotFired);
		}
		else
		{
			return;
		}

		EnableRifle();
	}
}
