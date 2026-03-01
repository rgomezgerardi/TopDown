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

	// mouse cursor should always be shown
	bShowMouseCursor = true;
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
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::SelectHoldStarted);
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::SelectHoldTriggered);
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectHoldCompleted);
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::SelectHoldCompleted);

			EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectClick);
			EnhancedInputComponent->BindAction(DeselectClickAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::DeselectClick);

			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::SelectionModifier);
			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectionModifier);
			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::SelectionModifier);

			EnhancedInputComponent->BindAction(InteractHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::InteractHoldStarted);
			EnhancedInputComponent->BindAction(InteractHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::InteractHoldTriggered);

			// Touch Interaction
			EnhancedInputComponent->BindAction(TouchPrimaryHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::TouchPrimaryHoldStarted);
			EnhancedInputComponent->BindAction(TouchPrimaryHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::TouchPrimaryHoldTriggered);
			EnhancedInputComponent->BindAction(TouchPrimaryHoldAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::TouchPrimaryHoldCompleted);

			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Started, this, &AStrategyPlayerController::TouchSecondaryStarted);
			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::TouchSecondaryTriggered);
			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::TouchSecondaryCompleted);
			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::TouchSecondaryCompleted);
		}
	}
}

void AStrategyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// ensure we have the right pawn type
	ControlledPawn = Cast<AStrategyPawn>(InPawn);
	check(ControlledPawn);

	CameraComponent->Initialize(ControlledPawn);

	// cast the HUD pointer
	StrategyHUD = Cast<AStrategyHUD>(GetHUD());
	check(StrategyHUD);
}

const TArray<AStrategyUnit*>& AStrategyPlayerController::GetSelectedUnits() const
{
	return SelectionComponent->GetSelectedUnits();
}

void AStrategyPlayerController::SelectHoldStarted(const FInputActionValue& Value)
{
	// save the selection start position
	StartingSelectionPosition = GetMouseLocation();
}

void AStrategyPlayerController::SelectHoldTriggered(const FInputActionValue& Value)
{
	// get the current mouse position
	FVector2D SelectionPosition = GetMouseLocation();

	// calculate the size of the selection box
	FVector2D SelectionSize = SelectionPosition - StartingSelectionPosition;

	// update the selection box on the HUD
	if (StrategyHUD)
	{
		StrategyHUD->DragSelectUpdate(StartingSelectionPosition, SelectionSize, SelectionPosition, true);
	}
}

void AStrategyPlayerController::SelectHoldCompleted(const FInputActionValue& Value)
{
	// reset the drag box on the HUD
	if (StrategyHUD)
	{
		StrategyHUD->DragSelectUpdate(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, false);
	}
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
	UE_LOG(LogTemp, Warning, TEXT("InteractClickCompleted: UnitCount=%d"), SelectionComponent->GetSelectedUnitCount());

	bool bGotLocation = GetLocationUnderCursor(CachedInteraction);
	UE_LOG(LogTemp, Warning, TEXT("GetLocationUnderCursor: %s, Location=%s"), bGotLocation ? TEXT("true") : TEXT("false"), *CachedInteraction.ToString());

	if (SelectionComponent->GetSelectedUnitCount() > 0 && bGotLocation)
	{
		if (SelectionComponent->GetDoubleTapActive())
		{
			SelectionComponent->SetDoubleTapActive(false);
		}
		else
		{
			DoMoveUnitsCommand();
		}
	}
}

void AStrategyPlayerController::TouchPrimaryHoldStarted(const FInputActionValue& Value)
{
	// save the tap press time
	LastTapPressTime = GetWorld()->GetRealTimeSeconds();

	// save the starting interaction position
	StartingInteractionPosition = Value.Get<FVector2D>();
}

void AStrategyPlayerController::TouchPrimaryHoldTriggered(const FInputActionValue& Value)
{
	// is this touch longer than a tap?
	if ((GetWorld()->GetRealTimeSeconds() - LastTapPressTime) > TouchTapMaxAllowedTime)
	{
		// if we're not doing a box select, do a drag scroll
		if (!SelectionComponent->GetSelectionModifier())
		{
			FVector2D WorkingPosition;
			bool bPressed;
			GetInputTouchState(ETouchIndex::Touch1, WorkingPosition.X, WorkingPosition.Y, bPressed);
			CameraComponent->DragScroll(StartingInteractionPosition, WorkingPosition);
		}
	}
}

void AStrategyPlayerController::TouchPrimaryHoldCompleted(const FInputActionValue& Value)
{
	// check if we're doing a tap or double tap.
	// we have to do this manually because EnhancedInput tap triggers work differently on touch inputs
	bool bTapped = false;
	bool bDoubleTapped = false;

	CheckTouchTap(bTapped, bDoubleTapped);

	if (bTapped)
	{
		if (bDoubleTapped)
		{
			// ensure we are not doing a box select
			if (!SelectionComponent->GetSelectionModifier())
			{
				// depending on the double tap toggle, select or deselect all units
				if (SelectionComponent->GetDoubleTapActive())
				{
					SelectionComponent->DeselectAll();
				}
				else
				{
					SelectionComponent->SelectAllOnScreen(GetWorld());
				}

				// toggle the double tap flag
				SelectionComponent->SetDoubleTapActive(!SelectionComponent->GetDoubleTapActive());
			}
		}

	// no double tap, handle this touch input normally
	}
	else
	{
		// ensure we're not already box selecting, or were just box selecting
		if (!(SelectionComponent->GetSelectionModifier() || (GetWorld()->GetRealTimeSeconds() - LastBoxSelectTime) < TouchTapMaxAllowedTime))
		{
			// project the touch location and cache the selection point
			CachedInteraction = CachedSelection = ProjectTouchPointToWorldSpace();

			// do a selection action with the cached location
			DoSelectionCommand();
		}
	}
}

