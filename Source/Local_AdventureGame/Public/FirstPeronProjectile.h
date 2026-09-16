// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FirstPeronProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS(BlueprintType,Blueprintable)//像蓝图公开
class LOCAL_ADVENTUREGAME_API AFirstPeronProjectile : public AActor//飞镖实体
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFirstPeronProjectile();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Projectile | Physics")
	float PhysicsForce=100.f;
	
	//HitComp：被击中的组件;OtherActor：被击中的Actor;OtherComp：造成碰撞的组件（在本例中为发射物的碰撞组件）
	//NormalImpulse：碰撞的法线冲量;Hit：一个FHitResult引用，包含有关碰撞事件的更多数据，如时间、距离和位置
	UFUNCTION()
	void OnHit(UPrimitiveComponent*HitComp,AActor*OtherActor,UPrimitiveComponent*OtherComp,FVector NormalImpulse,const FHitResult& Hit);

   UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Projectile | Mesh")
	TObjectPtr<UStaticMeshComponent>ProjectileMesh;
	
	//生命周期
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Projectile | Lifespan")
	float ProjectileLifespan=5.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Projectile | Components")
	TObjectPtr<USphereComponent>CollisionComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Projectile | Components")
	TObjectPtr<UProjectileMovementComponent>ProjectileMovement;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
