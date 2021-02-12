#pragma once
#include "CoreMinimal.h"
#include "RuntimeMeshComponent.h"

#include "TerrainConstants.generated.h"


UENUM()
enum class ECubeside : uint8
{
	BOTTOM, TOP, LEFT, RIGHT, FRONT, BACK,
};

USTRUCT()
struct FTerrainVertexData
{
	GENERATED_BODY()
public:
	UPROPERTY()
		TArray<FVector> Vertices;
	UPROPERTY()
		TArray<FVector> Normals;
	UPROPERTY()
		TArray<FVector2D> TextureCoordinates;
	UPROPERTY()
		TArray<int32> Triangles;

	UPROPERTY()
		TArray<FRuntimeMeshTangent> Tangents;
	UPROPERTY()
		TArray<FColor> VertexColors;
};

USTRUCT(Blueprintable, BlueprintType)
struct FTerrainConfiguration
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int32 ChunkSize = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		int32 BlockSize = 2000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		FIntVector WorldSize = { 32, 32, 1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float WorldFloor = -500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float WorldCeil = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float WorldCurve = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
		float SeedAngle = 90.0f;

	float TransformNoiseIntoWorldHeight(float noise_value) 
	{
		return FMath::Clamp(
			FMath::Pow((noise_value + 1.0f) / 2.0f, WorldCurve) * (WorldCeil - WorldFloor) + WorldFloor,
			WorldFloor, 
			WorldCeil);
	};
};

USTRUCT(Blueprintable, BlueprintType)
struct FTerrainNoiseOctave
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float amplitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float frequency; 
};


USTRUCT(Blueprintable, BlueprintType)
struct FTerrainNoiseParams
{
	GENERATED_BODY()
public:
	UPROPERTY()
		TArray<FTerrainNoiseOctave> octaves;
};