// Fill out your copyright notice in the Description page of Project Settings.


#include "AdventureCharacter.h"

void AAdventureCharacter::OnRep_CurrentTool()
{
	ApplyCurrentTool();
}

void AAdventureCharacter::ApplyCurrentTool()
{
	if (!IsValid(CurrentTool))
	{
		EquippedTool = nullptr;
		return;
	}
	
	CurrentTool->OwningCharacter = this;
	EquippedTool = CurrentTool;

	//虚幻引擎会将两个物体接合在一起，以便它们在移动时作为一个整体交互。
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	
	//是否是本地玩家控制的Pawn
	USkeletalMeshComponent*AttachMesh=IsLocallyControlled()?FirstPersonMeshComponent:GetMesh();
	
	if (AttachMesh)
	{
		//AttachToComponent用于将Actor的根骨骼组件附加到目标组件
		CurrentTool->AttachToComponent(AttachMesh,AttachmentRules,FName(TEXT("HandGrip_R")));
	}
	
	if (IsLocallyControlled() &&
	  CurrentTool->FirstPersonToolAnim &&
	  CurrentTool->FirstPersonToolAnim->GeneratedClass)
	{
		FirstPersonMeshComponent->SetAnimInstanceClass(
			CurrentTool->FirstPersonToolAnim->GeneratedClass
		);
	}

	if (CurrentTool->ThirdPersonToolAnim &&
	  CurrentTool->ThirdPersonToolAnim->GeneratedClass)
	{
		GetMesh()->SetAnimInstanceClass(
			CurrentTool->ThirdPersonToolAnim->GeneratedClass
		);
	}
	
	//本地输入的玩家
	if (IsLocallyControlled())
	{
		if (APlayerController*PlayerController=Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem*Subsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				//优先级数值越大，优先级越高。
				Subsystem ->AddMappingContext(CurrentTool->ToolMappingContext,1);
			}
			if (UseAction)
			{
				CurrentTool->BindInputAction(UseAction);
			}
		}
	}
}

// Sets default values
AAdventureCharacter::AAdventureCharacter()
{
	
	 bReplicates = true;
	 SetReplicateMovement(true);
	
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
	
	MaxHealth=100.f;
	CurrentHealth=MaxHealth;
	bIsDead=false;
}

// Called when the game starts or when spawned
void AAdventureCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	check(GEngine!=nullptr);
	if (IsLocallyControlled())
	{
		if (APlayerController*PlayerController=Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem*Subsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem ->AddMappingContext(FirstPersonContext,0);
			}
		}
	}

	// Only the owning player sees the first person mesh.
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	GetMesh()->SetOwnerNoSee(true); 
	
	if (IsLocallyControlled() &&
	  FirstPersonDefaultAnim &&
	  FirstPersonDefaultAnim->GeneratedClass)
	{
		FirstPersonMeshComponent->SetAnimInstanceClass(
			FirstPersonDefaultAnim->GeneratedClass
		);
	}
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
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MovementValue.Y);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MovementValue.X);
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
		if (ToolDefinition->ID==InvenytoryItem->ID)
		{
			return true;
		}
	}
	return false;
}

bool AAdventureCharacter::AttachTool(UEquippableToolDefinition* ToolDefinition)
{
	if (!HasAuthority()) return false;
	if (!IsValid(ToolDefinition)||!ToolDefinition->ToolAsset) return false;
	
	if (IsToolAlreadyOwned(ToolDefinition)) return false;
	
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		//生成Actor
		AEquippableToolBase*ToolToEquip=GetWorld()->SpawnActor<AEquippableToolBase>(ToolDefinition->ToolAsset,this->GetActorTransform(),SpawnParams);
		
		if (!ToolToEquip) return false;
		
		InventoryComponent->ToolInventory.Add(ToolDefinition);
		
		CurrentTool=ToolToEquip;
		ApplyCurrentTool();
		
		ForceNetUpdate();
	return true;
}

bool AAdventureCharacter::GiveItem(UItemDefinition* ItemDefinition)
{
	if (!HasAuthority()||!IsValid(ItemDefinition)) return false;
	
	switch (ItemDefinition->ItemType)
	{
	case EItemType::Tool:
		{
			UEquippableToolDefinition*ToolDefinition=Cast<UEquippableToolDefinition>(ItemDefinition);
			if (ToolDefinition!=nullptr)
			{
				return AttachTool(ToolDefinition);
			}
			break;
		}
	case EItemType::Consumable:
		{
			return false;
			break;
		}
	default:
		break;
	}
	return true;
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

void AAdventureCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AAdventureCharacter,CurrentTool);
	DOREPLIFETIME(AAdventureCharacter,InventoryComponent);
	DOREPLIFETIME(AAdventureCharacter,CurrentHealth);
	DOREPLIFETIME(AAdventureCharacter,bIsDead);
}

void AAdventureCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AAdventureCharacter::OnHealthUpdate()
{
	if (IsLocallyControlled())
	{
		FString healthText = FString::Printf(TEXT("Health: %f"), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, healthText);
	}
}

float AAdventureCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                      class AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	if (bIsDead) return 0.f;
	
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	float DamageApplied=CurrentHealth-ActualDamage;
	SetCurrentHealth(DamageApplied);
	
	return ActualDamage;
}

void AAdventureCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth=FMath::Clamp(healthValue,0.0f,MaxHealth);//调用复制函数
		OnHealthUpdate();
		
		
		if (CurrentHealth<=0.f&&!bIsDead)
		{
			Die();
		}
	}
}

void AAdventureCharacter::OnRep_IsDeadOrRevive()
{
	DecideWhetherToReviveOrDie();
}

void AAdventureCharacter::DecideWhetherToReviveOrDie()
{
	if (bIsDead)
	{
		MulticastHandleDeath();
		if (IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("您已死亡"));
		}
	}
	else
	{
		MulticastHandleRespawn();
		if (IsLocallyControlled())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("您已复活"));
		}
	}
}

void AAdventureCharacter::Die()
{
	if (!HasAuthority()||bIsDead) return;
	
	bIsDead=true;//同步整个客户端
	
	MulticastHandleDeath();
	
	if (IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("您已死亡"));
	}
	
	ForceNetUpdate();
	
}

void AAdventureCharacter::MulticastHandleDeath_Implementation()
{
	UAnimInstance*AnimInst=GetMesh()->GetAnimInstance();
	UAnimInstance*FirstPersonInst=FirstPersonMeshComponent->GetAnimInstance();
	
	if (AnimInst)
	{
		if (!AnimInst->Montage_IsPlaying(DeathMontage))
		{
			AnimInst->Montage_Play(DeathMontage);
		}
	}
	
	if (IsLocallyControlled()&&FirstPersonInst)
	{
		if (!FirstPersonInst->Montage_IsPlaying(DeathMontage))
		{
			FirstPersonInst->Montage_Play(DeathMontage);
		}
	}
}


void AAdventureCharacter::Revive()
{
	if (!HasAuthority()||!bIsDead) return;
	
	bIsDead=false;
	
	if (IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, TEXT("您已复活"));
	}
	
	MulticastHandleRespawn();
	ForceNetUpdate();
}

void AAdventureCharacter::MulticastHandleRespawn_Implementation()
{
}




