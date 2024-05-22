// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerShooter/Weapon/Weapon.h"
#include "multiplayerShooter/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "multiplayerShooter/BlasterAnimInstance.h"
#include "multiplayerShooter/BlasterComponents/BlasterPlayerController.h"
#include "multiplayerShooter/GameModes/BlasterGameMode.h"
#include "TimerManager.h"
#include "particles/ParticleSystemComponent.h"
#include "particles/ParticleSystem.h"
#include "multiplayerShooter/PlayerState/BlasterPlayerState.h"
#include "multiplayerShooter/Weapon/WeaponTypes.h"
#include "multiplayerShooter/BlasterComponents/BuffComponent.h"
#include "multiplayerShooter/Audio/BlasterAudioHandler.h"
#include "AkAudioEvent.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	blasterAudioHandler = CreateDefaultSubobject<ABlasterAudioHandler>(TEXT("BlasterAudioHandler"));
	blasterAudioHandler->SetOwner(this);


	//create components
	cameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	cameraSpringArm->SetupAttachment(GetMesh());
	cameraSpringArm->TargetArmLength = 600.f;
	cameraSpringArm->bUsePawnControlRotation = true;

	camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	camera->SetupAttachment(cameraSpringArm, USpringArmComponent::SocketName);
	camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	combatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	combatComponent->SetIsReplicated(true);
	combatComponent->SetIsReplicated(true);

	buffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	buffComponent->SetIsReplicated(true);

	//display name
	overheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	overheadWidget->SetupAttachment(RootComponent);

	//force can crouch
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//don't collide camera with player
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	
	turningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//replicated variables
	DOREPLIFETIME_CONDITION(ABlasterCharacter, overlappedWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, health);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//bind controls
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipKeyPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchKeyPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ABlasterCharacter::CrouchKeyReleased);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimKeyPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimKeyReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("SelfDamage", IE_Pressed, this, &ABlasterCharacter::SelfDamageButtonPressed);
	PlayerInputComponent->BindAction("RestartGame", IE_Pressed, this, &ABlasterCharacter::RestartButtonPressed);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
}


void ABlasterCharacter::PostInitializeComponents() //similar to beginplay but only called when all components are loaded
{
	Super::PostInitializeComponents();
	if (combatComponent)
	{
		combatComponent->character = this;
	}

	if (buffComponent)
	{
		buffComponent->character = this;
		buffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
}

void ABlasterCharacter::PollInit()
{
	if (playerState == nullptr)
	{
		playerState = GetPlayerState<ABlasterPlayerState>();
		if (playerState)
		{
			playerState->AddToScore(0.f);
			playerState->AddToDefeats(0);
		}
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	//overriding unreal mp movement replication to include turning for proxies
	Super::OnRep_ReplicatedMovement();

	ProxiesTurnInPlace();
	timeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHudHealth();

	if (HasAuthority())
	{
		//bind inherited damage function to a custom one
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffsetTick(DeltaTime);
	HideCharacterFromCamera();
	PollInit();

}

void ABlasterCharacter::AimOffsetTick(float DeltaTime) //calculates character aiming direction for animation
{

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		//if it's been too long since movement was replicated, do so
		timeSinceLastMovementReplication += DeltaTime;

		if (timeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAimOffsetPitch();
	}
}

//Keybinds

void ABlasterCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value != 0.f) 
	{
		const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X)); //get forward vector
		AddMovementInput(direction, value);
	}
}

void ABlasterCharacter::MoveRight(float value)
{
	const FRotator yawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector direction(FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y)); //get sideways vector
	AddMovementInput(direction, value);
}

void ABlasterCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void ABlasterCharacter::LookUp(float value)
{
	AddControllerPitchInput(value * -1);
}

void ABlasterCharacter::EquipKeyPressed()
{
	if (combatComponent)
	{
		if (HasAuthority())
		{
			combatComponent->EquipWeapon(overlappedWeapon);
		}
		else 
		{
			ServerEquipKeyPressed();
		}
	}
}

void ABlasterCharacter::ServerEquipKeyPressed_Implementation()
{
	if (combatComponent)
	{
		combatComponent->EquipWeapon(overlappedWeapon);
	}
}

void ABlasterCharacter::Jump()
{
	Super::Jump();

	if (!inAir) //sound plays once per jump
	{
		UGameplayStatics::PlaySoundAtLocation(this, jumpSound, GetActorLocation());
	}

	if (bIsCrouched) //crouch-jump
	{
		Crouch();
	}

	inAir = true;
}

void ABlasterCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	UGameplayStatics::PlaySoundAtLocation(this, landingSound, GetActorLocation());
	inAir = false;
}

