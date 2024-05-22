// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BlasterPlayerController.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, FVector normalImpulse, const FHitResult& hit)
{
	ACharacter* ownerCharacter = Cast<ACharacter>(GetOwner());


	if (ownerCharacter)
	{
		ownerController = ownerCharacter->Controller;
		if (ownerController)
		{
			UGameplayStatics::ApplyDamage(otherActor, damage, ownerController, this, UDamageType::StaticClass());
			ABlasterCharacter* otherCharacter = Cast<ABlasterCharacter>(otherActor);
			if (otherCharacter)
			{
				DisplayHitmarker();
			}
		}
	}

	//call last or else bullet will be destroyed before code runs
	Super::OnHit(hitComponent, otherActor, otherComponent, normalImpulse, hit);
}

void AProjectileBullet::DisplayHitmarker()
{
	if (ownerController->HasAuthority())
	{
		ownerPlayerController = Cast<ABlasterPlayerController>(ownerController);
		if (ownerPlayerController)
		{
			ownerPlayerController->HandleHitmarker();
		}
	}
}

