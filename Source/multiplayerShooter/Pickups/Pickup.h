// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	

	APickup();

	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;


protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComp,
		int32 otherBodyIndex,
		bool bFromSweep,
		const FHitResult& sweepResult);

private:

	UPROPERTY(EditAnywhere)
	class USphereComponent* overlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* pickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* pickupMesh;

	void Rotate();

	FTimerHandle bindOverlapTimer;
	float bindOverlapTime = .25f;
	void BindOverlapTimerFinished();

public:	

};
