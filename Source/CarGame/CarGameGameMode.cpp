// CarGameGameMode.cpp
// "Let's go!" - Mark Wolff

#include "CarGameGameMode.h"
#include "CarPawn.h"
#include "TestArena.h"
#include "UObject/ConstructorHelpers.h"

ACarGameGameMode::ACarGameGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> CarPawnBP(
		TEXT("/Game/Blueprints/BP_CarPawn"));
	if (CarPawnBP.Succeeded())
	{
		DefaultPawnClass = CarPawnBP.Class;
	}
	else
	{
		DefaultPawnClass = ACarPawn::StaticClass();
	}
}

void ACarGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnTestArena)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		GetWorld()->SpawnActor<ATestArena>(
			ATestArena::StaticClass(), TestArenaLocation, FRotator::ZeroRotator, SpawnParams);

		UE_LOG(LogTemp, Log, TEXT("TestArena spawned at %s. Set bSpawnTestArena=false in GameMode to disable."),
			*TestArenaLocation.ToString());
	}
}
