// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "multiplayerShooter/Characters/BlasterCharacter.h"
#include "multiplayerShooter/BlasterComponents/BlasterPlayerController.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "BulletCasing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "WeaponTypes.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	//create mesh
	weaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	weaponMesh->SetupAttachment(RootComponent);
	weaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	weaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(weaponMesh);

	weaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	weaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	//create collision sphere
	areaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSpehere"));
	areaSphere->SetupAttachment(RootComponent);

	areaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	areaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//create pickup text widget
	pickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	pickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateWeapon();
}

//replicated variables
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, weaponState);
	DOREPLIFETIME(AWeapon, ammo);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	weaponState = EWeaponState::EWS_Initial;

	//pickup collision calculated on server only
	if (HasAuthority())
	{
		areaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		areaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		//collision delegates
		areaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		areaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (pickupWidget)
	{
		pickupWidget->SetVisibility(false);
	}
}

void AWeapon::ShowPickupWidget(bool showWidget)
{
	if (pickupWidget)
	{
		pickupWidget->SetVisibility(showWidget);
	}
}

void AWeapon::RotateWeapon()
{
	if (weaponState == EWeaponState::EWS_Initial)
	{
		FRotator newRotation = FRotator(0, 3, 0);
		FQuat quatRotation = FQuat(newRotation);
		AddActorLocalRotation(quatRotation);
	}
}

void AWeapon::Fire(const FVector& hitTarget)
{
	if (fireAnimation)
	{
		weaponMesh->PlayAnimation(fireAnimation, false);
		FName name;
		UGameplayStatics::SpawnSoundAttached(fireSound, this->weaponMesh , name, GetActorLocation(), EAttachLocation::KeepWorldPosition);
	}

	if (casingClass)
	{
		const USkeletalMeshSocket* ammoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));

		if (ammoEjectSocket)
		{
			FTransform socketTransform = ammoEjectSocket->GetSocketTransform(GetWeaponMesh());

			FActorSpawnParameters spawnParameters;
			UWorld* world = GetWorld();

			if (world)
			{
				//spawn ammo casing
				world->SpawnActor<ABulletCasing>(
					casingClass,
					socketTransform.GetLocation(),
					socketTransform.GetRotation().Rotator(),
					spawnParameters
				);
			}
		}
	}

	SpendAmmo();
}

void AWeapon::Dropped()
{
	//on weapon dropped
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules detachRules(EDetachmentRule::KeepWorld, true);
	weaponMesh->DetachFromComponent(detachRules); //detach from player
	weaponMesh->AddImpulse(-1000 * weaponMesh->GetForwardVector());
	weaponMesh->AddImpulse(-200 * weaponMesh->GetUpVector()); //throw weapon
	SetOwner(nullptr);
	ownerCharacter = nullptr;
	ownerController = nullptr;
}

//glow outline
void AWeapon::EnableCustomDepth(bool enable)
{
	if (weaponMesh)
	{
		weaponMesh->SetRenderCustomDepth(enable);
	}
}

//on replication of weaponstate variable - for clients
void AWeapon::OnRep_WeaponState()
{
	switch (weaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEWSEquipped();
		break;

	case EWeaponState::EWS_Dropped:

		break;
	}
}

void AWeapon::SetWeaponState(EWeaponState state)
{
	weaponState = state;
	switch (weaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEWSEquipped();
		break;

	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		OnEWSDropped();
		break;
	}
}

void AWeapon::OnEWSEquipped()
{
	ShowPickupWidget(false);
	weaponMesh->SetSimulatePhysics(false);
	weaponMesh->SetEnableGravity(false);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
}

void AWeapon::OnEWSDropped()
{
	weaponMesh->SetSimulatePhysics(true);
	weaponMesh->SetEnableGravity(true);
	weaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	weaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	weaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}


void AWeapon::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	ABlasterCharacter* blasterCharacter = Cast<ABlasterCharacter>(otherActor);
	if (blasterCharacter)
	{
		blasterCharacter->SetOverlappedWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex)
{
	ABlasterCharacter* blasterCharacter = Cast<ABlasterCharacter>(otherActor);
	if (blasterCharacter)
	{
		blasterCharacter->SetOverlappedWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	ownerCharacter = ownerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : ownerCharacter;
	if (ownerCharacter)
	{
		ownerController = ownerController == nullptr ? Cast<ABlasterPlayerController>(ownerCharacter->Controller) : ownerController;
		if (ownerController)
		{
			ownerController->SetHUDWeaponAmmo(ammo);
		}
	}
}

void AWeapon::AddAmmo(int32 ammoToAdd)
{
	ammo = FMath::Clamp(ammo - ammoToAdd, 0, weaponMaxAmmo);
	SetHUDAmmo();
}


void AWeapon::SpendAmmo()
{
	ammo = FMath::Clamp(ammo - 1, 0, weaponMaxAmmo);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		ownerCharacter = nullptr;
		ownerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}


