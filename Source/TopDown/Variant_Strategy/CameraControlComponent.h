// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraControlComponent.generated.h"

class AStrategyPawn;
class UInputAction;
struct FInputActionValue;
class UEnhancedInputComponent;

/**
 * Handles all camera movement, zoom, and drag-scroll logic.
 * Lives on AStrategyPlayerController.
 */
UCLASS(ClassGroup="Strategy", meta=(BlueprintSpawnableComponent))
class UCameraControlComponent : public UActorComponent
{
	GENERATED_BODY()

	/** Input Action for moving the camera */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveCameraAction;

	/** Input Action for zooming the camera */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ZoomCameraAction;

	/** Input Action for resetting the camera to its default position */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ResetCameraAction;

	/** Minimum allowed camera zoom level */
	UPROPERTY(EditAnywhere, Category="Camera", meta=(ClampMin=0, ClampMax=10000))
	float MinZoomLevel = 1000.0f;

	/** Maximum allowed camera zoom level */
	UPROPERTY(EditAnywhere, Category="Camera", meta=(ClampMin=0, ClampMax=10000))
	float MaxZoomLevel = 2500.0f;

	/** Scales zoom inputs by this value */
	UPROPERTY(EditAnywhere, Category="Camera", meta=(ClampMin=0, ClampMax=1000))
	float ZoomScaling = 100.0f;

	/** Affects how fast the camera moves while dragging with the mouse */
	UPROPERTY(EditAnywhere, Category="Camera", meta=(ClampMin=0, ClampMax=10000))
	float DragMultiplier = 0.1f;

	/** Current camera zoom level */
	float CameraZoom = 0.0f;
	/** Default camera zoom level */
	float DefaultZoom = 0.0f;
	

	UPROPERTY()
	TObjectPtr<AStrategyPawn> ControlledPawn;

public:

	UCameraControlComponent();

	void Initialize(AStrategyPawn* InPawn);
	void BindInputs(UEnhancedInputComponent* EnhancedInputComponent);
	
	/** Drag scroll the camera */
	void DragScroll(FVector2D StartPosition, FVector2D CurrentPosition);

protected:

	/** Moves the camera by the given input */
	void MoveCamera(const FInputActionValue& Value);
	
	/** Changes the camera zoom level by the given input */
	void ZoomCamera(const FInputActionValue& Value);
	
	/** Resets the camera to its initial value */
	void ResetCamera(const FInputActionValue& Value);
};