// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Coreminimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void AddToScore(float amount);
	void AddToDefeats(int32 defeatsAmount);

	UFUNCTION(Client, UnReliable)
	void ClientUpdateKillFeed(const FString& Killer);

	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

private:

	UPROPERTY() //using UPROPERTY() on fwd declared classes to default to nullptr to avoid crashing in if checks
	class ABlasterCharacter* character;
	UPROPERTY()
	class ABlasterPlayerController* controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 defeats;
};
