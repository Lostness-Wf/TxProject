// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIController_DeathMatch.generated.h"
#include "Bunker.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;
class AFPSCharacterBase;

UCLASS()
class TXPROJECT_API AAIController_DeathMatch : public AAIController
{
	GENERATED_BODY()
	
public:
	AAIController_DeathMatch();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void FindEnemy(AFPSCharacterBase* Enemy);

	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void ShootEnemy();

	AFPSCharacterBase* GetEnemy();

private:
	ABunker* CalcPointNearEnemy(ABunker* SpActor, AFPSCharacterBase* Enemy, APawn* AIAttacker);
	ABunker* FindPointNear(FVector Loc);
	FVector CalcPointFarEnemy(ABunker* SpActor, AFPSCharacterBase* Enemy, APawn* AIAttacker);

	bool HasEnemy(AActor* Enemy);
	void SetEnemy(APawn* InPawn);

	UBlackboardComponent* BBComp;
	UBehaviorTreeComponent* BTComp;
	int32 EnemyKeyID;
};
