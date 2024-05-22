// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
	
protected: 

	virtual void OnHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, FVector normalImpulse, const FHitResult& hit) override;

	void DisplayHitmarker();

private:

	class AController* ownerController;
	class ABlasterPlayerController* ownerPlayerController;
	bool otherActorIsPlayerControlled;
};

