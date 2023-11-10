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
#include "UMG/Public/Blueprint/UserWidget.h"

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

void AFPSCharacterBase::DelayBeginPlayCallBack()
{
	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());

	if (FPSPlayerController)
	{
		FPSPlayerController->CreatPlayerUI();
	}
	else
	{
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		ActionInfo.ExecutionFunction = TEXT("DelayBeginPlayCallBack");
		ActionInfo.UUID = FMath::Rand();
		ActionInfo.Linkage = 0;
		UKismetSystemLibrary::Delay(this, 0.5, ActionInfo);
	}
}

// Called when the game starts or when spawned
void AFPSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	Health = 100;
	IsAiming = false;
	IsFiring = false;
	IsReloading = false;

	OnTakePointDamage.AddDynamic(this, &AFPSCharacterBase::OnHit);

	ClientArmsAnimBP = FPArmsMesh->GetAnimInstance();
	ServerBodysAnimBP = GetMesh()->GetAnimInstance();

	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());

	if (FPSPlayerController)
	{
		FPSPlayerController->CreatPlayerUI();
	}
	else
	{
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		ActionInfo.ExecutionFunction = TEXT("DelayBeginPlayCallBack");
		ActionInfo.UUID = FMath::Rand();
		ActionInfo.Linkage = 0;
		UKismetSystemLibrary::Delay(this, 0.5, ActionInfo);
	}

	StartWithKindOfWeapon();
}

void AFPSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, IsFiring, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, IsReloading, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, ActiveWeapon, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase, IsAiming, COND_None);
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
		ActiveWeapon = ServerPrimaryWeapon->KindOfWeapon;
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
	}
}

void AFPSCharacterBase::EquipSrcondary(AWeaponBaseServer* WeaponBaseServer)
{
	if (ServerSecondaryWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon already exist"))
	}
	else
	{
		ServerSecondaryWeapon = WeaponBaseServer;
		ServerSecondaryWeapon->SetOwner(this);
		ServerSecondaryWeapon->K2_AttachToComponent(GetMesh(), TEXT("Weapon_Right"),
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true);
		ClientEquipFPArmsSecondary();
		ActiveWeapon = ServerSecondaryWeapon->KindOfWeapon;
		ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);
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
	
	//InputComponent->BindAxis(TEXT("MoveRight"), this, &AFPSCharacterBase::MoveRight);
	//InputComponent->BindAxis(TEXT("MoveForward"), this, &AFPSCharacterBase::MoveForward);

	InputComponent->BindAxis(TEXT("Turn"), this, &AFPSCharacterBase::Turn);
	InputComponent->BindAxis(TEXT("LookUp"), this, &AFPSCharacterBase::LookUp);

	InputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AFPSCharacterBase::JumpAction);
	InputComponent->BindAction(TEXT("Jump"), IE_Released, this, &AFPSCharacterBase::StopJumpAction);

	InputComponent->BindAction(TEXT("LowSpeedWalk"), IE_Pressed, this, &AFPSCharacterBase::LowSpeedWalkAction);
	InputComponent->BindAction(TEXT("LowSpeedWalk"), IE_Released, this, &AFPSCharacterBase::NormalSpeedWalkAction);

	InputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AFPSCharacterBase::InputFirePressed);
	InputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AFPSCharacterBase::InputFireReleased);

	InputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AFPSCharacterBase::InputReload);

	InputComponent->BindAction(TEXT("Aiming"), IE_Pressed, this, &AFPSCharacterBase::InputAimingPressed);
	InputComponent->BindAction(TEXT("Aiming"), IE_Released, this, &AFPSCharacterBase::InputAimingReleased);
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

	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();

	if (CurrentServerWeapon)
	{
		int32 GunCurrentAmmo = CurrentServerWeapon->GunCurrentAmmo;
		int32 CLipCurrentAmmo = CurrentServerWeapon->ClipCurrentAmmo;
		int32 const MaxClipAmmo = CurrentServerWeapon->MaxClipAmmo;

		IsReloading = false;

		//备弹数小于所需装填子弹，剩余备弹全部装填
		if (MaxClipAmmo - CLipCurrentAmmo >= GunCurrentAmmo)
		{
			CLipCurrentAmmo += GunCurrentAmmo;
			GunCurrentAmmo = 0;
		}
		//备弹数大于装填所需子弹
		else
		{
			GunCurrentAmmo -= MaxClipAmmo - CLipCurrentAmmo;
			CLipCurrentAmmo = MaxClipAmmo;
		}

		CurrentServerWeapon->GunCurrentAmmo = GunCurrentAmmo;
		CurrentServerWeapon->ClipCurrentAmmo = CLipCurrentAmmo;

		ClientUpdateAmmoUI(CLipCurrentAmmo, GunCurrentAmmo);
	}
}

