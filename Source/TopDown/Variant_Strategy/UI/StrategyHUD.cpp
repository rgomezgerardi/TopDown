// Copyright Epic Games, Inc. All Rights Reserved.

#include "StrategyHUD.h"
#include "StrategyUnit.h"
#include "StrategyPlayerController.h"
#include "StrategyUI.h"

void AStrategyHUD::BeginPlay()
{
	Super::BeginPlay();

	// spawn the UI widget
	UIWidget = CreateWidget<UStrategyUI>(GetOwningPlayerController(), UIWidgetClass);
	check(UIWidget);

	// add the UI widget to the screen
	UIWidget->AddToViewport(0);
}

void AStrategyHUD::DrawHUD()
{
	Super::DrawHUD();

	if (AStrategyPlayerController* PC = Cast<AStrategyPlayerController>(GetOwningPlayerController()))
	{
		const TArray<AStrategyUnit*>& SelectedUnits = PC->GetSelectedUnits();

		UIWidget->SetSelectedUnitsCount(SelectedUnits.Num());

		for (AStrategyUnit* CurrentUnit : SelectedUnits)
		{
			if (IsValid(CurrentUnit))
			{
				FVector2D ScreenCoords;
				if (PC->ProjectWorldLocationToScreen(CurrentUnit->GetActorLocation(), ScreenCoords, true))
				{
					DrawText("Selected", FColor::White, ScreenCoords.X - 25.0f, ScreenCoords.Y + 25.0f, nullptr, 1.5f);
				}
			}
		}
	}
}