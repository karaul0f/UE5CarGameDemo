// CarGameGameMode.h - The dungeon master of this whole operation.
// "That's the right answer!" - Van Darkholme

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CarGameGameMode.generated.h"

UCLASS()
class CARGAME_API ACarGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACarGameGameMode();

	virtual void BeginPlay() override;

	/** Auto-spawn the test arena at game start. Disable once you have a real level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bSpawnTestArena = true;

	/** Where to spawn the test arena */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FVector TestArenaLocation = FVector::ZeroVector;
};
