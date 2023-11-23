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

	UFUNCTION()
	void DelayBeginPlayCallBack();

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

	//Temp
	UPROPERTY(EditAnywhere)
	EWeaponType TestStartWeapon;

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void InputMoveRight(float AxisValue);

	UFUNCTION(BlueprintCallable)
	void InputMoveForward(float AxisValue);

	UFUNCTION(BlueprintCallable)
	void InputTurn(float AxisValue);

	UFUNCTION(BlueprintCallable)
	void InputLookUp(float AxisValue);

	UFUNCTION(BlueprintCallable)
	void JumpAction();

	UFUNCTION(BlueprintCallable)
	void StopJumpAction();

	UFUNCTION(BlueprintCallable)
	void LowSpeedWalkAction();

	UFUNCTION(BlueprintCallable)
	void NormalSpeedWalkAction();

	UFUNCTION(BlueprintCallable)
	void InputFirePressed();

	UFUNCTION(BlueprintCallable)
	void InputFireReleased();

	UFUNCTION(BlueprintCallable)
	void InputReload();

	UFUNCTION(BlueprintCallable)
	void InputAimingPressed();

	UFUNCTION(BlueprintCallable)
	void InputAimingReleased();

	UFUNCTION(BlueprintCallable)
	bool IsCurrentWeaponSniper();

	UFUNCTION(BlueprintCallable)
	void InputFireGrenade();

public:
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);

	void EquipSrcondary(AWeaponBaseServer* WeaponBaseServer);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateFPArmsBlendPose(int NewIndex);

public:
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerSecondaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientSecondaryWeapon;

	UPROPERTY(meta = (AllowPrivateAccess = "true"), Replicated)
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

	//ɳӥɢ��
	float PistolSpreadMin = 0;
	float PistolSpreadMax = 0;

	//�Ƿ������
	UPROPERTY(Replicated)
	bool IsFiring;

	//�Ƿ��ڻ���
	UPROPERTY(Replicated)
	bool IsReloading;

	//���������ص�����
	UFUNCTION()
	void DelayPlayArmReloadCallBack();

	//��ǹ����
	void AutoFire();

	//��ǹ��ʼ���
	void FireWeaponPrimary();

	//��ǹֹͣ���
	void StopFirePrimary();

	//��ǹ���߼��
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//ɳӥ��ʼ���
	void FireWeaponSecondary();

	//ɳӥֹͣ���
	void StopFireSecondary();

	//ɳӥ���߼��
	void PistolLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//ɳӥɢ������
	UFUNCTION()
	void DelaySpreadWeaponShootCallBack();

	//�ѻ�ǹ��ǹ
	void FireWeaponSniper();

	//�ѻ�ǹֹͣ��ǹ
	void StopFireSniper();

	//�ѻ�ǹ���߼��
	void SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UPROPERTY(Replicated)
	bool IsAiming;

	//�ѻ�ǹ����UI
	UPROPERTY(VisibleAnywhere, Category = "SniperUI")
	UUserWidget* WidgetScope;

	UPROPERTY(EditAnywhere, Category = "SniperUI")
	TSubclassOf<UUserWidget> SniperScopeBPClass;

	//�ѻ�ǹ��ǹ���
	UFUNCTION()
	void DelaySniperShootCallBack();

	//����˺�Ӧ��
	void DamagePlayer(UPhysicalMaterial* PhysicalMaterial, AActor* DamageActor, FVector& HitFromDirection, FHitResult& HitInfo);

	//�ܵ�Ӧ���˺��ص�
	UFUNCTION()
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION()
	void GrenadeExplosion();

	UPROPERTY(EditAnywhere, Category = "Health")
	float Health = 100;

	UPROPERTY(BlueprintReadOnly)
	bool IsDead = false;

	UPROPERTY(EditAnywhere,Category = "Sound")
	USoundBase* HeadSound;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MulticastPlayHeadSound();
	void MulticastPlayHeadSound_Implementation();
	void MulticastPlayHeadSound_Validation();

	void DeathMatchDeath(AActor* DamageCauser);

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

	//���������������
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//���������������
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFirePistolWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFirePistolWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFirePistolWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireSniperWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//����������������
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadPrimary();
	void ServerReloadPrimary_Implementation();
	bool ServerReloadPrimary_Validation();

	//����������������
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadSecondary();
	void ServerReloadSecondary_Implementation();
	bool ServerReloadSecondary_Validation();

	//ֹͣ���
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFiring();
	void ServerStopFiring_Implementation();
	bool ServerStopFiring_Validation();

	//������������׼״̬
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAiming(bool AimingState);
	void ServerSetAiming_Implementation(bool AimingState);
	bool ServerSetAiming_Validation(bool AimingState);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnGrenade();
	void ServerSpawnGrenade_Implementation();
	void ServerSpawnGrenade_Validation();

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

	//�ͻ���װ��������
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPriamry();

	//�ͻ���װ��������
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsSecondary();

	//�ͻ��˿���
	UFUNCTION(Client, Reliable)
	void ClientFire();

	//�ͻ��˸����ӵ�����UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//�ͻ��˸���Ѫ��UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateHealthUI(float NewHealth, AActor* DamageActor);

	//�ͻ��������������ת��ͷ
	UFUNCTION(Client, Reliable)
	void ClientRecoil();

	UFUNCTION(Client, Reliable)
	void ClientReload();

	UFUNCTION(Client, Reliable)
	void ClientAiming();
	
	UFUNCTION(Client, Reliable)
	void ClientEndAiming();

	UFUNCTION(Client, Reliable)
	void ClientDeathMatchDeath();
};
