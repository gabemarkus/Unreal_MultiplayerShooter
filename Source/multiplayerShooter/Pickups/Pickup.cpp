// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	//setup components
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	overlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	overlapSphere->SetupAttachment(RootComponent);
	overlapSphere->SetSphereRadius(60.f);
	overlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	overlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	overlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	pickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	pickupMesh->SetupAttachment(overlapSphere);
	pickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	//start timer for pickup spawn
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(bindOverlapTimer, this, &APickup::BindOverlapTimerFinished, bindOverlapTime);
	}
}

//for child classes
void APickup::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{

}

void APickup::Rotate()
{
	FRotator newRotation = FRotator(0, -.6f, 0);
	FQuat quatRotation = FQuat(newRotation);
	AddActorLocalRotation(quatRotation);
}

void APickup::BindOverlapTimerFinished()
{
	overlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Rotate();
}

void APickup::Destroyed()
{
	Super::Destroyed();

	if (pickupSound)
	{
		UGameplayStatics::PlaySound2D(this, pickupSound);
	}
}

