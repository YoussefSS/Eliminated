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

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
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
	MoveForwardAxisVal = Val;
}

void APlayerCharacter::MoveRight(float Val)
{
	MoveRightAxisVal = Val;
}

void APlayerCharacter::Jump()
{
	ResetMovementToWalk();
	Super::Jump();
}

void APlayerCharacter::StartSprint()
{
	if (PlayerStatus == EPlayerStatus::EMS_CrouchedNoWeapon)
	{
		ResetMovementToWalk();
	}
	bIsSprinting = true;
}

void APlayerCharacter::StopSprint()
{
	bIsSprinting = false;
}

void APlayerCharacter::ToggleCrouch()
{
	if (!bIsCrouched)
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
	ResetMovementToWalk();

	StopSprint();
	Super::Crouch();
	PlayerStatus = EPlayerStatus::EMS_CrouchedNoWeapon;
	bIsCrouched = true;
	if (CameraBoom)
	{
		CameraBoom->SetRelativeLocation(FVector(0, 0, CamHeightCrouched));
	}
}

void APlayerCharacter::StopCrouch()
{
	ResetMovementToWalk();
}

void APlayerCharacter::StartAimDownSights() // Implementation for C++ method (can be overriden in BP)
{
	ResetMovementToWalk();
	
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	StartAimDownSights_Event();

	PlayerStatus = EPlayerStatus::EMS_Pistol;
	EnablePistol();

	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());
	if (PC)
	{
		PC->ShowCrossHair();
	}
}

void APlayerCharacter::StopAimDownSights()
{
	DisableCurrentWeapon();

	if (PlayerStatus == EPlayerStatus::EMS_CrouchedNoWeapon) return;

	ResetMovementToWalk();
}

void APlayerCharacter::ResetMovementToWalk()
{
	PlayerStatus = EPlayerStatus::EMS_NoWeapon;

	
	// Uncrouching
	Super::UnCrouch();
	bIsCrouched = false;
	if (CameraBoom)
	{
		CameraBoom->SetRelativeLocation(FVector(0, 0, 0));
	}

	// Stopping looking down sights
	DisableCurrentWeapon();
	StopAimDownSights_Event();
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());
	if (PC)
	{
		PC->HideCrossHair();
	}
}

void APlayerCharacter::StartFire()
{
	if (CurrentWeapon)
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

	if (CurrentWeapon->CanReload())
	{
		DoReload();
	}
}

void APlayerCharacter::DoReload()
{
	ensure(CurrentWeapon);

	bIsReloading = true;
	CurrentWeapon->StartReload();
}

void APlayerCharacter::StopReload()
{
	if (!CurrentWeapon) return;

	bIsReloading = false;
	CurrentWeapon->EndReload();
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
	case EPlayerStatus::EMS_Pistol:
		WalkMultiplier = WalkMultiplier_AimDownSight;
		SprintMultiplier = SprintMultiplier_AimDownSight;
		break;
	case EPlayerStatus::EMS_CrouchedNoWeapon:
		WalkMultiplier = WalkMultiplier_Crouched;
		break;
	case EPlayerStatus::EMS_MAX:
	default:
		WalkMultiplier = 0;
		SprintMultiplier = 0;
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::UpdateMovementAxisInput: Reached default case"));
		break;

	}
	float MoveSpeedForward = bIsSprinting ? (MoveForwardAxisVal * SprintMultiplier) : (MoveForwardAxisVal * WalkMultiplier);
	float MoveSpeedRight = bIsSprinting ? (MoveRightAxisVal * SprintMultiplier) : (MoveRightAxisVal * WalkMultiplier);

	/* Combining both movement inputs so that diagonal movement doesn't become faster than horizontal/vertical movement */
	if (FMath::IsNearlyEqual(FMath::Abs(MoveSpeedForward), FMath::Abs(MoveSpeedRight))) // If both axis are equal
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
}

FVector APlayerCharacter::GetPawnViewLocation() const
{
	if (Camera)
	{
		return Camera->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void APlayerCharacter::DisableCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DisableWeapon();
	}
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
		}

		EnablePistol();
	}
}
