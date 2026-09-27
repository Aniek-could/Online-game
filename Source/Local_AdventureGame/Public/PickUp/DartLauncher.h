// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp/EquippableToolBase.h"
#include "DartLauncher.generated.h"

class AFirstPeronProjectile;

UCLASS()
class LOCAL_ADVENTUREGAME_API ADartLauncher : public AEquippableToolBase//发射器
{
	GENERATED_BODY()
	
public:
	
	virtual void Use() override;
	
	virtual void BindInputAction(const UInputAction*ActionToBind) override;
	
	UPROPERTY(EditAnywhere,Category="Projectile")
	TSubclassOf<AFirstPeronProjectile>ProjectileClass;

protected:
	
	//客户端发送请求 → 服务器接收并执行 → 确保请求绝对不丢失。
	//FVector_NetQuantize- UE 专门为网络同步设计的“压缩版 FVector”，核心作用是大幅减少位置数据的传输带宽。
	UFUNCTION(Server,Reliable)
	void ServerUse(FVector_NetQuantize TargetLocation);
	
	void SpawnProjectile(const FVector& TargetLocation);
	
	float LastUsedTime = -100.f;
	
	bool bInputBound=false;
};
