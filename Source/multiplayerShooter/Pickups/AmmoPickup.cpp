//Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "multiplayerShooter/BlasterComponents/CombatComponent.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"

void AAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	Super::OnSphereOverlap(overlappedComponent, otherActor, otherComp, otherBodyIndex, bFromSweep, sweepResult);

	ABlasterCharacter* character = Cast<ABlasterCharacter>(otherActor);

	if (character && character->GetEquippedWeapon() == nullptr) return; //dont pickup ammo if no weapon equipped

	if (character)
	{
		UCombatComponent* combatComponent = character->GetCombatComponent();
		if (combatComponent)
		{
			combatComponent->PickupAmmo(ammoAmount);
		}
	}

	Destroy();
}
