// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/EquippableToolBase.h"

// Sets default values
AEquippableToolBase::AEquippableToolBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ToolMeshComponent=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ToolMesh"));
	check(ToolMeshComponent!=nullptr);
}

void AEquippableToolBase::Use()
{
}

void AEquippableToolBase::BindInputAction(const UInputAction* ActionToBind)
{
}

// Called when the game starts or when spawned
void AEquippableToolBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEquippableToolBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

