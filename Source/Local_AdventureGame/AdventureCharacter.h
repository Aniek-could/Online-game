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
#include "Logging/LogMacros.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "AdventureCharacter.generated.h"


class UInputMappingContext;
class UInputAction;
class UEnhancedInputComponent;
class UAnimBlueprint;


UCLASS()
class LOCAL_ADVENTUREGAME_API AAdventureCharacter : public ACharacter
{
	GENERATED_BODY()
	
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
	
protected://动画
	
	//人物idle动画
	UPROPERTY(EditAnywhere,Category=Animation)
	UAnimBlueprint*FirstPersonDefaultAnim;
	
protected://装备
	
	//装备物品
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Tools")
	TObjectPtr<AEquippableToolBase>EquippedTool;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentTool)
	TObjectPtr<AEquippableToolBase>CurrentTool=nullptr;
	
	UFUNCTION()
	void OnRep_CurrentTool();
	
	void ApplyCurrentTool();

public:
	// Sets default values for this character's properties
	AAdventureCharacter();
	
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public://动作

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//输入绑定动作
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
public://摄像机和人物
	
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
	
	//获取摄像机
	UFUNCTION()
	FVector GetCameraTargetLocation();
	
public://装备
	
	//角色已有的物品栏组件
	UPROPERTY(Replicated)
	TObjectPtr<UInventoryComponent>InventoryComponent;
	
	//检查是否已经装备该工具
	UFUNCTION()
	bool IsToolAlreadyOwned(UEquippableToolDefinition*ToolDefinition);
	
	//装备
	UFUNCTION()
	bool AttachTool(UEquippableToolDefinition*ToolDefinition);
	
	//其他类尝试向玩家授予物品时
	UFUNCTION()
	bool GiveItem(UItemDefinition*ItemDefinition);
	
public://复制
	
	//复制前提
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	
};
