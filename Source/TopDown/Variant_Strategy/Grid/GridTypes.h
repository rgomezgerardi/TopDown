#pragma once

#include "CoreMinimal.h"
#include "GridTypes.generated.h"

/**
 * Tactic Grid 3D cordinates
 * X, Y = horizontal position
 * Floor = vertical/height position
 */
USTRUCT(BlueprintType)
struct FGridCoordinate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 Y;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 Floor;
    
    // Default Constructor
    FGridCoordinate()
        : X(0), Y(0), Floor(0)
    {
    }

    // Parameterized Constructor
    FGridCoordinate(int32 InX, int32 InY, int32 InFloor)
        : X(InX), Y(InY), Floor(InFloor)
    {
    }

    bool operator==(const FGridCoordinate& Other) const
    {
        return X == Other.X && Y == Other.Y && Floor == Other.Floor;
    }

    bool operator!=(const FGridCoordinate& Other) const
    {
        return !(*this == Other);
    }

    // Hash function (for TMap or TSet)
    friend uint32 GetTypeHash(const FGridCoordinate &Coord)
    {
        uint32 Hash = HashCombine(GetTypeHash(Coord.X), GetTypeHash(Coord.Y));
        Hash = HashCombine(Hash, GetTypeHash(Coord.Floor));
        return Hash;
    }

    // ToString for debugging
    FString ToString() const
    {
        return FString::Printf(TEXT("(%d, %d, F%d)"), X, Y, Floor);
    }
};