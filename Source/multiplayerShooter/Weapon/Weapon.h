// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

//custom weapon state enum
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class MULTIPLAYERSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap (
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComp,
		int32 otherBodyIndex,
		bool bFromSweep,
		const FHitResult& sweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComp,
		int32 otherBodyIndex
	);

public:	
	virtual void Tick(float DeltaTime) override;
	void RotateWeapon();
	//for replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;
	

	void ShowPickupWidget(bool showWidget);
	virtual void Fire(const FVector& hitTarget);
	void Dropped();
	void SetHUDAmmo();
	void AddAmmo(int32 ammoToAdd);

	UPROPERTY(BlueprintReadOnly)
	bool weaponInitState;

	//Textures for weapon crosshairs:

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* crosshairCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* crosshairLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* crosshairRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* crosshairTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* crosshairBottom;

	//For zoom when aiming:

	UPROPERTY(EditAnywhere)
	float zoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float zoomSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool autoWeapon = true;

	UPROPERTY(EditAnywhere, Category = Combat)
	float fireDelay = .2f;

	//custom depth (weapon glow outline)

	void EnableCustomDepth(bool enable);

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* pickupWidget;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* weaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* areaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState weaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* fireAnimation;

	UPROPERTY(EditAnywhere)
	class USoundBase* fireSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletCasing> casingClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 ammo;

	UPROPERTY(EditAnywhere)
	int32 weaponMaxAmmo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendAmmo();

	UPROPERTY()
	class ABlasterCharacter* ownerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* ownerController;

	UPROPERTY(EditAnywhere)
	EWeaponType weaponType;

	void OnEWSEquipped();
	void OnEWSDropped();


public:
	void SetWeaponState(EWeaponState state);
	FORCEINLINE class USphereComponent* GetAreaSphere() const { return areaSphere; }
	FORCEINLINE class USkeletalMeshComponent* GetWeaponMesh() const { return weaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return zoomedFOV; }
	FORCEINLINE float GetZoomSpeed() const { return zoomSpeed; }
	FORCEINLINE bool IsEmpty() const { return ammo <= 0; }
	FORCEINLINE EWeaponType GetWeaponType() const { return weaponType; }
	FORCEINLINE int32 GetAmmo() const { return ammo; }
	FORCEINLINE int32 GetWeaponCapacity() const { return weaponMaxAmmo; }
	FORCEINLINE EWeaponState GetWeaponState() const { return weaponState; }
	
};
