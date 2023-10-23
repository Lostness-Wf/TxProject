// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
AFPSCharacterBase::AFPSCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	if (PlayerCamera)
	{
		PlayerCamera->SetupAttachment(RootComponent);
		PlayerCamera->bUsePawnControlRotation = true;
	}

	FPArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArmsMesh"));
	if (FPArmsMesh)
	{
		FPArmsMesh->SetupAttachment(PlayerCamera);
		FPArmsMesh->SetOnlyOwnerSee(true);
	}
	//�����˳ƹ����Լ����ɼ�
	GetMesh()->SetOwnerNoSee(true);

	//�����˳ƹ�����ײ
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
}

// Called when the game starts or when spawned
void AFPSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	Health = 100;

	OnTakePointDamage.AddDynamic(this, &AFPSCharacterBase::OnHit);

	StartWithKindOfWeapon();
	
	ClientArmsAnimBP = FPArmsMesh->GetAnimInstance();
	ServerBodysAnimBP = GetMesh()->GetAnimInstance();

	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());

	if (FPSPlayerController)
	{
		FPSPlayerController->CreatPlayerUI();
	}
}

void AFPSCharacterBase::EquipPrimary(AWeaponBaseServer* WeaponBaseServer)
{
	if (ServerPrimaryWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon already exist"))
	}
	else
	{
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		ServerPrimaryWeapon->K2_AttachToComponent(GetMesh(), TEXT("Weapon_Right"),
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true);
		ClientEquipFPArmsPriamry();

		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
	}
}

// Called every frame
void AFPSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFPSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis(TEXT("MoveRight"), this, &AFPSCharacterBase::MoveRight);
	InputComponent->BindAxis(TEXT("MoveForward"), this, &AFPSCharacterBase::MoveForward);

	InputComponent->BindAxis(TEXT("Turn"), this, &AFPSCharacterBase::AddControllerYawInput);
	InputComponent->BindAxis(TEXT("LookUp"), this, &AFPSCharacterBase::AddControllerPitchInput);

	InputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AFPSCharacterBase::JumpAction);
	InputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AFPSCharacterBase::StopJumpAction);

	InputComponent->BindAction(TEXT("LowSpeedWalk"), IE_Pressed, this, &AFPSCharacterBase::LowSpeedWalkAction);
	InputComponent->BindAction(TEXT("LowSpeedWalk"), IE_Released, this, &AFPSCharacterBase::NormalSpeedWalkAction);

	InputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AFPSCharacterBase::InputFirePressed);
	InputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AFPSCharacterBase::InputFireReleased);
}


void AFPSCharacterBase::ResetRecoil()
{
	NewVerticalRecoilAmount = 0;
	OldVerticalRecoilAmount = 0;
	VerticalRecoilAmount = 0;
	RecoilXCoordPerShoot = 0;
}

void AFPSCharacterBase::AutoFire()
{
	//�жϵ����Ƿ�Ϊ��
	if (ServerPrimaryWeapon->ClipCurrentAmmo)
	{
		//���������ã����ٵ�ҩ�����ߣ��˺������ף��ܱ�������������ǹ��������
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);

		//�ͻ��˵��ã���ǹ�������ֱ۶����������������Ļ������������������
		ClientFire();
		
		//������
		ClientRecoil();

	}
	else
	{
		//û�ӵ��رռ�ʱ��
		GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
	}
}

void AFPSCharacterBase::FireWeaponPrimary()
{
	//�жϵ����Ƿ�Ϊ��
	if (ServerPrimaryWeapon->ClipCurrentAmmo)
	{
		//���������ã����ٵ�ҩ�����ߣ��˺������ף��ܱ�������������ǹ��������
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);

		//�ͻ��˵��ã���ǹ�������ֱ۶����������������Ļ������������������
		ClientFire();

		//������
		ClientRecoil();

		//Temp
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo : %d"), ServerPrimaryWeapon->ClipCurrentAmmo));

		if (ServerPrimaryWeapon->IsAutoGun)
		{
			//��ʼ��ʱ��
			GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AFPSCharacterBase::AutoFire, ServerPrimaryWeapon->AutoFireRate, true);
		}
	}
}

void AFPSCharacterBase::StopFirePrimary()
{
	//�رռ�ʱ��
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	//���ú�����
	ResetRecoil();

}

