// Fill out your copyright notice in the Description page of Project Settings.


#include "TFF/Player/Martin.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values when unreal starts
AMartin::AMartin()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0, 0, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

// Called when the game starts or when spawned
void AMartin::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (IMC_Player)
			{
				Subsystem->AddMappingContext(IMC_Player, 0);
			}
		}
	}

}


// Called to bind functionality to input
void AMartin::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMartin::Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMartin::Look);
		EIC->BindAction(IA_JumpAction, ETriggerEvent::Started, this, &AMartin::StartJump);
		EIC->BindAction(IA_JumpAction, ETriggerEvent::Completed, this, &AMartin::StopJump);

		// Combat
		EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Started, this, &AMartin::PrimaryAttackStarted);
		EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Completed, this, &AMartin::PrimaryAttackReleased);

		EIC->BindAction(IA_SecondaryAttack, ETriggerEvent::Started, this, &AMartin::SecondaryAttackStarted);
		EIC->BindAction(IA_SecondaryAttack, ETriggerEvent::Completed, this, &AMartin::SecondaryAttackReleased);

		EIC->BindAction(IA_Block, ETriggerEvent::Started, this, &AMartin::BlockStarted);
		EIC->BindAction(IA_Block, ETriggerEvent::Completed, this, &AMartin::BlockReleased);
	}
	
}

void AMartin::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

// Movement Functions
void AMartin::Look(const FInputActionValue& Value)
{
	FVector2D LookAxis = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(-LookAxis.Y);
}

void AMartin::StartJump()
{
	Jump();
}

void AMartin::StopJump()
{
	StopJumping();
}

float AMartin::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{

	EPlayerBlockStatus CurrentStatus = CombatComponent->GetCurrentBlockStatus();

	if (CurrentStatus == EPlayerBlockStatus::Parrying)
	{
		// Perfect Parry: Nullify damage and trigger parry animation
		CombatComponent->HandleSuccessfulParry();
		return 0.0f; // Take no damage
	}
	else if (CurrentStatus == EPlayerBlockStatus::Blocking)
	{
		// Standard Block: Mitigate damage (50% reduction)
		DamageAmount *= 0.5f;
	}

	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - DamageToApply, 0.0f, MaxHealth);
	
	UCameraComponent* PlayerCam = FindComponentByClass<UCameraComponent>();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Makes Camera shake on hit, Hit CameraShakeClass BP set in editor
		if (HitCameraShakeClass)
		{
			PC->ClientStartCameraShake(HitCameraShakeClass);
		}


		// Makes Screen flash red with some lowered saturation
		if (PlayerCam) 
		{
			PlayerCam->PostProcessSettings.bOverride_SceneColorTint = true;
			PlayerCam->PostProcessSettings.SceneColorTint = FLinearColor::Red;
			PlayerCam->PostProcessSettings.bOverride_ColorSaturation = true;
			PlayerCam->PostProcessSettings.ColorSaturation = FVector4(1.0f, 0.2f, 0.2f, 1.0f);

			FTimerHandle CameraTintTimerHandle;

			// Set timer to clear the effect after 0.15s
			GetWorldTimerManager().SetTimer(
				CameraTintTimerHandle,
				[this, PlayerCam]()
				{
					PlayerCam->PostProcessSettings.bOverride_SceneColorTint = false;
					PlayerCam->PostProcessSettings.bOverride_ColorSaturation = false;
				},
				0.15f,
				false
			);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Player got hit, now has %f health"), Health);

	if (Health <= 0.f) {
		HandleDeath();
	}
	return DamageToApply;
}

void AMartin::HandleDeath() 
{

}

// Combat Functions
void AMartin::PrimaryAttackStarted(const FInputActionValue&)
{
	CombatComponent->StartAttack(EPlayerAttackHand::Right);
}

void AMartin::PrimaryAttackReleased(const FInputActionValue&)
{
	// Only release if we were actually charging
	if (CombatComponent->GetbRightCharging()) {
		CombatComponent->ReleaseAttack(EPlayerAttackHand::Right);
	}
}

void AMartin::SecondaryAttackStarted(const FInputActionValue&)
{
	CombatComponent->StartAttack(EPlayerAttackHand::Left);
}

void AMartin::SecondaryAttackReleased(const FInputActionValue&)
{
	// Only release if we were actually charging
	if (CombatComponent->GetbLeftCharging()) {
		CombatComponent->ReleaseAttack(EPlayerAttackHand::Left);
	}
}

void AMartin::BlockStarted(const FInputActionValue& Value)
{
	CombatComponent->StartBlock();
}

void AMartin::BlockReleased(const FInputActionValue& Value)
{
	CombatComponent->ReleaseBlock();
}



