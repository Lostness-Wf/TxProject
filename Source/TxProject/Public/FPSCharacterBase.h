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

	//初始自带武器
	void StartWithKindOfWeapon();

	void PurchaseWeapon(EWeaponType WeaponType);
	
	AWeaponBaseClient* GetCurrentClientFPArmsWeaponActor();
	AWeaponBaseServer* GetCurrentServerTPBodysWeaponActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//连击计时器句柄
	FTimerHandle AutoFireTimerHandle;

	//后坐曲线X坐标
	float RecoilXCoordPerShoot;

	//重置后坐
	void ResetRecoil();

	//垂直后坐
	float NewVerticalRecoilAmount;
	float OldVerticalRecoilAmount;
	float VerticalRecoilAmount;

	//水平后坐
	float NewHorizontalRecoilAmount;
	float OldHorizontalRecoilAmount;
	float HorizontalRecoilAmount;

	//沙鹰散射
	float PistolSpreadMin = 0;
	float PistolSpreadMax = 0;

	//是否在射击
	UPROPERTY(Replicated)
	bool IsFiring;

	//是否在换弹
	UPROPERTY(Replicated)
	bool IsReloading;

	//换弹结束回调函数
	UFUNCTION()
	void DelayPlayArmReloadCallBack();

	//步枪连击
	void AutoFire();

	//步枪开始射击
	void FireWeaponPrimary();

	//步枪停止射击
	void StopFirePrimary();

	//步枪射线检测
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//沙鹰开始射击
	void FireWeaponSecondary();

	//沙鹰停止射击
	void StopFireSecondary();

	//沙鹰射线检测
	void PistolLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//沙鹰散射清零
	UFUNCTION()
	void DelaySpreadWeaponShootCallBack();

	//狙击枪开枪
	void FireWeaponSniper();

	//狙击枪停止开枪
	void StopFireSniper();

	//狙击枪射线检测
	void SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UPROPERTY(Replicated)
	bool IsAiming;

	//狙击枪开镜UI
	UPROPERTY(VisibleAnywhere, Category = "SniperUI")
	UUserWidget* WidgetScope;

	UPROPERTY(EditAnywhere, Category = "SniperUI")
	TSubclassOf<UUserWidget> SniperScopeBPClass;

	//狙击枪开枪间隔
	UFUNCTION()
	void DelaySniperShootCallBack();

	//玩家伤害应用
	void DamagePlayer(UPhysicalMaterial* PhysicalMaterial, AActor* DamageActor, FVector& HitFromDirection, FHitResult& HitInfo);

	//受到应用伤害回调
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
	//静步
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalkAction();
	void ServerLowSpeedWalkAction_Implementation();
	bool ServerLowSpeedWalkAction_Validation();

	//静步还原
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalkAction();
	void ServerNormalSpeedWalkAction_Implementation();
	bool ServerNormalSpeedWalkAction_Validation();

	//服务器主武器射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//服务器副武器射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFirePistolWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFirePistolWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFirePistolWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireSniperWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//服务器主武器换弹
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadPrimary();
	void ServerReloadPrimary_Implementation();
	bool ServerReloadPrimary_Validation();

	//服务器副武器换弹
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadSecondary();
	void ServerReloadSecondary_Implementation();
	bool ServerReloadSecondary_Validation();

	//停止射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFiring();
	void ServerStopFiring_Implementation();
	bool ServerStopFiring_Validation();

	//服务器设置瞄准状态
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAiming(bool AimingState);
	void ServerSetAiming_Implementation(bool AimingState);
	bool ServerSetAiming_Validation(bool AimingState);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnGrenade();
	void ServerSpawnGrenade_Implementation();
	void ServerSpawnGrenade_Validation();

	//射击组播
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShooting();
	void MultiShooting_Implementation();
	bool MultiShooting_Validation();

	//生成弹孔组播
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiSpawnBulletDecall(FVector Location, FRotator Rotation);
	void MultiSpawnBulletDecall_Implementation(FVector Location, FRotator Rotation);
	bool MultiSpawnBulletDecall_Validation(FVector Location, FRotator Rotation);

	//第三人称身体换弹动画组播
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiReload();
	void MultiReload_Implementation();
	void MultiReload_Validation();

	//客户端装备主武器
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPriamry();

	//客户端装备副武器
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsSecondary();

	//客户端开火
	UFUNCTION(Client, Reliable)
	void ClientFire();

	//客户端更新子弹数量UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//客户端更新血量UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateHealthUI(float NewHealth, AActor* DamageActor);

	//客户端射击后坐力旋转镜头
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
