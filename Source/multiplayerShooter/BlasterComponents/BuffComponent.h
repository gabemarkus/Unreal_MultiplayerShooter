// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;

	void SetInitialJumpVelocity(float velocity);

protected:
	virtual void BeginPlay() override;

	void Heal(float deltaTime);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void HealParams(float healAmount, float healTime);

	void HandleJumpBuff(float buffJumpVelocity, float buffTime);

	void ApplyJumpBuff(float buffJumpVelocity);

	void DisplayJumpBuffMessage();

	bool hasJumpBuff;

private:

	UPROPERTY()
	class ABlasterCharacter* character;
	
	//health buff
	bool isHealing = false;
	float healingRate = 0;
	float amountToHeal = 0;

	//jump buff
	FTimerHandle jumpBuffTimer;
	void ResetJump();
	float initialJumpVelocity;

	class ABlasterPlayerController* playerController;
};
