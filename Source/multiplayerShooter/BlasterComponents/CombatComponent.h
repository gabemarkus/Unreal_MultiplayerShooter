// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "multiplayerShooter/HUD/BlasterHUD.h"
#include "multiplayerShooter/Weapon/WeaponTypes.h"
#include "multiplayerShooter/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter; //friend -> blastercharacter can access protected and private objects

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(class AWeapon* weaponToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void PickupAmmo(int32 ammoAmount);

	FORCEINLINE void ClearEquippedWeapon() { equippedWeapon = nullptr; UE_LOG(LogTemp, Warning, TEXT("equipped weapon cleared")); }

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool aiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool aiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool isPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& traceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& traceHitTarget);

	void LineTraceUnderCrosshairs(FHitResult& lineTraceHitResult);

	void SetHUDCrosshair(float deltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();
	void UpdateAmmoValues();

private:
	class ABlasterCharacter* character;
	class ABlasterPlayerController* playerController;
	class ABlasterHUD* hud;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* equippedWeapon;

	UPROPERTY(Replicated)
	bool isAiming;

	UPROPERTY(EditAnywhere)
	float baseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float aimWalkSpeed;

	bool fireButtonPressed;

	FVector HitTarget;

	//HUD and crosshairs

	FHUDPackage HUDpackage;

	float crosshairVelocityFactor; //amount to spread crosshairs based on velocity
	float crosshairInAirFactor;
	float crosshairCrouchFactor;
	float crosshairAimFactor;
	float crosshairShootingFactor;

	//Fov when not aiming
	float defaultFOV;

	float currentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float zoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float zoomSpeed = 20.f;

	void InterpolateFOV(float deltaTime);

	//automatic fire

	bool canFire = true;

	bool CheckCanFire();

	UPROPERTY(ReplicatedUsing = OnRep_PlayerAmmo)
	int32 playerAmmo;

	UFUNCTION()
	void OnRep_PlayerAmmo();

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState combatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(EditAnywhere)
	int32 startingARAmmo = 30;

	TMap<EWeaponType, int32> carriedAmmoMap;
	
	FTimerHandle fireTimer;

	void FireTimerFinished();
	void StartFireTimer();

	void SpawnClip();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletCasing> droppedClip;

	UPROPERTY(EditAnywhere)
	class USoundBase* reloadSound;

	bool islocallyreloading;

};
