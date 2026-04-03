// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TFF/Enemys/SkeletonEnemy.h"
#include "SkeletonAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TFF_API USkeletonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// CITE: The standard update function for animation data
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector SmoothedVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float VelocityX;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float VelocityY;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	ESkeletonCombatState CurrentCombatState;
	
	// Weight for blending into combat stance animations
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float StanceWeight;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float VelocityInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StanceInterpSpeed = 4.0f;

private:
	// CITE: Internal reference to the skeleton actor
	UPROPERTY()
	class ASkeletonEnemy* SkeletonCharacter;
};
