// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

class UItemDefinition;

/**
 * 
 */
UENUM()
enum class EItemType : uint8//物品类型
{
	Tool UMETA(DisplayName = "Tool"),
	Consumable UMETA(DisplayName = "Consumable")//消耗品
};

USTRUCT()
struct FItemText//物品描述
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FText Name;
	
	UPROPERTY(EditAnywhere)
	FText Description;
};

USTRUCT()
struct  FItemData:public FTableRowBase//物品数据（id,类型，描述）
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,Category="Item Data")
	FName ID;
	
	UPROPERTY(EditAnywhere,Category="Item Data")
	EItemType ItemType;
	
	UPROPERTY(EditAnywhere,Category="Item Data")
	FItemText ItemText;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	TObjectPtr<UItemDefinition>ItemBase;//让当前对象持有一个对 UItemDefinition 数据资产的强引用
	
};

class LOCAL_ADVENTUREGAME_API ItemData 
{
public:
	ItemData();
	~ItemData();
};