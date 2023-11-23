// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeBase.generated.h"

class URadialForceComponent;
class USphereComponent;
class AFPSCharacterBase;

UCLASS()
class TXPROJECT_API AGrenadeBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* GrenadeMesh;

private:
	UPROPERTY(VisibleAnywhere, Category = "Force")
	URadialForceComponent* RadialForce;

	UPROPERTY(EditAnywhere, Category = "ExplosionEffect")
	UParticleSystemComponent* ExplosionEffect;
	
	UPROPERTY(EditAnywhere, Category = "ExplosionEffect")
	USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, Category = "Collision")
	USphereComponent* SphereCollision;

	UPROPERTY()
	TArray<AFPSCharacterBase*> PlayerInCollision;

public:
	UFUNCTION()
	void Explosion();

	UFUNCTION()
	void OnOtherBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOtherEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MulticastExplosion();
	void MulticastExplosion_Implementation();
	void MulticastExplosion_Validation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerExplosion();
	void ServerExplosion_Implementation();
	void ServerExplosion_Validation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDestroyActor();
	void ServerDestroyActor_Implementation();
	void ServerDestroyActor_Validation();
};
