// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlasterPlayerController.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Heal(DeltaTime);
}

void UBuffComponent::HealParams(float healAmount, float healTime) //receive parameters from healthpickup class
{
	isHealing = true;
	healingRate = healAmount / healTime;
	amountToHeal += healAmount;
} 

void UBuffComponent::Heal(float deltaTime) //apply health buff
{
	if (!isHealing || character == nullptr || character->GetIsEliminated()) return;

	const float healThisFrame = healingRate * deltaTime;
	character->SetHealth(FMath::Clamp(character->GetHealth() + healThisFrame, 0.f, character->GetMaxHealth()));
	character->UpdateHudHealth();
	amountToHeal -= healThisFrame;

	if (amountToHeal <= 0 || character->GetHealth() >= character->GetMaxHealth())
	{
		isHealing = false;
		amountToHeal = 0;
	}
} 

void UBuffComponent::SetInitialJumpVelocity(float velocity)
{
	initialJumpVelocity = velocity;
}

void UBuffComponent::HandleJumpBuff(float buffJumpVelocity, float buffTime)
{
	if (character == nullptr) return;
	
	//set jump buff timer, reset jump velocity when completed
	character->GetWorldTimerManager().SetTimer(jumpBuffTimer, this, &UBuffComponent::ResetJump, buffTime);

	//apply jump buff and display text
	ApplyJumpBuff(buffJumpVelocity);
	DisplayJumpBuffMessage();
}

void UBuffComponent::ApplyJumpBuff(float buffJumpVelocity)
{
	if (character->GetCharacterMovement())
	{
		character->GetCharacterMovement()->JumpZVelocity = buffJumpVelocity;
		hasJumpBuff = true;
	}
}

void UBuffComponent::DisplayJumpBuffMessage()
{
	playerController = Cast<ABlasterPlayerController>(character->Controller);
	if (playerController)
	{
		playerController->SetJumpBuffActiveMessage();
	}
}

void UBuffComponent::ResetJump()
{
	if (character && character->GetCharacterMovement())
	{
		character->GetCharacterMovement()->JumpZVelocity = initialJumpVelocity;
		hasJumpBuff = false;
	}

}
