// Copyright Epic Games, Inc. All Rights Reserved.

#include "StrategyGameMode.h"
#include "StrategyGameState.h"
#include "StrategyPlayerController.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"

AStrategyGameMode::AStrategyGameMode()
{
	GameStateClass = AStrategyGameState::StaticClass();
	PlayerControllerClass = AStrategyPlayerController::StaticClass();
}

void AStrategyGameMode::BeginPlay()
{
	Super::BeginPlay();

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	UE_LOG(LogTemp, Warning, TEXT("AStrategyGameMode::BeginPlay - GridManager: %s"), GridManager ? TEXT("valid") : TEXT("null"));

	if (!GridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("AStrategyGameMode: No AGridManager found in level."));
	}
}