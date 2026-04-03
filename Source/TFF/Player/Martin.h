// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h" 
#include "CombatComponent.h"
#include "Martin.generated.h"

class UInputMappingContext;
class UInputAction;
class UCombatComponent;

UCLASS()
class TFF_API AMartin : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMartin();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess="true"))
	class UCameraComponent* FirstPersonCamera;

	// Combat Component
	UPROPERTY(VisibleAnywhere)
	UCombatComponent* CombatComponent;


	// Input handlers
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();

	// Combat handlers
	void PrimaryAttackStarted(const FInputActionValue& Value); // Called when left mouse is pressed
	void PrimaryAttackReleased(const FInputActionValue& Value); // Called when left mouse is released

	void SecondaryAttackStarted(const FInputActionValue& Value); // Called when right mouse is pressed
	void SecondaryAttackReleased(const FInputActionValue& Value); // Called when right mouse is released

	void BlockStarted(const FInputActionValue& Value);
	void BlockReleased(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.0f;


public:	
	// Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_JumpAction;

	// Input for attacks
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_PrimaryAttack;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_SecondaryAttack;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_Block;


	// Input Mapping Contexts
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* IMC_Player;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;
	
private:
	void HandleDeath();
};
