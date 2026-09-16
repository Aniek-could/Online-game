// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "AdventureCharacter.h"
#include "PickupBase.generated.h"

class UItemDefinition;
inline FTimerHandle RespawnTimerHandle;

UCLASS(BlueprintType, Blueprintable)
class LOCAL_ADVENTUREGAME_API APickupBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupBase();
	
	void InitializePickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditInstanceOnly,Category="Pickup | Item Table")
	FName PickupItemID;
	
	UPROPERTY(EditInstanceOnly,Category="Pickup | Item Table")
	TSoftObjectPtr<UDataTable>PickupDataTable;
	
	UPROPERTY(VisibleAnywhere,Category="Pickup | Item Table")
	TObjectPtr<UItemDefinition>ReferenceItem;
	
	UPROPERTY(VisibleDefaultsOnly,Category="Pickup | Item Table")//关卡实例面板看不见
	TObjectPtr<UStaticMeshComponent>PickupMeshComponent;
	
	
	//碰撞事件
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Pickup | Components")
	TObjectPtr<USphereComponent>SphereComponent;
	
	UFUNCTION(BlueprintCallable,Category="Pickup | Item Table")
	void OnSphereBeginOverlap(UPrimitiveComponent*OverlappedComponent,AActor*OtherActor,UPrimitiveComponent*OtherComp,int32 OtherBoxIndex,bool bFromSweep,const FHitResult& SweepResult);
	
	//重新生成
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Pickup | ReSpawn")
	bool bShouldReSpawn;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Pickup | ReSpawn")
	float ReSpawnTime=4.f;
	
public:
#if WITH_EDITOR//这段代码只在编辑器环境下编译，打包成最终游戏（Shipping）时，请直接把这段代码删掉。

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
