// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackData.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class EPlayerBlockStatus : uint8 {
	None,
	Parrying,
	Blocking
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TFF_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	// Old tick implementation for charging attacks
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called by character input
	void StartAttack(EPlayerAttackHand Hand);
	void ReleaseAttack(EPlayerAttackHand Hand);
	void StartBlock();
	void ReleaseBlock();

	// Mesh for playing attack animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (EditInline = "true"))
	USkeletalMeshComponent* ArmsMesh;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	float RightHandCharge = 0.f;
	float LeftHandCharge = 0.f;

	bool bRightCharging = false;
	bool bLeftCharging = false;
	EPlayerBlockStatus CurrentBlockStatus = EPlayerBlockStatus::None;

	UPROPERTY(EditAnywhere)
	TArray<UAttackData*> AttackList;

	// Helper to find specific attack types 
	UAttackData* GetAttackDataByType(EPlayerAttackHand Hand, EPlayerAttackType Type);
	UAttackData* SelectAttack(EPlayerAttackHand Hand, float ChargeTime);

	void PlayAttackMontage(UAttackData* AttackData);
	void PlayAttackMontage(UAnimMontage* Montage);

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ExecuteMeleeTrace();

private:
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 300.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRadius = 20.f;

	FTimerHandle ParryTimerHandle;

	// function to drop the state from Parrying to Blocking
	void DowngradeParryToBlock();

	UPROPERTY()
	UAttackData* ActiveAttackData;

public:
	UAttackData* GetActiveAttackData() const { return ActiveAttackData; }

	float GetbRightCharging() const { return bRightCharging; }
	float GetbLeftCharging() const { return bLeftCharging; }
	EPlayerBlockStatus GetCurrentBlockStatus() const { return CurrentBlockStatus; }

	// Blocking 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animations")
	UAnimMontage* BlockStartMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animations")
	UAnimMontage* ParrySuccessMontage;

	// The duration of the parry window
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
	float ParryWindowDuration = 0.5f;

	// Called by AMartin upon successfully parrying an incoming attack
	void HandleSuccessfulParry();

	void PlayCameraShake(TSubclassOf<class UCameraShakeBase> ShakeClass);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	TSubclassOf<UCameraShakeBase> BlockCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	TSubclassOf<UCameraShakeBase> ParrySuccessCameraShake;

};
