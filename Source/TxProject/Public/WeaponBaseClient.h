// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseClient.generated.h"

UCLASS()
class TXPROJECT_API AWeaponBaseClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBaseClient();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ClientArmsFireMontage;

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FPGunAnimation")
	void PlayShootAnimation();

	//�������������
	void DisplayWeaponEffect();
};
