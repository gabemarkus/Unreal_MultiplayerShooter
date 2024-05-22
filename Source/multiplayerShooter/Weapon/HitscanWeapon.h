// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitscanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AHitscanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& hitTarget);

private:
	
	UPROPERTY(EditAnywhere)
	float damage = 50.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* impactParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* beamParticles;

	FVector cameraLocation;

	FTransform socketTransform;

	class ABlasterPlayerController* playerController;
protected:

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFX(FVector_NetQuantize HitLocation, FVector_NetQuantize beamHitLocation, bool blockingHit);

};
