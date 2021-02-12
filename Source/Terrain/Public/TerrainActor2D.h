// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"
#include "GridManagerComponent.h"
#include "TerrainConstants.h"
//#include "FastNoise/FastNoise.h"

#include "TerrainActor2D.generated.h"

class URuntimeMeshComponent;
class UGridManagerComponent;
class UCanvas;
class UCanvasRenderTarget2D;

DECLARE_STATS_GROUP(TEXT("TerrainActor2D"), STATGROUP_TerrainActor2D, STATCAT_Advanced);

UCLASS(HideCategories=(Materials))
class TERRAIN_API ATerrainActor2D : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATerrainActor2D();
	virtual bool ShouldTickIfViewportsOnly() const override {
		return true;
	};
	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateChunkStreaming();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		UCanvasRenderTarget2D* MinimapCanvas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		UTexture2D* TestImage;
protected:
	virtual void PostInitializeComponents() override;
	void Init();
	void OnConstruction(const FTransform& Transform);

	UFUNCTION(BlueprintCallable)
	void RedrawMinimapBackground(class UCanvas* Canvas, int32 Width, int32 Height);
	
	void DrawChunk(UChunk & chunk, int32 section);
	//void MarchCube(GRIDCELL cell, FTerrainVertexData & vdata);
	//void CreateQuad(ECubeside side, FVector pos, FTerrainVertexData& vdata);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URuntimeMeshComponent* m_TerrainMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGridManagerComponent* m_GridManagerComponent;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	//	uint8 WorldSizeX = 16;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	//	uint8 WorldSizeY = 16;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		UMaterialInterface* DefaultMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		AActor* CameraReference;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		FVector CameraLocation = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		uint8 CameraChunkRadius = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		FTerrainConfiguration m_TerrainConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		TArray<FTerrainNoiseOctave> m_Octaves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int m_CanvasSize = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		FVector4 test_vector = FVector4(32, 32, 256, 256);

	UPROPERTY()
		int32 mesh_section = 0;
	TMap<FIntVector, int32> m_MeshSectionLookup;
	TArray<FIntVector> m_ResidentChunks;
	//	FastNoise myNoise; // Create a FastNoise object

public:
//	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
		
};

