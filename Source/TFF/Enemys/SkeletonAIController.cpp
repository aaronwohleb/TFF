// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletonAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TFF/Player/Martin.h"
#include "SkeletonEnemy.h"

ASkeletonAIController::ASkeletonAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Sight Configuration
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 2000.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ASkeletonAIController::BeginPlay()
{
	Super::BeginPlay();
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ASkeletonAIController::OnTargetDetected);
	ResetAttackTimer();
}

void ASkeletonAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (Cast<AMartin>(Actor))
	{
		TargetActor = Stimulus.WasSuccessfullySensed() ? Actor : nullptr;
	}
}

void ASkeletonAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetActor)
	{
		HandleMovementLogic();

		float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());

		// Only switch stances or attack if we are within range
		if (DistanceToTarget <= AttackRange)
		{
			StanceChangeTimer -= DeltaTime;

			if (StanceChangeTimer <= 0.f)
			{
				UpdateCombatStance();

				// Reset the timer to a random value between 1.0 and 1.5 seconds
				StanceChangeTimer = FMath::RandRange(1.f, 1.5f);
			}

			AttackTimer -= DeltaTime;
			if (AttackTimer <= 0.f)
			{
				AttemptAttack(); // Triggers attack based on current stance
				ResetAttackTimer(); // Start the 3-5s countdown again
			}

			// Strafing Logic
			StrafeTimer -= DeltaTime;
			if (StrafeTimer <= 0.f)
			{
				// 30% chance to strafe left, 30% right, 40% stay central
				int32 Rand = FMath::RandRange(0, 10);
				if (Rand < 2) CurrentStrafeDirection = -1;      // Left
				else if (Rand < 4) CurrentStrafeDirection = 1;  // Right
				else CurrentStrafeDirection = 0;               // Central

				StrafeTimer = FMath::RandRange(1.0f, StrafeChangeInterval);
			}
		}
	}
}

void ASkeletonAIController::ResetAttackTimer()
{
	// Sets the next attack to happen between 3 and 5 seconds from now
	AttackTimer = FMath::RandRange(3.0f, 5.0f);
}

void ASkeletonAIController::UpdateCombatStance()
{
	// Get the controlled pawn and cast it to our SkeletonEnemy class
	ASkeletonEnemy* Skeleton = Cast<ASkeletonEnemy>(GetPawn());
	if (!Skeleton) return;

	// Generate a random number between 0 and 100 to choose a state
	int32 RandomPercentage = FMath::RandRange(0, 100);
	ESkeletonCombatState CurrentStance = Skeleton->GetCombatState();

	// Stace cant be Idle Attacking twice in a row
	if (RandomPercentage < 20 && CurrentStance != ESkeletonCombatState::IdleAttack)
	{
		// 20% chance to go to Idle
		Skeleton->SetCombatState(ESkeletonCombatState::IdleAttack);
		return;
	}
	else 
	{
		// 50% chance to go to Blocking (Left or Right)
		if (FMath::RandBool())
		{
			Skeleton->SetCombatState(ESkeletonCombatState::BlockingLeft);
		}
		else
		{
			Skeleton->SetCombatState(ESkeletonCombatState::BlockingRight);
		}
		return;
	}

	


}

void ASkeletonAIController::AttemptAttack()
{
	ASkeletonEnemy* Skeleton = Cast<ASkeletonEnemy>(GetPawn());
	if (!Skeleton) return;

	StopMovement();
	ESkeletonCombatState CurrentStance = Skeleton->GetCombatState();
	bool bShouldPunchLeft = false;

	if (CurrentStance == ESkeletonCombatState::BlockingLeft)
	{
		bShouldPunchLeft = false; // Left is blocking, punch right
	}
	else if (CurrentStance == ESkeletonCombatState::BlockingRight)
	{
		bShouldPunchLeft = true; // Right is blocking, punch left
	}
	else
	{
		// Randomly choose an arm if in a neutral combat stance
		bShouldPunchLeft = FMath::RandBool();
	}

	Skeleton->SetCombatState(ESkeletonCombatState::Attacking);

	Skeleton->PlayPunchMontage(bShouldPunchLeft);
}

void ASkeletonAIController::ForceImmediateAttack()
{
	AttackTimer = 0.f;
}

void ASkeletonAIController::HandleMovementLogic()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !TargetActor) return;

	float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
	ASkeletonEnemy* Skeleton = Cast<ASkeletonEnemy>(ControlledPawn);
	UCharacterMovementComponent* MoveComp = Skeleton ? Skeleton->GetCharacterMovement() : nullptr;

	if (!MoveComp || !Skeleton) return;

	ESkeletonCombatState CurrentState = Skeleton->GetCombatState();

	if (DistanceToTarget > AttackRange && 
		(CurrentState == ESkeletonCombatState::Idle || CurrentState == ESkeletonCombatState::Running))
	{
		MoveComp->MaxWalkSpeed = RunningSpeed; // Sprint Speed
		Skeleton->SetCombatState(ESkeletonCombatState::Running); // Transition out of stance
		MoveToActor(TargetActor, 100.f);
	}
	else if (Skeleton->GetCombatState() != ESkeletonCombatState::Attacking)
	{
		MoveComp->MaxWalkSpeed = WalkingSpeed; // Walk Speed

		// Phase 2: Enter Attack Mode & Maintain Range
		StopMovement();

		// Logic for walking forward/backward based on OptimalCombatDistance

		FVector ForwardDir = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();
		ForwardDir.Normalize();

		FVector RightDir = FVector::CrossProduct(FVector::UpVector, ForwardDir);

		if (DistanceToTarget < OptimalCombatDistance - 50.f)
		{
			// Too close: Back up
			ControlledPawn->AddMovementInput(-ForwardDir, 0.5f);
		}
		else if (DistanceToTarget > OptimalCombatDistance + 50.f)
		{
			// Too far: Step forward
			ControlledPawn->AddMovementInput(ForwardDir, 0.5f);
		}

		// Strafe Movement
		if (CurrentStrafeDirection != 0)
		{
			// Apply sideways movement based on the timer roll
			ControlledPawn->AddMovementInput(RightDir, 0.5f * CurrentStrafeDirection);
		}
		if (DistanceToTarget >= BreakAttackRange) {
						// If we exceed the break range, chase the player again
			Skeleton->SetCombatState(ESkeletonCombatState::Running);
			StanceChangeTimer = 0.f; // When skeleton catches up, re-evaluate stance immediately
		}

	}
}

void ASkeletonAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	Super::UpdateControlRotation(DeltaTime, bUpdatePawn);

	// Ensure the skeleton always faces the player during combat
	if (TargetActor && GetPawn())
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
		LookAtRotation.Pitch = 0.f; // Keep the skeleton upright
		LookAtRotation.Roll = 0.f;

		GetPawn()->SetActorRotation(FMath::RInterpTo(GetPawn()->GetActorRotation(), LookAtRotation, DeltaTime, 5.f));
	}
}