// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"

#include "Chunk.h"
#include "MarchingCubes.h"
#include "TerrainConstants.h"


#include "GridManagerComponent.generated.h"

class URuntimeMeshComponent;

UCLASS( meta=(BlueprintSpawnableComponent) )
class TERRAIN_API UGridManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	//const uint16 WORLD_OFFSET = 16384;
	
	UGridManagerComponent(const FObjectInitializer& ObjectInitializer);
	//TODO: maybe do world bounds instead you idiot
	void Init(FTerrainConfiguration tconfig, FTerrainNoiseParams nparams);

	TArray<UChunk*> GetChunksAroundCamera(FVector cam_location, uint8 camera_chunk_radius);
	
	UChunk* GetChunk(FIntVector location);
	UChunk* GetOrCreateChunk(FIntVector location);
	UChunk* GetOrCreateChunk(uint16 xx, uint16 yy, uint16 zz);

	FIntVector WorldToChunkLocation(FVector loc);
	void SetVertexDataForChunk(UChunk & chunk, FTerrainVertexData & vdata);
		//const GRIDCELL GetGridcell(FIntVector chunk_xyz, FIntVector gridcell_xyz);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void CreateChunk(FIntVector location);
	uint16 m_ChunkSize;
	uint16 m_BlockSize;
	FIntVector m_WorldSize;
	float m_WorldFloor;
	float m_WorldCeil;
	FTerrainConfiguration m_TerrainConfig;
	FTerrainNoiseParams m_NoiseParams;

	UTexture2D* m_MinimapSlice;

	UPROPERTY()
	TMap<FIntVector, UChunk*> m_Chunks;

private:

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