void ABlasterCharacter::FireButtonPressed()
{
	if (combatComponent)
	{
		combatComponent->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	combatComponent->FireButtonPressed(false);
}

void ABlasterCharacter::ReloadButtonPressed()
{
	combatComponent->Reload();
}

void ABlasterCharacter::RestartButtonPressed()
{
	if (HasAuthority())
	{
		playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : playerController = playerController;

		if (playerController)
		{
			playerController->SetHUDDebugMessages();

			ABlasterGameMode* gameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

			if (gameMode)
			{
				gameMode->RestartLevel();
			}
		}
	}
}

void ABlasterCharacter::SelfDamageButtonPressed() //for testing
{
	health -= 20;
}

void ABlasterCharacter::CrouchKeyPressed()
{
	Crouch();
}

void ABlasterCharacter::CrouchKeyReleased()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ABlasterCharacter::AimKeyPressed()
{
	if (combatComponent)
	{
		combatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimKeyReleased()
{
	if (combatComponent)
	{
		combatComponent->SetAiming(false);
	}
}

//

void ABlasterCharacter::HideCharacterFromCamera()
{
	if (IsLocallyControlled() && !eliminated)
	{
		const bool bCameraTooClose = (camera->GetComponentLocation() - GetActorLocation()).Size() < hideCharacterThreshold;
		GetMesh()->SetIsReplicated(!bCameraTooClose);
		GetMesh()->SetVisibility(!bCameraTooClose);
		if (combatComponent->equippedWeapon)
		{
			combatComponent->equippedWeapon->GetWeaponMesh()->bOwnerNoSee = bCameraTooClose;
		}
	}
}

void ABlasterCharacter::SetOverlappedWeapon(AWeapon* weapon)
{
	if (IsLocallyControlled())
	{
		if (overlappedWeapon)
		{
			overlappedWeapon->ShowPickupWidget(false);
		}
	}

	overlappedWeapon = weapon;

	if (IsLocallyControlled())
	{
		if (overlappedWeapon)
		{
			overlappedWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappedWeapon(AWeapon* lastWeapon)
{
	if (overlappedWeapon)
	{
		overlappedWeapon->ShowPickupWidget(true);
	}
	if (lastWeapon)
	{
		lastWeapon->ShowPickupWidget(false);
	}
}

//calculate aim offset for anim and turning in place
void ABlasterCharacter::AimOffset(float deltaTime)
{
	if (combatComponent && combatComponent->equippedWeapon == nullptr) return;

	float speed = CalculateSpeed();
	bool isInAir = GetCharacterMovement()->IsFalling();

	if (speed == 0 && !isInAir) //not moving or jumping
	{
		rotateRootBone = true;
		FRotator currentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, startingAimRotation);
		aimOffsetYaw = deltaAimRotation.Yaw;
		
		if (turningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			interpAimOffsetYaw = aimOffsetYaw;
		}

		bUseControllerRotationYaw = true;
		
		TurnInPlace(deltaTime);
	}

	if (speed > 0 || isInAir) // moving or jumping
	{
		rotateRootBone = false;
		startingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		aimOffsetYaw = 0.f;
		bUseControllerRotationYaw = true;
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAimOffsetPitch();
}

void ABlasterCharacter::CalculateAimOffsetPitch()
{
	aimOffsetPitch = GetBaseAimRotation().Pitch;
	if (aimOffsetPitch > 90.f && !IsLocallyControlled()) //correction for server changing angles - 270-360 to -90-0
	{
		FVector2D inRange(270.f, 360.f);
		FVector2d outRange(-90.f, 0.f);
		aimOffsetPitch = FMath::GetMappedRangeValueClamped(inRange, outRange, aimOffsetPitch);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector velocity = GetVelocity();
	velocity.Z = 0.f; //not interested in Z velocity
	return velocity.Size(); //magnitude of velocity vector
}

void ABlasterCharacter::ProxiesTurnInPlace() //used in animBP transition rules
{
	if (combatComponent == nullptr || combatComponent->equippedWeapon == nullptr) return;

	rotateRootBone = false;

	if (CalculateSpeed() > 0.f) 
	{
		turningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	proxyRotationLastFrame = currentProxyRotation;
	currentProxyRotation = GetActorRotation();
	float proxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(currentProxyRotation, proxyRotationLastFrame).Yaw;

	if (FMath::Abs(proxyYaw) > turnThreshold)
	{
		if (proxyYaw > turnThreshold) 
		{
			turningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (proxyYaw < -turnThreshold)
		{
			turningInPlace = ETurningInPlace::ETIP_Left;
		}
		else 
		{
			turningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	turningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::TurnInPlace(float DeltaTime) //used in animBP transition rules
{
	if (aimOffsetYaw > 90.f)
	{
		turningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (aimOffsetYaw < -90.f)
	{
		turningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (turningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		interpAimOffsetYaw = FMath::FInterpTo(interpAimOffsetYaw, 0.f, DeltaTime, 10.f);
		aimOffsetYaw = interpAimOffsetYaw;
		if (FMath::Abs(aimOffsetYaw) < 15.f)
		{
			turningInPlace = ETurningInPlace::ETIP_NotTurning;
			startingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool isAiming) //firing animation
{
	if (combatComponent == nullptr || combatComponent->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && fireWeaponMontage)
	{
		animInstance->Montage_Play(fireWeaponMontage);
		FName montageSectionName;
		montageSectionName = isAiming ? FName("RifleAim") : FName("RifleHip");
		animInstance->Montage_JumpToSection(montageSectionName);
	}
}

//Health and damage

void ABlasterCharacter::ReceiveDamage(AActor* damagedActor, float damage, const UDamageType* damageType, AController* instigatorController, AActor* damageCauser)
{
	if (instigatorController == playerController) return; //no self damage

	health = FMath::Clamp(health - damage, 0.f, maxHealth);

	UpdateHudHealth();
	PlayHitReactMontage(GetIsAiming());

	if (health <= 0.f)
	{
		//call PlayerEliminated in game mode
		ABlasterGameMode* blasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (blasterGameMode)
		{
			playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : playerController;
			ABlasterPlayerController* attackerController = Cast<ABlasterPlayerController>(instigatorController);
			blasterGameMode->PlayerEliminated(this, playerController, attackerController);
		}
	}
}

void ABlasterCharacter::OnRep_Health(float lastHealth)
{
	UpdateHudHealth();
	if (health < lastHealth)
	{
		PlayHitReactMontage(GetIsAiming());
	}
}

void ABlasterCharacter::UpdateHudHealth()
{
	playerController = playerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : playerController = playerController;

	if (playerController)
	{
		playerController->SetHUDHealth(health, maxHealth);
	}
}

void ABlasterCharacter::PlayHitReactMontage(bool isAiming)
{
	if (eliminated || combatComponent == nullptr || combatComponent->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && hitReactMontage && !animInstance->IsAnyMontagePlaying())
	{
		animInstance->Montage_Play(hitReactMontage);
		FName montageSectionName;
		montageSectionName = isAiming ? FName("RifleAim") : FName("RifleHip");
		animInstance->Montage_JumpToSection(montageSectionName);
	}
}

void ABlasterCharacter::PlayEliminatedMontage()
{
	UE_LOG(LogTemp, Warning, TEXT("EliminatedMontageCalled"));
	if (combatComponent == nullptr || combatComponent->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && eliminatedMontage)
	{
		animInstance->Montage_Play(eliminatedMontage);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (combatComponent == nullptr || combatComponent->equippedWeapon == nullptr) return;

	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && reloadMontage)
	{
		animInstance->Montage_Play(reloadMontage);
		FName montageSectionName;

		switch (combatComponent->equippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			montageSectionName = FName("Rifle");
			break;
		}

		animInstance->Montage_JumpToSection(montageSectionName);
	}
}

void ABlasterCharacter::Eliminated()
{
	//drop gun
	if (combatComponent && combatComponent->equippedWeapon)
	{
		combatComponent->equippedWeapon->Dropped();
	}

	MulticastEliminated();

	//timer before explosion
	GetWorldTimerManager().SetTimer(eliminatedTimer, this, &ABlasterCharacter::EliminatedTimerEnd, respawnDelay);
}

void ABlasterCharacter::MulticastEliminated_Implementation()
{
	if (playerController)
	{
		playerController->SetHUDWeaponAmmo(0); //weapon dropped so no weapon ammo to display
	}

	eliminated = true;
	//disable physics for ragdoll
	GetCapsuleComponent()->Deactivate();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->AddImpulse(FVector(0, 0, 250000)); //launch!

	//disable movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (playerController)
	{
		DisableInput(playerController);
	}
}

void ABlasterCharacter::EliminatedTimerEnd()
{
	Explode();
	//respawn timer
	GetWorldTimerManager().SetTimer(requestRespawnTimer, this, &ABlasterCharacter::RequestRespawn, 1.5f);
}

void ABlasterCharacter::Explode_Implementation()
{
	GetMesh()->SetVisibility(false);
	GetMesh()->SetEnableGravity(false);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), explosion, GetMesh()->GetRelativeTransform());
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), explodeSound, GetActorLocation());
}

void ABlasterCharacter::RequestRespawn()
{
	ABlasterGameMode* blasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	blasterGameMode->RequestRespawn(this, playerController);
}

void ABlasterCharacter::PlayFootstepSound()
{
}

void ABlasterCharacter::PlayMPFootstepSound()
{
}

//getters
bool ABlasterCharacter::GetHasWeaponEquipped()
{
	return (combatComponent && combatComponent->equippedWeapon);
}

bool ABlasterCharacter::GetIsAiming()
{
	return (combatComponent && combatComponent->isAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (combatComponent == nullptr) return nullptr;
	return combatComponent->equippedWeapon;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (combatComponent == nullptr) return ECombatState::ECS_MAX;
	
	return combatComponent->combatState;
}
