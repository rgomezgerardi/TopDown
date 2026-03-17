// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid/GridManager.h"
#include "Grid/Pathfinder.h"
#include "DrawDebugHelpers.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeGrid();
}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShowDebugGrid)
	{
		DrawDebugGrid();
	}
}

FGridCoordinate AGridManager::WorldToGrid(const FVector &WorldLocation) const
{
	// Offset relative to grid origin
	FVector LocalPosition = WorldLocation - GridOrigin;

	// Conversión a coordenadas discretas
	int32 X = FMath::FloorToInt(LocalPosition.X / CellSize);
	int32 Y = FMath::FloorToInt(LocalPosition.Y / CellSize);
	int32 Floor = FMath::Max(0, FMath::FloorToInt(LocalPosition.Z / FloorHeight));

	return FGridCoordinate(X, Y, Floor);
}

FVector AGridManager::GridToWorld(const FGridCoordinate &GridCoord) const
{
	// Cell Center
	float WorldX = (GridCoord.X + 0.5f) * CellSize;
	float WorldY = (GridCoord.Y + 0.5f) * CellSize;
	float WorldZ = (GridCoord.Floor + 0.5f) * FloorHeight;

	return GridOrigin + FVector(WorldX, WorldY, WorldZ);
}

bool AGridManager::IsValidGridPosition(const FGridCoordinate &GridCoord) const
{
	// Por ahora solo valida que no sea negativo
	// Más adelante: verificar contra datos del nivel
	return GridCoord.X >= 0 && GridCoord.Y >= 0 && GridCoord.Floor >= 0;
}

void AGridManager::DrawDebugGrid() const
{
    if (!GetWorld()) return;
    
    const float HalfCellSize = CellSize * 0.5f;
    const float BoxHeight = 5.0f;  // Thin box height
    
    for (int32 Floor = 0; Floor < DebugFloorCount; ++Floor)
    {
        for (int32 Y = 0; Y < DebugGridSizeY; ++Y)
        {
            for (int32 X = 0; X < DebugGridSizeX; ++X)
            {
                FGridCoordinate Coord(X, Y, Floor);
                const FCellData* Data = CellDataMap.Find(Coord);
                
                // Determine color based on cell state
                FColor CellColor;
                if (!Data)
                {
                    CellColor = FColor(128, 128, 128, 50);  // Gray - no data
                }
                else if (!Data->bWalkable)
                {
                    CellColor = FColor(255, 0, 0, 100);     // Red - blocked
                }
                else if (Data->bOccupied)
                {
                    CellColor = FColor(255, 255, 0, 100);   // Yellow - occupied
                }
                else
                {
                    CellColor = FColor(0, 255, 0, 50);      // Green - walkable
                }
                
                // Calculate box position (at floor level, not centered vertically)
                FVector CellWorldPos = GridOrigin + FVector(
                    (X + 0.5f) * CellSize,
                    (Y + 0.5f) * CellSize,
                    Floor * FloorHeight + BoxHeight  // Just above floor
                );
                
                FVector BoxExtent(HalfCellSize - 2.0f, HalfCellSize - 2.0f, BoxHeight);
                
                DrawDebugBox(
                    GetWorld(),
                    CellWorldPos,
                    BoxExtent,
                    CellColor,
                    false,
                    -1.0f,
                    0,
                    2.0f
                );
            }
        }
    }

    // Draw highlighted (reachable) cells on top
    for (const FGridCoordinate& Coord : HighlightedCells)
    {
        FVector CellWorldPos = GridOrigin + FVector(
            (Coord.X + 0.5f) * CellSize,
            (Coord.Y + 0.5f) * CellSize,
            Coord.Floor * FloorHeight + 8.0f
        );

        FVector BoxExtent(CellSize * 0.5f - 2.0f, CellSize * 0.5f - 2.0f, 8.0f);

        DrawDebugBox(
            GetWorld(),
            CellWorldPos,
            BoxExtent,
            FColor(0, 150, 255, 180),
            false,
            -1.0f,
            0,
            4.0f
        );
    }
}

