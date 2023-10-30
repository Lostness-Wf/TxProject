// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KismetMultiFPSLibrary.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FDeathMatchPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName PlayerName;

	UPROPERTY(BlueprintReadWrite)
	int PlayerScore;

	FDeathMatchPlayerData()
	{
		PlayerName = " ";
		PlayerScore = 0;
	}
};

UCLASS()
class TXPROJECT_API UKismetMultiFPSLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
