// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseServer.h"
#include "FPSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBaseServer::AWeaponBaseServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECC_WorldStatic);

	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);

	WeaponMesh->SetOwnerNoSee(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetSimulatePhysics(true);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBaseServer::OnOtherBeginOverlap);

	SetReplicates(true);
}

void AWeaponBaseServer::OnOtherBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(OtherActor);
	if (FPSCharacter)
	{
		EquipWeapon();
		if (KindOfWeapon == EWeaponType::DesertEagle)
		{
			FPSCharacter->EquipSrcondary(this);
		}
		else
		{
		FPSCharacter->EquipPrimary(this);
		}
	}
}

//¼ñµ½Ç¹Ê±ÉèÖÃÅö×²
void AWeaponBaseServer::EquipWeapon()
{
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetSimulatePhysics(false);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AWeaponBaseServer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		//»ðÑæÁ£×ÓVFX
		FName MuzzleFlashSocketName = TEXT("Fire_FX_Slot");
		if (KindOfWeapon == EWeaponType::M4A1)
		{
			MuzzleFlashSocketName = TEXT("MuzzleSocket");
		}

		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh,
			MuzzleFlashSocketName, FVector::ZeroVector,
			FRotator::ZeroRotator, FVector::OneVector,
			EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None,
			true);


	}UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
}

bool AWeaponBaseServer::MultiShootingEffect_Validate()
{
	return true;
}

void AWeaponBaseServer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION(AWeaponBaseServer, ClipCurrentAmmo, COND_None);
}
