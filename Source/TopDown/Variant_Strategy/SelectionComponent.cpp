#include "SelectionComponent.h"
#include "StrategyUnit.h"
#include "Kismet/GameplayStatics.h"

USelectionComponent::USelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USelectionComponent::SelectUnit(AStrategyUnit* Unit)
{
	if (!IsValid(Unit) || SelectedUnits.Contains(Unit))
	{
		return;
	}

	SelectedUnits.Add(Unit);
	Unit->UnitSelected();
	OnUnitSelected.Broadcast(Unit);
}

void USelectionComponent::DeselectUnit(AStrategyUnit* Unit)
{
	if (!IsValid(Unit))
	{
		return;
	}

	if (SelectedUnits.Remove(Unit) > 0)
	{
		Unit->UnitDeselected();
		OnUnitDeselected.Broadcast(Unit);
	}
}

void USelectionComponent::DeselectAll()
{
	// notify each unit before clearing the list
	for (AStrategyUnit* Unit : SelectedUnits)
	{
		if (IsValid(Unit))
		{
			Unit->UnitDeselected();
			OnUnitDeselected.Broadcast(Unit);
		}
	}

	SelectedUnits.Empty();
}

void USelectionComponent::SelectAllOnScreen(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// find all strategy units in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, AStrategyUnit::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AStrategyUnit* Unit = Cast<AStrategyUnit>(Actor);

		// only select units that are visible on screen and not already selected
		if (Unit && Unit->WasRecentlyRendered(0.2f) && !SelectedUnits.Contains(Unit))
		{
			SelectedUnits.Add(Unit);
			Unit->UnitSelected();
		}
	}
}

void USelectionComponent::DragSelectUnits(const TArray<AStrategyUnit*>& Units)
{
	if (Units.Num() > 0)
	{
		// replace current selection with the boxed units
		DeselectAll();

		for (AStrategyUnit* Unit : Units)
		{
			if (IsValid(Unit))
			{
				SelectedUnits.Add(Unit);
				Unit->UnitSelected();
			}
		}
	}
	else
	{
		// nothing in the box — clear the current selection
		if (SelectedUnits.Num() > 0)
		{
			DeselectAll();
		}
	}
}

void USelectionComponent::SetSelectionModifier(bool bActive)
{
	bSelectionModifier = bActive;
}

bool USelectionComponent::GetSelectionModifier() const
{
	return bSelectionModifier;
}

bool USelectionComponent::GetDoubleTapActive() const
{
	return bDoubleTapActive;
}

void USelectionComponent::SetDoubleTapActive(bool bActive)
{
	bDoubleTapActive = bActive;
}

const TArray<AStrategyUnit*>& USelectionComponent::GetSelectedUnits() const
{
	return SelectedUnits;
}

int32 USelectionComponent::GetSelectedUnitCount() const
{
	return SelectedUnits.Num();
}