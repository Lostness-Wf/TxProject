// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/SphereComponent.h"
#include "WeaponBaseClient.h"

#include "WeaponBaseServer.generated.h"

UENUM()
enum class EWeaponType : uint8
{
	None = 0,
	AK47 UMETA(DisplayName = "Ak47"),
	DesertEagle UMETA(DisplayName = "DesertEagle")
};

UCLASS()
class TXPROJECT_API AWeaponBaseServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBaseServer();

	UPROPERTY(EditAnywhere)
	EWeaponType KindOfWeapon;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	USphereComponent* SphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBaseClient> ClientWeaponBaseBPClass;

	UFUNCTION()
	void OnOtherBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EquipWeapon();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//枪口火焰
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//枪声
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	//备弹量（除了弹夹里，还剩多少子弹）
	UPROPERTY(EditAnywhere)
	int32 GunCurrentAmmo;

	//弹夹剩余子弹
	UPROPERTY(EditAnywhere, Replicated)
	int32 ClipCurrentAmmo;

	//弹夹最大容量
	UPROPERTY(EditAnywhere)
	int32 MaxClipAmmo;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysShootAnimMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysReloadAnimMontage;

	UPROPERTY(EditAnywhere)
	float BulletDistance;

	//弹孔贴花
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BullteDecalMaterial;

	//基础伤害
	UPROPERTY(EditAnywhere)
	float BaseDamage;

	//是否为自动步枪（是否可连击）
	UPROPERTY(EditAnywhere)
	bool IsAutoGun;

	//连击间隔
	UPROPERTY(EditAnywhere)
	float AutoFireRate;

	//垂直后坐力曲线
	UPROPERTY(EditAnywhere)
	UCurveFloat* VerticalRecoilCurve;

	//垂直后坐力曲线
	UPROPERTY(EditAnywhere)
	UCurveFloat* HorizontalRecoilCurve;

	//跑打随机范围
	UPROPERTY(EditAnywhere)
	float MovingFireRandomRange;

	UFUNCTION(Server, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validation();
};
