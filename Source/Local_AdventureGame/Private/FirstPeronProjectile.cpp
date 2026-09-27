// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPeronProjectile.h"

#include "AdventureCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AFirstPeronProjectile::AFirstPeronProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//所有客户端都看得见
	bReplicates=true;
	SetReplicateMovement(true);
	
	CollisionComponent=CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	check(CollisionComponent!=nullptr);
   
	ProjectileMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	check(ProjectileMesh!=nullptr);
	
	ProjectileMovement=CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    check(ProjectileMovement!=nullptr);
	
	ProjectileMesh->SetupAttachment(CollisionComponent);
	CollisionComponent->InitSphereRadius(5.f);
	
	//将碰撞组件的碰撞配置文件名称设置为"Projectile"。
	CollisionComponent->BodyInstance.SetCollisionProfileName("Projectile");
	
	CollisionComponent->OnComponentHit.AddDynamic(this,&AFirstPeronProjectile::OnHit);
	
	//碰撞检测也以CollisionComponent为中心
	RootComponent=CollisionComponent;
	
	//告诉 ProjectileMovementComponent 每帧应该移动哪个组件。
	ProjectileMovement->UpdatedComponent=CollisionComponent;
	
	ProjectileMovement->InitialSpeed=3000.f;
	ProjectileMovement->MaxSpeed=3000.f;
	//对象是否应旋转以匹配速度的方向
	ProjectileMovement->bRotationFollowsVelocity=true;
	//反弹力
	ProjectileMovement->Bounciness=0.2f;
	ProjectileMovement->Friction=0.8f;
	
	//设置生命周期
	InitialLifeSpan=ProjectileLifespan;
}


void AFirstPeronProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()||!IsValid(OtherActor)||OtherActor==this||OtherActor==GetInstigator()) return;
	//如果在地面
	
	if (AAdventureCharacter*HitCharacter=Cast<AAdventureCharacter>(OtherActor))
	{
		if (HitCharacter->GetMovementComponent())
		{
			FVector LaunchVelocity=GetVelocity().GetSafeNormal()*CharacterKnockSpeed;
			LaunchVelocity.Z=FMath::Max(LaunchVelocity.Z,CharacterKnockbackUpward);
			HitCharacter->LaunchCharacter(LaunchVelocity,true,true);
		}
		
		Destroy();
		return;
	}
	
	if (IsValid(OtherComp)&OtherComp->IsSimulatingPhysics())//碰撞到的组件正在做物理模拟
	{
		//对碰撞到的物理物体施加一个冲量（瞬时推力）
		OtherComp->AddImpulseAtLocation(GetVelocity()*PhysicsForce,Hit.ImpactPoint);
		
		Destroy();
	}
}

// Called when the game starts or when spawned
void AFirstPeronProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(GetInstigator()))
	{
		//忽略发射者
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(),true);
	}
}

// Called every frame
void AFirstPeronProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

