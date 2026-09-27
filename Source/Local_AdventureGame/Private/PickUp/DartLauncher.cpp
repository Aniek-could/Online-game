// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/DartLauncher.h"
#include "AdventureCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "FirstPeronProjectile.h"
#include  "EnhancedInputComponent.h"

void ADartLauncher::Use()
{
	if (!IsValid(OwningCharacter)||!IsValid(ProjectileClass)||!GetWorld()) return;
	
	// 获取的是 自游戏运行以来的绝对时间
	const float CurrentTime=GetWorld()->GetTimeSeconds();
	
	if (CurrentTime-LastUsedTime<UseCooldown) return;
	LastUsedTime=CurrentTime;
	
	const FVector TargetLocation = OwningCharacter->GetCameraTargetLocation();
	
	if (HasAuthority())
	{
		SpawnProjectile(TargetLocation);
	}
	else
	{
		ServerUse(TargetLocation);
	}
}

void ADartLauncher::BindInputAction(const UInputAction* ActionToBind)
{
	Super::BindInputAction(ActionToBind);
	
	if (bInputBound||!ActionToBind||!IsValid(OwningCharacter)||!OwningCharacter->IsLocallyControlled()) return;
	
	if (APlayerController*PlayerController=Cast<APlayerController>(OwningCharacter->GetController()))
	{
		if (!PlayerController) return;
		
		if (UEnhancedInputComponent*EnhancedInputComponent=Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(ActionToBind,ETriggerEvent::Triggered,this,&ADartLauncher::Use);
			bInputBound=true;
		}
	}
	
	
}

void ADartLauncher::SpawnProjectile(const FVector& TargetLocation)
{
	if (!HasAuthority()||!IsValid(OwningCharacter)||!ProjectileClass||!ToolMeshComponent) return;
	
	UWorld*World =GetWorld();
	if (!World) return;
	
	FVector SocketLocation=ToolMeshComponent->GetSocketLocation("Muzzle");
		
	//FindLookAtRotation 会计算并返回在 SocketLocation 处面对 TargetPosition 所需的旋转。
	FRotator SpawnRotator=UKismetMathLibrary::FindLookAtRotation(SocketLocation,TargetLocation);
	 
	FVector SpawnLocation= SocketLocation+UKismetMathLibrary::GetForwardVector(SpawnRotator)*10.0f;
	
	//结构体
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.Owner = OwningCharacter;
	ActorSpawnParams.Instigator = OwningCharacter;
	//生成 Actor 时，先尝试挪个位置避开碰撞；如果挪不开，就干脆别生成了。"
	ActorSpawnParams.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
        
	World->SpawnActor<AFirstPeronProjectile>(ProjectileClass,SpawnLocation,SpawnRotator,ActorSpawnParams);
}

void ADartLauncher::ServerUse_Implementation(FVector_NetQuantize TargetLocation)
{
	if (!HasAuthority()||!IsValid(OwningCharacter)||!ProjectileClass||!GetWorld()) return;
	
	if (GetOwner()!=OwningCharacter) return;	
	
	const float CurrentTime=GetWorld()->GetTimeSeconds();
	
	if (CurrentTime-LastUsedTime<UseCooldown) return;
	LastUsedTime=CurrentTime;
	
	SpawnProjectile(TargetLocation);
}


