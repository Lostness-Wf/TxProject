// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"

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

	IsFiring = false;
	IsReloading = false;
}

void AFPSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, IsFiring, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, IsReloading, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, ActiveWeapon, COND_None);
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

	InputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AFPSCharacterBase::InputReload);
}


void AFPSCharacterBase::ResetRecoil()
{
	RecoilXCoordPerShoot	  = 0;

	NewVerticalRecoilAmount   = 0;
	OldVerticalRecoilAmount   = 0;
	VerticalRecoilAmount	  = 0;
	NewHorizontalRecoilAmount = 0;
	OldHorizontalRecoilAmount = 0;
	HorizontalRecoilAmount	  = 0;
}

void AFPSCharacterBase::DelayPlayArmReloadCallBack()
{
	//Temp
	//UE_LOG(LogTemp, Warning, TEXT("ReloadCallBack"));

	int32 GunCurrentAmmo = ServerPrimaryWeapon->GunCurrentAmmo;
	int32 CLipCurrentAmmo = ServerPrimaryWeapon->ClipCurrentAmmo;
	int32 const MaxClipAmmo = ServerPrimaryWeapon->MaxClipAmmo;

	IsReloading = false;

	//������С������װ���ӵ���ʣ�౸��ȫ��װ��
	if (MaxClipAmmo - CLipCurrentAmmo >= GunCurrentAmmo)
	{
		CLipCurrentAmmo += GunCurrentAmmo;
		GunCurrentAmmo = 0;
	}
	//����������װ�������ӵ�
	else
	{
		GunCurrentAmmo -= MaxClipAmmo - CLipCurrentAmmo;
		CLipCurrentAmmo = MaxClipAmmo;
	}

	ServerPrimaryWeapon->GunCurrentAmmo = GunCurrentAmmo;
	ServerPrimaryWeapon->ClipCurrentAmmo = CLipCurrentAmmo;

	ClientUpdateAmmoUI(CLipCurrentAmmo, GunCurrentAmmo);
}

void AFPSCharacterBase::AutoFire()
{
	//�жϵ����Ƿ�Ϊ��
	if (ServerPrimaryWeapon->ClipCurrentAmmo)
	{
		//���������ã����ٵ�ҩ�����ߣ��˺������ף��ܱ�������������ǹ��������
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}

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
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0 && !IsReloading)
	{
		//���������ã����ٵ�ҩ�����ߣ��˺������ף��ܱ�������������ǹ��������
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}

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
	//����IsFire
	ServerStopFiring();

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
			//�ܴ�
			FVector Vector = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
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
		//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit actor name : %s"), *HitResult.GetActor()->GetName()));

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
		PurchaseWeapon(TestStartWeapon);
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
			ActiveWeapon = EWeaponType::AK47;
			EquipPrimary(ServerWeapon);
		}
		break;

	case EWeaponType::M4A1:
		{
			//M4A1��ͼ·��
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/M4A1/BP_M4A1_Server.BP_M4A1_Server_C'"));
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
			ServerWeapon->EquipWeapon();
			ActiveWeapon = EWeaponType::M4A1;
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

AWeaponBaseServer* AFPSCharacterBase::GetCurrentServerTPBodysWeaponActor()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::None:
		return nullptr;
		break;
	case EWeaponType::AK47:
	{
		return ServerPrimaryWeapon;
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

	IsFiring = true;

	//��ǹ���߼��
	RifleLineTrace(CameraLocation, CameraRotation, IsMoving);

	//Temp
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo : %d"), ServerPrimaryWeapon->ClipCurrentAmmo));
}

bool AFPSCharacterBase::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSCharacterBase::ServerReloadPrimary_Implementation()
{
	//�жϵ���δ�����һ��б���
	if (ServerPrimaryWeapon->ClipCurrentAmmo < ServerPrimaryWeapon->MaxClipAmmo && ServerPrimaryWeapon->GunCurrentAmmo > 0)
	{
		//�ͻ����ֱ�Reload����
		ClientReload();
		IsReloading = true;
		//����������ಥ��������
		MultiReload();

		if (ClientPrimaryWeapon)
		{
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmsReloadMontage->GetPlayLength(), ActionInfo);
		}
	}
}

bool AFPSCharacterBase::ServerReloadPrimary_Validate()
{
	return true;
}

void AFPSCharacterBase::ServerStopFiring_Implementation()
{
	IsFiring = false;
}

bool AFPSCharacterBase::ServerStopFiring_Validate()
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

void AFPSCharacterBase::MultiReload_Implementation()
{
	//�����˳����廻�������鲥
	AWeaponBaseServer* CurrentSererWeapon = GetCurrentServerTPBodysWeaponActor();

	if (ServerBodysAnimBP)
	{
		if (CurrentSererWeapon)
		{
			ServerBodysAnimBP->Montage_Play(CurrentSererWeapon->ServerTPBodysReloadAnimMontage);
		}
	}
}

bool AFPSCharacterBase::MultiReload_Validate()
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
			
			FName WeaponSocketName = TEXT("WeaponSocket");
			if (ActiveWeapon == EWeaponType::M4A1)
			{
				WeaponSocketName = TEXT("M4A1_Socket");
			}

			ClientPrimaryWeapon->K2_AttachToComponent(FPArmsMesh, WeaponSocketName,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
		}

		//��һ�˳��ֱ۳�ǹ����
		if (ClientPrimaryWeapon)
		{
			UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPose);
		}
	}
}

void AFPSCharacterBase::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentWeapon = GetCurrentClientFPArmsWeaponActor();
	if (ClientPrimaryWeapon)
	{
		//����ǹ֧�������
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
	UCurveFloat* HorizontalRecoilCurve = nullptr;

	if (ServerPrimaryWeapon)
	{
		VerticalRecoilCurve = ServerPrimaryWeapon->VerticalRecoilCurve;
		HorizontalRecoilCurve = ServerPrimaryWeapon->HorizontalRecoilCurve;
	}

	RecoilXCoordPerShoot += 0.1;

	if (VerticalRecoilCurve)
	{
		NewVerticalRecoilAmount = VerticalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);
	}

	if (HorizontalRecoilCurve)
	{
		NewHorizontalRecoilAmount = HorizontalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);
	}

	VerticalRecoilAmount = NewVerticalRecoilAmount - OldVerticalRecoilAmount;
	HorizontalRecoilAmount = NewHorizontalRecoilAmount - OldHorizontalRecoilAmount;

	if (FPSPlayerController)
	{
		FRotator ControllerRotater = FPSPlayerController->GetControlRotation();
		FPSPlayerController->SetControlRotation(FRotator(ControllerRotater.Pitch + VerticalRecoilAmount,
			ControllerRotater.Yaw + HorizontalRecoilAmount,ControllerRotater.Roll));
	}

	OldVerticalRecoilAmount = NewVerticalRecoilAmount;
	OldHorizontalRecoilAmount = NewHorizontalRecoilAmount;
}

void AFPSCharacterBase::ClientReload_Implementation()
{
	//�ͻ����ֱ�Reload����
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();
	if (CurrentClientWeapon)
	{
		//Temp
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ClientReload")));
		UAnimMontage* ClientArmsFireMontage = CurrentClientWeapon->ClientArmsReloadMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		CurrentClientWeapon->PlayReloadAnimation();
	}
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

void AFPSCharacterBase::InputReload()
{

	if (!IsReloading)
	{
		if (!IsFiring)
		{
			switch (ActiveWeapon)
			{
			case EWeaponType::AK47:
				{
					ServerReloadPrimary();
				}
			case EWeaponType::DesertEagle:
				break;
			default:
				break;
			}
		}
	}
}
