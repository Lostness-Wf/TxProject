// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "FPSCharacterBase.h"

// Sets default values
AGrenadeBase::AGrenadeBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	RootComponent = GrenadeMesh;

	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForce"));
	RadialForce->SetupAttachment(GrenadeMesh);

	ExplosionEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionEffect"));
	ExplosionEffect->SetupAttachment(GrenadeMesh);

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(GrenadeMesh);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldDynamic);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AGrenadeBase::OnOtherBeginOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AGrenadeBase::OnOtherEndOverlap);
}

// Called when the game starts or when spawned
void AGrenadeBase::BeginPlay()
{
	Super::BeginPlay();
	
	FLatentActionInfo ActionInfo;
	ActionInfo.CallbackTarget = this;
	ActionInfo.ExecutionFunction = TEXT("Explosion");
	ActionInfo.UUID = FMath::Rand();
	ActionInfo.Linkage = 0;
	UKismetSystemLibrary::Delay(this, 3, ActionInfo);
}

// Called every frame
void AGrenadeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrenadeBase::Explosion()
{
	ServerExplosion();
}

void AGrenadeBase::OnOtherBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacterBase* Player = Cast<AFPSCharacterBase>(OtherActor);
	if (Player)
	{
		PlayerInCollision.Add(Player);
	}
}

void AGrenadeBase::OnOtherEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSCharacterBase* Player = Cast<AFPSCharacterBase>(OtherActor);
	if (Player)
	{
		PlayerInCollision.Remove(Player);
	}
}

void AGrenadeBase::MulticastExplosion_Implementation()
{
	RadialForce->FireImpulse();
	ExplosionEffect->ActivateSystem(true);
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
}

bool AGrenadeBase::MulticastExplosion_Validate()
{
	return true;
}

void AGrenadeBase::ServerExplosion_Implementation()
{
	TSubclassOf<UDamageType> DamageType;
	for (auto Player : PlayerInCollision)
	{
		Player->GrenadeExplosion(GetOwner());
	}
	MulticastExplosion();

	Destroy();
}

bool AGrenadeBase::ServerExplosion_Validate()
{
	return true;
}

