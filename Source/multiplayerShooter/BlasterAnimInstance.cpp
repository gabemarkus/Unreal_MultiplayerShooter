// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "multiplayerShooter\Weapon\Weapon.h"
#include "multiplayerShooter/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	blasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner()); //get owning character
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (blasterCharacter == nullptr)
	{
		blasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (blasterCharacter == nullptr) return;

	FVector velocity = blasterCharacter->GetVelocity(); //getting speed
	velocity.Z = 0.f; //not interested in Z velocity
	speed = velocity.Size(); //magnitude of velocity vector

	//set variables for anim bp
	hasWeaponEquipped = blasterCharacter->GetHasWeaponEquipped();
	equippedWeapon = blasterCharacter->GetEquippedWeapon();
	isCrouching = blasterCharacter->bIsCrouched;
	isAiming = blasterCharacter->GetIsAiming();
	isInAir = blasterCharacter->GetCharacterMovement()->IsFalling();
	isAccelerating = blasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	turningInPlace = blasterCharacter->GetTurningInPlace();
	rotateRootBone = blasterCharacter->GetShouldRotateRootBone();
	eliminated = blasterCharacter->GetIsEliminated();

	//getting movement rotation
	FRotator aimRotation = blasterCharacter->GetBaseAimRotation();
	FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(blasterCharacter->GetVelocity());
	FRotator dRotation = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation);
	deltaRotation = FMath::RInterpTo(deltaRotation, dRotation, DeltaTime, 5.f); //smoothing
	yawOffset = deltaRotation.Yaw;
	
	//getting leaning rotation
	rotationLastFrame = rotation;
	rotation = blasterCharacter->GetActorRotation();
	const FRotator deltaLean = UKismetMathLibrary::NormalizedDeltaRotator(rotation, rotationLastFrame);
	const float target = deltaLean.Yaw / DeltaTime;
	const float interp = FMath::FInterpTo(leanOffset, target, DeltaTime, 1.f); //smoothing
	leanOffset = FMath::Clamp(interp, -90.f, 90.f) / 1.3f;

	aimOffsetYaw = blasterCharacter->GetAimOffsetYaw();
	aimOffsetPitch = blasterCharacter->GetAimOffsetPitch();

	if (!isAccelerating)
	{
		leanOffset = 0;
	}

	//gluing left hand to weapon
	if (hasWeaponEquipped && equippedWeapon && equippedWeapon->GetWeaponMesh() && blasterCharacter->GetMesh())
	{
		leftHandTransform = equippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("leftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector outPosition;
		FRotator outRotation;
		blasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), leftHandTransform.GetLocation(), FRotator::ZeroRotator, outPosition, outRotation);

		leftHandTransform.SetLocation(outPosition);
		leftHandTransform.SetRotation(FQuat(outRotation));
	}

	useFABRIK = blasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}