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

	//ǹ�ڻ���
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//ǹ��
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	//�����������˵������ʣ�����ӵ���
	UPROPERTY(EditAnywhere)
	int32 GunCurrentAmmo;

	//����ʣ���ӵ�
	UPROPERTY(EditAnywhere, Replicated)
	int32 ClipCurrentAmmo;

	//�����������
	UPROPERTY(EditAnywhere)
	int32 MaxClipAmmo;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysShootAnimMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerTPBodysReloadAnimMontage;

	UPROPERTY(EditAnywhere)
	float BulletDistance;

	//��������
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BullteDecalMaterial;

	//�����˺�
	UPROPERTY(EditAnywhere)
	float BaseDamage;

	//�Ƿ�Ϊ�Զ���ǹ���Ƿ��������
	UPROPERTY(EditAnywhere)
	bool IsAutoGun;

	//�������
	UPROPERTY(EditAnywhere)
	float AutoFireRate;

	//��ֱ����������
	UPROPERTY(EditAnywhere)
	UCurveFloat* VerticalRecoilCurve;

	//��ֱ����������
	UPROPERTY(EditAnywhere)
	UCurveFloat* HorizontalRecoilCurve;

	//�ܴ������Χ
	UPROPERTY(EditAnywhere)
	float MovingFireRandomRange;

	UFUNCTION(Server, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validation();
};
