// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "SkeletonAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;



UCLASS()
class TFF_API ASkeletonAIController : public AAIController
{
	GENERATED_BODY()
	

public:
	ASkeletonAIController();
protected:
	virtual void BeginPlay() override;

	// Perception Component to handle player detection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* PerceptionComp;

	// Config for Sight sense
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	// The distance where the skeleton stops running and enters fighting stance
	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	float AttackRange = 400.f;

	// The distance where the skeleton breaks off attack and starts running at the player again
	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	float BreakAttackRange = 650.f;

	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	float RunningSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	float WalkingSpeed = 300.f;

	// The distance the skeleton tries to maintain (forward/backward)
	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	float OptimalCombatDistance = 200.f;

	// -1 for Left, 1 for Right, 0 for Standing Still
	int32 CurrentStrafeDirection = 0;

	float StrafeTimer = 1.f;

	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	float StrafeChangeInterval = 3.0f; // How often he re-rolls his move

	// Called when perception updates (player seen/lost)
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

private:
	AActor* TargetActor = nullptr;

	void HandleMovementLogic();
	virtual void Tick(float DeltaTime) override;

protected:
	// How often the AI considers changing its stance (e.g., every 1.5 to 3 seconds)
	float StanceChangeTimer = 0.f;

	float AttackTimer = 0.f;

	void ResetAttackTimer();

	// Logic to decide which stance to enter
	void UpdateCombatStance();

	// Logic to trigger an attack based on the current stance
	void AttemptAttack();

public:
	void ForceImmediateAttack();

	AActor* GetTargetActor() { return TargetActor; }

};