void AFPSCharacterBase::RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	FHitResult HitResult;

	if (ServerPrimaryWeapon)
	{
		if (IsMoving)
		{

		}
		else
		{
			EndLocation = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
		}
	}

	bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray,
		EDrawDebugTrace::None, HitResult, true, FLinearColor::Red, FLinearColor::Green, 3.f);
	 
	if (HitSuccess)
	{
		//Temp
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit actor name : %s"), *HitResult.GetActor()->GetName()));

		AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(HitResult.GetActor());

		if (FPSCharacter)
		{
			//�����Ӧ���˺�
			DamagePlayer(HitResult.PhysMaterial.Get(), HitResult.GetActor(), CameraLocation, HitResult);
		}
		else
		{
			FRotator XRotator =  UKismetMathLibrary::MakeRotFromX(HitResult.Normal);

			//�鲥��ǽ�����ɵ���
			MultiSpawnBulletDecall(HitResult.Location, XRotator);
		}

	}

}

void AFPSCharacterBase::DamagePlayer(UPhysicalMaterial* PhysicalMaterial, AActor* DamageActor, FVector& HitFromDirection, FHitResult& HitInfo)
{
	//���ݻ���λ��Ӧ�ò�ͬ�˺�ֵ
	if (ServerPrimaryWeapon)
	{
		switch (PhysicalMaterial->SurfaceType)
			{
				case SurfaceType1:
				{
					//Head
					UGameplayStatics::ApplyPointDamage(DamageActor, ServerPrimaryWeapon->BaseDamage * 4, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
		
				case SurfaceType2:
				{
					//Body
					UGameplayStatics::ApplyPointDamage(DamageActor, ServerPrimaryWeapon->BaseDamage * 1, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
		
				case SurfaceType3:
				{
					//Arm
					UGameplayStatics::ApplyPointDamage(DamageActor, ServerPrimaryWeapon->BaseDamage * 0.8, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
		
				case SurfaceType4:
				{
					//Leg
					UGameplayStatics::ApplyPointDamage(DamageActor, ServerPrimaryWeapon->BaseDamage * 0.7, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
			}
	}

}

void AFPSCharacterBase::OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	Health -= Damage;

	ClientUpdateHealthUI(Health);

	if (Health <= 0)
	{
		//��ɫ����
	}

	//Temp
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("PlayerName : %s Health : %f"),*GetName(), Health));

}

void AFPSCharacterBase::StartWithKindOfWeapon()
{
	if (HasAuthority())
	{
		PurchaseWeapon(EWeaponType::AK47);
	}
}

void AFPSCharacterBase::PurchaseWeapon(EWeaponType WeaponType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	switch (WeaponType)
	{
	case EWeaponType::None:
		{
		}
		break;

	case EWeaponType::AK47:
		{
		UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/AK47/BP_AK47_Server.BP_AK47_Server_C'"));
		AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
		ServerWeapon->EquipWeapon();
		EquipPrimary(ServerWeapon);
		}
		break;

	case EWeaponType::DesertEagle:
		{
		}
		break;

	default:
		break;
	}
}

AWeaponBaseClient* AFPSCharacterBase::GetCurrentClientFPArmsWeaponActor()
{
	switch (ActiveWeapon)
	{
		case EWeaponType::None:
			return nullptr;
			break;
		case EWeaponType::AK47:
		{
			return ClientPrimaryWeapon;
		}
			break;
		case EWeaponType::DesertEagle:
			return nullptr;
			break;
		default:
			return nullptr;
			break;
	}
}

void AFPSCharacterBase::ServerLowSpeedWalkAction_Implementation()
{
	this->GetCharacterMovement()->MaxWalkSpeed = 300;
}

bool AFPSCharacterBase::ServerLowSpeedWalkAction_Validate()
{
	return true;
}

void AFPSCharacterBase::ServerNormalSpeedWalkAction_Implementation()
{
	this->GetCharacterMovement()->MaxWalkSpeed = 600;
}

bool AFPSCharacterBase::ServerNormalSpeedWalkAction_Validate()
{
	return true;
}

void AFPSCharacterBase::ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{
		//��Ч�������鲥
		ServerPrimaryWeapon->MultiShootingEffect();
	
		//�ӵ�����-1
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
	
		//���ŵ����˳������������
		MultiShooting();

		//�ͻ���UI����
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
	}

	//��ǹ���߼��
	RifleLineTrace(CameraLocation, CameraRotation, IsMoving);

	//Temp
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo : %d"), ServerPrimaryWeapon->ClipCurrentAmmo));
}

bool AFPSCharacterBase::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSCharacterBase::MultiShooting_Implementation()
{
	if (ServerBodysAnimBP)
	{
		if (ServerPrimaryWeapon)
		{
			ServerBodysAnimBP->Montage_Play(ServerPrimaryWeapon->ServerTPBodysShootAnimMontage);
		}
	}
}

bool AFPSCharacterBase::MultiShooting_Validate()
{
	return true;
}

void AFPSCharacterBase::MultiSpawnBulletDecall_Implementation(FVector Location, FRotator Rotation)
{
	if (ServerPrimaryWeapon)
	{
		UDecalComponent* Decal =  UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ServerPrimaryWeapon->BullteDecalMaterial, FVector(8, 8, 8),
			Location, Rotation, 10);

		if (Decal)
		{
			Decal->SetFadeScreenSize(0.001);
		}
	}
}

bool AFPSCharacterBase::MultiSpawnBulletDecall_Validate(FVector Location, FRotator Rotation)
{
	return true;
}

void AFPSCharacterBase::ClientEquipFPArmsPriamry_Implementation()
{
	if (ServerPrimaryWeapon)
	{
		if (ClientPrimaryWeapon)
		{

		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerPrimaryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);
			ClientPrimaryWeapon->K2_AttachToComponent(FPArmsMesh, TEXT("WeaponSocket"),
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
		}
	}
}

void AFPSCharacterBase::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentWeapon = GetCurrentClientFPArmsWeaponActor();
	if (ClientPrimaryWeapon)
	{
		//����ǹ֧��ƶ���
		CurrentWeapon->PlayShootAnimation();

		//����ֱ۶���
		UAnimMontage* ClientArmsFireMontage = CurrentWeapon->ClientArmsFireMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1);

		//�������Ч�������������ӣ�
		CurrentWeapon->DisplayWeaponEffect();

		//Camera Shake
		FPSPlayerController->PlayerCameraShake(CurrentWeapon->CameraShakeClass);

		//׼����ɢ����
		FPSPlayerController->DoCrosshairRecoil();

	}

}

void AFPSCharacterBase::ClientUpdateAmmoUI_Implementation(int32 ClipCurrentAmmo, int32 GunCurrentAmmo)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipCurrentAmmo, GunCurrentAmmo);
	}
}

