
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "StrategyGameState.generated.h"

class AStrategyUnit;

/** Defines the current phase of a player turn */
UENUM(BlueprintType)
enum class EStrategyTurnPhase : uint8
{
	PlayerMovement	UMETA(DisplayName = "Player Movement"),
	PlayerAction	UMETA(DisplayName = "Player Action"),
	EnemyTurn		UMETA(DisplayName = "Enemy Turn")
};

/**
 * Tracks the current state of the strategy game.
 * Stores turn phase, turn number, and which units have already acted.
 * Lives on AStrategyGameMode and is accessible from any system.
 */
UCLASS()
class AStrategyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/** Current phase of the active turn */
	UPROPERTY(BlueprintReadOnly, Category = "Turn")
	EStrategyTurnPhase TurnPhase = EStrategyTurnPhase::PlayerMovement;

	/** Current turn number, starts at 1 */
	UPROPERTY(BlueprintReadOnly, Category = "Turn")
	int32 TurnNumber = 1;

	/** Units that have already used their action this turn */
	UPROPERTY(BlueprintReadOnly, Category = "Turn")
	TArray<AStrategyUnit*> UnitsActedThisTurn;

public:

	/** Returns true if the given unit has already acted this turn */
	bool HasUnitActed(AStrategyUnit* Unit) const;

	/** Marks a unit as having acted this turn */
	void MarkUnitActed(AStrategyUnit* Unit);

	/** Clears all acted units — called at the start of each new player turn */
	void ResetActedUnits();

	/** Returns the current turn phase as a display string (for HUD) */
	UFUNCTION(BlueprintPure, Category = "Turn")
	FString GetTurnPhaseString() const;
};