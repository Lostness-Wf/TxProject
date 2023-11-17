// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController_DeathMatch.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "AIModule/Classes/BehaviorTree/BehaviorTreeComponent.h"
#include "AIAttacker.h"
#include "AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "FPSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "EngineUtils.h"

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

void AAIController_DeathMatch::FindEnemy()
{
	AAIAttacker* AIAttacker = Cast<AAIAttacker>(GetPawn());
	if (AIAttacker)
	{
		AFPSCharacterBase* Enemy = Cast<AFPSCharacterBase>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (Enemy)
		{
			if (HasEnemy(Enemy))
			{
				//Enemy¿É¼û
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
				else if (AIAttacker->ServerPrimaryWeapon && AIAttacker->ServerPrimaryWeapon->ClipCurrentAmmo <= 0 && !AIAttacker->IsReloading)
				{
					bCanFire = false;
					AIAttacker->InputReload();
				}
			}
		}

		bCanFire ? AIAttacker->InputFirePressed() : AIAttacker->InputFireReleased();
	}
}

//ABunker* AAIController_DeathMatch::FindSplineActor()
//{
//
//}

AFPSCharacterBase* AAIController_DeathMatch::GetEnemy()
{
	if (BBComp)
	{
		return Cast<AFPSCharacterBase>(BBComp->GetValue<UBlackboardKeyType_Object>(EnemyKeyID));
	}

	return nullptr;
}

FVector AAIController_DeathMatch::GetOrigin(AFPSCharacterBase* Enemy, APawn* AIAttacker)
{
	ABunker* SpActor = FindPointNear(AIAttacker->GetActorLocation());
	if (SpActor)
	{
		if (!HasEnemy(Enemy))
		{
			SpActor = CalcPointNearEnemy(SpActor, Enemy, AIAttacker);
		}
		return CalcPointFarEnemy(SpActor, Enemy, AIAttacker);
	}

	return FVector::ZeroVector;
}

void AAIController_DeathMatch::StopBehavior()
{
	AAIAttacker* AIAttacker = Cast<AAIAttacker>(GetPawn());
	AIAttacker->InputFireReleased();
	BTComp->StopTree();
	ClearFocus(EAIFocusPriority::LastFocusPriority);
}

ABunker* AAIController_DeathMatch::CalcPointNearEnemy(ABunker* SpActor, AFPSCharacterBase* Enemy, APawn* AIAttacker)
{
	float BestDisSq = MAX_FLT;
	ABunker* BestActor = SpActor;
	const FVector SpLoc = SpActor->GetActorLocation();
	const FVector EnemyLoc = Enemy->GetActorLocation();
	const FVector AILoc = AIAttacker->GetActorLocation();

	const float SpToEnemyDis = (EnemyLoc - SpLoc).Size();
	const float AIToEnemyDis = (AILoc - EnemyLoc).Size();

	const ABunker* NearSpForEnemy = FindPointNear(EnemyLoc);
	const float AIToNearActorForEnemyDis = (AILoc - NearSpForEnemy->GetActorLocation()).Size();
	const float DisValue = abs(AIToNearActorForEnemyDis - AIToEnemyDis);

	for (TActorIterator<ABunker>it(GetWorld()); it; ++it)
	{
		if (*it != SpActor)
		{
			const float Dis = (EnemyLoc - it->GetActorLocation()).Size();
			if (Dis < SpToEnemyDis)
			{
				const float SplineDis = (SpLoc - it->GetActorLocation()).Size();
				if (SplineDis < BestDisSq)
				{
					const float AIToItDis = (AILoc - it->GetActorLocation()).Size();
					if (AIToItDis <= AIToNearActorForEnemyDis)
					{
						if (*it == NearSpForEnemy)
						{
							if (AIToNearActorForEnemyDis < AIToEnemyDis && DisValue > 500.0f)
							{
								BestDisSq = SplineDis;
								BestActor = *it;
							}
						}
						else
						{
							BestDisSq = SplineDis;
							BestActor = *it;
						}
					}
				}
			}
		}
	}

	return BestActor;
}

ABunker* AAIController_DeathMatch::FindPointNear(FVector Loc)
{
	float BestDisSq = MAX_FLT;
	ABunker* BestActor = nullptr;
	for (TActorIterator<ABunker>it(GetWorld()); it; ++it)
	{
		const float Dis = (Loc - it->GetActorLocation()).SizeSquared();
		if (Dis < BestDisSq)
		{
			BestDisSq = Dis;
			BestActor = *it;
		}
	}

	return BestActor;
}

FVector AAIController_DeathMatch::CalcPointFarEnemy(ABunker* SpActor, AFPSCharacterBase* Enemy, APawn* AIAttacker)
{
	float MinDisSq = 0.0f;
	FVector MinDisVec = FVector::ZeroVector;
	FVector Point= FVector::ZeroVector;
	const FVector EnemyLoc = Enemy->GetActorLocation();
	const FVector AILoc = AIAttacker->GetActorLocation();

	USplineComponent* SpComp = SpActor->GetSplineComp();
	for (int32 Count = 0; Count < SpComp->GetNumberOfSplinePoints() - 1; Count++)
	{
		Point = SpComp->GetLocationAtSplinePoint(Count, ESplineCoordinateSpace::World);
		const float DisSq = (EnemyLoc - Point).SizeSquared();
		if (DisSq > MinDisSq)
		{
			MinDisSq = DisSq;
			MinDisVec = Point;
		}
	}

	const ABunker* NearActor = FindPointNear(EnemyLoc);
	if (NearActor == SpActor)
	{
		const FVector NearActorLoc = NearActor->GetActorLocation();
		if ((EnemyLoc - NearActorLoc).Size() > 2000.0f)
		{
			MinDisVec = EnemyLoc + 1500.0f * (AILoc - EnemyLoc).GetSafeNormal();
		}
	}

	return MinDisVec;
}

bool AAIController_DeathMatch::HasEnemy(AActor* Enemy)
{
	static FName Tag = FName(TEXT("HasEnemy"));
	FCollisionQueryParams Params(Tag, true, GetPawn());
	Params.bReturnPhysicalMaterial = true;

	APawn* AIAttacker = GetPawn();
	FVector Start = AIAttacker->GetActorLocation();
	Start.Z += 90;
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
