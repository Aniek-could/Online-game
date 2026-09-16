// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/DartLauncher.h"
#include "AdventureCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "FirstPeronProjectile.h"
#include  "EnhancedInputComponent.h"

void ADartLauncher::Use()
{
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Blue,TEXT("DartLaucher is Loading"));
	
	UWorld* const World = GetWorld();
	if (World!=nullptr&&ProjectileClass!=nullptr)
	{
		FVector TargetLocation = OwningCharacter->GetCameraTargetLocation();
		
		FVector SocketLocation=ToolMeshComponent->GetSocketLocation("Muzzle");
		
		//FindLookAtRotation 会计算并返回在 SocketLocation 处面对 TargetPosition 所需的旋转。
		FRotator SpawnRotator=UKismetMathLibrary::FindLookAtRotation(SocketLocation,TargetLocation);
	 
		FVector SpawnLocation= SocketLocation+UKismetMathLibrary::GetForwardVector(SpawnRotator)*10.0;
	
		//结构体
		FActorSpawnParameters ActorSpawnParams;
		//生成 Actor 时，先尝试挪个位置避开碰撞；如果挪不开，就干脆别生成了。"
		ActorSpawnParams.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
        
		World->SpawnActor<AFirstPeronProjectile>(ProjectileClass,SpawnLocation,SpawnRotator,ActorSpawnParams);
	}
}

void ADartLauncher::BindInputAction(const UInputAction* ActionToBind)
{
	Super::BindInputAction(ActionToBind);
	if (APlayerController*PlayerController=Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (UEnhancedInputComponent*EnhancedInputComponent=Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(ActionToBind,ETriggerEvent::Triggered,this,&ADartLauncher::Use);
		}
	}
}


