// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BlasterPlayerController.h"
#include "multiplayerShooter/BlasterComponents/CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "multiplayerShooter/PlayerState/BlasterPlayerState.h"
#include "multiplayerShooter/BlasterGameState.h"

namespace MatchState //making custom match state
{
	 const FName Cooldown; //match over, display winner, begin cooldown timer
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true; //allow pre-match timer
}

void ABlasterGameMode::BeginPlay()
{
	timeOfLevelStart = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	HandleCountdowns();
}

void ABlasterGameMode::HandleCountdowns()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		countDownTime = warmUpTime - GetWorld()->GetTimeSeconds() + timeOfLevelStart;
		if (countDownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		countDownTime = warmUpTime + matchTime - GetWorld()->GetTimeSeconds() + timeOfLevelStart;
		if (countDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		countDownTime = cooldownTime + warmUpTime + matchTime - GetWorld()->GetTimeSeconds() + timeOfLevelStart;
		if (countDownTime <= 0.f)
		{
				RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	//get all player controllers and set match state when changed
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; it++)
	{
		ABlasterPlayerController* player = Cast<ABlasterPlayerController>(*it);
		if (player)
		{
			player->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* eliminatedCharacter, ABlasterPlayerController* victimController, ABlasterPlayerController* attackerController)
{
	ABlasterPlayerState* attackerPlayerState = attackerController ? Cast<ABlasterPlayerState>(attackerController->PlayerState) : nullptr; //get attacker state
	ABlasterPlayerState* victimPlayerState = victimController ? Cast<ABlasterPlayerState>(victimController->PlayerState) : nullptr; //get victim state
	ABlasterGameState* blasterGameState = GetGameState<ABlasterGameState>();

	if (attackerPlayerState && attackerPlayerState != victimPlayerState && blasterGameState)
	{
		attackerPlayerState->AddToScore(1.f);
		blasterGameState->UpdateTopScore(attackerPlayerState);
	}

	if (victimPlayerState)
	{
		victimPlayerState->AddToDefeats(1);
	}

	if (victimPlayerState && attackerPlayerState)
	{
		victimPlayerState->ClientUpdateKillFeed(attackerPlayerState->GetPlayerName());
	}

	if (eliminatedCharacter)
	{
		eliminatedCharacter->Eliminated();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* eliminatedCharacter, AController* eliminatedController)
{
	if (eliminatedCharacter)
	{
		eliminatedCharacter->Reset(); //depossess player controller in preparation for controlling a new one
		eliminatedCharacter->Destroy();
	}

	if (eliminatedController)
	{
		//spawn player at random playerStart
		TArray<AActor*> playerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts); //get all spawn points
		randomPlayerStartID = FMath::RandRange(0, playerStarts.Num() - 1); //random num

		if (randomPlayerStartID == lastPlayerStartID) //dont pick same spawn as last time
		{
			randomPlayerStartID = (randomPlayerStartID > 0) ? randomPlayerStartID-- : randomPlayerStartID++;
		}

		lastPlayerStartID = randomPlayerStartID;
		RestartPlayerAtPlayerStart(eliminatedController, playerStarts[randomPlayerStartID]); //spawn player
	}
}

void ABlasterGameMode::RestartLevel()
{
	RestartGame();
}