#if WITH_EDITOR
void AGridManager::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Fuerza redibujado inmediato en el editor
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(AGridManager, CellSize) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AGridManager, FloorHeight) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AGridManager, GridOrigin) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AGridManager, DebugGridSizeX) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AGridManager, DebugGridSizeY) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(AGridManager, DebugFloorCount))
		{
			// El tick se encargará del redibujado
		}
	}
}
#endif

// Cell data access functions
FCellData *AGridManager::GetCellData(const FGridCoordinate &Coord)
{
	return CellDataMap.Find(Coord);
}

const FCellData *AGridManager::GetCellData(const FGridCoordinate &Coord) const
{
	return CellDataMap.Find(Coord);
}

void AGridManager::SetCellData(const FGridCoordinate &Coord, const FCellData &Data)
{
	CellDataMap.Add(Coord, Data);
}

bool AGridManager::IsCellWalkable(const FGridCoordinate &Coord) const
{
	const FCellData *Data = CellDataMap.Find(Coord);
	return Data != nullptr ? Data->bWalkable : false;
}

bool AGridManager::IsCellOccupied(const FGridCoordinate &Coord) const
{
	const FCellData *Data = CellDataMap.Find(Coord);
	return Data != nullptr ? Data->bOccupied : false;
}

void AGridManager::InitializeGrid()
{
    ClearGrid();
    
    // Create all cells in the debug grid bounds
    for (int32 Floor = 0; Floor < DebugFloorCount; ++Floor)
    {
        for (int32 Y = 0; Y < DebugGridSizeY; ++Y)
        {
            for (int32 X = 0; X < DebugGridSizeX; ++X)
            {
                FGridCoordinate Coord(X, Y, Floor);
                FCellData DefaultData;
                
                CellDataMap.Add(Coord, DefaultData);
            }
        }
    }
    
    // Mark some cells as blocked for testing
    FCellData BlockedCell;
    BlockedCell.bWalkable = false;
    BlockedCell.bOccupied = false;
    
    SetCellData(FGridCoordinate(3, 3, 0), BlockedCell);
    SetCellData(FGridCoordinate(4, 3, 0), BlockedCell);
    SetCellData(FGridCoordinate(5, 3, 0), BlockedCell);
    SetCellData(FGridCoordinate(3, 4, 0), BlockedCell);
    SetCellData(FGridCoordinate(5, 4, 0), BlockedCell);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Grid initialized: %dx%dx%d = %d cells"), 
            DebugGridSizeX, DebugGridSizeY, DebugFloorCount, CellDataMap.Num()));
    }
}


void AGridManager::ClearGrid()
{
	CellDataMap.Empty();
}

void AGridManager::HighlightCells(const TArray<FGridCoordinate>& Cells)
{
	HighlightedCells = Cells;
}

void AGridManager::ClearHighlights()
{
	HighlightedCells.Empty();
}

void AGridManager::DebugDrawPath(FGridCoordinate Start, FGridCoordinate Goal)
{
    if (!GetWorld()) return;
    
    // Find path
    TArray<FGridCoordinate> Path = FPathfinder::FindPath(Start, Goal, this);
    
    if (Path.Num() == 0)
    {
        // No path found
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                TEXT("No path found!"));
        }
        return;
    }
    
    // Debug message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("Path found: %d steps"), Path.Num()));
    }
    
    // Draw lines connecting path cells
    for (int32 i = 0; i < Path.Num() - 1; ++i)
    {
        FVector Start3D = GridToWorld(Path[i]) + FVector(0, 0, 10);
        FVector End3D = GridToWorld(Path[i + 1]) + FVector(0, 0, 10);
        
        DrawDebugLine(
            GetWorld(),
            Start3D,
            End3D,
            FColor::Cyan,
            true,
            -1.0f,
            0,
            5.0f
        );
    }
    
    // Draw spheres at ALL waypoints
    for (int32 i = 0; i < Path.Num(); ++i)
    {
        FVector WaypointPos = GridToWorld(Path[i]) + FVector(0, 0, 10);
        
        // Last waypoint (goal) is green and bigger
        FColor SphereColor = (i == Path.Num() - 1) ? FColor::Green : FColor::Yellow;
        float SphereSize = (i == Path.Num() - 1) ? 30.0f : 20.0f;
        
        DrawDebugSphere(
            GetWorld(),
            WaypointPos,
            SphereSize,
            8,
            SphereColor,
            true,
            -1.0f
        );
    }
}