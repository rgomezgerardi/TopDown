// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitCommandComponent.generated.h"

class AStrategyUnit;

/**
 * Handles unit movement commands along a sequence of world positions.
 * Receives a pre-calculated path from the Controller and moves a unit along it in one smooth movement.
 * Lives on AStrategyPlayerController.
 * Has no knowledge of the grid system.
 */
UCLASS(ClassGroup="Strategy", meta=(BlueprintSpawnableComponent))
class UUnitCommandComponent : public UActorComponent
{
	GENERATED_BODY()

	/** The unit currently being moved */
	AStrategyUnit* ActiveUnit = nullptr;

	/** Final destination of the current move command */
	FVector FinalDestination = FVector::ZeroVector;

public:

	UUnitCommandComponent();

	/** Moves a single unit along the given world-space path in one smooth movement.
	 *  Returns false if the path is empty or the unit is invalid. */
	bool MoveUnit(AStrategyUnit* Unit, const TArray<FVector>& Path);

	/** Returns the final destination of the last move command */
	FVector GetDestination() const;

private:

	/** Called when the active unit finishes moving */
	UFUNCTION()
	void OnMoveCompleted(AStrategyUnit* MovedUnit);
};