void AStrategyPlayerController::TouchSecondaryStarted(const FInputActionValue& Value)
{
	// raise the selection modifier flag
	SelectionComponent->SetSelectionModifier(true);

	// save the starting position for the second finger
	StartingSecondFingerPosition = Value.Get<FVector2D>();
}

void AStrategyPlayerController::TouchSecondaryTriggered(const FInputActionValue& Value)
{
	// update the current position for the second finger
	CurrentSecondFingerPosition = Value.Get<FVector2D>();

	// are we box selecting, and the finger has moved enough on the touchscreen?
	if (SelectionComponent->GetSelectionModifier() && !StartingSecondFingerPosition.Equals(CurrentSecondFingerPosition, 10.0f))
	{
		// update the current interaction position
		CurrentInteractionPosition = CurrentSecondFingerPosition;

		// update the selection box on the HUD
		if (StrategyHUD)
		{
			const FVector2D DragSize = CurrentSecondFingerPosition - StartingSecondFingerPosition;
			StrategyHUD->DragSelectUpdate(StartingInteractionPosition, DragSize, CurrentSecondFingerPosition, true);
		}
	}
}

void AStrategyPlayerController::TouchSecondaryCompleted(const FInputActionValue& Value)
{
	// lower the selection modifier flag
	SelectionComponent->SetSelectionModifier(false);

	// save the last box selection time
	LastBoxSelectTime = GetWorld()->GetRealTimeSeconds();

	// hide the selection box on the HUD
	if (StrategyHUD)
	{
		StrategyHUD->DragSelectUpdate(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, false);
	}
}

void AStrategyPlayerController::DoSelectionCommand()
{
	// do a sphere sweep to look for actors to select
	FHitResult OutHit;

	const FVector Start = CachedSelection;
	const FVector End = Start + FVector::UpVector * 350.0f;

	FCollisionShape InteractionSphere;
	InteractionSphere.SetSphere(InteractionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetPawn());
	QueryParams.bTraceComplex = true;

	GetWorld()->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectParams, InteractionSphere, QueryParams);

	// did we hit a unit?
	if (OutHit.bBlockingHit)
	{
		TargetUnit = Cast<AStrategyUnit>(OutHit.GetActor());

		if (TargetUnit)
		{
			// SelectUnit handles the toggle: adds if not selected, removes if already selected
			SelectionComponent->SelectUnit(TargetUnit);
		}
	}
	else
	{
		if (InputMode == SIM_Touch || (InputMode == SIM_Mouse && SelectionComponent->GetSelectedUnitCount() > 0))
		{
			DoMoveUnitsCommand();
		}
	}
}

void AStrategyPlayerController::DoMoveUnitsCommand()
{
	// get GridManager from GameMode on demand — guaranteed available after OnActorsInitialized
	if (!GridManager)
	{
		if (AStrategyGameMode* GM = Cast<AStrategyGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GridManager = GM->GetGridManager();
			UE_LOG(LogTemp, Warning, TEXT("GameMode found: %s, GridManager: %s"), *GM->GetName(), GridManager ? TEXT("valid") : TEXT("null"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode cast failed"));
		}
	}

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

	// convert world positions to grid coordinates
	FGridCoordinate StartCoord = GridManager->WorldToGrid(Unit->GetActorLocation());
	FGridCoordinate GoalCoord = GridManager->WorldToGrid(CurrentMoveGoal);

	UE_LOG(LogTemp, Warning, TEXT("Unit world pos: %s"), *Unit->GetActorLocation().ToString());
	UE_LOG(LogTemp, Warning, TEXT("StartCoord: %s"), *StartCoord.ToString());
	UE_LOG(LogTemp, Warning, TEXT("GoalCoord: %s"), *GoalCoord.ToString());

	// find path using A*
	TArray<FGridCoordinate> GridPath = FPathfinder::FindPath(StartCoord, GoalCoord, GridManager);

	UE_LOG(LogTemp, Warning, TEXT("Path length: %d"), GridPath.Num());

	if (GridPath.Num() == 0)
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

FVector AStrategyPlayerController::ProjectTouchPointToWorldSpace()
{
	// get the touch coordinates for the first finger
	float TouchX, TouchY = 0.0f;
	bool bPressed = false;

	GetInputTouchState(ETouchIndex::Touch1, TouchX, TouchY, bPressed);

	FVector WorldLocation = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;

	// deproject the coords into world space
	if (DeprojectScreenPositionToWorld(TouchX, TouchY, WorldLocation, WorldDirection))
	{
		// intersect with a horizontal plane and return the resulting point
		const FPlane IntersectPlane(FVector::ZeroVector, FVector::UpVector);

		return FMath::LinePlaneIntersection(WorldLocation, WorldLocation + (WorldDirection * 100000.0f), IntersectPlane);
	}

	// failed to deproject, return a zero vector
	return FVector::ZeroVector;
}

void AStrategyPlayerController::CheckTouchTap(bool& bTapped, bool& bDoubleTapped)
{
	// get the current game time
	const float GameTime = GetWorld()->GetRealTimeSeconds();

	// if the player released touch before the max allowed time since press, we have a tap
	bTapped = (GameTime - LastTapPressTime) < TouchTapMaxAllowedTime;

	if (bTapped)
	{
		// we have a double tap if another tap happened before the last release time
		if ((GameTime - LastTapReleaseTime) < TouchDoubleTapMaxAllowedTime)
		{
			++TapCount;
		}
		else
		{
			TapCount = 0;
		}
	}
	else
	{
		TapCount = 0;
	}

	// we have a double tap if the tap count is not zero
	bDoubleTapped = TapCount >= 1;

	// save the tap release time
	LastTapReleaseTime = GameTime;
}