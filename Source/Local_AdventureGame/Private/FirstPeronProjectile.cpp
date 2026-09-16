// Fill out your copyright notice in the Description page of Project Settings.


#include "FirstPeronProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AFirstPeronProjectile::AFirstPeronProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
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
	//如果在地面
	if (FVector::DotProduct(Hit.ImpactNormal,FVector::UpVector)>0.7f)
	{
		FRotator Flat =GetActorRotation();
		Flat.Pitch=0.f;
		Flat.Roll=0.f;
		SetActorRotation(Flat);
		
		return;
	}
	
	//碰撞道人立即销毁 
	if ((OtherActor!=nullptr)&&(OtherActor!=this)&&(OtherComp->IsSimulatingPhysics()))//碰撞到的组件正在做物理模拟
	{
		//对碰撞到的物理物体施加一个冲量（瞬时推力）
		OtherComp->AddImpulseAtLocation(GetVelocity()*PhysicsForce,GetActorLocation());
		
		Destroy();
	}
}

// Called when the game starts or when spawned
void AFirstPeronProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFirstPeronProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

