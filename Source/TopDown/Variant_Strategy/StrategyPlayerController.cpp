// Copyright Epic Games, Inc. All Rights Reserved.

#include "StrategyPlayerController.h"
#include "CameraControlComponent.h"
#include "SelectionComponent.h"
#include "UnitCommandComponent.h"
#include "StrategyGameMode.h"
#include "Grid/GridManager.h"
#include "Grid/Pathfinder.h"
#include "Grid/GridTypes.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "StrategyPawn.h"
#include "InputActionValue.h"
#include "StrategyHUD.h"
#include "Engine/CollisionProfile.h"
#include "StrategyUnit.h"

AStrategyPlayerController::AStrategyPlayerController()
{
	CameraComponent = CreateDefaultSubobject<UCameraControlComponent>(TEXT("CameraControl"));
	SelectionComponent = CreateDefaultSubobject<USelectionComponent>(TEXT("Selection"));
	UnitCommandComponent = CreateDefaultSubobject<UUnitCommandComponent>(TEXT("UnitCommand"));

	bShowMouseCursor = true;
	bEnableTouchEvents = false;
	bEnableTouchOverEvents = false;
}

void AStrategyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (AStrategyGameMode* GM = Cast<AStrategyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GridManager = GM->GetGridManager();
	}
}

void AStrategyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only set up input on local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			// choose the context based on the input mode
			UInputMappingContext* ChosenContext = nullptr;

			switch (InputMode)
			{
			case SIM_Mouse:
				ChosenContext = MouseMappingContext;
				break;
			case SIM_Touch:
				ChosenContext = TouchMappingContext;
				break;
			}

			Subsystem->AddMappingContext(ChosenContext, 0);
		}

		// bind the input mappings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Camera - delegated to component
			CameraComponent->BindInputs(EnhancedInputComponent);

			// Mouse Interaction
			EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectClick);
			EnhancedInputComponent->BindAction(DeselectClickAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::DeselectClick);

			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::SelectionModifier);
			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectionModifier);
			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::SelectionModifier);

			EnhancedInputComponent->BindAction(InteractHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::InteractHoldStarted);
			EnhancedInputComponent->BindAction(InteractHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::InteractHoldTriggered);

		}
	}
}

void AStrategyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledPawn = Cast<AStrategyPawn>(InPawn);
	if (!ControlledPawn) return;

	CameraComponent->Initialize(ControlledPawn);

	StrategyHUD = Cast<AStrategyHUD>(GetHUD());

	// register highlight delegates
	SelectionComponent->OnUnitSelected.AddLambda([this](AStrategyUnit* Unit)
	{
		if (GridManager && !Unit->bHasMoved)
		{
			FGridCoordinate UnitCoord = GridManager->WorldToGrid(Unit->GetActorLocation());
			TArray<FGridCoordinate> Reachable = FPathfinder::GetReachableCells(UnitCoord, Unit->MovementRange, GridManager);
			GridManager->HighlightCells(Reachable);
		}
	});

	// clear highlights when a unit is deselected
	SelectionComponent->OnUnitDeselected.AddLambda([this](AStrategyUnit* Unit)
	{
		if (GridManager)
		{
			GridManager->ClearHighlights();
		}
	});
}

const TArray<AStrategyUnit*>& AStrategyPlayerController::GetSelectedUnits() const
{
	return SelectionComponent->GetSelectedUnits();
}

void AStrategyPlayerController::SelectClick(const FInputActionValue& Value)
{
	if (GetLocationUnderCursor(CachedSelection))
	{
		CachedInteraction = CachedSelection;
		DoSelectionCommand();
	}
}

void AStrategyPlayerController::DeselectClick(const FInputActionValue& Value)
{
	SelectionComponent->DeselectAll();
	TargetUnit = nullptr;
}

void AStrategyPlayerController::SelectionModifier(const FInputActionValue& Value)
{
	// update the selection modifier flag on the component
	SelectionComponent->SetSelectionModifier(Value.Get<bool>());
}

void AStrategyPlayerController::InteractHoldStarted(const FInputActionValue& Value)
{
	// save the starting interaction position
	StartingInteractionPosition = GetMouseLocation();
}

