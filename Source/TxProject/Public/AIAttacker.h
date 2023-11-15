// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSCharacterBase.h"
#include "AIAttacker.generated.h"

class UBehaviorTree;

UCLASS()
class TXPROJECT_API AAIAttacker : public AFPSCharacterBase
{
	GENERATED_BODY()
	
public:
	AAIAttacker();

	UPROPERTY(EditDefaultsOnly, Category = "BehaviorTree")
	UBehaviorTree* AttackerBT;
};
