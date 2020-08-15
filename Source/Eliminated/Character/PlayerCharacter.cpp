// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera\CameraComponent.h"
#include "GameFramework\CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(GetRootComponent());
	CameraArm->TargetArmLength = 600.f;
	CameraArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->JumpZVelocity = 500.f;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->RotationRate = FRotator(0.f, CharacterRotationRateWalk, 0.f);

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
	Super::Jump();
}

void APlayerCharacter::StartSprint()
{
	bIsSprinting = true;
}

void APlayerCharacter::StopSprint()
{
	bIsSprinting = false;
}

void APlayerCharacter::StartAimDownSights_Implementation() // Implementation for C++ method (can be overriden in BP)
{
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	MovementStatus = EMovementStatus::EMS_Pistol;
}

void APlayerCharacter::StopAimDownSights_Implementation()
{
	//CameraArm->TargetArmLength = 600.f;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	MovementStatus = EMovementStatus::EMS_NoWeapon;
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
	float WalkMultiplier;
	float SprintMultiplier;
	switch (MovementStatus)
	{
	case EMovementStatus::EMS_NoWeapon: 
		WalkMultiplier = WalkMultiplier_NoWeapon;
		SprintMultiplier = SprintMultiplier_NoWeapon;
		break;
	case EMovementStatus::EMS_Pistol:
		WalkMultiplier = WalkMultiplier_AimDownSight;
		SprintMultiplier = SprintMultiplier_AimDownSight;
		break;
	case EMovementStatus::EMS_MAX:
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
}

