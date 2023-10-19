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
		ServerPrimaryWeapon->K2_AttachToComponent(GetMesh(), "Weapon_Right",
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true);
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
