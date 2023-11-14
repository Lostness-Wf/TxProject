// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TxProjectGameModeBase.generated.h"

class AAIController_DeathMatch;

UCLASS()
class TXPROJECT_API ATxProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ATxProjectGameModeBase();

	UFUNCTION(BlueprintCallable)
	AAIController_DeathMatch* CreateAIController();

	UFUNCTION(BlueprintCallable)
	void CreatAIAttacker();
};
