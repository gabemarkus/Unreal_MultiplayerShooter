// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UMyUserWidget::SetDisplayText(FString textToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(textToDisplay));
	}
}
void UMyUserWidget::showPlayerNetRole(APawn* inPawn)
{
FString playerName = inPawn->GetPlayerState()->GetPlayerName();
FString nameString = FString::Printf(TEXT("NAME"));
SetDisplayText(nameString);
}

//void UMyUserWidget::OnLevelRemovedFromWorld(ULevel* inLevel, UWorld* inWorld)
//{
	//RemoveFromParent();
//	Super::OnLevelRemovedFromWorld(inLevel, inWorld);
//}
