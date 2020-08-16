// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "Eliminated\Character\PlayerCharacter.h" // TODO: Low Pri: remove this include

#include "PlayerAnimInstance.generated.h"



enum class EPlayerStatus : uint8;
/** Update animation properties in this class for the animation asset to see
 * 
 */
UCLASS()
class ELIMINATED_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = "AnimationProperties")
	void UpdateAnimationProperties();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
    class APlayerCharacter* PlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
    float MovementSpeedTotal;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
	float Direction;

    // Set this in BP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationProperties")
    float DeltaTimeFromBP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
    bool bIsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
    EPlayerStatus PlayerStatus;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
    float AimPitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
	float AimYaw;
};
