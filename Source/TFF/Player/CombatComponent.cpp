// Fill out your copyright notice in the Description page of Project Settings.

#include "TFF/Player/CombatComponent.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AttackData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraShakeBase.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();	

	if (AActor* Owner = GetOwner())
	{

		// find the skeletal mesh named "Arms"
		TArray<USkeletalMeshComponent*> SkelMeshes;
		Owner->GetComponents(SkelMeshes);
		for (auto* Comp : SkelMeshes)
		{
			if (Comp->GetName() == "Arms")
			{
				ArmsMesh = Comp;
				break;
			}
		}
	}
}


// Called every frame (Old tick implementaion for charging attacks)
//void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//	if (bRightCharging) RightHandCharge += DeltaTime;
//	if (bLeftCharging)  LeftHandCharge += DeltaTime;
//	
//}

void UCombatComponent::StartAttack(EPlayerAttackHand Hand)
{
	if (!bRightCharging && !bLeftCharging && CurrentBlockStatus == EPlayerBlockStatus::None) { // Prevent starting another charge while already charging

		if (Hand == EPlayerAttackHand::Right)
		{
			RightHandCharge = GetWorld()->GetTimeSeconds();
			bRightCharging = true;
		}
		else
		{
			LeftHandCharge = GetWorld()->GetTimeSeconds();
			bLeftCharging = true;
		}

		// Find the Hook data to play the 'Charging' loop immediately
		UAttackData* HookData = GetAttackDataByType(Hand, EPlayerAttackType::Hook);
		if (HookData && HookData->ChargingMontage)
		{
			PlayAttackMontage(HookData->ChargingMontage); // Uses the raw montage
		}
	}
}



void UCombatComponent::ReleaseAttack(EPlayerAttackHand Hand)
{
	// Calculate the duration: (Current Time - Start Time)
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	float CalculatedCharge = (Hand == EPlayerAttackHand::Right)
		? (CurrentTime - RightHandCharge)
		: (CurrentTime - LeftHandCharge);

	// Stop charging loop before release
	if (ArmsMesh && ArmsMesh->GetAnimInstance())
	{
		UAttackData* HookData = GetAttackDataByType(Hand, EPlayerAttackType::Hook);
		if (HookData && HookData->ChargingMontage)
		{
			ArmsMesh->GetAnimInstance()->Montage_Stop(1.0f, HookData->ChargingMontage);
		}
	}

	if (Hand == EPlayerAttackHand::Right) bRightCharging = false;
	else bLeftCharging = false;

	ActiveAttackData = SelectAttack(Hand, CalculatedCharge);

	if (ActiveAttackData)
	{
		PlayAttackMontage(ActiveAttackData); // Uses the AttackData

		if(ActiveAttackData->AttackType == EPlayerAttackType::Jab)
		{
			UE_LOG(LogTemp, Warning, TEXT("Performed Jab with %s hand, Damage: %f"), (Hand == EPlayerAttackHand::Right) ? TEXT("Right") : TEXT("Left"), ActiveAttackData->Damage);
		}
		else if(ActiveAttackData->AttackType == EPlayerAttackType::Hook)
		{
			UE_LOG(LogTemp, Warning, TEXT("Performed Hook with %s hand, Damage: %f"), (Hand == EPlayerAttackHand::Right) ? TEXT("Right") : TEXT("Left"), ActiveAttackData->Damage);
		}
	}
}

void UCombatComponent::StartBlock()
{
	// Cancel any active attack charges immediately to prevent state bleeding
	bRightCharging = false;
	bLeftCharging = false;

	UE_LOG(LogTemp, Warning, TEXT("Started a block"));

	// Stop charging montage
	if (ArmsMesh && ArmsMesh->GetAnimInstance())
	{
		UAttackData* RightHook = GetAttackDataByType(EPlayerAttackHand::Right, EPlayerAttackType::Hook);
		if (RightHook && RightHook->ChargingMontage)
		{
			ArmsMesh->GetAnimInstance()->Montage_Stop(0.1f, RightHook->ChargingMontage);
		}

		UAttackData* LeftHook = GetAttackDataByType(EPlayerAttackHand::Left, EPlayerAttackType::Hook);
		if (LeftHook && LeftHook->ChargingMontage)
		{
			ArmsMesh->GetAnimInstance()->Montage_Stop(0.1f, LeftHook->ChargingMontage);
		}
	}

	// Set the state to Parrying
	CurrentBlockStatus = EPlayerBlockStatus::Parrying;

	// Play block animation
	if (BlockStartMontage)
	{
		PlayAttackMontage(BlockStartMontage);
	}
	PlayCameraShake(BlockCameraShake);

	// Start parry timer
	GetWorld()->GetTimerManager().SetTimer(
		ParryTimerHandle,
		this,
		&UCombatComponent::DowngradeParryToBlock,
		ParryWindowDuration,
		false // Do not loop
	);
}

