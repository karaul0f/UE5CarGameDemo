// TestArena.cpp - "Welcome to the dungeon. I'm the master, and you're the slaves."
// A self-contained test environment with floor, walls, ramps, and scattered obstacles.

#include "TestArena.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ATestArena::ATestArena()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Find basic meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!CubeFinder.Succeeded()) return;
	UStaticMesh* CubeMesh = CubeFinder.Object;

	// Floor removed — the level already has a landscape.
	// If you need a floor (empty level), uncomment:
	// Floor = CreateObstacle(TEXT("Floor"), CubeMesh,
	//     FVector(0.f, 0.f, -50.f), FVector(300.f, 300.f, 1.f));

	// ---- Outer walls: ring of 8 thick walls ----
	const int32 NumWalls = 8;
	const float WallRadius = 5000.f;
	for (int32 i = 0; i < NumWalls; i++)
	{
		FName WallName = *FString::Printf(TEXT("Wall_%d"), i);
		float Angle = (360.f / NumWalls) * i;
		float Rad = FMath::DegreesToRadians(Angle);

		FVector Pos(
			FMath::Cos(Rad) * WallRadius,
			FMath::Sin(Rad) * WallRadius,
			150.f
		);

		CreateObstacle(WallName, CubeMesh, Pos,
			FVector(8.f, 2.f, 4.f),
			FRotator(0.f, Angle + 90.f, 0.f));
	}

	// ---- Inner obstacles: scattered cubes to crash into ----
	struct FObstacleInfo { FName Name; FVector Pos; FVector Scale; };
	const FObstacleInfo InnerObstacles[] = {
		{ TEXT("Pillar_0"), FVector(1200.f, 600.f, 150.f),   FVector(1.5f, 1.5f, 3.f) },
		{ TEXT("Pillar_1"), FVector(-1000.f, -800.f, 150.f),  FVector(1.5f, 1.5f, 3.f) },
		{ TEXT("Pillar_2"), FVector(600.f, -1400.f, 150.f),   FVector(1.5f, 1.5f, 3.f) },
		{ TEXT("Pillar_3"), FVector(-1800.f, 1000.f, 150.f),  FVector(1.5f, 1.5f, 3.f) },
		{ TEXT("Block_0"),  FVector(0.f, 2500.f, 100.f),      FVector(3.f, 3.f, 2.f) },
		{ TEXT("Block_1"),  FVector(-2500.f, -200.f, 100.f),   FVector(4.f, 1.f, 2.f) },
		{ TEXT("Block_2"),  FVector(2800.f, -1500.f, 100.f),   FVector(2.f, 5.f, 2.f) },
		{ TEXT("Block_3"),  FVector(800.f, 3000.f, 100.f),    FVector(6.f, 1.f, 2.f) },
	};
	for (const auto& Obs : InnerObstacles)
	{
		CreateObstacle(Obs.Name, CubeMesh, Obs.Pos, Obs.Scale);
	}

	// ---- Ramp 1: gentle slope for speed launch ----
	Ramp = CreateObstacle(TEXT("Ramp"), CubeMesh,
		FVector(2500.f, 0.f, 30.f),
		FVector(8.f, 5.f, 0.3f),
		FRotator(-12.f, 0.f, 0.f));

	// ---- Ramp 2: steeper ramp aimed at a wall ----
	Ramp2 = CreateObstacle(TEXT("Ramp2"), CubeMesh,
		FVector(-2000.f, 2000.f, 30.f),
		FVector(6.f, 4.f, 0.3f),
		FRotator(-20.f, 135.f, 0.f));
}

UStaticMeshComponent* ATestArena::CreateObstacle(FName Name, UStaticMesh* Mesh,
	FVector Location, FVector Scale, FRotator Rotation)
{
	UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Comp->SetupAttachment(Root);
	Comp->SetStaticMesh(Mesh);
	Comp->SetRelativeLocation(Location);
	Comp->SetRelativeScale3D(Scale);
	Comp->SetRelativeRotation(Rotation);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->SetNotifyRigidBodyCollision(true);
	return Comp;
}
