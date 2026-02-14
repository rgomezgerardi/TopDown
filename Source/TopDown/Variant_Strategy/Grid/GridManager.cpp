// Fill out your copyright notice in the Description page of Project Settings.


#include "Grid/GridManager.h"
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
	int32 Floor = FMath::FloorToInt(LocalPosition.Z / FloorHeight);

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
	if (!GetWorld()) //Prevets crash when world is null
		return;

	const float LineThickness = 2.0f;
	const FColor FloorColors[] = {
		FColor::Green,
		FColor::Blue,
		FColor::Yellow,
		FColor::Cyan,
		FColor::Magenta};

	for (int32 Floor = 0; Floor < DebugFloorCount; ++Floor)
	{
		float Z = GridOrigin.Z + (Floor * FloorHeight); // Calculate world height for this floor
		FColor Color = FloorColors[Floor % 5]; // Cycle through colors

		// Horizontal Lines (Y)
		for (int32 X = 0; X <= DebugGridSizeX; ++X)
		{
			FVector Start = GridOrigin + FVector(X * CellSize, 0, Floor * FloorHeight);
			FVector End = GridOrigin + FVector(X * CellSize, DebugGridSizeY * CellSize, Floor * FloorHeight);
			DrawDebugLine(GetWorld(), Start, End, Color, false, -1.0f, 0, LineThickness);
		}

		// Vertical Lines (X)
		for (int32 Y = 0; Y <= DebugGridSizeY; ++Y)
		{
			FVector Start = GridOrigin + FVector(0, Y * CellSize, Floor * FloorHeight);
			FVector End = GridOrigin + FVector(DebugGridSizeX * CellSize, Y * CellSize, Floor * FloorHeight);
			DrawDebugLine(GetWorld(), Start, End, Color, false, -1.0f, 0, LineThickness);
		}
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