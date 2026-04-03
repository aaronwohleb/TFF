#include "SkeletonAttackNotify.h"
#include "SkeletonEnemy.h" 
#include "Components/SkeletalMeshComponent.h"

void USkeletonAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{

	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// Check if the actor playing the animation is our Skeleton
	ASkeletonEnemy* Skeleton = Cast<ASkeletonEnemy>(MeshComp->GetOwner());

	if (Skeleton)
	{
		ESkeletonCombatState CurrentState = Skeleton->GetCombatState();
		// Execute logic based on the dropdown choice in the Montage
		switch (NotifyType)
		{
		case EAttackNotifyType::AttackHit:
			Skeleton->Attack(); // Performs the distance and dot product check
			break;

		case EAttackNotifyType::AttackReset:
			Skeleton->ResetCombatState(CurrentState); // Switches state from Attacking back to CombatStance
			break;
		}
	}
}