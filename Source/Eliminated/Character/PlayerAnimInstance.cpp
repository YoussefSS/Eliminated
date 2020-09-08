// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "Eliminated\Character\SCharacterBase.h"
#include "GameFramework\CharacterMovementComponent.h"
#include "Kismet\KismetMathLibrary.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (!SCharacterBase)
	{
		SCharacterBase = Cast<ASCharacterBase>(TryGetPawnOwner());
	}
}

void UPlayerAnimInstance::UpdateAnimationProperties()
{
	if(SCharacterBase)
	{
		FVector Speed = SCharacterBase->GetVelocity();

		Direction = CalculateDirection(Speed, SCharacterBase->GetActorRotation());

		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0);
		MovementSpeedTotal = LateralSpeed.Size();

		bIsInAir = SCharacterBase->GetCharacterMovement()->IsFalling();
		PlayerStatus = SCharacterBase->GetMovementStatus();
		bIsReloading = SCharacterBase->IsReloading();
		bIsCrouched = SCharacterBase->IsCrouched();
		bIsDead = SCharacterBase->IsDead();

		//// Calculating aim offset
		FRotator ROTA = SCharacterBase->GetControlRotation()	- SCharacterBase->GetActorRotation();
		FRotator ROTB = FMath::RInterpTo(FRotator(AimPitch, AimYaw, 0), ROTA, DeltaTimeFromBP, 15);

		AimPitch = UKismetMathLibrary::ClampAngle(ROTB.Pitch, -90, 90);
		AimYaw = UKismetMathLibrary::ClampAngle(ROTB.Yaw, -90, 90);

	}
	else
	{
		SCharacterBase = Cast<ASCharacterBase>(TryGetPawnOwner());
	}


}
