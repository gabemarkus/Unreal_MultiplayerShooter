// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "Kismet/GameplayStatics.h"


void AProjectileWeapon::Fire(const FVector& hitTarget)
{
	Super::Fire(hitTarget);

	if (!HasAuthority()) return;

	APawn* instigatorPawn = Cast<APawn>(GetOwner());

	//spawn projectile at muzzle socket
	const USkeletalMeshSocket* muzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (muzzleFlashSocket)
	{
		FTransform socketTransform = muzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector toTarget = hitTarget - socketTransform.GetLocation();
		FRotator targetRotation = toTarget.Rotation();
		if (projectileClass && instigatorPawn)
		{
			FActorSpawnParameters spawnParameters;
			spawnParameters.Owner = GetOwner();
			spawnParameters.Instigator = instigatorPawn;

			UWorld* world = GetWorld();
			if (world)
			{
				world->SpawnActor<AProjectile>(
					projectileClass,
					socketTransform.GetLocation(),
					targetRotation,
					spawnParameters
				);
			}
		}
	}

}
