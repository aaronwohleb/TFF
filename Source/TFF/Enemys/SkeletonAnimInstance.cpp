// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletonAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h" 

void USkeletonAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds); //

    if (!SkeletonCharacter)
    {
        SkeletonCharacter = Cast<ASkeletonEnemy>(TryGetPawnOwner()); 
    }

    if (!SkeletonCharacter) return;

    // Grab the values the AI just set
    FVector TargetVelocity = SkeletonCharacter->GetVelocity();

    SmoothedVelocity = FMath::VInterpTo(SmoothedVelocity, TargetVelocity, DeltaSeconds, VelocityInterpSpeed);

    GroundSpeed = SmoothedVelocity.Size();

    const FRotator Rotation = SkeletonCharacter->GetActorRotation();
    const FVector LocalVelocity = Rotation.UnrotateVector(SmoothedVelocity);

    VelocityX = LocalVelocity.X; // Left / Right
    VelocityY = LocalVelocity.Y; // Forward / Backward

    // This allows the AnimBP to see if we are in 'Running' or 'CombatStance'
    CurrentCombatState = SkeletonCharacter->GetCombatState();

    float TargetWeight = (CurrentCombatState == ESkeletonCombatState::Idle ||
        CurrentCombatState == ESkeletonCombatState::Running) ? 0.0f : 1.0f;

    // Smoothly interpolate the weight so it doesn't snap
    StanceWeight = FMath::FInterpTo(StanceWeight, TargetWeight, DeltaSeconds, 5.0f);
}

