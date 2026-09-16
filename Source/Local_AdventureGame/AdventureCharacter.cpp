// Fill out your copyright notice in the Description page of Project Settings.


#include "AdventureCharacter.h"

// Sets default values
AAdventureCharacter::AAdventureCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it
    	PrimaryActorTick.bCanEverTick = true;
     
    	// Create a first person camera component
    	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    	check(FirstPersonCameraComponent != nullptr);
     
    	// Create a first person mesh component for the owning player
    	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
    	check(FirstPersonMeshComponent != nullptr);
     
    	// Attach the first person mesh to the skeletal mesh
    	FirstPersonMeshComponent->SetupAttachment(GetMesh());
     
      // The first-person mesh is included in First Person rendering (use FirstPersonFieldofView and FirstPersonScale on this mesh) 
    	FirstPersonMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
     
    	// Only the owning player sees the first-person mesh
    	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
     
    	// The owning player doesn't see the regular (third-person) body mesh, but it casts a shadow
    	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
     
    	// Set the first person mesh to not collide with other objects
    	FirstPersonMeshComponent->SetCollisionProfileName(FName("NoCollision"));
     
    	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, FName("head"));
     
    	// Position the camera slightly above the eyes and rotate it to behind the player's head
    	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FirstPersonCameraOffset, FRotator(0.0f, 90.0f, -90.0f));
    	FirstPersonCameraComponent->bUsePawnControlRotation = true;
     
      // Enable first-person rendering on the camera and set default FOV and scale values
    	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
    	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
    	FirstPersonCameraComponent->FirstPersonFieldOfView = FirstPersonFieldOfView;
    	FirstPersonCameraComponent->FirstPersonScale = FirstPersonViewScale;
	
	    InventoryComponent=CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	    check(InventoryComponent!=nullptr);
}

// Called when the game starts or when spawned
void AAdventureCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	check(GEngine!=nullptr);
	
	if (APlayerController*PlayerController=Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem*Subsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem ->AddMappingContext(FirstPersonContext,0);
		}
	}
	
	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Red,TEXT("We are using AdventureCharacter."));

	// Only the owning player sees the first person mesh.
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	GetMesh()->SetOwnerNoSee(true); 

	// Set the animations on the first person mesh.
	FirstPersonMeshComponent->SetAnimInstanceClass(FirstPersonDefaultAnim->GeneratedClass);
	
}

// Called every frame
void AAdventureCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAdventureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this,&AAdventureCharacter::Look);
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AAdventureCharacter::Move);
		
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this,&ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Completed,this,&ACharacter::StopJumping);
	}
}

void AAdventureCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementValue=Value.Get<FVector2D>();
	
	if (Controller)
	{  
		const FVector Right=GetActorRightVector();
		AddMovementInput(Right,MovementValue.X);
		
		const FVector Forward=GetActorForwardVector();
		AddMovementInput(Forward,MovementValue.Y);
	}
}

void AAdventureCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue=Value.Get<FVector2D>();
	
	if (Controller)
	{
		AddControllerPitchInput(LookAxisValue.Y);
		AddControllerYawInput(LookAxisValue.X);
	}
}

bool AAdventureCharacter::IsToolAlreadyOwned(UEquippableToolDefinition* ToolDefinition)
{
	for (UEquippableToolDefinition*InvenytoryItem : InventoryComponent->ToolInventory)
	{
		if (ToolDefinition->ID==ToolDefinition->ID)
		{
			return true;
		}
	}
	return false;
}

void AAdventureCharacter::AttachTool(UEquippableToolDefinition* ToolDefinition)
{
	if (not IsToolAlreadyOwned(ToolDefinition))
	{
		//生成Actor
		AEquippableToolBase*ToolToEquip=GetWorld()->SpawnActor<AEquippableToolBase>(ToolDefinition->ToolAsset,this->GetActorTransform());
		
		//虚幻引擎会将两个物体接合在一起，以便它们在移动时作为一个整体交互。
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		//AttachToActor用于将一个Actor附加到目标父级Actor
		ToolToEquip->AttachToActor(this,AttachmentRules);
		//AttachToComponent用于将Actor的根骨骼组件附加到目标组件
		//MyActor->AttachToActor(ParentActor, AttachmentRules, OptionalSocketName)
		ToolToEquip->AttachToComponent(FirstPersonMeshComponent,AttachmentRules,FName(TEXT("HandGrip_R")));
	
		FirstPersonMeshComponent->SetAnimInstanceClass(ToolToEquip->FirstPersonToolAnim->GeneratedClass);
		GetMesh()->SetAnimInstanceClass(ToolToEquip->ThirdPersonToolAnim->GeneratedClass);
	
		InventoryComponent->ToolInventory.Add(ToolDefinition);
		ToolToEquip->OwningCharacter=this;
		
		EquippedTool=ToolToEquip;
		
		if (APlayerController*PlayerController=Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem*Subsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				//优先级数值越大，优先级越高。
				Subsystem ->AddMappingContext(ToolToEquip->ToolMappingContext,1);
			}
			ToolToEquip->BindInputAction(UseAction);
		}
	}
}

void AAdventureCharacter::GiveItem(UItemDefinition* ItemDefinition)
{
	switch (ItemDefinition->ItemType)
	{
	case EItemType::Tool:
		{
			UEquippableToolDefinition*ToolDefinition=Cast<UEquippableToolDefinition>(ItemDefinition);
			if (ToolDefinition!=nullptr)
			{
				AttachTool(ToolDefinition);
			}
			break;
		}
	case EItemType::Consumable:
		{
			break;
		}
	default:
		break;
	}
}

FVector AAdventureCharacter::GetCameraTargetLocation()
{
	FVector TargetPosition;
	
	UWorld* const World = GetWorld();
	if (World!=nullptr)
	{
		FHitResult Hit;
		
		const FVector TraceStart=FirstPersonCameraComponent->GetComponentLocation();
		const FVector TraceEnd=TraceStart+FirstPersonCameraComponent->GetForwardVector()*10000.0;
	
	    World->LineTraceSingleByChannel(Hit,TraceStart,TraceEnd,ECC_Visibility);//只检测对Visibility通道设为Block的物体
		//现在，Hit 值包含关于命中结果的信息，例如撞击的位置和法线。
		
		
		//ImpactPoint:表示射线（或扫描形状）与物体表面的实际接触点的世界坐标。
		TargetPosition=Hit.bBlockingHit?Hit.ImpactPoint:Hit.TraceEnd;
	}
	
	return TargetPosition;
}