void AFPSCharacterBase::AutoFire()
{
	//判断弹夹是否为空
	if (ServerPrimaryWeapon->ClipCurrentAmmo)
	{
		//服务器调用，减少弹药，射线，伤害，弹孔，能被所有人听到开枪声和粒子
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}

		//客户端调用，开枪动画，手臂动画，射击声音，屏幕抖动，后坐力，粒子
		ClientFire();
		
		//后坐力
		ClientRecoil();

	}
	else
	{
		//没子弹关闭计时器
		GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
	}
}

void AFPSCharacterBase::FireWeaponPrimary()
{
	//判断弹夹是否为空
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0 && !IsReloading)
	{
		//服务器调用，减少弹药，射线，伤害，弹孔，能被所有人听到开枪声和粒子
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}

		//客户端调用，开枪动画，手臂动画，射击声音，屏幕抖动，后坐力，粒子
		ClientFire();

		//后坐力
		ClientRecoil();

		//Temp
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo : %d"), ServerPrimaryWeapon->ClipCurrentAmmo));

		if (ServerPrimaryWeapon->IsAutoGun)
		{
			//开始计时器
			GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &AFPSCharacterBase::AutoFire, ServerPrimaryWeapon->AutoFireRate, true);
		}
	}
}

void AFPSCharacterBase::StopFirePrimary()
{
	//更改IsFire
	ServerStopFiring();

	//关闭计时器
	GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);

	//重置后座力
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
			//跑打
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
			//打到玩家应用伤害
			DamagePlayer(HitResult.PhysMaterial.Get(), HitResult.GetActor(), CameraLocation, HitResult);
		}
		else
		{
			FRotator XRotator =  UKismetMathLibrary::MakeRotFromX(HitResult.Normal);

			//组播打到墙壁生成弹孔
			MultiSpawnBulletDecall(HitResult.Location, XRotator);
		}

	}

}

void AFPSCharacterBase::FireWeaponSecondary()
{
	//判断弹夹是否为空
	if (ServerSecondaryWeapon->ClipCurrentAmmo > 0 && !IsReloading)
	{
		//服务器调用，减少弹药，射线，伤害，弹孔，能被所有人听到开枪声和粒子
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}

		//客户端调用，开枪动画，手臂动画，射击声音，屏幕抖动，后坐力，粒子
		ClientFire();
	}
}

void AFPSCharacterBase::StopFireSecondary()
{
	//更改IsFire
	ServerStopFiring();
}

void AFPSCharacterBase::PistolLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVector;
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	FHitResult HitResult;

	if (ServerSecondaryWeapon)
	{
		if (IsMoving)
		{
			//跑打
			FRotator Rotator;
			Rotator.Roll = CameraRotation.Roll;
			Rotator.Pitch = CameraRotation.Pitch + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin, PistolSpreadMax);
			Rotator.Yaw = CameraRotation.Yaw + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin, PistolSpreadMax);
			CameraForwardVector = UKismetMathLibrary::GetForwardVector(Rotator);

			FVector Vector = CameraLocation + CameraForwardVector * ServerSecondaryWeapon->BulletDistance;
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
			EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
		}
		else
		{
			//沙鹰连续射击精准度下降，停止射击精准度恢复
			FRotator Rotator;
			Rotator.Roll = CameraRotation.Roll;
			Rotator.Pitch = CameraRotation.Pitch + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin, PistolSpreadMax);
			Rotator.Yaw = CameraRotation.Yaw + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin, PistolSpreadMax);
			CameraForwardVector = UKismetMathLibrary::GetForwardVector(Rotator);

			EndLocation = CameraLocation + CameraForwardVector * ServerSecondaryWeapon->BulletDistance;
		}
	}

	bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray,
		EDrawDebugTrace::None, HitResult, true, FLinearColor::Red, FLinearColor::Green, 3.f);

	PistolSpreadMin -= ServerSecondaryWeapon->SpreadWeaponMaxIndex;
	PistolSpreadMax += ServerSecondaryWeapon->SpreadWeaponMinIndex;

	if (HitSuccess)
	{
		AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(HitResult.GetActor());

		if (FPSCharacter)
		{
			//打到玩家应用伤害
			DamagePlayer(HitResult.PhysMaterial.Get(), HitResult.GetActor(), CameraLocation, HitResult);
		}
		else
		{
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);

			//组播打到墙壁生成弹孔
			MultiSpawnBulletDecall(HitResult.Location, XRotator);
		}
	}
}

