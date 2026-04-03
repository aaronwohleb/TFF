// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimMontage.h"
#include "AttackData.generated.h"


UENUM(BlueprintType) 
enum class EPlayerAttackHand : uint8 {
	Right,
	Left
};

UENUM(BlueprintType)
enum class EPlayerAttackType : uint8 {
	Jab,
	Hook
};


/**
 * 
 */
UCLASS()
class TFF_API UAttackData : public UDataAsset
{
	GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EPlayerAttackHand Hand;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EPlayerAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimMontage* ChargingMontage; // charging attack montage

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinChargeTime = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Damage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback")
    TSubclassOf<class UCameraShakeBase> AttackCameraShake;

};
