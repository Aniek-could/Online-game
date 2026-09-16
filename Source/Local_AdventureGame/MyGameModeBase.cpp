// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"

 void AMyGameModeBase::StartPlay()
{
	Super::StartPlay();
 	
 	check(GEngine != nullptr);//检查，check(变量)√
 	
 	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Green,TEXT("Hello World, this is AdventureGameMode!"));
}