void AFPSCharacterBase::DelaySpreadWeaponShootCallBack()
{
	PistolSpreadMin = 0;
	PistolSpreadMax = 0;
}

void AFPSCharacterBase::FireWeaponSniper()
{
	//判断弹夹是否为空
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0 && !IsReloading && !IsFiring)
	{
		//服务器调用，减少弹药，射线，伤害，弹孔，能被所有人听到开枪声和粒子
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireSniperWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), true);
		}
		else
		{
			ServerFireSniperWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), false);
		}

		//客户端调用，开枪动画，手臂动画，射击声音，屏幕抖动，后坐力，粒子
		ClientFire();
	}
}

void AFPSCharacterBase::StopFireSniper()
{

}

void AFPSCharacterBase::SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	FHitResult HitResult;

	if (ServerPrimaryWeapon)
	{
		//是否开镜
		if (IsAiming)
		{
			if (IsMoving)
			{
				//开镜跑打
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
			ClientEndAiming();
		}
		//没开镜
		else
		{
			FVector Vector = CameraLocation + CameraForwardVector * ServerPrimaryWeapon->BulletDistance;
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
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
			//打到玩家应用伤害
			DamagePlayer(HitResult.PhysMaterial.Get(), HitResult.GetActor(), CameraLocation, HitResult);
		}
		else
		{
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);

			//组播打到墙壁生成弹孔
			MultiSpawnBulletDecall(HitResult.Location, XRotator);
		}
	}
}

void AFPSCharacterBase::DelaySniperShootCallBack()
{
	IsFiring = false;
}

