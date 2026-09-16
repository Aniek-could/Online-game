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
};
