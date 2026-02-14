// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridTypes.h"
#include "CoreMinimal.h"
#include "Grid/CellData.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UCLASS()
class TOPDOWN_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridManager();

	// Grid Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float FloorHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;

	// Convertion World ↔ Grid
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FGridCoordinate WorldToGrid(const FVector &WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GridToWorld(const FGridCoordinate &GridCoord) const;

	// Validación
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsValidGridPosition(const FGridCoordinate &GridCoord) const;


	// Cell Data Management
	/** 
	* Get cell data at coordinate. Returns nullptr if cell doesn't exist.
	* Returned pointer is valid until CellDataMap is modified.
	*/
	FCellData *GetCellData(const FGridCoordinate &Coord);

	/** Get cell data (const version). Returns nullptr if cell doesn't exist. */
	const FCellData *GetCellData(const FGridCoordinate &Coord) const;

	/** Set cell data at coordinate. Creates cell if it doesn't exist. */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetCellData(const FGridCoordinate &Coord, const FCellData &Data);

	/** Check if cell is walkable. Returns false if cell doesn't exist. */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsCellWalkable(const FGridCoordinate &Coord) const;

	/** Check if cell is occupied. Returns false if cell doesn't exist. */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsCellOccupied(const FGridCoordinate &Coord) const;

	
	// Grid initialization
	/** Initialize grid with default walkable cells */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGrid();

	/** Clear all cell data */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ClearGrid();


	// Debugging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	bool bShowDebugGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	int32 DebugGridSizeX = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	int32 DebugGridSizeY = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	int32 DebugFloorCount = 3;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void DrawDebugGrid() const;

	/** Storage for all cell data in the grid */
	UPROPERTY()
	TMap<FGridCoordinate, FCellData> CellDataMap;
};