void AFPSCharacterBase::DamagePlayer(UPhysicalMaterial* PhysicalMaterial, AActor* DamageActor, FVector& HitFromDirection, FHitResult& HitInfo)
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();

	//根据击中位置应用不同伤害值
	if (CurrentServerWeapon)
	{
		switch (PhysicalMaterial->SurfaceType)
			{
				case SurfaceType1:
				{
					//Head
					UGameplayStatics::ApplyPointDamage(DamageActor, CurrentServerWeapon->BaseDamage * 4, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
		
				case SurfaceType2:
				{
					//Body
					UGameplayStatics::ApplyPointDamage(DamageActor, CurrentServerWeapon->BaseDamage * 1, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
		
				case SurfaceType3:
				{
					//Arm
					UGameplayStatics::ApplyPointDamage(DamageActor, CurrentServerWeapon->BaseDamage * 0.8, HitFromDirection, HitInfo, GetController(),
						this, UDamageType::StaticClass());
				}
				break;
		
				case SurfaceType4:
				{
					//Leg
					UGameplayStatics::ApplyPointDamage(DamageActor, CurrentServerWeapon->BaseDamage * 0.7, HitFromDirection, HitInfo, GetController(),
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
		//角色死亡
		DeathMatchDeath(DamageCauser);
	}

	//Temp
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("PlayerName : %s Health : %f"),*GetName(), Health));

}

void AFPSCharacterBase::DeathMatchDeath(AActor* DamageActor)
{	
	ClientDeathMatchDeath();

	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();
	AWeaponBaseServer* CurrentServerWeapon =  GetCurrentServerTPBodysWeaponActor();

	if (CurrentClientWeapon)
	{
		CurrentClientWeapon->Destroy();
	}

	if (CurrentServerWeapon)
	{
		CurrentServerWeapon->Destroy();
	}

	AMultiFPSPlayerController* MultiFPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());
	if (MultiFPSPlayerController)
	{
		MultiFPSPlayerController->DeathMatchDeath(DamageActor);
	}
}

void AFPSCharacterBase::StartWithKindOfWeapon()
{
	if (HasAuthority())
	{
		//随机武器
		PurchaseWeapon(static_cast<EWeaponType>(UKismetMathLibrary::RandomIntegerInRange(0, static_cast<int8>(EWeaponType::EEnd) - 1)));
	}
}

void AFPSCharacterBase::PurchaseWeapon(EWeaponType WeaponType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;	

	switch (WeaponType)
	{
		case EWeaponType::AK47:
			{
				UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/AK47/BP_AK47_Server.BP_AK47_Server_C'"));
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
			}
			break;

		case EWeaponType::M4A1:
			{
				UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/M4A1/BP_M4A1_Server.BP_M4A1_Server_C'"));
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
			}
			break;

		case EWeaponType::MP7:
			{
				UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/MP7/BP_MP7_Server.BP_MP7_Server_C'"));
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
			}
			break;

		case EWeaponType::DesertEagle:
			{
				UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/DesertEagle/BP_DesertEagle_Server.BP_DesertEagle_Server_C'"));
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
				ServerWeapon->EquipWeapon();
				EquipSrcondary(ServerWeapon);
			}
			break;

		case EWeaponType::Sniper:
			{
				UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr, TEXT("/Script/Engine.Blueprint'/Game/Blueprint/Weapon/Sniper/BP_Sniper_Server.BP_Sniper_Server_C'"));
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SpawnInfo);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
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
		case EWeaponType::AK47:
			{
				return ClientPrimaryWeapon;
			}
			break;

		case EWeaponType::M4A1:
			{
				return ClientPrimaryWeapon;
			}
			break;

		case EWeaponType::MP7:
			{
				return ClientPrimaryWeapon;
			}
			break;

		case EWeaponType::DesertEagle:
			{
				return ClientSecondaryWeapon;
			}
			break;

		case EWeaponType::Sniper:
			{
				return ClientPrimaryWeapon;
			}
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

		case EWeaponType::AK47:
			{
				return ServerPrimaryWeapon;
			}
			break;

		case EWeaponType::M4A1:
			{
				return ServerPrimaryWeapon;
			}
			break;

		case EWeaponType::MP7:
			{
				return ServerPrimaryWeapon;
			}
			break;

		case EWeaponType::DesertEagle:
			{
				return ServerSecondaryWeapon;
			}
			break;

		case EWeaponType::Sniper:
			{
				return ServerPrimaryWeapon;
			}
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
		//特效和声音组播
		ServerPrimaryWeapon->MultiShootingEffect();
	
		//子弹数量-1
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
	
		//播放第三人称身体射击动画
		MultiShooting();

		//客户端UI更新
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
	}

	IsFiring = true;

	//步枪射线检测
	RifleLineTrace(CameraLocation, CameraRotation, IsMoving);

	//Temp
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo : %d"), ServerPrimaryWeapon->ClipCurrentAmmo));
}

bool AFPSCharacterBase::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSCharacterBase::ServerFirePistolWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerSecondaryWeapon)
	{
		//散射清零计数器
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		ActionInfo.ExecutionFunction = TEXT("DelaySpreadWeaponShootCallBack");
		ActionInfo.UUID = FMath::Rand();
		ActionInfo.Linkage = 0;
		UKismetSystemLibrary::Delay(this, ServerSecondaryWeapon->SpreadWeaponCallBackRate, ActionInfo);

		//特效和声音组播
		ServerSecondaryWeapon->MultiShootingEffect();

		//子弹数量-1
		ServerSecondaryWeapon->ClipCurrentAmmo -= 1;

		//播放第三人称身体射击动画
		MultiShooting();

		//客户端UI更新
		ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);
	}

	IsFiring = true;

	PistolLineTrace(CameraLocation, CameraRotation, IsMoving);
}

