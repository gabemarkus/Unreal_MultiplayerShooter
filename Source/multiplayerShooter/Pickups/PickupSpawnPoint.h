// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	

	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APickup> pickupClass;

	UPROPERTY()
	APickup* spawnedPickup;

	void SpawnPickup();

	void SpawnTimerFinished();

	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);

private:
	
	FTimerHandle spawnTimer;

	UPROPERTY(EditAnywhere)
	float spawnTime = 1.f;

public:	

};
