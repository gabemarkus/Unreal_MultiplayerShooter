// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletCasing.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ABulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletCasing();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, FVector normalImpulse, const FHitResult& hit);

	void DestroyCasing();

	void ShrinkCasing();

private:

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* casingMesh;

	UPROPERTY(EditAnywhere)
	float casingImpulse;

	UPROPERTY(EditAnywhere)
	bool shrinks;

	UPROPERTY(EditAnywhere)
	float decayTime;

	bool soundplayed = false;

	UPROPERTY(EditAnywhere)
	class USoundCue* casingSound;

};
