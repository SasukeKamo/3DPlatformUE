// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "EAbilityType.h"
#include "UE_CDPCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AUE_CDPCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

public:
	AUE_CDPCharacter();
	
	UFUNCTION(BlueprintCallable)
	void GrantAbility(EAbilityType Ability);
	
	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	bool bHasDoubleJump = false;
	
	bool bCanDoubleJump = true;

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	bool bHasSprint = false;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	float SprintSpeed = 900.0f;

	UFUNCTION(BlueprintCallable)
	bool CanClimbLedge(FVector& OutClimbLocation);
	
	UFUNCTION(BlueprintCallable)
	void StartClimbing();

	UFUNCTION()
	void FinishClimbing();
	
	UPROPERTY(BlueprintReadOnly, Category="Climbing")
	bool bIsClimbing = false;

	UPROPERTY(EditAnywhere, Category = "Climbing")
	float MaxHorizontalGrabDistance = 40.f;
	
	UPROPERTY(EditAnywhere, Category = "Climbing")
	float MaxClimbHeight = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climbing")
	float ClimbForwardOffset = 30.f;

	UPROPERTY(EditAnywhere, Category = "Climbing")
	float ClimbStartHeightOffset = 30.f;

	UPROPERTY(EditAnywhere, Category="Climbing")
	float ClimbHeightOffset = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	UAnimMontage* ClimbMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	float ClimbLerpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	bool bIsLerpingToLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	FVector ClimbTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	UCurveFloat* ClimbPositionCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	float ClimbLerpDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	float ClimbLerpElapsedTime = 0.0f;

	FTimerHandle ClimbFinishHandle;

	bool bWantsToClimb = false;
	FVector PendingClimbLocation;
	
protected:

	virtual void Tick(float DeltaSeconds) override;
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;

	void StartSprinting();
	void StopSprinting();
	

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

