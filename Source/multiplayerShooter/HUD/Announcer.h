// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcer.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UAnnouncer : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Countdown;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InfoText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchStartsIn;
	
};
