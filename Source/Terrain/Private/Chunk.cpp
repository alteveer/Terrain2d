#include "Chunk.h"

void UChunk::Init(FIntVector loc, int32 chunk_size, int32 block_size)
{
	m_Location = loc;
	m_ChunkSize = chunk_size;
	m_BlockSize = block_size;
	m_Corners.Init(CORNER({ 0.0 }), chunk_size * chunk_size);
}

const CORNER & UChunk::get_corner(int32 xx, int32 yy, int32 zz)
{
	int32 idx = xx + (m_ChunkSize * yy);
	return m_Corners[idx];
}

const CORNER & UChunk::get_corner(FIntVector v)
{
	return get_corner(v.X, v.Y, v.Z);
}

void UChunk::set_corner(int32 xx, int32 yy, int32 zz, double val, float dx, float dy)
{
	int32 key = xx + (m_ChunkSize * yy);
	m_Corners[key].val = val;
	m_Corners[key].dx = dx;
	m_Corners[key].dy = dy;
}

const FIntVector UChunk::location()
{
	return m_Location;
}

const FVector UChunk::world_location()
{
	return FVector(m_Location) * FVector(m_ChunkSize) * FVector(m_BlockSize);
}

const uint8 UChunk::chunksize()
{
	return m_ChunkSize;
}

const TArray<FColor> UChunk::get_data_for_minimap()
{
	TArray<FColor> out_array;
	for (CORNER& corner : m_Corners)
	{
		out_array.Add(FColor(corner.val > 0 ? 255 : 0, 0, 0));
	}
	return out_array;
}

void UChunk::update_data_image()
{
	FCreateTexture2DParameters Params;
	Params.bDeferCompression = true;
	Params.bSRGB = false;
	
	Params.CompressionSettings = TC_Grayscale;
	Params.bUseAlpha = false;

	m_DataImage = FImageUtils::CreateTexture2D(
		m_ChunkSize, m_ChunkSize,
		this->get_data_for_minimap(),
		this, m_Location.ToString(), RF_NoFlags, Params);
	m_DataImage->AddressX = TA_Clamp;
	m_DataImage->AddressY = TA_Clamp;
	m_DataImage->RefreshSamplerStates();
}

UTexture2D * UChunk::get_data_image()
{
	return m_DataImage;
}