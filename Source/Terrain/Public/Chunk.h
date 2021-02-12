// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "Engine/Public/ImageUtils.h"

#include "Chunk.generated.h"


typedef struct {
	double val;
	float dx;
	float dy;
} CORNER;

UCLASS()
class TERRAIN_API UChunk : public UObject
{
	GENERATED_BODY()
public:
	void Init(FIntVector loc, int32 chunk_size, int32 block_size);

	const CORNER & get_corner(int32 xx, int32 yy, int32 zz);
	const CORNER & get_corner(FIntVector v);
	void set_corner(int32 xx, int32 yy, int32 zz, double val, float dx, float dy);

	const FIntVector location();
	const FVector world_location();
	const uint8 chunksize();

	const TArray<FColor> get_data_for_minimap();
	void update_data_image();
	UTexture2D* get_data_image();

protected:
	FIntVector m_Location;
	uint8 m_ChunkSize;
	uint16 m_BlockSize;
	
	UPROPERTY(EditAnywhere)
	UTexture2D* m_DataImage;

	TArray<CORNER> m_Corners;
	
};