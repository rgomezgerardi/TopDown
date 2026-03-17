#include "Grid/Pathfinder.h"
#include "Grid/GridManager.h"

TArray<FGridCoordinate> FPathfinder::FindPath(
    const FGridCoordinate& Start,
    const FGridCoordinate& Goal,
    AGridManager* GridManager)
{
    // Validate inputs
    if (!GridManager)
    {
        return TArray<FGridCoordinate>();
    }
    
    if (!GridManager->IsCellWalkable(Start) || !GridManager->IsCellWalkable(Goal))
    {
        return TArray<FGridCoordinate>();
    }
    
    if (Start == Goal)
    {
        return TArray<FGridCoordinate>{Start};
    }
    
    // Open set: nodes to evaluate (priority queue behavior)
    TArray<FGridCoordinate> OpenSet;
    OpenSet.Add(Start);
    
    // Closed set: nodes already evaluated
    TSet<FGridCoordinate> ClosedSet;
    
    // All nodes data (G, H, F costs and parent)
    TMap<FGridCoordinate, FPathNode> AllNodes;
    AllNodes.Add(Start, FPathNode(Start, 0, CalculateHeuristic(Start, Goal), FGridCoordinate(-1, -1, -1)));
    
    // A* main loop
    while (OpenSet.Num() > 0)
    {
        // Find node with lowest F cost in open set
        FGridCoordinate Current = OpenSet[0];
        int32 CurrentIndex = 0;
        
        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            FPathNode* NodeA = AllNodes.Find(OpenSet[i]);
            FPathNode* NodeB = AllNodes.Find(Current);
            
            if (NodeA && NodeB && NodeA->FCost < NodeB->FCost)
            {
                Current = OpenSet[i];
                CurrentIndex = i;
            }
        }
        
        // Remove current from open set, add to closed set
        OpenSet.RemoveAt(CurrentIndex);
        ClosedSet.Add(Current);
        
        // Goal reached
        if (Current == Goal)
        {
            return ReconstructPath(AllNodes, Goal);
        }
        
        // Evaluate neighbors
        TArray<FGridCoordinate> Neighbors = GetNeighbors(Current, GridManager);
        
        for (const FGridCoordinate& Neighbor : Neighbors)
        {
            // Skip if already evaluated
            if (ClosedSet.Contains(Neighbor))
            {
                continue;
            }
            
            // Calculate tentative G cost (distance from start to neighbor through current)
            FPathNode* CurrentNode = AllNodes.Find(Current);
            if (!CurrentNode)
            {
                continue;
            }
            
            int32 TentativeG = CurrentNode->GCost + 1;  // Movement cost = 1 per cell
            
            // Check if this path to neighbor is better
            FPathNode* NeighborNode = AllNodes.Find(Neighbor);
            bool bIsNewNode = !NeighborNode;
            bool bIsBetterPath = bIsNewNode || TentativeG < NeighborNode->GCost;
            
            if (bIsBetterPath)
            {
                // Update or create neighbor node
                int32 H = CalculateHeuristic(Neighbor, Goal);
                FPathNode NewNode(Neighbor, TentativeG, H, Current);
                AllNodes.Add(Neighbor, NewNode);
                
                // Add to open set if new
                if (bIsNewNode)
                {
                    OpenSet.Add(Neighbor);
                }
            }
        }
    }
    
    // No path found
    return TArray<FGridCoordinate>();
}

TArray<FGridCoordinate> FPathfinder::GetReachableCells(
    const FGridCoordinate& Origin,
    int32 MaxRange,
    AGridManager* GridManager)
{
    if (!GridManager || MaxRange <= 0)
    {
        return TArray<FGridCoordinate>();
    }

    TArray<FGridCoordinate> Reachable;
    TMap<FGridCoordinate, int32> Visited; // coord → min cost to reach it

    TQueue<TPair<FGridCoordinate, int32>> Queue;
    Queue.Enqueue(TPair<FGridCoordinate, int32>(Origin, 0));
    Visited.Add(Origin, 0);

    while (!Queue.IsEmpty())
    {
        TPair<FGridCoordinate, int32> Current;
        Queue.Dequeue(Current);

        FGridCoordinate Coord = Current.Key;
        int32 Cost = Current.Value;

        if (Coord != Origin)
        {
            Reachable.Add(Coord);
        }

        if (Cost >= MaxRange)
        {
            continue;
        }

        TArray<FGridCoordinate> Neighbors = GetNeighbors(Coord, GridManager);
        for (const FGridCoordinate& Neighbor : Neighbors)
        {
            int32 NewCost = Cost + 1;
            if (!Visited.Contains(Neighbor) || Visited[Neighbor] > NewCost)
            {
                Visited.Add(Neighbor, NewCost);
                Queue.Enqueue(TPair<FGridCoordinate, int32>(Neighbor, NewCost));
            }
        }
    }

    return Reachable;
}

int32 FPathfinder::CalculateHeuristic(const FGridCoordinate& A, const FGridCoordinate& B)
{
    // Manhattan distance
    return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y) + FMath::Abs(A.Floor - B.Floor);
}

TArray<FGridCoordinate> FPathfinder::GetNeighbors(
    const FGridCoordinate& Coord,
    AGridManager* GridManager)
{
    TArray<FGridCoordinate> Neighbors;
    
    // Cardinal directions: North, South, East, West
    TArray<FGridCoordinate> Directions = {
        FGridCoordinate(0, 1, 0),   // North
        FGridCoordinate(0, -1, 0),  // South
        FGridCoordinate(1, 0, 0),   // East
        FGridCoordinate(-1, 0, 0)   // West
    };
    
    for (const FGridCoordinate& Dir : Directions)
    {
        FGridCoordinate Neighbor = Coord + Dir;
        
        // Check if walkable
        if (GridManager->IsValidGridPosition(Neighbor) && GridManager->IsCellWalkable(Neighbor))
        {
            Neighbors.Add(Neighbor);
        }
    }
    
    return Neighbors;
}

TArray<FGridCoordinate> FPathfinder::ReconstructPath(
    const TMap<FGridCoordinate, FPathNode>& AllNodes,
    const FGridCoordinate& Goal)
{
    TArray<FGridCoordinate> Path;
    FGridCoordinate Current = Goal;
    
    // Follow parent chain from goal to start
    while (true)
    {
        Path.Add(Current);
        
        const FPathNode* Node = AllNodes.Find(Current);
        if (!Node)
        {
            break;
        }
        
        // Check if we reached start (parent is invalid sentinel)
        if (Node->Parent.X == -1 && Node->Parent.Y == -1)
        {
            break;
        }
        
        Current = Node->Parent;
    }
    
    // Reverse to get start → goal order
    Algo::Reverse(Path);
    
    return Path;
}