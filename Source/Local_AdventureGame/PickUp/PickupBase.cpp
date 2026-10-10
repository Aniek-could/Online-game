// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickupBase.h"
#include "Data/ItemDefinition.h"

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
		
		//服务器绑定
		if (HasAuthority())
		{
			//防止多次绑定
			SphereComponent->OnComponentBeginOverlap.RemoveDynamic(this,&APickupBase::OnSphereBeginOverlap);
			SphereComponent->OnComponentBeginOverlap.AddDynamic(this,&APickupBase::OnSphereBeginOverlap);
		}
		
		ApplyPickupState();
	}
		
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializePickup();
}

void APickupBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APickupBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBoxIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!bPickupAvailable) return;
    
	AAdventureCharacter*Character=Cast<AAdventureCharacter>(OtherActor);
	if (!IsValid(Character)||!IsValid(ReferenceItem)) return;
		
	if (Character->GiveItem(ReferenceItem))
	{
		bPickupAvailable=false;
		ApplyPickupState();
		ForceNetUpdate();
		
		if (bShouldReSpawn)
		{
			GetWorldTimerManager().SetTimer(RespawnTimerHandle,this,&APickupBase::RespawnPickup,ReSpawnTime,false);
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



