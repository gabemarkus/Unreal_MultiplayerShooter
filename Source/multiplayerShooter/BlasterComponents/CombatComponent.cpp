// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "multiplayerShooter/Weapon/Weapon.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/Weapon/BulletCasing.h"
#include "multiplayerShooter/Pickups/AmmoPickup.h"
#include "BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	baseWalkSpeed = 900.f;
	aimWalkSpeed = 400.f;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (character && character->IsLocallyControlled()) 
	{
		//linetrace - in tick to get red crosshair when over enemy
		FHitResult HitResult;
		LineTraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshair(DeltaTime);
		InterpolateFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, equippedWeapon);
	DOREPLIFETIME(UCombatComponent, isAiming);
	DOREPLIFETIME(UCombatComponent, combatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, playerAmmo, COND_OwnerOnly); //replicate to client only
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimaryComponentTick.Target = this;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	PrimaryComponentTick.RegisterTickFunction(GetComponentLevel());

	if(character) 
	{
		character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed;

		if (character->GetCamera())
		{
			defaultFOV = character->GetCamera()->FieldOfView;
			currentFOV = defaultFOV;
		}

		if (character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

//Firing

void UCombatComponent::FireButtonPressed(bool isPressed)
{
	fireButtonPressed = isPressed;

	if (fireButtonPressed) 
	{
		if (equippedWeapon == nullptr) return;

		Fire();
	}
}

bool UCombatComponent::CheckCanFire()
{
	if (equippedWeapon == nullptr) return false;
	return !equippedWeapon->IsEmpty() && canFire && combatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::Fire()
{
	if (CheckCanFire())
	{
		canFire = false;

		//fire
		ServerFire(HitTarget);

		//widen crosshair
		if (equippedWeapon)
		{
			crosshairShootingFactor = 1.f;
		}

		StartFireTimer(); //timer for automatic weapons
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	//need to call multicastRPCs from server RPC so it is replicated to all machines
	//if called only on client it would only "replicate" to client
	MulticastFire(traceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	if (character && combatState == ECombatState::ECS_Unoccupied)
	{
		character->PlayFireMontage(isAiming);
		//fire equipped weapon at line trace hit
		equippedWeapon->Fire(traceHitTarget);
	}
}

//automatic weapons auto fire
void UCombatComponent::StartFireTimer()
{
	if (equippedWeapon == nullptr || character == nullptr) return;

	character->GetWorldTimerManager().SetTimer(
		fireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		equippedWeapon->fireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	canFire = true;

	if (fireButtonPressed && equippedWeapon->autoWeapon)
	{
		Fire();
	}
}
//


void UCombatComponent::SetAiming(bool aiming)
{
	isAiming = aiming;

	//slower movement if aiming
	if (character)
	{
		character->GetCharacterMovement()->MaxWalkSpeed = isAiming ? aimWalkSpeed : baseWalkSpeed;
	}

	ServerSetAiming(aiming);
}

void UCombatComponent::ServerSetAiming_Implementation(bool aiming)
{
	isAiming = aiming;
	if (character)
	{
		character->GetCharacterMovement()->MaxWalkSpeed = isAiming ? aimWalkSpeed : baseWalkSpeed;
	}
}

void UCombatComponent::LineTraceUnderCrosshairs(FHitResult& lineTraceHitResult)
{
	//Get center of screen

	FVector2d viewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(viewportSize);
	}

	FVector2D crosshairLocation(viewportSize.X / 2.f, viewportSize.Y / 2.f); 
	FVector crosshairWorldPos;
	FVector crosshairWorldDir;

	bool screenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		crosshairLocation,
		crosshairWorldPos,
		crosshairWorldDir
	);

	//line trace setup

	if (screenToWorld)
	{
		FVector lineTraceStart = crosshairWorldPos;

		// start line trace further from character to avoid unwanted collisions

		if (character)
		{
			float distanceToCharacter = (character->GetActorLocation() - lineTraceStart).Size();
			lineTraceStart += crosshairWorldDir * (distanceToCharacter + 100.f);
		}

		FVector lineTraceEnd = lineTraceStart + crosshairWorldDir * TRACE_LENGTH;

		FCollisionQueryParams traceParameters;
		traceParameters.AddIgnoredActor(character);

		//line trace
		GetWorld()->LineTraceSingleByChannel(lineTraceHitResult,
			lineTraceStart,
			lineTraceEnd,
			ECollisionChannel::ECC_Visibility,
			traceParameters
		);

		//red crosshair if aiming over enemy
		if (lineTraceHitResult.GetActor() && lineTraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDpackage.crosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDpackage.crosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshair(float deltaTime)
{
	if (character == nullptr || character->Controller == nullptr) return;

	playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : playerController; 	//avoids casting every frame

	//enable crosshair hud if weapon is equipped
	if (playerController)
	{
		hud = hud == nullptr ? Cast<ABlasterHUD>(playerController->GetHUD()) : hud;
		if (hud)
		{
			if (equippedWeapon)
			{
				HUDpackage.crosshairCenter = equippedWeapon->crosshairCenter;
				HUDpackage.crosshairRight = equippedWeapon->crosshairRight;
				HUDpackage.crosshairLeft = equippedWeapon->crosshairLeft;
				HUDpackage.crosshairTop = equippedWeapon->crosshairTop;
				HUDpackage.crosshairBottom = equippedWeapon->crosshairBottom;
			}
			else
			{
				HUDpackage.crosshairCenter = nullptr;
				HUDpackage.crosshairRight = nullptr;
				HUDpackage.crosshairLeft = nullptr;
				HUDpackage.crosshairTop = nullptr;
				HUDpackage.crosshairBottom = nullptr;
			}

			//crosshair spread changes based on movement / actions

			FVector2D walkSpeedRange(0.f, character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D velocityMultiplierRange(0.f, 1.f);
			FVector velocity = 3 * character->GetVelocity();

			crosshairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplierRange, velocity.Size());

			if (character->GetCharacterMovement()->IsFalling()) //falling
			{
				crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 1.3f, deltaTime, 10.f);
			}
			else
			{
				crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 0.f, deltaTime, 10.f);
			}

			if (character->GetCharacterMovement()->IsCrouching()) //crouching
			{
				crosshairCrouchFactor = FMath::FInterpTo(crosshairCrouchFactor, -.4f, deltaTime, 5.f);
			}
			else
			{
				crosshairCrouchFactor = FMath::FInterpTo(crosshairCrouchFactor, 0.f, deltaTime, 5.f);
			}

			if (isAiming) //aiming down sights
			{
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, .3f, deltaTime, 30.f);
			}
			else
			{
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, 0.f, deltaTime, 30.f);
			}

			crosshairShootingFactor = FMath::FInterpTo(crosshairShootingFactor, 0.f, deltaTime, 30.f);

			HUDpackage.crosshairSpread =
				.8f +
				crosshairVelocityFactor +
				crosshairInAirFactor +
				crosshairCrouchFactor -
				crosshairAimFactor;

			hud->SetHUDPackage(HUDpackage);
		}
	}
}

void UCombatComponent::InterpolateFOV(float deltaTime)
{
	if (equippedWeapon == nullptr) return;

	// interpolate FOV to weapon-set FOV (zoom)

	if (isAiming)
	{
		currentFOV = FMath::FInterpTo(currentFOV, equippedWeapon->GetZoomedFOV(), deltaTime, equippedWeapon->GetZoomSpeed());
	}
	else
	{
		currentFOV = FMath::FInterpTo(currentFOV, defaultFOV, deltaTime, zoomSpeed);
	}

	if (character && character->GetCamera())
	{
		character->GetCamera()->SetFieldOfView(currentFOV);
	}
}

//Ammo and reloading

void UCombatComponent::InitializeCarriedAmmo()
{
	carriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, startingARAmmo);
}

void UCombatComponent::Reload() //called in blastercharacter
{
	if (playerAmmo > 0 && combatState != ECombatState::ECS_Reloading && equippedWeapon && equippedWeapon->GetAmmo() < equippedWeapon->GetWeaponCapacity() && !islocallyreloading)
	{
		ServerReload();
		HandleReload();
		islocallyreloading = true;
	}
} 

void UCombatComponent::ServerReload_Implementation()
{
	if (character == nullptr || equippedWeapon == nullptr) return;

	combatState = ECombatState::ECS_Reloading;
	if(!character->IsLocallyControlled()) HandleReload();

}

void UCombatComponent::HandleReload()
{
	character->PlayReloadMontage();
	UGameplayStatics::PlaySoundAtLocation(this, reloadSound, character->GetActorLocation());
}

int32 UCombatComponent::AmountToReload()
{
	if (equippedWeapon == nullptr) return 0;

	int32 roomInMag = equippedWeapon->GetWeaponCapacity() - equippedWeapon->GetAmmo();

	if (carriedAmmoMap.Contains(equippedWeapon->GetWeaponType()))
	{
		int32 amountCarried = carriedAmmoMap[equippedWeapon->GetWeaponType()];
		int32 least = FMath::Min(roomInMag, amountCarried);
		return FMath::Clamp(roomInMag, 0, least); //return smallest between room left in mag and ammo carried
	}
	return 0;
}

void UCombatComponent::UpdateAmmoValues()
{
	if (equippedWeapon == nullptr) return;

	int32 reloadAmount = AmountToReload();

	if (carriedAmmoMap.Contains(equippedWeapon->GetWeaponType()))
	{
		playerAmmo = carriedAmmoMap[equippedWeapon->GetWeaponType()] -= reloadAmount; //subtract amount to reload from carried ammo
	}

	playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : playerController;
	if (playerController)
	{
		playerController->SetHUDPlayerAmmo(playerAmmo); //update hud
	}
	equippedWeapon->AddAmmo(-reloadAmount); //reload weapon
}

void UCombatComponent::FinishReloading() //called in anim blueprint to determine when player can fire again
{
	if (character == nullptr) return;
	islocallyreloading = false;
	if (character->HasAuthority())
	{
		combatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (fireButtonPressed) //fire if holding fire key when reload anim finished
	{
		Fire();
	}
}

void UCombatComponent::PickupAmmo(int32 ammoAmount)
{
	carriedAmmoMap[EWeaponType::EWT_AssaultRifle] += ammoAmount;
	UpdateAmmoValues();
}

void UCombatComponent::OnRep_PlayerAmmo()
{
	playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : playerController;
	if (playerController)
	{
		playerController->SetHUDPlayerAmmo(playerAmmo);
	}
}

void UCombatComponent::SpawnClip() //spawn clip mesh when reloading
{
	UWorld* world = GetWorld();
	FActorSpawnParameters spawnParameters;

	world->SpawnActor<ABulletCasing>(
		droppedClip,
		equippedWeapon->GetActorLocation(),
		equippedWeapon->GetActorRotation(),
		spawnParameters);
}

//Equipping weapon

void UCombatComponent::EquipWeapon(AWeapon* weaponToEquip)
{
	if (character == nullptr || weaponToEquip == nullptr) return;

	if (equippedWeapon) //drop currently equipped weapon
	{
		equippedWeapon->Dropped();
	}

	equippedWeapon = weaponToEquip;
	equippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* handSocket = character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	handSocket->AttachActor(equippedWeapon, character->GetMesh()); //attach new weapon to hand

	equippedWeapon->SetOwner(character);
	equippedWeapon->SetHUDAmmo();

	if(carriedAmmoMap.Contains(equippedWeapon->GetWeaponType()))
	{
		playerAmmo = carriedAmmoMap[equippedWeapon->GetWeaponType()];
	}

	playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : playerController;
	if (playerController)
	{
		playerController->SetHUDPlayerAmmo(playerAmmo); //set new weapon ammo in HUD
	}

	//change controls
	character->GetCharacterMovement()->bOrientRotationToMovement = false;
	character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	equippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* handSocket = character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	handSocket->AttachActor(equippedWeapon, character->GetMesh());
	character->GetCharacterMovement()->bOrientRotationToMovement = false;
	character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_CombatState()
{
	switch (combatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;

	case ECombatState::ECS_Unoccupied:
		if (fireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

