// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "Eliminated\Character\PlayerCharacter.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Kismet\KismetMathLibrary.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}
}

void UPlayerAnimInstance::UpdateAnimationProperties()
{
	if(PlayerCharacter)
	{
		FVector Speed = PlayerCharacter->GetVelocity();

		Direction = CalculateDirection(Speed, PlayerCharacter->GetActorRotation());

		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0);
		MovementSpeedTotal = LateralSpeed.Size();

		bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();
		PlayerStatus = PlayerCharacter->GetMovementStatus();
		bIsReloading = PlayerCharacter->IsReloading();
		bIsCrouched = PlayerCharacter->IsCrouched();
		bShotFired = PlayerCharacter->bShotFired;

		//// Calculating aim offset
		FRotator ROTA = PlayerCharacter->GetControlRotation()	- PlayerCharacter->GetActorRotation();
		FRotator ROTB = FMath::RInterpTo(FRotator(AimPitch, AimYaw, 0), ROTA, DeltaTimeFromBP, 15);

		AimPitch = UKismetMathLibrary::ClampAngle(ROTB.Pitch, -90, 90);
		AimYaw = UKismetMathLibrary::ClampAngle(ROTB.Yaw, -90, 90);

	}
	else
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}


}
