// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

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
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimationProperties")
    bool bIsInAir;
};
