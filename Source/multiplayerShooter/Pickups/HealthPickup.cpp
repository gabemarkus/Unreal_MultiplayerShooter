// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	Super::OnSphereOverlap(overlappedComponent, otherActor, otherComp, otherBodyIndex, bFromSweep, sweepResult);

	ABlasterCharacter* character = Cast<ABlasterCharacter>(otherActor);
	if (character)
	{
		UBuffComponent* buffComponent = character->GetBuff();
		if (buffComponent)
		{
			//pass heal parameters to buff component and start healing
			buffComponent->HealParams(healAmount, healTime);
		}
	}

	Destroy();
}

void AHealthPickup::Destroyed()
{
	Super::Destroyed();
}

void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
