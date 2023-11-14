// Fill out your copyright notice in the Description page of Project Settings.


#include "AIAttacker.h"
#include "AIController_DeathMatch.h"

AAIAttacker::AAIAttacker()
{
	AIControllerClass = AAIController_DeathMatch::StaticClass();
}
