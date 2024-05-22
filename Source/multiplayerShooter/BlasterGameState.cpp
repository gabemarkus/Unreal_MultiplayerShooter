// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerShooter/PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, topScoringPlayers);

}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* player)
{
	float playerScore = player->GetScore();

	if (topScoringPlayers.Num() == 0)
	{
		topScoringPlayers.Add(player);
		topScore = playerScore;
	}
	else if (playerScore == topScore)
	{
		topScoringPlayers.AddUnique(player);
	}
	else if(playerScore > topScore)
	{
		topScoringPlayers.Empty();
		topScoringPlayers.AddUnique(player);
		topScore = playerScore;
	}
}
