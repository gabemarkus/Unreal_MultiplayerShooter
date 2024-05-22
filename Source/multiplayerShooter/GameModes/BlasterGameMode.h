// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState //making custom match state
{
	extern MULTIPLAYERSHOOTER_API const FName Cooldown; //match over, display winner, begin cooldown timer
}

UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float deltaTime) override;

	void HandleCountdowns();

	virtual void PlayerEliminated(class ABlasterCharacter* eliminatedCharacter, class ABlasterPlayerController* victimController, class ABlasterPlayerController* attackerController);
	virtual void RequestRespawn(class ACharacter* eliminatedCharacter, AController* eliminatedController);

	void RestartLevel();

	UPROPERTY(EditDefaultsOnly)
	float warmUpTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float matchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float cooldownTime = 120.f;

	float timeOfLevelStart = 0.f;

protected:

	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	int32 randomPlayerStartID;
	int32 lastPlayerStartID;

	float countDownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return countDownTime; }
};
