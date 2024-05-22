// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override; //overriding replicated destroy function to replicate sound+impact to clients

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComponent, FVector normalImpulse, const FHitResult& hit);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit();

	UPROPERTY(EditAnywhere)
	float damage = 10;

private:

	UPROPERTY(EditAnywhere)
	class UBoxComponent* collisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* projectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* tracer;

	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* tracerComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* impactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* impactSound;



public:	


};
