// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickupBase.h"
#include "Data/ItemDefinition.h"
#include "UI/UI_ShootingBase.h"


// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	bReplicates=true;
	SetReplicateMovement(false);
	PrimaryActorTick.bCanEverTick = false;
	
	PickupMeshComponent=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	check(PickupMeshComponent!=nullptr);

	SphereComponent=CreateDefaultSubobject<USphereComponent>(FName("SphereComponent"));
	check(SphereComponent!=nullptr);
	SphereComponent->SetupAttachment(PickupMeshComponent);
	SphereComponent->SetSphereRadius(32.f);
	
	PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickupBase::InitializePickup()
{
	if (PickupDataTable.IsNull() || PickupItemID.IsNone()) return;
	
	UDataTable*Table=PickupDataTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load pickup data table: %s"),
		 *PickupDataTable.ToString());
		
		return;
	}
	
	{
		const FItemData*ItemDataRow=Table->FindRow<FItemData>(PickupItemID,PickupItemID.ToString());
		if (!ItemDataRow || !IsValid(ItemDataRow->ItemBase.Get())) return;
		
		UItemDefinition*TempItemDefinition=ItemDataRow->ItemBase.Get();
		
		ReferenceItem=TempItemDefinition->CreateItemCopy();//更新
		
		if (TempItemDefinition->WorldMesh.IsValid())
		{
			PickupMeshComponent->SetStaticMesh(TempItemDefinition->WorldMesh.Get());
			GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Black,TEXT("生成网格体"));
		}else
		{
			UStaticMesh*WorldMesh=TempItemDefinition->WorldMesh.LoadSynchronous();
			PickupMeshComponent->SetStaticMesh(WorldMesh);
			GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Black,TEXT("未生成网格体"));
		}
		
		ApplyPickupState();
	}
		
}


// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	//服务器绑定
	if (HasAuthority())
	{
		//防止多次绑定
		SphereComponent->OnComponentBeginOverlap.RemoveDynamic(this,&APickupBase::OnSphereBeginOverlap);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this,&APickupBase::OnSphereBeginOverlap);
			
		SphereComponent->OnComponentEndOverlap.RemoveDynamic(this, &APickupBase::OnSphereEndOverlap);
		SphereComponent->OnComponentEndOverlap.AddDynamic(this, &APickupBase::OnSphereEndOverlap);
	}
	
	InitializePickup();
}


void APickupBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	Super::EndPlay(EndPlayReason);
	
	for (const TPair<TWeakObjectPtr<AAdventureCharacter>, TObjectPtr<UUI_ShootingBase>>& Pair : PickupWidgetMap)
	{
		if (IsValid(Pair.Value)) Pair.Value->RemoveFromParent();
	}
	PickupWidgetMap.Empty();
}

void APickupBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBoxIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!bPickupAvailable) return;
	
	AAdventureCharacter*Character=Cast<AAdventureCharacter>(OtherActor);
	if (!IsValid(Character)) return;
	
	//添加进来的成员
	if (OverlappingCharacters.Contains(Character)) return;
	OverlappingCharacters.Add(Character);
	
	if (!PickupWidgetClass) return;
	
	const FText ItemName=IsValid(ReferenceItem)?ReferenceItem->ItemText.Name:FText::GetEmpty();
	const FText ItemDesc=IsValid(ReferenceItem)?ReferenceItem->ItemText.Description:FText::GetEmpty();
	Character->ClientShowPickupUI(this,ItemName,ItemDesc);	
}

void APickupBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;
	
	AAdventureCharacter*Character=Cast<AAdventureCharacter>(OtherActor);
	if (!IsValid(Character)) return;
	
	OverlappingCharacters.Remove(Character);
	Character->ClientHidePickupUI(this);
}

void APickupBase::HandlePickUpButtonClicked(AAdventureCharacter* TargetCharacter)
{
	if (!HasAuthority()||!bPickupAvailable) return;
	if (!IsValid(TargetCharacter)||!IsValid(ReferenceItem)) return;
	if (!OverlappingCharacters.Contains(TargetCharacter)) return;
	
	if (TargetCharacter->GiveItem(ReferenceItem))
	{
		bPickupAvailable=false;
		//TWeakObjectPtr<T> 是 UE 里针对 UObject 体系的弱引用指针：它能安全地“观察”一个对象，但不会阻止这个对象被 GC 回收；对象销毁后，IsValid() 会变成 false。
		for (const TWeakObjectPtr<AAdventureCharacter>&W:OverlappingCharacters)
		{
			//W.Get() 从弱引用里取出原始指针 AAdventureCharacter*
			if (AAdventureCharacter*C=W.Get()) C->ClientHidePickupUI(this);
			
		}
		OverlappingCharacters.Empty();
		
			ApplyPickupState();
			ForceNetUpdate();
			
			if (bShouldReSpawn)
			{
				GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APickupBase::RespawnPickup, ReSpawnTime, false);
			}
	}
}

