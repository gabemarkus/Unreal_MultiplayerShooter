// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "multiplayerShooter/HUD/BlasterHUD.h"
#include "multiplayerShooter/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BuffComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerShooter/GameModes/BlasterGameMode.h"
#include "multiplayerShooter/HUD/Announcer.h"
#include "multiplayerShooter/HUD/DebugOverlay.h"
#include "Kismet/GameplayStatics.h"
#include "multiplayerShooter/BlasterGameState.h"
#include "multiplayerShooter/PlayerState/BlasterPlayerState.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	blasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ABlasterPlayerController::PollInit()
{
	//wait for HUD to load before setting values
	if (characterOverlay == nullptr)
	{
		if (blasterHUD && blasterHUD->characterOverlay)
		{
			characterOverlay = blasterHUD->characterOverlay;
			if (characterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	//get match state and settings from game mode
	//used for displaying timers on hud

	ABlasterGameMode* gameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (gameMode)
	{
		warmupTime = gameMode->warmUpTime;
		matchTime = gameMode->matchTime;
		cooldownTime = gameMode->cooldownTime;
		levelStartTime = gameMode->timeOfLevelStart;
		matchState = gameMode->GetMatchState();

		if (blasterHUD && matchState == MatchState::WaitingToStart)
		{
			blasterHUD->AddAnnouncer();
		}

		ClientJoinMidGame(matchState, warmupTime, matchTime, levelStartTime, cooldownTime);
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName state, float warmup, float match, float startTime, float cooldown)
{
	//sync client's match state and settings to the server's
	//otherwise if a client joined midgame they would display the default time rather than the actual time remaining
	warmupTime = warmup;
	matchTime = match;
	levelStartTime = startTime;
	matchState = state;
	cooldownTime = cooldown;
	OnMatchStateSet(matchState);

	if (blasterHUD && matchState == MatchState::WaitingToStart && !HasAuthority())
	{
		blasterHUD->AddAnnouncer();
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, matchState);
}

void ABlasterPlayerController::OnPossess(APawn* inPawn)
{
	Super::OnPossess(inPawn);

	character = Cast<ABlasterCharacter>(inPawn);

	if (character)
	{
		SetHUDHealth(character->GetHealth(), character->GetMaxHealth());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName newState)
{
	matchState = newState;

	HandleMatchStates();
}

void ABlasterPlayerController::OnRep_MatchState()
{
	//client handle match state
	HandleMatchStates();
}

void ABlasterPlayerController::HandleMatchStates()
{
	if (matchState == MatchState::WaitingToStart)
	{

	}
	else if (matchState == MatchState::InProgress)
	{
		HandleMatchHasStartedState();
	}
	else if (matchState == MatchState::Cooldown)
	{
		HandleMatchCooldownState();
	}
}

void ABlasterPlayerController::HandleMatchHasStartedState()
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	if (blasterHUD)
	{
		blasterHUD->AddCharacterOverlay();
		if (blasterHUD->announcer)
		{
			blasterHUD->announcer->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleMatchCooldownState() //match over
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	if (blasterHUD)
	{
		blasterHUD->characterOverlay->RemoveFromParent(); //hide gameplay hud

		if (blasterHUD->announcer && blasterHUD->announcer->MatchStartsIn && blasterHUD->announcer->InfoText)
		{
			blasterHUD->announcer->SetVisibility(ESlateVisibility::Visible); //show announcer hud and change text
			FString cooldownAnnouncementText("New match starting in...");
			blasterHUD->announcer->MatchStartsIn->SetText(FText::FromString(cooldownAnnouncementText));

			//display winner/loser/tie
			ABlasterGameState* blasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* blasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (blasterGameState && blasterPlayerState)
			{
				TArray<ABlasterPlayerState*> topPlayers = blasterGameState->topScoringPlayers;
				FString infoTextString;
				if (topPlayers.Num() == 0) //no high score
				{
					infoTextString = FString("No one won...");
				}
				else if (topPlayers.Num() == 1 && topPlayers[0] == blasterPlayerState) //current player won
				{
					infoTextString = FString("You won!!");
				}
				else if (topPlayers.Num() == 1) //another player won
				{
					infoTextString = FString::Printf(TEXT("Winner: \n%s"), *topPlayers[0]->GetPlayerName());
				}
				else if (topPlayers.Num() > 1) //multiple players won
				{
					infoTextString = FString("Players tied for the win: \n");
					for (auto tiedPlayer : topPlayers)
					{
						infoTextString.Append(FString::Printf(TEXT(" % s\n "), *tiedPlayer->GetPlayerName()));
					}
				}

				blasterHUD->announcer->InfoText->SetText(FText::FromString(infoTextString));

			}
		}
	}
}

//Death message (killed by...)
//called from BlasterPlayerState

void ABlasterPlayerController::SetDeathMessage(FString killer)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->KilledBy &&
		blasterHUD->characterOverlay->KilledByName;

	if (hudValid)
	{
		blasterHUD->characterOverlay->KilledByName->SetText(FText::FromString(killer));
		blasterHUD->characterOverlay->KilledByName->SetVisibility(ESlateVisibility::Visible);
		blasterHUD->characterOverlay->KilledBy->SetVisibility(ESlateVisibility::Visible);

		FTimerHandle deathMessageTimer;
		GetWorldTimerManager().SetTimer(deathMessageTimer, this, &ABlasterPlayerController::DeathMessageTimerFinished, 2.5f);
	}
}

void ABlasterPlayerController::DeathMessageTimerFinished()
{
	blasterHUD->characterOverlay->KilledByName->SetVisibility(ESlateVisibility::Hidden);
	blasterHUD->characterOverlay->KilledBy->SetVisibility(ESlateVisibility::Hidden);
}

//Jump buff message
//Called from JumpBuff

void ABlasterPlayerController::SetJumpBuffActiveMessage()
{
	ClientDisplayJumpBuffText();
}

void ABlasterPlayerController::ClientDisplayJumpBuffText_Implementation()
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->JumpBuffText;

	if (hudValid)
	{
		blasterHUD->characterOverlay->JumpBuffText->SetVisibility(ESlateVisibility::Visible);
		FTimerHandle jumpBuffTextTimer;
		GetWorldTimerManager().SetTimer(jumpBuffTextTimer, this, &ABlasterPlayerController::JumpBuffTimerFinished, 10.f);
	}
}

void ABlasterPlayerController::JumpBuffTimerFinished()
{
	blasterHUD->characterOverlay->JumpBuffText->SetVisibility(ESlateVisibility::Hidden);
}

//Hitmarker
//called in hitscanweapon and projectilebullet

void ABlasterPlayerController::HandleHitmarker()
{
	ClientHandleHitmarker(); 
	//must call playhitmarker animation from client RPC to replicate to clients
	//client RPC also handles server
}

void ABlasterPlayerController::ClientHandleHitmarker_Implementation()
{
	PlayHitmarkerAnimation();
}

void ABlasterPlayerController::PlayHitmarkerAnimation()
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->HitmarkerJPEG;

	if (hudValid)
	{
		blasterHUD->characterOverlay->PlayAnimation(blasterHUD->characterOverlay->HitmarkerJPEGAnimation);
	}
}

//HUD health
//called in BlasterCharacter when gaining or losing hp

void ABlasterPlayerController::SetHUDHealth(float health, float maxHealth)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;

	bool hudValid = blasterHUD && 
		blasterHUD->characterOverlay && 
		blasterHUD->characterOverlay->HealthBar && 
		blasterHUD->characterOverlay->HealthText;

	if (hudValid)
	{
		//health percentage for health bar
		const float healthPercent = health / maxHealth;
		blasterHUD->characterOverlay->HealthBar->SetPercent(healthPercent);

		FString healthText = FString::Printf(TEXT("%d - %d"), FMath::CeilToInt(health), FMath::CeilToInt(maxHealth));
		blasterHUD->characterOverlay->HealthText->SetText(FText::FromString(healthText));
	}
	else
	{
		characterOverlayUnitialized = true;
		HUDHealth = health;
		HUDMaxHealth = maxHealth;
	}
}

//HUD score
//called in BlasterPlayerState

void ABlasterPlayerController::SetHUDScore(float score)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->ScoreAmount;

	if (hudValid)
	{
		FString scoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(score));
		blasterHUD->characterOverlay->ScoreAmount->SetText(FText::FromString(scoreText));
		//got kill sound
		if (score > 0)
		{
			UGameplayStatics::PlaySound2D(this, gotKillSound);
		}
	}
	else
	{
		characterOverlayUnitialized = true;
		HUDScore = score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 defeats)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->DefeatAmount;

	if (hudValid)
	{
		FString defeatsText = FString::Printf(TEXT("%d"), defeats);
		blasterHUD->characterOverlay->DefeatAmount->SetText(FText::FromString(defeatsText));
		//died sound
		if (defeats > 0)
		{
			UGameplayStatics::PlaySound2D(this, diedSound);
		}


	}
	else
	{
		characterOverlayUnitialized = true;
		HUDScore = defeats;
	}
}

