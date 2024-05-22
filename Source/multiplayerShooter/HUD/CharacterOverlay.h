// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DefeatAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KilledBy;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KilledByName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CountdownText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* JumpBuffText;

	UPROPERTY(meta = (BindWidget))
	class UImage* HitmarkerJPEG;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* HitmarkerJPEGAnimation;

};