void APickupBase::ShowPickupUIFor(AAdventureCharacter* TargetCharacter, const FText& ItemName, const FText& ItemDesc)
{
	if (!IsValid(TargetCharacter)||!PickupWidgetClass) return;
	
	APlayerController*PC=Cast<APlayerController>(TargetCharacter->GetController());
	if (!IsValid(PC)||!PC->IsLocalController()) return;
	
	TargetCharacter->AddInteractablePickup(this);
	
	PC->SetShowMouseCursor(false);
	PC->SetInputMode(FInputModeGameOnly());
	
	if (PickupWidgetMap.Contains(TargetCharacter)) return;
	
	UUI_ShootingBase*WidgetInstance=CreateWidget<UUI_ShootingBase>(PC,PickupWidgetClass);
	if (!IsValid(WidgetInstance)) return;
	
	WidgetInstance->InitPickupWidget(TargetCharacter,this,ItemName,ItemDesc);
	WidgetInstance->AddToViewport();
	
	
	PickupWidgetMap.Add(TargetCharacter,WidgetInstance);
}

void APickupBase::HidePickupUIFor(AAdventureCharacter* TargetCharacter)
{
	if (!IsValid(TargetCharacter)) return;
	
	UUI_ShootingBase*Widget=nullptr;
	if (TObjectPtr<UUI_ShootingBase>*Found=PickupWidgetMap.Find(TargetCharacter))
	{
		//所以要用 ->Get() 把 TObjectPtr<UUI_ShootingBase> 转成普通指针 UUI_ShootingBase*，然后赋给 Widget。
		Widget=Found->Get();
	}
	PickupWidgetMap.Remove(TargetCharacter);
	TargetCharacter->RemoveInteractablePickup(this);
	
	if (IsValid(Widget)) Widget->RemoveFromParent();
	
	if (APlayerController* PC = Cast<APlayerController>(TargetCharacter->GetController()))
	{
		if (PC->IsLocalController())
		{
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
}


//当你在 UE 编辑器的“细节（Details）”面板中修改 PickupItemID 这个属性时，自动更新该 Actor 的网格体（Mesh）和碰撞体大小。
void APickupBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName ChangedPropertyName= PropertyChangedEvent.Property?PropertyChangedEvent.Property->GetFName():NAME_None;

	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(APickupBase, PickupItemID) ||
		ChangedPropertyName == GET_MEMBER_NAME_CHECKED(APickupBase, PickupDataTable))
	{
		if (PickupItemID.IsNone())
		{
			return;
		}

		UDataTable*Table=PickupDataTable.LoadSynchronous();
		if (!IsValid(Table))
		{
			UE_LOG(LogTemp, Warning, TEXT("PickupDataTable is not assigned or could not be loaded for %s."),
				*GetName());
			return;
		}

		const FItemData*ItemDataRow=Table->FindRow<FItemData>(PickupItemID,PickupItemID.ToString());
		if (!ItemDataRow || !IsValid(ItemDataRow->ItemBase.Get()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Could not find a valid item row for '%s' in '%s'."),
				*PickupItemID.ToString(), *Table->GetName());
			return;
		}

		UItemDefinition*TempItemDefinition=ItemDataRow->ItemBase.Get();
		UStaticMesh*WorldMesh=TempItemDefinition->WorldMesh.LoadSynchronous();

		if (IsValid(PickupMeshComponent))
		{
			PickupMeshComponent->SetStaticMesh(WorldMesh);
		}

		if (IsValid(SphereComponent))
		{
			SphereComponent->SetSphereRadius(32.f);
		}
	}
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupBase::OnRep_PickupAvailable()
{
	ApplyPickupState();
}

void APickupBase::ApplyPickupState()
{
	if (!IsValid(PickupMeshComponent)||!IsValid(SphereComponent)) return;
	
	//根据 bPickupAvailable 的值控制 PickupMeshComponent 的显示/隐藏
	//Propagate to Children — 是否传播到子组件
	PickupMeshComponent->SetVisibility(bPickupAvailable,true);
	SphereComponent->SetVisibility(bPickupAvailable,true);
	SphereComponent->SetCollisionEnabled(bPickupAvailable?ECollisionEnabled::QueryOnly:ECollisionEnabled::NoCollision);
}

void APickupBase::RespawnPickup()
{
	if (!HasAuthority()) return;
	
	bPickupAvailable=true;
	//服务器本身不会触发自己的 OnRep_bPickupAvailable()。不调用 ApplyPickupState()，监听服务器上的拾取物可能仍然不可见或没有碰撞。
	ApplyPickupState();
	//强制引擎在下一帧立即同步该 Actor 的所有复制属性，跳过正常的网络更新间隔等待。
	ForceNetUpdate();
}

void APickupBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APickupBase,bPickupAvailable);
}