bool AFPSCharacterBase::ServerFirePistolWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSCharacterBase::ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{

		//特效和声音组播
		ServerPrimaryWeapon->MultiShootingEffect();

		//子弹数量-1
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;

		//播放第三人称身体射击动画
		MultiShooting();

		//客户端UI更新
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
	}

	if (ClientPrimaryWeapon)
	{
		//散射清零计数器
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		ActionInfo.ExecutionFunction = TEXT("DelaySniperShootCallBack");
		ActionInfo.UUID = FMath::Rand();
		ActionInfo.Linkage = 0;
		UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmsFireMontage->GetPlayLength() / 1.5, ActionInfo);
	}

	IsFiring = true;

	//步枪射线检测
	SniperLineTrace(CameraLocation, CameraRotation, IsMoving);
}

bool AFPSCharacterBase::ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSCharacterBase::ServerReloadPrimary_Implementation()
{
	//判断弹夹未满而且还有备弹
	if (ServerPrimaryWeapon->ClipCurrentAmmo < ServerPrimaryWeapon->MaxClipAmmo && ServerPrimaryWeapon->GunCurrentAmmo > 0)
	{
		//客户端手臂Reload动画
		ClientReload();
		IsReloading = true;

		//服务器身体多播换弹动画
		MultiReload();

		if (ClientPrimaryWeapon)
		{
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmsReloadMontage->GetPlayLength() / 1.5, ActionInfo);
		}
	}
}

bool AFPSCharacterBase::ServerReloadPrimary_Validate()
{
	return true;
}

void AFPSCharacterBase::ServerReloadSecondary_Implementation()
{
	//判断弹夹未满而且还有备弹
	if (ServerSecondaryWeapon->ClipCurrentAmmo < ServerSecondaryWeapon->MaxClipAmmo && ServerSecondaryWeapon->GunCurrentAmmo > 0)
	{
		//客户端手臂Reload动画
		ClientReload();
		IsReloading = true;
		//服务器身体多播换弹动画
		MultiReload();

		if (ClientSecondaryWeapon)
		{
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientSecondaryWeapon->ClientArmsReloadMontage->GetPlayLength() / 1.5, ActionInfo);
		}
	}
}

bool AFPSCharacterBase::ServerReloadSecondary_Validate()
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

void AFPSCharacterBase::ServerSetAiming_Implementation(bool AimingState)
{
	IsAiming = AimingState;
}

bool AFPSCharacterBase::ServerSetAiming_Validate(bool AimingState)
{
	return true;
}

void AFPSCharacterBase::MultiShooting_Implementation()
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();
	if (ServerBodysAnimBP)
	{
		if (CurrentServerWeapon)
		{
			ServerBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPBodysShootAnimMontage);
		}
	}
}

bool AFPSCharacterBase::MultiShooting_Validate()
{
	return true;
}

void AFPSCharacterBase::MultiSpawnBulletDecall_Implementation(FVector Location, FRotator Rotation)
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();
	if (CurrentServerWeapon)
	{
		UDecalComponent* Decal =  UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CurrentServerWeapon->BullteDecalMaterial, FVector(5, 5, 5),
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
	//第三人称身体换弹动画组播
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
			if (ActiveWeapon == EWeaponType::Sniper)
			{
				WeaponSocketName = TEXT("AWP_Socket");
			}

			ClientPrimaryWeapon->K2_AttachToComponent(FPArmsMesh, WeaponSocketName,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
		}

		//第一人称手臂持枪动画
		if (ClientPrimaryWeapon)
		{
			UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPose);
		}
	}
}

void AFPSCharacterBase::ClientEquipFPArmsSecondary_Implementation()
{
	if (ServerSecondaryWeapon)
	{
		if (ClientSecondaryWeapon)
		{

		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientSecondaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerSecondaryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);

			FName WeaponSocketName = TEXT("WeaponSocket");

			ClientSecondaryWeapon->K2_AttachToComponent(FPArmsMesh, WeaponSocketName,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
		}

		//第一人称手臂持枪动画
		if (ClientSecondaryWeapon)
		{
			UpdateFPArmsBlendPose(ClientSecondaryWeapon->FPArmsBlendPose);
		}
	}
}

