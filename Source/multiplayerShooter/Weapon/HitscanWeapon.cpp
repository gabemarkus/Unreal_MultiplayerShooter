// Fill out your copyright notice in the Description page of Project Settings.


#include "HitscanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "Particles/ParticleSystemComponent.h"
#include "multiplayerShooter/BlasterComponents/BlasterPlayerController.h"


void AHitscanWeapon::Fire(const FVector& hitTarget)
{
	Super::Fire(hitTarget);

	APawn* owner = Cast<APawn>(GetOwner());
	if (owner == nullptr) return;

	AController* instigatorController = owner->GetController();
	ABlasterCharacter* player = Cast<ABlasterCharacter>(owner);
	if (player)
	{
		UCameraComponent* camera = player->GetCamera();
		if (camera)
		{
			cameraLocation = camera->GetComponentLocation();
		}
	}

	const USkeletalMeshSocket* muzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (muzzleFlashSocket && instigatorController)
	{
		
		//line trace from camera
		FVector ltStart = cameraLocation;
		FVector ltEnd = ltStart + (hitTarget - ltStart) * 1.01f;
		FHitResult ltHit;

		//lt from weapon for trail
		socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector beamStart = socketTransform.GetLocation();
		FVector beamEnd = beamStart + (hitTarget - beamStart) * 1.01f;
		FHitResult beamHit;


		UWorld* world = GetWorld();
		if (world)
		{
			FCollisionQueryParams params;
			params.AddIgnoredActor(player);
		
			//lt from camera
			world->LineTraceSingleByChannel(
				ltHit,
				ltStart,
				ltEnd,
				ECollisionChannel::ECC_Visibility,
				params
			);

			//lt from weapon for trail
			world->LineTraceSingleByChannel(
				beamHit,
				beamStart,
				beamEnd,
				ECollisionChannel::ECC_Visibility
			);

			if (ltHit.bBlockingHit)
			{
				ABlasterCharacter* character = Cast<ABlasterCharacter>(ltHit.GetActor());
				if (character)
				{
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
							character,
							damage,
							instigatorController,
							this,
							UDamageType::StaticClass()
						);

						playerController = Cast<ABlasterPlayerController>(instigatorController);

						if (playerController)
						{
							playerController->HandleHitmarker();
						}
					}
				}
			}

			MulticastFX(ltHit.ImpactPoint, beamHit.ImpactPoint, ltHit.bBlockingHit);
		}
	}
}

void AHitscanWeapon::MulticastFX_Implementation(FVector_NetQuantize HitLocation, FVector_NetQuantize beamHitLocation, bool blockingHit)
{
	if (blockingHit)
	{
		if (impactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				impactParticles,
				HitLocation,
				HitLocation.Rotation()
			);
		}
	}

	if (beamParticles)
	{
		UParticleSystemComponent* beam = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			beamParticles,
			socketTransform
		);

		if (beam)
		{
			beam->SetVectorParameter(FName("Target"), HitLocation);
		}
	}
}


