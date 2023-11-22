// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

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

void AGrenadeBase::MulticastExplosion_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("MulticastExplosion"));

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
	UE_LOG(LogTemp, Warning, TEXT("ServerExplosion"));
	MulticastExplosion();
}

bool AGrenadeBase::ServerExplosion_Validate()
{
	return true;
}

