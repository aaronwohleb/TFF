// Fill out your copyright notice in the Description page of Project Settings.


#include "Dummy.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ADummy::ADummy()
{
	// Doesn't tick
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Ensure the mesh has collision enabled and is set to 'Block' combat traces
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

}

// Called when the game starts or when spawned
void ADummy::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	
}

// Called every frame
float ADummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageToApply, 0.0f, MaxHealth);

	UE_LOG(LogTemp, Warning, TEXT("AHHHH GOT HIT BAD, now i got %f"),Health);

	if (Health <= 0.f) {
		HandleDestruction();
	}

	return DamageToApply;
}

void ADummy::HandleDestruction() {
	Destroy();
}

