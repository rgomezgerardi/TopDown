// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StrategyPlayerController.generated.h"

class AStrategyPawn;
class UInputMappingContext;
class UNiagaraSystem;
struct FInputActionValue;
class AStrategyHUD;
class AStrategyNPC;
class UInputAction;
class AStrategyUnit;
class UCameraControlComponent;
class USelectionComponent;
class UUnitCommandComponent;
class AGridManager;

/** Enum to determine the last used input type */
UENUM(BlueprintType)
enum EStrategyInputMode : uint8
{
	SIM_Mouse	UMETA(DisplayName = "Mouse"),
	SIM_Touch	UMETA(DisplayName = "Touch")
};

/**
 *  Player Controller for a top-down strategy game.
 *  Handles unit selection and commands.
 *  Implements both mouse and touch controls.
 */
UCLASS(abstract)
class AStrategyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCameraControlComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USelectionComponent> SelectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UUnitCommandComponent> UnitCommandComponent;

	/** Reference to the level's GridManager, cached from GameMode on possess */
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;

	/** Strategy Pawn associated with this controller */
	TObjectPtr<AStrategyPawn> ControlledPawn;

	/** Strategy HUD associated with this controller */
	TObjectPtr<AStrategyHUD> StrategyHUD;

	/** Determines the chosen input type */
	UPROPERTY(EditAnywhere, Category="Input")
	TEnumAsByte<EStrategyInputMode> InputMode = SIM_Mouse;

	/** Input mapping context to use with mouse input */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* MouseMappingContext;

	/** Input mapping context to use with touch input */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* TouchMappingContext;

	/** Input Action for select and click */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SelectClickAction;

	/** Input Action for deselecting all units */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DeselectClickAction;

	/** Input Action for click interaction */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractClickAction;

	/** Input Action for interaction press and hold */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractHoldAction;

	/** Input Action for modifying selection mode */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SelectionModifierAction;

	/** Max distance to look for nearby units when doing a click or touch interaction */
	UPROPERTY(EditAnywhere, Category="Input", meta = (ClampMin = 0, ClampMax = 10000, Units = "cm"))
	float InteractionRadius = 250.0f;

	/** Max distance between the starting and current position of the second touch finger to be considered a box selection */
	UPROPERTY(EditAnywhere, Category="Input", meta = (ClampMin = 0, ClampMax = 10000))
	float MinSecondFingerDistanceForBoxSelect = 10.0f;

	/** Saves the world location of the last initiated interaction */
	FVector CachedInteraction;

	/** Saves the world location of the last initiated unit selection */
	FVector CachedSelection;

	/** Saves the world location where the player started a press and hold interaction */
	FVector2D StartingInteractionPosition;

	/** Saves the current world location of the player's cursor in press and hold interaction */
	FVector2D CurrentInteractionPosition;

	/** Trace channel to use for selection trace checks */
	UPROPERTY(EditAnywhere, Category = "Selection")
	TEnumAsByte<ETraceTypeQuery> SelectionTraceChannel;

	/** Currently targeted unit (last unit under cursor during a selection sweep) */
	AStrategyUnit* TargetUnit = nullptr;

public:

	/** Constructor */
	AStrategyPlayerController();

	/** Cache systems once BeginPlay guarantees GameMode is fully initialized */
	virtual void BeginPlay() override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Returns the selection component */
	USelectionComponent* GetSelectionComponent() const { return SelectionComponent; }

	/** Returns the list of currently selected units */
	const TArray<AStrategyUnit*>& GetSelectedUnits() const;

protected:

	/** Select click action */
	void SelectClick(const FInputActionValue& Value);

	/** Deselect all units */
	void DeselectClick(const FInputActionValue& Value);

	/** Presses or releases the selection modifier key */
	void SelectionModifier(const FInputActionValue& Value);

	/** Starts an interaction hold input */
	void InteractHoldStarted(const FInputActionValue& Value);

	/** Interaction hold input triggered */
	void InteractHoldTriggered(const FInputActionValue& Value);

	/** Interaction click input started */
	void InteractClickStarted(const FInputActionValue& Value);

	/** Interaction click input completed */
	void InteractClickCompleted(const FInputActionValue& Value);

	/** Attempt to select or deselect the unit under the cursor */
	void DoSelectionCommand();

	/** Move all selected units to the cached interaction location */
	void DoMoveUnitsCommand();

	/** Returns the current mouse position in screen space */
	FVector2D GetMouseLocation();

	/** Attempts to get the world location under the cursor, returns true if successful */
	bool GetLocationUnderCursor(FVector& Location);

	/** Spawns cursor feedback effect at the given location */
	UFUNCTION(BlueprintImplementableEvent, Category="Cursor", meta = (DisplayName="Cursor Feedback"))
	void BP_CursorFeedback(FVector Location, bool bPositive);
};