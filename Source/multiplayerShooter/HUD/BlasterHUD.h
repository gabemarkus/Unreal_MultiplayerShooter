// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* crosshairCenter;
	UTexture2D* crosshairLeft;
	UTexture2D* crosshairRight;
	UTexture2D* crosshairTop;
	UTexture2D* crosshairBottom;
	float crosshairSpread;
	FLinearColor crosshairColor;
};

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> characterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* characterOverlay;

	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> announcerClass;

	UPROPERTY()
	class UAnnouncer* announcer;

	void AddAnnouncer();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> debugClass;

	UPROPERTY()
	class UDebugOverlay* debugOverlay;

	void AddDebug();

private:
	FHUDPackage HUDpackage;

	void DrawCrosshair(UTexture2D* texture, FVector2D viewportCenter, FVector2D spread, FLinearColor crosshairColor);

	UPROPERTY(EditAnywhere)
	float crosshairSpreadMax = 16.f;

protected:

	virtual void BeginPlay() override;


public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& package) {HUDpackage = package;}
};
