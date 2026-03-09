// TestArena.h - The performance stage. "This is our dungeon."
// Place in level or let GameMode auto-spawn. Provides floor, walls, ramps, and obstacles for testing.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestArena.generated.h"

UCLASS()
class CARGAME_API ATestArena : public AActor
{
	GENERATED_BODY()

public:
	ATestArena();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UStaticMeshComponent> Floor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UStaticMeshComponent> Ramp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arena")
	TObjectPtr<UStaticMeshComponent> Ramp2;

private:
	UStaticMeshComponent* CreateObstacle(FName Name, UStaticMesh* Mesh, FVector Location, FVector Scale, FRotator Rotation = FRotator::ZeroRotator);
};
