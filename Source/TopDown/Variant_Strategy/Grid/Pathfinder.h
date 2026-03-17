#pragma once

#include "CoreMinimal.h"
#include "GridTypes.h"

/**
 * Pathfinding system using A* algorithm.
 * Finds shortest path between two grid coordinates considering walkability.
 */
class TOPDOWN_API FPathfinder
{
public:
    /**
     * Find path from Start to Goal using A* algorithm.
     * 
     * @param Start Starting grid coordinate
     * @param Goal Target grid coordinate
     * @param GridManager Reference to grid for walkability checks
     * @return Array of coordinates forming path (empty if no path found)
     */
    static TArray<FGridCoordinate> FindPath(
        const FGridCoordinate& Start,
        const FGridCoordinate& Goal,
        class AGridManager* GridManager
    );

    /** Get all cells reachable from Origin within MaxRange steps using BFS.
     *  Excludes Origin itself. */
    static TArray<FGridCoordinate> GetReachableCells(
        const FGridCoordinate& Origin,
        int32 MaxRange,
        class AGridManager* GridManager
    );

private:
    /** Internal node representation for A* */
    struct FPathNode
    {
        FGridCoordinate Coord;
        int32 GCost;  // Distance from start
        int32 HCost;  // Heuristic distance to goal
        int32 FCost;  // G + H (total estimated cost)
        FGridCoordinate Parent;  // For path reconstruction
        
        FPathNode()
            : Coord(0, 0, 0)
            , GCost(0)
            , HCost(0)
            , FCost(0)
            , Parent(0, 0, 0)
        {}
        
        FPathNode(FGridCoordinate InCoord, int32 InG, int32 InH, FGridCoordinate InParent)
            : Coord(InCoord)
            , GCost(InG)
            , HCost(InH)
            , FCost(InG + InH)
            , Parent(InParent)
        {}
    };
    
    /** Calculate Manhattan distance heuristic */
    static int32 CalculateHeuristic(const FGridCoordinate& A, const FGridCoordinate& B);
    
    /** Get walkable neighbors of a coordinate */
    static TArray<FGridCoordinate> GetNeighbors(
        const FGridCoordinate& Coord,
        class AGridManager* GridManager
    );
    
    /** Reconstruct path from goal to start using parent chain */
    static TArray<FGridCoordinate> ReconstructPath(
        const TMap<FGridCoordinate, FPathNode>& AllNodes,
        const FGridCoordinate& Goal
    );
};