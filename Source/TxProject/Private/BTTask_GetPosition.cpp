// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GetPosition.h"
#include "AIController_DeathMatch.h"
#include "FPSCharacterBase.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "AIModule/Classes/BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

EBTNodeResult::Type UBTTask_GetPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController_DeathMatch* ALC = Cast<AAIController_DeathMatch>(OwnerComp.GetOwner());
	if (ALC)
	{
		APawn* AIAttacker = ALC->GetPawn();
		AFPSCharacterBase* Enemy = ALC->GetEnemy();
		if (AIAttacker && Enemy)
		{
			const FVector Origin = ALC->GetOrigin(Enemy, AIAttacker);
			const float Radius = 200.0f;

			UNavigationSystemV1* NavMesh = FNavigationSystem::GetCurrent<UNavigationSystemV1>(ALC);
			if (NavMesh)
			{
				FNavLocation Loc;
				NavMesh->GetRandomReachablePointInRadius(Origin, Radius, Loc);
				if (Loc.Location != FVector::ZeroVector)
				{
					OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), Loc.Location);
					return EBTNodeResult::Succeeded;
				}
			}
		}
	}

	return EBTNodeResult::Failed;
}
