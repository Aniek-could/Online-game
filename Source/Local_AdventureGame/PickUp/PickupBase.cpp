// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickupBase.h"
#include "Data/ItemDefinition.h"

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PickupMeshComponent=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	check(PickupMeshComponent!=nullptr);

	SphereComponent=CreateDefaultSubobject<USphereComponent>(FName("SphereComponent"));
	check(SphereComponent!=nullptr);
	SphereComponent->SetupAttachment(PickupMeshComponent);
	SphereComponent->SetSphereRadius(32.f);
}

void APickupBase::InitializePickup()
{
	if (PickupDataTable&& !PickupItemID.IsNone())
	{
		const FItemData*ItemDataRow=PickupDataTable->FindRow<FItemData>(PickupItemID,PickupItemID.ToString());
		
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
		
		PickupMeshComponent->SetVisibility(true);
		SphereComponent->SetVisibility(true);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		//绑定
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this,&APickupBase::OnSphereBeginOverlap);
	}
		
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializePickup();
}

void APickupBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBoxIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Black,TEXT("Attempting a pickup collision"));
    
	AAdventureCharacter*Character=Cast<AAdventureCharacter>(OtherActor);
	if (Character!=nullptr)
	{
		Character->GiveItem(ReferenceItem);
		
		SphereComponent->OnComponentBeginOverlap.RemoveAll(this);
		
		PickupMeshComponent->SetVisibility(false);
		PickupMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (bShouldReSpawn)
		{
			GetWorldTimerManager().SetTimer(RespawnTimerHandle,this,&APickupBase::InitializePickup,ReSpawnTime,false,ReSpawnTime);
		}
	}
}


//当你在 UE 编辑器的“细节（Details）”面板中修改 PickupItemID 这个属性时，自动更新该 Actor 的网格体（Mesh）和碰撞体大小。
void APickupBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName ChangedPropertyName= PropertyChangedEvent.Property?PropertyChangedEvent.Property->GetFName():NAME_None;

	if (ChangedPropertyName==GET_ENUMERATOR_NAME_CHECKED(APickupBase,PickupItemID))
	{
		if (const FItemData*ItemDataRow=PickupDataTable->FindRow<FItemData>(PickupItemID,PickupItemID.ToString()))
		{
			UItemDefinition*TempItemDefinition=ItemDataRow->ItemBase;
			PickupMeshComponent->SetStaticMesh(TempItemDefinition->WorldMesh.Get());
			SphereComponent->SetSphereRadius(32.f);
		}
	}
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

