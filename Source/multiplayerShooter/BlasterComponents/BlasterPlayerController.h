// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	//overrides
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* inPawn) override;
	virtual void ReceivedPlayer() override; //earliest way to get server time for time sync
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//

	void SetHUDHealth(float health, float maxHealth);
	void SetHUDDefeats(int32 defeats);
	void SetHUDScore(float score);
	void SetHUDWeaponAmmo(int32 ammo);
	void SetHUDPlayerAmmo(int32 ammo);
	void SetHUDCountdown(float countdownTime);
	void SetHUDAnnouncerCountdown(float countdownTime);
	void SetDeathMessage(FString killer);
	void SetJumpBuffActiveMessage();
	void HandleHitmarker();
	void SetHUDDebugMessages();

	//match state
	void OnMatchStateSet(FName newState);
	void HandleMatchStates();
	void HandleMatchHasStartedState();
	void HandleMatchCooldownState();
	//

	void CheckTimeSync(float DeltaTime);
	virtual float GetServerTime(); //synced with server world time


protected:

	virtual void BeginPlay() override;
	void SetHUDTime();

	//Sync time between client & server

	//request current server time and passes time when request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float timeOfClientRequest);

	//reports current server time to the client in response to serverrequestservertime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float timeOfClientRequest, float timeServerRecievedClientRequest);

	//difference between client and server time
	float clientServerDelta = 0.f;

	//how often client/server time is synced
	float timeSyncFrequency = 3.f;

	float timeSinceLastSync = 0.f;

	UFUNCTION(Client, Reliable)
	void ClientHandleHitmarker();

	UFUNCTION(Client, Reliable)
	void ClientDisplayJumpBuffText();

	UFUNCTION()
	void PlayHitmarkerAnimation();

	void PollInit();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName state, float warmup, float match, float startTime, float coolDown);

private:
	
	class ABlasterCharacter* character;

	UPROPERTY()
	class ABlasterGameMode* blasterGameMode;

	UPROPERTY()
	class ABlasterHUD* blasterHUD;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName matchState;

	UPROPERTY()
	class UCharacterOverlay* characterOverlay;

	//store and call HUD functions/variables in case characterOverlay is uninitialized on calling
	bool characterOverlayUnitialized = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	//

	void DeathMessageTimerFinished();
	void JumpBuffTimerFinished();
	void DebugMessageTimerFinished();

	float levelStartTime = 0.f;
	float matchTime = 0.f;
	float warmupTime = 0.f;
	float cooldownTime = 0.f;
	uint32 countdownInt = 0;
	
	//sounds
	UPROPERTY(EditAnywhere)
	USoundBase* tickSound;

	UPROPERTY(EditAnywhere)
	USoundBase* gotKillSound;

	UPROPERTY(EditAnywhere)
	USoundBase* diedSound;
	//

};
