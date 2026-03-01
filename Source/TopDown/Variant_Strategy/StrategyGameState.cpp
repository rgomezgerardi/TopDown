
#include "StrategyGameState.h"
#include "StrategyUnit.h"

bool AStrategyGameState::HasUnitActed(AStrategyUnit* Unit) const
{
	return UnitsActedThisTurn.Contains(Unit);
}

void AStrategyGameState::MarkUnitActed(AStrategyUnit* Unit)
{
	if (IsValid(Unit) && !UnitsActedThisTurn.Contains(Unit))
	{
		UnitsActedThisTurn.Add(Unit);
	}
}

void AStrategyGameState::ResetActedUnits()
{
	UnitsActedThisTurn.Empty();
}

FString AStrategyGameState::GetTurnPhaseString() const
{
	switch (TurnPhase)
	{
	case EStrategyTurnPhase::PlayerMovement:
		return TEXT("Movement");
	case EStrategyTurnPhase::PlayerAction:
		return TEXT("Action");
	case EStrategyTurnPhase::EnemyTurn:
		return TEXT("Enemy Turn");
	default:
		return TEXT("Unknown");
	}
}