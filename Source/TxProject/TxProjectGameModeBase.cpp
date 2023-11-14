// Copyright Epic Games, Inc. All Rights Reserved.


#include "TxProjectGameModeBase.h"
#include "AIController_DeathMatch.h"
#include "AIAttacker.h"

ATxProjectGameModeBase::ATxProjectGameModeBase()
{

}

AAIController_DeathMatch* ATxProjectGameModeBase::CreateAIController()
{
	FActorSpawnParameters Spawn;
	Spawn.Instigator = nullptr;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAIController_DeathMatch* ALC = GetWorld()->SpawnActor<AAIController_DeathMatch>(Spawn);

	return ALC;
}

void ATxProjectGameModeBase::CreatAIAttacker()
{
	for (FConstControllerIterator it = GetWorld()->GetControllerIterator(); it; ++it)
	{
		AAIController_DeathMatch* ALC = Cast<AAIController_DeathMatch>(*it);
		if (ALC)
		{
			RestartPlayer(ALC);
		}
	}
}
