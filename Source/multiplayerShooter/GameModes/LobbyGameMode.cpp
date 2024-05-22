// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* newPlayer) //first safe place to access player that joined session
{
	Super::PostLogin(newPlayer);

	//get array of players in session
	int32 numberofPlayers = GameState.Get()->PlayerArray.Num();
	
	if (numberofPlayers == 1) //start match when this many players are connected
	{
		UWorld* world = GetWorld();
		if (world)
		{
			bUseSeamlessTravel = true;
			world->ServerTravel(FString("/Game/Maps/BlasterGameMap?listen")); //change map
		}
	}
}
