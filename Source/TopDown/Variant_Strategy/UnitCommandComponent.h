// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitCommandComponent.generated.h"

class AStrategyUnit;

/**
 * Handles unit movement commands along a sequence of world positions.
 * Receives a pre-calculated path from the Controller and moves a single unit step by step.
 * Lives on AStrategyPlayerController.
 * Has no knowledge of the grid system.
 */
UCLASS(ClassGroup="Strategy", meta=(BlueprintSpawnableComponent))
class UUnitCommandComponent : public UActorComponent
{
	GENERATED_BODY()

	/** The unit currently being moved */
	AStrategyUnit* ActiveUnit = nullptr;

	/** The full path of world positions to follow */
	TArray<FVector> CachedPath;

	/** Index of the next position to move toward in CachedPath */
	int32 CurrentPathIndex = 0;

public:

	UUnitCommandComponent();

	/** Moves a single unit along the given world-space path.
	 *  Returns false if the path is empty or the unit is invalid. */
	bool MoveUnit(AStrategyUnit* Unit, const TArray<FVector>& Path);

	/** Returns the final destination of the last move command */
	FVector GetDestination() const;

private:

	/** Moves the active unit to the next position in CachedPath */
	void MoveToNextPosition();

	/** Called when the active unit finishes moving to a position */
	UFUNCTION()
	void OnMoveCompleted(AStrategyUnit* MovedUnit);
};