void AFPSCharacterBase::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentWeapon = GetCurrentClientFPArmsWeaponActor();
	if (CurrentWeapon)
	{
		//播放枪支射击动画
		CurrentWeapon->PlayShootAnimation();

		//射击手臂动画
		UAnimMontage* ClientArmsFireMontage = CurrentWeapon->ClientArmsFireMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage, 1);

		//本地射击效果（声音，粒子）
		CurrentWeapon->DisplayWeaponEffect();

		//Camera Shake
		AMultiFPSPlayerController* MultiFPSController = Cast<AMultiFPSPlayerController>(GetController());
		if (MultiFPSController)
		{
			MultiFPSController->PlayerCameraShake(CurrentWeapon->CameraShakeClass);
			//准星扩散动画
			MultiFPSController->DoCrosshairRecoil();
		}
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
	//客户端手臂Reload动画
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();
	if (CurrentClientWeapon)
	{
		//Temp
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ClientReload")));
		UAnimMontage* ClientArmsFireMontage = CurrentClientWeapon->ClientArmsReloadMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage, 1.5);
		CurrentClientWeapon->PlayReloadAnimation();
		UGameplayStatics::PlaySound2D(GetWorld(), CurrentClientWeapon->ReloadSound);
	}
}

void AFPSCharacterBase::ClientAiming_Implementation()
{
	FPArmsMesh->SetHiddenInGame(true);

	if (ClientPrimaryWeapon)
	{
		ClientPrimaryWeapon->SetActorHiddenInGame(true);
		PlayerCamera->SetFieldOfView(ClientPrimaryWeapon->AimingFOV);
		UGameplayStatics::PlaySound2D(GetWorld(), ClientPrimaryWeapon->AnimInSound);
	}

	WidgetScope = CreateWidget<UUserWidget>(GetWorld(), SniperScopeBPClass);
	WidgetScope->AddToViewport();
}

void AFPSCharacterBase::ClientEndAiming_Implementation()
{
	FPArmsMesh->SetHiddenInGame(false);

	if (ClientPrimaryWeapon)
	{
		ClientPrimaryWeapon->SetActorHiddenInGame(false);
		PlayerCamera->SetFieldOfView(90);
		UGameplayStatics::PlaySound2D(GetWorld(), ClientPrimaryWeapon->AnimOutSound);
	}

	if (WidgetScope)
	{
		WidgetScope->RemoveFromParent();
	}
}

void AFPSCharacterBase::ClientDeathMatchDeath_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeaponActor();

	if (CurrentClientWeapon)
	{
		CurrentClientWeapon->Destroy();
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

void AFPSCharacterBase::Turn(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

void AFPSCharacterBase::LookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
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
		case EWeaponType::AK47:
			{
				FireWeaponPrimary();
			}
			break;

		case EWeaponType::DesertEagle:
			{
				FireWeaponSecondary();
			}
			break;

		case EWeaponType::M4A1:
			{
				FireWeaponPrimary();
			}
			break;

		case EWeaponType::MP7:
			{
				FireWeaponPrimary();
			}
			break;

		case EWeaponType::Sniper:
			{
				FireWeaponSniper();
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
		case EWeaponType::AK47:
			{
				StopFirePrimary();
			}
			break;

		case EWeaponType::DesertEagle:
			{
				StopFireSecondary();
			}
			break;

		case EWeaponType::M4A1:
			{
				StopFirePrimary();
			}
			break;

		case EWeaponType::MP7:
			{
				StopFirePrimary();
			}
			break;

		case EWeaponType::Sniper:
			{
				StopFireSniper();
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
				break;

			case EWeaponType::M4A1:
				{
					ServerReloadPrimary();
				}
				break;

			case EWeaponType::MP7:
				{
					ServerReloadPrimary();
				}
				break;

			case EWeaponType::DesertEagle:
				{
					ServerReloadSecondary();
				}
				break;

			case EWeaponType::Sniper:
				{
					ServerReloadPrimary();
				}
				break;

			default:
				break;
			}
		}
	}
}

void AFPSCharacterBase::InputAimingPressed()
{
	if (ActiveWeapon == EWeaponType::Sniper)
	{
		ServerSetAiming(true);
		ClientAiming();
	}
}

void AFPSCharacterBase::InputAimingReleased()
{
	if (ActiveWeapon == EWeaponType::Sniper)
	{
		ServerSetAiming(false);
		ClientEndAiming();
	}
}

bool AFPSCharacterBase::IsCurrentWeaponSniper()
{
	if (ActiveWeapon == EWeaponType::Sniper)
	{
		return true;
	}
	else
	{
		return false;
	}
}
