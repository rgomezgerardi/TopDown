// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraControlComponent.h"
#include "StrategyPawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"

UCameraControlComponent::UCameraControlComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCameraControlComponent::Initialize(AStrategyPawn* InPawn)
{
	ControlledPawn = InPawn;

	// Read the camera's current OrthoWidth as our default zoom.
	// This way the Blueprint/CDO value on the pawn drives the starting point — we don't hardcode it here.
	DefaultZoom = CameraZoom = ControlledPawn->GetCamera()->OrthoWidth;
}

void UCameraControlComponent::BindInputs(UEnhancedInputComponent* EIC)
{
	EIC->BindAction(MoveCameraAction, ETriggerEvent::Triggered, this, &UCameraControlComponent::MoveCamera);
	EIC->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this, &UCameraControlComponent::ZoomCamera);
	EIC->BindAction(ResetCameraAction, ETriggerEvent::Triggered, this, &UCameraControlComponent::ResetCamera);
}

void UCameraControlComponent::DragScroll(FVector2D StartPosition, FVector2D CurrentPosition)
{
	const FVector2D Delta = StartPosition - CurrentPosition;
	const FRotator CameraRot(0.0f, -45.0f, 0.0f);
	const FVector ScrollDelta = CameraRot.RotateVector(FVector(Delta.X, Delta.Y, 0.0f)) * DragMultiplier;

	ControlledPawn->AddActorWorldOffset(ScrollDelta);
}

// -------------------------------------------------------
// Private input handlers
// -------------------------------------------------------

void UCameraControlComponent::MoveCamera(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	// get the forward input component vector
	FRotator ForwardRot = Cast<APlayerController>(GetOwner())->GetControlRotation();
	ForwardRot.Pitch = 0.0f;
	ForwardRot.Roll = 0.0f;

	// get the right input component vector  
	FRotator RightRot = Cast<APlayerController>(GetOwner())->GetControlRotation();
	RightRot.Pitch = 0.0f;
	RightRot.Roll = 0.0f;

	// add the forward input
	ControlledPawn->AddMovementInput(ForwardRot.RotateVector(FVector::ForwardVector), InputVector.X + InputVector.Y);

	// add the right input
	ControlledPawn->AddMovementInput(RightRot.RotateVector(FVector::RightVector), InputVector.X - InputVector.Y);
}

void UCameraControlComponent::ZoomCamera(const FInputActionValue& Value)
{
	CameraZoom = FMath::Clamp(CameraZoom - (Value.Get<float>() * ZoomScaling), MinZoomLevel, MaxZoomLevel);
	ControlledPawn->SetZoomModifier(CameraZoom);
}

void UCameraControlComponent::ResetCamera(const FInputActionValue& Value)
{
	CameraZoom = DefaultZoom;
	ControlledPawn->SetZoomModifier(DefaultZoom);
}
