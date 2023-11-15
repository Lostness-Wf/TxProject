// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController_DeathMatch.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "AIModule/Classes/BehaviorTree/BehaviorTreeComponent.h"
#include "AIAttacker.h"
#include "AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "FPSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

AAIController_DeathMatch::AAIController_DeathMatch()
{
	BBComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BBComp"));
	BTComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BTComp"));

	EnemyKeyID = 0;
	UE_LOG(LogTemp, Warning, TEXT("AAIController_DeathMatch"))
}

void AAIController_DeathMatch::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("OnPossess"))

	AAIAttacker* Attacker = Cast<AAIAttacker>(InPawn);
	if (Attacker && Attacker->AttackerBT)
	{
		if (Attacker->AttackerBT->BlackboardAsset)
		{
			BBComp->InitializeBlackboard(*Attacker->AttackerBT->BlackboardAsset);
		}

		EnemyKeyID = BBComp->GetKeyID("Enemy");
		BTComp->StartTree(*Attacker->AttackerBT);
	}
}

void AAIController_DeathMatch::OnUnPossess()
{
	Super::OnUnPossess();

	BTComp->StopTree();
}

void AAIController_DeathMatch::FindEnemy(AFPSCharacterBase* Enemy)
{
	AAIAttacker* AIAttacker = Cast<AAIAttacker>(GetPawn());
	if (AIAttacker)
	{
		AFPSCharacterBase* Enemy = Cast<AFPSCharacterBase>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (Enemy)
		{
			if (HasEnemy(Enemy))
			{
				//Enemy�ɼ�
				SetEnemy(Enemy);
			}
		}
	}
}

void AAIController_DeathMatch::ShootEnemy()
{
	AAIAttacker* AIAttacker = Cast<AAIAttacker>(GetPawn());
	if (AIAttacker)
	{
		bool bCanFire = false;
		AFPSCharacterBase* Enemy = GetEnemy();
		if (Enemy)
		{
			if (LineOfSightTo(Enemy, AIAttacker->GetActorLocation()))
			{
				if (AIAttacker->ServerPrimaryWeapon && AIAttacker->ServerPrimaryWeapon->ClipCurrentAmmo > 0 && !AIAttacker->IsReloading)
				{
					bCanFire = true;
				}
			}
		}

		bCanFire ? AIAttacker->InputFirePressed() : AIAttacker->InputFireReleased();
	}
}

AFPSCharacterBase* AAIController_DeathMatch::GetEnemy()
{
	if (BBComp)
	{
		return Cast<AFPSCharacterBase>(BBComp->GetValue<UBlackboardKeyType_Object>(EnemyKeyID));
	}

	return nullptr;
}

bool AAIController_DeathMatch::HasEnemy(AActor* Enemy)
{
	static FName Tag = FName(TEXT("HasEnemy"));
	FCollisionQueryParams Params(Tag, true, GetPawn());
	Params.bReturnPhysicalMaterial = true;

	APawn* AIAttacker = GetPawn();
	FVector Start = AIAttacker->GetActorLocation();
	Start.Z += 88;
	const FVector End = Enemy->GetActorLocation();

	FHitResult HitResult(ForceInit);
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray,
		EDrawDebugTrace::None, HitResult, true, FLinearColor::Red, FLinearColor::Green, 3.f);

	if (HitSuccess)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor == Enemy)
		{
			return true;
		}
	}

	return false;
}

void AAIController_DeathMatch::SetEnemy(APawn* InPawn)
{
	if (BBComp)
	{
		BBComp->SetValue<UBlackboardKeyType_Object>(EnemyKeyID, InPawn);
		SetFocus(InPawn);
	}
}
