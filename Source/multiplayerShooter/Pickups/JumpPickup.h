// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AJumpPickup : public APickup
{
	GENERATED_BODY()

protected:

	virtual void OnSphereOverlap(
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComp,
		int32 otherBodyIndex,
		bool bFromSweep,
		const FHitResult& sweepResult);

private:

	UPROPERTY(EditAnywhere)
	float jumpVelocityBuff = 4000.f;

	UPROPERTY(EditAnywhere)
	float jumpBuffTime = 10.f;
};
