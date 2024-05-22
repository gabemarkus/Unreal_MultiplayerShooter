// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnTimer((AActor*)nullptr);
}

void APickupSpawnPoint::SpawnPickup()
{
	spawnedPickup = GetWorld()->SpawnActor<APickup>(pickupClass, GetActorTransform());

	if (HasAuthority() && spawnedPickup)
	{
		spawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnTimer);
	}
	
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	GetWorldTimerManager().SetTimer(spawnTimer, this, &APickupSpawnPoint::SpawnTimerFinished, spawnTime);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}
