#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SkeletonAttackNotify.generated.h"

// Define the two types of notifies you requested
UENUM(BlueprintType)
enum class EAttackNotifyType : uint8
{
	AttackHit    UMETA(DisplayName = "AttackHit"),
	AttackReset  UMETA(DisplayName = "AttackReset")
};

UCLASS()
class TFF_API USkeletonAttackNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	// This creates the dropdown menu in the Montage editor
	UPROPERTY(EditAnywhere, Category = "Notify Settings")
	EAttackNotifyType NotifyType;

	// This is the core function that triggers during the animation
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};