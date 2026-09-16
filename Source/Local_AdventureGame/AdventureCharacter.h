// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputActionValue.h"
#include "PickUp/EquippableToolBase.h"
#include "Data/EquippableToolDefinition.h"
#include "Public/PickUp/InventoryComponent.h"
#include "Data/ItemData.h"
#include "AdventureCharacter.generated.h"


class UInputMappingContext;
class UInputAction;
class UEnhancedInputComponent;
class UAnimBlueprint;


UCLASS()
class LOCAL_ADVENTUREGAME_API AAdventureCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAdventureCharacter();
	
	virtual void BeginPlay() override;

protected:
	
	// 输入
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input)
	TObjectPtr<UInputMappingContext>FirstPersonContext;
    
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input)
	TObjectPtr<UInputAction>MoveAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input)
	TObjectPtr<UInputAction>JumpAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input)
	TObjectPtr<UInputAction>LookAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Input)
	TObjectPtr<UInputAction>UseAction;
	
	//人物idle动画
	UPROPERTY(EditAnywhere,Category=Animation)
	UAnimBlueprint*FirstPersonDefaultAnim;
	
	//装备物品
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Tools")
	TObjectPtr<AEquippableToolBase>EquippedTool;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//输入绑定动作
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	//摄像机
	UPROPERTY(VisibleAnywhere,Category=Camera)
	UCameraComponent*FirstPersonCameraComponent;
	
	UPROPERTY(EditAnywhere,Category=Camera)
	float FirstPersonFieldOfView=70.f;
	
	UPROPERTY(EditAnywhere,Category=Camera)
	float FirstPersonViewScale=0.6f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector FirstPersonCameraOffset = FVector(0.0f,30.0f,0.0f);
	
	//网格体
	UPROPERTY(VisibleAnywhere,Category=Mesh)
	USkeletalMeshComponent* FirstPersonMeshComponent ;
	
	//角色已有的物品栏组件
	UPROPERTY(VisibleAnywhere,Category="Inventory")
	TObjectPtr<UInventoryComponent>InventoryComponent;
	
	//检查是否已经装备该工具
	UFUNCTION()
	bool IsToolAlreadyOwned(UEquippableToolDefinition*ToolDefinition);
	
	//装备
	UFUNCTION()
	void AttachTool(UEquippableToolDefinition*ToolDefinition);
	
	//其他类尝试向玩家授予物品时
	UFUNCTION()
	void GiveItem(UItemDefinition*ItemDefinition);
	
	//获取摄像机
	UFUNCTION()
	FVector GetCameraTargetLocation();
	
};
