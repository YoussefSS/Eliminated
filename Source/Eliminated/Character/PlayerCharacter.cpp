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

	bUseControllerRotationYaw = false; // Might want to set this to true when aiming down sights and orientrotationtomovement to false
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->JumpZVelocity = 500.f;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
	if (FMath::IsNearlyEqual(Val, 0)) return;
	
	// Getting movement direction
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// Getting movement speed
	float MoveSpeed = bIsSprinting ? Val * SprintMultiplier : Val * WalkMultiplier;


	AddMovementInput(Direction, MoveSpeed);
}

void APlayerCharacter::MoveRight(float Val)
{
	if (FMath::IsNearlyEqual(Val, 0)) return;

	// Getting movement direction
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); // Find out which way is right

	// Getting movement speed
	float MoveSpeed = bIsSprinting ? Val * SprintMultiplier : Val * WalkMultiplier;


	AddMovementInput(Direction, MoveSpeed);
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

void APlayerCharacter::UpdateRotationRate()
{
	// Limit rotation while falling/jumping
	if (GetCharacterMovement())
	{
		if (GetCharacterMovement()->IsFalling())
		{
			GetCharacterMovement()->RotationRate = FRotator(0.f, 100.f, 0.f);
		}
		else
		{
			GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
		}
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRotationRate();

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
}

