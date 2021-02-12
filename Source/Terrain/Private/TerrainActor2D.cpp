// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainActor2D.h"
#include "Engine/CollisionProfile.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Engine/Public/ImageUtils.h"
#include "CanvasItem.h"

DECLARE_CYCLE_STAT(TEXT("Terrain ~ UpdateStreaming"), STAT_UpdateStreaming, STATGROUP_TerrainActor2D);
DECLARE_CYCLE_STAT(TEXT("Terrain ~ UpdateStreaming - Add Chunks"), STAT_AddChunks, STATGROUP_TerrainActor2D);
DECLARE_CYCLE_STAT(TEXT("Terrain ~ UpdateStreaming - Prune Chunks"), STAT_PruneChunks, STATGROUP_TerrainActor2D);
DECLARE_CYCLE_STAT(TEXT("Terrain ~ DrawChunk"), STAT_DrawChunk, STATGROUP_TerrainActor2D);

// Sets default values
ATerrainActor2D::ATerrainActor2D()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.TickInterval = 0.2f;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	m_TerrainMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Terrain0"));
	m_TerrainMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	m_TerrainMeshComponent->Mobility = EComponentMobility::Static;
	m_TerrainMeshComponent->SetMeshSectionCollisionEnabled(0, true);
	m_TerrainMeshComponent->SetCanEverAffectNavigation(true);

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 20
	m_TerrainMeshComponent->SetGenerateOverlapEvents(false);
#else
	m_TerrainMeshComponent->bGenerateOverlapEvents = false;
#endif
	m_TerrainMeshComponent->ClearAllMeshSections();

	m_GridManagerComponent = CreateDefaultSubobject<UGridManagerComponent>(TEXT("ChunkMan0"));
	RootComponent = m_TerrainMeshComponent;
	//myNoise.SetNoiseType(FastNoise::SimplexFractal); // Set the desired noise type

}

void ATerrainActor2D::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Init();
}
void ATerrainActor2D::Init()
{
	m_GridManagerComponent->Init(
		m_TerrainConfig,
		{ m_Octaves }
	);
	//myNoise.SetFrequency(Frequency);
	mesh_section = 0;
	m_TerrainMeshComponent->ClearAllMeshSections();
	m_MeshSectionLookup.Empty();
	
	MinimapCanvas = Cast<UCanvasRenderTarget2D>(
		UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
			GetWorld(), UCanvasRenderTarget2D::StaticClass(), m_CanvasSize, m_CanvasSize));
	MinimapCanvas->MipGenSettings = TMGS_NoMipmaps;

	//MinimapCanvas->InitAutoFormat(256, 256);
	//MinimapCanvas->CreateCanvasRenderTarget2D(
	//	GetWorld(), UCanvasRenderTarget2D::StaticClass(), 256, 256);
	MinimapCanvas->OnCanvasRenderTargetUpdate.AddDynamic(this, 
		&ATerrainActor2D::RedrawMinimapBackground);

	UpdateChunkStreaming();
}

void ATerrainActor2D::OnConstruction(const FTransform & Transform)
{
	Init();
}


void ATerrainActor2D::RedrawMinimapBackground(class UCanvas* Canvas, int32 Width, int32 Height)
{
	for (const auto &chunk_key : m_ResidentChunks)
	{
		auto chunk = m_GridManagerComponent->GetChunk(chunk_key);
		if (chunk == nullptr)
			continue;
		FVector draw_location = (chunk->world_location() - CameraLocation);
		draw_location /= m_TerrainConfig.BlockSize;
		
		float draw_offset = m_CanvasSize / 2;
		draw_location += FVector(draw_offset, draw_offset, draw_offset);
		auto tex = chunk->get_data_image();
		
		// TODO: smooth minimap motion
		//Canvas->DrawTile(tex, FMath::FloorToFloat(draw_location.X), FMath::FloorToFloat(draw_location.Y),
		Canvas->DrawTile(tex, draw_location.X, draw_location.Y,
			tex->GetSizeX(), tex->GetSizeY(), 0, 0, tex->GetSizeX(), tex->GetSizeY(), BLEND_Additive);
			//test_vector.X, test_vector.Y, 0, 0, test_vector.Z, test_vector.W);

	}
}

void ATerrainActor2D::UpdateChunkStreaming()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateStreaming);
	auto chunks = m_GridManagerComponent->GetChunksAroundCamera(CameraLocation, CameraChunkRadius);
	m_ResidentChunks.Empty();
	bool update_minimap = false;
	for (const auto &chunk : chunks)
	{
		SCOPE_CYCLE_COUNTER(STAT_AddChunks);
		FIntVector loc = (*chunk).location();
		if (!m_MeshSectionLookup.Contains(loc))
		{
			m_MeshSectionLookup.Add(loc, mesh_section);
			DrawChunk(*chunk, mesh_section);
			
			update_minimap = true;
			mesh_section++;
		}
		m_ResidentChunks.Add(loc);
	}

	for (auto it = m_MeshSectionLookup.CreateIterator(); it; ++it)
	{
		SCOPE_CYCLE_COUNTER(STAT_PruneChunks);
		if(!m_ResidentChunks.Contains(it->Key))
		{
			m_TerrainMeshComponent->ClearMeshSection(it->Value);
			it.RemoveCurrent();
		}
	}

	//if (update_minimap)
	{
		MinimapCanvas->UpdateResource();
	}
	
	
}


void ATerrainActor2D::DrawChunk(UChunk & chunk, int32 section)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawChunk);
	FTerrainVertexData vertex_data = FTerrainVertexData();

	m_GridManagerComponent->SetVertexDataForChunk(chunk, vertex_data);

	m_TerrainMeshComponent->ClearMeshSection(section);
	m_TerrainMeshComponent->GetOrCreateRuntimeMesh()->CreateMeshSection(
		section, 
		vertex_data.Vertices, vertex_data.Triangles, vertex_data.Normals, 
		vertex_data.TextureCoordinates, vertex_data.VertexColors, vertex_data.Tangents, true);

	m_TerrainMeshComponent->GetOrCreateRuntimeMesh()->SetSectionMaterial(section, DefaultMaterial);

}
// Called when the game starts or when spawned
void ATerrainActor2D::BeginPlay()
{
	Super::BeginPlay();

	
}

// Called every frame
void ATerrainActor2D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CameraReference != nullptr)
	{
		CameraLocation = CameraReference->GetActorLocation();
	}
	else
	{
		CameraLocation = FVector(0.f, 0.f, 0.f);
		auto pc = GetWorld()->GetFirstPlayerController();
		if (pc != nullptr)
			CameraLocation = pc->GetViewTarget()->GetActorLocation();
	}
	UpdateChunkStreaming();
}

