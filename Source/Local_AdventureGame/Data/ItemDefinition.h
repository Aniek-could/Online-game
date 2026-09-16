// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ItemData.h"
#include "ItemDefinition.generated.h"


//创建这个数据资产主要是因为ItemData不适合放mesh
UCLASS(BlueprintType, Blueprintable)
class LOCAL_ADVENTUREGAME_API UItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,Category="Item Data")
	FName ID;
	
	UPROPERTY(EditAnywhere,Category="Item Data")
	EItemType ItemType;
	
	UPROPERTY(EditAnywhere,Category="Item Data")
	FItemText ItemText;
	
	UPROPERTY(EditAnywhere,Category="Item Data")
	TSoftObjectPtr<UStaticMesh>WorldMesh;
	
	virtual UItemDefinition*CreateItemCopy() const;
	
};



