// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PickUp/PickupBase.h"
#include "UI_ShootingBase.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS(Abstract)
class LOCAL_ADVENTUREGAME_API UUI_ShootingBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	//meta = (BindWidget) 会强制要求 WBP 里存在同名同类型的控件,否则蓝图编译报错
	UPROPERTY(BlueprintReadOnly,meta=(BindWidget))
	TObjectPtr<UButton>PickupButton;
	
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void InitPickupWidget(AAdventureCharacter* InCharacter, APickupBase* InPickup, const FText& InName, const FText& InDesc);

	UPROPERTY() TObjectPtr<AAdventureCharacter> OwnerCharacter;   // 标记 UPROPERTY 防止被 GC
	UPROPERTY() TObjectPtr<APickupBase> OwnerPickup;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> ItemNameText;
	
protected:
	
	// 是“控件实例初始化完成”时调用
	virtual void NativeOnInitialized() override;
	
	UFUNCTION()
	void OnPickupButtonClicked();
};
