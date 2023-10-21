// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	//第三人称骨骼自己不可见
	GetMesh()->SetOwnerNoSee(true);

	//第三人称骨骼碰撞
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
}

// Called when the game starts or when spawned
void AFPSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	StartWithKindOfWeapon();
	
	ClientArmsAnimBP = FPArmsMesh->GetAnimInstance();

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


void AFPSCharacterBase::FireWeaponPrimary()
{
	//服务器调用，减少弹药，射线，伤害，弹孔，能被所有人听到开枪声和粒子
	ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);

	//客户端调用，开枪动画，手臂动画，射击声音，屏幕抖动，后坐力，粒子
	ClientFire();
}

void AFPSCharacterBase::StopFirePrimary()
{

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
	//RPC组播
	ServerPrimaryWeapon->MultiShootingEffect();
}

bool AFPSCharacterBase::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
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
		//播放枪支设计动画
		CurrentWeapon->PlayShootAnimation();

		//射击手臂动画
		UAnimMontage* ClientArmsFireMontage = CurrentWeapon->ClientArmsFireMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1);

		//本地射击效果（声音，粒子）
		CurrentWeapon->DisplayWeaponEffect();

		//Camera Shake
		FPSPlayerController->PlayerCameraShake(CurrentWeapon->CameraShakeClass);

		//准星扩散动画
		FPSPlayerController->DoCrosshairRecoil();

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
