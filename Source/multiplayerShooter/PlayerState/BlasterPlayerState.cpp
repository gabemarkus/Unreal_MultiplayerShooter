// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, defeats);
}

void ABlasterPlayerState::AddToScore(float amount)
{
	SetScore(Score += amount);

	character = character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : character;
	if (character)
	{
		controller = controller == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : controller;
		if (controller)
		{
			controller->SetHUDScore(Score);
		}
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	character = character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : character;
	if (character)
	{
		controller = controller == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : controller;
		if (controller)
		{
			controller->SetHUDScore(Score);
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 defeatsAmount)
{
	defeats += defeatsAmount;
	character = character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : character;
	if (character)
	{
		controller = controller == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : controller;
		if (controller)
		{
			controller->SetHUDDefeats(defeats);
		}
	}
}

void ABlasterPlayerState::ClientUpdateKillFeed_Implementation(const FString& Killer)
{
	character = character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : character;
	if (character)
	{
		controller = controller == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : controller;
		if (controller)
		{
			controller->SetDeathMessage(Killer);
		}
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	character = character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : character;
	if (character)
	{
		controller = controller == nullptr ? Cast<ABlasterPlayerController>(character->Controller) : controller;
		if (controller)
		{
			controller->SetHUDDefeats(defeats);
		}
	}
}