// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SkeletonEnemy.generated.h"

UENUM(BlueprintType)
enum class ESkeletonCombatState : uint8
{
	Idle,
	Running,
	BlockingLeft,
	BlockingRight,
	IdleAttack,
	Attacking,
	Dead	
};

UCLASS()
class TFF_API ASkeletonEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ASkeletonEnemy();

protected:
	virtual void BeginPlay() override;

	// Health implementation mirroring Dummy for consistency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 300.0f;

	// Current stance for the AI
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	ESkeletonCombatState CombatState = ESkeletonCombatState::Idle;

	// Reference for the Left Arm punch
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat | Animations")
	class UAnimMontage* PunchLeftMontage;

	// Reference for the Right Arm punch
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat | Animations")
	class UAnimMontage* PunchRightMontage;

public:
	// Override TakeDamage for custom hit logic and flashing
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Function to handle the red flash effect via Materials
	void PlayHitFlash();

	void SetCombatState(ESkeletonCombatState NewState) { CombatState = NewState; }

	ESkeletonCombatState GetCombatState() const { return CombatState; }

	void Attack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseDamage = 20.0f;

	void PlayPunchMontage(bool bLeftArm);

	void ResetCombatState(ESkeletonCombatState LastState);

private:

	void HandleDeath();
};
