// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "Announcer.h"
#include "CharacterOverlay.h"
#include "DebugOverlay.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	AddDebug();
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* playerController = GetOwningPlayerController();

	if (playerController && characterOverlayClass)
	{
		characterOverlay = CreateWidget<UCharacterOverlay>(playerController, characterOverlayClass);
		characterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncer()
{
	APlayerController* playerController = GetOwningPlayerController();

	if (playerController && announcerClass)
	{
		announcer = CreateWidget<UAnnouncer>(playerController, announcerClass);
		announcer->AddToViewport();
	}
}

void ABlasterHUD::AddDebug()
{
	APlayerController* playerController = GetOwningPlayerController();

	if (playerController && debugClass)
	{
		debugOverlay = CreateWidget<UDebugOverlay>(playerController, debugClass);
		debugOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D viewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(viewportSize);
		const FVector2D viewportCenter(viewportSize.X / 2, viewportSize.Y / 2);

		float spreadScaled = crosshairSpreadMax * HUDpackage.crosshairSpread;

		if (HUDpackage.crosshairCenter)
		{
			FVector2D spread(0.f, 0.f);
			DrawCrosshair(HUDpackage.crosshairCenter, viewportCenter, spread, HUDpackage.crosshairColor);
		}
		if (HUDpackage.crosshairLeft)
		{
			FVector2D spread(-spreadScaled, 0.f);
			DrawCrosshair(HUDpackage.crosshairLeft, viewportCenter, spread, HUDpackage.crosshairColor);
		}
		if (HUDpackage.crosshairRight)
		{
			FVector2D spread(spreadScaled, 0.f);
			DrawCrosshair(HUDpackage.crosshairRight, viewportCenter, spread, HUDpackage.crosshairColor);
		}
		if (HUDpackage.crosshairTop)
		{
			FVector2D spread(0.f, -spreadScaled);
			DrawCrosshair(HUDpackage.crosshairTop, viewportCenter, spread, HUDpackage.crosshairColor);
		}
		if (HUDpackage.crosshairBottom)
		{
			FVector2D spread(0.f, spreadScaled);
			DrawCrosshair(HUDpackage.crosshairBottom, viewportCenter, spread, HUDpackage.crosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* texture, FVector2D viewportCenter, FVector2D spread, FLinearColor crosshairColor)
{
	const float textureWidth = texture->GetSizeX();
	const float textureHeight = texture->GetSizeY();
	const FVector2D textureDrawPoint(
		viewportCenter.X - (textureWidth / 2) + spread.X,
		viewportCenter.Y - (textureHeight / 2) + spread.Y
	);

	DrawTexture(
		texture,
		textureDrawPoint.X,
		textureDrawPoint.Y,
		textureWidth,
		textureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		crosshairColor
		);
}