void UCombatComponent::DowngradeParryToBlock()
{
	if (CurrentBlockStatus == EPlayerBlockStatus::Parrying)
	{
		CurrentBlockStatus = EPlayerBlockStatus::Blocking;
	}
}

void UCombatComponent::ReleaseBlock()
{
	// Clear the timer 
	GetWorld()->GetTimerManager().ClearTimer(ParryTimerHandle);

	CurrentBlockStatus = EPlayerBlockStatus::None;

	// Stop the block montage if it is currently playing
	if (ArmsMesh && ArmsMesh->GetAnimInstance() && BlockStartMontage)
	{
		ArmsMesh->GetAnimInstance()->Montage_Stop(0.2f, BlockStartMontage);
	}
}

void UCombatComponent::HandleSuccessfulParry()
{
	// This is called from AMartin::TakeDamage during the Parrying state

	if (ParrySuccessMontage)
	{
		PlayAttackMontage(ParrySuccessMontage);
	}

	// TODO: add stun effect to damage causer, time dilation and sound effect
	PlayCameraShake(ParrySuccessCameraShake);
	UE_LOG(LogTemp, Warning, TEXT("Parry Successful! Damage negated."));
}

UAttackData* UCombatComponent::GetAttackDataByType(EPlayerAttackHand Hand, EPlayerAttackType Type)
{
	for (UAttackData* Attack : AttackList)
	{
		if (Attack && Attack->Hand == Hand && Attack->AttackType == Type) return Attack;
	}
	return nullptr;
}

UAttackData* UCombatComponent::SelectAttack(EPlayerAttackHand Hand, float ChargeTime)
{
	for (UAttackData* Attack : AttackList)
	{
		if (Attack->Hand == Hand)
		{
			// choose hook if charge long enough
			if (ChargeTime >= Attack->MinChargeTime && Attack->AttackType == EPlayerAttackType::Hook)
				return Attack;

			// choose jab if light press
			if (ChargeTime < Attack->MinChargeTime && Attack->AttackType == EPlayerAttackType::Jab)
				return Attack;
		}
	}
	return nullptr;
}

void UCombatComponent::PlayAttackMontage(UAnimMontage* Montage)
{
	if (!Montage || !ArmsMesh) return;
	UAnimInstance* AnimInstance = ArmsMesh->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
	}
}

// Wrapper for Data Asset convenience
void UCombatComponent::PlayAttackMontage(UAttackData* AttackData)
{
	// The previous code used AttackData->Montage. 
		// Ensure this matches the property name in AttackData.h
	if (AttackData && AttackData->AttackMontage)
	{
		PlayAttackMontage(AttackData->AttackMontage);
	}
	else if (AttackData)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackData found for %s, but AttackMontage is NULL!"), *AttackData->GetName());
	}
}

void UCombatComponent::PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if (!ShakeClass) return;

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->ClientStartCameraShake(ShakeClass);
		}
	}
}

void UCombatComponent::ExecuteMeleeTrace() {
	if (!ActiveAttackData) return;

	AActor* Owner = GetOwner();
	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC || !Owner) return;

	// Creates a starting vector and traces a line from the camera to what the player is looking at (in player's attack range)

	// pull camera location and rotation
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// Calculate End point based on current view direction
	FVector End = CameraLocation + (CameraRotation.Vector() * AttackRange);


	// Setup trace parameters
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	FHitResult HitResult;

	// Perform sphere trace using a radius around crosshair
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		CameraLocation,
		End,
		AttackRadius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None, // Set to ForDuration to see the trace, Set to None for production
		HitResult,
		true
	);

	if (bHit) {
		AActor* HitActor = HitResult.GetActor();

		// Pulls the damage from the current attack
		float FinalDamage = ActiveAttackData->Damage;

		if (IsValid(HitActor))
		{
				UGameplayStatics::ApplyDamage(
				HitResult.GetActor(),
				FinalDamage,
				GetOwner()->GetInstigatorController(),
				GetOwner(),
				UDamageType::StaticClass()
			);

			if (IsValid(HitActor))
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit %s with %f damage!"),
					*HitResult.GetActor()->GetName(), FinalDamage);
			}
		}
	}

	ActiveAttackData = nullptr;


}