void AFPSCharacterBase::ClientUpdateHealthUI_Implementation(float NewHealth)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateHealthUI(NewHealth);
	}
}

void AFPSCharacterBase::ClientRecoil_Implementation()
{
	UCurveFloat* VerticalRecoilCurve = nullptr;

	if (ServerPrimaryWeapon)
	{
		VerticalRecoilCurve = ServerPrimaryWeapon->VerticalRecoilCurve;
	}

	RecoilXCoordPerShoot += 0.1;

	if (VerticalRecoilCurve)
	{
		NewVerticalRecoilAmount = VerticalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);
	}

	VerticalRecoilAmount = NewVerticalRecoilAmount - OldVerticalRecoilAmount;

	if (FPSPlayerController)
	{
		FRotator ControllerRotater = FPSPlayerController->GetControlRotation();
		FPSPlayerController->SetControlRotation(FRotator(ControllerRotater.Pitch + VerticalRecoilAmount,
			ControllerRotater.Yaw,ControllerRotater.Roll));
	}

	OldVerticalRecoilAmount = NewVerticalRecoilAmount;
}

void AFPSCharacterBase::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector(), AxisValue, false);
}

void AFPSCharacterBase::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector(), AxisValue, false);
}

void AFPSCharacterBase::JumpAction()
{
	Jump();
}

void AFPSCharacterBase::StopJumpAction()
{
	StopJumping();
}

void AFPSCharacterBase::LowSpeedWalkAction()
{
	this->GetCharacterMovement()->MaxWalkSpeed = 300;
	ServerLowSpeedWalkAction();
}

void AFPSCharacterBase::NormalSpeedWalkAction()
{
	this->GetCharacterMovement()->MaxWalkSpeed = 600;
	ServerNormalSpeedWalkAction();
}

void AFPSCharacterBase::InputFirePressed()
{
	switch (ActiveWeapon)
	{
		case EWeaponType::None:
		break;

		case EWeaponType::AK47:
		{
			FireWeaponPrimary();
		}
		break;

		case EWeaponType::DesertEagle:
		{
		}
		break;

		default:
		break;
	}
}

void AFPSCharacterBase::InputFireReleased()
{
	switch (ActiveWeapon)
	{
		case EWeaponType::None:
		break;

		case EWeaponType::AK47:
		{
			StopFirePrimary();
		}
		break;

		case EWeaponType::DesertEagle:
		{
		}
		break;

		default:
		break;
	}
}
