// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "multiplayerShooter/TurnInPlace.h"
#include "multiplayerShooter/Interfaces/InteractWithCrosshairsInterface.h"
#include "multiplayerShooter/CombatState.h"
#include "BlasterCharacter.generated.h"

UCLASS(Blueprintable)
class MULTIPLAYERSHOOTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;

	void AimOffsetTick(float DeltaTime);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//need to overwrite for replicating variables - stores a TArray of replicated variables
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//earliest way to access CombatComponent
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool isAiming);
	void PlayHitReactMontage(bool isAiming);
	void PlayEliminatedMontage();
	void PlayReloadMontage();

	virtual void OnRep_ReplicatedMovement() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();	
	
	UFUNCTION(NetMulticast, Reliable)
	void Explode();

	void Eliminated();

	void UpdateHudHealth();

protected:


	virtual void BeginPlay() override;

	//player movement and actions
	void MoveForward(float value);
	void MoveRight(float value);
	void Turn(float value);
	void LookUp(float value);
	void EquipKeyPressed();
	void CrouchKeyPressed();
	void CrouchKeyReleased();
	void AimKeyPressed();
	void AimKeyReleased();
	void AimOffset(float deltaTime);
	void CalculateAimOffsetPitch();
	void ProxiesTurnInPlace();
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void RestartButtonPressed();
	void SelfDamageButtonPressed();

	UFUNCTION()
	void ReceiveDamage(AActor* damagedActor, float damage, const UDamageType* damageType, class AController* instigatorController, AActor* damageCauser);

	// poll for relevant classes and initialize hud
	void PollInit();

private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* cameraSpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* overheadWidget;

	void HideCharacterFromCamera();

	UPROPERTY(EditAnywhere)
	float hideCharacterThreshold = 500.f;

	//replicated variable
	//ReplicatedUsing - function called on replication!
	UPROPERTY(ReplicatedUsing = OnRep_OverlappedWeapon)
	class AWeapon* overlappedWeapon;

	UFUNCTION()
	void OnRep_OverlappedWeapon(AWeapon* lastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* combatComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* buffComponent;

	//RPC - client-server event - remote procedure call
	UFUNCTION(Server, Reliable)
	void ServerEquipKeyPressed();

	float aimOffsetYaw;
	float interpAimOffsetYaw;
	float aimOffsetPitch;
	FRotator startingAimRotation;

	ETurningInPlace turningInPlace;
	void TurnInPlace(float DeltaTime);

	bool inAir;

	//Animation montages

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* fireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* hitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* eliminatedMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* reloadMontage;

	//proxy rotation

	bool rotateRootBone;
	float turnThreshold = 30.f;
	FRotator proxyRotationLastFrame;
	FRotator currentProxyRotation;
	float timeSinceLastMovementReplication;
	float CalculateSpeed();

	//player health

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float maxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, Category = "Player Stats", VisibleAnywhere)
	float health = 100.f;

	UFUNCTION()
	void OnRep_Health(float lastHealth);

	UPROPERTY()
	class ABlasterPlayerController* playerController;

	bool eliminated = false;

	FTimerHandle eliminatedTimer;
	void EliminatedTimerEnd();

	UPROPERTY(EditDefaultsOnly)
	float respawnDelay = 3.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* explosion;

	FTimerHandle requestRespawnTimer;
	void RequestRespawn();

	class ABlasterPlayerState* playerState;
//audio

	UPROPERTY()
	class ABlasterAudioHandler* blasterAudioHandler;

	UFUNCTION(BlueprintCallable)
	void PlayFootstepSound();

	UFUNCTION(BlueprintCallable)
	void PlayMPFootstepSound();

	UPROPERTY(EditAnywhere)
	class USoundBase* explodeSound;

	UPROPERTY(EditAnywhere)
	class USoundBase* jumpSound;

	UPROPERTY(EditAnywhere)
	class USoundBase* landingSound;

public:
	void SetOverlappedWeapon(AWeapon* weapon);
	FORCEINLINE float GetAimOffsetYaw() const { return aimOffsetYaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return aimOffsetPitch; }
	bool GetHasWeaponEquipped();
	bool GetIsAiming();
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return turningInPlace; }
	FORCEINLINE UCameraComponent* GetCamera() const { return camera; }
	FORCEINLINE bool GetShouldRotateRootBone() const { return rotateRootBone; }
	FORCEINLINE bool GetIsEliminated() const { return eliminated; }
	FORCEINLINE float GetHealth() const { return health; }
	FORCEINLINE float GetMaxHealth() const { return maxHealth; }
	FORCEINLINE float GetRespawnDelay() const { return respawnDelay; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return combatComponent; }
	FORCEINLINE UBuffComponent* GetBuff() const { return buffComponent; }
	FORCEINLINE void SetHealth(float amount) { health = amount; }
	FORCEINLINE ABlasterAudioHandler* GetAudioHandler()	const { return blasterAudioHandler; }
	FORCEINLINE ABlasterPlayerController* GetPlayerController() const { return playerController; }
};
 