//HUD ammo
//called in Weapon and BlasterCharacter
void ABlasterPlayerController::SetHUDWeaponAmmo(int32 ammo)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->WeaponAmmoAmount;

	if (hudValid)
	{
		FString ammoText = FString::Printf(TEXT("%d"), ammo);
		blasterHUD->characterOverlay->WeaponAmmoAmount->SetText(FText::FromString(ammoText));
	}
}

void ABlasterPlayerController::SetHUDPlayerAmmo(int32 ammo)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->PlayerAmmoAmount;

	if (hudValid)
	{
		FString ammoText = FString::Printf(TEXT("%d"), ammo);
		blasterHUD->characterOverlay->PlayerAmmoAmount->SetText(FText::FromString(ammoText));
	}
}

//Hud countdown and time sync
void ABlasterPlayerController::SetHUDCountdown(float countdownTime)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->characterOverlay &&
		blasterHUD->characterOverlay->CountdownText;

	if (hudValid)
	{
		if (countdownTime <= 0.f || FMath::IsNaN(countdownTime)) //never see negative countdown time
		{
			blasterHUD->characterOverlay->CountdownText->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
		else
		{
			blasterHUD->characterOverlay->CountdownText->SetVisibility(ESlateVisibility::Visible);
		}

		int32 min = FMath::FloorToInt(countdownTime / 60);
		int32 sec = countdownTime - min * 60;

		FString countdownText = FString::Printf(TEXT("%02d : %02d"), min, sec);
		blasterHUD->characterOverlay->CountdownText->SetText(FText::FromString(countdownText));

		//tick sound
		if (sec <= 3)
		{
			if (min == 0)
			{
				UGameplayStatics::PlaySound2D(this, tickSound);
			}
		}
	}
}

