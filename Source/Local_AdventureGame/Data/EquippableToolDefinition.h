// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDefinition.h"
#include "EquippableToolDefinition.generated.h"

class AEquippableToolBase;
class UInputMappingContext;

UCLASS(BlueprintType, Blueprintable)
class LOCAL_ADVENTUREGAME_API UEquippableToolDefinition : public UItemDefinition
{
	GENERATED_BODY()
	
public:
	//该属性只能在蓝图的"类默认值"（Class Defaults）面板中编辑，当蓝图被实例化到场景中后，该属性在实例的细节面板中不可编辑（会显示为灰色或根本不显示）。
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AEquippableToolBase>ToolAsset;//TSubclassOf-子类也可以
	
	
	UFUNCTION()
	virtual UEquippableToolDefinition*CreateItemCopy() const override;
};
