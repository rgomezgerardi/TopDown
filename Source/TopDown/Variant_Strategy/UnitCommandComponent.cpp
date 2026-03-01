// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnitCommandComponent.h"
#include "StrategyUnit.h"

UUnitCommandComponent::UUnitCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UUnitCommandComponent::MoveUnit(AStrategyUnit* Unit, const TArray<FVector>& Path)
{
	if (!IsValid(Unit) || Path.Num() == 0)
	{
		return false;
	}

	// if already moving, clean up previous movement first
	if (ActiveUnit != nullptr)
	{
		ActiveUnit->OnMoveCompleted.RemoveDynamic(this, &UUnitCommandComponent::OnMoveCompleted);
		ActiveUnit = nullptr;
		CachedPath.Empty();
		CurrentPathIndex = 0;
	}

	// stop any current movement
	Unit->StopMoving();

	// cache state for this move command
	ActiveUnit = Unit;
	CachedPath = Path;
	CurrentPathIndex = 0;

	// subscribe to the unit's move completed delegate
	ActiveUnit->OnMoveCompleted.AddDynamic(this, &UUnitCommandComponent::OnMoveCompleted);

	// start moving to the first position
	MoveToNextPosition();

	return true;
}

FVector UUnitCommandComponent::GetDestination() const
{
	if (CachedPath.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	return CachedPath.Last();
}

void UUnitCommandComponent::MoveToNextPosition()
{
	if (!IsValid(ActiveUnit))
	{
		// clean up if unit was destroyed mid-movement
		ActiveUnit = nullptr;
		CachedPath.Empty();
		CurrentPathIndex = 0;
		return;
	}

	// reached the end of the path — clean up
	if (CurrentPathIndex >= CachedPath.Num())
	{
		ActiveUnit->OnMoveCompleted.RemoveDynamic(this, &UUnitCommandComponent::OnMoveCompleted);
		ActiveUnit = nullptr;
		CachedPath.Empty();
		CurrentPathIndex = 0;
		return;
	}

	ActiveUnit->MoveToLocation(CachedPath[CurrentPathIndex], 5.0f);
}

void UUnitCommandComponent::OnMoveCompleted(AStrategyUnit* MovedUnit)
{
	if (!IsValid(MovedUnit))
	{
		return;
	}

	++CurrentPathIndex;
	MoveToNextPosition();
}