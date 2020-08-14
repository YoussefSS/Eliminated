// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "Eliminated\Character\PlayerCharacter.h"
#include "GameFramework\CharacterMovementComponent.h"
void UPlayerAnimInstance::NativeInitializeAnimation()
{
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
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0);
		MovementSpeed = LateralSpeed.Size();

		bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();
	}
	else
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}


}
