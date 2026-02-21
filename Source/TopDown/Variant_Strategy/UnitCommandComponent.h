// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitCommandComponent.generated.h"

class AStrategyUnit;

/**
 * Handles unit movement commands and interaction resolution.
 * Lives on AStrategyPlayerController.
 * The Controller owns all input bindings and passes selected units and goals as parameters.
 */
UCLASS(ClassGroup="Strategy", meta=(BlueprintSpawnableComponent))
class UUnitCommandComponent : public UActorComponent
{
	GENERATED_BODY()

	/** The world location of the last issued move command.
	 *  Cached here because OnMoveCompleted fires after the cursor has moved. */
	FVector CachedGoal = FVector::ZeroVector;

	/** Cached list of units from the last move command.
	 *  Used in OnMoveCompleted to exclude selected units from the interaction overlap test. */
	TArray<AStrategyUnit*> CachedUnits;

	/** Copy of the interaction radius from the Controller */
	float InteractionRadius = 0.0f;

	/** If true, allow units to trigger interactions on arrival.
	 *  Set to false after the first unit arrives to prevent interaction chains. */
	bool bAllowInteraction = true;

public:

	UUnitCommandComponent();

	/** Stores the interaction radius from the Controller for use in move and overlap calculations */
	void Initialize(float InInteractionRadius);

	/** Orders all units in the list to move toward Goal.
	 *  The closest unit moves to Goal directly; others move to random navigable points around it.
	 *  Returns false if any move request failed. */
	bool MoveUnits(const TArray<AStrategyUnit*>& Units, FVector Goal);

	/** Resets the interaction flag, allowing the next move command to trigger interactions */
	void ResetInteraction();

private:

	/** Called when a unit finishes moving. Checks for nearby interactable objects at CachedGoal. */
	UFUNCTION()
	void OnMoveCompleted(AStrategyUnit* MovedUnit);

	/** Returns the unit in Units closest to Location */
	AStrategyUnit* GetClosestUnitToLocation(const TArray<AStrategyUnit*>& Units, FVector Location);
};
