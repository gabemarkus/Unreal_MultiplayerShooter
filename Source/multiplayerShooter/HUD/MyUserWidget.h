// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString textToDisplay);

	UFUNCTION(BlueprintCallable)
	void showPlayerNetRole(APawn* inPawn);

protected:

//	virtual void OnLevelRemovedFromWorld(ULevel* inLevel, UWorld* inWorld) override;
};
