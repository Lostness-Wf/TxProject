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

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ClientArmsAnimBP;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ServerBodysAnimBP;

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

	void InputReload();

public:
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);

private:
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Test")  //Temp
	EWeaponType ActiveWeapon;

	//��ʼ�Դ�����
	void StartWithKindOfWeapon();

	void PurchaseWeapon(EWeaponType WeaponType);
	
	AWeaponBaseClient* GetCurrentClientFPArmsWeaponActor();
	AWeaponBaseServer* GetCurrentServerTPBodysWeaponActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//������ʱ�����
	FTimerHandle AutoFireTimerHandle;

	//��������X����
	float RecoilXCoordPerShoot;

	//���ú���
	void ResetRecoil();

	//��ֱ����
	float NewVerticalRecoilAmount;
	float OldVerticalRecoilAmount;
	float VerticalRecoilAmount;

	//ˮƽ����
	float NewHorizontalRecoilAmount;
	float OldHorizontalRecoilAmount;
	float HorizontalRecoilAmount;

	//�Ƿ������
	UPROPERTY(Replicated)
	bool IsFiring;

	//�Ƿ��ڻ���
	UPROPERTY(Replicated)
	bool IsReloading;

	//��ǹ����
	void AutoFire();

	//��ǹ��ʼ���
	void FireWeaponPrimary();

	//��ǹֹͣ���
	void StopFirePrimary();

	//��ǹ���߼��
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//����˺�Ӧ��
	void DamagePlayer(UPhysicalMaterial* PhysicalMaterial, AActor* DamageActor, FVector& HitFromDirection, FHitResult& HitInfo);

	//�ܵ�Ӧ���˺��ص�
	UFUNCTION()
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	float Health;

public:
	//����
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalkAction();
	void ServerLowSpeedWalkAction_Implementation();
	bool ServerLowSpeedWalkAction_Validation();

	//������ԭ
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalkAction();
	void ServerNormalSpeedWalkAction_Implementation();
	bool ServerNormalSpeedWalkAction_Validation();

	//���������
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//����������
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadPrimary();
	void ServerReloadPrimary_Implementation();
	bool ServerReloadPrimary_Validation();

	//ֹͣ���
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFiring();
	void ServerStopFiring_Implementation();
	bool ServerStopFiring_Validation();

	//����鲥
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShooting();
	void MultiShooting_Implementation();
	bool MultiShooting_Validation();

	//���ɵ����鲥
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiSpawnBulletDecall(FVector Location, FRotator Rotation);
	void MultiSpawnBulletDecall_Implementation(FVector Location, FRotator Rotation);
	bool MultiSpawnBulletDecall_Validation(FVector Location, FRotator Rotation);

	//�����˳����廻�������鲥
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiReload();
	void MultiReload_Implementation();
	void MultiReload_Validation();

	//�ͻ���װ������
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPriamry();

	//�ͻ��˿���
	UFUNCTION(Client, Reliable)
	void ClientFire();

	//�ͻ��˸����ӵ�����UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//�ͻ��˸���Ѫ��UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateHealthUI(float NewHealth);

	//�ͻ��������������ת��ͷ
	UFUNCTION(Client, Reliable)
	void ClientRecoil();

	UFUNCTION(Client, Reliable)
	void ClientReload();
};
