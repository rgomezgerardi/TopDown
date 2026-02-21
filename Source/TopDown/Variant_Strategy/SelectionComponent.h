#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SelectionComponent.generated.h"

class AStrategyUnit;

/**
 * Handles unit selection state and logic.
 * Lives on AStrategyPlayerController.
 * The Controller owns all input bindings and calls this component's methods.
 */
UCLASS(ClassGroup="Strategy", meta=(BlueprintSpawnableComponent))
class USelectionComponent : public UActorComponent
{
	GENERATED_BODY()

	/** Currently selected units */
	TArray<AStrategyUnit*> SelectedUnits;

	/** If true, the player is adding or removing units from the selection (shift-select) */
	bool bSelectionModifier = false;

	/** If true, double-tap touch select-all mode is active */
	bool bDoubleTapActive = false;

public:

	USelectionComponent();

	/** Adds a unit to the selection, or removes it if already selected */
	void SelectUnit(AStrategyUnit* Unit);

	/** Removes a specific unit from the selection */
	void DeselectUnit(AStrategyUnit* Unit);

	/** Deselects all currently selected units */
	void DeselectAll();

	/** Selects all units currently visible on screen */
	void SelectAllOnScreen(UWorld* World);

	/** Replaces the current selection with the units inside the drag-select box.
	 *  Clears selection if the array is empty. */
	void DragSelectUnits(const TArray<AStrategyUnit*>& Units);

	/** Sets the selection modifier flag (e.g. Shift held) */
	void SetSelectionModifier(bool bActive);

	/** Returns the current state of the selection modifier flag */
	bool GetSelectionModifier() const;

	/** Returns the double-tap select-all toggle state */
	bool GetDoubleTapActive() const;

	/** Sets the double-tap select-all toggle state */
	void SetDoubleTapActive(bool bActive);

	/** Returns the list of currently selected units */
	const TArray<AStrategyUnit*>& GetSelectedUnits() const;

	/** Returns the number of currently selected units */
	int32 GetSelectedUnitCount() const;
};