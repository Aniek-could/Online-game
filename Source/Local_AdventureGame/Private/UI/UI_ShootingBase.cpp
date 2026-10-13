// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UI_ShootingBase.h"
#include "Components/TextBlock.h"
#include "AdventureCharacter.h"
#include "Components/Button.h"

void UUI_ShootingBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (PickupButton)
	{
		PickupButton->OnClicked.AddDynamic(this,&UUI_ShootingBase::OnPickupButtonClicked);
	}
}

void UUI_ShootingBase::OnPickupButtonClicked()
{
	if (IsValid(OwnerCharacter)&&IsValid(OwnerPickup))
	{
		OwnerCharacter->ServerRequestPickup(OwnerPickup);
	}
}

void UUI_ShootingBase::InitPickupWidget(AAdventureCharacter* InCharacter, APickupBase* InPickup,
										const FText& InName, const FText& InDesc)
{
	OwnerCharacter = InCharacter;
	OwnerPickup    = InPickup;
	if (ItemNameText) ItemNameText->SetText(InName);
}