
#pragma once

#include "CoreMinimal.h"
#include "CellData.generated.h"

/**
 * Gameplay data for a single grid cell.
 * Stores properties that affect pathfinding, combat, and unit placement.
 */
USTRUCT(BlueprintType)
struct FCellData
{
    GENERATED_BODY()

    /** Whether units can traverse this cell */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    bool bWalkable = true;

    /** Whether a unit currently occupies this cell */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    bool bOccupied = false;
};