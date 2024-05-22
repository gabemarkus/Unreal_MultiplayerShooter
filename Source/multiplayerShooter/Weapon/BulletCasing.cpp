// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletCasing.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	casingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletCasingMesh"));
	SetRootComponent(casingMesh);
	casingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	casingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	casingMesh->SetSimulatePhysics(true);
	casingMesh->SetEnableGravity(true);
	casingImpulse = 3.f;
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();

	casingMesh->SetNotifyRigidBodyCollision(true);
	casingMesh->OnComponentHit.AddDynamic(this, &ABulletCasing::OnHit); //binding oncomponenthit callback
	casingMesh->AddImpulse(GetActorForwardVector() * casingImpulse);
}

void ABulletCasing::OnHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, FVector normalImpulse, const FHitResult& hit)
{

	if (casingSound)
	{
		if (shrinks)
		{
			UGameplayStatics::PlaySoundAtLocation(this, casingSound, GetActorLocation());
		}
		else
		{
			if (!soundplayed)
			{
				UGameplayStatics::PlaySoundAtLocation(this, casingSound, GetActorLocation(), .5f);
				soundplayed = true;
			}
		}
	}

	//casingMesh->SetNotifyRigidBodyCollision(false);
	if (shrinks)
	{
		FTimerHandle shrinkTimerHandle;
		GetWorldTimerManager().SetTimer(shrinkTimerHandle, this, &ABulletCasing::ShrinkCasing, .6f, false);
	}

	FTimerHandle timerHandle;
	GetWorldTimerManager().SetTimer(timerHandle, this, &ABulletCasing::DestroyCasing, decayTime, false);
}

void ABulletCasing::ShrinkCasing()
{
	FVector newScale;
	newScale.Set(1, 1, 1);
	casingMesh->SetWorldScale3D(newScale);
}

void ABulletCasing::DestroyCasing()
{
	Destroy();
}