void AStrategyPlayerController::InteractHoldTriggered(const FInputActionValue& Value)
{
	// do a drag scroll
	FVector2D WorkingPosition;

	if (InputMode == EStrategyInputMode::SIM_Mouse)
	{
		GetMousePosition(WorkingPosition.X, WorkingPosition.Y);
	}
	else
	{
		bool bPressed;
		GetInputTouchState(ETouchIndex::Touch1, WorkingPosition.X, WorkingPosition.Y, bPressed);
	}

	CameraComponent->DragScroll(StartingInteractionPosition, WorkingPosition);
}

void AStrategyPlayerController::InteractClickStarted(const FInputActionValue& Value)
{
}

void AStrategyPlayerController::InteractClickCompleted(const FInputActionValue& Value)
{
}

void AStrategyPlayerController::DoSelectionCommand()
{
	FHitResult OutHit;
	const FVector Start = CachedSelection;
	const FVector End = Start + FVector::UpVector * 350.0f;

	FCollisionShape Sphere;
	Sphere.SetSphere(InteractionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetPawn());

	GetWorld()->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectParams, Sphere, QueryParams);

	if (OutHit.bBlockingHit)
	{
		AStrategyUnit* HitUnit = Cast<AStrategyUnit>(OutHit.GetActor());
		if (!HitUnit) return;

		if (TargetUnit && TargetUnit != HitUnit)
		{
			// different unit — deselect previous
			SelectionComponent->DeselectUnit(TargetUnit);
			TargetUnit = nullptr;
		}

		if (TargetUnit == HitUnit)
		{
			// same unit — deselect
			SelectionComponent->DeselectUnit(TargetUnit);
			TargetUnit = nullptr;
		}
		else
		{
			// new unit — select
			TargetUnit = HitUnit;
			SelectionComponent->SelectUnit(TargetUnit);
		}
	}
	else if (TargetUnit)
	{
		DoMoveUnitsCommand();
	}
}

void AStrategyPlayerController::DoMoveUnitsCommand()
{
	if (!GridManager)
	{
		return;
	}

	// get the first selected unit
	const TArray<AStrategyUnit*>& Units = SelectionComponent->GetSelectedUnits();
	if (Units.Num() == 0)
	{
		return;
	}
	AStrategyUnit* Unit = Units[0];

	// set the movement goal based on input mode
	FVector CurrentMoveGoal = (InputMode == EStrategyInputMode::SIM_Mouse) ? CachedInteraction : CachedSelection;

	if (Unit->bHasMoved)
	{
		BP_CursorFeedback(CurrentMoveGoal, false);
		return;
	}

	// convert world positions to grid coordinates
	FGridCoordinate StartCoord = GridManager->WorldToGrid(Unit->GetActorLocation());
	FGridCoordinate GoalCoord = GridManager->WorldToGrid(CurrentMoveGoal);

	// find path using A*
	TArray<FGridCoordinate> GridPath = FPathfinder::FindPath(StartCoord, GoalCoord, GridManager);

	if (GridPath.Num() == 0)
	{
		BP_CursorFeedback(CachedInteraction, false);
		return;
	}

	// reject if path exceeds unit's movement range
	int32 PathCost = GridPath.Num() - 1;
	if (PathCost > Unit->MovementRange)
	{
		BP_CursorFeedback(CachedInteraction, false);
		return;
	}

	// convert grid path to world positions
	TArray<FVector> WorldPath;
	WorldPath.Reserve(GridPath.Num());
	for (const FGridCoordinate& Coord : GridPath)
	{
		WorldPath.Add(GridManager->GridToWorld(Coord));
	}

	// move the unit along the world path
	const bool bSuccess = UnitCommandComponent->MoveUnit(Unit, WorldPath);

	if (bSuccess)
	{
		Unit->bHasMoved = true;
		GridManager->ClearHighlights();
	}

	BP_CursorFeedback(CachedInteraction, bSuccess);
}

FVector2D AStrategyPlayerController::GetMouseLocation()
{
	// attempt to get the mouse position from this PC
	float MouseX, MouseY;

	if (GetMousePosition(MouseX, MouseY))
	{
		return FVector2D(MouseX, MouseY);
	}

	// return an invalid vector
	return FVector2D::ZeroVector;
}

bool AStrategyPlayerController::GetLocationUnderCursor(FVector& Location)
{
	// trace the visibility channel at the cursor location
	FHitResult OutHit;

	GetHitResultUnderCursorByChannel(SelectionTraceChannel, true, OutHit);

	// if there was a blocking hit, return the hit location
	if (OutHit.bBlockingHit)
	{
		Location = OutHit.Location;
		return true;
	}

	return OutHit.bBlockingHit;
}