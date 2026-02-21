// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnitCommandComponent.h"
#include "StrategyUnit.h"
#include "NavigationSystem.h"
#include "Engine/OverlapResult.h"

UUnitCommandComponent::UUnitCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUnitCommandComponent::Initialize(float InInteractionRadius)
{
	InteractionRadius = InInteractionRadius;
}

bool UUnitCommandComponent::MoveUnits(const TArray<AStrategyUnit*>& Units, FVector Goal)
{
	// cache the goal so OnMoveCompleted can use it after the cursor has moved
	CachedGoal = Goal;

	// cache the units so OnMoveCompleted can exclude them from the interaction overlap test
	CachedUnits = Units;

	// get the closest unit to the goal — this will be our lead unit
	AStrategyUnit* Closest = GetClosestUnitToLocation(Units, Goal);

	// this will be set to true if any of the move requests fail
	bool bInteractionFailed = false;

	// process each unit in the list
	for (AStrategyUnit* CurrentUnit : Units)
	{
		if (IsValid(CurrentUnit))
		{
			// stop the unit
			CurrentUnit->StopMoving();

			// move the lead unit to the goal, all other units to random navigable points around it
			FVector MoveGoal = Goal;

			if (CurrentUnit != Closest)
			{
				UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(GetWorld(), Goal, MoveGoal, InteractionRadius * 0.66f);
			}

			// subscribe to the unit's move completed delegate
			CurrentUnit->OnMoveCompleted.AddDynamic(this, &UUnitCommandComponent::OnMoveCompleted);

			// set up movement to the goal location
			if (!CurrentUnit->MoveToLocation(MoveGoal, InteractionRadius * 0.66f))
			{
				bInteractionFailed = true;
			}
		}
	}

	return !bInteractionFailed;
}

void UUnitCommandComponent::ResetInteraction()
{
	bAllowInteraction = true;
}

void UUnitCommandComponent::OnMoveCompleted(AStrategyUnit* MovedUnit)
{
	// is the unit valid?
	if (IsValid(MovedUnit))
	{
		// unsubscribe from the delegate
		MovedUnit->OnMoveCompleted.RemoveDynamic(this, &UUnitCommandComponent::OnMoveCompleted);

		// skip if interactions are locked
		if (!bAllowInteraction)
		{
			return;
		}

		// disallow additional interactions until we reset
		bAllowInteraction = false;

		// is the unit close enough to the cached goal location?
		if (FVector::Dist2D(CachedGoal, MovedUnit->GetActorLocation()) < InteractionRadius)
		{
			// do an overlap test to find nearby interactive objects
			TArray<FOverlapResult> OutOverlaps;

			FCollisionShape CollisionSphere;
			CollisionSphere.SetSphere(InteractionRadius);

			FCollisionObjectQueryParams ObjectParams;
			ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MovedUnit);

			for (const AStrategyUnit* CurSelected : CachedUnits)
			{
				QueryParams.AddIgnoredActor(CurSelected);
			}

			if (GetWorld()->OverlapMultiByObjectType(OutOverlaps, CachedGoal, FQuat::Identity, ObjectParams, CollisionSphere, QueryParams))
			{
				for (const FOverlapResult& CurrentOverlap : OutOverlaps)
				{
					if (AStrategyUnit* CurrentUnit = Cast<AStrategyUnit>(CurrentOverlap.GetActor()))
					{
						CurrentUnit->Interact(MovedUnit);
					}
				}
			}
		}
	}
}

AStrategyUnit* UUnitCommandComponent::GetClosestUnitToLocation(const TArray<AStrategyUnit*>& Units, FVector Location)
{
	// closest unit and distance
	AStrategyUnit* OutUnit = nullptr;
	float Closest = 0.0f;

	// process each unit in the list
	for (AStrategyUnit* CurrentUnit : Units)
	{
		if (CurrentUnit != nullptr)
		{
			// have we found a candidate already?
			if (OutUnit != nullptr)
			{
				// calculate the squared distance to the target location
				float Dist = FVector::DistSquared2D(Location, CurrentUnit->GetActorLocation());

				// is this unit closer?
				if (Dist < Closest)
				{
					OutUnit = CurrentUnit;
					Closest = Dist;
				}
			}
			else
			{
				// no previously selected unit, so use this one
				OutUnit = CurrentUnit;
				Closest = FVector::DistSquared2D(Location, CurrentUnit->GetActorLocation());
			}
		}
	}

	return OutUnit;
}
