// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StrategyHUD.generated.h"

class UStrategyUI;

/**
 *  Simple strategy game HUD
 *  Draws the selection box and unit selected overlays
 */
UCLASS(abstract)
class AStrategyHUD : public AHUD
{
	GENERATED_BODY()
	
protected:

	/** Pointer to the UI user widget */
	UPROPERTY()
	TObjectPtr<UStrategyUI> UIWidget;

	/** Type of UI Widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UStrategyUI> UIWidgetClass;

public:

	/** Initialization */
	virtual void BeginPlay() override;
protected:

	/** Draws the HUD */
	virtual void DrawHUD() override;
};