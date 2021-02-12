// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManagerComponent.h"
#include "srdnoise23.h"
#include "ImageUtils.h"

// Sets default values for this component's properties
UGridManagerComponent::UGridManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UGridManagerComponent::Init(FTerrainConfiguration tconfig, FTerrainNoiseParams nparams) {
	m_ChunkSize = tconfig.ChunkSize;
	m_BlockSize = tconfig.BlockSize;
	m_WorldSize = tconfig.WorldSize;

	m_TerrainConfig = tconfig;
	m_NoiseParams = nparams;

	m_Chunks.Empty();
	
}
TArray<UChunk*> UGridManagerComponent::GetChunksAroundCamera(FVector cam_location, uint8 camera_chunk_radius)
{
	TArray<UChunk*> out_chunks;
	FIntVector cam_chunk_vector = WorldToChunkLocation(cam_location);
	for (int y = cam_chunk_vector.Y - camera_chunk_radius; y <= cam_chunk_vector.Y + camera_chunk_radius; y++)
	{
		for (int x = cam_chunk_vector.X - camera_chunk_radius; x <= cam_chunk_vector.X + camera_chunk_radius; x++)
		{
			FIntVector key = FIntVector(x, y, 0);
			auto ch = GetOrCreateChunk(key);
			if (ch == nullptr)
				continue;
			out_chunks.Add(ch);
		}
	}
	return out_chunks;
}

FIntVector UGridManagerComponent::WorldToChunkLocation(FVector loc)
{
	float chunk_span = float(m_BlockSize * m_ChunkSize);
	//TODO: consider biasing to camera angle or velocity
	FIntVector chunk_key = FIntVector((loc + (chunk_span / 2.0f)) / chunk_span);
	return chunk_key;
}
UChunk * UGridManagerComponent::GetOrCreateChunk(uint16 xx, uint16 yy, uint16 zz)
{
	return GetOrCreateChunk(FIntVector({ xx, yy, zz }));
}
UChunk * UGridManagerComponent::GetOrCreateChunk(FIntVector location)
{
	//TODO: support infinite ocean somehow, maybe send a static chunk full of zeroes?
	if (FMath::Abs(location.X) > m_WorldSize.X - 1 ||
		FMath::Abs(location.Y) > m_WorldSize.Y - 1)
	{
		return nullptr;
	}
	if (!m_Chunks.Contains(location))
	{
		CreateChunk(location);
	}

	return m_Chunks[location];
}

UChunk * UGridManagerComponent::GetChunk(FIntVector location)
{
	//TODO: support infinite ocean somehow, maybe send a static chunk full of zeroes?
	if (FMath::Abs(location.X) > m_WorldSize.X - 1 ||
		FMath::Abs(location.Y) > m_WorldSize.Y - 1)
	{
		return nullptr;
	}
	if (!m_Chunks.Contains(location))
	{
		return nullptr;
	}		

	return m_Chunks[location];
}

// Called when the game starts
void UGridManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UGridManagerComponent::CreateChunk(FIntVector location)
{
	auto new_chunk = NewObject<UChunk>();
	new_chunk->Init(location, m_ChunkSize, m_BlockSize);
	auto world_loc = new_chunk->world_location();
	for (int y = 0; y < m_ChunkSize; y++)
	{
		for (int x = 0; x < m_ChunkSize; x++)
		{
		
			FVector pos = world_loc + FVector(x, y, 0) * FVector(m_BlockSize);

			double noise_val = 0.0;
			float dx = 0.0f;
			float dy = 0.0f;
			float dx_param = 0.0f;
			float dy_param = 0.0f;
			float num_octaves = 0.0f;
			for (FTerrainNoiseOctave & octave : m_NoiseParams.octaves) {
				noise_val += srdnoise2(pos.X, pos.Y, octave.frequency * .00001, m_TerrainConfig.SeedAngle, &dx_param, &dy_param) * octave.amplitude;
				dx += dx_param;
				dy += dy_param;
				num_octaves += 1.0f;
			}
			noise_val = m_TerrainConfig.TransformNoiseIntoWorldHeight(noise_val);
			if (num_octaves > 0.0f) {
				dx /= num_octaves;
				dy /= num_octaves;
			}
			dx = FMath::Pow((dx + 1.0f) / 2.0f, m_TerrainConfig.WorldCurve);
			dy = FMath::Pow((dy + 1.0f) / 2.0f, m_TerrainConfig.WorldCurve);
			new_chunk->set_corner(x, y, 0, noise_val, dx, dy);
			
		}
	}
	new_chunk->update_data_image();
	m_Chunks.Add(location, new_chunk);
	//m_Chunks[location] = new_chunk;
}

void UGridManagerComponent::SetVertexDataForChunk(UChunk & chunk, FTerrainVertexData & vdata)
{
	vdata.Vertices.SetNumZeroed((m_ChunkSize + 1) * (m_ChunkSize + 1));
	vdata.Normals.SetNumZeroed((m_ChunkSize + 1) * (m_ChunkSize + 1));
	vdata.TextureCoordinates.SetNumZeroed((m_ChunkSize + 1) * (m_ChunkSize + 1));
	vdata.Triangles.SetNumZeroed((m_ChunkSize + 1) * (m_ChunkSize + 1) * 6);

	FVector wloc = chunk.world_location();
	FIntVector chunk_index = FIntVector();
	FIntVector corner_index = FIntVector();
	
	for (int y = 0; y < (m_ChunkSize + 1); y++)
	{
		for (int x = 0; x < (m_ChunkSize + 1); x++)
		{
			chunk_index = chunk.location();
			corner_index = FIntVector(x, y, 0);
			if (x == m_ChunkSize)
			{
				chunk_index.X += 1;
				corner_index.X = 0;
			}
			if (y == m_ChunkSize)
			{
				chunk_index.Y += 1;
				corner_index.Y = 0;
			}
			auto chk = GetOrCreateChunk(chunk_index);
			CORNER cnr;
			if (chk == nullptr)
			{
				cnr = CORNER({ 0.0 });
			}
			else {
				cnr = chk->get_corner(corner_index);
			}

			int key = (y * (m_ChunkSize + 1)) + x;
			vdata.Vertices[key] = FVector(
				x * m_BlockSize + wloc.X,
				y * m_BlockSize + wloc.Y,
				cnr.val + wloc.Z
			);
			vdata.Normals[key] = FVector(-cnr.dx, -cnr.dy, 1.0f).GetSafeNormal();
		}
	}
	
	for (int i = 1; i < m_ChunkSize * m_ChunkSize + m_ChunkSize; i++)
	{ 
		if (i % (m_ChunkSize + 1) == 0)
			continue;

		vdata.Triangles[i*6] = i - 1;
		vdata.Triangles[i*6 + 1] = i + m_ChunkSize;
		vdata.Triangles[i*6 + 2] = i;
		vdata.Triangles[i*6 + 3] = i + m_ChunkSize;
		vdata.Triangles[i*6 + 4] = i + m_ChunkSize + 1;
		vdata.Triangles[i*6 + 5] = i;
	}

	vdata.Vertices.Num();
	vdata.Triangles.Num();

}

// Called every frame
void UGridManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

