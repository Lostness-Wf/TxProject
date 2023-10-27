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

	//是否在射击
	UPROPERTY(Replicated)
	bool IsFiring;

	//是否在换弹
	UPROPERTY(Replicated)
	bool IsReloading;

	//步枪连击
	void AutoFire();

	//步枪开始射击
	void FireWeaponPrimary();

	//步枪停止射击
	void StopFirePrimary();

	//步枪射线检测
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//玩家伤害应用
	void DamagePlayer(UPhysicalMaterial* PhysicalMaterial, AActor* DamageActor, FVector& HitFromDirection, FHitResult& HitInfo);

	//受到应用伤害回调
	UFUNCTION()
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	float Health;

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

	//服务器射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//服务器换弹
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadPrimary();
	void ServerReloadPrimary_Implementation();
	bool ServerReloadPrimary_Validation();

	//停止射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFiring();
	void ServerStopFiring_Implementation();
	bool ServerStopFiring_Validation();

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

	//客户端装备武器
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPriamry();

	//客户端开火
	UFUNCTION(Client, Reliable)
	void ClientFire();

	//客户端更新子弹数量UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//客户端更新血量UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateHealthUI(float NewHealth);

	//客户端射击后坐力旋转镜头
	UFUNCTION(Client, Reliable)
	void ClientRecoil();

	UFUNCTION(Client, Reliable)
	void ClientReload();
};
