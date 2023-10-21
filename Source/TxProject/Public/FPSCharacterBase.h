// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "WeaponBaseServer.h"
#include "MultiFPSPlayerController.h"

#include "FPSCharacterBase.generated.h"

UCLASS()
class TXPROJECT_API AFPSCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSCharacterBase();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* PlayerCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FPArmsMesh;

	UAnimInstance* ClientArmsAnimBP;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AMultiFPSPlayerController* FPSPlayerController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveRight(float AxisValue);
	void MoveForward(float AxisValue);

	void JumpAction();
	void StopJumpAction();
	void LowSpeedWalkAction();
	void NormalSpeedWalkAction();
	void InputFirePressed();
	void InputFireReleased();

public:
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);

private:
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Test")  //Temp
	EWeaponType ActiveWeapon;

	//³õÊ¼×Ô´øÎäÆ÷
	void StartWithKindOfWeapon();

	void PurchaseWeapon(EWeaponType WeaponType);
	
	AWeaponBaseClient* GetCurrentClientFPArmsWeaponActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//²½Ç¹Éä»÷
	void FireWeaponPrimary();
	//Í£Ö¹Éä»÷
	void StopFirePrimary();

public:
	//RPCÍøÂçÍ¬²½
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalkAction();
	void ServerLowSpeedWalkAction_Implementation();
	bool ServerLowSpeedWalkAction_Validation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalkAction();
	void ServerNormalSpeedWalkAction_Implementation();
	bool ServerNormalSpeedWalkAction_Validation();

	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPriamry();

	UFUNCTION(Server, Reliable)
	void ClientFire();
};
