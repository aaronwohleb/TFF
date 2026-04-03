// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletonEnemy.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h" 
#include "../Player/AttackData.h"
#include "SkeletonAIController.h"
#include <TFF/Player/CombatComponent.h>
#include <Kismet/GameplayStatics.h>

// Sets default values
ASkeletonEnemy::ASkeletonEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; // Tick in blueprint character
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
}

// Called when the game starts or when spawned
void ASkeletonEnemy::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	
}

float ASkeletonEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
	AController* EventInstigator, AActor* DamageCauser)
{

	UE_LOG(LogTemp, Warning, TEXT("Skeleton Enemy got hit"))

	UCombatComponent* PlayerCombat = DamageCauser->FindComponentByClass<UCombatComponent>();
	// 1. Try to see if the damage type used is our HandDamageType
	if ( PlayerCombat->GetActiveAttackData())
	{

		UAttackData* Data = PlayerCombat->GetActiveAttackData();
		EPlayerAttackHand PlayerHand = Data->Hand; //

		bool bSuccessfulBlock = false;

		if (CombatState == ESkeletonCombatState::BlockingLeft && PlayerHand == EPlayerAttackHand::Right)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skeleton BLOCKED Player Right Attack! Countering..."));

			bSuccessfulBlock = true;
		}
		else if (CombatState == ESkeletonCombatState::BlockingRight && PlayerHand == EPlayerAttackHand::Left)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skeleton BLOCKED Player Left Attack! Countering..."));

			bSuccessfulBlock = true;
		}

		if (bSuccessfulBlock)
		{
			// Reset the AI's attack timer so it continues being aggressive after the block
			if (ASkeletonAIController* AIC = Cast<ASkeletonAIController>(GetController()))
			{
				AIC->ForceImmediateAttack(); // Sets AttackTimer to 0
			}
			return 0.f; // Blocked! No damage applied.
		}
	}

	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - DamageToApply, 0.0f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("Skeleton Enemy got hit, now has %f health"),Health);

	PlayHitFlash();
	if (Health <= 0.f) {
		HandleDeath();
	}
	return DamageToApply;
}

void ASkeletonEnemy::PlayHitFlash()
{
	// Get the mesh component inherited from ACharacter
	USkeletalMeshComponent* MeshComp = GetMesh();

	if (MeshComp)
	{
		// 1. Instantly set the material parameter to 1.0 (Red Flash)
		MeshComp->SetScalarParameterValueOnMaterials(TEXT("HitAlpha"), 1.0f);

		// 2. Create a timer to reset the value
		FTimerHandle FlashTimer;

		// Capture MeshComp by value [=] to ensure it is available when the timer fires
		GetWorldTimerManager().SetTimer(FlashTimer, [MeshComp]()
			{
				if (IsValid(MeshComp))
				{
					MeshComp->SetScalarParameterValueOnMaterials(TEXT("HitAlpha"), 0.0f);
				}
			}, 0.1f, false);
	}
}

/*
	Checks to see if the punch hit the player during an attack animation.
*/
void ASkeletonEnemy::Attack() 
{

	UE_LOG(LogTemp, Warning, TEXT("CalledAttack"));
	ASkeletonAIController* AICon = Cast<ASkeletonAIController>(GetController());

	// Fallback: If AICon isn't available or hasn't found the target yet, 
	// we use the player pawn directly.
	AActor* Target = (AICon && AICon->GetTargetActor()) ? AICon->GetTargetActor() : UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (!Target) return;

	float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	// Check if the target is within a frontal cone
	FVector ToPlayer = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	float DotProduct = FVector::DotProduct(GetActorForwardVector(), ToPlayer);
	UE_LOG(LogTemp, Warning, TEXT("Distance to player %f dot product %f"), Distance, DotProduct);
	if (Distance <= AttackRange && DotProduct > 0.5f) // 0.5f is roughly a 60-degree cone in front
	{
		UGameplayStatics::ApplyDamage(
			Target,
			BaseDamage,
			GetController(), 
			this,           
			UDamageType::StaticClass()
		);

		UE_LOG(LogTemp, Warning, TEXT("Skeleton hit %s!"), *Target->GetName());
	}
}

void ASkeletonEnemy::PlayPunchMontage(bool bLeftArm)
{
	UAnimMontage* SelectedPunch = bLeftArm ? PunchLeftMontage : PunchRightMontage;

	if (SelectedPunch)
	{
		PlayAnimMontage(SelectedPunch);
	}

}

void ASkeletonEnemy::ResetCombatState(ESkeletonCombatState LastState)
{
	if (LastState == ESkeletonCombatState::BlockingLeft)
	{
		SetCombatState(ESkeletonCombatState::BlockingLeft);
		return;
	}
	else if (LastState == ESkeletonCombatState::BlockingRight) {
		SetCombatState(ESkeletonCombatState::BlockingRight);
		return;
	}
	else {
		SetCombatState(ESkeletonCombatState::IdleAttack);
		return; 
	}
}

void ASkeletonEnemy::HandleDeath()
{
	// 1. Safely handle the AI Controller
	if (AController* AICon = GetController())
	{
		AICon->StopMovement();
		AICon->Destroy(); // Remove the brain so it stops logic
	}

	// 2. Disable Capsule to stop the player from bumping into a dead body
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 3. Configure the Mesh for Ragdoll
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);

		// Apply the backward push
		FVector DeathImpulse = GetActorForwardVector() * -2000.0f;
		MeshComp->AddImpulse(DeathImpulse, NAME_None, true);
	}

	// 4. Set cleanup timer
	SetLifeSpan(10.0f);
}

