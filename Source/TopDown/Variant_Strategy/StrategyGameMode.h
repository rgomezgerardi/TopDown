// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StrategyGameMode.generated.h"

class AGridManager;
class AStrategyGameState;
class AStrategyPlayerController;

/**
 * GameMode for the top-down strategy game.
 * Owns references to global systems (GridManager).
 * Registers the GameState and PlayerController classes.
 */
UCLASS(abstract)
class AStrategyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AStrategyGameMode();

	virtual void BeginPlay() override;

	/** Returns the GridManager for this level */
	AGridManager* GetGridManager() const { return GridManager; }

private:

	/** Reference to the level's GridManager, cached on BeginPlay */
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;
};