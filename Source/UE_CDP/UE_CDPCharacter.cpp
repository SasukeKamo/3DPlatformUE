#include "UE_CDPCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AUE_CDPCharacter

AUE_CDPCharacter::AUE_CDPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	PrimaryActorTick.bCanEverTick = true;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUE_CDPCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AUE_CDPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AUE_CDPCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUE_CDPCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUE_CDPCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AUE_CDPCharacter::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AUE_CDPCharacter::StopSprinting);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void AUE_CDPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AUE_CDPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUE_CDPCharacter::Jump()
{
	UE_LOG(LogTemp, Warning, TEXT("Jump() → isFalling = %s | hasDoubleJump = %s | canDoubleJump = %s"),
		GetCharacterMovement()->IsFalling() ? TEXT("true") : TEXT("false"),
		bHasDoubleJump ? TEXT("true") : TEXT("false"),
		bCanDoubleJump ? TEXT("true") : TEXT("false"));

	if (GetCharacterMovement()->IsFalling())
	{
		if (bHasDoubleJump && bCanDoubleJump)
		{
			GetCharacterMovement()->Velocity.Z = 0;
			LaunchCharacter(FVector(0, 0, GetCharacterMovement()->JumpZVelocity), false, true);
			bCanDoubleJump = false;
			UE_LOG(LogTemplateCharacter, Warning, TEXT("Double jump!"));
		}
	}
	else
	{
		FVector ClimbLocation;
		if (CanClimbLedge(ClimbLocation))
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("CanClimbLedge returned true"));
			StartClimbing();
			return;
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("CanClimbLedge returned false"));
		}
		Super::Jump();
	}
}


void AUE_CDPCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bCanDoubleJump = true;
	UE_LOG(LogTemp, Warning, TEXT("Landed called!"));
}


void AUE_CDPCharacter::GrantAbility(EAbilityType Ability)
{
	switch (Ability)
	{
	case EAbilityType::DoubleJump:
		bHasDoubleJump = true;
		UE_LOG(LogTemplateCharacter, Warning, TEXT("DoubleJump granted"));
		break;
	case EAbilityType::Sprint:
		bHasSprint = true;
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Sprint granted"));
		break;
	default:
		break;
	}
}

void AUE_CDPCharacter::StartSprinting()
{
	if (bHasSprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Sprint ON"));
	}
}

void AUE_CDPCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	UE_LOG(LogTemplateCharacter, Warning, TEXT("Sprint OFF"));
}

bool AUE_CDPCharacter::CanClimbLedge(FVector& OutClimbLocation)
{
	FVector Start = GetActorLocation() + FVector(0, 0, 90);
	FVector Forward = GetActorForwardVector();
	FVector WallEnd = Start + Forward * 100.0f;

	FHitResult WallHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(WallHit, Start, WallEnd, ECC_Visibility, Params))
	{
		FVector LedgeStart = WallHit.ImpactPoint + FVector(0, 0, 110);
		FVector LedgeEnd = LedgeStart - FVector(0, 0, 150);

		FHitResult LedgeHit;
		if (GetWorld()->LineTraceSingleByChannel(LedgeHit, LedgeStart, LedgeEnd, ECC_Visibility, Params))
		{
			float LedgeHeight = (LedgeHit.ImpactPoint.Z + ClimbHeightOffset) - GetActorLocation().Z;
			FVector LedgeLoc = LedgeHit.ImpactPoint + FVector(0, 0, 55);
			float HorizontalDist = FVector::Dist2D(LedgeLoc, GetActorLocation());

			UE_LOG(LogTemplateCharacter, Warning, TEXT("CanClimbLedge → height: %.2f | horizontal: %.2f"), LedgeHeight, HorizontalDist);

			if (LedgeHeight <= MaxClimbHeight && HorizontalDist <= MaxHorizontalGrabDistance)
			{
				OutClimbLocation = LedgeLoc;
				return true;
			}
		}
	}

	return false;
}


void AUE_CDPCharacter::StartClimbing()
{
	if (bIsClimbing || bWantsToClimb) return;

	FVector ClimbLocation;
	if (!CanClimbLedge(ClimbLocation)) return;

	PendingClimbLocation = ClimbLocation;
	bIsClimbing = true;
	bWantsToClimb = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	if (ClimbMontage)
	{
		FVector StartOffset = FVector(0, -30.f, ClimbStartHeightOffset);
		SetActorLocation(GetActorLocation() + StartOffset);
		float Duration = PlayAnimMontage(ClimbMontage);
		if (Duration > 0.f)
		{
			GetWorldTimerManager().SetTimer(ClimbFinishHandle, this, &AUE_CDPCharacter::FinishClimbing, Duration, false);
			UE_LOG(LogTemplateCharacter, Warning, TEXT("Playing Climb Montage for %.2f seconds"), Duration);
		}
		else
		{
			bIsClimbing = false;
			bWantsToClimb = false;
		}
	}
	else
	{
		bIsClimbing = false;
		bWantsToClimb = false;
	}
}

void AUE_CDPCharacter::FinishClimbing()
{
	FVector ForwardOffset = GetActorForwardVector() * ClimbForwardOffset;
	
	ClimbTargetLocation = PendingClimbLocation + ForwardOffset;
	bIsLerpingToLedge = true;
	ClimbLerpElapsedTime = 0.0f;
	bCanDoubleJump = true;
	
	GetWorldTimerManager().ClearTimer(ClimbFinishHandle);
	UE_LOG(LogTemplateCharacter, Warning, TEXT("Starting final climb position interpolation"));
}

void AUE_CDPCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bIsClimbing && !bIsLerpingToLedge && GetCharacterMovement()->IsFalling())
	{
		FVector ClimbLocation;
		if (CanClimbLedge(ClimbLocation))
		{
			StartClimbing();
		}
	}
	
	if (bIsLerpingToLedge)
	{
		ClimbLerpElapsedTime += DeltaSeconds;
		float Alpha = FMath::Clamp(ClimbLerpElapsedTime / ClimbLerpDuration, 0.0f, 1.0f);
		
		if (Alpha >= 1.0f)
		{
			bIsLerpingToLedge = false;
			bIsClimbing = false;
			bWantsToClimb = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			UE_LOG(LogTemplateCharacter, Warning, TEXT("Finished smooth climb transition"));
		}
	}
}