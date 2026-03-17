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
		ActiveUnit->StopMoving();
		ActiveUnit = nullptr;
	}

	ActiveUnit = Unit;
	FinalDestination = Path.Last();

	ActiveUnit->OnMoveCompleted.AddDynamic(this, &UUnitCommandComponent::OnMoveCompleted);

	return ActiveUnit->MoveAlongPath(Path, 5.0f);
}

FVector UUnitCommandComponent::GetDestination() const
{
	return FinalDestination;
}

void UUnitCommandComponent::OnMoveCompleted(AStrategyUnit* MovedUnit)
{
	if (!IsValid(MovedUnit))
	{
		return;
	}

	ActiveUnit->OnMoveCompleted.RemoveDynamic(this, &UUnitCommandComponent::OnMoveCompleted);
	ActiveUnit = nullptr;
}