void ABlasterPlayerController::SetHUDAnnouncerCountdown(float countdownTime)
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->announcer &&
		blasterHUD->announcer->Countdown;

	if (hudValid)
	{
		if (countdownTime < 0.f || FMath::IsNaN(countdownTime))
		{
			blasterHUD->announcer->Countdown->SetText(FText());
			return;
		}

		int32 min = FMath::FloorToInt(countdownTime / 60);
		int32 sec = countdownTime - min * 60;

		FString countdownText = FString::Printf(TEXT("%02d : %02d"), min, sec);
		blasterHUD->announcer->Countdown->SetText(FText::FromString(countdownText));

		//tick sound
		if (sec <= 3)
		{
			if (min == 0)
			{
				UGameplayStatics::PlaySound2D(this, tickSound);
			}
		}
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float timeLeft = 0.f;
	if (matchState == MatchState::WaitingToStart) timeLeft = warmupTime - GetServerTime() - levelStartTime;
	else if (matchState == MatchState::InProgress) timeLeft = warmupTime + matchTime - GetServerTime() + levelStartTime;
	else if (matchState == MatchState::Cooldown) timeLeft = warmupTime + matchTime + cooldownTime - GetServerTime() + levelStartTime;

	uint32 secondsLeft = 0.f;

	if (timeLeft >= 0)
	{
		 secondsLeft = FMath::CeilToInt(timeLeft);
	}

	if (HasAuthority())
	{
		blasterGameMode = blasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : blasterGameMode;
		if (blasterGameMode)
		{
			secondsLeft = FMath::CeilToInt(blasterGameMode->GetCountdownTime() + levelStartTime);
		}
	}

	if (countdownInt != secondsLeft)
	{
		if (matchState == MatchState::WaitingToStart || matchState == MatchState::Cooldown)
		{
			SetHUDAnnouncerCountdown(secondsLeft);
		}
		if (matchState == MatchState::InProgress)
		{
			SetHUDCountdown(secondsLeft);
		}
	}
	countdownInt = secondsLeft;
}

//syncing server and client time
//we are using getworld() to get time elapsed, so client will have different time than the server

//1- client sends rpc to server requesting time and providing time of request
//2- server responds with rpc providing time of receipt and client request time
//3- round trip time = current client time - client request time
//4- current server time = server time of receipt + 1/2 round trip time (assuming symmetrical rtt)
//5- client/server delta = current server time - current client time
//6- GetServerTime() = current client time - client/server delta

//RPC to request server time
//sent to server from client
void ABlasterPlayerController::ServerRequestServerTime_Implementation(float timeOfClientRequest)
{
	float serverTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(timeOfClientRequest, serverTimeOfReceipt);
}

//server returns time of receipt and time of client request so we can calculate delta
void ABlasterPlayerController::ClientReportServerTime_Implementation(float timeOfClientRequest, float timeServerRecievedClientRequest)
{
	float roundTripTime = GetWorld()->GetTimeSeconds() - timeOfClientRequest;
	float currentServerTime = timeServerRecievedClientRequest + (roundTripTime / 2); //assuming symmetrical round trip time
	clientServerDelta = currentServerTime - GetWorld()->GetTimeSeconds();
}

//we calculate the client's current time
//client time + clientserverdelta
//called in SetHUDTime
float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + clientServerDelta;
}

//sync with server clock as soon as possible
void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

//continuously sync server/client time at timesyncfrequency interval
//called in tick
void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	timeSinceLastSync += DeltaTime;
	if (IsLocalController() && timeSinceLastSync > timeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		timeSinceLastSync = 0.f;
	}
}

//hud text when restarting match with button

void ABlasterPlayerController::SetHUDDebugMessages()
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->debugOverlay &&
		blasterHUD->debugOverlay->UserRestart;

	if (hudValid)
	{
		blasterHUD->debugOverlay->UserRestart->SetVisibility(ESlateVisibility::Visible);
	}
}

void ABlasterPlayerController::DebugMessageTimerFinished()
{
	blasterHUD = blasterHUD == nullptr ? Cast <ABlasterHUD>(GetHUD()) : blasterHUD;
	bool hudValid = blasterHUD &&
		blasterHUD->debugOverlay &&
		blasterHUD->debugOverlay->UserRestart;

	if (hudValid)
	{
		blasterHUD->debugOverlay->UserRestart->SetVisibility(ESlateVisibility::Hidden);
	}
}

