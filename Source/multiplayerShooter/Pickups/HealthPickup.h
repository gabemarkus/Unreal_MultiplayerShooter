// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
	
public:

	AHealthPickup();
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;

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
	float healAmount = 100.0f;

	UPROPERTY(EditAnywhere)
	float healTime = 5.f;
};
