// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	Super::OnSphereOverlap(overlappedComponent, otherActor, otherComp, otherBodyIndex, bFromSweep, sweepResult);

	ABlasterCharacter* character = Cast<ABlasterCharacter>(otherActor);
	
	if (character)
	{
		UBuffComponent* buffComponent = character->GetBuff();
		if (buffComponent && !buffComponent->hasJumpBuff)
		{
			buffComponent->HandleJumpBuff(jumpVelocityBuff, jumpBuffTime);	
			Destroy();
		}
	}
}
