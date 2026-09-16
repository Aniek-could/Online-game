// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ItemDefinition.h"


UItemDefinition* UItemDefinition::CreateItemCopy() const
{
	UItemDefinition*ItemCopy=NewObject<UItemDefinition>(StaticClass());
	
	ItemCopy->ID = this->ID;
	ItemCopy->ItemType = this->ItemType;
	ItemCopy->ItemText = this->ItemText;
	ItemCopy->WorldMesh = this->WorldMesh;
	
	return ItemCopy;
}
